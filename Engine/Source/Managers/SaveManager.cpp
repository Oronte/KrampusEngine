#include "SaveManager.h"
#include <Utilities/Debug/Exception.h>
#include "Utilities/Debug/Logger.h"
#include "Core/AssetPack.h"
#include <bit>
#include <span>


String                      SaveManager::saveDirectory = SaveManager::SAVE_DIRECTORY;
SaveManager::EventCallback  SaveManager::onSave = nullptr;
SaveManager::EventCallback  SaveManager::onLoad = nullptr;

namespace
{
    constexpr Byte KA7_MAGIC[] = { 'K', 'A', '7', 0 };
    constexpr std::size_t KA7_HEADER_SIZE = 32;

    /// CRC-32 (IEEE 802.3, reflected polynomial 0xEDB88320), bit by bit.
    Int ComputeCRC32(const std::vector<Byte>& _data) noexcept
    {
        constexpr std::uint32_t POLYNOMIAL = 0xEDB88320u;
        std::uint32_t _crc = 0xFFFFFFFFu;
        for (const Byte _byte : _data)
        {
            _crc ^= _byte;
            for (int _bit = 0; _bit < 8; ++_bit)
                _crc = (_crc >> 1) ^ (POLYNOMIAL & (0u - (_crc & 1u)));
        }
        return static_cast<Int>(~_crc);
    }

    /// Little-endian writer of the .ka7 binary format.
    class BinaryWriter
    {
    public:
        std::vector<Byte> buffer;

        void WriteU8(Byte _value) { buffer.push_back(_value); }

        template<typename T>
        void WriteLittleEndian(T _value)
        {
            const auto _bits = static_cast<std::make_unsigned_t<T>>(_value);
            for (std::size_t _i = 0; _i < sizeof(T); ++_i)
                buffer.push_back(static_cast<Byte>(_bits >> (_i * 8)));
        }

        void WriteU16(Short _value)     { WriteLittleEndian(_value); }
        void WriteU32(Int _value)       { WriteLittleEndian(_value); }
        void WriteI64(LongLong _value)  { WriteLittleEndian(_value); }
        void WriteDouble(Double _value) { WriteI64(std::bit_cast<LongLong>(_value)); }

        void WriteString(const String& _string)
        {
            const std::string& _raw = _string;
            WriteU32(static_cast<Int>(_raw.size()));
            buffer.insert(buffer.end(), _raw.begin(), _raw.end());
        }

        void WriteNode(const SaveNode& _node)
        {
            WriteString(_node.GetName());
            WriteU32(static_cast<Int>(_node.GetFields().size()));
            for (const auto& [_key, _value] : _node.GetFields())
            {
                WriteString(_key);
                std::visit([this](const auto& _alternative) { WriteVariant(_alternative); }, _value);
            }
        }

    private:
        void WriteType(SaveType _type) { WriteU8(static_cast<Byte>(_type)); }

        void WriteNodeOrEmpty(const SaveNodePtr& _node) { WriteNode(_node ? *_node : SaveNode{}); }

        template<typename T, typename WriteOne>
        void WriteArray(SaveType _type, const std::vector<T>& _values, WriteOne _writeOne)
        {
            WriteType(_type);
            WriteU32(static_cast<Int>(_values.size()));
            for (const T& _value : _values) (this->*_writeOne)(_value);
        }

        void WriteVariant(Bool _value)               { WriteType(SaveType::Bool);     WriteU8(_value ? 1 : 0); }
        void WriteVariant(LongLong _value)           { WriteType(SaveType::LongLong); WriteI64(_value); }
        void WriteVariant(Double _value)             { WriteType(SaveType::Double);   WriteDouble(_value); }
        void WriteVariant(const String& _value)      { WriteType(SaveType::String);   WriteString(_value); }
        void WriteVariant(const SaveNodePtr& _value) { WriteType(SaveType::Node);     WriteNodeOrEmpty(_value); }
        void WriteVariant(const std::vector<Byte>& _value)
        {
            WriteType(SaveType::Bytes);
            WriteU32(static_cast<Int>(_value.size()));
            buffer.insert(buffer.end(), _value.begin(), _value.end());
        }
        void WriteVariant(const std::vector<LongLong>& _value)    { WriteArray(SaveType::ArrayLongLong, _value, &BinaryWriter::WriteI64); }
        void WriteVariant(const std::vector<Double>& _value)      { WriteArray(SaveType::ArrayDouble,   _value, &BinaryWriter::WriteDouble); }
        void WriteVariant(const std::vector<String>& _value)      { WriteArray(SaveType::ArrayString,   _value, &BinaryWriter::WriteString); }
        void WriteVariant(const std::vector<SaveNodePtr>& _value) { WriteArray(SaveType::ArrayNode,     _value, &BinaryWriter::WriteNodeOrEmpty); }
    };

    /// Little-endian reader of the .ka7 binary format. Throws on truncated or corrupted data.
    class BinaryReader
    {
    public:
        explicit BinaryReader(const std::vector<Byte>& _buffer) : buffer(_buffer) {}

        Byte ReadU8()
        {
            CheckBounds(1);
            return buffer[position++];
        }

        template<typename T>
        T ReadLittleEndian()
        {
            using Bits = std::make_unsigned_t<T>;
            CheckBounds(sizeof(T));
            Bits _bits = 0;
            for (std::size_t _i = 0; _i < sizeof(T); ++_i)
                _bits |= static_cast<Bits>(static_cast<Bits>(buffer[position + _i]) << (_i * 8));
            position += sizeof(T);
            return static_cast<T>(_bits);
        }

        Short    ReadU16()    { return ReadLittleEndian<Short>(); }
        Int      ReadU32()    { return ReadLittleEndian<Int>(); }
        LongLong ReadI64()    { return ReadLittleEndian<LongLong>(); }
        Double   ReadDouble() { return std::bit_cast<Double>(ReadI64()); }

        String ReadString()
        {
            const auto _bytes = ReadSpan(ReadCount());
            return String(std::string(_bytes.begin(), _bytes.end()));
        }

        SaveNode ReadNode()
        {
            SaveNode _node(ReadString());
            const std::size_t _fieldCount = ReadCount();
            _node.GetFields().reserve(_fieldCount);
            for (std::size_t _i = 0; _i < _fieldCount; ++_i)
            {
                String _key = ReadString();
                _node.GetFields()[std::move(_key)] = ReadValue();
            }
            return _node;
        }

        SaveValue ReadValue()
        {
            switch (static_cast<SaveType>(ReadU8()))
            {
            case SaveType::Bool:          return ReadU8() != 0;
            case SaveType::LongLong:      return ReadI64();
            case SaveType::Double:        return ReadDouble();
            case SaveType::String:        return ReadString();
            case SaveType::Bytes:
            {
                const auto _bytes = ReadSpan(ReadCount());
                return std::vector<Byte>(_bytes.begin(), _bytes.end());
            }
            case SaveType::ArrayLongLong: return ReadArray<LongLong>(&BinaryReader::ReadI64);
            case SaveType::ArrayDouble:   return ReadArray<Double>(&BinaryReader::ReadDouble);
            case SaveType::ArrayString:   return ReadArray<String>(&BinaryReader::ReadString);
            case SaveType::Node:          return ReadNodePtr();
            case SaveType::ArrayNode:     return ReadArray<SaveNodePtr>(&BinaryReader::ReadNodePtr);
            }
            THROW_EXCEPTION("Ka7::SaveManager: unknown value type in the .ka7 file");
        }

    private:
        const std::vector<Byte>& buffer;
        std::size_t position = 0;

        SaveNodePtr ReadNodePtr() { return std::make_shared<SaveNode>(ReadNode()); }

        /// A count read from the file can be anything: a negative one is rejected here.
        std::size_t ReadCount()
        {
            const Int _count = ReadU32();
            if (_count < 0) THROW_EXCEPTION("Ka7::SaveManager: truncated or corrupted data");
            return static_cast<std::size_t>(_count);
        }

        std::span<const Byte> ReadSpan(std::size_t _size)
        {
            CheckBounds(_size);
            const std::span<const Byte> _bytes(buffer.data() + position, _size);
            position += _size;
            return _bytes;
        }

        template<typename T>
        std::vector<T> ReadArray(T (BinaryReader::*_readOne)())
        {
            const std::size_t _count = ReadCount();
            CheckBounds(_count);   // every element takes at least one byte: a corrupted count cannot reserve gigabytes
            std::vector<T> _values;
            _values.reserve(_count);
            for (std::size_t _i = 0; _i < _count; ++_i) _values.push_back((this->*_readOne)());
            return _values;
        }

        void CheckBounds(std::size_t _size) const
        {
            if (_size > buffer.size() - position)
                THROW_EXCEPTION("Ka7::SaveManager: truncated or corrupted data");
        }
    };

    void WriteHeader(BinaryWriter& _writer, Short _version, Short _flags, LongLong _timestamp, Int _checksum, Int _dataSize)
    {
        for (const Byte _byte : KA7_MAGIC) _writer.WriteU8(_byte);
        _writer.WriteU16(_version);
        _writer.WriteU16(_flags);
        _writer.WriteI64(_timestamp);
        _writer.WriteU32(_checksum);
        _writer.WriteU32(_dataSize);
        _writer.WriteU32(0);   // reserved
        _writer.WriteU32(0);   // reserved
    }

    struct ParsedHeader
    {
        Short    version   = 0;
        Short    flags     = 0;
        LongLong timestamp = 0;
        Int      checksum  = 0;
        Int      dataSize  = 0;
        Bool     valid     = false;
    };

    ParsedHeader ReadHeader(const std::vector<Byte>& _raw)
    {
        BinaryReader _reader(_raw);
        ParsedHeader _header;
        for (const Byte _expected : KA7_MAGIC)
            if (_reader.ReadU8() != _expected) return _header;

        _header.version   = _reader.ReadU16();
        _header.flags     = _reader.ReadU16();
        _header.timestamp = _reader.ReadI64();
        _header.checksum  = _reader.ReadU32();
        _header.dataSize  = _reader.ReadU32();
        _header.valid     = true;
        return _header;
    }

    /// Reads the fixed-size header at the start of a .ka7 stream; invalid when the stream is too short or not a .ka7.
    ParsedHeader ReadHeader(std::istream& _stream)
    {
        std::vector<Byte> _raw(KA7_HEADER_SIZE);
        _stream.read(reinterpret_cast<char*>(_raw.data()), static_cast<std::streamsize>(_raw.size()));
        return _stream.fail() ? ParsedHeader{} : ReadHeader(_raw);
    }

    constexpr Short FLAG_COMPRESSED = 1;

    /// Copies the header fields into the metadata; false when the header is invalid (fields left untouched).
    bool FillMetadata(SaveMetadata& _meta, const ParsedHeader& _header)
    {
        if (!_header.valid) return false;
        _meta.version      = _header.version;
        _meta.timestamp    = _header.timestamp;
        _meta.isCompressed = (_header.flags & FLAG_COMPRESSED) != 0;
        return true;
    }
}


std::filesystem::path SaveManager::GetSavePath(const String& name) 
{
    return std::filesystem::path(saveDirectory)
        / (name + SAVE_EXTENSION);
}

void SaveManager::EnsureSaveDirectoryExists() 
{
    std::error_code ec;
    std::filesystem::create_directories(saveDirectory, ec);
}

void SaveManager::SetSaveDirectory(String dir)
{
    const std::string& _raw = dir;
    if (!_raw.empty() && _raw.back() != '/' && _raw.back() != '\\')
        dir += "/";
    saveDirectory = std::move(dir);
}

void SaveManager::SetOnSaveCallback(EventCallback cb) { onSave = std::move(cb); }
void SaveManager::SetOnLoadCallback(EventCallback cb) { onLoad = std::move(cb); }

Bool SaveManager::Exists(const String& name)
{
    return std::filesystem::exists(GetSavePath(name));
}

Bool SaveManager::Delete(const String& name)
{
    std::error_code ec;
    return std::filesystem::remove(GetSavePath(name), ec);
}

Bool SaveManager::Rename(const String& oldName, const String& newName)
{
    std::error_code ec;
    std::filesystem::rename(GetSavePath(oldName), GetSavePath(newName), ec);
    return !ec;
}

Bool SaveManager::Copy(const String& src, const String& dst)
{
    EnsureSaveDirectoryExists();
    std::error_code ec;
    std::filesystem::copy_file(
        GetSavePath(src), GetSavePath(dst),
        std::filesystem::copy_options::overwrite_existing, ec);
    return !ec;
}

std::vector<SaveMetadata> SaveManager::ListSaves() 
{
    std::vector<SaveMetadata> result;
    std::error_code ec;
    if (!std::filesystem::exists(saveDirectory, ec)) return result;

    for (const auto& entry : std::filesystem::directory_iterator(saveDirectory, ec))
    {
        if (!entry.is_regular_file())                          continue;
        if (entry.path().extension() != SAVE_EXTENSION) continue;

        SaveMetadata meta;
        meta.name = String(entry.path().stem().string());
        meta.filePath = String(entry.path().string());

        std::ifstream f(entry.path(), std::ios::binary);
        FillMetadata(meta, ReadHeader(f));
        result.push_back(std::move(meta));
    }
    return result;
}

std::optional<SaveMetadata> SaveManager::GetMetadata(const String& name)
{
    const auto path = GetSavePath(name);
    std::ifstream f(path, std::ios::binary);
    SaveMetadata meta;
    meta.name = name;
    meta.filePath = String(path.string());
    if (!FillMetadata(meta, ReadHeader(f))) return std::nullopt;
    return meta;
}

SaveResult SaveManager::Save(const String& name, const SaveNode& root, const Bool& compress)
{
    EnsureSaveDirectoryExists();
    return SaveToFile(GetSavePath(name), root, compress);
}

SaveResult SaveManager::SaveToFile(const std::filesystem::path& path, const SaveNode& root, const Bool& compress)
{
    const String name = String(path.string());
    LOG(Krampus::VerbosityType::Verbose, "SaveManager: writing " + path.generic_string() + (compress ? " (compressed)" : ""));
    std::error_code dirError;
    std::filesystem::create_directories(path.parent_path(), dirError);
    try 
    {
        BinaryWriter dataWriter;
        dataWriter.buffer.reserve(4096);
        dataWriter.WriteNode(root);
        const auto& data = dataWriter.buffer;

        const Int      checksum  = ComputeCRC32(data);
        const Int      dataSize  = static_cast<Int>(data.size());
        const LongLong timestamp = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        const Short    flags     = compress ? FLAG_COMPRESSED : 0;

        BinaryWriter hdrWriter;
        WriteHeader(hdrWriter, CURRENT_VERSION, flags, timestamp, checksum, dataSize);

        // Written next to the target, then renamed over it: a crash or a full disk in the middle leaves the previous save intact.
        std::filesystem::path tempPath = path;
        tempPath += ".tmp";
        {
            std::ofstream file(tempPath, std::ios::binary | std::ios::trunc);
            if (!file)
            {
                if (onSave) onSave(name, SaveResult::ErrorIO);
                return SaveResult::ErrorIO;
            }

            for (const std::vector<Byte>* _bytes : { &std::as_const(hdrWriter.buffer), &data })
                file.write(reinterpret_cast<const char*>(_bytes->data()), static_cast<std::streamsize>(_bytes->size()));
            file.close();

            if (file.fail())
            {
                std::error_code removeError;
                std::filesystem::remove(tempPath, removeError);
                if (onSave) onSave(name, SaveResult::ErrorIO);
                return SaveResult::ErrorIO;
            }
        }

        std::error_code renameError;
        std::filesystem::rename(tempPath, path, renameError);
        if (renameError)
        {
            std::filesystem::remove(tempPath, renameError);
            if (onSave) onSave(name, SaveResult::ErrorIO);
            return SaveResult::ErrorIO;
        }

        if (onSave) onSave(name, SaveResult::Success);
        return SaveResult::Success;
    }
    catch (...) 
    {
        LOG_WARNING("SaveManager: saving \"" + name + "\" failed (cannot write the file)");
        if (onSave) onSave(name, SaveResult::ErrorIO);
        return SaveResult::ErrorIO;
    }
}

SaveResult SaveManager::Load(const String& name, SaveNode& outRoot)
{
    return LoadFromFile(GetSavePath(name), outRoot);
}

SaveResult SaveManager::LoadFromFile(const std::filesystem::path& path, SaveNode& outRoot)
{
    const String name = String(path.string());
    LOG(Krampus::VerbosityType::Verbose, "SaveManager: reading " + path.generic_string());

    if (!Krampus::AssetPack::Exists(path)) 
    {
        if (onLoad) onLoad(name, SaveResult::ErrorNotFound);
        return SaveResult::ErrorNotFound;
    }

    try 
    {
        // A scene of a packed game is in its exe, a save is a file: the pack is asked first.
        std::unique_ptr<std::istream> fileStream = Krampus::AssetPack::OpenStream(path);
        if (!fileStream)
        {
            if (onLoad) onLoad(name, SaveResult::ErrorIO);
            return SaveResult::ErrorIO;
        }
        std::istream& file = *fileStream;

        const ParsedHeader h = ReadHeader(file);
        if (!h.valid)
        {
            if (onLoad) onLoad(name, SaveResult::ErrorCorrupted);
            return SaveResult::ErrorCorrupted;
        }

        if (h.version > CURRENT_VERSION)
        {
            if (onLoad) onLoad(name, SaveResult::ErrorVersionMismatch);
            return SaveResult::ErrorVersionMismatch;
        }

        if (h.dataSize < 0)
        {
            if (onLoad) onLoad(name, SaveResult::ErrorCorrupted);
            return SaveResult::ErrorCorrupted;
        }
        std::vector<Byte> data(static_cast<std::size_t>(h.dataSize));
        file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()));
        if (file.fail())
        {
            if (onLoad) onLoad(name, SaveResult::ErrorCorrupted);
            return SaveResult::ErrorCorrupted;
        }

        if (ComputeCRC32(data) != h.checksum) 
        {
            if (onLoad) onLoad(name, SaveResult::ErrorChecksum);
            return SaveResult::ErrorChecksum;
        }

        BinaryReader dataReader(data);
        outRoot = dataReader.ReadNode();

        if (onLoad) onLoad(name, SaveResult::Success);
        return SaveResult::Success;
    }
    catch (...) 
    {
        LOG_WARNING("SaveManager: loading \"" + name + "\" failed (file missing or corrupted)");
        if (onLoad) onLoad(name, SaveResult::ErrorCorrupted);
        return SaveResult::ErrorCorrupted;
    }
}

SaveNode SaveManager::LoadOrThrow(const String& name) 
{
    SaveNode root;
    const auto result = Load(name, root);
    if (result != SaveResult::Success)
        THROW_EXCEPTION(
            String("Ka7::SaveManager::LoadOrThrow - '")
             + name + "': " + ToString(result));
    return root;
}

std::optional<SaveNode> SaveManager::TryLoad(const String& name) 
{
    SaveNode root;
    if (Load(name, root) == SaveResult::Success) return root;
    return std::nullopt;
}


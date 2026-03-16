#include "SaveManager.h"
#include <Utilities/Debug/Exception.h>


String                      SaveManager::saveDirectory = SaveManager::SAVE_DIRECTORY;
SaveManager::EventCallback  SaveManager::onSave = nullptr;
SaveManager::EventCallback  SaveManager::onLoad = nullptr;

namespace 
{
    static const Byte KA7_MAGIC[4] = { Byte('K'), Byte('A'), Byte('7'), Byte(0) };
    static const Int  KA7_HDR_SIZE = Int(32);

    Int ComputeCRC32(const std::vector<Byte>& data) noexcept 
    {
        static const LongLong POLY = LongLong(0xEDB88320LL);
        static const LongLong MASK32 = LongLong(0xFFFFFFFFLL);

        LongLong crc = LongLong(0xFFFFFFFFLL);

        for (Int i = 0; i < Int(data.size()); ++i) 
        {
            crc = crc ^ data[i].ToLongLong();
            for (Int j = 0; j < 8; ++j) 
            {
                LongLong lowBit = crc & LongLong(1);
                LongLong mask = LongLong(0) - lowBit;
                crc = (crc >> 1) ^ (POLY & mask);
            }
        }

        return ((crc ^ LongLong(0xFFFFFFFFLL)) & MASK32).ToInt();
    }

    std::vector<unsigned char> ByteVecToRaw(const std::vector<Byte>& src) 
    {
        std::vector<unsigned char> out;
        out.reserve(src.size());
        for (const auto& b : src)
            out.push_back(static_cast<unsigned char>(b));  
        return out;
    }

    std::vector<Byte> RawToByteVec(const std::vector<unsigned char>& src)
    {
        std::vector<Byte> out;
        out.reserve(src.size());
        for (const auto c : src)
            out.push_back(Byte(c));  
        return out;
    }

    std::vector<Byte> RawToByteVec(const unsigned char* src, std::size_t n) 
    {
        std::vector<Byte> out;
        out.reserve(n);
        for (std::size_t i = 0; i < n; ++i)
            out.push_back(Byte(src[i]));
        return out;
    }

    class BinaryWriter 
    {
    public:
        std::vector<Byte> buffer;

        void WriteU8(const Byte& v) 
        {
            buffer.push_back(v);
        }

        void WriteU16(const Short& v) 
        {
            buffer.push_back((v & Short(0xFF)).ToByte());
            buffer.push_back(((v >> 8) & Short(0xFF)).ToByte());
        }

        void WriteU32(const Int& v) 
        {
            for (Int i = 0; i < 4; ++i)
                buffer.push_back((v >> (i * 8)).ToByte());
        }

        void WriteI64(const LongLong& v) 
        {
            for (Int i = 0; i < 8; ++i)
                buffer.push_back((v >> (i * 8)).ToByte());
        }

        void WriteDouble(const Double& v) 
        {
            LongLong bits = std::bit_cast<long long>(static_cast<double>(v));
            WriteI64(bits);
        }

        void WriteString(const String& s)
        {
            const auto& raw = s.StdString();
            WriteU32(Int(raw.size()));
            for (const auto c : raw)
                buffer.push_back(Byte(c));
        }

        void WriteBytes(const std::vector<Byte>& v) 
        {
            WriteU32(Int(v.size()));
            buffer.insert(buffer.end(), v.begin(), v.end());
        }

        void WriteNode(const SaveNode& node) 
        {
            WriteString(node.GetName());
            const auto& fields = node.GetFields();
            WriteU32(Int(fields.size()));
            for (const auto& [key, val] : fields) 
            {
                WriteString(key);
                WriteValue(val);
            }
        }

        void WriteValue(const SaveValue& val)
        {
            std::visit([this](const auto& v) { WriteVariant(v); }, val);
        }

    private:
        void WriteVariant(const Bool& v) 
        {
            WriteU8(Byte((unsigned char)SaveType::Bool));
            WriteU8(v ? Byte(1) : Byte(0));
        }
        void WriteVariant(const LongLong& v) 
        {
            WriteU8(Byte((unsigned char)SaveType::LongLong));
            WriteI64(v);
        }
        void WriteVariant(const Double& v) 
        {
            WriteU8(Byte((unsigned char)SaveType::Double));
            WriteDouble(v);
        }
        void WriteVariant(const String& v) 
        {
            WriteU8(Byte((unsigned char)SaveType::String));
            WriteString(v);
        }
        void WriteVariant(const std::vector<Byte>& v) 
        {
            WriteU8(Byte((unsigned char)SaveType::Bytes));
            WriteBytes(v);
        }
        void WriteVariant(const std::vector<LongLong>& v) 
        {
            WriteU8(Byte((unsigned char)SaveType::ArrayLongLong));
            WriteU32(Int(v.size()));
            for (const auto& x : v) WriteI64(x);
        }
        void WriteVariant(const std::vector<Double>& v) 
        {
            WriteU8(Byte((unsigned char)SaveType::ArrayDouble));
            WriteU32(Int(v.size()));
            for (const auto& x : v) WriteDouble(x);
        }
        void WriteVariant(const std::vector<String>& v) 
        {
            WriteU8(Byte((unsigned char)SaveType::ArrayString));
            WriteU32(Int(v.size()));
            for (const auto& s : v) WriteString(s);
        }
        void WriteVariant(const SaveNodePtr& v) 
        {
            WriteU8(Byte((unsigned char)SaveType::Node));
            WriteNode(v ? *v : SaveNode{});
        }
        void WriteVariant(const std::vector<SaveNodePtr>& v) 
        {
            WriteU8(Byte((unsigned char)SaveType::ArrayNode));
            WriteU32(Int(v.size()));
            for (const auto& n : v) WriteNode(n ? *n : SaveNode{});
        }
    };

    class BinaryReader 
    {
    public:
        explicit BinaryReader(const std::vector<Byte>& buf)
            : buf(buf), pos(0) 
        {
        }

        [[nodiscard]] Bool HasRemaining(const Int& n) const 
        {
            return (pos + n) <= Int(buf.size());
        }

        Byte ReadU8() 
        {
            CheckBounds(Int(1));
            Byte b = buf[pos];
            ++pos;
            return b;
        }

        Short ReadU16() 
        {
            CheckBounds(Int(2));
            Short lo = buf[pos].ToShort();
            Short hi = buf[pos + 1].ToShort();
            pos += Int(2);
            return lo | (hi << 8);
        }

        Int ReadU32() 
        {
            CheckBounds(Int(4));
            Int result = Int(0);
            for (Int i = 0; i < 4; ++i)
                result |= buf[pos + i].ToInt() << (i * 8);
            pos += Int(4);
            return result;
        }

        LongLong ReadI64() 
        {
            CheckBounds(Int(8));
            LongLong result = LongLong(0);
            for (Int i = 0; i < 8; ++i)
                result |= buf[pos + i].ToLongLong() << (i * 8);
            pos += Int(8);
            return result;
        }

        Double ReadDouble() 
        {
            LongLong bits = ReadI64();
            double raw = std::bit_cast<double>(static_cast<long long>(bits));
            return Double(raw);
        }

        String ReadString() 
        {
            Int len = ReadU32();
            CheckBounds(len);
            std::string raw;
            for (Int i = 0; i < len; ++i)
                raw += static_cast<char>(buf[pos + i]);
            pos += len;
            return String(raw);
        }

        std::vector<Byte> ReadBytes() 
        {
            Int len = ReadU32();
            CheckBounds(len);
            std::vector<Byte> v;
            const std::size_t count = static_cast<std::size_t>(static_cast<int>(len));
            v.reserve(count);
            for (std::size_t i = 0; i < count; ++i)
                v.push_back(buf[pos + Int(static_cast<int>(i))]);
            pos += len;
            return v;
        }

        SaveNode ReadNode() 
        {
            SaveNode node(ReadString());
            Int fieldCount = ReadU32();
            node.GetFields().reserve(static_cast<std::size_t>(static_cast<int>(fieldCount)));
            for (Int i = 0; i < fieldCount; ++i) {
                String key = ReadString();
                node.GetFields()[std::move(key)] = ReadValue();
            }
            return node;
        }

        SaveValue ReadValue() 
        {
            Byte typeByte = ReadU8();
            Int  typeId = typeByte.ToInt();

            if (typeId == Int(0)) return Bool(ReadU8() != Byte(0));
            else if (typeId == Int(1)) return ReadI64();
            else if (typeId == Int(2)) return ReadDouble();
            else if (typeId == Int(3)) return ReadString();
            else if (typeId == Int(4)) return ReadBytes();
            else if (typeId == Int(5)) 
            {
                Int count = ReadU32();
                std::vector<LongLong> v;
                v.reserve(static_cast<std::size_t>(static_cast<int>(count)));
                for (Int i = 0; i < count; ++i) v.push_back(ReadI64());
                return v;
            }
            else if (typeId == Int(6)) 
            {
                Int count = ReadU32();
                std::vector<Double> v;
                v.reserve(static_cast<std::size_t>(static_cast<int>(count)));
                for (Int i = 0; i < count; ++i) v.push_back(ReadDouble());
                return v;
            }
            else if (typeId == Int(7)) 
            {
                Int count = ReadU32();
                std::vector<String> v;
                v.reserve(static_cast<std::size_t>(static_cast<int>(count)));
                for (Int i = 0; i < count; ++i) v.push_back(ReadString());
                return v;
            }
            else if (typeId == Int(8))
            {
                return std::make_shared<SaveNode>(ReadNode());
            }
            else if (typeId == Int(9)) 
            {
                Int count = ReadU32();
                std::vector<SaveNodePtr> v;
                v.reserve(static_cast<std::size_t>(static_cast<int>(count)));
                for (Int i = 0; i < count; ++i)
                    v.push_back(std::make_shared<SaveNode>(ReadNode()));
                return v;
            }
            THROW_EXCEPTION("Ka7::SaveManager: type inconnu dans le fichier .ka7");
        }

    private:
        const std::vector<Byte>& buf;
        Int pos;

        void CheckBounds(const Int& n) const 
        {
            if ((pos + n) > Int(buf.size()))
                THROW_EXCEPTION("Ka7::SaveManager: données tronquées");
        }
    };


    void WriteHeader(BinaryWriter& w,
        const Short& version,
        const Short& flags,
        const LongLong& timestamp,
        const Int& checksum,
        const Int& dataSize)
    {
        for (const auto& b : KA7_MAGIC) w.WriteU8(b);
        w.WriteU16(version);
        w.WriteU16(flags);
        w.WriteI64(timestamp);
        w.WriteU32(checksum);
        w.WriteU32(dataSize);
        w.WriteU32(Int(0));
        w.WriteU32(Int(0));
    }

    struct ParsedHeader 
    {
        Short    version;
        Short    flags;
        LongLong timestamp;
        Int      checksum;
        Int      dataSize;
        Bool     valid      = Bool::False();
    };

    ParsedHeader ReadHeader(BinaryReader& r)
    {
        ParsedHeader h;

        for (Int i = 0; i < 4; ++i) 
        {
            Byte b = r.ReadU8();
            if (b != KA7_MAGIC[i]) return h;
        }

        h.version = r.ReadU16();
        h.flags = r.ReadU16();
        h.timestamp = r.ReadI64();
        h.checksum = r.ReadU32();
        h.dataSize = r.ReadU32();
        r.ReadU32();
        r.ReadU32();
        h.valid = Bool::True();
        return h;
    }

}


std::filesystem::path SaveManager::GetSavePath(const String& name) 
{
    return std::filesystem::path(saveDirectory.StdString())
        / (name.StdString() + SAVE_EXTENSION.StdString());
}

void SaveManager::EnsureSaveDirectoryExists() 
{
    std::error_code ec;
    std::filesystem::create_directories(saveDirectory.StdString(), ec);
}

void SaveManager::SetSaveDirectory(String dir)
{
    if (dir.IsEmpty().Not() && dir.At(dir.Length() - Int(1)) != '/' &&
        dir.At(dir.Length() - Int(1)) != '\\')
        dir += String("/");
    saveDirectory = std::move(dir);
}

void SaveManager::SetOnSaveCallback(EventCallback cb) { onSave = std::move(cb); }
void SaveManager::SetOnLoadCallback(EventCallback cb) { onLoad = std::move(cb); }

Bool SaveManager::Exists(const String& name) 
{
    return Bool(std::filesystem::exists(GetSavePath(name)));
}

Bool SaveManager::Delete(const String& name) 
{
    std::error_code ec;
    return Bool(std::filesystem::remove(GetSavePath(name), ec));
}

Bool SaveManager::Rename(const String& oldName, const String& newName) 
{
    std::error_code ec;
    std::filesystem::rename(GetSavePath(oldName), GetSavePath(newName), ec);
    return Bool(!ec);
}

Bool SaveManager::Copy(const String& src, const String& dst)
{
    EnsureSaveDirectoryExists();
    std::error_code ec;
    std::filesystem::copy_file(
        GetSavePath(src), GetSavePath(dst),
        std::filesystem::copy_options::overwrite_existing, ec);
    return Bool(!ec);
}

std::vector<SaveMetadata> SaveManager::ListSaves() 
{
    std::vector<SaveMetadata> result;
    std::error_code ec;
    if (!std::filesystem::exists(saveDirectory.StdString(), ec)) return result;

    for (const auto& entry : std::filesystem::directory_iterator(saveDirectory.StdString(), ec))
    {
        if (!entry.is_regular_file())                          continue;
        if (entry.path().extension() != SAVE_EXTENSION.StdString()) continue;

        SaveMetadata meta;
        meta.name = String(entry.path().stem().string());
        meta.filePath = String(entry.path().string());

        std::ifstream f(entry.path(), std::ios::binary);
        if (f)
        {
            unsigned char rawHdrBuf[32] = {};
            f.read(reinterpret_cast<char*>(rawHdrBuf), 32);
            if (!f.fail())
            {
                std::vector<Byte> hdrBuf = RawToByteVec(rawHdrBuf, 32);
                BinaryReader r(hdrBuf);
                auto h = ReadHeader(r);
                if (h.valid) 
                {
                    meta.version = h.version;
                    meta.timestamp = h.timestamp;
                    meta.isCompressed = Bool((h.flags & Short(1)) != Short(0));
                }
            }
        }
        result.push_back(std::move(meta));
    }
    return result;
}

std::optional<SaveMetadata> SaveManager::GetMetadata(const String& name) 
{
    auto path = GetSavePath(name);
    if (!std::filesystem::exists(path)) return std::nullopt;

    std::ifstream f(path, std::ios::binary);
    if (!f) return std::nullopt;

    unsigned char rawHdrBuf[32] = {};
    f.read(reinterpret_cast<char*>(rawHdrBuf), 32);
    if (f.fail()) return std::nullopt;
    std::vector<Byte> hdrBuf = RawToByteVec(rawHdrBuf, 32);

    BinaryReader r(hdrBuf);
    auto h = ReadHeader(r);
    if (!h.valid) return std::nullopt;

    SaveMetadata meta;
    meta.name = name;
    meta.filePath = String(path.string());
    meta.version = h.version;
    meta.timestamp = h.timestamp;
    meta.isCompressed = Bool((h.flags & Short(1)) != Short(0));
    return meta;
}

SaveResult SaveManager::Save(const String& name,
    const SaveNode& root,
    const Bool& compress)
{
    EnsureSaveDirectoryExists();
    try 
    {
        BinaryWriter dataWriter;
        dataWriter.buffer.reserve(4096);
        dataWriter.WriteNode(root);
        const auto& data = dataWriter.buffer;

        Int      checksum = ComputeCRC32(data);
        Int      dataSize = Int(data.size());
        LongLong timestamp = LongLong(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());
        Short flags = compress ? Short(1) : Short(0);

        BinaryWriter hdrWriter;
        WriteHeader(hdrWriter, CURRENT_VERSION, flags, timestamp, checksum, dataSize);

        std::ofstream file(GetSavePath(name), std::ios::binary | std::ios::trunc);
        if (!file) 
        {
            if (onSave) onSave(name, SaveResult::ErrorIO);
            return SaveResult::ErrorIO;
        }

        const auto rawHdr = ByteVecToRaw(hdrWriter.buffer);
        const auto rawData = ByteVecToRaw(data);
        file.write(reinterpret_cast<const char*>(rawHdr.data()),
            static_cast<std::streamsize>(rawHdr.size()));
        file.write(reinterpret_cast<const char*>(rawData.data()),
            static_cast<std::streamsize>(rawData.size()));

        if (file.fail()) 
        {
            if (onSave) onSave(name, SaveResult::ErrorIO);
            return SaveResult::ErrorIO;
        }

        if (onSave) onSave(name, SaveResult::Success);
        return SaveResult::Success;
    }
    catch (...) 
    {
        if (onSave) onSave(name, SaveResult::ErrorIO);
        return SaveResult::ErrorIO;
    }
}

SaveResult SaveManager::Load(const String& name, SaveNode& outRoot) 
{
    auto path = GetSavePath(name);

    if (!std::filesystem::exists(path)) 
    {
        if (onLoad) onLoad(name, SaveResult::ErrorNotFound);
        return SaveResult::ErrorNotFound;
    }

    try 
    {
        std::ifstream file(path, std::ios::binary);
        if (!file) 
        {
            if (onLoad) onLoad(name, SaveResult::ErrorIO);
            return SaveResult::ErrorIO;
        }

        unsigned char rawHdrBuf[32] = {};
        file.read(reinterpret_cast<char*>(rawHdrBuf), 32);
        if (file.fail()) 
        {
            if (onLoad) onLoad(name, SaveResult::ErrorCorrupted);
            return SaveResult::ErrorCorrupted;
        }
        std::vector<Byte> hdrBuf = RawToByteVec(rawHdrBuf, 32);

        BinaryReader hdrReader(hdrBuf);
        auto h = ReadHeader(hdrReader);

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

        const std::size_t dataLen = static_cast<std::size_t>(static_cast<int>(h.dataSize));
        std::vector<unsigned char> rawData(dataLen);
        file.read(reinterpret_cast<char*>(rawData.data()),
            static_cast<std::streamsize>(dataLen));
        if (file.fail()) 
        {
            if (onLoad) onLoad(name, SaveResult::ErrorCorrupted);
            return SaveResult::ErrorCorrupted;
        }
        std::vector<Byte> data = RawToByteVec(rawData);

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
            String("Ka7::SaveManager::LoadOrThrow — '")
            .StdString() + name.StdString() + "': " + ToString(result));
    return root;
}

std::optional<SaveNode> SaveManager::TryLoad(const String& name) 
{
    SaveNode root;
    if (Load(name, root) == SaveResult::Success) return root;
    return std::nullopt;
}
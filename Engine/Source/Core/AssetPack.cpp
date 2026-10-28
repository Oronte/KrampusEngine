#include "AssetPack.h"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>
#include <unordered_map>

using namespace Krampus;

namespace
{
	constexpr char		MAGIC[8]	= { 'K', 'P', 'A', 'C', 'K', '0', '0', '1' };
	constexpr size_t	FOOTER_SIZE	= 8 + 8 + 8;	// magic, where the pack starts, its size

	struct Item
	{
		std::uint64_t	offset	= 0;		// from the start of the data region
		std::uint64_t	size	= 0;
	};

	std::vector<std::uint8_t>					g_data;		// the pack, kept for the whole run
	std::size_t									g_dataStart = 0;	// where the files begin in g_data
	std::unordered_map<std::string, Item>		g_items;

	void WriteU64(std::vector<std::uint8_t>& _out, const std::uint64_t& _value)
	{
		for (int _i = 0; _i < 8; ++_i) _out.push_back(static_cast<std::uint8_t>((_value >> (8 * _i)) & 0xFF));
	}

	void WriteU32(std::vector<std::uint8_t>& _out, const std::uint32_t& _value)
	{
		for (int _i = 0; _i < 4; ++_i) _out.push_back(static_cast<std::uint8_t>((_value >> (8 * _i)) & 0xFF));
	}

	std::uint64_t ReadU64(const std::uint8_t* _p)
	{
		std::uint64_t _value = 0;
		for (int _i = 0; _i < 8; ++_i) _value |= static_cast<std::uint64_t>(_p[_i]) << (8 * _i);
		return _value;
	}

	std::uint32_t ReadU32(const std::uint8_t* _p)
	{
		std::uint32_t _value = 0;
		for (int _i = 0; _i < 4; ++_i) _value |= static_cast<std::uint32_t>(_p[_i]) << (8 * _i);
		return _value;
	}

	bool IsExcluded(const std::filesystem::path& _relative)
	{
		static const char* FOLDERS[] = { "Editor", "Logs", "Scripts", "Screenshots", "Saves" };
		for (const std::filesystem::path& _part : _relative)
			for (const char* _folder : FOLDERS)
				if (_part == _folder) return true;

		const std::string _name = _relative.filename().string();
		return _name == "TODO.txt" || _relative.extension() == ".pdb";
	}
}

std::string AssetPack::Normalize(const std::filesystem::path& _path)
{
	std::string _text = _path.generic_string();
	std::replace(_text.begin(), _text.end(), '\\', '/');
	while (_text.rfind("./", 0) == 0) _text.erase(0, 2);
	std::transform(_text.begin(), _text.end(), _text.begin(), [](unsigned char _c) { return static_cast<char>(std::tolower(_c)); });
	return _text;
}

bool AssetPack::Mount(const std::filesystem::path& _exe)
{
	std::ifstream _file(_exe, std::ios::binary);
	if (!_file) return false;

	_file.seekg(0, std::ios::end);
	const std::uint64_t _fileSize = static_cast<std::uint64_t>(_file.tellg());
	if (_fileSize < FOOTER_SIZE) return false;

	std::uint8_t _footer[FOOTER_SIZE];
	_file.seekg(static_cast<std::streamoff>(_fileSize - FOOTER_SIZE));
	_file.read(reinterpret_cast<char*>(_footer), FOOTER_SIZE);
	if (!_file || std::memcmp(_footer, MAGIC, 8) != 0) return false;

	const std::uint64_t _start = ReadU64(_footer + 8);
	const std::uint64_t _size = ReadU64(_footer + 16);
	if (_start + _size + FOOTER_SIZE != _fileSize) return false;

	g_data.assign(static_cast<size_t>(_size), 0);
	_file.seekg(static_cast<std::streamoff>(_start));
	_file.read(reinterpret_cast<char*>(g_data.data()), static_cast<std::streamsize>(_size));
	if (!_file) { g_data.clear(); return false; }

	// Index: count, then for each file: path length (2), path, offset (8), size (8). The data follows the index.
	const std::uint8_t* _p = g_data.data();
	const std::uint8_t* _end = _p + g_data.size();
	if (_end - _p < 4) { g_data.clear(); return false; }
	const std::uint32_t _count = ReadU32(_p);
	_p += 4;

	g_items.clear();
	for (std::uint32_t _i = 0; _i < _count; ++_i)
	{
		if (_end - _p < 2) { g_items.clear(); g_data.clear(); return false; }
		const std::size_t _length = static_cast<std::size_t>(_p[0]) | (static_cast<std::size_t>(_p[1]) << 8);
		_p += 2;
		if (static_cast<std::size_t>(_end - _p) < _length + 16) { g_items.clear(); g_data.clear(); return false; }

		const std::string _path(reinterpret_cast<const char*>(_p), _length);
		_p += _length;
		g_items[_path] = { ReadU64(_p), ReadU64(_p + 8) };
		_p += 16;
	}
	g_dataStart = static_cast<std::size_t>(_p - g_data.data());
	return true;
}

bool AssetPack::IsMounted()
{
	return !g_items.empty();
}

bool AssetPack::Find(const std::string& _path, const std::uint8_t*& _data, std::size_t& _size)
{
	if (g_items.empty()) return false;

	const auto _it = g_items.find(Normalize(_path));
	if (_it == g_items.end()) return false;

	_data = g_data.data() + g_dataStart + static_cast<std::size_t>(_it->second.offset);
	_size = static_cast<std::size_t>(_it->second.size);
	return true;
}

bool AssetPack::Exists(const std::filesystem::path& _path)
{
	const std::uint8_t* _data = nullptr;
	std::size_t _size = 0;
	if (Find(_path.generic_string(), _data, _size)) return true;

	std::error_code _error;
	return std::filesystem::exists(_path, _error);
}

bool AssetPack::ReadAll(const std::filesystem::path& _path, std::vector<std::uint8_t>& _out)
{
	const std::uint8_t* _data = nullptr;
	std::size_t _size = 0;
	if (Find(_path.generic_string(), _data, _size))
	{
		_out.assign(_data, _data + _size);
		return true;
	}

	std::ifstream _file(_path, std::ios::binary);
	if (!_file) return false;
	_out.assign(std::istreambuf_iterator<char>(_file), std::istreambuf_iterator<char>());
	return true;
}

std::unique_ptr<std::istream> AssetPack::OpenStream(const std::filesystem::path& _path)
{
	const std::uint8_t* _data = nullptr;
	std::size_t _size = 0;
	if (Find(_path.generic_string(), _data, _size))
		return std::make_unique<std::istringstream>(std::string(reinterpret_cast<const char*>(_data), _size), std::ios::binary);

	auto _file = std::make_unique<std::ifstream>(_path, std::ios::binary);
	if (!*_file) return nullptr;
	return _file;
}

bool AssetPack::BuildPak(const std::filesystem::path& _projectRoot, const std::filesystem::path& _outPak, std::string& _report)
{
	namespace fs = std::filesystem;

	// What goes in: every file of Content/ a game can use.
	const fs::path _content = _projectRoot / "Content";
	struct File { std::string path; fs::path source; std::uint64_t size; };
	std::vector<File> _files;

	std::error_code _error;
	for (const fs::directory_entry& _entry : fs::recursive_directory_iterator(_content, _error))
	{
		if (!_entry.is_regular_file()) continue;
		const fs::path _relative = fs::relative(_entry.path(), _projectRoot, _error);
		if (IsExcluded(fs::relative(_entry.path(), _content, _error))) continue;
		_files.push_back({ Normalize(_relative), _entry.path(), static_cast<std::uint64_t>(_entry.file_size(_error)) });
	}
	if (_files.empty())
	{
		_report = "There is nothing to pack in " + _content.string();
		return false;
	}
	std::sort(_files.begin(), _files.end(), [](const File& _a, const File& _b) { return _a.path < _b.path; });

	// The pack: index, then data.
	std::vector<std::uint8_t> _pack;
	WriteU32(_pack, static_cast<std::uint32_t>(_files.size()));
	std::uint64_t _offset = 0;
	for (const File& _file : _files)
	{
		_pack.push_back(static_cast<std::uint8_t>(_file.path.size() & 0xFF));
		_pack.push_back(static_cast<std::uint8_t>((_file.path.size() >> 8) & 0xFF));
		_pack.insert(_pack.end(), _file.path.begin(), _file.path.end());
		WriteU64(_pack, _offset);
		WriteU64(_pack, _file.size);
		_offset += _file.size;
	}

	std::uint64_t _total = 0;
	for (const File& _file : _files)
	{
		std::ifstream _in(_file.source, std::ios::binary);
		if (!_in)
		{
			_report = "Cannot read " + _file.source.string();
			return false;
		}
		const std::size_t _before = _pack.size();
		_pack.insert(_pack.end(), std::istreambuf_iterator<char>(_in), std::istreambuf_iterator<char>());
		if (_pack.size() - _before != _file.size)
		{
			_report = _file.source.string() + " changed while it was packed";
			return false;
		}
		_total += _file.size;
	}

	// pack | footer (the footer says where the pack starts: 0, nothing comes before it in a pak file)
	if (_outPak.has_parent_path()) fs::create_directories(_outPak.parent_path(), _error);
	std::ofstream _out(_outPak, std::ios::binary | std::ios::trunc);
	if (!_out)
	{
		_report = "Cannot write " + _outPak.string();
		return false;
	}
	_out.write(reinterpret_cast<const char*>(_pack.data()), static_cast<std::streamsize>(_pack.size()));

	std::vector<std::uint8_t> _footer(MAGIC, MAGIC + 8);
	WriteU64(_footer, 0);
	WriteU64(_footer, static_cast<std::uint64_t>(_pack.size()));
	_out.write(reinterpret_cast<const char*>(_footer.data()), static_cast<std::streamsize>(_footer.size()));
	if (!_out.good())
	{
		_report = "Cannot finish writing " + _outPak.string();
		return false;
	}

	std::ostringstream _text;
	_text << _files.size() << " files, " << (_total / 1024) << " KB of assets";
	_report = _text.str();
	return true;
}

// PackTool <project root> <output.pak>
//     Writes <output.pak>: the files of <project root>/Content in one file that the game mounts at start (see Engine/Source/Core/AssetPack.h).
// PackTool --icon <picture> <exe> [<exe>...]
//     Puts an icon in the exes (what the Explorer, the taskbar and the shortcuts show). The picture is a .ico (used as it is) or a
//     .png / .jpg / .bmp / .tga (made square, then stored at 256, 64, 48, 32, 24 and 16 pixels).
// Run by the Package target (Ship/CMakeLists.txt, CMake/Package.cmake).

#include "Core/AssetPack.h"
#include <SFML/Graphics/Image.hpp>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#pragma comment(lib, "shell32.lib")

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace
{
	/// <summary>One picture of an icon: its size (0 = 256) and its bytes (PNG, or what a .ico holds).</summary>
	struct IconImage
	{
		std::uint8_t				width		= 0;
		std::uint8_t				height		= 0;
		std::uint8_t				colors		= 0;
		std::uint16_t				planes		= 1;
		std::uint16_t				bitCount	= 32;
		std::vector<std::uint8_t>	bytes;
	};

#pragma pack(push, 2)
	struct GroupHeader { WORD reserved; WORD type; WORD count; };
	struct GroupEntry { BYTE width; BYTE height; BYTE colors; BYTE reserved; WORD planes; WORD bitCount; DWORD bytes; WORD id; };
#pragma pack(pop)

	std::vector<std::uint8_t> LoadBytes(const std::filesystem::path& _path)
	{
		std::ifstream _file(_path, std::ios::binary);
		return std::vector<std::uint8_t>((std::istreambuf_iterator<char>(_file)), std::istreambuf_iterator<char>());
	}

	template<typename T> T ReadAt(const std::vector<std::uint8_t>& _data, const size_t _offset)
	{
		T _value { };
		if (_offset + sizeof(T) <= _data.size()) std::memcpy(&_value, _data.data() + _offset, sizeof(T));
		return _value;
	}

	/// <summary>The pictures of a .ico file.</summary>
	bool ReadIco(const std::filesystem::path& _path, std::vector<IconImage>& _images, std::string& _error)
	{
		const std::vector<std::uint8_t> _data = LoadBytes(_path);
		if (ReadAt<std::uint16_t>(_data, 2) != 1) { _error = "not a .ico file"; return false; }
		const std::uint16_t _count = ReadAt<std::uint16_t>(_data, 4);
		for (std::uint16_t _i = 0; _i < _count; ++_i)
		{
			const size_t _entry = 6 + 16 * static_cast<size_t>(_i);
			IconImage _image;
			_image.width = ReadAt<std::uint8_t>(_data, _entry);
			_image.height = ReadAt<std::uint8_t>(_data, _entry + 1);
			_image.colors = ReadAt<std::uint8_t>(_data, _entry + 2);
			_image.planes = ReadAt<std::uint16_t>(_data, _entry + 4);
			_image.bitCount = ReadAt<std::uint16_t>(_data, _entry + 6);
			const std::uint32_t _size = ReadAt<std::uint32_t>(_data, _entry + 8);
			const std::uint32_t _offset = ReadAt<std::uint32_t>(_data, _entry + 12);
			if (_size == 0 || static_cast<size_t>(_offset) + _size > _data.size()) { _error = "damaged .ico file"; return false; }
			_image.bytes.assign(_data.begin() + _offset, _data.begin() + _offset + _size);
			_images.push_back(std::move(_image));
		}
		if (_images.empty()) _error = "empty .ico file";
		return !_images.empty();
	}

	/// <summary>_source (square) at _size x _size: the average of the pixels each new pixel covers (alpha weighted), or the nearest
	/// pixel when it grows.</summary>
	sf::Image Resize(const sf::Image& _source, const unsigned _size)
	{
		const unsigned _from = _source.getSize().x;
		sf::Image _result({ _size, _size }, sf::Color::Transparent);
		for (unsigned _y = 0; _y < _size; ++_y)
			for (unsigned _x = 0; _x < _size; ++_x)
			{
				const unsigned _x0 = _x * _from / _size, _y0 = _y * _from / _size;
				const unsigned _x1 = std::max(_x0 + 1, (_x + 1) * _from / _size), _y1 = std::max(_y0 + 1, (_y + 1) * _from / _size);
				double _r = 0.0, _g = 0.0, _b = 0.0, _a = 0.0;
				for (unsigned _sy = _y0; _sy < _y1; ++_sy)
					for (unsigned _sx = _x0; _sx < _x1; ++_sx)
					{
						const sf::Color _c = _source.getPixel({ _sx, _sy });
						_r += _c.r * static_cast<double>(_c.a);
						_g += _c.g * static_cast<double>(_c.a);
						_b += _c.b * static_cast<double>(_c.a);
						_a += _c.a;
					}
				const double _count = static_cast<double>((_x1 - _x0) * (_y1 - _y0));
				if (_a > 0.0)
					_result.setPixel({ _x, _y }, sf::Color(static_cast<std::uint8_t>(_r / _a + 0.5), static_cast<std::uint8_t>(_g / _a + 0.5),
						static_cast<std::uint8_t>(_b / _a + 0.5), static_cast<std::uint8_t>(_a / _count + 0.5)));
			}
		return _result;
	}

	/// <summary>An icon picture the old way (what every part of Windows reads, the taskbar too): a 32 bit bitmap upside down, then the
	/// 1 bit mask (1 = transparent), each line of the mask padded to 4 bytes.</summary>
	std::vector<std::uint8_t> IconBitmap(const sf::Image& _image)
	{
		const unsigned _size = _image.getSize().x;
		const unsigned _maskLine = ((_size + 31) / 32) * 4;
		BITMAPINFOHEADER _header { };
		_header.biSize = sizeof(BITMAPINFOHEADER);
		_header.biWidth = static_cast<LONG>(_size);
		_header.biHeight = static_cast<LONG>(_size * 2);	// the colors and the mask
		_header.biPlanes = 1;
		_header.biBitCount = 32;
		_header.biCompression = BI_RGB;
		_header.biSizeImage = _size * _size * 4 + _maskLine * _size;

		std::vector<std::uint8_t> _bytes(sizeof(_header) + _header.biSizeImage, 0);
		std::memcpy(_bytes.data(), &_header, sizeof(_header));
		std::uint8_t* _colors = _bytes.data() + sizeof(_header);
		std::uint8_t* _mask = _colors + _size * _size * 4;
		for (unsigned _y = 0; _y < _size; ++_y)
			for (unsigned _x = 0; _x < _size; ++_x)
			{
				const sf::Color _c = _image.getPixel({ _x, _size - 1 - _y });	// bottom line first
				std::uint8_t* _pixel = _colors + (_y * _size + _x) * 4;
				_pixel[0] = _c.b; _pixel[1] = _c.g; _pixel[2] = _c.r; _pixel[3] = _c.a;
				if (_c.a == 0) _mask[_y * _maskLine + _x / 8] |= static_cast<std::uint8_t>(0x80 >> (_x % 8));
			}
		return _bytes;
	}

	/// <summary>A picture (png, jpg...) made square (centered, transparent around) at the usual icon sizes: 256 as PNG, the others
	/// as bitmaps (Windows reads PNG safely at 256 only: the taskbar showed a blank icon with small PNG pictures).</summary>
	bool ReadPicture(const std::filesystem::path& _path, std::vector<IconImage>& _images, std::string& _error)
	{
		sf::Image _picture;
		if (!_picture.loadFromFile(_path)) { _error = "cannot read the picture"; return false; }
		const sf::Vector2u _size = _picture.getSize();
		const unsigned _side = std::max(_size.x, _size.y);
		sf::Image _square({ _side, _side }, sf::Color::Transparent);
		if (!_square.copy(_picture, { (_side - _size.x) / 2, (_side - _size.y) / 2 })) { _error = "cannot copy the picture"; return false; }

		for (const unsigned _iconSize : { 256u, 64u, 48u, 32u, 24u, 16u })
		{
			const sf::Image _resized = Resize(_square, _iconSize);
			IconImage _image;
			_image.width = _image.height = static_cast<std::uint8_t>(_iconSize >= 256 ? 0 : _iconSize);
			if (_iconSize >= 256)
			{
				const std::optional<std::vector<std::uint8_t>> _png = _resized.saveToMemory("png");
				if (!_png) { _error = "cannot encode the icon"; return false; }
				_image.bytes = *_png;
			}
			else
				_image.bytes = IconBitmap(_resized);
			_images.push_back(std::move(_image));
		}
		return true;
	}

	/// <summary>Writes the pictures in the exe as its icon (group 1, pictures 1..n). Types 3 = RT_ICON, 14 = RT_GROUP_ICON (wide).</summary>
	bool WriteIcon(const std::filesystem::path& _exe, const std::vector<IconImage>& _images, std::string& _error)
	{
		HANDLE _update = BeginUpdateResourceW(_exe.c_str(), FALSE);
		if (!_update) { _error = "cannot open " + _exe.string(); return false; }

		const WORD _language = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
		std::vector<std::uint8_t> _group(sizeof(GroupHeader) + sizeof(GroupEntry) * _images.size());
		const GroupHeader _header { 0, 1, static_cast<WORD>(_images.size()) };
		std::memcpy(_group.data(), &_header, sizeof(_header));
		bool _ok = true;
		for (size_t _i = 0; _i < _images.size() && _ok; ++_i)
		{
			const IconImage& _image = _images[_i];
			const WORD _id = static_cast<WORD>(_i + 1);
			const GroupEntry _entry { _image.width, _image.height, _image.colors, 0, _image.planes, _image.bitCount,
				static_cast<DWORD>(_image.bytes.size()), _id };
			std::memcpy(_group.data() + sizeof(GroupHeader) + sizeof(GroupEntry) * _i, &_entry, sizeof(_entry));
			_ok = UpdateResourceW(_update, MAKEINTRESOURCEW(3), MAKEINTRESOURCEW(_id), _language, const_cast<std::uint8_t*>(_image.bytes.data()),
				static_cast<DWORD>(_image.bytes.size())) != 0;
		}
		_ok = _ok && UpdateResourceW(_update, MAKEINTRESOURCEW(14), MAKEINTRESOURCEW(1), _language, _group.data(), static_cast<DWORD>(_group.size())) != 0;
		if (!EndUpdateResourceW(_update, _ok ? FALSE : TRUE) || !_ok)
		{
			_error = "cannot write the icon in " + _exe.string();
			return false;
		}
		// The Explorer (and the taskbar, which shows the icon of the exe) keeps icons in a cache: the exe was seen without its icon
		// just after the link, tell it to read it again.
		const std::wstring _full = std::filesystem::absolute(_exe).wstring();
		SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATHW | SHCNF_FLUSH, _full.c_str(), nullptr);
		return true;
	}

	int SetIcon(int _argc, char** _argv)
	{
		if (_argc < 4)
		{
			std::cerr << "usage: PackTool --icon <picture> <exe> [<exe>...]" << std::endl;
			return 2;
		}
		const std::filesystem::path _picture = _argv[2];
		std::string _extension = _picture.extension().string();
		std::transform(_extension.begin(), _extension.end(), _extension.begin(), [](unsigned char _c) { return static_cast<char>(std::tolower(_c)); });

		std::vector<IconImage> _images;
		std::string _error;
		const bool _read = _extension == ".ico" ? ReadIco(_picture, _images, _error) : ReadPicture(_picture, _images, _error);
		if (!_read)
		{
			std::cerr << "PackTool error: icon " << _picture.string() << ": " << _error << std::endl;
			return 1;
		}
		for (int _i = 3; _i < _argc; ++_i)
			if (!WriteIcon(_argv[_i], _images, _error))
			{
				std::cerr << "PackTool error: " << _error << std::endl;
				return 1;
			}
		std::cout << "Package: icon " << _picture.filename().string() << " (" << _images.size() << " sizes)" << std::endl;
		return 0;
	}
}

int main(int _argc, char** _argv)
{
	if (_argc >= 2 && std::string(_argv[1]) == "--icon") return SetIcon(_argc, _argv);

	if (_argc < 3)
	{
		std::cerr << "usage: PackTool <project root> <output.pak>   or   PackTool --icon <picture> <exe> [<exe>...]" << std::endl;
		return 2;
	}

	std::string _report;
	if (!Krampus::AssetPack::BuildPak(_argv[1], _argv[2], _report))
	{
		std::cerr << "PackTool error: " << _report << std::endl;
		return 1;
	}

	std::cout << "Package: " << _report << std::endl;
	return 0;
}

#include "Music.h"
#include "Core/AssetPack.h"

bool Krampus::Music::Open(const String& _path)
{
	const std::uint8_t* _data = nullptr;
	std::size_t _size = 0;
	if (AssetPack::Find(_path, _data, _size))	// a packed game has its files in its exe (the bytes stay in memory for the whole run)
		return source->openFromMemory(_data, _size);
	return source->openFromFile(_path);
}

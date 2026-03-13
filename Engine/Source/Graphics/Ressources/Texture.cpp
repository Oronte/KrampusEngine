#include "Texture.h"

Krampus::Texture::Texture(const String& _path, const Bool& _rgb)
{
	texture = std::make_unique<sf::Texture>(std::filesystem::path(_path.StdString()), _rgb);
}

Krampus::Texture::Texture(const String& _path, const Bool& _rgb, const IRect& _area)
{
	texture = std::make_unique<sf::Texture>(std::filesystem::path(_path.StdString()), _rgb, _area);
}

Krampus::Texture::Texture(const void* data, const ULongLong& _size, const Bool& _rgb)
{
	texture = std::make_unique<sf::Texture>(data, _size, _rgb);
}

Krampus::Texture::Texture(const void* data, const ULongLong& _size, const Bool& _rgb, const IRect& _area)
{
	texture = std::make_unique<sf::Texture>(data, _size, _rgb, _area);
}

Krampus::Texture::Texture(const Image& _image, const Bool& _rgb)
{
	texture = std::make_unique<sf::Texture>(_image, _rgb);
}

Krampus::Texture::Texture(const Image& _image, const Bool& _rgb, const IRect& _area)
{
	texture = std::make_unique<sf::Texture>(_image, _rgb, _area);
}

Krampus::Texture::Texture(const UVector2& _size, const Bool& _rgb)
{
	texture = std::make_unique<sf::Texture>(_size, _rgb);
}

Krampus::Texture::Texture(const sf::Texture& _texture)
{
	texture = std::make_unique<sf::Texture>(_texture);
}

Bool Krampus::Texture::Resize(const UVector2& _size, const Bool& _rgb)
{
	if (!texture)
	{
		LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
		return false;
	}
	return texture->resize(_size, _rgb);
}

Bool Krampus::Texture::LoadFromFile(const String& _filename, const Bool& _rgb, const IRect& _area)
{
	if (!texture)
	{
		LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
		return false;
	}
	return texture->loadFromFile(std::filesystem::path(_filename.StdString()), _rgb, _area);
}

Bool Krampus::Texture::LoadFromMemory(const void* _data, const std::size_t& _size, const Bool& _rgb, const IRect& _area)
{
	if (!texture)
	{
		LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
		return false;
	}
	return texture->loadFromMemory(_data, _size, _rgb, _area);
}

Bool Krampus::Texture::LoadFromImage(const Image& _image, const Bool& _rgb, const IRect& _area)
{
	if (!texture)
	{
		LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
		return false;
	}
	return texture->loadFromImage(_image, _rgb, _area);
}

void Krampus::Texture::Bind(const Texture* _texture, const Bool& _isNoralize)
{
	if (!texture || !_texture)
	{
		LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
		return;
	}
	texture->bind(_texture->Get(), _isNoralize ? sf::CoordinateType::Normalized : sf::CoordinateType::Pixels);
}

Krampus::Image Krampus::Texture::CopyToImage() const
{
	if (!texture)
	{
		LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
		return Krampus::Image();
	}
	return Image(texture->copyToImage());
}

Bool Krampus::Texture::GenerateMipmap()
{
	if (!texture)
	{
		LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
		return false;
	}
	return texture->generateMipmap();
}

#include "Texture.h"

Krampus::Texture::Texture()
{
	texture = std::make_unique<sf::Texture>();
}

Krampus::Texture::Texture(const std::string& _path, const bool& _rgb)
{
	texture = std::make_unique<sf::Texture>(std::filesystem::path(_path), _rgb);
}

Krampus::Texture::Texture(const std::string& _path, const bool& _rgb, const IRect& _area)
{
	texture = std::make_unique<sf::Texture>(std::filesystem::path(_path), _rgb, _area);
}

Krampus::Texture::Texture(const void* data, std::size_t _size, const bool& _rgb)
{
	texture = std::make_unique<sf::Texture>(data, _size, _rgb);
}

Krampus::Texture::Texture(const void* data, std::size_t _size, const bool& _rgb, const IRect& _area)
{
	texture = std::make_unique<sf::Texture>(data, _size, _rgb, _area);
}

Krampus::Texture::Texture(const Image& _image, const bool& _rgb)
{
	texture = std::make_unique<sf::Texture>(_image, _rgb);
}

Krampus::Texture::Texture(const Image& _image, const bool& _rgb, const IRect& _area)
{
	texture = std::make_unique<sf::Texture>(_image, _rgb, _area);
}

Krampus::Texture::Texture(const UVector2& _size, const bool& _rgb)
{
	texture = std::make_unique<sf::Texture>(_size, _rgb);
}

Krampus::Texture::Texture(const sf::Texture& _texture)
{
	texture = std::make_unique<sf::Texture>(_texture);
}

bool Krampus::Texture::Resize(const UVector2& _size, const bool& _rgb)
{
	if (!texture)
	{
		LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
		return false;
	}
	return texture->resize(_size, _rgb);
}

bool Krampus::Texture::LoadFromFile(const std::string& _filename, const bool& _rgb, const IRect& _area)
{
	if (!texture)
	{
		LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
		return false;
	}
	return texture->loadFromFile(std::filesystem::path(_filename), _rgb, _area);
}

bool Krampus::Texture::LoadFromMemory(const void* _data, const std::size_t& _size, const bool& _rgb, const IRect& _area)
{
	if (!texture)
	{
		LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
		return false;
	}
	return texture->loadFromMemory(_data, _size, _rgb, _area);
}

bool Krampus::Texture::LoadFromImage(const Image& _image, const bool& _rgb, const IRect& _area)
{
	if (!texture)
	{
		LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
		return false;
	}
	return texture->loadFromImage(_image, _rgb, _area);
}

void Krampus::Texture::Bind(const Texture* _texture, const bool& _isNoralize)
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

bool Krampus::Texture::GenerateMipmap()
{
	if (!texture)
	{
		LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
		return false;
	}
	return texture->generateMipmap();
}

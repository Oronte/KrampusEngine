#include "EditorThumbnails.h"

using namespace Krampus;

namespace
{
	std::unordered_map<std::string, std::unique_ptr<sf::Texture>>& Cache()
	{
		static std::unordered_map<std::string, std::unique_ptr<sf::Texture>> _cache;	// null = could not be loaded
		return _cache;
	}
}

sf::Texture* Krampus::ThumbnailOf(const std::filesystem::path& _path)
{
	std::unordered_map<std::string, std::unique_ptr<sf::Texture>>& _cache = Cache();
	const std::string _key = _path.string();

	const auto _found = _cache.find(_key);
	if (_found != _cache.end()) return _found->second.get();

	if (_cache.size() > 400) _cache.clear();

	std::error_code _error;
	std::unique_ptr<sf::Texture> _texture;
	if (std::filesystem::file_size(_path, _error) < 16u * 1024u * 1024u && !_error)
	{
		_texture = std::make_unique<sf::Texture>();
		if (!_texture->loadFromFile(_path))
			_texture.reset();
		else
			_texture->setSmooth(true);
	}
	return (_cache[_key] = std::move(_texture)).get();
}

ImTextureID Krampus::TextureId(const sf::Texture& _texture)
{
	return reinterpret_cast<ImTextureID>(static_cast<std::intptr_t>(_texture.getNativeHandle()));
}

void Krampus::ClearThumbnails()
{
	Cache().clear();
}

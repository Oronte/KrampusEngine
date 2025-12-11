#include "TextureManager.h"

using namespace Krampus;

Krampus::TextureManager::TextureManager()
{
	std::filesystem::create_directories(texturePath);
	LoadDefaultTexture();
}

void Krampus::TextureManager::LoadDefaultTexture()
{
	const std::string _finalPath = defaultTexturePath + "." + defaultTextureExtension;
	std::shared_ptr<sf::Texture> _shared = LoadOrGetSharedTexture(_finalPath);
	if (_shared) defaultTexture.SetSharedTexture(_shared);
	else LOG(VerbosityType::Error, "Cannot load default texture : " + _finalPath);
}

void Krampus::TextureManager::SetTextureToDefault(Texture& _texture)
{
	_texture = defaultTexture;
}

std::shared_ptr<sf::Texture> Krampus::TextureManager::LoadOrGetSharedTexture(const std::string& _path, const IRect& _rect)
{
	const std::string _finalPath = texturePath + _path;

	auto _it = cache.find(_finalPath);
	if (_it != cache.end()) return _it->second;


	std::shared_ptr<sf::Texture> _texture = std::make_shared<sf::Texture>();
	bool _loaded = false;

	if (_rect == IRect()) _loaded = _texture->loadFromFile(_finalPath);
	else _loaded = _texture->loadFromFile(_finalPath, false, _rect);

	if (!_loaded)
	{
		LOG(VerbosityType::Error, "Cannot open file with the following path : '" + _finalPath + "' !");
		return nullptr;
	}

	cache.emplace(_finalPath, _texture);
	return _texture;
}

void Krampus::TextureManager::SetTexture(Shape* _shape, const Texture* _texture)
{
	if (!_shape)
	{
		LOG(VerbosityType::Error, "You try to set the texture of a nullptr shape");
		return;
	}
	_shape->SetTexture(_texture);
}

std::string Krampus::TextureManager::GetExtensionNameByType(const TextureExtensionType& _textureType)
{
	const std::string _extensionNames[] =
	{
		"png",
		"jpg",
		"gif",
	};

	return _extensionNames[CAST(int, _textureType)];
}

void Krampus::TextureManager::Load(ShapeObject* _shapeObject, const std::string& _path, const TextureExtensionType& _textureType, const IRect& _rect, const bool _isRepeated, const bool _smooth)
{
	if (!_shapeObject)
	{
		LOG(VerbosityType::Error, "ShapeObject is nullptr in TextureManager::Load");
		return;
	}

	Texture& texRef = _shapeObject->GetTextureRef();

	if (_path.empty())
	{
		LOG(VerbosityType::Error, "Cannot open file with an empty path !");
		SetTextureToDefault(texRef);
		return;
	}

	const std::string textureRelative = _path + "." + GetExtensionNameByType(_textureType);
	auto sharedTex = LoadOrGetSharedTexture(textureRelative, _rect);

	if (!sharedTex)
	{
		// set default if load failed
		SetTextureToDefault(texRef);
	}
	else
	{
		// assign shared pointer into the actor's Texture wrapper (cheap -> shares pixels)
		texRef.SetSharedTexture(sharedTex);

		// apply flags
		texRef.SetRepeated(_isRepeated);
		texRef.SetSmooth(_smooth);
	}

	// finally link shape to the texture pointer
	SetTexture(_shapeObject->GetShape(), &texRef);
}

void Krampus::TextureManager::SetTextureRect(Shape* _shape, const IVector2& _start, const IVector2& _size)
{
	SetTextureRect(_shape, IRect(_start, _size));
}

void Krampus::TextureManager::SetTextureRect(Shape* _shape, const IRect& _rect)
{
	if (!_shape)
	{
		LOG(VerbosityType::Error, "You try to set the texture rect of a nullptr shape");
		return;
	}
	_shape->SetTextureRect(_rect);
}

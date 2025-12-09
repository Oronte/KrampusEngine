#include "TextureManager.h"

using namespace Krampus;

Krampus::TextureManager::TextureManager()
{
	std::filesystem::create_directories(texturePath);
	LoadDefaultTexture();
}

void Krampus::TextureManager::LoadDefaultTexture()
{
	SetTextureToDefault(defaultTexture);
}

void Krampus::TextureManager::SetTextureToDefault(Texture& _texture)
{
	const std::string& _finalPath = defaultTexturePath + "." + defaultTextureExtension;
	LoadTexture(_texture, _finalPath);
}

void Krampus::TextureManager::LoadTexture(Texture& _texture, const std::string& _path, const IRect& _rect)
{
	const std::string& _finalPath = texturePath + _path;

	if (!_texture.LoadFromFile(_finalPath, false, _rect))
	{
		LOG(VerbosityType::Error, "Cannot open file with the following path : \'" + _finalPath + "\' !");
		SetTextureToDefault(_texture);
	}
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
	Texture& _texture = _shapeObject->GetTextureRef();

	if (_path.empty())
	{
		LOG(VerbosityType::Error, "Cannot open file with an empty path !");
		SetTextureToDefault(_texture);
	}
	else
	{
		const std::string& _texturePath = _path + "." + GetExtensionNameByType(_textureType);
		LoadTexture(_texture, _texturePath, _rect);
		_texture.SetRepeated(_isRepeated);
		_texture.SetSmooth(_smooth);
	}

	SetTexture(_shapeObject->GetShape(), &_texture);
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

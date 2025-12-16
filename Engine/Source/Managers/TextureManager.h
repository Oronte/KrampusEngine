#pragma once
#include "Utilities/System/Singleton.h"
#include "Graphics/Shapes/ShapeObject.h"

#define M_TEXTURE Krampus::TextureManager::GetInstance()

namespace Krampus
{

	class TextureManager : public Singleton<TextureManager>
	{
		std::string texturePath = CONTENT_PATH + "Textures/";
		std::string defaultTexturePath = "Default";
		std::string defaultTextureExtension = "png";
		Texture defaultTexture;

		std::unordered_map<std::string, std::shared_ptr<sf::Texture>> cache;

	public:
		TextureManager();

	private:
		void LoadDefaultTexture();
		void SetTextureToDefault(Texture& _texture);

		std::shared_ptr<sf::Texture> LoadOrGetSharedTexture(const std::string& _path, const IRect& _rect = IRect());
		
		void SetTexture(Shape* _shape, const Texture* _texture);
		std::string GetExtensionNameByType(const TextureExtensionType& _textureType);

	public:
		void Load(ShapeObject* _shapeObject, const std::string& _path, const TextureExtensionType& _textureType = TextureExtensionType::PNG,
			const IRect& _rect = IRect(), const bool _isRepeated = false, const bool _smooth = true);
		void SetTextureRect(Shape* _shape, const IVector2& _start, const IVector2& _size);
		void SetTextureRect(Shape* _shape, const IRect& _rect);
	};

}


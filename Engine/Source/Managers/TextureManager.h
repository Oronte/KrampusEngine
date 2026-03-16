#pragma once
#include "Utilities/System/Singleton.h"
#include "Graphics/Shapes/ShapeObject.h"

#define M_TEXTURE Krampus::TextureManager::GetInstance()

namespace Krampus
{

	class TextureManager : public Singleton<TextureManager>
	{
#ifdef SHIP
		String		texturePath					= "../../Content/Textures/";
#else
		String		texturePath					= CONTENT_PATH + "Textures/";
#endif
		String		defaultTexturePath			= "_internal/Default";
		String		defaultTextureExtension		= "png";
		Texture		defaultTexture;

		std::unordered_map<String, std::shared_ptr<sf::Texture>> cache;

	public:
		TextureManager();

	private:
		void LoadDefaultTexture();
		void SetTextureToDefault(Texture& _texture);

		std::shared_ptr<sf::Texture> LoadOrGetSharedTexture(const String& _path, const IRect& _rect = IRect());
		
		void SetTexture(Shape* _shape, const Texture* _texture);
		String GetExtensionNameByType(const TextureExtensionType& _textureType);

	public:
		void Load(ShapeObject* _shapeObject, const String& _path, const TextureExtensionType& _textureType = TextureExtensionType::PNG,
			const IRect& _rect = IRect(), const Bool& _isRepeated = false, const Bool& _smooth = true);
		void SetTextureRect(Shape* _shape, const IVector2& _start, const IVector2& _size);
		void SetTextureRect(Shape* _shape, const IRect& _rect);
	};

}


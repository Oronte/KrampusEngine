#pragma once
#include "Utilities/System/Singleton.h"
#include "Graphics/Shapes/ShapeObject.h"

#define M_TEXTURE Krampus::TextureManager::GetInstance()

namespace Krampus
{

	/// <summary>
	/// Singleton manager responsible for loading, caching, and mapping Texture resources onto Shapes.
	/// </summary>
	class TextureManager : public Singleton<TextureManager>
	{
		String		texturePath					= CONTENT_PATH + "Textures/";
		String		defaultTexturePath			= "_internal/Default";
		String		defaultTextureExtension		= "png";
		Texture		defaultTexture;

		std::unordered_map<String, std::shared_ptr<sf::Texture>> cache;

	public:
		/// <summary>
		/// Constructs the TextureManager and loads the default fallback texture.
		/// </summary>
		TextureManager();

	private:
		void LoadDefaultTexture();
		void SetTextureToDefault(Texture& _texture);

		std::shared_ptr<sf::Texture> LoadOrGetSharedTexture(const String& _path, const IRect& _rect = IRect());
		
		void SetTexture(Shape* _shape, const Texture* _texture);
		String GetExtensionNameByType(const TextureExtensionType& _textureType);

	public:
		/// <summary>
		/// Loads a texture from Content/Textures/ and assigns it to a ShapeObject.
		/// </summary>
		/// <param name="_shapeObject">Target ShapeObject.</param>
		/// <param name="_path">Texture file name without extension.</param>
		/// <param name="_textureType">Texture format type (PNG, JPG, etc.).</param>
		/// <param name="_rect">Sub-rectangle coordinates (empty for whole texture).</param>
		/// <param name="_isRepeated">Whether texture repeats/tiles.</param>
		/// <param name="_smooth">Whether bilinear filtering is enabled.</param>
		void Load(ShapeObject* _shapeObject, const String& _path, const TextureExtensionType& _textureType = TextureExtensionType::PNG,
			const IRect& _rect = IRect(), const Bool& _isRepeated = false, const Bool& _smooth = true);

		/// <summary>
		/// Sets the texture coordinates sub-rectangle on a Shape using origin offset and dimensions.
		/// </summary>
		/// <param name="_shape">Target Shape.</param>
		/// <param name="_start">Top-left pixel coordinates in texture.</param>
		/// <param name="_size">Width and height in pixels.</param>
		void SetTextureRect(Shape* _shape, const IVector2& _start, const IVector2& _size);

		/// <summary>
		/// Sets the texture coordinates sub-rectangle on a Shape.
		/// </summary>
		/// <param name="_shape">Target Shape.</param>
		/// <param name="_rect">Sub-rectangle bounds.</param>
		void SetTextureRect(Shape* _shape, const IRect& _rect);
	};

}

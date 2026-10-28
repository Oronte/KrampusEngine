#pragma once
#include "Shape.h"
#include "Graphics/Ressources/Shader.h"

namespace Krampus
{

	/// <summary>
	/// Configuration data for initializing a circular shape object.
	/// </summary>
	struct CircleShapeData
	{
		Float					radius			= 50.0f;
		String					texturePath		= "_internal/Default";
		TextureExtensionType	textureType		= TextureExtensionType::PNG;
		IRect					textureRect;
		Int						pointCount		= 30;

		/// <summary>
		/// Constructs CircleShapeData with circle properties.
		/// </summary>
		/// <param name="_radius">Radius of the circle.</param>
		/// <param name="_texturePath">Path to texture resource.</param>
		/// <param name="_textureType">Texture file extension type.</param>
		/// <param name="_textureRect">Sub-rectangle of texture.</param>
		/// <param name="_pointCount">Number of approximation points.</param>
		CircleShapeData(const Float& _radius = 50.0f, const String& _texturePath = "_internal/Default", TextureExtensionType _textureType = TextureExtensionType::PNG, const IRect& _textureRect = IRect(), const Int& _pointCount = 30)
			: radius(_radius), texturePath(_texturePath), textureType(_textureType), textureRect(_textureRect), pointCount(_pointCount) { }
	};

	/// <summary>
	/// Configuration data for initializing a rectangular shape object.
	/// </summary>
	struct RectangleShapeData
	{
		FVector2				size			= FVector2(200.0f, 100.0f);
		String					texturePath		= "_internal/Default";
		TextureExtensionType	textureType		= TextureExtensionType::PNG;
		IRect					textureRect;
		Bool					isRepeated		= false;

		/// <summary>
		/// Constructs RectangleShapeData with rectangle properties.
		/// </summary>
		/// <param name="_size">Width and height dimensions.</param>
		/// <param name="_texturePath">Path to texture resource.</param>
		/// <param name="_textureType">Texture file extension type.</param>
		/// <param name="_textureRect">Sub-rectangle of texture.</param>
		/// <param name="_isRepeated">Whether texture repeating is enabled.</param>
		RectangleShapeData(const FVector2& _size = FVector2(200.0f, 100.0f), const String& _texturePath = "_internal/Default", TextureExtensionType _textureType = TextureExtensionType::PNG, const IRect& _textureRect = IRect(), const Bool& _isRepeated = false)
			: size(_size), texturePath(_texturePath), textureType(_textureType), textureRect(_textureRect), isRepeated(_isRepeated) { }
	};

	/// <summary>
	/// Supported 2D shape geometry types.
	/// </summary>
	enum class ShapeType
	{
		Circle,
		Rectangle
	};

	/// <summary>
	/// Union holding geometric dimension values (radius for circle, 2D size vector for rectangle).
	/// </summary>
	union ShapeSizeData
	{
		Float		radius;
		FVector2	size;

		/// <summary>
		/// Default constructor initializing radius to zero.
		/// </summary>
		ShapeSizeData() : radius(0.0f) {}
	};

	/// <summary>
	/// High-level 2D shape renderer holding an underlying Shape, its Texture, dimensions, and type.
	/// </summary>
	class ShapeObject
	{
		Texture					texture;
		std::unique_ptr<Shape>	shape;
		ShapeType				shapeType;
		ShapeSizeData			sizeData;
		String					texturePath;
		TextureExtensionType	textureType	= TextureExtensionType::PNG;

	public:
		/// <summary>
		/// Gets the shape type (Circle or Rectangle).
		/// </summary>
		/// <returns>ShapeType enum value.</returns>
		inline ShapeType GetShapeType()
		{
			return shapeType;
		}

		/// <summary>
		/// Gets a reference to the texture applied to the shape.
		/// </summary>
		/// <returns>Reference to Texture.</returns>
		inline Texture& GetTextureRef()
		{
			return texture;
		}

		/// <summary>
		/// Gets a pointer to the polymorphic Shape instance.
		/// </summary>
		/// <returns>Pointer to Shape.</returns>
		inline Shape* GetShape() const
		{
			return shape.get();
		}

		/// <summary>
		/// Gets a reference to the shape dimension data.
		/// </summary>
		/// <returns>Reference to ShapeSizeData union.</returns>
		inline ShapeSizeData& GetSizeData()
		{
			return sizeData;
		}

		/// <summary>
		/// Constructs a ShapeObject initialized as a circle.
		/// </summary>
		/// <param name="_data">Circle parameters.</param>
		ShapeObject(const CircleShapeData& _data);

		/// <summary>
		/// Constructs a ShapeObject initialized as a rectangle.
		/// </summary>
		/// <param name="_data">Rectangle parameters.</param>
		ShapeObject(const RectangleShapeData& _data);

		/// <summary>
		/// Sets the dimensions of the rectangular shape.
		/// </summary>
		/// <param name="_size">Dimensions vector.</param>
		void SetSize(const FVector2& _size);

		/// <summary>
		/// Gets the dimensions of the rectangular shape.
		/// </summary>
		/// <returns>Dimensions vector.</returns>
		FVector2 GetSize() const;

		/// <summary>
		/// Sets the radius of the circular shape.
		/// </summary>
		/// <param name="_radius">Radius value.</param>
		void SetRadius(const float& _radius);

		/// <summary>
		/// Gets the radius of the circular shape.
		/// </summary>
		/// <returns>Radius value.</returns>
		Float GetRadius() const;

		/// <summary>
		/// Sets the point count approximation for the circle polygon.
		/// </summary>
		/// <param name="_pointCount">Number of points.</param>
		void SetPointCount(const size_t& _pointCount);

		/// <summary>
		/// Gets the point count of the shape polygon.
		/// </summary>
		/// <returns>Number of points.</returns>
		Int GetPointCount() const;

		/// <summary>
		/// Loads and applies a texture resource to the shape object.
		/// </summary>
		/// <param name="_path">Texture file path or resource key.</param>
		/// <param name="_textureType">File format type.</param>
		/// <param name="_rect">Sub-rectangle coordinates.</param>
		/// <param name="_isRepeated">Whether texture repeats/tiles.</param>
		/// <param name="_smooth">Whether bilinear filtering is enabled.</param>
		/// <summary>Extension type last given to SetTexture.</summary>
		inline TextureExtensionType GetTextureType() const
		{
			return textureType;
		}

		/// <summary>
		/// Turns the shape into a circle or a rectangle, keeping its transform, color and texture.
		/// The new shape is as big as the old one (a circle of radius r becomes a square of side 2r).
		/// </summary>
		void SetShapeType(const ShapeType& _type);

		/// <summary>Path last given to SetTexture (empty if none).</summary>
		inline const String& GetTexturePath() const
		{
			return texturePath;
		}

		void SetTexture(const String& _path, TextureExtensionType _textureType = TextureExtensionType::PNG, const IRect& _rect = IRect(), const Bool& _isRepeated = false, const Bool& _smooth = true);
	};

}

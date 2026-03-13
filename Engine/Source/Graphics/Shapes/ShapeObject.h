#pragma once
#include "Shape.h"
#include "Graphics/Ressources/Shader.h"

namespace Krampus
{

	struct CircleShapeData
	{
		Float					radius			= 50.0f;
		String					texturePath		= "_internal/Default";
		TextureExtensionType	textureType		= TextureExtensionType::PNG;
		IRect					textureRect;
		Int						pointCount		= 30;

		CircleShapeData(const Float& _radius = 50.0f, const String& _texturePath = "_internal/Default", TextureExtensionType _textureType = TextureExtensionType::PNG, const IRect& _textureRect = IRect(), const Int& _pointCount = 30)
			: radius(_radius), texturePath(_texturePath), textureType(_textureType), textureRect(_textureRect), pointCount(_pointCount) { }
	};

	struct RectangleShapeData
	{
		FVector2				size			= FVector2(200.0f, 100.0f);
		String					texturePath		= "_internal/Default";
		TextureExtensionType	textureType		= TextureExtensionType::PNG;
		IRect					textureRect;
		Bool					isRepeated		= false;

		RectangleShapeData(const FVector2& _size = FVector2(200.0f, 100.0f), const String& _texturePath = "_internal/Default", TextureExtensionType _textureType = TextureExtensionType::PNG, const IRect& _textureRect = IRect(), const Bool& _isRepeated = false)
			: size(_size), texturePath(_texturePath), textureType(_textureType), textureRect(_textureRect), isRepeated(_isRepeated) { }
	};

	enum class ShapeType
	{
		Circle,
		Rectangle
	};

	union ShapeSizeData
	{
		Float		radius;
		FVector2	size;

		ShapeSizeData() : radius(0.0f) {}
	};

	class ShapeObject
	{
		Texture					texture;
		std::unique_ptr<Shape>	shape;
		ShapeType				shapeType;
		ShapeSizeData			sizeData;

	public:
		inline ShapeType GetShapeType()
		{
			return shapeType;
		}
		inline Texture& GetTextureRef()
		{
			return texture;
		}
		inline Shape* GetShape() const
		{
			return shape.get();
		}
		inline ShapeSizeData& GetSizeData()
		{
			return sizeData;
		}

		ShapeObject(const CircleShapeData& _data);
		ShapeObject(const RectangleShapeData& _data);

		void SetSize(const FVector2& _size);
		FVector2 GetSize() const;
		void SetRadius(const float& _radius);
		Float GetRadius() const;
		void SetPointCount(const size_t& _pointCount);
		Int GetPointCount() const;
		void SetTexture(const String& _path, TextureExtensionType _textureType = TextureExtensionType::PNG, const IRect& _rect = IRect(), const Bool& _isRepeated = false, const Bool& _smooth = true);
	};

}




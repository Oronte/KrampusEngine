#pragma once
#include "Shape.h"

namespace Krampus
{

	struct CircleShapeData
	{
		float radius = 50.0f;
		std::string texturePath = "Default";
		TextureExtensionType textureType = TextureExtensionType::PNG;
		IRect textureRect;
		size_t pointCount = 30;
	};

	struct RectangleShapeData
	{
		FVector2 size = FVector2(200.0f, 100.0f);
		std::string texturePath = "Default";
		TextureExtensionType textureType = TextureExtensionType::PNG;
		IRect textureRect;
		bool isRepeated = false;
	};

	enum class ShapeType
	{
		Circle,
		Rectangle
	};

	union ShapeSizeData
	{
		float radius;
		FVector2 size;

		ShapeSizeData() : radius(0.0f) {}
	};

	class ShapeObject
	{
		Texture texture;
		std::unique_ptr<Shape> shape;
		ShapeType shapeType;
		ShapeSizeData sizeData;

	public:
		INLINE ShapeType GetShapeType()
		{
			return shapeType;
		}
		INLINE Texture& GetTextureRef()
		{
			return texture;
		}
		INLINE Shape* GetShape() const
		{
			return shape.get();
		}
		INLINE ShapeSizeData& GetSizeData()
		{
			return sizeData;
		}

		ShapeObject(const CircleShapeData& _data);
		ShapeObject(const RectangleShapeData& _data);
	};

}




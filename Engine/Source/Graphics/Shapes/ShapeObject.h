#pragma once
#include "Shape.h"
#include "Graphics/Ressources/Shader.h"

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
	};

}




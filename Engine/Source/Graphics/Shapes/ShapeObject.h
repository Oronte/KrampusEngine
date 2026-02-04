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

		void SetSize(const FVector2& _size);
		FVector2 GetSize() const;
		void SetRadius(const float& _radius);
		float GetRadius() const;
		void SetPointCount(const size_t& _pointCount);
		size_t GetPointCount() const;
		void SetTexture(const std::string& _path, TextureExtensionType _textureType = TextureExtensionType::PNG, const IRect& _rect = IRect(), const bool _isRepeated = false, const bool _smooth = true);
	};

}




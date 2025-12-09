#pragma once
#include "Shape.h"

namespace Krampus
{

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

		// Circle
		ShapeObject(const float& _radius, const std::string& _path = "Default", const TextureExtensionType& _textureType = TextureExtensionType::PNG,
			const IRect& _rect = IRect(), const size_t& _pointCount = 30);
		// Rectangle
		ShapeObject(const FVector2& _size, const std::string& _path = "Default", const TextureExtensionType& _textureType = TextureExtensionType::PNG,
			const IRect& _rect = IRect(), const bool& _isRepeated = false);
	};

}




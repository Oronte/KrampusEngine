#include "ShapeObject.h"
#include "CircleShape.h"
#include "RectangleShape.h"
#include "Managers/TextureManager.h"

using namespace Krampus;


Krampus::ShapeObject::ShapeObject(const float& _radius, const std::string& _path, const TextureExtensionType& _textureType, const IRect& _rect, const size_t& _pointCount)
{
	shapeType = ShapeType::Circle;
	sizeData.radius = _radius;
	shape = std::make_unique<CircleShape>(_radius, _pointCount);
	M_TEXTURE.Load(this, _path, _textureType);
}

Krampus::ShapeObject::ShapeObject(const FVector2& _size, const std::string& _path, const TextureExtensionType& _textureType, const IRect& _rect, const bool& _isRepeated)
{
	shapeType = ShapeType::Rectangle;
	sizeData.size = _size;
	shape = std::make_unique<RectangleShape>(_size);
	M_TEXTURE.Load(this, _path, _textureType);
}

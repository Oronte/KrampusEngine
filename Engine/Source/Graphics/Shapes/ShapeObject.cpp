#include "ShapeObject.h"
#include "CircleShape.h"
#include "RectangleShape.h"
#include "Managers/TextureManager.h"

using namespace Krampus;


Krampus::ShapeObject::ShapeObject(const CircleShapeData& _data)
{
	shapeType = ShapeType::Circle;
	const float& _radius = _data.radius;
	sizeData.radius = _radius;
	shape = std::make_unique<CircleShape>(_radius, _data.pointCount);
	shape->SetOriginAtMiddle();
	M_TEXTURE.Load(this, _data.texturePath, _data.textureType);
}

Krampus::ShapeObject::ShapeObject(const RectangleShapeData& _data)
{
	shapeType = ShapeType::Rectangle;
	const FVector2& _size = _data.size;
	sizeData.size = _size;
	shape = std::make_unique<RectangleShape>(_size);
	shape->SetOriginAtMiddle();
	M_TEXTURE.Load(this, _data.texturePath, _data.textureType);
}

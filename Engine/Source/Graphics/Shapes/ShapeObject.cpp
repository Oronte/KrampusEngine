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
	SetTexture(_data.texturePath, _data.textureType, _data.textureRect);
}

Krampus::ShapeObject::ShapeObject(const RectangleShapeData& _data)
{
	shapeType = ShapeType::Rectangle;
	const FVector2& _size = _data.size;
	sizeData.size = _size;
	shape = std::make_unique<RectangleShape>(_size);
	shape->SetOriginAtMiddle();
	SetTexture(_data.texturePath, _data.textureType, _data.textureRect, _data.isRepeated);
}

void Krampus::ShapeObject::SetSize(const FVector2& _size)
{
	if (shapeType != ShapeType::Rectangle) shape = std::make_unique<RectangleShape>(_size);
	else Cast<RectangleShape>(shape.get())->SetSize(_size);
}

FVector2 Krampus::ShapeObject::GetSize() const
{
	if (shapeType != ShapeType::Rectangle)
	{
		LOG_WARNING("Tried to access to the size of a shape that is not a Rectangle; invalid size !");
		return FVector2::Zero();
	}
	return Cast<RectangleShape>(shape.get())->GetSize();
}

void Krampus::ShapeObject::SetRadius(const float& _radius)
{
	if (shapeType != ShapeType::Circle) shape = std::make_unique<CircleShape>(_radius);
	else Cast<CircleShape>(shape.get())->SetRadius(_radius);
}

Float Krampus::ShapeObject::GetRadius() const
{
	if (shapeType != ShapeType::Circle)
	{
		LOG_WARNING("Tried to access to the radius of a shape that is not a Circle; invalid radius !");
		return 0.0f;
	}
	return Cast<CircleShape>(shape.get())->GetRadius();
}

void Krampus::ShapeObject::SetPointCount(const size_t& _pointCount)
{
	if (shapeType != ShapeType::Circle) shape = std::make_unique<CircleShape>(50.0f, _pointCount);
	else Cast<CircleShape>(shape.get())->SetPointCount(_pointCount);
}

Int Krampus::ShapeObject::GetPointCount() const
{
	if (shapeType != ShapeType::Circle)
	{
		LOG_WARNING("Tried to access to the point count of a shape that is not a Circle; invalid result !");
		return size_t(0);
	}
	return Cast<CircleShape>(shape.get())->GetPointCount();
}

void Krampus::ShapeObject::SetTexture(const String& _path, TextureExtensionType _textureType, const IRect& _rect, const Bool& _isRepeated, const Bool& _smooth)
{
	M_TEXTURE.Load(this, _path, _textureType, _rect, _isRepeated, _smooth);
}

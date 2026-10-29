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
	if (shapeType != ShapeType::Rectangle) SetShapeType(ShapeType::Rectangle);
	Cast<RectangleShape>(shape.get())->SetSize(_size);
	sizeData.size = _size;
	shape->SetOriginAtMiddle();
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
	if (shapeType != ShapeType::Circle) SetShapeType(ShapeType::Circle);
	Cast<CircleShape>(shape.get())->SetRadius(_radius);
	sizeData.radius = _radius;
	shape->SetOriginAtMiddle();
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
	if (shapeType != ShapeType::Circle) SetShapeType(ShapeType::Circle);
	Cast<CircleShape>(shape.get())->SetPointCount(_pointCount);
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
	texturePath = _path;
	textureType = _textureType;
	M_TEXTURE.Load(this, _path, _textureType, _rect, _isRepeated, _smooth);
}


void Krampus::ShapeObject::SetShapeType(const ShapeType& _type)
{
	if (_type == shapeType) return;

	const FVector2	_position	= shape->GetPosition();
	const Angle		_rotation	= shape->GetRotation();
	const FVector2	_scale		= shape->GetScale();
	const Color		_fill		= shape->GetFillColor();
	const IRect		_rect		= shape->GetTextureRect();	// what the new shape shows of the picture, and how the picture is filtered
	const Bool		_repeated	= texture.IsRepeated();
	const Bool		_smooth		= texture.IsSmooth();

	if (_type == ShapeType::Rectangle)
	{
		const FVector2 _size(sizeData.radius * 2.0f, sizeData.radius * 2.0f);
		shape = std::make_unique<RectangleShape>(_size);
		sizeData.size = _size;
	}
	else
	{
		const float _radius = std::min(sizeData.size.x, sizeData.size.y) * 0.5f;
		shape = std::make_unique<CircleShape>(_radius, 30);
		sizeData.radius = _radius;
	}
	shapeType = _type;

	shape->SetOriginAtMiddle();
	shape->SetPosition(_position);
	shape->SetRotation(_rotation);
	shape->SetScale(_scale);
	shape->SetFillColor(_fill);
	SetTexture(texturePath, textureType, IRect(), _repeated, _smooth);	// the new shape needs the texture again
	if (_rect.GetSize().x > 0 && _rect.GetSize().y > 0) shape->SetTextureRect(_rect);
}

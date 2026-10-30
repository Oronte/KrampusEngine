#include "ShapeObject.h"
#include "CircleShape.h"
#include "RectangleShape.h"
#include "PolygonShape.h"
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
	ListenToShapeSaves();
}

Krampus::ShapeObject::ShapeObject(const RectangleShapeData& _data)
{
	shapeType = ShapeType::Rectangle;
	const FVector2& _size = _data.size;
	sizeData.size = _size;
	shape = std::make_unique<RectangleShape>(_size);
	shape->SetOriginAtMiddle();
	SetTexture(_data.texturePath, _data.textureType, _data.textureRect, _data.isRepeated);
	ListenToShapeSaves();
}

Krampus::ShapeObject::ShapeObject(const PolygonShapeData& _data)
{
	shapeType = ShapeType::Polygon;
	polygonName = _data.shape.StdString();
	const Polygon2D& _polygon = ShapeAssets::Get(polygonName);
	sizeData.size = _polygon.Bounds().GetSize();
	shape = std::make_unique<PolygonShape>(_polygon);	// its origin is the (0, 0) of the points, not its middle
	SetTexture(_data.texturePath, _data.textureType, _data.textureRect);
	ListenToShapeSaves();
}

void Krampus::ShapeObject::ListenToShapeSaves()
{
	// A shape saved in the Shape Editor shows at once on everything that uses it.
	shapeSavedHandle = ShapeAssets::OnChanged().AddListener([this](const std::string& _name)
	{
		if (shapeType == ShapeType::Polygon && _name == polygonName)
		{
			const Polygon2D& _polygon = ShapeAssets::Get(polygonName);
			Cast<PolygonShape>(shape.get())->SetPolygon(_polygon);
			sizeData.size = _polygon.Bounds().GetSize();
		}
	});
}

void Krampus::ShapeObject::SetPolygon(const std::string& _shapeName)
{
	polygonName = _shapeName;
	if (shapeType != ShapeType::Polygon)
	{
		SetShapeType(ShapeType::Polygon);	// takes polygonName
		return;
	}
	const Polygon2D& _polygon = ShapeAssets::Get(polygonName);
	Cast<PolygonShape>(shape.get())->SetPolygon(_polygon);
	sizeData.size = _polygon.Bounds().GetSize();
}

Polygon2D Krampus::ShapeObject::GetOutline() const
{
	if (shapeType == ShapeType::Polygon) return Cast<PolygonShape>(shape.get())->GetPolygon();
	if (shapeType == ShapeType::Rectangle) return Polygon2D::Box(sizeData.size);
	return Polygon2D::Regular(std::max(8, static_cast<int>(GetPointCount())), sizeData.radius, 0.f);
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
	if (shapeType == ShapeType::Polygon) return sizeData.size;	// the bounds of its points
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

	// How big the old shape is, whatever it is: the new one gets about the same size.
	const FVector2 _oldSize = shapeType == ShapeType::Circle ? FVector2(sizeData.radius * 2.0f, sizeData.radius * 2.0f) : sizeData.size;

	if (_type == ShapeType::Rectangle)
	{
		shape = std::make_unique<RectangleShape>(_oldSize);
		sizeData.size = _oldSize;
	}
	else if (_type == ShapeType::Circle)
	{
		const float _radius = std::max(1.0f, std::min(_oldSize.x, _oldSize.y) * 0.5f);
		shape = std::make_unique<CircleShape>(_radius, 30);
		sizeData.radius = _radius;
	}
	else
	{
		if (polygonName.empty()) polygonName = ShapeAssets::DEFAULT_NAME;
		const Polygon2D& _polygon = ShapeAssets::Get(polygonName);
		shape = std::make_unique<PolygonShape>(_polygon);
		sizeData.size = _polygon.Bounds().GetSize();
	}
	shapeType = _type;

	if (_type != ShapeType::Polygon) shape->SetOriginAtMiddle();
	shape->SetPosition(_position);
	shape->SetRotation(_rotation);
	shape->SetScale(_scale);
	shape->SetFillColor(_fill);
	SetTexture(texturePath, textureType, IRect(), _repeated, _smooth);	// the new shape needs the texture again
	if (_rect.GetSize().x > 0 && _rect.GetSize().y > 0) shape->SetTextureRect(_rect);
}

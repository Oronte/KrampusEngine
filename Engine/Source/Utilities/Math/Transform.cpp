#include "Transform.h"

using namespace Krampus;

Krampus::Transform::Transform()
{
	origin = position = FVector2();
	rotation = Angle();
	scale = FVector2(1.0f);
}

Krampus::Transform::Transform(const FVector2& _position, const Angle& _rotation, const FVector2& _scale)
{
	origin = FVector2();
	position = _position;
	rotation = _rotation;
	scale = _scale;
}

Krampus::Transform::Transform(const FVector2& _origin, const FVector2& _position, const Angle& _rotation, const FVector2& _scale)
{
	origin = _origin;
	position = _position;
	rotation = _rotation;
	scale = _scale;
}

std::string Krampus::Transform::ToString() const
{
	return std::format("Position = {} | Rotation (degrees) = {} | Scale = {} | Origin = {}",
		position.ToString(),
		rotation.ToDegrees(),
		scale.ToString(),
		origin.ToString());
}

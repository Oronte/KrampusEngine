#include "Transform.h"

using namespace Krampus;

Krampus::Transform::Transform()
{
	position = FVector2();
	rotation = Angle();
	scale = FVector2(1.0f);
}

Krampus::Transform::Transform(const FVector2& _position, const Angle& _rotation, const FVector2& _scale)
{
	position = _position;
	rotation = _rotation;
	scale = _scale;
}

void Krampus::Transform::LookAt(const FVector2& _target)
{
	const FVector2 _direction = _target - position;
	rotation = Angle(_direction.AtanToRadian());
}

std::string Krampus::Transform::ToString() const
{
	return std::format("Position = {} | Rotation (degrees) = {} | Scale = {}",
		position.ToString(),
		rotation.ToDegrees(),
		scale.ToString());
}

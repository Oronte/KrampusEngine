#include "Collider2D.h"

Krampus::Collider2D::Collider2D(Level* _level, const float& _radius)
	: Actor(_level)
{
	collision = CreateComponent<CollisionComponent>(_radius, CollisionChannel::Trigger, CollisionChannel::All);
}

Krampus::Collider2D::Collider2D(Level* _level, const FVector2& _size)
	: Actor(_level)
{
	collision = CreateComponent<CollisionComponent>(_size, CollisionChannel::Trigger, CollisionChannel::All);
}

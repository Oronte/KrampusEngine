#include "PhysicsActor.h"
#include "GameFramework/Level.h"

Krampus::PhysicsActor::PhysicsActor(Level* _level, const RectangleShapeData& _data)
	: SpriteActor(_level, _data)
{
	rigidbody = CreateComponent<RigidbodyComponent>();
	rigidbody->BindCollisionResponse();
}

Krampus::PhysicsActor::PhysicsActor(Level* _level, const CircleShapeData& _data)
	: SpriteActor(_level, _data)
{
	rigidbody = CreateComponent<RigidbodyComponent>();
	rigidbody->BindCollisionResponse();
}

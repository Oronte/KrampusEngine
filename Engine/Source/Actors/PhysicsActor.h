#pragma once
#include "Components/RigidbodyComponent.h"
#include "SpriteActor.h"

namespace Krampus
{

	class PhysicsActor : public SpriteActor
	{
		RigidbodyComponent* rigidbody = nullptr;

	public:
		PhysicsActor(Level* _level, const RectangleShapeData& _data);
		PhysicsActor(Level* _level, const CircleShapeData& _data);
	};


}

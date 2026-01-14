#pragma once
#include "Components/CollisionComponent.h"
#include "Actor.h"


namespace Krampus
{

	class Collider2D : public Actor
	{
		CollisionComponent* collision;

	public:
		Collider2D(Level* _level, const float& _radius);
		Collider2D(Level* _level, const FVector2& _size);
	};

}


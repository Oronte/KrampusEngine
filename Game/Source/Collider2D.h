#pragma once
#include "Actors/Actor.h"
#include "Components/CollisionComponent.h"

using namespace Krampus;

class Collider2D : public Actor
{
	CollisionComponent* collision = nullptr;

public:
	Collider2D(Level* _level, const FVector2& _size);
	Collider2D(Level* _level, float _radius);
};


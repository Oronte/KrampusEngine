#pragma once
#include "Actors/Actor.h"
#include "Components/SpriteComponent.h"
#include "Components/AnimationComponent.h"
#include "Components/CameraComponent.h"
#include "Components/CollisionComponent.h"

using namespace Krampus;

class Player : public Actor
{
	SpriteComponent* sprite = nullptr;
	AnimationComponent* animation = nullptr;
	CameraComponent* camera = nullptr;
	CollisionComponent* collision = nullptr;

public:
	Player(Level* _level);
};


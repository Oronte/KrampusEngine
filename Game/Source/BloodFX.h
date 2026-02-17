#pragma once
#include "Actors/Actor.h"
#include "Components/SpriteComponent.h"
#include "Components/AnimationComponent.h"

using namespace Krampus;

class BloodFX : public Actor
{
	SpriteComponent* sprite = nullptr;
	AnimationComponent* animation = nullptr;

	Event<>::ListenerHandle handle;

public:
	BloodFX(Level* _level);
};

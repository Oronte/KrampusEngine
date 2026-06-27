#pragma once
#include "Actors/Actor.h"
#include "Components/SpriteComponent.h"
#include "Components/AnimationComponent.h"

using namespace Krampus;

class DeathFX : public Actor
{
	SpriteComponent*		sprite		= nullptr;
	AnimationComponent*		animation	= nullptr;

	Sound					boomSound;

	Event<>::ListenerHandle handle;

public:
	DeathFX(Level* _level);

	virtual void Construct() override;
	virtual void BeginPlay() override;
};


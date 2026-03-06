#pragma once
#include "Actors/Actor.h"
#include "Components/SpriteComponent.h"
#include "Components/AnimationComponent.h"

using namespace Krampus;

class Background : public Actor
{
	SpriteComponent* sprite = nullptr;
	AnimationComponent* animation = nullptr;

public:
	Background(Level* _level);

	virtual void Construct() override;
	virtual void BeginPlay() override;
};


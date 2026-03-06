#pragma once
#include "Actors/Actor.h"
#include "Components/SpriteComponent.h"
#include "Components/InputComponent.h"

using namespace Krampus;

class Weapon : public Actor
{
	SpriteComponent* sprite = nullptr;
	InputComponent* inputs = nullptr;

	float bulletsPerSec = 7.5f;
	bool canShoot = true;
	float spread = 0.15f;

public:
	Weapon(Level* _level);

	virtual void Construct() override;

	void Shoot();
};


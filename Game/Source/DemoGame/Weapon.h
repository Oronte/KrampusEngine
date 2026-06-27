#pragma once
#include "Actors/Actor.h"
#include "Components/SpriteComponent.h"
#include "Components/InputComponent.h"
#include "Player.h"

using namespace Krampus;

class Weapon : public Actor
{
	SpriteComponent*	sprite			= nullptr;
	InputComponent*		inputs			= nullptr;

	Player*				player			= nullptr;
	Float				bulletsPerSec	= 7.5f;
	Bool				canShoot		= true;
	Float				spread			= 0.15f;

	Sound				shootSound;

	Event<>::ListenerHandle handle;

public:
	Weapon(Level* _level, Player* _player);

	virtual void Construct() override;

	void Shoot();
};


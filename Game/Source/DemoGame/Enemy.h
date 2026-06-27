#pragma once
#include "Player.h"
#include "Components/AnimationComponent.h"

using namespace Krampus;

class Enemy : public Actor
{
	SpriteComponent*			sprite		= nullptr;
	AnimationComponent*			animation	= nullptr;
	CollisionComponent*			collision	= nullptr;
	RigidbodyComponent*			rigidbody	= nullptr;
	HealthComponent*			health		= nullptr;

	Sound						enemySound;
	Sound						hurtSound;

	Player*						player		= nullptr;
	Event<CollisionInfo>::ListenerHandle handle;
	Event<>::ListenerHandle		deathHandle;
	Event<>::ListenerHandle		soundHandle;
	Event<Int>::ListenerHandle	hurtSoundHandle;

public:
	Enemy(Level* _level, Player* _player);

	virtual void Construct() override;
	virtual void Tick(const Float& _deltaTime) override;
	void Die();
};


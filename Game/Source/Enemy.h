#pragma once
#include "Actors/Actor.h"
#include "Components/SpriteComponent.h"
#include "Components/AnimationComponent.h"
#include "Components/CollisionComponent.h"
#include "Components/RigidbodyComponent.h"
#include "HealthComponent.h"

using namespace Krampus;

class Enemy : public Actor
{
	SpriteComponent* sprite = nullptr;
	AnimationComponent* animation = nullptr;
	CollisionComponent* collision = nullptr;
	RigidbodyComponent* rigidbody = nullptr;
	HealthComponent* health = nullptr;

	Actor* player = nullptr;
	Event<CollisionInfo>::ListenerHandle handle;

public:
	Enemy(Level* _level, Actor* _player);

	virtual void Construct() override;
	virtual void Tick(const float& _deltaTime) override;
	void Die();
};


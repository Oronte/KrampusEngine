#pragma once
#include "Actors/Actor.h"
#include "Components/SpriteComponent.h"
#include "Components/AnimationComponent.h"
#include "Components/CollisionComponent.h"

using namespace Krampus;

class Bullet : public Actor
{
	SpriteComponent* sprite = nullptr;
	AnimationComponent* animation = nullptr;
	CollisionComponent* collision = nullptr;

	float moveSpeed = 1000.0f;
	FVector2 direction = FVector2::Zero();
	bool canMove = true;
	float lifeSpan = 0.5f;
	float damage = 25.0f;
	bool isDying = false;

	Timer* lifeSpanTimer = nullptr;
	Event<CollisionInfo>::ListenerHandle handle;

public:
	Bullet(Level* _level, const FVector2& _direction);

	virtual void Construct() override;
	virtual void BeginPlay() override;
	virtual void Tick(const float& _deltaTime) override;
	
	void Move(float _deltaTime);
	void OnCollision(CollisionInfo _info);
};


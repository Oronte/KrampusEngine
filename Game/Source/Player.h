#pragma once
#include "Actors/Actor.h"
#include "Components/SpriteComponent.h"
#include "Components/CollisionComponent.h"
#include "Components/RigidbodyComponent.h"
#include "Components/InputComponent.h"
#include "Components/AnimationComponentSM.h"
#include "Components/CameraComponent.h"
#include "Components/AudioListenerComponent.h"
#include "HealthComponent.h"


using namespace Krampus;

class Player : public Actor
{
	SpriteComponent*			sprite		= nullptr;
	CollisionComponent*			collision	= nullptr;
	RigidbodyComponent*			rigidbody	= nullptr;
	InputComponent*				inputs		= nullptr;
	CameraComponent*			camera		= nullptr;
	AudioListenerComponent*		listener	= nullptr;
	HealthComponent*			health		= nullptr;
	AnimationComponentSM*		animationSM = nullptr;

	float						moveSpeed	= 200.0f;
	float						jumpForce	= 500.0f;
	bool						isRunning	= false;
	bool						canMove		= true;
	bool						isJumping	= false;
	bool						isHit		= false;
	bool						isDying		= false;

	Event<CollisionInfo>::ListenerHandle handle;
	Event<CollisionInfo>::ListenerHandle handleTemp;
	Event<>::ListenerHandle deathHandle;

public:
	Player(Level* _level);

	virtual void Construct() override;
	virtual void BeginPlay() override;
	virtual void Tick(const float& _deltaTime) override;

	void MoveLeft();
	void MoveRight();
	void Stop();
	void Jump();

	void Die();
};


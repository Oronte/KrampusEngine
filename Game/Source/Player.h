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

	Float						moveSpeed	= 200.0f;
	Float						jumpForce	= 500.0f;
	Bool						isRunning	= false;
	Bool						canMove		= true;
	Bool						isJumping	= false;
	Bool						isHit		= false;
	Bool						isDying		= false;
	Int							score		= 0;

	Sound						footStep;
	Sound						hurtSound;

	Event<CollisionInfo>::ListenerHandle handle;
	Event<CollisionInfo>::ListenerHandle handleTemp;
	Event<>::ListenerHandle		deathHandle;
	Event<>::ListenerHandle		footstepHandle;
	Event<Int>::ListenerHandle	hurtHandle;

public:
	Event<>						onDeath;

public:
	inline void AddScore(const int _score)
	{
		score += _score;
	}
	inline Int GetScore() const
	{
		return score;
	}

	Player(Level* _level);

	virtual void Construct() override;
	virtual void BeginPlay() override;
	virtual void Tick(const Float& _deltaTime) override;

	void MoveLeft();
	void MoveRight();
	void Stop();
	void Jump();

	void Die();
};


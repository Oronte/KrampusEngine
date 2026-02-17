#pragma once
#include "Actors/Actor.h"
#include "Components/SpriteComponent.h"
#include "Components/AnimationComponent.h"
#include "Components/CameraComponent.h"
#include "Components/CollisionComponent.h"
#include "Components/InputComponent.h"

using namespace Krampus;

class Player : public Actor
{
	SpriteComponent* sprite = nullptr;
	AnimationComponent* animation = nullptr;
	CameraComponent* camera = nullptr;
	CollisionComponent* collision = nullptr;
	InputComponent* inputs = nullptr;

	enum class State
	{
		Moving,
		Idle
	};
	State state = State::Idle;

public:
	Player(Level* _level);

	void MoveLeft();
	void MoveRight();

	void Idle();
	void Move();
	void Dash();
	void Hit();
	void Death();
	void Sleep();
};


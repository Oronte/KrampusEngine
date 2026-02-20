#pragma once
#include "Actors/Actor.h"
#include "Components/SpriteComponent.h"
#include "Components/CollisionComponent.h"
#include "Components/InputComponent.h"
#include "Components/AnimationComponent.h"

using namespace Krampus;

class Player : public Actor
{
	SpriteComponent* sprite = nullptr;
	CollisionComponent* collision = nullptr;
	InputComponent* inputs = nullptr;
	AnimationComponent* animation = nullptr;

	//Data
	float moveSpeed = 200.0f;
	float health = 1000.0f;
	float maxHealth = 1000.0f;

public:
	Player(Level* _level);

	void MoveLeft();
	void MoveRight();
	void MoveUp();
	void MoveDown();
};


#pragma once
#include "Actors/Actor.h"
#include "Components/SpriteComponent.h"
#include "Components/CollisionComponent.h"
#include "Components/InputComponent.h"
#include "Components/AnimationComponent.h"
#include "Components/CameraComponent.h"


class CustomActor : public Krampus::Actor
{
	Krampus::SpriteComponent* sprite = nullptr;
	Krampus::CollisionComponent* collision = nullptr;
	Krampus::InputComponent* inputs = nullptr;
	Krampus::AnimationComponent* animation = nullptr;
	Krampus::CameraComponent* camera = nullptr;

	bool isRunning = false;

public:
	CustomActor(Krampus::Level* _level);

	virtual void Construct() override;
	virtual void BeginPlay() override;
	virtual void Tick(const float& _deltaTime) override;
	virtual void Deconstruct() override;
	virtual void BeginDestroy() override;

	void MoveLeft();
	void MoveRight();
	void Stop();
};


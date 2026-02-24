#pragma once
#include "Core/CoreMinimal.h"
#include "Actors/Actor.h"
#include "Components/SpriteComponent.h"
#include "Components/CollisionComponent.h"
#include "Components/InputComponent.h"
#include "Components/AnimationComponent.h"
#include "Components/CameraComponent.h"
#include "Components/AudioListenerComponent.h"

using namespace Krampus;


// Container of Components but can have it's own logic
class CustomActor : public Actor
{
	SpriteComponent* sprite = nullptr;
	CollisionComponent* collision = nullptr;
	InputComponent* inputs = nullptr;
	AnimationComponent* animation = nullptr;
	CameraComponent* camera = nullptr;
	AudioListenerComponent* listener = nullptr;

	bool isRunning = false;

public:
	// The constructor must start his parametters with a Krampus::Level*
	CustomActor(Level* _level);

	// Called when the actor is registered
	virtual void Construct() override;
	// Called when the actor start playing
	virtual void BeginPlay() override;
	// Called every frames
	virtual void Tick(const float& _deltaTime) override;
	// Called when the actor is unregistered
	virtual void Deconstruct() override;
	// Called when the actor is destroyed
	virtual void BeginDestroy() override;

	void MoveLeft();
	void MoveRight();
	void Stop();

	virtual std::string ToString() const override
	{
		return "I am an CustomActor and i can be cast to a string";
	}
};


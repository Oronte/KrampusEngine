#pragma once
#include "Core/CoreMinimal.h"
#include "Components/Component.h"

// Reusable logic
class CustomComponent : public Krampus::Component
{
	Krampus::Timer* timer = nullptr;
	Krampus::Event<Krampus::Timer*> onComponentDestroyed;
	Krampus::Event<Krampus::Timer*>::ListenerHandle eventHandle;

public:
	// The constructor must start his parametters with a Krampus::Actor*
	CustomComponent(Krampus::Actor* _owner);

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

	void PrintTimer(Krampus::Timer* _timer);
};


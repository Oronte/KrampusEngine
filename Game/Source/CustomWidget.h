#pragma once
#include "UI/Widget.h"
#include "Components/LabelComponent.h"

class CustomWidget : public Krampus::Widget
{
	Krampus::LabelComponent* label = nullptr;
	float time = 0.0f;

public:
	// The constructor must start his parametters with a Krampus::Level*
	CustomWidget(Krampus::Level* _level, float _startTime);

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

private:
	void UpdateText();
};


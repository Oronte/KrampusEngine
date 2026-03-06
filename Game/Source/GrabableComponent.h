#pragma once
#include "Components/Component.h"

using namespace Krampus;

class GrabableComponent : public Component
{
	Actor* graber = nullptr;

public:
	void SetGraber(Actor* _graber)
	{
		graber = _graber;
	}

	GrabableComponent(Actor* _owner, Actor* _graber);

	virtual void Tick(const float& _deltaTime) override;
};


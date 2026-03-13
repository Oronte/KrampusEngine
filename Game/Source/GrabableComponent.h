#pragma once
#include "Components/Component.h"
#include "Player.h"

using namespace Krampus;

class GrabableComponent : public Component
{
	Player*	graber		= nullptr;
	Bool	canTrack	= true;

	Event<>::ListenerHandle handle;

public:
	void SetGraber(Player* _graber)
	{
		graber = _graber;
	}

	GrabableComponent(Actor* _owner, Player* _graber);

	virtual void Construct() override;
	virtual void Tick(const Float& _deltaTime) override;
};


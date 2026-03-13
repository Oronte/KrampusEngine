#pragma once
#include "Actors/Actor.h"
#include "GrabableComponent.h"

class WeaponSocket : public Krampus::Actor
{
	GrabableComponent*	grabable	= nullptr;

public:
	WeaponSocket(Krampus::Level* _level, Player* _grabber = nullptr);

	void SetGraber(Player* _grabber);
};
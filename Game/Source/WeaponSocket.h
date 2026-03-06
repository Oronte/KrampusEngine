#pragma once
#include "Actors/Actor.h"
#include "GrabableComponent.h"

class WeaponSocket : public Krampus::Actor
{
	GrabableComponent* grabable = nullptr;

public:
	WeaponSocket(Krampus::Level* _level, Krampus::Actor* _grabber = nullptr);

	void SetGraber(Krampus::Actor* _grabber);
};


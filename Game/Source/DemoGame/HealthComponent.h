#pragma once
#include "Components/Component.h"

using namespace Krampus;

class HealthComponent : public Component
{
	Int				health		= 100;
	Int				maxHealth	= 100;

public:
	Event<>			onDeath;
	Event<Int>	onHealthUpdate;

public:
	Int GetCurrentHealth() const
	{
		return health;
	}
	Int GetMaxHealth() const
	{
		return maxHealth;
	}

	HealthComponent(Actor* _owner, Int _health = 100);

	bool Damage(Int _damage);
};


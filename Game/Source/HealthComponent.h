#pragma once
#include "Components/Component.h"

using namespace Krampus;

class HealthComponent : public Component
{
	float health = 100.0f;

public:
	HealthComponent(Actor* _owner, float _health = 100.0f);

	void Damage(float _damage);
	void Die();
};


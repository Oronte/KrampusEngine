#include "HealthComponent.h"
#include "Actors/Actor.h"
#include "Enemy.h"
#include "GameFramework/Level.h"

HealthComponent::HealthComponent(Actor* _owner, float _health)
	: Component(_owner), health(_health)
{
}

bool HealthComponent::Damage(float _damage)
{
	health -= _damage;
	if (health <= 0.0f)
	{
		health = 0.0f;
		onDeath.Broadcast();
		return false;
	}
	return true;
}

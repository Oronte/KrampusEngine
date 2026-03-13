#include "HealthComponent.h"
#include "Actors/Actor.h"
#include "Enemy.h"
#include "GameFramework/Level.h"

HealthComponent::HealthComponent(Actor* _owner, Int _health)
	: Component(_owner), health(_health), maxHealth(_health)
{
}

bool HealthComponent::Damage(Int _damage)
{
	health -= _damage;
	health = IMath::MaxVal(health, 0);

	onHealthUpdate.Broadcast(health);

	if (health == 0.0f)
	{
		onDeath.Broadcast();
		return false;
	}
	return true;
}

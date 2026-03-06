#include "HealthComponent.h"
#include "Actors/Actor.h"
#include "Enemy.h"
#include "GameFramework/Level.h"

HealthComponent::HealthComponent(Actor* _owner, float _health)
	: Component(_owner), health(_health)
{
}

void HealthComponent::Damage(float _damage)
{
	health -= _damage;
	if (health <= 0.0f)
	{
		health = 0.0f;
		Die();
	}
}

void HealthComponent::Die()
{
	if (Enemy* _owner = Cast<Enemy>(owner))
	{
		_owner->Die();
	}
	else owner->Destroy();
}

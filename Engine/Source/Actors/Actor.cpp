#include "Actor.h"
#include "GameFramework/Level.h"
#include <chrono>
#include <random>

Krampus::Actor::Actor(Level* _level)
	: SceneObject(_level)
{
	static std::mt19937_64 _random(std::random_device{ }() ^ static_cast<std::uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count()));
	name = NAME_OF(Actor);
	do id = _random(); while (id == 0);	// 0 is never an actor
}

void Krampus::Actor::OnDestroy()
{
	level->GetActorManager().MarkForDeleteActor(this);
	ForgetReferencesTo(level, this);	// a reference to it is nullptr from now on, not a pointer to a deleted actor
}

void Krampus::Actor::OnComponentRemoved(Component* _component)
{
	ForgetReferencesTo(level, _component);
}

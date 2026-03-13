#include "Component.h"
#include "Utilities/Math/Transform.h"
#include "Actors/Actor.h"
#include "GameFramework/Level.h"

using namespace Krampus;

Engine* Component::GetWorld() const
{
	return level->GetWorld();
}

Float Krampus::Component::GetDeltaTime() const
{
	return owner->GetDeltaTime();
}

Component::Component(Actor* _owner)
	: transform(_owner->transform)
{
	name = NAME_OF(Component);
	owner = _owner;
	level = owner->GetLevel();
}

Krampus::Timer* Krampus::Component::CreateTimer(const std::function<void()>& _callback, const Float& _duration, const Bool& _isLoop, const Bool& _startRunning)
{
	return owner->CreateTimer(_callback, _duration, _isLoop, _startRunning);
}

void Component::Construct()
{
	SetActive(true);
}

void Component::Deconstruct()
{
	SetActive(false);
}

std::string Krampus::Component::ToString() const
{
	if (!owner) return "Component with no owner";
	return "Component : owner = " + owner->ToString();
}

#include "Component.h"
#include "Utilities/Math/Transform.h"
#include "Actors/Actor.h"
#include "GameFramework/Level.h"

using namespace Krampus;

Component::Component(Actor* _owner)
	: transform(_owner->transform)
{
	owner = _owner;
	level = owner->GetLevel();
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
	if (!owner) return "Component wath no owner";
	return "Component : owner = " + owner->ToString();
}

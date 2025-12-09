#include "Component.h"
#include "Utilities/Math/Transform.h"
#include "Actors/Actor.h"

using namespace Krampus;

Component::Component(Actor* _owner)
{
	owner = _owner;
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

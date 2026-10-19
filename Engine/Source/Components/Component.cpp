#include "Component.h"
#include "Actors/Actor.h"
#include "GameFramework/Level.h"

using namespace Krampus;

Level* Krampus::Component::GetLevel() const
{
	return owner->GetLevel();
}

Engine* Component::GetWorld() const
{
	return GetLevel()->GetWorld();
}

Float Krampus::Component::GetDeltaTime() const
{
	return owner->GetDeltaTime();
}

Component::Component(Actor* _owner)
{
	name = NAME_OF(Component);
	owner = _owner;
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

void Krampus::Component::SetPosition(const FVector2& _newPosition)
{
	GetOwner()->SetPosition(_newPosition);
}

FVector2 Krampus::Component::GetPosition() const
{
	return GetOwner()->GetPosition();
}

void Krampus::Component::SetRotation(const Angle& _newRotation)
{
	GetOwner()->SetRotation(_newRotation);
}

Angle Krampus::Component::GetRotation() const
{
	return GetOwner()->GetRotation();
}

void Krampus::Component::SetScale(const FVector2& _newScale)
{
	GetOwner()->SetScale(_newScale);
}

FVector2 Krampus::Component::GetScale() const
{
	return GetOwner()->GetScale();
}

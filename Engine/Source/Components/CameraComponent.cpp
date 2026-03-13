#include "CameraComponent.h"
#include "Actors/Actor.h"
#include "GameFramework/Level.h"

Krampus::CameraComponent::CameraComponent(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(CameraComponent);
	view = std::make_unique<sf::View>(transform.position, FVector2(level->GetWindowRef().GetSize()));
}

Krampus::CameraComponent::CameraComponent(Actor* _owner, const FVector2& _center, const FVector2& _size)
	: Component(_owner)
{
	name = NAME_OF(CameraComponent);
	view = std::make_unique<sf::View>(_center, _size);
}

Krampus::CameraComponent::CameraComponent(Actor* _owner, const FVector2& _size)
	: Component(_owner)
{
	name = NAME_OF(CameraComponent);
	view = std::make_unique<sf::View>(_size / 2.0f, _size);
}

Krampus::CameraComponent::CameraComponent(Actor* _owner, const FRect& _rect)
	: Component(_owner)
{
	name = NAME_OF(CameraComponent);
	view = std::make_unique<sf::View>(_rect);
}

void Krampus::CameraComponent::SetCurrent()
{
	level->GetCameraManagerRef().SetCurrent(this);
}

void Krampus::CameraComponent::Tick(const Float& _deltaTime)
{
	Component::Tick(_deltaTime);

	if (!attachedToOwner) return;
	
	if (!freezePostition) 
		SetCenter(transform.position);
	if (!freezeRotation)
		SetRotation(transform.rotation);
}

void Krampus::CameraComponent::BeginDestroy()
{
	Component::BeginDestroy();

	level->GetCameraManagerRef().SetCurrent(nullptr);
}

std::string Krampus::CameraComponent::ToString() const
{
	return name + " -> View Center = " + FVector2(view->getCenter()).ToString() + ", View Size = " + FVector2(view->getSize()).ToString();
}

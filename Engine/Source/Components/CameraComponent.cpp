#include "CameraComponent.h"
#include "Actors/Actor.h"
#include "GameFramework/Level.h"

Krampus::CameraComponent::CameraComponent(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(CameraComponent);
	view = std::make_unique<sf::View>(GetActorPosition(), FVector2(GetLevel()->GetWindowRef().GetSize()));

	InitEvent();
}

Krampus::CameraComponent::CameraComponent(Actor* _owner, const FVector2& _center, const FVector2& _size)
	: Component(_owner)
{
	name = NAME_OF(CameraComponent);
	view = std::make_unique<sf::View>(_center, _size);
	
	InitEvent();
}

Krampus::CameraComponent::CameraComponent(Actor* _owner, const FVector2& _size)
	: Component(_owner)
{
	name = NAME_OF(CameraComponent);
	view = std::make_unique<sf::View>(_size / 2.0f, _size);

	InitEvent();
}

Krampus::CameraComponent::CameraComponent(Actor* _owner, const FRect& _rect)
	: Component(_owner)
{
	name = NAME_OF(CameraComponent);
	view = std::make_unique<sf::View>(_rect);

	InitEvent();
}

void Krampus::CameraComponent::SetCurrent()
{
	GetLevel()->GetCameraManagerRef().SetCurrent(this);
}

void Krampus::CameraComponent::BeginDestroy()
{
	Component::BeginDestroy();

	GetLevel()->GetCameraManagerRef().SetCurrent(nullptr);
}

void Krampus::CameraComponent::InitEvent()
{
	onOwnerMoveHandle = GetOwner()->onMove.AddListener([this](FVector2 _newPos)
		{
			if (!attachedToOwner || freezePostition)
				return;
			SetCenter(_newPos);
		});
	onOwnerRotateHandle = GetOwner()->onRotate.AddListener([this](Angle _newRot)
		{
			if (!attachedToOwner || freezeRotation)
				return;
			SetRotation(_newRot);
		});
}

std::string Krampus::CameraComponent::ToString() const
{
	return name + " -> View Center = " + FVector2(view->getCenter()).ToString() + ", View Size = " + FVector2(view->getSize()).ToString();
}

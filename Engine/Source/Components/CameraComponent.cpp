#include "CameraComponent.h"
#include "Actors/Actor.h"
#include "GameFramework/Level.h"
#include "Utilities/Debug/Debug.h"

Krampus::CameraComponent::CameraComponent(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(CameraComponent);
	view = std::make_unique<sf::View>(GetPosition(), FVector2(GetLevel()->GetWindow().GetSize()));

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
	GetLevel()->GetCameraManager().SetCurrent(this);
}

void Krampus::CameraComponent::Construct()
{
	Component::Construct();
	if (attachedToOwner && !freezePostition)
		SetCenter(GetPosition());

	if (attachedToOwner && !freezeRotation)
		SetRotation(GetRotation());

}

void Krampus::CameraComponent::BeginDestroy()
{
	Component::BeginDestroy();

	// Only the camera in use lets go; destroying another one must not blank the view.
	if (GetLevel()->GetCameraManager().GetCurrent() == this)
		GetLevel()->GetCameraManager().SetCurrent(nullptr);
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

void Krampus::CameraComponent::DrawDebug()
{
#ifdef DEBUG
	// The area of the world this camera shows (yellow), with a cross at its center.
	const sf::Vector2f _center = view->getCenter();
	const sf::Vector2f _size = view->getSize();
	const FVector2 _middle(_center.x, _center.y);
	Debug::DrawDebugRect(GetLevel(), _middle, FVector2(_size.x, _size.y), Angle::FromDegrees(view->getRotation().asDegrees()), Color::Yellow());
	Debug::DrawDebugPoint(GetLevel(), _middle, 4.f, Color::Yellow());
#endif
}

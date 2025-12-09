#include "CameraComponent.h"
#include "Actors/Actor.h"
#include "GameFramework/Level.h"
#include "Graphics/Window/MainWindow.h"

Krampus::CameraComponent::CameraComponent(Actor* _owner)
	: Component(_owner)
{
	const FVector2& _windowSize = FVector2(MAIN_WINDOW.GetSize());
	view = std::make_unique<sf::View>(_windowSize / 2.0f, _windowSize);
}

Krampus::CameraComponent::CameraComponent(Actor* _owner, const FVector2& _center, const FVector2& _size)
	: Component(_owner)
{
	view = std::make_unique<sf::View>(_center, _size);
}

Krampus::CameraComponent::CameraComponent(Actor* _owner, const FVector2& _size)
	: Component(_owner)
{
	view = std::make_unique<sf::View>(_size / 2.0f, _size);
}

Krampus::CameraComponent::CameraComponent(Actor* _owner, const FRect& _rect)
	: Component(_owner)
{
	view = std::make_unique<sf::View>(_rect);
}
#include "AudioListenerComponent.h"

Krampus::AudioListenerComponent::AudioListenerComponent(Actor* _owner)
	: Component(_owner)
{
}

void Krampus::AudioListenerComponent::BeginPlay()
{
	Component::BeginPlay();

	UpdateListener();
}

void Krampus::AudioListenerComponent::Tick(const float& _deltaTime)
{
	Component::Tick(_deltaTime);

	UpdateListener();
}

void Krampus::AudioListenerComponent::UpdateListener()
{
	const FVector2& _position = transform.position;
	const FVector2& _forward = transform.Forward();
	const FVector2& _up = transform.Right();


	sf::Listener::setPosition(sf::Vector3f(_position.x, _position.y, 0.0f));
	sf::Listener::setDirection(sf::Vector3f(_forward.x, _forward.y, 0.0f));
	sf::Listener::setUpVector(sf::Vector3f(_up.x, _up.y, 0.0f));
	//sf::Listener::setVelocity(transform.GetVelocity());
}

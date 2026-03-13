#include "AudioListenerComponent.h"

Krampus::AudioListenerComponent::AudioListenerComponent(Actor* _owner, const Bool& _topView)
	: Component(_owner), topView(_topView)
{
	name = NAME_OF(AudioListenerComponent);
}

void Krampus::AudioListenerComponent::BeginPlay()
{
	Component::BeginPlay();

	UpdateListener();
}

void Krampus::AudioListenerComponent::Tick(const Float& _deltaTime)
{
	Component::Tick(_deltaTime);

	UpdateListener();
}

void Krampus::AudioListenerComponent::UpdateListener()
{
	const FVector2& _position = transform.position;
	FVector2 _forward;

	if (topView)
	{
		_forward = transform.Up();
		sf::Listener::setUpVector(sf::Vector3f(0.0f, 0.0f, 1.0f));
	}
	else
	{
		_forward = transform.Right();
		const FVector2& _up = transform.Up();
		sf::Listener::setUpVector(sf::Vector3f(_up.x, _up.y, 0.0f));
	}


	sf::Listener::setPosition(sf::Vector3f(_position.x, _position.y, 0.0f));
	sf::Listener::setDirection(sf::Vector3f(_forward.x, _forward.y, 0.0f));
	//sf::Listener::setVelocity(transform.GetVelocity());
}

std::string Krampus::AudioListenerComponent::ToString() const
{
	return name + " -> Is Top View = " + topView.ToString();
}

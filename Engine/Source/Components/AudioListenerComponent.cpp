#include "AudioListenerComponent.h"

Krampus::AudioListenerComponent::AudioListenerComponent(Actor* _owner, const Bool& _topView)
	: Component(_owner), topView(_topView)
{
	name = NAME_OF(AudioListenerComponent);
}

void Krampus::AudioListenerComponent::BeginPlay()
{
	Component::BeginPlay();

	dirty = true;
	UpdateListener();
}

void Krampus::AudioListenerComponent::Tick(const Float& _deltaTime)
{
	Component::Tick(_deltaTime);

	if (active)
		UpdateListener();
}

void Krampus::AudioListenerComponent::SetActive(const Bool& _active)
{
	if (active == _active)
		return;

	active = _active;

	if (!active)
	{
		sf::Listener::setGlobalVolume(0.0f);
	}
	else
	{
		sf::Listener::setGlobalVolume(100.0f);
		dirty = true;
		UpdateListener();
	}
}

Bool Krampus::AudioListenerComponent::HasChanged(
	const FVector2& _position,
	const FVector2& _forward,
	const FVector2& _up) const
{
	return	_position	!= cachedPosition	||
			_forward	!= cachedForward	||
			_up			!= cachedUp;
}

void Krampus::AudioListenerComponent::UpdateListener()
{
	const FVector2 _position = GetActorPosition();
	const FVector2 _right = GetActorRightVector();
	const FVector2 _up = GetActorUpVector();


	FVector2 _forward;
	sf::Vector3f _sfUp;

	if (topView)
	{
		_forward	= _up;
		_sfUp		= sf::Vector3f(0.0f, 0.0f, 1.0f);
	}
	else
	{
		_forward	= _right;
		_sfUp		= sf::Vector3f(_up.x, 1.0f, _up.y);
	}

	if (!dirty && !HasChanged(_position, _forward, _up))
		return;

	sf::Listener::setPosition (sf::Vector3f(_position.x, 0.0f, _position.y));
	sf::Listener::setDirection (sf::Vector3f(_forward.x,  0.0f, _forward.y));
	sf::Listener::setUpVector  (_sfUp);

	FVector2 _dir = _position - cachedPosition;
	sf::Listener::setVelocity(sf::Vector3f(_dir.x, 0.0f, _dir.y));

	cachedPosition	= _position;
	cachedForward	= _forward;
	cachedUp		= _up;
	dirty			= false;
}

std::string Krampus::AudioListenerComponent::ToString() const
{
	return	name
		+	" -> Is Top View = "	+ topView.ToString()
		+	" | Active = "			+ active.ToString()
		+	" | Volume = "			+ std::to_string(sf::Listener::getGlobalVolume());
}

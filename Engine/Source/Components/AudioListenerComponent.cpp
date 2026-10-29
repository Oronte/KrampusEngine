#include "AudioListenerComponent.h"
#include "GameFramework/Level.h"
#include "Utilities/Debug/Debug.h"

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
	const FVector2 _position = GetPosition();
	const FVector2 _right = GetRightVector();
	const FVector2 _up = GetUpVector();


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

void Krampus::AudioListenerComponent::DrawDebug()
{
#ifdef DEBUG
	// Where the listener hears from: a line along its direction, the inner cone (green: full volume) and the outer cone (yellow: reduced to outerGain).
	const FVector2 _position = GetPosition();
	const FVector2 _forward = (topView ? GetUpVector() : GetRightVector()).Normalized();
	const sf::Listener::Cone _cone = sf::Listener::getCone();
	constexpr float _length = 220.0f;

	const auto _edge = [&](const float& _degrees, const Color& _color)
	{
		const FVector2 _end = (_position + _forward * _length).RotateAround(_position, _degrees * FMath::pi / 180.0f);
		Debug::DrawDebugLine(GetLevel(), _position, (_end - _position).Normalized(), _length, 2.0f, _color);
	};

	Debug::DrawDebugCircle(GetLevel(), _position, 16.0f, 15, Color::Cyan());
	Debug::DrawDebugLine(GetLevel(), _position, _forward, _length, 2.0f, Color::Cyan());
	const float _inner = _cone.innerAngle.asDegrees() * 0.5f;
	const float _outer = _cone.outerAngle.asDegrees() * 0.5f;
	_edge(_inner, Color::Green());
	_edge(-_inner, Color::Green());
	_edge(_outer, Color::Yellow());
	_edge(-_outer, Color::Yellow());
#endif
}

#include "InputComponent.h"

Krampus::InputComponent::InputComponent(Actor* _owner)
	: Component(_owner) { }

void Krampus::InputComponent::Bind(Event<>* _event, std::function<void()> _callback, bool _once)
{
	if (!_event)
	{
		LOG_ERROR("The event that you trying to bind is null");
		return;
	}

	handlesVoid.push_back(_event->AddListener(_callback, _once));
}

void Krampus::InputComponent::Bind(Event<float>* _event, std::function<void(float)> _callback, bool _once)
{
	if (!_event)
	{
		LOG_ERROR("The event that you trying to bind is null");
		return;
	}

	handlesFloat.push_back(_event->AddListener(_callback, _once));
}

void Krampus::InputComponent::Bind(Event<FVector2>* _event, std::function<void(FVector2)> _callback, bool _once)
{
	if (!_event)
	{
		LOG_ERROR("The event that you trying to bind is null");
		return;
	}

	handlesFVector2.push_back(_event->AddListener(_callback, _once));
}

void Krampus::InputComponent::Bind(Event<IVector2>* _event, std::function<void(IVector2)> _callback, bool _once)
{
	if (!_event)
	{
		LOG_ERROR("The event that you trying to bind is null");
		return;
	}

	handlesIVector2.push_back(_event->AddListener(_callback, _once));
}

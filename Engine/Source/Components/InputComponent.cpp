#include "InputComponent.h"

Krampus::InputComponent::InputComponent(Actor* _owner)
	: Component(_owner) { }

std::string Krampus::InputComponent::ToString() const
{
	const Int& _totalCount = handlesVoid.size() + handlesFloat.size() + handlesFVector2.size() + handlesIVector2.size();
	return name + " -> Binded Callback Count = " + _totalCount.ToString();
}

void Krampus::InputComponent::Bind(Event<>* _event, std::function<void()> _callback, const Bool& _once)
{
	if (!_event)
	{
		LOG_ERROR("The event that you trying to bind is null");
		return;
	}

	handlesVoid.push_back(_event->AddListener(_callback, _once));
}

void Krampus::InputComponent::Bind(Event<Float>* _event, std::function<void(Float)> _callback, const Bool& _once)
{
	if (!_event)
	{
		LOG_ERROR("The event that you trying to bind is null");
		return;
	}

	handlesFloat.push_back(_event->AddListener(_callback, _once));
}

void Krampus::InputComponent::Bind(Event<FVector2>* _event, std::function<void(FVector2)> _callback, const Bool& _once)
{
	if (!_event)
	{
		LOG_ERROR("The event that you trying to bind is null");
		return;
	}

	handlesFVector2.push_back(_event->AddListener(_callback, _once));
}

void Krampus::InputComponent::Bind(Event<IVector2>* _event, std::function<void(IVector2)> _callback, const Bool& _once)
{
	if (!_event)
	{
		LOG_ERROR("The event that you trying to bind is null");
		return;
	}

	handlesIVector2.push_back(_event->AddListener(_callback, _once));
}

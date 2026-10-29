#include "InputComponent.h"
#include "Managers/InputManager.h"

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

	handlesVoid.push_back(_event->AddListener([this, _callback]() { if (enabled) _callback(); }, ListenerOptions{.once = _once}));
}

void Krampus::InputComponent::Bind(Event<Float>* _event, std::function<void(Float)> _callback, const Bool& _once)
{
	if (!_event)
	{
		LOG_ERROR("The event that you trying to bind is null");
		return;
	}

	handlesFloat.push_back(_event->AddListener([this, _callback](Float _value) { if (enabled) _callback(_value); }, ListenerOptions{.once = _once}));
}

void Krampus::InputComponent::Bind(Event<FVector2>* _event, std::function<void(FVector2)> _callback, const Bool& _once)
{
	if (!_event)
	{
		LOG_ERROR("The event that you trying to bind is null");
		return;
	}

	handlesFVector2.push_back(_event->AddListener([this, _callback](FVector2 _value) { if (enabled) _callback(_value); }, ListenerOptions{.once = _once}));
}

void Krampus::InputComponent::Bind(Event<IVector2>* _event, std::function<void(IVector2)> _callback, const Bool& _once)
{
	if (!_event)
	{
		LOG_ERROR("The event that you trying to bind is null");
		return;
	}

	handlesIVector2.push_back(_event->AddListener([this, _callback](IVector2 _value) { if (enabled) _callback(_value); }, ListenerOptions{.once = _once}));
}

void Krampus::InputComponent::ApplyBindings()
{
	bindingHandles.clear();

	InputManager* _input = GetWorld()->GetInputManager();
	for (const Binding& _binding : bindings)
	{
		// Copy of the list: the lambda must not depend on the vector, which can change.
		const auto _call = [this, _calls = _binding.calls]() { if (enabled) _calls.Invoke(this); };

		if (Event<>* _event = _input->FindEvent(_binding.source))
			bindingHandles.push_back(_event->AddListener(_call));
		else if (Event<Float>* _floatEvent = _input->FindFloatEvent(_binding.source))
			bindingHandles.push_back(_floatEvent->AddListener([_call](Float) { _call(); }));
		else if (Event<FVector2>* _vectorEvent = _input->FindVectorEvent(_binding.source))
			bindingHandles.push_back(_vectorEvent->AddListener([_call](FVector2) { _call(); }));
		else if (Event<IVector2>* _pointEvent = _input->FindPointEvent(_binding.source))
			bindingHandles.push_back(_pointEvent->AddListener([_call](IVector2) { _call(); }));
		else
			LOG_WARNING("InputComponent: there is no input event named \"" + _binding.source + "\"");
	}
}

// One binding per line: source '=' EventList (the list never contains a line break).
std::string Krampus::InputComponent::EncodeBindings(const std::vector<Binding>& _bindings)
{
	std::string _text;
	for (const Binding& _binding : _bindings)
	{
		if (!_text.empty()) _text += '\n';
		_text += _binding.source + "=" + _binding.calls.Encode();
	}
	return _text;
}

std::vector<Krampus::InputComponent::Binding> Krampus::InputComponent::DecodeBindings(const std::string& _text)
{
	std::vector<Binding> _bindings;
	size_t _start = 0;
	while (_start < _text.size())
	{
		size_t _end = _text.find('\n', _start);
		if (_end == std::string::npos) _end = _text.size();

		const std::string _line = _text.substr(_start, _end - _start);
		if (const size_t _equal = _line.find('='); _equal != std::string::npos)
			_bindings.push_back(Binding{ _line.substr(0, _equal), EventList::Decode(_line.substr(_equal + 1)) });
		_start = _end + 1;
	}
	return _bindings;
}

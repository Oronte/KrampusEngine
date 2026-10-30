#include "BlueprintComponent.h"
#include "Actors/Actor.h"
#include "Core/Engine.h"
#include "Managers/InputManager.h"

using namespace Krampus;

BlueprintComponent::BlueprintComponent(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(BlueprintComponent);
}

void BlueprintComponent::BeginPlay()
{
	Component::BeginPlay();
	Start();
}

void BlueprintComponent::Start()
{
	StopListening();
	started = false;
	if (blueprint.empty()) return;

	const std::shared_ptr<const BlueprintAsset> _asset = BlueprintAsset::Load(blueprint);
	if (!_asset) return;
	if (!instance.Compile(_asset, GetOwner(), blueprint)) return;

	started = true;
	ListenToInputs();
	instance.FireEvent("Event.BeginPlay");
}

void BlueprintComponent::ListenToInputs()
{
	InputManager* _inputs = GetWorld() ? GetWorld()->GetInputManager() : nullptr;
	if (!_inputs) return;

	// Event Input nodes: each name once.
	std::vector<std::string> _events = instance.InputEvents();
	std::sort(_events.begin(), _events.end());
	_events.erase(std::unique(_events.begin(), _events.end()), _events.end());
	for (const std::string& _name : _events)
	{
		Event<>* _event = _inputs->FindEvent(_name);
		if (!_event)
		{
			LOG_WARNING("Blueprint " + blueprint + ": no input \"" + _name + "\" (an input is written like Space.Press, A.Release, MouseLeft.Press)");
			continue;
		}
		inputHandles.push_back(_event->AddListener([this, _name]()
		{
			if (IsActive() && !GetWorld()->IsLevelPaused()) instance.FireEvent("Event.Input", _name);
		}));
	}

	// Is Input Held nodes: their key is followed through its Press and Release events.
	for (const std::string& _name : instance.InputsHeldChecks())
	{
		Event<>* _press = _inputs->FindEvent(_name + ".Press");
		Event<>* _release = _inputs->FindEvent(_name + ".Release");
		if (!_press || !_release)
		{
			LOG_WARNING("Blueprint " + blueprint + ": Is Input Held: no input \"" + _name + "\" (Space, A, MouseLeft...)");
			continue;
		}
		instance.held[_name] = false;
		inputHandles.push_back(_press->AddListener([this, _name]() { instance.held[_name] = true; }));
		inputHandles.push_back(_release->AddListener([this, _name]() { instance.held[_name] = false; }));
	}
}

void BlueprintComponent::StopListening()
{
	for (Event<>::ListenerHandle& _handle : inputHandles) _handle.Disconnect();
	inputHandles.clear();
}

void BlueprintComponent::Tick(const Float& _deltaTime)
{
	Component::Tick(_deltaTime);
	if (started) instance.Tick(static_cast<float>(_deltaTime));
}

void BlueprintComponent::Deconstruct()
{
	StopListening();	// the input events outlive the component
	Component::Deconstruct();
}

void BlueprintComponent::CallEvent(const std::string& _name)
{
	if (started) instance.FireEvent("Event.Custom", _name);
}

std::string BlueprintComponent::ToString() const
{
	return name + " -> " + (blueprint.empty() ? std::string("(no blueprint)") : blueprint) + (IsRunning() ? " (running)" : "");
}

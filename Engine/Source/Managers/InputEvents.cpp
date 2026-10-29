#include "InputManager.h"

// The events of the InputManager by name, so the editor and scenes can bind functions to them (InputComponent).
// Every key / button gives three events: "A.Press", "A.Hold" (every frame while held) and "A.Release".

void Krampus::InputManager::BuildEventTable()
{
	if (!eventNames.empty()) return;

	const auto _addInput = [this](const std::string& _name, Input& _input)
	{
		voidEvents[_name + ".Press"] = &_input.onPress;
		voidEvents[_name + ".Hold"] = &_input.onPerform;
		voidEvents[_name + ".Release"] = &_input.onRelease;
		for (const char* _suffix : { ".Press", ".Hold", ".Release" })
			eventNames.push_back(_name + _suffix);
	};
	const auto _addVoid = [this](const std::string& _name, Event<>& _event) { voidEvents[_name] = &_event; eventNames.push_back(_name); };
	const auto _addFloat = [this](const std::string& _name, Event<Float>& _event) { floatEvents[_name] = &_event; eventNames.push_back(_name); };
	const auto _addVector = [this](const std::string& _name, Event<FVector2>& _event) { vectorEvents[_name] = &_event; eventNames.push_back(_name); };
	const auto _addPoint = [this](const std::string& _name, Event<IVector2>& _event) { pointEvents[_name] = &_event; eventNames.push_back(_name); };

#define KRAMPUS_KEY(Name, SfmlKey) _addInput(#Name, Name);
#define KRAMPUS_MOUSE_BUTTON(Name, SfmlName) _addInput(#Name, Name);
#define KRAMPUS_PAD_BUTTON(Name) _addInput(#Name, Name);
#include "InputKeys.def"

	_addPoint("MouseMovedWorld", MouseMovedWorld);
	_addPoint("MouseMovedScreen", MouseMovedScreen);
	_addFloat("MouseWheelScroll", MouseWheelScroll);
	_addVoid("FocusGain", FocusGain);
	_addVoid("FocusLost", FocusLost);
	_addVoid("MouseEntered", MouseEntred);
	_addVoid("MouseLeft", MouseLeft);
	_addVector("LeftJoystick", LeftJoystick);
	_addVector("RightJoystick", RightJoystick);
	_addFloat("BackTriggerLeft", BackTriggerLeft);
	_addFloat("BackTriggerRight", BackTriggerRight);
	_addVector("DPad", DPad);
	_addPoint("WindowResize", WindowResize);
	_addVoid("WindowClose", WindowClose);
}

const std::vector<std::string>& Krampus::InputManager::GetEventNames()
{
	BuildEventTable();
	return eventNames;
}

Krampus::Event<>* Krampus::InputManager::FindEvent(const std::string& _name)
{
	BuildEventTable();
	const auto _found = voidEvents.find(_name);
	return _found == voidEvents.end() ? nullptr : _found->second;
}

auto Krampus::InputManager::FindFloatEvent(const std::string& _name) -> Event<Float>*
{
	BuildEventTable();
	const auto _found = floatEvents.find(_name);
	return _found == floatEvents.end() ? nullptr : _found->second;
}

auto Krampus::InputManager::FindVectorEvent(const std::string& _name) -> Event<FVector2>*
{
	BuildEventTable();
	const auto _found = vectorEvents.find(_name);
	return _found == vectorEvents.end() ? nullptr : _found->second;
}

auto Krampus::InputManager::FindPointEvent(const std::string& _name) -> Event<IVector2>*
{
	BuildEventTable();
	const auto _found = pointEvents.find(_name);
	return _found == pointEvents.end() ? nullptr : _found->second;
}

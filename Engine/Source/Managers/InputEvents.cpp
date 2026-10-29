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

#define KINPUT(Name) _addInput(#Name, Name);
	KINPUT(A) KINPUT(B) KINPUT(C) KINPUT(D) KINPUT(E) KINPUT(F) KINPUT(G) KINPUT(H) KINPUT(I) KINPUT(J) KINPUT(K) KINPUT(L) KINPUT(M)
	KINPUT(N) KINPUT(O) KINPUT(P) KINPUT(Q) KINPUT(R) KINPUT(S) KINPUT(T) KINPUT(U) KINPUT(V) KINPUT(W) KINPUT(X) KINPUT(Y) KINPUT(Z)
	KINPUT(Num0) KINPUT(Num1) KINPUT(Num2) KINPUT(Num3) KINPUT(Num4) KINPUT(Num5) KINPUT(Num6) KINPUT(Num7) KINPUT(Num8) KINPUT(Num9)
	KINPUT(Escape) KINPUT(LControl) KINPUT(LShift) KINPUT(LAlt) KINPUT(LSystem) KINPUT(RControl) KINPUT(RShift) KINPUT(RAlt) KINPUT(RSystem) KINPUT(Menu)
	KINPUT(LBracket) KINPUT(RBracket) KINPUT(Semicolon) KINPUT(Comma) KINPUT(Period) KINPUT(Quote) KINPUT(Slash) KINPUT(Backslash) KINPUT(Equal) KINPUT(Hyphen)
	KINPUT(Space) KINPUT(Enter) KINPUT(Backspace) KINPUT(Tab) KINPUT(PageUp) KINPUT(PageDown) KINPUT(End) KINPUT(Home) KINPUT(Insert) KINPUT(Delete)
	KINPUT(F1) KINPUT(F2) KINPUT(F3) KINPUT(F4) KINPUT(F5) KINPUT(F6) KINPUT(F7) KINPUT(F8) KINPUT(F9) KINPUT(F10) KINPUT(F11) KINPUT(F12) KINPUT(F13) KINPUT(F14) KINPUT(F15)
	KINPUT(Pause)
	KINPUT(MouseLeftClick) KINPUT(MouseRightClick) KINPUT(MouseMiddleClick) KINPUT(MouseExtraButton1) KINPUT(MouseExtraButton2)
	KINPUT(ButtonA) KINPUT(ButtonB) KINPUT(ButtonX) KINPUT(ButtonY) KINPUT(ButtonStart) KINPUT(ButtonBack) KINPUT(ButtonLB) KINPUT(ButtonRB) KINPUT(ButtonLS) KINPUT(ButtonRS)
#undef KINPUT

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

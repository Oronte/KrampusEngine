#pragma once
#include "Core/Engine.h"

#define M_INPUT Krampus::InputManager::GetInstance()

namespace Krampus
{

	// TODO make std::unordored_map with automatic creation
	class InputManager : public Singleton<InputManager>
	{
		using Key = sf::Keyboard::Key;
		using MouseButton = sf::Mouse::Button;
		using MouseWheel = sf::Mouse::Wheel;
		using JoystickAxis = sf::Joystick::Axis;

		struct Input
		{
			Event<> onPress;
			Event<> onPerform; // Called every framewhile holding
			Event<> onRelease;

		private:
			bool isPressed = false;

			void Press()
			{
				if (isPressed) return;
				onPress.Broadcast();
				isPressed = true;
			}
			void Release()
			{
				if (!isPressed) return;
				onRelease.Broadcast();
				isPressed = false;
			}
			void Update()
			{
				if (isPressed) onPerform.Broadcast();
			}
			friend class InputManager;
		};

	public:
		// KEYBOARD
		Input A;
		Input B;
		Input C;
		Input D;
		Input E;
		Input F;
		Input G;
		Input H;
		Input I;
		Input J;
		Input K;
		Input L;
		Input M;
		Input N;
		Input O;
		Input P;
		Input Q;
		Input R;
		Input S;
		Input T;
		Input U;
		Input V;
		Input W;
		Input X;
		Input Y;
		Input Z;
		Input Num0;
		Input Num1;
		Input Num2;
		Input Num3;
		Input Num4;
		Input Num5;
		Input Num6;
		Input Num7;
		Input Num8;
		Input Num9;
		Input Escape;
		Input LControl;
		Input LShift;
		Input LAlt;
		Input LSystem;
		Input RControl;
		Input RShift;
		Input RAlt;
		Input RSystem;
		Input Menu;
		Input LBracket;
		Input RBracket;
		Input Semicolon;
		Input Comma;
		Input Period;
		Input Quote;
		Input Slash;
		Input Backslash;
		//Input Tilde;
		Input Equal;
		Input Hyphen;
		Input Space;
		Input Enter;
		Input Backspace;
		Input Tab;
		Input PageUp;
		Input PageDown;
		Input End;
		Input Home;
		Input Insert;
		Input Delete;
		Input F1;
		Input F2;
		Input F3;
		Input F4;
		Input F5;
		Input F6;
		Input F7;
		Input F8;
		Input F9;
		Input F10;
		Input F11;
		Input F12;
		Input F13;
		Input F14;
		Input F15;
		Input Pause;

		// MOUSE BUTTONS
		Input MouseLeftClick;
		Input MouseRightClick;
		Input MouseMiddleClick;
		Input MouseExtraButton1;
		Input MouseExtraButton2;

		// MOUSE MOVEMENT 
		Event<IVector2> MouseMoved;

		// MOUSE WHEEL 
		Event<float> MouseWheelScroll;      // 1 / -1 for each scroll

		// OTHER MOUSE EVENTS
		Event<> FocusGain;
		Event<> FocusLost;
		Event<> MouseEntred;
		Event<> MouseLeft;

		// JOYSTICK / GAMEPAD 
		Event<FVector2> LeftJoystick;       // [-100 ; 100]
		Event<FVector2> RightJoystick;      // [-100 ; 100]
		Event<float> BackTriggerLeft;       // LT [0 ; 100]
		Event<float> BackTriggerRight;      // RT [0 ; 100]
		Event<FVector2> DPad;				// [-100 ; 100]

		Input ButtonA;
		Input ButtonB;
		Input ButtonX;
		Input ButtonY;
		Input ButtonStart;
		Input ButtonBack;
		Input ButtonLB;
		Input ButtonRB;
		Input ButtonLS;
		Input ButtonRS;

		// Other Events
		Event<IVector2> WindowResize;
		Event<> WindowClose;

		InputManager() = default;

	private:
		void Update(const std::optional<sf::Event>& _event);
		void UpdateSystemEvent(const std::optional<sf::Event>& _event);
		void UpdateKey(Input& _input, const Key& _key);
		void UpdateMouseButton(Input& _input, const MouseButton& _mouseButton);

		friend class Engine;
	};

}


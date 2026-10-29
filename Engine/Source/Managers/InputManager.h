#pragma once
#include "Core/KrampusObject.h"

namespace Krampus
{

	/// <summary>
	/// Centralized input processing system mapping keyboard, mouse, and game controller hardware events to engine events.
	/// </summary>
	class InputManager : public KrampusObject
	{
		using Key			= sf::Keyboard::Key;
		using MouseButton	= sf::Mouse::Button;
		using MouseWheel	= sf::Mouse::Wheel;
		using JoystickAxis	= sf::Joystick::Axis;

	public:
		/// <summary>
		/// Structure encapsulating press, continuous hold, and release events for an individual button or key.
		/// </summary>
		struct Input
		{
			/// <summary>Broadcast when key or button is first pressed down.</summary>
			Event<>			onPress;
			/// <summary>Broadcast every frame while key or button is continuously held.</summary>
			Event<>			onPerform; // Called every framewhile holding
			/// <summary>Broadcast when key or button is released.</summary>
			Event<>			onRelease;

		private:
			Bool			isPressed	= false;

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
		Event<IVector2> MouseMovedWorld;
		Event<IVector2> MouseMovedScreen;

		// MOUSE WHEEL 
		Event<Float> MouseWheelScroll;      // 1 / -1 for each scroll

		// OTHER MOUSE EVENTS
		Event<> FocusGain;
		Event<> FocusLost;
		Event<> MouseEntred;
		Event<> MouseLeft;

		// JOYSTICK / GAMEPAD 
		Event<FVector2> LeftJoystick;       // [-100 ; 100]
		Event<FVector2> RightJoystick;      // [-100 ; 100]
		Event<Float> BackTriggerLeft;       // LT [0 ; 100]
		Event<Float> BackTriggerRight;      // RT [0 ; 100]
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

		/// <summary>
		/// Names of every event ("A.Press", "A.Hold", "A.Release", "MouseWheelScroll", "FocusGain"...), for the editor.
		/// </summary>
		const std::vector<std::string>& GetEventNames();

		/// <summary>The event with this name, or nullptr if there is none of this kind (the kind depends on the name: see FindFloatEvent...).</summary>
		Event<>* FindEvent(const std::string& _name);
		Event<Float>* FindFloatEvent(const std::string& _name);
		Event<FVector2>* FindVectorEvent(const std::string& _name);
		Event<IVector2>* FindPointEvent(const std::string& _name);

		/// <summary>
		/// Constructs the InputManager associated with the engine world.
		/// </summary>
		/// <param name="_engine">Pointer to Engine world.</param>
		InputManager(Engine* _engine)
			: KrampusObject(_engine) { }

	private:
		std::vector<std::string>						eventNames;
		std::unordered_map<std::string, Event<>*>		voidEvents;
		std::unordered_map<std::string, Event<Float>*>	floatEvents;
		std::unordered_map<std::string, Event<FVector2>*>	vectorEvents;
		std::unordered_map<std::string, Event<IVector2>*>	pointEvents;

		void BuildEventTable();
		void Update(const std::optional<sf::Event>& _event);
		void UpdateOnPerform();
		void UpdateSystemEvent(const std::optional<sf::Event>& _event);
		void UpdateKey(Input& _input, const Key& _key);
		void UpdateMouseButton(Input& _input, const MouseButton& _mouseButton);

		friend class Engine;
	};

}

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
		// Every key, mouse button and gamepad button is an Input (onPress, onPerform, onRelease): A..Z, Num0..Num9, Space, Enter, F1..F15,
		// MouseLeftClick, ButtonA... The list is in InputKeys.def, shared with the code that polls them and the one that names them for the editor.
#define KRAMPUS_KEY(Name, SfmlKey) Input Name;
#define KRAMPUS_MOUSE_BUTTON(Name, SfmlName) Input Name;
#define KRAMPUS_PAD_BUTTON(Name) Input Name;
#include "InputKeys.def"

		// MOUSE MOVEMENT 
		Event<IVector2> MouseMovedWorld;
		Event<IVector2> MouseMovedScreen;

		// MOUSE WHEEL 
		Event<Float> MouseWheelScroll;      // 1 / -1 for each scroll

		// TEXT
		Event<std::uint32_t> TextEntered;   // unicode of each character typed (8 = Backspace, 13 = Enter)

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

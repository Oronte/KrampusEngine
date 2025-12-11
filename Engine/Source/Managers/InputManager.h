#pragma once
#include "Core/Engine.h"

#define M_INPUT Krampus::InputManager::GetInstance()

namespace Krampus
{
	class Window;

	struct Input
	{
		Event<> onPerform;
		Event<> onPerform;
		bool isPressed = false;
	};

	class InputManager : public Singleton<InputManager>
	{
	public:
		// ==================== KEYBOARD ====================
		Event<> A;
		Event<> B;
		Event<> C;
		Event<> D;
		Event<> E;
		Event<> F;
		Event<> G;
		Event<> H;
		Event<> I;
		Event<> J;
		Event<> K;
		Event<> L;
		Event<> M;
		Event<> N;
		Event<> O;
		Event<> P;
		Event<> Q;
		Event<> R;
		Event<> S;
		Event<> T;
		Event<> U;
		Event<> V;
		Event<> W;
		Event<> X;
		Event<> Y;
		Event<> Z;
		Event<> Num0;
		Event<> Num1;
		Event<> Num2;
		Event<> Num3;
		Event<> Num4;
		Event<> Num5;
		Event<> Num6;
		Event<> Num7;
		Event<> Num8;
		Event<> Num9;
		Event<> Escape;
		Event<> LControl;
		Event<> LShift;
		Event<> LAlt;
		Event<> LSystem;
		Event<> RControl;
		Event<> RShift;
		Event<> RAlt;
		Event<> RSystem;
		Event<> Menu;
		Event<> LBracket;
		Event<> RBracket;
		Event<> Semicolon;
		Event<> Comma;
		Event<> Period;
		Event<> Quote;
		Event<> Slash;
		Event<> Backslash;
		Event<> Tilde;
		Event<> Equal;
		Event<> Hyphen;
		Event<> Space;
		Event<> Enter;
		Event<> Backspace;
		Event<> Tab;
		Event<> PageUp;
		Event<> PageDown;
		Event<> End;
		Event<> Home;
		Event<> Insert;
		Event<> Delete;
		Event<> F1;
		Event<> F2;
		Event<> F3;
		Event<> F4;
		Event<> F5;
		Event<> F6;
		Event<> F7;
		Event<> F8;
		Event<> F9;
		Event<> F10;
		Event<> F11;
		Event<> F12;
		Event<> F13;
		Event<> F14;
		Event<> F15;
		Event<> Pause;

		// ==================== MOUSE BUTTONS ====================
		Event<> MouseLeftClick;
		Event<> MouseRightClick;
		Event<> MouseMiddleClick;
		Event<> MouseExtraButton1;
		Event<> MouseExtraButton2;

		// ==================== MOUSE MOVEMENT ====================
		Event<FVector2> MouseMoved;
		Event<FVector2> MouseDelta;

		// ==================== MOUSE WHEEL ====================
		Event<float> MouseWheelVertical;
		Event<float> MouseWheelHorizontal;

		// ==================== JOYSTICK / GAMEPAD ====================
		Event<float> LeftJoystickX;          // [-100 ; 100]
		Event<float> LeftJoystickY;          // [-100 ; 100]
		Event<float> RightJoystickX;         // [-100 ; 100]
		Event<float> RightJoystickY;         // [-100 ; 100]
		Event<float> BackTriggerLeft;        // LT [0 ; 100]
		Event<float> BackTriggerRight;       // RT [0 ; 100]
		Event<float> DPadX;                  // [-100 ; 100]
		Event<float> DPadY;                  // [-100 ; 100]

		Event<> ButtonA;
		Event<> ButtonB;
		Event<> ButtonX;
		Event<> ButtonY;
		Event<> ButtonStart;
		Event<> ButtonBack;
		Event<> ButtonLB;
		Event<> ButtonRB;
		Event<> ButtonLS;
		Event<> ButtonRS;

		InputManager() = default;

	private:
		void Update(Window& _window);

		friend void Engine::Update();
	};
}


#include "InputActorExemple.h"
#include "Core/Engine.h"
#include "Managers/InputManager.h"

Krampus::Exemple::InputActorExemple::InputActorExemple(Level* _level)
	: Actor(_level)
{
	inputs = CreateComponent<InputComponent>();
}

void Krampus::Exemple::InputActorExemple::Construct()
{
	Super::Construct();

	InputManager* _inputManager = GetWorld()->GetInputManager();

	// KEYBOARD
	inputs->Bind(&_inputManager->A.onPress, [this]() { LOG_MSG("A Press"); });
	inputs->Bind(&_inputManager->A.onPerform, [this]() { LOG_MSG("A Hold"); });
	inputs->Bind(&_inputManager->A.onRelease, [this]() { LOG_MSG("A Release"); });

	inputs->Bind(&_inputManager->B.onPress, [this]() { LOG_MSG("B Press"); });
	inputs->Bind(&_inputManager->B.onPerform, [this]() { LOG_MSG("B Hold"); });
	inputs->Bind(&_inputManager->B.onRelease, [this]() { LOG_MSG("B Release"); });

	inputs->Bind(&_inputManager->C.onPress, [this]() { LOG_MSG("C Press"); });
	inputs->Bind(&_inputManager->C.onPerform, [this]() { LOG_MSG("C Hold"); });
	inputs->Bind(&_inputManager->C.onRelease, [this]() { LOG_MSG("C Release"); });

	inputs->Bind(&_inputManager->D.onPress, [this]() { LOG_MSG("D Press"); });
	inputs->Bind(&_inputManager->D.onPerform, [this]() { LOG_MSG("D Hold"); });
	inputs->Bind(&_inputManager->D.onRelease, [this]() { LOG_MSG("D Release"); });

	inputs->Bind(&_inputManager->E.onPress, [this]() { LOG_MSG("E Press"); });
	inputs->Bind(&_inputManager->E.onPerform, [this]() { LOG_MSG("E Hold"); });
	inputs->Bind(&_inputManager->E.onRelease, [this]() { LOG_MSG("E Release"); });

	inputs->Bind(&_inputManager->F.onPress, [this]() { LOG_MSG("F Press"); });
	inputs->Bind(&_inputManager->F.onPerform, [this]() { LOG_MSG("F Hold"); });
	inputs->Bind(&_inputManager->F.onRelease, [this]() { LOG_MSG("F Release"); });

	inputs->Bind(&_inputManager->G.onPress, [this]() { LOG_MSG("G Press"); });
	inputs->Bind(&_inputManager->G.onPerform, [this]() { LOG_MSG("G Hold"); });
	inputs->Bind(&_inputManager->G.onRelease, [this]() { LOG_MSG("G Release"); });

	inputs->Bind(&_inputManager->H.onPress, [this]() { LOG_MSG("H Press"); });
	inputs->Bind(&_inputManager->H.onPerform, [this]() { LOG_MSG("H Hold"); });
	inputs->Bind(&_inputManager->H.onRelease, [this]() { LOG_MSG("H Release"); });

	inputs->Bind(&_inputManager->I.onPress, [this]() { LOG_MSG("I Press"); });
	inputs->Bind(&_inputManager->I.onPerform, [this]() { LOG_MSG("I Hold"); });
	inputs->Bind(&_inputManager->I.onRelease, [this]() { LOG_MSG("I Release"); });

	inputs->Bind(&_inputManager->J.onPress, [this]() { LOG_MSG("J Press"); });
	inputs->Bind(&_inputManager->J.onPerform, [this]() { LOG_MSG("J Hold"); });
	inputs->Bind(&_inputManager->J.onRelease, [this]() { LOG_MSG("J Release"); });

	inputs->Bind(&_inputManager->K.onPress, [this]() { LOG_MSG("K Press"); });
	inputs->Bind(&_inputManager->K.onPerform, [this]() { LOG_MSG("K Hold"); });
	inputs->Bind(&_inputManager->K.onRelease, [this]() { LOG_MSG("K Release"); });

	inputs->Bind(&_inputManager->L.onPress, [this]() { LOG_MSG("L Press"); });
	inputs->Bind(&_inputManager->L.onPerform, [this]() { LOG_MSG("L Hold"); });
	inputs->Bind(&_inputManager->L.onRelease, [this]() { LOG_MSG("L Release"); });

	inputs->Bind(&_inputManager->M.onPress, [this]() { LOG_MSG("M Press"); });
	inputs->Bind(&_inputManager->M.onPerform, [this]() { LOG_MSG("M Hold"); });
	inputs->Bind(&_inputManager->M.onRelease, [this]() { LOG_MSG("M Release"); });

	inputs->Bind(&_inputManager->N.onPress, [this]() { LOG_MSG("N Press"); });
	inputs->Bind(&_inputManager->N.onPerform, [this]() { LOG_MSG("N Hold"); });
	inputs->Bind(&_inputManager->N.onRelease, [this]() { LOG_MSG("N Release"); });

	inputs->Bind(&_inputManager->O.onPress, [this]() { LOG_MSG("O Press"); });
	inputs->Bind(&_inputManager->O.onPerform, [this]() { LOG_MSG("O Hold"); });
	inputs->Bind(&_inputManager->O.onRelease, [this]() { LOG_MSG("O Release"); });

	inputs->Bind(&_inputManager->P.onPress, [this]() { LOG_MSG("P Press"); });
	inputs->Bind(&_inputManager->P.onPerform, [this]() { LOG_MSG("P Hold"); });
	inputs->Bind(&_inputManager->P.onRelease, [this]() { LOG_MSG("P Release"); });

	inputs->Bind(&_inputManager->Q.onPress, [this]() { LOG_MSG("Q Press"); });
	inputs->Bind(&_inputManager->Q.onPerform, [this]() { LOG_MSG("Q Hold"); });
	inputs->Bind(&_inputManager->Q.onRelease, [this]() { LOG_MSG("Q Release"); });

	inputs->Bind(&_inputManager->R.onPress, [this]() { LOG_MSG("R Press"); });
	inputs->Bind(&_inputManager->R.onPerform, [this]() { LOG_MSG("R Hold"); });
	inputs->Bind(&_inputManager->R.onRelease, [this]() { LOG_MSG("R Release"); });

	inputs->Bind(&_inputManager->S.onPress, [this]() { LOG_MSG("S Press"); });
	inputs->Bind(&_inputManager->S.onPerform, [this]() { LOG_MSG("S Hold"); });
	inputs->Bind(&_inputManager->S.onRelease, [this]() { LOG_MSG("S Release"); });

	inputs->Bind(&_inputManager->T.onPress, [this]() { LOG_MSG("T Press"); });
	inputs->Bind(&_inputManager->T.onPerform, [this]() { LOG_MSG("T Hold"); });
	inputs->Bind(&_inputManager->T.onRelease, [this]() { LOG_MSG("T Release"); });

	inputs->Bind(&_inputManager->U.onPress, [this]() { LOG_MSG("U Press"); });
	inputs->Bind(&_inputManager->U.onPerform, [this]() { LOG_MSG("U Hold"); });
	inputs->Bind(&_inputManager->U.onRelease, [this]() { LOG_MSG("U Release"); });

	inputs->Bind(&_inputManager->V.onPress, [this]() { LOG_MSG("V Press"); });
	inputs->Bind(&_inputManager->V.onPerform, [this]() { LOG_MSG("V Hold"); });
	inputs->Bind(&_inputManager->V.onRelease, [this]() { LOG_MSG("V Release"); });

	inputs->Bind(&_inputManager->W.onPress, [this]() { LOG_MSG("W Press"); });
	inputs->Bind(&_inputManager->W.onPerform, [this]() { LOG_MSG("W Hold"); });
	inputs->Bind(&_inputManager->W.onRelease, [this]() { LOG_MSG("W Release"); });

	inputs->Bind(&_inputManager->X.onPress, [this]() { LOG_MSG("X Press"); });
	inputs->Bind(&_inputManager->X.onPerform, [this]() { LOG_MSG("X Hold"); });
	inputs->Bind(&_inputManager->X.onRelease, [this]() { LOG_MSG("X Release"); });

	inputs->Bind(&_inputManager->Y.onPress, [this]() { LOG_MSG("Y Press"); });
	inputs->Bind(&_inputManager->Y.onPerform, [this]() { LOG_MSG("Y Hold"); });
	inputs->Bind(&_inputManager->Y.onRelease, [this]() { LOG_MSG("Y Release"); });

	inputs->Bind(&_inputManager->Z.onPress, [this]() { LOG_MSG("Z Press"); });
	inputs->Bind(&_inputManager->Z.onPerform, [this]() { LOG_MSG("Z Hold"); });
	inputs->Bind(&_inputManager->Z.onRelease, [this]() { LOG_MSG("Z Release"); });

	// NUMBERS
	inputs->Bind(&_inputManager->Num0.onPress, [this]() { LOG_MSG("Num0 Press"); });
	inputs->Bind(&_inputManager->Num0.onPerform, [this]() { LOG_MSG("Num0 Hold"); });
	inputs->Bind(&_inputManager->Num0.onRelease, [this]() { LOG_MSG("Num0 Release"); });

	// CONTINUE SAME PATTERN FOR Num1 -> Num9 ...

	// SPECIAL KEYS
	inputs->Bind(&_inputManager->Escape.onPress, [this]() { LOG_MSG("Escape Press"); });
	inputs->Bind(&_inputManager->Escape.onPerform, [this]() { LOG_MSG("Escape Hold"); });
	inputs->Bind(&_inputManager->Escape.onRelease, [this]() { LOG_MSG("Escape Release"); });

	inputs->Bind(&_inputManager->LControl.onPress, [this]() { LOG_MSG("LControl Press"); });
	inputs->Bind(&_inputManager->LControl.onPerform, [this]() { LOG_MSG("LControl Hold"); });
	inputs->Bind(&_inputManager->LControl.onRelease, [this]() { LOG_MSG("LControl Release"); });

	// CONTINUE SAME PATTERN FOR ALL OTHER Input ...

	// MOUSE BUTTONS
	inputs->Bind(&_inputManager->MouseLeftClick.onPress, [this]() { LOG_MSG("MouseLeftClick Press"); });
	inputs->Bind(&_inputManager->MouseLeftClick.onPerform, [this]() { LOG_MSG("MouseLeftClick Hold"); });
	inputs->Bind(&_inputManager->MouseLeftClick.onRelease, [this]() { LOG_MSG("MouseLeftClick Release"); });

	inputs->Bind(&_inputManager->MouseRightClick.onPress, [this]() { LOG_MSG("MouseRightClick Press"); });
	inputs->Bind(&_inputManager->MouseRightClick.onPerform, [this]() { LOG_MSG("MouseRightClick Hold"); });
	inputs->Bind(&_inputManager->MouseRightClick.onRelease, [this]() { LOG_MSG("MouseRightClick Release"); });

	inputs->Bind(&_inputManager->MouseMiddleClick.onPress, [this]() { LOG_MSG("MouseMiddleClick Press"); });
	inputs->Bind(&_inputManager->MouseMiddleClick.onPerform, [this]() { LOG_MSG("MouseMiddleClick Hold"); });
	inputs->Bind(&_inputManager->MouseMiddleClick.onRelease, [this]() { LOG_MSG("MouseMiddleClick Release"); });

	// MOUSE MOVEMENT
	inputs->Bind(&_inputManager->MouseMovedWorld, [this](IVector2 _v)
		{
			LOG_MSG("Mouse Moved World at : " + _v.ToString());
		});

	inputs->Bind(&_inputManager->MouseMovedScreen, [this](IVector2 _v)
		{
			LOG_MSG("Mouse Moved Screen at : " + _v.ToString());
		});

	// MOUSE WHEEL
	inputs->Bind(&_inputManager->MouseWheelScroll, [this](Float _f)
		{
			LOG_MSG("Mouse Wheel Scroll : " + _f.ToString());
		});

	// WINDOW / FOCUS
	inputs->Bind(&_inputManager->FocusGain, [this]()
		{
			LOG_MSG("Focus Gain");
		});

	inputs->Bind(&_inputManager->FocusLost, [this]()
		{
			LOG_MSG("Focus Lost");
		});

	inputs->Bind(&_inputManager->MouseEntred, [this]()
		{
			LOG_MSG("Mouse Entered");
		});

	inputs->Bind(&_inputManager->MouseLeft, [this]()
		{
			LOG_MSG("Mouse Left");
		});

	// GAMEPAD
	inputs->Bind(&_inputManager->LeftJoystick, [this](FVector2 _v)
		{
			LOG_MSG("Left Joystick : " + _v.ToString());
		});

	inputs->Bind(&_inputManager->RightJoystick, [this](FVector2 _v)
		{
			LOG_MSG("Right Joystick : " + _v.ToString());
		});

	inputs->Bind(&_inputManager->BackTriggerLeft, [this](Float _f)
		{
			LOG_MSG("BackTriggerLeft : " + _f.ToString());
		});

	inputs->Bind(&_inputManager->BackTriggerRight, [this](Float _f)
		{
			LOG_MSG("BackTriggerRight : " + _f.ToString());
		});

	inputs->Bind(&_inputManager->DPad, [this](FVector2 _v)
		{
			LOG_MSG("DPad : " + _v.ToString());
		});

	// GAMEPAD BUTTONS
	inputs->Bind(&_inputManager->ButtonA.onPress, [this]() { LOG_MSG("ButtonA Press"); });
	inputs->Bind(&_inputManager->ButtonA.onPerform, [this]() { LOG_MSG("ButtonA Hold"); });
	inputs->Bind(&_inputManager->ButtonA.onRelease, [this]() { LOG_MSG("ButtonA Release"); });

	// CONTINUE SAME PATTERN FOR
	// ButtonB
	// ButtonX
	// ButtonY
	// ButtonStart
	// ButtonBack
	// ButtonLB
	// ButtonRB
	// ButtonLS
	// ButtonRS

	// WINDOW EVENTS
	inputs->Bind(&_inputManager->WindowResize, [this](IVector2 _v)
		{
			LOG_MSG("Window Resize : " + _v.ToString());
		});

	inputs->Bind(&_inputManager->WindowClose, [this]()
		{
			LOG_MSG("Window Close");
		});
}

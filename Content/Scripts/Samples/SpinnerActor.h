#pragma once
#include "Reflection/Reflection.h"

namespace Krampus
{

	/// <summary>
	/// Sample script: turns every frame. Its KPROPERTY fields appear in the Inspector and are saved in .ka7 scenes.
	/// </summary>
	class SpinnerActor : public Actor
	{
		KACTOR(SpinnerActor, Actor)

	public:
		SpinnerActor(Level* _level);

		KPROPERTY_RANGE(Float, speed, -360.f, 360.f) = 90.f;	// degrees per second
		KPROPERTY(Bool, clockwise) = true;
		KPROPERTY(String, label) = "Spinner";
		KPROPERTY(FVector2, offset);
		KPROPERTY(Color, tint) = Color(255, 255, 255);
		KPROPERTY_READONLY(Int, turns) = 0;

		void Tick(const Float& _deltaTime) override;
	};

}

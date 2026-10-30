#pragma once
#include "Reflection/Reflection.h"

namespace Krampus
{

	// Add it from the Inspector (Add Component) or from an actor constructor with CreateComponent<MovementComponent>().
	class MovementComponent : public Component
	{
		KCOMPONENT(MovementComponent, Component)

	public:
		MovementComponent(Actor* _owner);

		KPROPERTY(Float, moveSpeed) = 1.f;

		KFUNCTION(MoveLeft)
		void MoveLeft();
		KFUNCTION(MoveRight)
		void MoveRight();
	};

}

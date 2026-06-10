#pragma once
#include "Actors/Actor.h"
#include "Components/InputComponent.h"

namespace Krampus::Exemple
{

	class InputActorExemple : public Actor
	{
		InputComponent* inputs = nullptr;

	public:
		InputActorExemple(Level* _level);

		virtual void Construct() override;
	};

}
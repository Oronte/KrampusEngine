#pragma once
#include "Actors/Actor.h"
#include "Components/AnimationComponentSM.h"

namespace Krampus::Exemple
{
	class AnimationSMExemple : public Actor
	{
		AnimationComponentSM* animation = nullptr;
		Float time = 0.0f;
		Float maxTime = 1.0f;
		Bool canUpdateAnimation = false;

	public:
		AnimationSMExemple(Level* _level);

		virtual void Construct() override;
		virtual void BeginPlay() override;
		virtual void Tick(const Float& _deltaTime) override;
		virtual void Deconstruct() override;
		virtual void BeginDestroy() override;
	};
}
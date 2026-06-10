#pragma once
#include "Actors/Actor.h"
#include "Components/AnimationComponent.h"

namespace Krampus::Exemple
{
	class AnimationExemple : public Actor
	{
		AnimationComponent* animation = nullptr;

	public:
		AnimationExemple(Level* _level);

		virtual void Construct() override;
		virtual void BeginPlay() override;
		virtual void Tick(const Float& _deltaTime) override;
		virtual void Deconstruct() override;
		virtual void BeginDestroy() override;
	};
}
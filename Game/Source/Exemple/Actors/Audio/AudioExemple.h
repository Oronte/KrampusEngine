#pragma once
#include "Actors/Actor.h"
#include "Components/SpatialSoundComponent.h"

namespace Krampus::Exemple
{
	class AudioExemple : public Actor
	{
		SpatialSoundComponent* soundComponent = nullptr;

	public:
		AudioExemple(Level* _level);

		virtual void Construct() override;
		virtual void BeginPlay() override;
		virtual void Tick(const Float& _deltaTime) override;
		virtual void Deconstruct() override;
		virtual void BeginDestroy() override;
	};
}
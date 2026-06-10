#pragma once
#include "Actors/Actor.h"
#include "Components/AudioListenerComponent.h"

namespace Krampus::Exemple
{
	class AudioListener3DExemple : public Actor
	{
		AudioListenerComponent* listener = nullptr;

	public:
		AudioListener3DExemple(Level* _level);

		virtual void Construct() override;
		virtual void BeginPlay() override;
		virtual void Tick(const Float& _deltaTime) override;
		virtual void Deconstruct() override;
		virtual void BeginDestroy() override;
	};
}
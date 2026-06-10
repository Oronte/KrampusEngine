#pragma once
#include "Actors/Actor.h"
#include "Components/CameraComponent.h"

namespace Krampus::Exemple
{

	class CameraActorExemple : public Actor
	{
		CameraComponent* camera = nullptr;
		Event<Float>::ListenerHandle onWheelScroll;

	public:
		CameraActorExemple(Level* _level);

		virtual void Construct() override;
		virtual void BeginPlay() override;
		virtual void Tick(const Float& _deltaTime) override;
		virtual void Deconstruct() override;
	};

}
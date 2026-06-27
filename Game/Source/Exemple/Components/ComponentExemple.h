#pragma once
#include "Components/Component.h"

namespace Krampus::Exemple
{

	class ComponentExemple : public Krampus::Component
	{
	public:
		// All Components must have their public constructor starting with "Krampus::Actor* _owner" 
		ComponentExemple(Krampus::Actor* _owner);
		ComponentExemple(Krampus::Actor* _owner, Float _exempleParameter);

		// Called when added to an Actor
		virtual void Construct() override;
		// Called at the first frame the owner play's in the scene
		virtual void BeginPlay() override;
		// Called every frame while playing
		virtual void Tick(const Float& _deltaTime) override;
		// Called when this Component is remove from the owner
		virtual void Deconstruct() override;
		// Called after "Deconstruct" 
		virtual void BeginDestroy() override;

		// Order : Constructor -> Construct -> BeginPlay (-> Tick) -> Deconstruct -> BeginDestroy -> Destructor
	};

}
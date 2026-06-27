#pragma once
#include "Actors/Actor.h"
#include "Exemple/Components/ComponentExemple.h"


namespace Krampus::Exemple
{

	class ActorExemple : public Krampus::Actor
	{
		ComponentExemple* component = nullptr;

	public:
		// All Actors must have their public constructor starting with "Krampus::Level* _level"
		ActorExemple(Krampus::Level* _level);
		ActorExemple(Krampus::Level* _level, Float _exempleParameter);

		// Called after the Constructor
		virtual void Construct() override;
		// Called at the first frame this actor play's in the scene
		virtual void BeginPlay() override;
		// Called every frame while playing
		virtual void Tick(const Float& _deltaTime) override;
		// Called when this actor is remove from the scene (including when changing scene)
		virtual void Deconstruct() override;
		// Called after "Deconstruct" 
		virtual void BeginDestroy() override;

		// Order : Constructor -> Construct -> BeginPlay (-> Tick) -> Deconstruct -> BeginDestroy -> Destructor
	};

}
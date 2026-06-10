#pragma once
#include "Actors/Actor.h"
#include "Components/RigidbodyComponent.h"

namespace Krampus::Exemple
{

	class RigidbodyExemple : public Actor
	{
		CollisionComponent* collision = nullptr;
		RigidbodyComponent* rb = nullptr;

	public:
		RigidbodyExemple(Level* _level);

		virtual void Construct() override;
	};

}
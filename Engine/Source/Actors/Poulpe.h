#pragma once
#include "SpriteActor.h"
#include "Components/RigidbodyComponent.h"
#include "Components/AnimationComponent.h"

namespace Krampus
{

	class Poulpe : public SpriteActor
	{
		RigidbodyComponent* rigidbody = nullptr;
		AnimationComponent* animation = nullptr;

	public:
		Poulpe(Level* _level);
	};

}
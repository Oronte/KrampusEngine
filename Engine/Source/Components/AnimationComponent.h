#pragma once
#include "BasicAnimation.h"

namespace Krampus
{

	/// <summary>Sprite-sheet animations of the actor's SpriteComponent.</summary>
	class AnimationComponent : public BasicAnimation<Component, SpriteComponent>
	{
	public:
		explicit AnimationComponent(Actor* _owner) : BasicAnimation(_owner) { name = NAME_OF(AnimationComponent); }
	};

}

#pragma once
#include "Utilities/Math/Physics.h"
#include "Component.h"

namespace Krampus
{

	class SpriteComponent;

	class CollisionComponent : public Component
	{
		SpriteComponent* sprite;

	public:
		Event<CollisionInfo> onCollision;

	public:
		// The owner require a SpriteComponent
		CollisionComponent(Actor* _owner);

		void ComputeCollision(CollisionComponent* _other);

	private:
		bool CircleToCircle(CollisionComponent* _other);
		bool RectToRectOBB(CollisionComponent* _other);
		bool RectToRectAABB(CollisionComponent* _other);
		bool CircleToRect(CollisionComponent* _circle, CollisionComponent* _rect);
	};

}


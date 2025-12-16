#pragma once
#include "SpriteComponent.h"

namespace Krampus
{

	struct CollisionInfo
	{
		FVector2 normal;
		float penetration = 0.f;
		FVector2 contactPoint;
		class CollisionComponent* collision;
	};

	class CollisionComponent : public Component
	{
		FVector2 halfSize;
		ShapeType type;
		SpriteComponent* sprite;

	public:
		Event<CollisionInfo> onCollision;

	public:
		// The owner require a SpriteComponent
		CollisionComponent(Actor* _owner);

		void ComputeCollision(CollisionComponent* _other);

	private:
		bool CircleToCircle(CollisionComponent* _other);
	};

}


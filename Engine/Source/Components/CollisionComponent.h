#pragma once
#include "Utilities/Math/Vector2D.h"
#include "Utilities/Math/Angle.h"
#include "Component.h"

namespace Krampus
{

	struct CollisionInfo
	{
		FVector2 normal;
		float penetration = 0.f;
		FVector2 contactPoint;
		class CollisionComponent* collision;
	};

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
		//OBB
		bool CircleToCircle(CollisionComponent* _other);
		bool RectToRect(CollisionComponent* _other);

		void GetAxes(const CollisionComponent* _component, FVector2 axes[2]);
		float ProjectOBB(const CollisionComponent* _component, const FVector2& axis);
	};

}


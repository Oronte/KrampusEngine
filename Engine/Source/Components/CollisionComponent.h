#pragma once
#include "Utilities/Math/Physics.h"
#include "Component.h"

namespace Krampus
{

	enum class CollisionChannel : uint32_t
	{
		None = 0,
		Wall = 1 << 0,
		Player = 1 << 1,
		Enemy = 1 << 2,
		Projectile = 1 << 3,
		Trigger = 1 << 4
	};

	INLINE CollisionChannel operator|(CollisionChannel _a, CollisionChannel _b)
	{
		return CAST(CollisionChannel, (CAST(uint32_t, _a) | CAST(uint32_t, _b)));
	}

	INLINE CollisionChannel operator&(CollisionChannel _a, CollisionChannel _b)
	{
		return CAST(CollisionChannel, (CAST(uint32_t, _a) & CAST(uint32_t, _b)));
	}

	class SpriteComponent;

	class CollisionComponent : public Component
	{
		ShapeSizeData& sizeData;
		ShapeType shapeType;
		std::unordered_set<CollisionComponent*> collidingComponents;

	public:
		// TODO ca dit Enter et Exit 2fois
		Event<CollisionInfo> onCollisionEnter;
		Event<CollisionInfo> onCollision;
		Event<CollisionComponent*> onCollisionExit;

		CollisionChannel channel = CollisionChannel::None;
		CollisionChannel mask = CollisionChannel::None;

	public:
		// The owner require a SpriteComponent
		CollisionComponent(Actor* _owner, 
			const CollisionChannel& _channel = CollisionChannel::None,
			const CollisionChannel& _mask = CollisionChannel::None);
		CollisionComponent(Actor* _owner, const ShapeSizeData& _sizeData,
			const CollisionChannel& _channel = CollisionChannel::None,
			const CollisionChannel& _mask = CollisionChannel::None);

		void ResolveCollision(const CollisionInfo& _info);
		void BindCollisionResolution();
		void ComputeCollision(CollisionComponent* _other);

	private:
		bool CanCollide(const CollisionComponent* _other) const;

		bool CircleToCircle(CollisionComponent* _other);
		bool RectToRectOBB(CollisionComponent* _other);
		bool RectToRectAABB(CollisionComponent* _other);
		bool CircleToRect(CollisionComponent* _circle, CollisionComponent* _rect);

		virtual void BeginDestroy() override;
	};

}

////////////////////////////////////////////////////////
// 
// 
// 
////////////////////////////////////////////////////////
#pragma once
#include "Utilities/Math/Physics.h"
#include "Component.h"
#include "Graphics/Shapes/ShapeObject.h"
#include "Utilities/System/Debuggable.h"

namespace Krampus
{

	enum class CollisionChannel : uint32_t
	{
		None = 0,
		Wall = 1 << 0,
		Player = 1 << 1,
		Enemy = 1 << 2,
		Projectile = 1 << 3,
		Trigger = 1 << 4,

		All = Wall | Player | Enemy | Projectile | Trigger
	};

	inline CollisionChannel operator|(CollisionChannel _a, CollisionChannel _b)
	{
		return CAST(CollisionChannel, (CAST(uint32_t, _a) | CAST(uint32_t, _b)));
	}

	inline CollisionChannel operator&(CollisionChannel _a, CollisionChannel _b)
	{
		return CAST(CollisionChannel, (CAST(uint32_t, _a) & CAST(uint32_t, _b)));
	}

	class SpriteComponent;

	class CollisionComponent : public Component, public IDebuggable
	{
		ShapeSizeData sizeData;
		ShapeType shapeType;
		std::unordered_set<CollisionComponent*> collidingComponents;

		Event<CollisionInfo>::ListenerHandle onCollisionHandle;

	public:
		Event<CollisionInfo> onCollisionEnter;
		Event<CollisionInfo> onCollision;
		Event<CollisionComponent*> onCollisionExit;

		CollisionChannel channel = CollisionChannel::None;
		CollisionChannel mask = CollisionChannel::None;

	public:
		inline ShapeType GetShapeType() const
		{
			return shapeType;
		}
		inline float GetRadius() const
		{
			if (shapeType != ShapeType::Circle)
			{
				LOG_ERROR("You try to get the radius but the shape is not a Circle");
				return 0.0f;
			}
			return sizeData.radius;
		}
		inline FVector2 GetSize() const
		{
			if (shapeType != ShapeType::Rectangle)
			{
				LOG_ERROR("You try to get the size but the shape is not a Rectangle");
				return FVector2();
			}
			return sizeData.size;
		}

		static inline bool ContainsCollisionChannel(const CollisionChannel& _a, const CollisionChannel& _b)
		{
			return CAST(uint32_t, _a & _b) != 0;
		}

		// The owner require a SpriteComponent
		CollisionComponent(Actor* _owner, 
			const CollisionChannel& _channel = CollisionChannel::None,
			const CollisionChannel& _mask = CollisionChannel::None);
		CollisionComponent(Actor* _owner, const float& _radius,
			const CollisionChannel& _channel = CollisionChannel::None,
			const CollisionChannel& _mask = CollisionChannel::None);
		CollisionComponent(Actor* _owner, const FVector2& _size,
			const CollisionChannel& _channel = CollisionChannel::None,
			const CollisionChannel& _mask = CollisionChannel::None);

		void UpdateShapeValues(const SpriteComponent* _sprite);
		void SetSize(const FVector2& _size);
		void SetRadius(const float _radius);

		void ResolveCollision(const CollisionInfo& _info);
		void BindCollisionResolution();
		void ComputeCollision(CollisionComponent* _other);
		bool CanCollide(const CollisionComponent* _other) const;

		virtual void Tick(const float& _deltaTime) override;
		virtual void BeginDestroy() override;

	private:
		bool CircleToCircle(CollisionComponent* _other);
		bool RectToRectOBB(CollisionComponent* _other);
		bool RectToRectAABB(CollisionComponent* _other);
		bool CircleToRect(CollisionComponent* _circle, CollisionComponent* _rect);


		void DrawDebug() override;
	};

}

////////////////////////////////////////////////////////
// 
// 
// 
////////////////////////////////////////////////////////
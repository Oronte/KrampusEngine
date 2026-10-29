#pragma once
#include "Reflection/EventList.h"
#include "Utilities/Math/Physics.h"
#include "Component.h"
#include "Graphics/Shapes/ShapeObject.h"
#include "Utilities/System/Debuggable.h"
#include "Utilities/System/CollisionChannel.h"

namespace Krampus
{

	class SpriteComponent;

	/// <summary>
	/// Component that handles geometric collision detection (circles, AABB, OBB) and collision events.
	/// </summary>
	class CollisionComponent : public Component, public IDebuggable
	{
		ShapeSizeData							sizeData;
		ShapeType								shapeType;
		std::unordered_set<CollisionComponent*> collidingComponents;

		Event<CollisionInfo>::ListenerHandle	onCollisionHandle;
		Event<CollisionInfo>::ListenerHandle	enterCallsHandle;
		Event<CollisionComponent*>::ListenerHandle	exitCallsHandle;

		void InitEvents();

	public:
		/// <summary>
		/// Event triggered on the first frame of contact with another collider.
		/// </summary>
		Event<CollisionInfo>					onCollisionEnter;

		/// <summary>
		/// Event triggered each frame collision contact persists.
		/// </summary>
		Event<CollisionInfo>					onCollision;

		/// <summary>
		/// Event triggered when contact with another collider ceases.
		/// </summary>
		Event<CollisionComponent*>				onCollisionExit;

		/// <summary>Functions called from the editor (event lists) when a collision begins / ends.</summary>
		EventList								onEnterCalls;
		EventList								onExitCalls;

		/// <summary>
		/// Collision channel assigned to this collider.
		/// </summary>
		CollisionChannel						channel				= CollisionChannel::None;

		/// <summary>
		/// Mask determining which collision channels this collider interacts with.
		/// </summary>
		CollisionChannel						mask				= CollisionChannel::None;

	public:
		/// <summary>
		/// Gets the geometric shape type used for collision (Circle or Rectangle).
		/// </summary>
		/// <returns>The ShapeType enum value.</returns>
		inline ShapeType GetShapeType() const
		{
			return shapeType;
		}

		/// <summary>
		/// Gets the radius if this collider is a circle.
		/// </summary>
		/// <returns>Radius value, or 0.0f if not a circle.</returns>
		inline Float GetRadius() const
		{
			if (shapeType != ShapeType::Circle)
			{
				LOG_ERROR("You try to get the radius but the shape is not a Circle");
				return 0.0f;
			}
			return sizeData.radius;
		}

		/// <summary>
		/// Gets the dimensions if this collider is a rectangle.
		/// </summary>
		/// <returns>Size vector, or zero if not a rectangle.</returns>
		inline FVector2 GetSize() const
		{
			if (shapeType != ShapeType::Rectangle)
			{
				LOG_ERROR("You try to get the size but the shape is not a Rectangle");
				return FVector2();
			}
			return sizeData.size;
		}

		/// <summary>
		/// Checks whether channel mask A contains any matching flags from channel mask B.
		/// </summary>
		/// <param name="_a">Source channel mask.</param>
		/// <param name="_b">Filter channel mask.</param>
		/// <returns>True if any flags match, false otherwise.</returns>
		static inline Bool ContainsCollisionChannel(const CollisionChannel& _a, const CollisionChannel& _b)
		{
			return CAST(uint32_t, _a & _b) != 0;
		}

		/// <summary>
		/// Constructs a CollisionComponent matching the geometry of the owner's SpriteComponent.
		/// Note: The owner requires an existing SpriteComponent.
		/// </summary>
		/// <param name="_owner">Pointer to owning Actor.</param>
		/// <param name="_channel">Collision channel of this component.</param>
		/// <param name="_mask">Collision mask to detect collisions against.</param>
		CollisionComponent(Actor* _owner, 
			const CollisionChannel& _channel = CollisionChannel::None,
			const CollisionChannel& _mask = CollisionChannel::None);

		/// <summary>
		/// Constructs a circular CollisionComponent with a specified radius.
		/// </summary>
		/// <param name="_owner">Pointer to owning Actor.</param>
		/// <param name="_radius">Radius of the collision circle.</param>
		/// <param name="_channel">Collision channel.</param>
		/// <param name="_mask">Collision mask.</param>
		CollisionComponent(Actor* _owner, const Float& _radius,
			const CollisionChannel& _channel = CollisionChannel::None,
			const CollisionChannel& _mask = CollisionChannel::None);

		/// <summary>
		/// Constructs a rectangular CollisionComponent with specified dimensions.
		/// </summary>
		/// <param name="_owner">Pointer to owning Actor.</param>
		/// <param name="_size">Width and height of the collision box.</param>
		/// <param name="_channel">Collision channel.</param>
		/// <param name="_mask">Collision mask.</param>
		CollisionComponent(Actor* _owner, const FVector2& _size,
			const CollisionChannel& _channel = CollisionChannel::None,
			const CollisionChannel& _mask = CollisionChannel::None);

		/// <summary>
		/// Updates collision dimensions from the given sprite component's shape.
		/// </summary>
		/// <param name="_sprite">Pointer to source SpriteComponent.</param>
		void UpdateShapeValues(const SpriteComponent* _sprite);

		/// <summary>
		/// Forgets a collider that is going away: it must not stay in the list of overlaps (its address may be reused).
		/// </summary>
		/// <param name="_other">The collider being destroyed.</param>
		inline void ForgetCollision(CollisionComponent* _other)
		{
			collidingComponents.erase(_other);
		}

		/// <summary>
		/// Turns the collision shape into a circle or a rectangle of about the same size.
		/// </summary>
		void SetShapeType(const ShapeType& _type);

		/// <summary>
		/// Sets the dimensions of the rectangular collision shape.
		/// </summary>
		/// <param name="_size">New box dimensions.</param>
		void SetSize(const FVector2& _size);

		/// <summary>
		/// Sets the radius of the circular collision shape.
		/// </summary>
		/// <param name="_radius">New circle radius.</param>
		void SetRadius(const Float& _radius);

		/// <summary>
		/// Separates colliders based on penetration vector and collision info.
		/// </summary>
		/// <param name="_info">Collision contact information.</param>
		void ResolveCollision(const CollisionInfo& _info);

		/// <summary>
		/// Binds automatic position resolution to the onCollision event.
		/// </summary>
		void BindCollisionResolution();

		/// <summary>
		/// Tests collision against another collider and invokes collision events if intersecting.
		/// </summary>
		/// <param name="_other">Pointer to the other CollisionComponent.</param>
		void ComputeCollision(CollisionComponent* _other);

		/// <summary>
		/// Determines whether collision testing is valid between this collider and another based on channels/masks.
		/// </summary>
		/// <param name="_other">Pointer to the other CollisionComponent.</param>
		/// <returns>True if both can collide, false otherwise.</returns>
		bool CanCollide(const CollisionComponent* _other) const;

		/// <summary>
		/// Updates collision state each frame.
		/// </summary>
		/// <param name="_deltaTime">Elapsed time in seconds.</param>
		virtual void Tick(const Float& _deltaTime) override;

		/// <summary>
		/// Lifecycle callback invoked just before destruction to unbind events.
		/// </summary>
		virtual void BeginDestroy() override;

	private:
		bool CircleToCircle(CollisionComponent* _other);
		bool RectToRectOBB(CollisionComponent* _other);
		bool RectToRectAABB(CollisionComponent* _other);
		bool CircleToRect(CollisionComponent* _circle, CollisionComponent* _rect);

		void DrawDebug() override;

	public:
		/// <summary>
		/// Returns a string representation of the component.
		/// </summary>
		/// <returns>String representation.</returns>
		virtual std::string ToString() const override;
	};

}

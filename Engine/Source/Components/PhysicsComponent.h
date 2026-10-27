#pragma once
#include "Component.h"
#include "box2d/box2d.h"

namespace Krampus
{
	/// <summary>
	/// Component that integrates Box2D 2D rigid body physics simulation with an actor.
	/// </summary>
    class PhysicsComponent : public Component
    {
        b2BodyDef   body;
        b2BodyId    id;
        b2ShapeDef  shape;

        Event<>::ListenerHandle collisionResolveHandle;
        Event<FVector2>::ListenerHandle moveHandle;
        Event<Angle>::ListenerHandle rotateHandle;

    public:
		/// <summary>
		/// Constructs a PhysicsComponent using the geometry of the owner's SpriteComponent.
		/// Note: Requires an existing SpriteComponent on the owner actor.
		/// </summary>
		/// <param name="_owner">Pointer to owning Actor.</param>
        PhysicsComponent(Actor* _owner);

		/// <summary>
		/// Constructs a PhysicsComponent with a circular Box2D body of the specified radius.
		/// </summary>
		/// <param name="_owner">Pointer to owning Actor.</param>
		/// <param name="_radius">Radius of the physics circle.</param>
        PhysicsComponent(Actor* _owner, const Float& _radius);

		/// <summary>
		/// Constructs a PhysicsComponent with a box Box2D body of the specified dimensions.
		/// </summary>
		/// <param name="_owner">Pointer to owning Actor.</param>
		/// <param name="_size">Width and height of the physics box.</param>
        PhysicsComponent(Actor* _owner, const FVector2& _size);

		/// <summary>
		/// Lifecycle callback invoked when the component is deconstructed, destroying the Box2D body.
		/// </summary>
		virtual void Deconstruct() override;

		/// <summary>
		/// Callback invoked after collision resolution to synchronize transforms.
		/// </summary>
        void OnCollisionResolve();

		/// <summary>
		/// Configures the Box2D body type as dynamic (fully simulated by forces and gravity).
		/// </summary>
        void SetDynamic();

		/// <summary>
		/// Configures the Box2D body type as static (immovable, unaffected by forces).
		/// </summary>
        void SetStatic();

		/// <summary>
		/// Configures the Box2D body type as kinematic (moved only by script/velocity, unaffected by forces).
		/// </summary>
        void SetKinematic();

    private:
        void Init();
        void MakeCircle(const Float& _radius);
        void MakeRectangle(const FVector2& _size);
    };
}

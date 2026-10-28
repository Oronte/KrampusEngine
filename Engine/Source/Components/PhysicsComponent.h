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
        b2BodyId    id;

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
		/// Body type as box2d numbers it: 0 static, 1 kinematic, 2 dynamic.
		/// </summary>
		int GetBodyType() const;

		/// <summary>
		/// Sets the body type (0 static, 1 kinematic, 2 dynamic).
		/// </summary>
		void SetBodyType(const int& _type);

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

		/// <summary>
		/// Sets the linear velocity of an object.
		/// </summary>
		/// <param name="_velocity">The new linear velocity to set, represented as an FVector2.</param>
		void SetLinearVelocity(const FVector2& _velocity);

		/// <summary>
		/// Retrieves the current linear velocity as a 2D vector.
		/// </summary>
		/// <returns>A FVector2 representing the current linear velocity.</returns>
		FVector2 GetLinearVelocity() const;

		/// <summary>
		/// Sets the angular velocity to the specified value.
		/// </summary>
		/// <param name="_velocity">The new angular velocity to set.</param>
		void SetAngularVelocity(const Float& _velocity);

		/// <summary>
		/// Retrieves the current angular velocity.
		/// </summary>
		/// <returns>The current angular velocity as a Float.</returns>
		Float GetAngularVelocity() const;

		/// <summary>
		/// Sets the linear damping value.
		/// </summary>
		/// <param name="_damping">The linear damping value to set.</param>
		void SetLinearDamping(const Float& _damping);

		/// <summary>
		/// Retrieves the linear damping value.
		/// </summary>
		/// <returns>The linear damping value as a Float.</returns>
		Float GetLinearDamping() const;

		/// <summary>
		/// Sets the angular damping value.
		/// </summary>
		/// <param name="_damping">The new angular damping value to set.</param>
		void SetAngularDamping(const Float& _damping);

		/// <summary>
		/// Retrieves the angular damping value.
		/// </summary>
		/// <returns>The current angular damping as a Float.</returns>
		Float GetAngularDamping() const;

		/// <summary>
		/// Sets the gravity scale factor.
		/// </summary>
		/// <param name="_scale">The new gravity scale to apply.</param>
		void SetGravityScale(const Float& _scale);

		/// <summary>
		/// Retrieves the current gravity scale value.
		/// </summary>
		/// <returns>The gravity scale as a Float.</returns>
		Float GetGravityScale() const;

		/// <summary>
		/// Sets the mass data, including mass, inertia, and center of mass for an object.
		/// </summary>
		/// <param name="_mass">The mass value to set.</param>
		/// <param name="_inertia">The moment of inertia to set.</param>
		/// <param name="_center">The center of mass as a 2D vector.</param>
		void SetMassData(const Float& _mass, const Float& _inertia, const FVector2& _center);
		
		/// <summary>
		/// Retrieves the mass, inertia, and center of mass data.
		/// </summary>
		/// <param name="_mass">Reference to a variable that will receive the mass value.</param>
		/// <param name="_inertia">Reference to a variable that will receive the inertia value.</param>
		/// <param name="_center">Reference to a variable that will receive the center of mass as a 2D vector.</param>
		void GetMassData(Float& _mass, Float& _inertia, FVector2& _center) const;
		
		/// <summary>
		/// Calculates and applies the mass of an object based on its constituent shapes.
		/// </summary>
		void ApplyMassFromShapes();

		/// <summary>
		/// Sets whether the rotation is fixed.
		/// </summary>
		/// <param name="_fixed">A boolean value indicating if the rotation should be fixed (true) or not (false).</param>
		void SetFixedRotation(const bool& _fixed);

		/// <summary>
		/// Enables or disables sleeping for the physics object.
		/// </summary>
		/// <param name="_enable">A boolean value indicating if sleeping should be enabled (true) or disabled (false).</param>
		void EnableSleeping(const bool& _enable);
		
		/// <summary>
		/// Checks if sleeping is allowed for the physics object.
		/// </summary>
		/// <returns>A boolean value indicating if sleeping is allowed (true) or not (false).</returns>
		bool IsSleepingAllowed() const;
		
		/// <summary>
		/// Sets the sleep threshold for the physics object.
		/// </summary>
		/// <param name="_threshold">The sleep threshold value.</param>
		void SetSleepThreshold(const Float& _threshold);
		
		/// <summary>
		/// Retrieves the sleep threshold for the physics object.
		/// </summary>
		/// <returns>The sleep threshold value.</returns>
		Float GetSleepThreshold() const;
		
		/// <summary>
		/// Sets the awake state for the physics object.
		/// </summary>
		/// <param name="_awake">A boolean value indicating if the object should be awake (true) or asleep (false).</param>
		void SetAwake(const bool& _awake);
		
		/// <summary>
		/// Checks if the object is currently awake.
		/// </summary>
		/// <returns>true if the object is awake; otherwise, false.</returns>
		bool IsAwake() const;
		
		/// <summary>
		/// Enables the physics component.
		/// </summary>
		void Enable();
		
		/// <summary>
		/// Disables the physics component.
		/// </summary>
		void Disable();
		
		/// <summary>
		/// Enables or disables contact events for the physics object.
		/// </summary>
		/// <param name="_enable">A boolean value indicating if contact events should be enabled (true) or disabled (false).</param>
		void EnableContactEvents(bool _enable);
		
		/// <summary>
		/// Sets the bullet state for the physics object.
		/// </summary>
		/// <param name="_bullet">A boolean value indicating if the object should be treated as a bullet (true) or not (false).</param>
		void SetBullet(const bool& _bullet);

    private:
        void Init();
        void MakeCircle(const Float& _radius);
        void MakeRectangle(const FVector2& _size);
    };
}

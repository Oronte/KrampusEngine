#pragma once
#include "CollisionComponent.h"

namespace Krampus
{

	/// <summary>
	/// Component that provides custom Newtonian 2D physics simulation (mass, forces, impulse, friction, torque).
	/// </summary>
	class RigidbodyComponent : public Component
	{
	public:
		/// <summary>
		/// Whether gravity is applied to this rigidbody.
		/// </summary>
		Bool		useGravity				= true;

		/// <summary>
		/// Whether this body is kinematic (moved only by script, ignoring applied forces).
		/// </summary>
		Bool		isKinematic				= false;

		/// <summary>
		/// Acceleration of gravity in pixels per second squared.
		/// </summary>
		Float		gravity					= 981.f;

		/// <summary>
		/// Locks rotation if set to true.
		/// </summary>
		Bool		freezeRotation			= false;

		/// <summary>
		/// Locks movement along the X axis if set to true.
		/// </summary>
		Bool		freezeMovementX			= false;

		/// <summary>
		/// Locks movement along the Y axis if set to true.
		/// </summary>
		Bool		freezeMovementY			= false;

		/// <summary>
		/// Linear damping drag coefficient reducing velocity over time.
		/// </summary>
		Float		linearDamping			= 0.1f;

		/// <summary>
		/// Angular damping drag coefficient reducing angular velocity over time.
		/// </summary>
		Float		angularDamping			= 0.1f;

		/// <summary>
		/// Coefficient of restitution (bounciness) in range [0, 1].
		/// </summary>
		Float		restitution				= 0.15f;

		/// <summary>
		/// Static friction coefficient resisting initial motion.
		/// </summary>
		Float		staticFriction			= 0.5f;

		/// <summary>
		/// Dynamic friction coefficient resisting continuous sliding motion.
		/// </summary>
		Float		dynamicFriction			= 0.4f;

		/// <summary>
		/// Maximum cap for linear velocity magnitude.
		/// </summary>
		Float		maxLinearVelocity		= 2000.f;

		/// <summary>
		/// Maximum cap for angular velocity magnitude.
		/// </summary>
		Float		maxAngularVelocity		= 20.f;

	private:
		Float		mass					= 1.f;
		Float		inverseMass				= 1.f;
		Float		inertia					= 1.f;
		Float		inverseInertia			= 1.f;

		FVector2	velocity				= FVector2::Zero();
		Float		angularVelocity			= 0.f;
		FVector2	acceleration			= FVector2::Zero();
		FVector2	accumulatedForces		= FVector2::Zero();
		Float		accumulatedTorque		= 0.f;

		Event<CollisionInfo>::ListenerHandle onCollisionHandle;

	public:
		/// <summary>
		/// Constructs a RigidbodyComponent attached to the specified actor with an initial mass.
		/// </summary>
		/// <param name="_owner">Pointer to owning Actor.</param>
		/// <param name="_mass">Mass in kilograms (must be positive).</param>
		RigidbodyComponent(Actor* _owner, const Float& _mass = 1.0f);

		/// <summary>
		/// Adds a continuous force to be integrated during the current frame.
		/// </summary>
		/// <param name="_force">Force vector.</param>
		/// <param name="_applyPoint">Local application point offset relative to center of mass.</param>
		void AddForce(const FVector2& _force, const FVector2& _applyPoint = FVector2::Zero());

		/// <summary>
		/// Applies an instantaneous impulse directly altering linear and angular velocity.
		/// </summary>
		/// <param name="_impulse">Impulse vector.</param>
		/// <param name="contactVector">Contact point vector relative to center of mass.</param>
		void AddImpulse(const FVector2& _impulse, const FVector2& contactVector = FVector2::Zero());

		/// <summary>
		/// Clears all accumulated forces and torques.
		/// </summary>
		void ClearForces();

		/// <summary>
		/// Integrates motion over the elapsed time step.
		/// </summary>
		/// <param name="_deltaTime">Elapsed time in seconds.</param>
		virtual void Tick(const Float& _deltaTime) override;

		/// <summary>
		/// Binds physical collision response impulses to the owner's CollisionComponent.
		/// Note: Requires an existing CollisionComponent on the owner actor.
		/// </summary>
		void BindCollisionResponse();

		/// <summary>
		/// Handles collision impact response based on contact normal and penetration.
		/// </summary>
		/// <param name="_info">Collision contact information.</param>
		void OnCollision(const CollisionInfo& _info);

	private:
		void Integrate(const Float& _deltaTime);
		void ComputeInertia();

	public:
		/// <summary>
		/// Returns a string representation of the component.
		/// </summary>
		/// <returns>String representation.</returns>
		virtual std::string ToString() const override;
	};

}

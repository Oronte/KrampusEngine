#pragma once
#include "CollisionComponent.h"

namespace Krampus
{

	class RigidbodyComponent : public Component
	{
	public:
		Bool		useGravity				= true;
		Bool		isKinematic				= false;
		Float		gravity					= 981.f;

		Bool		freezeRotation			= false;
		Bool		freezeMovementX			= false;
		Bool		freezeMovementY			= false;

		Float		linearDamping			= 0.1f;
		Float		angularDamping			= 0.1f;
		Float		restitution				= 0.15f;
		Float		staticFriction			= 0.5f;
		Float		dynamicFriction			= 0.4f;

		Float		maxLinearVelocity		= 2000.f;
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
		RigidbodyComponent(Actor* _owner, const Float& _mass = 1.0f);

		// Adds a continuous force to be applied during integration
		void AddForce(const FVector2& _force, const FVector2& _applyPoint = FVector2::Zero());
		// Applies an instant impulse directly modifying velocity
		void AddImpulse(const FVector2& _impulse, const FVector2& contactVector = FVector2::Zero());
		// Clears all accumulated forces after simulation
		void ClearForces();

		virtual void Tick(const Float& _deltaTime) override;

		// Binds collision events to physical collision response (require a valid CollisionComponent)
		void BindCollisionResponse();
		// Handles collision response based on collision information
		void OnCollision(const CollisionInfo& _info);

	private:
		// Integrates forces, velocity, and position over time
		void Integrate(const Float& _deltaTime);
		void ComputeInertia();

	public:
		virtual std::string ToString() const override;
	};

}


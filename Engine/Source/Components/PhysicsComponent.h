#pragma once
#include "CollisionComponent.h"

namespace Krampus
{

	class PhysicsComponent : public Component
	{
	public:
		bool useGravity = true;
		bool isKinematic = false;
		float gravity = 980.f;

		float linearDamping = 0.1f;
		float angularDamping = 0.1f;
		float restitution = 0.2f;
		float staticFriction = 0.5f;
		float dynamicFriction = 0.3f;

		float maxLinearVelocity = 2000.f;
		float maxAngularVelocity = 20.f;

	private:
		float mass = 1.f;
		float inverseMass = 1.f;
		float inertia = 1.f;
		float inverseInertia = 1.f;

		FVector2 velocity = FVector2::Zero();
		float angularVelocity = 0.f;
		FVector2 acceleration = FVector2::Zero();
		FVector2 accumulatedForces = FVector2::Zero();
		float accumulatedTorque = 0.f;

	public:
		PhysicsComponent(Actor* _owner, const float& _mass = 1.0f);

		// Adds a continuous force to be applied during integration
		void AddForce(const FVector2& _force, const FVector2& _applyPoint = FVector2::Zero());
		// Applies an instant impulse directly modifying velocity
		void AddImpulse(const FVector2& _impulse, const FVector2& contactVector = FVector2::Zero());
		// Clears all accumulated forces after simulation
		void ClearForces();

		virtual void Tick(const float& _deltaTime) override;

		// Binds collision events to physical collision response (require a valid CollisionComponent)
		void BindCollisionResponse();
		// Handles collision response based on collision information
		void OnCollision(const CollisionInfo& _info);

	private:
		// Integrates forces, velocity, and position over time
		void Integrate(const float& _deltaTime);
		void ComputeInertia();
	};

}


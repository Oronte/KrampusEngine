#pragma once
#include "Components/CollisionComponent.h"
#include "Core/KrampusObject.h"
#include "box2d/box2d.h"

namespace Krampus
{

	/// <summary>
	/// Manages broad-phase and narrow-phase collision checks and multi-step collision resolution between colliders.
	/// </summary>
	class WorldPhysics : public KrampusObject
	{
		b2WorldId							physicsWorldID;
		std::vector<CollisionComponent*>	collisions;
		int									subStepsCount = 8;

	public:
		/// <summary>
		/// Event broadcast when all collisions have been resolved for the current frame.
		/// </summary>
		Event<>								onResolveCollision;

		/// <summary>
		/// Constructs a CollisionManager associated with the engine world.
		/// </summary>
		/// <param name="_engine">Pointer to Engine world.</param>
		WorldPhysics(Engine* _engine);

		/// <summary>
		/// Destroys the CollisionManager object and releases any associated resources.	
		/// </summary>
		~WorldPhysics();

		/// <summary>
		/// Registers a CollisionComponent to participate in collision checks.
		/// </summary>
		/// <param name="_component">Pointer to CollisionComponent.</param>
		void Register(CollisionComponent* _component);

		/// <summary>
		/// Executes multi-step collision detection and resolution across all registered colliders.
		/// </summary>
		void Update();

		/// <summary>
		/// Gets the Box2D physics world identifier.
		/// </summary>
		/// <returns>b2WorldId identifying the Box2D world.</returns>
		inline b2WorldId GetPhysicsWorldID()
		{
			return physicsWorldID;
		}

		/// <summary>
		/// Unregisters a CollisionComponent from collision processing.
		/// </summary>
		/// <param name="_toDelete">Pointer to CollisionComponent to remove.</param>
		inline void RemoveComponent(CollisionComponent* _toDelete)
		{
			std::erase_if(collisions, [&](const CollisionComponent* _component)
				{
					return _component == _toDelete;
				});
		}

		/// <summary>
		/// Sets the gravity vector for the physics world.
		/// </summary>
		/// <param name="_gravity">The gravity vector to set, in pixel units.</param>
		void SetGravity(const FVector2& _gravity)
		{
			const FVector2& _meterGravity = _gravity / PX_TO_M;
			b2World_SetGravity(physicsWorldID, b2Vec2(_meterGravity.x, _meterGravity.y));
		}

		/// <summary>
		/// Retrieves the gravity vector from the physics world and converts it to the appropriate units.
		/// </summary>
		/// <returns>A FVector2 representing the gravity vector in the converted units.</returns>
		FVector2 GetGravity()
		{
			b2Vec2 _gravity = b2World_GetGravity(physicsWorldID);
			return FVector2(_gravity.x, _gravity.y) * PX_TO_M;
		}

		/// <summary>
		/// Enables or disables the sleeping feature in the physics world.
		/// </summary>
		/// <param name="_enable">If true, enables sleeping; if false, disables sleeping.</param>
		void EnableSleeping(bool _enable)
		{
			b2World_EnableSleeping(physicsWorldID, _enable);
		}

		/// <summary>
		/// Checks if sleeping is enabled in the physics world.
		/// </summary>
		/// <returns>True if sleeping is enabled; otherwise, false.</returns>
		bool IsSleepingEnabled()
		{
			return b2World_IsSleepingEnabled(physicsWorldID);
		}

		/// <summary>
		/// Enables or disables continuous physics simulation in the physics world.
		/// </summary>
		/// <param name="_enable">A boolean value indicating whether to enable (true) or disable (false) continuous physics.</param>
		void EnableContinuousPhysics(bool _enable)
		{
			b2World_EnableContinuous(physicsWorldID, _enable);
		}
		
		/// <summary>
		/// Checks if continuous physics simulation is enabled in the physics world.
		/// </summary>
		/// <returns>True if continuous physics is enabled; otherwise, false.</returns>
		bool IsContinuousPhysicsEnabled()
		{
			return b2World_IsContinuousEnabled(physicsWorldID);
		}

		/// <summary>
		/// Enables or disables warm starting in the physics simulation.
		/// </summary>
		/// <param name="_enable">A boolean value indicating whether to enable (true) or disable (false) warm starting.</param>
		void EnableWarmStarting(bool _enable)
		{
			b2World_EnableWarmStarting(physicsWorldID, _enable);
		}

		/// <summary>
		/// Checks if warm starting is enabled for the physics world.
		/// </summary>
		/// <returns>True if warm starting is enabled; otherwise, false.</returns>
		bool IsWarmStartingEnabled()
		{
			return b2World_IsWarmStartingEnabled(physicsWorldID);
		}

		/// <summary>
		/// Sets the number of sub-steps.
		/// </summary>
		/// <param name="_count">The number of sub-steps to set.</param>
		void SetSubStepsCount(int _count)
		{
			subStepsCount = _count;
		}

		/// <summary>
		/// Returns the number of sub-steps.
		/// </summary>
		/// <returns>The current value of subStepsCount.</returns>
		NO_DISCARD int GetSubStepsCount()
		{
			return subStepsCount;
		}

		/// <summary>
		/// Sets the maximum linear speed for the physics world.
		/// </summary>
		/// <param name="_maxSpeed">The maximum linear speed to set.</param>
		void SetMaximumLinearSpeed(float _maxSpeed)
		{
			b2World_SetMaximumLinearSpeed(physicsWorldID, _maxSpeed);
		}

		/// <summary>
		/// Retrieves the maximum linear speed allowed in the physics world.
		/// </summary>
		/// <returns>The maximum linear speed as a float value.</returns>
		float GetMaximumLinearSpeed()
		{
			return b2World_GetMaximumLinearSpeed(physicsWorldID);
		}

		/// <summary>
		/// Sets the restitution threshold for the physics world.
		/// </summary>
		/// <param name="_threshold">The restitution threshold value to set.</param>
		void SetRestitutionThreshold(float _threshold)
		{
			b2World_SetRestitutionThreshold(physicsWorldID, _threshold);
		}

		/// <summary>
		/// Retrieves the restitution threshold value for the current physics world.
		/// </summary>
		/// <returns>The restitution threshold as a float value.</returns>
		float GetRestitutionThreshold()
		{
			return b2World_GetRestitutionThreshold(physicsWorldID);
		}

		/// <summary>
		/// Sets the hit event threshold for the physics world.
		/// </summary>
		/// <param name="_threshold">The threshold value to set for hit events.</param>
		void SetHitEventThreshold(float _threshold)
		{
			b2World_SetHitEventThreshold(physicsWorldID, _threshold);
		}

		/// <summary>
		/// Retrieves the hit event threshold value for the current physics world.
		/// </summary>
		/// <returns>The hit event threshold as a float value.</returns>
		float GetHitEventThreshold()
		{
			return b2World_GetHitEventThreshold(physicsWorldID);
		}

		/// <summary>
		/// Sets the contact tuning parameters for the physics world.
		/// </summary>
		/// <param name="_hertz">The frequency in Hertz for the contact solver.</param>
		/// <param name="_dampingRatio">The damping ratio for the contact solver.</param>
		/// <param name="_pushSpeed">The push speed for resolving contacts.</param>
		void SetContactTuning(float _hertz, float _dampingRatio, float _pushSpeed)
		{
			b2World_SetContactTuning(physicsWorldID, _hertz, _dampingRatio, _pushSpeed);
		}

		/// <summary>
		/// Rebuilds the static tree structure in the physics world.
		/// </summary>
		void RebuildStaticTree()
		{
			b2World_RebuildStaticTree(physicsWorldID);
		}

		/// <summary>
		/// Triggers an explosion using the provided explosion information.
		/// </summary>
		/// <param name="_info">The ExplosionInfo structure containing details about the explosion.</param>
		void Explode(ExplosionInfo _info);
	};
}
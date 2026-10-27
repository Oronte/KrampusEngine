#pragma once
#include "Components/CollisionComponent.h"
#include "Core/KrampusObject.h"

namespace Krampus
{

	/// <summary>
	/// Manages broad-phase and narrow-phase collision checks and multi-step collision resolution between colliders.
	/// </summary>
	class CollisionManager : public KrampusObject
	{
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
		CollisionManager(Engine* _engine);

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
	};

}

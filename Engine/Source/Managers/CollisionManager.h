#pragma once
#include "Components/CollisionComponent.h"
#include "Core/KrampusObject.h"

namespace Krampus
{

	class CollisionManager : public KrampusObject
	{
		std::vector<CollisionComponent*>	collisions;
		int									subStepsCount = 8;

	public:
		Event<>								onResolveCollision;

		CollisionManager(Engine* _engine);

		void Register(CollisionComponent* _component);

		void Update();

		inline void RemoveComponent(CollisionComponent* _toDelete)
		{
			std::erase_if(collisions, [&](const CollisionComponent* _component)
				{
					return _component == _toDelete;
				});
		}
	};

}


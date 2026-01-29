#pragma once
#include "Components/CollisionComponent.h"

namespace Krampus
{

	class CollisionManager
	{
		std::vector<CollisionComponent*> collisions;

	public:
		CollisionManager() = default;

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


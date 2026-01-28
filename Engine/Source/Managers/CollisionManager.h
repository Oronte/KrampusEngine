#pragma once
#include "Utilities/System/Singleton.h"
#include "Components/CollisionComponent.h"

#define M_COLLISION Krampus::CollisionManager::GetInstance()

namespace Krampus
{

	class CollisionManager : public Singleton<CollisionManager>
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


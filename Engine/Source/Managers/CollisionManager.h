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
	};

}


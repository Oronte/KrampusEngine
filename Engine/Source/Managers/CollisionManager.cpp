#include "CollisionManager.h"

void Krampus::CollisionManager::Register(CollisionComponent* _component)
{
	collisions.push_back(_component);
}

void Krampus::CollisionManager::Update()
{
	//const int& _collisionCount = CAST(int, collisions.size());
	//for (int _index = 0; _index < _collisionCount; _index++)
	//{
	//	CollisionComponent* _currentComponent = collisions[_index];
	//	for (int _i = _index + 1; _i < _collisionCount; _i++)
	//	{
	//		_currentComponent->ComputeCollision(collisions[_i]);
	//	}
	//}

	const int& _collisionCount = CAST(int, collisions.size());
	for (int _index = 0; _index < _collisionCount; _index++)
	{
		CollisionComponent* _currentComponent = collisions[_index];
		if (_currentComponent->onCollision.IsEmpty()) continue;
		for (int _i = 0; _i < _collisionCount; _i++)
		{
			if (_index == _i) continue;
			_currentComponent->ComputeCollision(collisions[_i]);
		}
	}
}

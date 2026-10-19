#include "CollisionManager.h"

#include "LevelManager.h"
#include "Core/Engine.h"
#include "GameFramework/Level.h"

Krampus::CollisionManager::CollisionManager(Engine *_engine)
	: KrampusObject(_engine)
{}

void Krampus::CollisionManager::Register(CollisionComponent* _component)
{
	collisions.push_back(_component);
}

void Krampus::CollisionManager::Update()
{
	{
		Level* _level = GetWorld()->GetLevelManager()->GetCurrentLevel();
		b2World_Step(_level->GetPhysicsWorldID(), _level->GetTimerManagerRef().GetDeltaTime(), subStepsCount);
	}

	const int& _collisionCount = CAST(int, collisions.size());
	for (int _index = 0; _index < _collisionCount; _index++)
	{
		CollisionComponent* _currentComponent = collisions[_index];
		if (!_currentComponent->IsActive()) continue;
		for (int _i = _index; _i < _collisionCount; _i++)
		{
			CollisionComponent* _otherComponent = collisions[_i];
			if (_index == _i || !_otherComponent->IsActive()) continue;
			_currentComponent->ComputeCollision(_otherComponent);
		}
	}

	onResolveCollision.Broadcast();
}

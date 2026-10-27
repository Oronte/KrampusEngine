#include "WorldPhysics.h"

#include "LevelManager.h"
#include "Core/Engine.h"
#include "GameFramework/Level.h"

Krampus::WorldPhysics::WorldPhysics(Engine *_engine)
	: KrampusObject(_engine)
{
	b2WorldDef _physicsWorld = b2DefaultWorldDef();
	_physicsWorld.gravity *= -1.0f;
	physicsWorldID = b2CreateWorld(&_physicsWorld);
}

Krampus::WorldPhysics::~WorldPhysics()
{
	b2DestroyWorld(physicsWorldID);
}

void Krampus::WorldPhysics::Register(CollisionComponent* _component)
{
	collisions.push_back(_component);
}

void Krampus::WorldPhysics::Update()
{
	{
		Level* _level = GetWorld()->GetLevelManager()->GetCurrentLevel();
		b2World_Step(physicsWorldID, _level->GetTimerManager().GetDeltaTime(), subStepsCount);
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


void Krampus::WorldPhysics::Explode(ExplosionInfo _info)
{
	b2ExplosionDef _explosionDef = b2DefaultExplosionDef();
	const FVector2& _pos = _info.position / PX_TO_M;
	_explosionDef.position = b2Vec2(_pos.x, _pos.y);
	_explosionDef.radius = _info.radius / PX_TO_M;
	_explosionDef.falloff = _info.falloff;
	_explosionDef.impulsePerLength = _info.impulsePerLength;
	_explosionDef.maskBits = CAST(uint64_t, _info.channel);

	b2World_Explode(physicsWorldID, &_explosionDef);
}
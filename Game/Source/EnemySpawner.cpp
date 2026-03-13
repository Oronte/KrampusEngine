#include "EnemySpawner.h"
#include "Enemy.h"
#include "GameFramework/Level.h"
#include "Managers/TimerManager.h"

EnemySpawner::EnemySpawner(Level* _level, Player* _player)
	: Actor(_level), player(_player)
{
	GetWorld()->GetTimerManager()->CreateTimer(this, &EnemySpawner::Spawn, spawnTime, true);
}

void EnemySpawner::Spawn()
{
	Level* _level = GetLevel();

	for (int _index = 0; _index < spawnCount; _index++)
	{
		_level->SpawnActorAt<Enemy>(Transform(spawnPoint + offset * _index, Angle()), player);
	}
}

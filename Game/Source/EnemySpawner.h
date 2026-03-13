#pragma once
#include "Actors/Actor.h"

using namespace Krampus;

class Player;

class EnemySpawner : public Actor
{
	Float		spawnTime	= 3.5f;
	Int			spawnCount	= 9;
	Player*		player		= nullptr;

public:
	FVector2	spawnPoint	= FVector2::Zero();
	FVector2	offset		= FVector2::Zero();

public:
	EnemySpawner(Level* _level, Player* _player);

	void Spawn();
};


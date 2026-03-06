#pragma once
#include "Actors/Actor.h"

using namespace Krampus;

class EnemySpawner : public Actor
{
	float spawnTime = 3.5f;
	int spawnCount = 10;
	Actor* player = nullptr;

public:
	FVector2 spawnPoint = FVector2::Zero();
	FVector2 offset = FVector2::Zero();

public:
	EnemySpawner(Level* _level, Actor* _player);

	void Spawn();
};


#include "DemoLevel.h"
#include "Player.h"
#include "Weapon.h"
#include "WeaponSocket.h"
#include "Collider2D.h"
#include "Background.h"
#include "EnemySpawner.h"

DemoLevel::DemoLevel(Krampus::Engine* _world)
	: Level(_world)
{
}

void DemoLevel::InitLevel()
{
	Player* _player = SpawnActor<Player>();
	_player->transform.position = FVector2(900, 0);

	Collider2D* _floor = SpawnActor<Collider2D>(FVector2(1500, 400));
	_floor->GetComponent<CollisionComponent>()->useDebug = true;
	_floor->transform.position = FVector2(900, 300);

	Collider2D* _leftWall = SpawnActor<Collider2D>(FVector2(100, 400));
	_leftWall->GetComponent<CollisionComponent>()->useDebug = true;
	_leftWall->transform.position = FVector2(200, -100);

	Collider2D* _rightWall = SpawnActor<Collider2D>(FVector2(100, 400));
	_rightWall->GetComponent<CollisionComponent>()->useDebug = true;
	_rightWall->transform.position = FVector2(1600, -100);

	Weapon* _weapon = SpawnActor<Weapon>();

	WeaponSocket* _weaponSocket = SpawnActor<WeaponSocket>();

	_weapon->transform.position = FVector2(75.0f, 0.0f);
	_weaponSocket->AddChild(_weapon);
	_weaponSocket->SetGraber(_player);

	EnemySpawner* _spawner = SpawnActor<EnemySpawner>(_player);
	_spawner->spawnPoint = FVector2(400.0f, -500.0f);
	_spawner->offset = FVector2(200.0f, 0.0f);

	Background* _background = SpawnActor<Background>();
	_background->transform.position = FVector2(900, -400);
}

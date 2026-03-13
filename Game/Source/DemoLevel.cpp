#include "DemoLevel.h"
#include "Player.h"
#include "Weapon.h"
#include "WeaponSocket.h"
#include "Collider2D.h"
#include "Background.h"
#include "EnemySpawner.h"
#include "HealthWidget.h"
#include "Managers/AudioManager.h"
#include "ScoreLabelWidget.h"

DemoLevel::DemoLevel(Krampus::Engine* _world)
	: Level(_world)
{
}

void DemoLevel::InitLevel()
{
	Super::InitLevel();

	// Exemple Log

	LOG_MSG("This is a message log.");
	LOG_WARNING("This is a warning log.");
	LOG_ERROR("This is a error log.");
	//LOG_FATAL("This is a fatal log."); // This will throw an exeption
	Logger::Clear();

	// Game Objects
	Player* _player = SpawnActor<Player>();
	_player->transform.position = FVector2(900, 0);

	HealthWidget* _healthWidget = SpawnWidget<HealthWidget>(_player->GetComponent<HealthComponent>());
	_healthWidget->SetScreenPosition(FVector2(150.0f, 850.0f));
	_healthWidget->transform.scale = FVector2(3.0f);

	ScoreLabelWidget* _scoreWidget = SpawnWidget<ScoreLabelWidget>(_player);
	_scoreWidget->SetScreenPosition(FVector2(1920 / 2, 150));
	_scoreWidget->transform.scale = FVector2(2.0f);

	Collider2D* _floor = SpawnActor<Collider2D>(FVector2(1500, 400));
	_floor->GetComponent<CollisionComponent>()->UseDebug();
	_floor->transform.position = FVector2(900, 300);

	Collider2D* _leftWall = SpawnActor<Collider2D>(FVector2(100, 400));
	_leftWall->GetComponent<CollisionComponent>()->UseDebug();
	_leftWall->transform.position = FVector2(200, -100);

	Collider2D* _rightWall = SpawnActor<Collider2D>(FVector2(100, 400));
	_rightWall->GetComponent<CollisionComponent>()->UseDebug();
	_rightWall->transform.position = FVector2(1600, -100);

	Weapon* _weapon = SpawnActor<Weapon>(_player);

	WeaponSocket* _weaponSocket = SpawnActor<WeaponSocket>(_player);

	_weapon->transform.position = FVector2(75.0f, 0.0f);
	_weaponSocket->AddChild(_weapon);
	_weaponSocket->SetGraber(_player);

	EnemySpawner* _spawner = SpawnActor<EnemySpawner>(_player);
	_spawner->spawnPoint = FVector2(400.0f, -500.0f);
	_spawner->offset = FVector2(200.0f, 0.0f);

	Background* _background = SpawnActor<Background>();
	_background->transform.position = FVector2(900, -400);

	backgroundMusic = M_AUDIO.CreateMusic("EpicMusic", AudioExtensionType::MP3);
	backgroundMusic.SetLoop(true);
	backgroundMusic.SetVolume(75.0f);
	backgroundMusic.Play();
}

void DemoLevel::Unload()
{
	Super::Unload();
	backgroundMusic.Stop();
}

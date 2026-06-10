#include "Level.h"
#include "Managers/LevelManager.h"
#include "Managers/CollisionManager.h"
#include "Actors/CameraActor.h"

using namespace Krampus;

Krampus::Level::Level(Engine* _engine)
	: KrampusObject(_engine), cameraManager(_engine), timerManager(_engine) { }

void Level::InitLevel()
{
	mainCamera = SpawnActor<CameraActor>();
	mainCamera->SetActorPosition(GetWindowRef().GetSize() / 2.0f);
}

void Level::Load()
{
	if (isLoaded) return;

	InitLevel();
	isLoaded = true;

	actorManager.BeginPlay();
	hud.BeginPlay();
}

void Krampus::Level::Unload()
{
	if (!isLoaded) return;
	isLoaded = false;
	GetWorld()->GetWindowRef().Clear(backgroundColor);
	actorManager.Deconstruct();
	actorManager.BeginDestroy();
	hud.Deconstruct();
	hud.BeginDestroy();
}

void Level::Update(const Float& _deltaTime)
{
	GetWorld()->GetWindowRef().Clear(backgroundColor);
	collisionManager.Update(); 
	actorManager.Update(_deltaTime);
	cameraManager.UpdateView();
	hud.Update(_deltaTime);
	actorManager.DestroyPendingDeleteActors();
	hud.DestroyPendingDeleteWidgets();

	Render();
}

Float Krampus::Level::UpdateTime()
{
	return timerManager.Update();
}

CameraComponent* Level::GetDefaultCamera() const
{
	return mainCamera->GetComponent<CameraComponent>();
}

void Krampus::Level::Render()
{
	cameraManager.Render(false); // TODO SplitScreen
	GetWorld()->GetWindowRef().Display();
}

#include "Level.h"
#include "Managers/LevelManager.h"
#include "Managers/WorldPhysics.h"
#include "Actors/CameraActor.h"

using namespace Krampus;

Krampus::Level::Level(Engine* _engine)
	: KrampusObject(_engine), cameraManager(_engine), timerManager(_engine), worldPhysics(_engine)
{
}

void Level::InitLevel()
{
	mainCamera = SpawnActor<CameraActor>();
	mainCamera->SetPosition(GetWindow().GetSize() / 2.0f);
}

void Level::Load()
{
	if (isLoaded) return;

	InitLevel();
	isLoaded = true;

	actorManager.BeginPlay();
	hud.BeginPlay();
	LOG_MSG("Level: loaded " + std::string(typeid(*this).name()) + " (" + std::to_string(actorManager.GetActors().size()) + " actors, " + std::to_string(hud.GetWidgets().size()) + " widgets)");
}

void Krampus::Level::Unload()
{
	if (!isLoaded) return;
	isLoaded = false;
	LOG_MSG("Level: unloading " + std::string(typeid(*this).name()));
	GetWorld()->GetWindowRef().Clear(backgroundColor);
	actorManager.Deconstruct();
	actorManager.BeginDestroy();
	hud.Deconstruct();
	hud.BeginDestroy();
}

void Level::Update(const Float& _deltaTime)
{
	actorManager.Update(_deltaTime);
	worldPhysics.Update();
	cameraManager.UpdateView();
	hud.Update(_deltaTime);
	actorManager.DestroyPendingDeleteActors();
	hud.DestroyPendingDeleteWidgets();
}

void Level::Render()
{
	GetWorld()->GetWindowRef().Clear(backgroundColor);
	cameraManager.Render(false); // TODO SplitScreen
}

Float Krampus::Level::UpdateTime()
{
	return timerManager.Update();
}

CameraComponent* Level::GetDefaultCamera() const
{
	return mainCamera->GetComponent<CameraComponent>();
}

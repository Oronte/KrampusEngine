#include "Level.h"
#include "Managers/LevelManager.h"
#include "Managers/CollisionManager.h"
#include <SFML/OpenGL.hpp>

using namespace Krampus;

Krampus::Level::Level(Engine* _engine)
	: KrampusObject(_engine), cameraManager(_engine) { }

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
	actorManager.BeginDestroy();
	hud.BeginDestroy();
}

void Level::Update(const float& _deltaTime)
{
	GetWorld()->GetWindowRef().Clear(backgroundColor);
	collisionManager.Update(); 
	actorManager.Update(_deltaTime);
	cameraManager.UpdateView();
	hud.Update(_deltaTime);
	actorManager.DestroyPendingDeleteActors();

	Render();
}

void Krampus::Level::Render()
{
	cameraManager.Render(false); // TODO SplitScreen
	GetWorld()->GetWindowRef().Display();
}

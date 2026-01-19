#include "Level.h"
#include "Graphics/Window/MainWindow.h"
#include "Managers/LevelManager.h"
#include "Managers/CollisionManager.h"

using namespace Krampus;

void Level::Load()
{
	if (isLoaded) return;

	InitLevel();
	isLoaded = true;

	actorManager.BeginPlay();
	hud.BeginPlay();
	LOG_MSG("Load");
}

void Krampus::Level::Unload()
{
	if (!isLoaded) return;
	isLoaded = false;
	MAIN_WINDOW.Clear(backgroundColor);
	actorManager.BeginDestroy();
	hud.BeginDestroy();
	LOG_MSG("Unload");
}

void Level::Update(const float& _deltaTime)
{
	MAIN_WINDOW.Clear(backgroundColor);
	M_COLLISION.Update(); 
	actorManager.Update(_deltaTime);
	cameraManager.UpdateView();
	hud.Update(_deltaTime);

	Render();
}

void Krampus::Level::Render()
{
	cameraManager.Render(MAIN_WINDOW, false); // TODO SplitScreen
	MAIN_WINDOW.Display();
}

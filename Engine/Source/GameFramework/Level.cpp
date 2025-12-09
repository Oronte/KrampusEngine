#include "Level.h"
#include "Graphics/Window/MainWindow.h"
#include "Managers/LevelManager.h"

using namespace Krampus;


Krampus::Level::Level(const std::string& _name)
{
	name = _name;

	M_LEVEL.RegisterLevel(this);
}

void Krampus::Level::InitLevel()
{
}

void Level::Load()
{
	if (isLoaded) return;

	InitLevel();
	isLoaded = true;

	actorManager.BeginPlay();
}

void Krampus::Level::Unload()
{
	if (!isLoaded) return;
	isLoaded = false;
	MAIN_WINDOW.Clear(backgroundColor);
	actorManager.BeginDestroy();
}

void Level::Update(const float& _deltaTime)
{
	MAIN_WINDOW.Clear(backgroundColor);
	cameraManager.Render(MAIN_WINDOW, false); // TODO SplitScreen
	MAIN_WINDOW.Display();
	actorManager.Update(_deltaTime);
}

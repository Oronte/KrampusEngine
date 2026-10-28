#include "GameEntry.h"
#include "Core/Engine.h"
#include "Scene/Scene.h"

// Levels are scene files (Content/Scenes/*.ka7), made in the editor. The game starts on the scene chosen in Settings > Ship.
void Krampus::GameStart(Engine& _engine)
{
	Scene::Open(_engine, _engine.GetSettings().ship.startScene);
}

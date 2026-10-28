#include "EditorContext.h"
#include "Scene/Scene.h"
#include "Managers/LevelManager.h"

using namespace Krampus;

void Krampus::Play(EditorContext& _context)
{
	if (_context.playing) return;

	// Whatever the game does while it runs, Stop gets the scene back as it is now.
	_context.hasSnapshot = false;
	if (Level* _level = _context.GetLevel())
	{
		if (Cast<SceneLevel>(_level))
		{
			std::filesystem::create_directories(PLAY_SNAPSHOT_PATH.parent_path());
			_context.hasSnapshot = Scene::Save(*_level, PLAY_SNAPSHOT_PATH);
		}
	}

	_context.playing = true;
	_context.engine->SetLevelPaused(false);
}

void Krampus::Stop(EditorContext& _context)
{
	if (!_context.playing) return;

	_context.playing = false;
	_context.engine->SetLevelPaused(true);
	_context.selected = nullptr;
	_context.selectedWidget = nullptr;

	// The level is swapped at the start of the next frame.
	if (_context.hasSnapshot)
		_context.engine->GetLevelManager()->SetLevel<SceneLevel>()->SetScenePath(PLAY_SNAPSHOT_PATH);
	else
		_context.engine->GetLevelManager()->ReloadLevel(); // level built by code: start it again

	_context.hasSnapshot = false;
}

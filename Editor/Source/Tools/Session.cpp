#include "EditorContext.h"
#include "Scene/Scene.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

using namespace Krampus;

// When the game is rebuilt the host reloads the module and the level would come back as the first scene: the scene being edited
// is saved just before the reload (with what was not saved yet) and opened again right after.

bool Krampus::IsHotReloading()
{
	return GetEnvironmentVariableW(L"KRAMPUS_HOT_RELOAD", nullptr, 0) > 0;
}

void Krampus::SaveSession(EditorContext& _context)
{
	Level* _level = _context.GetLevel();
	if (!_level || !Cast<SceneLevel>(_level)) return;

	std::error_code _error;
	std::filesystem::create_directories(RELOAD_BACKUP_PATH.parent_path(), _error);

	bool _saved = false;
	if (_context.playing && _context.hasSnapshot)
		_saved = std::filesystem::copy_file(PLAY_SNAPSHOT_PATH, RELOAD_BACKUP_PATH, std::filesystem::copy_options::overwrite_existing, _error);	// reload during Play: back to the edited scene
	else if (!_context.playing)
		_saved = Scene::Save(*_level, RELOAD_BACKUP_PATH);
	if (!_saved) return;

	std::ofstream _file(SESSION_FILE, std::ios::trunc);
	_file << _context.scenePath.generic_string() << "\n"
		<< _context.cameraCenter.x << " " << _context.cameraCenter.y << " " << _context.cameraZoom << "\n";	// the editor camera survives too
}

void Krampus::RestoreSession(EditorContext& _context)
{
	if (!IsHotReloading()) return;
	if (!std::filesystem::exists(RELOAD_BACKUP_PATH) || !std::filesystem::exists(SESSION_FILE)) return;

	std::ifstream _file(SESSION_FILE);
	std::string _scene;
	std::getline(_file, _scene);

	float _x = 0.f, _y = 0.f, _zoom = 1.f;
	if (_file >> _x >> _y >> _zoom)
	{
		_context.cameraCenter = FVector2(_x, _y);
		_context.cameraZoom = _zoom;
		_context.cameraReady = true;
	}

	_context.scenePath = _scene;
	Scene::Open(*_context.engine, RELOAD_BACKUP_PATH.generic_string());
	LOG_MSG("Game reloaded: scene " + (_scene.empty() ? std::string("(not saved)") : _scene) + " restored with its unsaved changes.");
}

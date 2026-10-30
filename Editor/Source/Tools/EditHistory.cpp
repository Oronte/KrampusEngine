#include "EditorContext.h"
#include "Scene/Scene.h"
#include "Managers/LevelManager.h"
#include "Components/CameraComponent.h"
#include "UI/Widgets/Widget.h"
#include <imgui.h>

using namespace Krampus;

std::filesystem::path EditHistory::Directory()
{
	return EDITOR_DATA_DIRECTORY / "_undo";
}

void EditHistory::Forget(std::vector<Entry>& _stack)
{
	std::error_code _error;
	for (const Entry& _entry : _stack)
		std::filesystem::remove(_entry.file, _error);
	_stack.clear();
}

void EditHistory::Clear()
{
	std::error_code _error;
	std::filesystem::remove_all(Directory(), _error);
	undoStack.clear();
	redoStack.clear();
	lastKey.clear();
	pending = false;
}

std::filesystem::path EditHistory::Snapshot(EditorContext& _context)
{
	Level* _level = _context.GetLevel();
	if (!_level) return { };

	std::error_code _error;
	std::filesystem::create_directories(Directory(), _error);

	char _name[32];
	std::snprintf(_name, sizeof(_name), "%06d.ka7", ++counter);
	const std::filesystem::path _file = Directory() / _name;
	return Scene::Save(*_level, _file, true) ? _file : std::filesystem::path();
}

EditHistory::Selection EditHistory::SelectionOf(EditorContext& _context)
{
	Selection _selection;
	Level* _level = _context.GetLevel();
	if (!_level) return _selection;

	if (_context.selected)
	{
		int _index = 0;
		for (const std::unique_ptr<Actor>& _actor : _level->GetActorManager().GetActors())
		{
			if (_actor.get() == _context.selected) { _selection = { 1, _index }; break; }
			++_index;
		}
	}
	else if (_context.selectedWidget)
	{
		int _index = 0;
		for (const std::unique_ptr<Widget>& _widget : _level->GetHUD().GetWidgets())
		{
			if (_widget.get() == _context.selectedWidget) { _selection = { 2, _index }; break; }
			++_index;
		}
	}
	return _selection;
}

void EditHistory::Record(EditorContext& _context, const std::string& _label, const std::string& _key)
{
	if (_context.playing || pending) return;

	Level* _level = _context.GetLevel();
	if (!_level || !Cast<SceneLevel>(_level)) return;

	const double _now = ImGui::GetTime();
	if (!_key.empty() && _key == lastKey && _now - lastKeyTime < 1.2)
	{
		lastKeyTime = _now;	// the same change goes on: still one step
		return;
	}
	lastKey = _key;
	lastKeyTime = _now;

	Entry _entry;
	_entry.file = Snapshot(_context);
	if (_entry.file.empty()) return;
	_entry.label = _label;
	_entry.selection = SelectionOf(_context);

	undoStack.push_back(_entry);
	++_context.sceneChanges;	// unsaved changes now
	Forget(redoStack);	// a new change ends the redo line

	if (undoStack.size() > MAX_STEPS)
	{
		std::error_code _error;
		std::filesystem::remove(undoStack.front().file, _error);
		undoStack.erase(undoStack.begin());
	}
	LOG(VerbosityType::Verbose, "History: step '" + _label + "' (" + std::to_string(undoStack.size()) + " in the undo list)");
}

void EditHistory::Restore(EditorContext& _context, const Entry& _entry)
{
	Level* _level = _context.GetLevel();
	LevelManager* _levels = _context.engine->GetLevelManager();

	_levels->SetNextChangeSilent();
	_levels->SetLevel<SceneLevel>()->SetScenePath(_entry.file);

	pending = true;
	pendingSerial = _levels->GetLevelSerial();
	pendingSelection = _entry.selection;
	pendingFile = _entry.file;
	_context.selected = nullptr;
	_context.selectedWidget = nullptr;
	lastKey.clear();
}

bool EditHistory::Undo(EditorContext& _context)
{
	if (!CanUndo() || _context.playing) return false;

	const Entry _target = undoStack.back();
	undoStack.pop_back();

	Entry _now;
	_now.file = Snapshot(_context);
	_now.label = _target.label;
	_now.selection = SelectionOf(_context);
	if (!_now.file.empty()) redoStack.push_back(_now);
	++_context.sceneChanges;

	LOG_MSG("Undo: " + _target.label);
	Restore(_context, _target);
	return true;
}

bool EditHistory::Redo(EditorContext& _context)
{
	if (!CanRedo() || _context.playing) return false;

	const Entry _target = redoStack.back();
	redoStack.pop_back();

	Entry _now;
	_now.file = Snapshot(_context);
	_now.label = _target.label;
	_now.selection = SelectionOf(_context);
	if (!_now.file.empty()) undoStack.push_back(_now);
	++_context.sceneChanges;

	LOG_MSG("Redo: " + _target.label);
	Restore(_context, _target);
	return true;
}

void EditHistory::Update(EditorContext& _context)
{
	if (!pending) return;
	LevelManager* _levels = _context.engine->GetLevelManager();
	if (_levels->GetLevelSerial() <= pendingSerial) return;	// the swap happens at the start of a frame
	pending = false;

	Level* _level = _context.GetLevel();
	if (_level)
	{
		if (pendingSelection.kind == 1)
		{
			const auto& _actors = _level->GetActorManager().GetActors();
			if (pendingSelection.index >= 0 && pendingSelection.index < static_cast<int>(_actors.size()))
				_context.selected = _actors[pendingSelection.index].get();
		}
		else if (pendingSelection.kind == 2)
		{
			const auto& _widgets = _level->GetHUD().GetWidgets();
			if (pendingSelection.index >= 0 && pendingSelection.index < static_cast<int>(_widgets.size()))
				_context.selectedWidget = _widgets[pendingSelection.index].get();
		}
	}

	std::error_code _error;
	std::filesystem::remove(pendingFile, _error);	// the scene that was put back: the level is made of it now
}

#include "EditorContext.h"
#include "Scene/Scene.h"
#include "Managers/LevelManager.h"
#include "Shortcuts.h"
#include <imgui.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

using namespace Krampus;

Level* EditorContext::GetLevel() const
{
	return engine->GetLevelManager()->GetCurrentLevel<Level>();
}

sf::View EditorContext::GetCameraView(const sf::Vector2f& _windowSize) const
{
	const float _zoom = std::max(cameraZoom, 0.01f);
	return sf::View(sf::Vector2f(cameraCenter.x, cameraCenter.y), sf::Vector2f(_windowSize.x / _zoom, _windowSize.y / _zoom));
}

void Krampus::NewScene(EditorContext& _context)
{
	OpenScene(_context, std::filesystem::path());
}

void Krampus::OpenScene(EditorContext& _context, const std::filesystem::path& _path)
{
	_context.history.Clear();	// the steps of another scene do not apply
	LOG(VerbosityType::Log, _path.empty() ? std::string("Editor: new scene") : "Editor: opening scene " + _path.generic_string());
	// The level is swapped at the start of the next frame, so this is safe to call from a panel.
	_context.engine->GetLevelManager()->SetLevel<SceneLevel>()->SetScenePath(_path);
	_context.scenePath = _path;
	_context.selected = nullptr;
	_context.selectedWidget = nullptr;

	// Opening a scene ends Play mode: the new level starts frozen.
	_context.playing = false;
	_context.hasSnapshot = false;
	_context.engine->SetLevelPaused(true);
}

bool Krampus::SaveScene(EditorContext& _context, const std::filesystem::path& _path)
{
	Level* _level = _context.GetLevel();
	LOG(VerbosityType::Log, "Editor: saving scene to " + _path.generic_string());
	if (!_level || !Scene::Save(*_level, _path)) return false;

	_context.scenePath = _path;
	return true;
}

void Krampus::DrawMainMenuBar(EditorContext& _context)
{
	// The scene being edited is the one the level was made from (the game opens its first scene by itself).
	if (!_context.playing)
		if (const SceneLevel* _sceneLevel = Cast<SceneLevel>(_context.GetLevel()))
			if (_sceneLevel->GetScenePath() != _context.scenePath && _sceneLevel->GetScenePath() != PLAY_SNAPSHOT_PATH && _sceneLevel->GetScenePath() != RELOAD_BACKUP_PATH && _sceneLevel->GetScenePath().parent_path() != EditHistory::Directory() && !_sceneLevel->GetScenePath().empty())
				_context.scenePath = _sceneLevel->GetScenePath();

	static char _saveAsName[128] = "";
	bool _openSaveAs = false;

	PumpCompile();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Scene"))
				NewScene(_context);

			if (ImGui::BeginMenu("Open Scene"))
			{
				std::error_code _error;
				bool _any = false;
				for (const auto& _entry : std::filesystem::directory_iterator(SCENES_DIRECTORY, _error))
				{
					if (_entry.path().extension() != Scene::EXTENSION) continue;
					_any = true;
					if (ImGui::MenuItem(_entry.path().stem().string().c_str()))
						OpenScene(_context, _entry.path());
				}
				if (!_any) ImGui::TextDisabled("No scene in Content/Scenes");
				ImGui::EndMenu();
			}

			ImGui::Separator();
			if (ImGui::MenuItem("Save Scene", Shortcuts::Text("Save Scene").c_str()))
			{
				if (_context.scenePath.empty()) _openSaveAs = true;
				else SaveScene(_context, _context.scenePath);
			}
			if (ImGui::MenuItem("Save Scene As..."))
				_openSaveAs = true;

			ImGui::Separator();
			if (ImGui::MenuItem("Package Game (folder for another PC)", Shortcuts::Text("Package Game").c_str(), false, !IsCompiling()))
				StartShip();
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
				ImGui::SetTooltip("Builds the engine and the game in Ship mode (no editor, optimized) and makes a folder that runs on any Windows PC, nothing to install: Bin/windows-x64/Ship/Package.");
			const std::filesystem::path _package = GetPackageDirectory();
			if (ImGui::MenuItem("Open Package Folder", nullptr, false, std::filesystem::exists(_package)))
				ShellExecuteW(nullptr, L"open", std::filesystem::absolute(_package).c_str(), nullptr, nullptr, SW_SHOWNORMAL);

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			const EditHistory& _history = _context.history;
			const std::string _undoText = _history.CanUndo() ? "Undo " + _history.UndoLabel() : std::string("Undo");
			const std::string _redoText = _history.CanRedo() ? "Redo " + _history.RedoLabel() : std::string("Redo");
			if (ImGui::MenuItem(_undoText.c_str(), Shortcuts::Text("Undo").c_str(), false, _history.CanUndo() && !_context.playing))
				_context.history.Undo(_context);
			if (ImGui::MenuItem(_redoText.c_str(), Shortcuts::Text("Redo").c_str(), false, _history.CanRedo() && !_context.playing))
				_context.history.Redo(_context);
			ImGui::Separator();
			if (ImGui::MenuItem("Delete Selection", Shortcuts::Text("Delete Selection").c_str(), false, (_context.selected || _context.selectedWidget) && !_context.playing))
				DeleteSelection(_context);
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Tools"))
		{
			if (ImGui::MenuItem("Animation Editor"))
				OpenAnimationEditor();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Settings"))
		{
			const std::vector<SettingsSection*>& _sections = _context.engine->GetSettings().GetSections();
			for (size_t _i = 0; _i < _sections.size(); ++_i)
			{
				if (!ImGui::MenuItem(_sections[_i]->Title())) continue;
				_context.settingsOpen = true;
				_context.settingsPage = static_cast<int>(_i);
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Editor"))
			{
				_context.settingsOpen = true;
				_context.settingsPage = static_cast<int>(_sections.size());
			}
			if (ImGui::MenuItem("Shortcuts"))
			{
				_context.settingsOpen = true;
				_context.settingsPage = static_cast<int>(_sections.size()) + 1;
			}
			if (ImGui::MenuItem("Colors"))
			{
				_context.settingsOpen = true;
				_context.settingsPage = static_cast<int>(_sections.size()) + 2;
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Level"))
			{
				_context.settingsOpen = true;
				_context.settingsPage = static_cast<int>(_sections.size()) + 3;
			}
			ImGui::EndMenu();
		}

		// Play / Stop, centered.
		const float _buttonsWidth = 120.f;
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - _buttonsWidth) * 0.5f);
		ImGui::BeginDisabled(_context.playing);
		if (ImGui::Button("Play", ImVec2(56.f, 0.f))) Play(_context);
		ImGui::EndDisabled();
		ImGui::BeginDisabled(!_context.playing);
		if (ImGui::Button("Stop", ImVec2(56.f, 0.f))) Stop(_context);
		ImGui::EndDisabled();

		const std::string _title = _context.scenePath.empty() ? "Untitled scene (not saved)" : _context.scenePath.string();
		ImGui::SameLine(0.f, 24.f);
		ImGui::TextDisabled("%s", _title.c_str());

		ImGui::SameLine(std::max(ImGui::GetCursorPosX() + 16.f, ImGui::GetWindowWidth() - 270.f));
		ImGui::BeginDisabled(IsCompiling());
		if (ImGui::Button(IsCompiling() ? (IsShipping() ? "Packaging..." : "Compiling...") : "Compile", ImVec2(100.f, 0.f)))
			StartCompile();
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Builds the game (EditorModule). When it is done the editor reloads it by itself, with your scene.");
		ImGui::SameLine();
		if (ImGui::Button("Open Solution"))
			OpenSolution();
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Open the Visual Studio solution. Build EditorModule there (Ctrl+B): the editor reloads the game by itself.");
		ImGui::EndMainMenuBar();
	}

	if (Shortcuts::Pressed("Save Scene"))
	{
		if (_context.scenePath.empty()) _openSaveAs = true;
		else SaveScene(_context, _context.scenePath);
	}

	if (_openSaveAs)
		ImGui::OpenPopup("Save Scene As");

	if (ImGui::BeginPopupModal("Save Scene As", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::InputText("Name", _saveAsName, sizeof(_saveAsName));
		ImGui::TextDisabled("Saved in Content/Scenes as <name>.ka7");

		const bool _valid = _saveAsName[0] != '\0';
		ImGui::BeginDisabled(!_valid);
		if (ImGui::Button("Save"))
		{
			SaveScene(_context, SCENES_DIRECTORY / (std::string(_saveAsName) + Scene::EXTENSION));
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

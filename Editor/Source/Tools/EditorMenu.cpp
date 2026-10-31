#include "EditorContext.h"
#include "Scene/Scene.h"
#include "Managers/LevelManager.h"
#include "Shortcuts.h"
#include "EditorTheme.h"
#include "GameFramework/Level.h"
#include "Actors/Actor.h"
#include "UI/Widgets/Widget.h"
#include "Graphics/Particles/ParticleEmitter.h"
#include <imgui.h>
#include <imgui_internal.h>

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

void Krampus::RequestOpenScene(EditorContext& _context, const std::filesystem::path& _path)
{
	if (_context.SceneDirty())
	{
		_context.pendingScene = _path;
		_context.askUnsaved = true;
		return;
	}
	OpenScene(_context, _path);
}

void Krampus::OpenScene(EditorContext& _context, const std::filesystem::path& _path)
{
	_context.history.Clear();	// the steps of another scene do not apply
	_context.sceneChanges = _context.savedChanges = 0;
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
	// While playing, the level holds what the game did (spawned actors, moved things): that is not the scene.
	if (_context.playing)
	{
		LOG_WARNING("Editor: stop Play before saving the scene (what the game changed while playing is not part of the scene)");
		return false;
	}

	Level* _level = _context.GetLevel();
	LOG(VerbosityType::Log, "Editor: saving scene to " + _path.generic_string());
	if (!_level || !Scene::Save(*_level, _path)) return false;

	_context.scenePath = _path;
	_context.savedChanges = _context.sceneChanges;
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

	// While the game runs the menu bar takes the Play color (darkened): the editor is not editing the scene then.
	const ImVec4 _playColor = EditorTheme::Get(EditorTheme::Custom::PlayMode);
	ImGui::PushStyleColor(ImGuiCol_MenuBarBg, _context.playing ? ImVec4(_playColor.x * 0.35f, _playColor.y * 0.35f, _playColor.z * 0.35f, 1.f) : ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Scene"))
				RequestOpenScene(_context, std::filesystem::path());

			if (ImGui::BeginMenu("Open Scene"))
			{
				std::error_code _error;
				bool _any = false;
				for (const auto& _entry : std::filesystem::directory_iterator(SCENES_DIRECTORY, _error))
				{
					if (_entry.path().extension() != Scene::EXTENSION) continue;
					_any = true;
					if (ImGui::MenuItem(_entry.path().stem().string().c_str()))
						RequestOpenScene(_context, _entry.path());
				}
				if (!_any) ImGui::TextDisabled("No scene in Content/Scenes");
				ImGui::EndMenu();
			}

			ImGui::Separator();
			if (ImGui::MenuItem(_context.SceneDirty() ? "Save Scene *" : "Save Scene", Shortcuts::Text("Save Scene").c_str(), false, !_context.playing))
			{
				if (_context.scenePath.empty()) _openSaveAs = true;
				else SaveScene(_context, _context.scenePath);
			}
			if (ImGui::MenuItem("Save Scene As...", nullptr, false, !_context.playing))
				_openSaveAs = true;
			if (_context.playing) ImGui::TextDisabled("Stop Play to save the scene");

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
			if (ImGui::MenuItem("Network"))
				OpenNetworkWindow();
			if (ImGui::MenuItem("Animation Editor"))
				OpenAnimationEditor();
			if (ImGui::MenuItem("Animation Graph"))
				OpenAnimationGraph();
			if (ImGui::MenuItem("Shape Editor"))
				OpenShapeEditor();
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

		// One Play / Stop button, centered: green to start, Play color to stop.
		{
			const std::string _play = std::string(EditorTheme::Symbol("\xE2\x96\xB6  ", "")) + "Play";	// play triangle
			const std::string _stop = std::string(EditorTheme::Symbol("\xE2\x96\xA0  ", "")) + "Stop";	// square
			const float _width = std::max(ImGui::CalcTextSize(_play.c_str()).x, ImGui::CalcTextSize(_stop.c_str()).x) + ImGui::GetStyle().FramePadding.x * 4.f;
			ImGui::SetCursorPosX((ImGui::GetWindowWidth() - _width) * 0.5f);

			const ImVec4 _color = _context.playing ? _playColor : EditorTheme::Get(EditorTheme::Custom::PlayButton);
			ImGui::PushStyleColor(ImGuiCol_Button, _color);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(std::min(1.f, _color.x * 1.15f), std::min(1.f, _color.y * 1.15f), std::min(1.f, _color.z * 1.15f), 1.f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(_color.x * 0.85f, _color.y * 0.85f, _color.z * 0.85f, 1.f));
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
			if (ImGui::Button(_context.playing ? _stop.c_str() : _play.c_str(), ImVec2(_width, 0.f)))
			{
				if (_context.playing) Stop(_context);
				else Play(_context);
			}
			ImGui::PopStyleColor(4);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(_context.playing ? "Stop the game and go back to the scene as it was before Play (%s)" : "Run the scene (%s). Stop brings it back as it was.", Shortcuts::Text("Play / Stop").c_str());
		}

		const std::string _title = _context.scenePath.empty() ? "Untitled scene" : _context.scenePath.stem().string();
		ImGui::SameLine(0.f, 24.f);
		ImGui::TextDisabled("%s%s", _title.c_str(), _context.SceneDirty() ? " *" : "");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s%s", _context.scenePath.empty() ? "Not saved yet (File > Save Scene As)" : _context.scenePath.generic_string().c_str(), _context.SceneDirty() ? "\nUnsaved changes" : "");

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
	ImGui::PopStyleColor();

	if (Shortcuts::Pressed("Save Scene"))
	{
		if (_context.playing) LOG_WARNING("Editor: stop Play before saving the scene");
		else if (_context.scenePath.empty()) _openSaveAs = true;
		else SaveScene(_context, _context.scenePath);
	}

	if (_openSaveAs)
		ImGui::OpenPopup("Save Scene As");

	if (ImGui::BeginPopupModal("Save Scene As", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::InputText("Name", _saveAsName, sizeof(_saveAsName));
		ImGui::TextDisabled("Saved in Content/Scenes as <name>.ka7");

		// A plain file name: no folder, no character Windows refuses, nothing that ends in a dot or a space.
		const std::string _name = _saveAsName;
		const bool _valid = !_name.empty() && _name.find_first_of("\\/:*?\"<>|") == std::string::npos && _name.back() != '.' && _name.back() != ' '
			&& _name.front() != ' ' && _name != "." && _name != "..";
		const std::filesystem::path _target = SCENES_DIRECTORY / (_name + Scene::EXTENSION);
		std::error_code _error;
		const bool _exists = _valid && std::filesystem::exists(_target, _error) && _target != _context.scenePath;
		if (!_name.empty() && !_valid) ImGui::TextColored(ImVec4(1.f, 0.5f, 0.3f, 1.f), "A file name cannot contain \\ / : * ? \" < > |");
		if (_exists) ImGui::TextColored(ImVec4(1.f, 0.8f, 0.3f, 1.f), "A scene named \"%s\" exists: it will be replaced", _name.c_str());

		ImGui::BeginDisabled(!_valid);
		if (ImGui::Button(_exists ? "Replace" : "Save"))
		{
			SaveScene(_context, _target);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	// ---- Unsaved changes: before another scene replaces this one, and before the editor closes.
	if (_context.askUnsaved)
	{
		ImGui::OpenPopup("Unsaved scene");
		_context.askUnsaved = false;
	}
	if (_context.askQuit)
	{
		ImGui::OpenPopup("Quit the editor");
		_context.askQuit = false;
	}
	const std::string _sceneName = _context.scenePath.empty() ? std::string("The new scene") : "\"" + _context.scenePath.stem().string() + "\"";
	if (ImGui::BeginPopupModal("Unsaved scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("%s has changes that are not saved.", _sceneName.c_str());
		ImGui::BeginDisabled(_context.scenePath.empty() || _context.playing);
		if (ImGui::Button("Save") && SaveScene(_context, _context.scenePath))
		{
			OpenScene(_context, _context.pendingScene);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("Don't save"))
		{
			OpenScene(_context, _context.pendingScene);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
		if (_context.scenePath.empty()) ImGui::TextDisabled("Never saved: use File > Save Scene As first to keep it");
		if (_context.playing) ImGui::TextDisabled("Stop Play to save it");
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Quit the editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("%s has changes that are not saved.", _sceneName.c_str());
		ImGui::BeginDisabled(_context.scenePath.empty() || _context.playing);
		if (ImGui::Button("Save and quit") && SaveScene(_context, _context.scenePath))
		{
			_context.engine->QuitGame();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("Quit without saving"))
		{
			_context.savedChanges = _context.sceneChanges;	// asked once
			_context.engine->QuitGame();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
		if (IsCompiling()) ImGui::TextColored(ImVec4(1.f, 0.8f, 0.3f, 1.f), "A build is running: it goes on without the editor");
		ImGui::EndPopup();
	}
}

void Krampus::DrawStatusBar(EditorContext& _context)
{
	// A bar under the dock space (ImGui side bar), like the one of Visual Studio: what is going on at a glance.
	const ImGuiWindowFlags _flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
	const bool _open = ImGui::BeginViewportSideBar("##StatusBar", ImGui::GetMainViewport(), ImGuiDir_Down, ImGui::GetFrameHeight(), _flags);
	if (_open && ImGui::BeginMenuBar())
	{
		const ImVec4 _warning = EditorTheme::Get(EditorTheme::Custom::ConsoleWarning);
		const auto _separator = []() { ImGui::SameLine(0.f, 12.f); ImGui::TextDisabled("|"); ImGui::SameLine(0.f, 12.f); };

		// Mode
		if (_context.playing) ImGui::TextColored(EditorTheme::Get(EditorTheme::Custom::PlayMode), "Playing");
		else ImGui::TextUnformatted("Editing");
		_separator();

		// Scene and its unsaved changes
		ImGui::TextUnformatted(_context.scenePath.empty() ? "Untitled scene" : _context.scenePath.stem().string().c_str());
		ImGui::SameLine();
		if (_context.SceneDirty()) ImGui::TextColored(_warning, "(unsaved: %s)", Shortcuts::Text("Save Scene").c_str());
		else ImGui::TextDisabled("(saved)");
		_separator();

		// Selection and how many actors
		Level* _level = _context.GetLevel();
		if (_context.selected) ImGui::Text("Selected: %s", _context.selected->GetName().StdString().c_str());
		else if (_context.selectedWidget) ImGui::Text("Selected: %s (HUD)", _context.selectedWidget->GetName().StdString().c_str());
		else ImGui::TextDisabled("Nothing selected");
		if (_level)
		{
			_separator();
			ImGui::TextDisabled("%d actor(s)", static_cast<int>(_level->GetActorManager().GetActors().size()));
			_separator();
			ImGui::TextDisabled("%d particle(s)", ParticleEmitter::GetTotalAlive());
		}

		// Right side: build, then frame rate
		const std::string _build = IsCompiling() ? (IsShipping() ? "Packaging..." : "Compiling...") : "";
		char _fps[32];
		std::snprintf(_fps, sizeof(_fps), "%.0f FPS", ImGui::GetIO().Framerate);
		const float _right = ImGui::CalcTextSize(_fps).x + (_build.empty() ? 0.f : ImGui::CalcTextSize(_build.c_str()).x + 30.f) + ImGui::GetStyle().WindowPadding.x * 2.f + ImGui::GetStyle().ItemSpacing.x * 2.f;
		ImGui::SameLine(std::max(ImGui::GetCursorPosX() + 20.f, ImGui::GetWindowWidth() - _right));
		if (!_build.empty())
		{
			ImGui::TextColored(_warning, "%s", _build.c_str());
			ImGui::SameLine(0.f, 30.f);
		}
		ImGui::TextDisabled("%s", _fps);

		ImGui::EndMenuBar();
	}
	ImGui::End();
}

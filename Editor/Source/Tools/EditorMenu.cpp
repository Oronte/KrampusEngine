#include "EditorContext.h"
#include "Scene/Scene.h"
#include "Managers/LevelManager.h"
#include <imgui.h>

using namespace Krampus;

Level* EditorContext::GetLevel() const
{
	return engine->GetLevelManager()->GetCurrentLevel<Level>();
}

void Krampus::NewScene(EditorContext& _context)
{
	OpenScene(_context, std::filesystem::path());
}

void Krampus::OpenScene(EditorContext& _context, const std::filesystem::path& _path)
{
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
	if (!_level || !Scene::Save(*_level, _path)) return false;

	_context.scenePath = _path;
	return true;
}

void Krampus::DrawMainMenuBar(EditorContext& _context)
{
	// The scene being edited is the one the level was made from (the game opens its first scene by itself).
	if (!_context.playing)
		if (const SceneLevel* _sceneLevel = Cast<SceneLevel>(_context.GetLevel()))
			if (_sceneLevel->GetScenePath() != _context.scenePath && _sceneLevel->GetScenePath() != PLAY_SNAPSHOT_PATH && !_sceneLevel->GetScenePath().empty())
				_context.scenePath = _sceneLevel->GetScenePath();

	static char _saveAsName[128] = "";
	bool _openSaveAs = false;

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
			if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
			{
				if (_context.scenePath.empty()) _openSaveAs = true;
				else SaveScene(_context, _context.scenePath);
			}
			if (ImGui::MenuItem("Save Scene As..."))
				_openSaveAs = true;

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
		ImGui::EndMainMenuBar();
	}

	if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false))
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

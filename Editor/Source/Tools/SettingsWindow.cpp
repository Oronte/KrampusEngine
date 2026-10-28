#include "EditorLayer.h"
#include "EditorSettings.h"
#include "Shortcuts.h"
#include <imgui.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

using namespace Krampus;

void EditorLayer::SaveSettings()
{
	const bool _engine = context.engine->GetSettings().Save();
	const bool _editor = EngineSettings::SaveSections(EditorSettings::GetPath(), { &editorSettings });
	if (_engine && _editor)
	{
		settingsDirty = false;
		LOG_MSG("Settings: saved");
	}
	else
	{
		LOG_ERROR("Settings: could not be saved");
	}
}

// The Settings window: one page per section (Logger, Physics, Time, Window, Audio, then the editor itself).
// A change is applied at once; Save writes it to the files, and closing the window with unsaved changes saves them.
void EditorLayer::DrawSettingsWindow()
{
	if (!context.settingsOpen) return;

	std::vector<SettingsSection*> _pages = context.engine->GetSettings().GetSections();
	_pages.push_back(&editorSettings);
	context.settingsPage = std::clamp(context.settingsPage, 0, static_cast<int>(_pages.size()));

	ImGui::SetNextWindowSize(ImVec2(760.f, 500.f), ImGuiCond_FirstUseEver);
	bool _open = true;
	if (ImGui::Begin("Settings", &_open))
	{
		if (ImGui::BeginChild("##pages", ImVec2(150.f, 0.f), ImGuiChildFlags_Borders))
		{
			for (int _i = 0; _i < static_cast<int>(_pages.size()); ++_i)
			{
				if (_i == static_cast<int>(_pages.size()) - 1) ImGui::Separator();	// the editor's own settings
				if (ImGui::Selectable(_pages[_i]->Title(), _i == context.settingsPage))
					context.settingsPage = _i;
			}
			if (ImGui::Selectable("Shortcuts", context.settingsPage == static_cast<int>(_pages.size())))
				context.settingsPage = static_cast<int>(_pages.size());
		}
		ImGui::EndChild();
		ImGui::SameLine();

		if (context.settingsPage == static_cast<int>(_pages.size()))
		{
			ImGui::BeginGroup();
			Shortcuts::DrawSettingsPage();
			ImGui::EndGroup();
		}
		else
		{
		ImGui::BeginGroup();
		SettingsSection& _section = *_pages[context.settingsPage];

		ImGui::SeparatorText(_section.Title());
		ImGui::TextWrapped("%s", _section.Description());
		ImGui::Spacing();

		if (ImGui::BeginChild("##values", ImVec2(0.f, -ImGui::GetFrameHeightWithSpacing() * 1.4f)))
		{
			if (DrawObjectProperties(_section))
				settingsDirty = true;

			if (std::string(_section.Title()) == "Ship")
			{
				ImGui::Spacing();
				std::error_code _relativeError;
				std::filesystem::path _sceneName = context.scenePath.empty() ? std::filesystem::path() : std::filesystem::relative(context.scenePath, SCENES_DIRECTORY, _relativeError);
				if (_relativeError || _sceneName.empty() || *_sceneName.begin() == "..") _sceneName = context.scenePath.stem();
				_sceneName.replace_extension();
				const std::string _openScene = _sceneName.generic_string();
				ImGui::BeginDisabled(_openScene.empty());
				if (ImGui::Button(("Use the open scene (" + (_openScene.empty() ? std::string("save it first") : _openScene) + ") as start scene").c_str()))
				{
					context.engine->GetSettings().ship.startScene = _openScene;
					settingsDirty = true;
					LOG_MSG("Settings: start scene is now " + _openScene);
				}
				ImGui::EndDisabled();
			}

			if (context.settingsPage == 0)	// Logger
			{
				ImGui::Spacing();
				ImGui::SeparatorText("Log file");
				if (ImGui::Button("Open Content/Logs/log.txt"))
					ShellExecuteW(nullptr, L"open", std::filesystem::absolute("Content/Logs/log.txt").c_str(), nullptr, nullptr, SW_SHOWNORMAL);
				ImGui::SameLine();
				if (ImGui::Button("Clear Console"))
					consoleLines.clear();
			}
		}
		ImGui::EndChild();

		if (ImGui::Button("Reset to defaults"))
		{
			_section.ResetToDefaults();
			_section.Apply();
			settingsDirty = true;
			LOG_MSG(std::string("Settings: ") + _section.Title() + " back to its defaults");
		}
		ImGui::SameLine();
		ImGui::BeginDisabled(!settingsDirty);
		if (ImGui::Button("Save"))
			SaveSettings();
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("Reload from files"))
		{
			context.engine->GetSettings().Load();
			EngineSettings::LoadSections(EditorSettings::GetPath(), { &editorSettings });
			settingsDirty = false;
		}
		ImGui::SameLine();
		if (settingsDirty) ImGui::TextColored(ImVec4(1.f, 0.85f, 0.3f, 1.f), "unsaved changes");
		else ImGui::TextDisabled("saved");

		ImGui::EndGroup();
		}
	}
	ImGui::End();

	if (!_open)
	{
		context.settingsOpen = false;
		if (settingsDirty) SaveSettings();
	}
}

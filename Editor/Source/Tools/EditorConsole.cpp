#include "EditorLayer.h"
#include "Layers/ScreenSpace.h"
#include "EditorThumbnails.h"
#include "Shortcuts.h"
#include "EditorTheme.h"
#include "FileDrop.h"
#include "GameplayTagWidgets.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui-SFML.h>
#include "GameFramework/Level.h"
#include "Components/CameraComponent.h"
#include "Components/PhysicsComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/BillboardComponent.h"
#include "Navigation/NavigationSystem.h"
#include "Graphics/Shaders/ShaderLibrary.h"
#include "Managers/TextureManager.h"
#include "Utilities/Debug/Debug.h"
#include "Utilities/Debug/Profiler.h"
#include <unordered_set>
#include "Utilities/Math/Polygon2D.h"
#include "Actors/Actor.h"
#include "UI/Widgets/Widget.h"
#include "Graphics/Shapes/ShapeAssets.h"

using namespace Krampus;

namespace
{
	ImVec4 ColorOf(const VerbosityType& _type)
	{
		switch (_type)
		{
		case VerbosityType::Warning:	return EditorTheme::Get(EditorTheme::Custom::ConsoleWarning);
		case VerbosityType::Error:		return EditorTheme::Get(EditorTheme::Custom::ConsoleError);
		case VerbosityType::Fatal:		return EditorTheme::Get(EditorTheme::Custom::ConsoleFatal);
		case VerbosityType::Display:	return EditorTheme::Get(EditorTheme::Custom::ConsoleMessage);
		default:						return EditorTheme::Get(EditorTheme::Custom::ConsoleLog);
		}
	}

	// ---- Console filters. A line looks like: "[29/10/2026 22:30:18] Display: Network: hosting on port 7777 (File : X.cpp | Func : F | Line : 12)"

	enum ConsoleGroup { GroupVerbose, GroupLog, GroupMessage, GroupWarning, GroupError, GroupCount };

	ConsoleGroup GroupOf(const VerbosityType& _type)
	{
		switch (_type)
		{
		case VerbosityType::VeryVerbose:
		case VerbosityType::Verbose:	return GroupVerbose;
		case VerbosityType::Log:		return GroupLog;
		case VerbosityType::Display:	return GroupMessage;
		case VerbosityType::Warning:	return GroupWarning;
		default:						return GroupError;
		}
	}

	/// <summary>The message without the date and the verbosity: "Network: hosting on port 7777 (File : ...)".</summary>
	std::string_view MessageOf(const std::string& _text)
	{
		const size_t _date = _text.find("] ");
		if (_date == std::string::npos) return _text;
		const size_t _type = _text.find(": ", _date + 2);
		if (_type == std::string::npos) return _text;
		return std::string_view(_text).substr(_type + 2);
	}

	/// <summary>"Network" for "Network: hosting...": the first word when a colon follows it. Empty when the message has none.</summary>
	std::string CategoryOf(const std::string& _text)
	{
		const std::string_view _message = MessageOf(_text);
		const size_t _colon = _message.find(':');
		if (_colon == std::string_view::npos || _colon == 0 || _colon > 24) return std::string();
		const std::string_view _word = _message.substr(0, _colon);
		if (_word.find_first_of(" ()[]\"'") != std::string_view::npos) return std::string();
		return std::string(_word);
	}

	/// <summary>The line without its "(File : ... | Line : ...)" end.</summary>
	std::string_view WithoutSource(const std::string& _text)
	{
		const size_t _source = _text.rfind(" (File : ");
		return _source == std::string::npos ? std::string_view(_text) : std::string_view(_text).substr(0, _source);
	}
}

void EditorLayer::DrawConsole()
{
	Logger::DrainEntries(consoleLines);
	if (consoleLines.size() > static_cast<size_t>(editorSettings.consoleMaxLines))
		consoleLines.erase(consoleLines.begin(), consoleLines.begin() + (consoleLines.size() - static_cast<size_t>(editorSettings.consoleMaxLines)));

	if (ImGui::Begin("Console"))
	{
		// Counts per kind and the categories that exist, over every kept line (the filters do not change them).
		int _counts[GroupCount] = { };
		std::set<std::string> _categories;
		for (const Logger::Entry& _line : consoleLines)
		{
			++_counts[GroupOf(_line.type)];
			if (std::string _category = CategoryOf(_line.text); !_category.empty()) _categories.insert(std::move(_category));
		}

		Bool* const _shown[GroupCount] = { &editorSettings.consoleShowVerbose, &editorSettings.consoleShowLog, &editorSettings.consoleShowMessages,
			&editorSettings.consoleShowWarnings, &editorSettings.consoleShowErrors };
		static constexpr const char* NAMES[GroupCount] = { "Verbose", "Log", "Messages", "Warnings", "Errors" };
		static constexpr VerbosityType COLORS[GroupCount] = { VerbosityType::Verbose, VerbosityType::Log, VerbosityType::Display, VerbosityType::Warning, VerbosityType::Error };

		bool _filtersChanged = false;
		if (ImGui::Button("Clear"))
			consoleLines.clear();
		ImGui::SameLine();
		bool _autoScroll = editorSettings.autoScrollConsole;
		if (ImGui::Checkbox("Auto-scroll", &_autoScroll))
		{
			editorSettings.autoScrollConsole = _autoScroll;
			_filtersChanged = true;
		}

		// One check box per kind of line, in its color, with how many there are.
		for (int _group = 0; _group < GroupCount; ++_group)
		{
			ImGui::SameLine();
			bool _value = *_shown[_group];
			const std::string _label = std::string(NAMES[_group]) + " (" + std::to_string(_counts[_group]) + ")##filter" + std::to_string(_group);
			ImGui::PushStyleColor(ImGuiCol_Text, ColorOf(COLORS[_group]));
			if (ImGui::Checkbox(_label.c_str(), &_value))
			{
				*_shown[_group] = _value;
				_filtersChanged = true;
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))	// right click: this kind only
			{
				for (int _other = 0; _other < GroupCount; ++_other) *_shown[_other] = _other == _group;
				_filtersChanged = true;
			}
			ImGui::SetItemTooltip("Right click: show only this kind of line");
		}

		// Category (the first word of the message, "Network: ...") and free text.
		ImGui::SetNextItemWidth(150.f);
		if (ImGui::BeginCombo("##category", consoleCategory.empty() ? "All categories" : consoleCategory.c_str()))
		{
			if (ImGui::Selectable("All categories", consoleCategory.empty())) consoleCategory.clear();
			for (const std::string& _category : _categories)
				if (ImGui::Selectable(_category.c_str(), _category == consoleCategory)) consoleCategory = _category;
			ImGui::EndCombo();
		}
		ImGui::SameLine();
		consoleSearch.Draw("##search", 220.f);
		if (!consoleSearch.IsActive() && !ImGui::IsItemActive())
		{
			// Hint drawn over the empty field.
			const ImVec2 _min = ImGui::GetItemRectMin();
			ImGui::GetWindowDrawList()->AddText(ImVec2(_min.x + ImGui::GetStyle().FramePadding.x, _min.y + ImGui::GetStyle().FramePadding.y),
				ImGui::GetColorU32(ImGuiCol_TextDisabled), "Search (a,b  -exclude)");
		}
		ImGui::SetItemTooltip("Words separated by commas: lines with one of them. A word starting with - hides the lines that have it.");
		ImGui::SameLine();
		if (ImGui::SmallButton("Reset filters"))
		{
			for (int _group = 0; _group < GroupCount; ++_group) *_shown[_group] = true;
			consoleCategory.clear();
			consoleSearch.Clear();
			_filtersChanged = true;
		}
		ImGui::SameLine();
		bool _source = editorSettings.consoleShowSource;
		if (ImGui::Checkbox("Source", &_source))
		{
			editorSettings.consoleShowSource = _source;
			_filtersChanged = true;
		}
		ImGui::SetItemTooltip("Show the file, function and line that logged each message");

		// The lines that pass every filter.
		consoleVisible.clear();
		for (int _i = 0; _i < static_cast<int>(consoleLines.size()); ++_i)
		{
			const Logger::Entry& _line = consoleLines[_i];
			if (!*_shown[GroupOf(_line.type)]) continue;
			if (!consoleCategory.empty() && CategoryOf(_line.text) != consoleCategory) continue;
			const std::string_view _searched = WithoutSource(_line.text);
			if (consoleSearch.IsActive() && !consoleSearch.PassFilter(_searched.data(), _searched.data() + _searched.size())) continue;
			consoleVisible.push_back(_i);
		}

		ImGui::SameLine();
		if (ImGui::SmallButton("Copy"))
		{
			std::string _text;
			for (const int _index : consoleVisible)
				_text.append(editorSettings.consoleShowSource ? std::string_view(consoleLines[_index].text) : WithoutSource(consoleLines[_index].text)).push_back('\n');
			ImGui::SetClipboardText(_text.c_str());
		}
		ImGui::SetItemTooltip("Copy the lines shown");
		ImGui::SameLine();
		ImGui::TextDisabled("%d / %d", static_cast<int>(consoleVisible.size()), static_cast<int>(consoleLines.size()));

		if (_filtersChanged)	// the filters are preferences of the editor: kept for the next session
			EngineSettings::SaveSections(EditorSettings::GetPath(), { &editorSettings });
		ImGui::Separator();

		if (ImGui::BeginChild("ConsoleLines", ImVec2(0.f, 0.f), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
		{
			// Only the lines on screen are drawn: thousands of lines stay cheap.
			ImGuiListClipper _clipper;
			_clipper.Begin(static_cast<int>(consoleVisible.size()));
			while (_clipper.Step())
				for (int _row = _clipper.DisplayStart; _row < _clipper.DisplayEnd; ++_row)
				{
					const Logger::Entry& _line = consoleLines[consoleVisible[_row]];
					const std::string_view _shownText = editorSettings.consoleShowSource ? std::string_view(_line.text) : WithoutSource(_line.text);

					ImGui::PushID(consoleVisible[_row]);
					ImGui::PushStyleColor(ImGuiCol_Text, ColorOf(_line.type));
					ImGui::TextUnformatted(_shownText.data(), _shownText.data() + _shownText.size());
					ImGui::PopStyleColor();
					if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
						ImGui::OpenPopup("line");
					if (ImGui::BeginPopup("line"))
					{
						if (ImGui::MenuItem("Copy line")) ImGui::SetClipboardText(_line.text.c_str());
						const std::string _category = CategoryOf(_line.text);
						if (!_category.empty() && ImGui::MenuItem(("Only \"" + _category + "\"").c_str())) consoleCategory = _category;
						ImGui::EndPopup();
					}
					ImGui::PopID();
				}

			if (editorSettings.autoScrollConsole && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				ImGui::SetScrollHereY(1.f);
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

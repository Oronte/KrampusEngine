#include "EditorContext.h"
#include "GameplayTagWidgets.h"
#include "Shortcuts.h"
#include "Reflection/Reflection.h"
#include "Actors/Actor.h"
#include "GameFramework/Level.h"
#include <imgui.h>
#include <algorithm>
#include <functional>

using namespace Krampus;

namespace
{
	struct Command
	{
		std::string					group;
		std::string					name;
		std::function<void()>		run;
	};

	bool	g_open		= false;
	bool	g_justOpen	= false;
	char	g_search[96] = "";
	int		g_cursor	= 0;

	std::string Lower(std::string _text)
	{
		std::transform(_text.begin(), _text.end(), _text.begin(), [](unsigned char _c) { return static_cast<char>(std::tolower(_c)); });
		return _text;
	}

	// Every word typed has to be somewhere in the name (or group).
	bool Matches(const Command& _command, const std::string& _search)
	{
		const std::string _text = Lower(_command.group + " " + _command.name);
		size_t _start = 0;
		while (_start < _search.size())
		{
			const size_t _space = _search.find(' ', _start);
			const std::string _word = _search.substr(_start, _space == std::string::npos ? std::string::npos : _space - _start);
			if (!_word.empty() && _text.find(_word) == std::string::npos) return false;
			if (_space == std::string::npos) break;
			_start = _space + 1;
		}
		return true;
	}

	std::vector<Command> BuildCommands(EditorContext& _context)
	{
		std::vector<Command> _all;
		const auto _add = [&](const char* _group, const std::string& _name, std::function<void()> _run) { _all.push_back({ _group, _name, std::move(_run) }); };

		_add("Scene", "New Scene", [&]() { RequestOpenScene(_context, std::filesystem::path()); });
		if (!_context.scenePath.empty() && !_context.playing)
			_add("Scene", "Save Scene", [&]() { SaveScene(_context, _context.scenePath); });
		_add("Game", _context.playing ? "Stop" : "Play", [&]() { if (_context.playing) Stop(_context); else Play(_context); });
		if (!IsCompiling())
		{
			_add("Game", "Compile", []() { StartCompile(); });
			_add("Game", "Package Game", [&]() { StartShip(_context); });
		}
		_add("Game", "Open Solution", []() { OpenSolution(); });
		_add("Edit", "Undo", [&]() { _context.history.Undo(_context); });
		_add("Edit", "Redo", [&]() { _context.history.Redo(_context); });

		_add("Tools", "Gameplay Tags", []() { OpenGameplayTagsWindow(); });
		_add("Tools", "Network", []() { OpenNetworkWindow(); });
		_add("Tools", "Profiler", []() { OpenProfiler(); });
		_add("Tools", "Animation Editor", []() { OpenAnimationEditor(); });
		_add("Tools", "Animation Graph", []() { OpenAnimationGraph(); });
		_add("Tools", "Shape Editor", []() { OpenShapeEditor(); });
		_add("Tools", "Shader Editor", []() { OpenShaderEditor(); });

		if (Level* _level = _context.GetLevel())
		{
			if (!_context.playing)
				for (ClassInfo* _class : ClassRegistry::GetActorClasses())
					_add("Add Actor", _class->name, [&_context, _class, _level]()
					{
						_context.history.Record(_context, "Add " + _class->name);
						Actor* _actor = _class->factory(_level);
						_actor->SetPosition(_context.cameraCenter);
						_context.Select(_actor);
					});

			for (const std::unique_ptr<Actor>& _actor : _level->GetActorManager().GetActors())
			{
				Actor* _raw = _actor.get();
				_add("Select", _raw->GetName(), [&_context, _raw]() { _context.Select(_raw); });
			}
		}
		return _all;
	}
}

void Krampus::OpenCommandPalette()
{
	g_open = true;
	g_justOpen = true;
}

void Krampus::DrawCommandPalette(EditorContext& _context)
{
	if (Shortcuts::Pressed("Command Palette") && !_context.playing) OpenCommandPalette();
	if (!g_open) return;

	const ImGuiViewport* _viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(_viewport->GetCenter().x, _viewport->Pos.y + _viewport->Size.y * 0.18f), ImGuiCond_Always, ImVec2(0.5f, 0.f));
	ImGui::SetNextWindowSize(ImVec2(520.f, 0.f));
	const ImGuiWindowFlags _flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings
		| ImGuiWindowFlags_AlwaysAutoResize;
	if (!ImGui::Begin("##CommandPalette", &g_open, _flags))
	{
		ImGui::End();
		return;
	}

	if (g_justOpen)
	{
		g_search[0] = 0;
		g_cursor = 0;
		ImGui::SetKeyboardFocusHere();
		g_justOpen = false;
	}

	ImGui::SetNextItemWidth(-FLT_MIN);
	if (ImGui::InputTextWithHint("##palettesearch", "Type a command, a tool, an actor to add or select...", g_search, sizeof(g_search)))
		g_cursor = 0;

	const std::string _search = Lower(g_search);
	std::vector<Command> _commands = BuildCommands(_context);
	std::vector<const Command*> _shown;
	for (const Command& _command : _commands)
		if (Matches(_command, _search)) _shown.push_back(&_command);
	if (_shown.size() > 12) _shown.resize(12);

	const int _count = static_cast<int>(_shown.size());
	if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) g_cursor = std::min(g_cursor + 1, _count - 1);
	if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) g_cursor = std::max(g_cursor - 1, 0);
	g_cursor = std::clamp(g_cursor, 0, std::max(_count - 1, 0));

	const Command* _chosen = nullptr;
	for (int _i = 0; _i < _count; ++_i)
	{
		ImGui::PushID(_i);
		const std::string _label = _shown[_i]->name;
		if (ImGui::Selectable(("##row" + std::to_string(_i)).c_str(), _i == g_cursor, 0, ImVec2(0.f, 0.f)))
			_chosen = _shown[_i];
		ImGui::SameLine(8.f);
		ImGui::TextUnformatted(_label.c_str());
		ImGui::SameLine(ImGui::GetWindowWidth() - 120.f);
		ImGui::TextDisabled("%s", _shown[_i]->group.c_str());
		ImGui::PopID();
	}
	if (_count == 0) ImGui::TextDisabled("Nothing matches");
	if (ImGui::IsKeyPressed(ImGuiKey_Enter) && _count > 0) _chosen = _shown[g_cursor];

	if (_chosen)
	{
		const std::function<void()> _run = _chosen->run;	// the list is rebuilt by the command itself otherwise
		g_open = false;
		_run();
	}
	if (ImGui::IsKeyPressed(ImGuiKey_Escape) || (ImGui::IsMouseClicked(0) && !ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)))
		g_open = false;

	ImGui::End();
}

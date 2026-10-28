#include "EditorContext.h"
#include "Scene/Scene.h"
#include <imgui.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

using namespace Krampus;

namespace
{
	enum class ScriptKind { Actor, Component };

	bool IsIdentifier(const std::string& _name)
	{
		if (_name.empty() || std::isdigit(static_cast<unsigned char>(_name[0]))) return false;
		return std::all_of(_name.begin(), _name.end(), [](char _c) { return std::isalnum(static_cast<unsigned char>(_c)) || _c == '_'; });
	}

	bool WriteText(const std::filesystem::path& _path, const std::string& _text)
	{
		std::ofstream _file(_path, std::ios::trunc);
		_file << _text;
		return _file.good();
	}

	// Creates Content/Scripts/<name>.h and .cpp with a reflected class ready to edit.
	std::string CreateScript(const std::string& _name, const ScriptKind& _kind)
	{
		const std::filesystem::path _header = SCRIPTS_DIRECTORY / (_name + ".h");
		const std::filesystem::path _source = SCRIPTS_DIRECTORY / (_name + ".cpp");
		if (std::filesystem::exists(_header) || std::filesystem::exists(_source))
			return "A script named " + _name + " already exists";

		std::error_code _error;
		std::filesystem::create_directories(SCRIPTS_DIRECTORY, _error);

		std::string _h, _c;
		if (_kind == ScriptKind::Actor)
		{
			_h = "#pragma once\n#include \"Reflection/Reflection.h\"\n\nnamespace Krampus\n{\n\n"
				"\tclass " + _name + " : public Actor\n\t{\n\t\tKACTOR(" + _name + ", Actor)\n\n\tpublic:\n"
				"\t\t" + _name + "(Level* _level);\n\n"
				"\t\t// Shown in the Inspector and saved in scenes.\n"
				"\t\tKPROPERTY(Float, speed) = 100.f;\n\n"
				"\t\tvoid BeginPlay() override;\n\t\tvoid Tick(const Float& _deltaTime) override;\n\t};\n\n}\n";
			_c = "#include \"" + _name + ".h\"\n\nnamespace Krampus\n{\n\n"
				+ _name + "::" + _name + "(Level* _level)\n\t: Actor(_level)\n{\n\tname = NAME_OF(" + _name + ");\n}\n\n"
				"void " + _name + "::BeginPlay()\n{\n\tActor::BeginPlay();\n}\n\n"
				"void " + _name + "::Tick(const Float& _deltaTime)\n{\n\tActor::Tick(_deltaTime);\n}\n\n}\n";
		}
		else
		{
			_h = "#pragma once\n#include \"Reflection/Reflection.h\"\n\nnamespace Krampus\n{\n\n"
				"\t// Add it from the Inspector (Add Component) or from an actor constructor with CreateComponent<" + _name + ">().\n"
				"\tclass " + _name + " : public Component\n\t{\n\t\tKCOMPONENT(" + _name + ", Component)\n\n\tpublic:\n"
				"\t\t" + _name + "(Actor* _owner);\n\n"
				"\t\t// Shown in the Inspector and saved in scenes.\n"
				"\t\tKPROPERTY(Float, value) = 1.f;\n\n"
				"\t\tvoid Tick(const Float& _deltaTime) override;\n\t};\n\n}\n";
			_c = "#include \"" + _name + ".h\"\n\nnamespace Krampus\n{\n\n"
				+ _name + "::" + _name + "(Actor* _owner)\n\t: Component(_owner)\n{\n\tname = NAME_OF(" + _name + ");\n}\n\n"
				"void " + _name + "::Tick(const Float& _deltaTime)\n{\n\tComponent::Tick(_deltaTime);\n}\n\n}\n";
		}

		if (!WriteText(_header, _h) || !WriteText(_source, _c))
			return "Cannot write the script files";

		LOG_MSG("Script created: " + _header.string() + ". Rebuild the Game project to load it.");
		return "";
	}

	void OpenWithSystem(const std::filesystem::path& _path)
	{
		ShellExecuteW(nullptr, L"open", std::filesystem::absolute(_path).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	}

	const char* KindOf(const std::filesystem::directory_entry& _entry)
	{
		if (_entry.is_directory()) return "Folder";

		const std::string _extension = _entry.path().extension().string();
		if (_extension == Scene::EXTENSION)					return "Scene";
		if (_extension == ".h" || _extension == ".cpp")		return "Script";
		if (_extension == ".png" || _extension == ".jpg")	return "Texture";
		if (_extension == ".wav" || _extension == ".mp3" || _extension == ".ogg") return "Audio";
		if (_extension == ".ttf" || _extension == ".otf")	return "Font";
		return "File";
	}
}

void Krampus::DrawContentPanel(EditorContext& _context)
{
	static std::filesystem::path	_current		= CONTENT_DIRECTORY;
	static char						_scriptName[64]	= "";
	static int						_scriptKind		= 0;
	static std::string				_scriptError;

	if (ImGui::Begin("Content"))
	{
		if (!std::filesystem::exists(_current)) _current = CONTENT_DIRECTORY;

		ImGui::BeginDisabled(_current == CONTENT_DIRECTORY);
		if (ImGui::Button("^ Up"))
			_current = _current.parent_path();
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::TextUnformatted(_current.generic_string().c_str());

		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 90.f);
		if (ImGui::Button("+ New Script"))
		{
			_scriptError.clear();
			ImGui::OpenPopup("NewScript");
		}
		if (ImGui::BeginPopup("NewScript"))
		{
			ImGui::InputText("Class name", _scriptName, sizeof(_scriptName));
			ImGui::RadioButton("Actor", &_scriptKind, 0);
			ImGui::SameLine();
			ImGui::RadioButton("Component", &_scriptKind, 1);
			ImGui::TextDisabled("Created in Content/Scripts");

			if (ImGui::Button("Create"))
			{
				const std::string _name = _scriptName;
				_scriptError = IsIdentifier(_name)
					? CreateScript(_name, _scriptKind == 0 ? ScriptKind::Actor : ScriptKind::Component)
					: "The name must be a valid C++ identifier";
				if (_scriptError.empty())
				{
					_scriptName[0] = '\0';
					ImGui::CloseCurrentPopup();
				}
			}
			if (!_scriptError.empty())
				ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "%s", _scriptError.c_str());
			ImGui::EndPopup();
		}
		ImGui::Separator();

		std::vector<std::filesystem::directory_entry> _entries;
		std::error_code _error;
		for (const auto& _entry : std::filesystem::directory_iterator(_current, _error))
			_entries.push_back(_entry);
		std::sort(_entries.begin(), _entries.end(), [](const auto& _a, const auto& _b)
		{
			if (_a.is_directory() != _b.is_directory()) return _a.is_directory();
			return _a.path().filename().string() < _b.path().filename().string();
		});

		if (ImGui::BeginTable("ContentTable", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
		{
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80.f);
			for (const auto& _entry : _entries)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				const bool _isDirectory = _entry.is_directory();
				const std::string _label = (_isDirectory ? "[+] " : "    ") + _entry.path().filename().string();
				ImGui::Selectable(_label.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick);

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					if (_isDirectory)									_current = _entry.path();
					else if (_entry.path().extension() == Scene::EXTENSION)	OpenScene(_context, _entry.path());
					else												OpenWithSystem(_entry.path());
				}

				ImGui::TableNextColumn();
				ImGui::TextDisabled("%s", KindOf(_entry));
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();
}

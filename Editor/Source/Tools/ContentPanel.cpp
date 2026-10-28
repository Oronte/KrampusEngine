#include "EditorContext.h"
#include "EditorThumbnails.h"
#include "Shortcuts.h"
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

		RefreshSolution();	// so that the new class shows up in Visual Studio
		LOG_MSG("Script created: " + _header.string() + ". Double-click it to open Visual Studio, then build EditorModule to load it.");
		return "";
	}


	void OpenWithSystem(const std::filesystem::path& _path)
	{
		ShellExecuteW(nullptr, L"open", std::filesystem::absolute(_path).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	}

	void ShowInExplorer(const std::filesystem::path& _path)
	{
		const std::wstring _arguments = L"/select,\"" + std::filesystem::absolute(_path).wstring() + L"\"";
		ShellExecuteW(nullptr, L"open", L"explorer.exe", _arguments.c_str(), nullptr, SW_SHOWNORMAL);
	}

	enum class Kind { Folder, Scene, Script, Texture, Audio, Font, Other };

	std::string LowerExtension(const std::filesystem::path& _path)
	{
		std::string _extension = _path.extension().string();
		std::transform(_extension.begin(), _extension.end(), _extension.begin(), [](unsigned char _c) { return static_cast<char>(std::tolower(_c)); });
		return _extension;
	}

	Kind KindOf(const std::filesystem::directory_entry& _entry)
	{
		if (_entry.is_directory()) return Kind::Folder;

		const std::string _extension = LowerExtension(_entry.path());
		if (_extension == Scene::EXTENSION)																return Kind::Scene;
		if (_extension == ".h" || _extension == ".cpp" || _extension == ".hpp")						return Kind::Script;
		if (_extension == ".png" || _extension == ".jpg" || _extension == ".jpeg" || _extension == ".bmp"
			|| _extension == ".tga" || _extension == ".gif")											return Kind::Texture;
		if (_extension == ".wav" || _extension == ".mp3" || _extension == ".ogg" || _extension == ".flac") return Kind::Audio;
		if (_extension == ".ttf" || _extension == ".otf")												return Kind::Font;
		return Kind::Other;
	}

	const char* NameOfKind(const Kind& _kind)
	{
		switch (_kind)
		{
		case Kind::Folder:	return "Folder";
		case Kind::Scene:	return "Scene";
		case Kind::Script:	return "Script";
		case Kind::Texture:	return "Texture";
		case Kind::Audio:	return "Audio";
		case Kind::Font:	return "Font";
		default:			return "File";
		}
	}

	// ---- Icons, drawn with the draw list so that no image file is needed.

	void DrawCenteredText(ImDrawList* _draw, const ImVec2& _min, const ImVec2& _max, const char* _text, const float& _fontSize, const ImU32& _color)
	{
		ImFont* _font = ImGui::GetFont();
		const ImVec2 _size = _font->CalcTextSizeA(_fontSize, FLT_MAX, 0.f, _text);
		_draw->AddText(_font, _fontSize, ImVec2((_min.x + _max.x - _size.x) * 0.5f, (_min.y + _max.y - _size.y) * 0.5f), _color, _text);
	}

	void DrawBadge(ImDrawList* _draw, const ImVec2& _min, const ImVec2& _max, const ImU32& _top, const ImU32& _bottom, const char* _text)
	{
		_draw->AddRectFilledMultiColor(_min, _max, _top, _top, _bottom, _bottom);
		_draw->AddRect(_min, _max, IM_COL32(255, 255, 255, 50), 6.f);
		DrawCenteredText(_draw, _min, _max, _text, (_max.y - _min.y) * 0.32f, IM_COL32(255, 255, 255, 235));
	}

	void DrawFolderIcon(ImDrawList* _draw, const ImVec2& _min, const ImVec2& _max, const bool& _open)
	{
		const float _w = _max.x - _min.x;
		const float _h = _max.y - _min.y;
		const ImVec2 _a(_min.x + _w * 0.06f, _min.y + _h * 0.20f);
		const ImVec2 _b(_max.x - _w * 0.06f, _max.y - _h * 0.14f);
		const ImU32 _back = IM_COL32(214, 150, 40, 255);
		const ImU32 _front = _open ? IM_COL32(255, 214, 110, 255) : IM_COL32(246, 190, 70, 255);

		_draw->AddRectFilled(_a, ImVec2(_a.x + _w * 0.40f, _a.y + _h * 0.16f), _back, 4.f);					// tab
		_draw->AddRectFilled(ImVec2(_a.x, _a.y + _h * 0.07f), _b, _back, 5.f);								// back
		_draw->AddRectFilled(ImVec2(_a.x, _a.y + _h * 0.20f), _b, _front, 5.f);								// front
		_draw->AddRect(ImVec2(_a.x, _a.y + _h * 0.20f), _b, IM_COL32(120, 80, 10, 120), 5.f);
	}

	void DrawAudioIcon(ImDrawList* _draw, const ImVec2& _min, const ImVec2& _max)
	{
		DrawBadge(_draw, _min, _max, IM_COL32(150, 90, 220, 255), IM_COL32(90, 50, 150, 255), "");
		const float _w = _max.x - _min.x;
		const float _h = _max.y - _min.y;
		const ImU32 _white = IM_COL32(255, 255, 255, 235);
		const ImVec2 _head(_min.x + _w * 0.40f, _min.y + _h * 0.68f);
		_draw->AddCircleFilled(_head, _w * 0.11f, _white);
		_draw->AddLine(ImVec2(_head.x + _w * 0.10f, _head.y), ImVec2(_head.x + _w * 0.10f, _min.y + _h * 0.24f), _white, 3.f);
		_draw->AddLine(ImVec2(_head.x + _w * 0.10f, _min.y + _h * 0.24f), ImVec2(_head.x + _w * 0.28f, _min.y + _h * 0.36f), _white, 3.f);
	}

	void DrawFileIcon(ImDrawList* _draw, const ImVec2& _min, const ImVec2& _max, const std::string& _extension)
	{
		const float _w = _max.x - _min.x;
		const float _h = _max.y - _min.y;
		const ImVec2 _a(_min.x + _w * 0.16f, _min.y + _h * 0.04f);
		const ImVec2 _b(_max.x - _w * 0.16f, _max.y - _h * 0.04f);
		const float _fold = _w * 0.22f;

		_draw->AddRectFilled(_a, _b, IM_COL32(205, 208, 215, 255), 4.f);
		_draw->AddTriangleFilled(ImVec2(_b.x - _fold, _a.y), ImVec2(_b.x, _a.y + _fold), ImVec2(_b.x - _fold, _a.y + _fold), IM_COL32(150, 154, 165, 255));
		DrawCenteredText(_draw, ImVec2(_a.x, _min.y + _h * 0.40f), ImVec2(_b.x, _b.y), _extension.empty() ? "?" : _extension.c_str() + 1, _w * 0.17f, IM_COL32(60, 62, 70, 255));
	}

	void DrawTextureIcon(ImDrawList* _draw, const ImVec2& _min, const ImVec2& _max, const std::filesystem::path& _path)
	{
		_draw->AddRectFilled(_min, _max, IM_COL32(38, 38, 44, 255), 5.f);

		if (sf::Texture* _texture = ThumbnailOf(_path))
		{
			// Fit inside the square, keep the proportions.
			const sf::Vector2f _size(_texture->getSize());
			if (_size.x > 0.f && _size.y > 0.f)
			{
				const float _scale = std::min((_max.x - _min.x) / _size.x, (_max.y - _min.y) / _size.y);
				const ImVec2 _half(_size.x * _scale * 0.5f, _size.y * _scale * 0.5f);
				const ImVec2 _center((_min.x + _max.x) * 0.5f, (_min.y + _max.y) * 0.5f);
				_draw->AddImage(TextureId(*_texture), ImVec2(_center.x - _half.x, _center.y - _half.y), ImVec2(_center.x + _half.x, _center.y + _half.y));
			}
		}
		else
		{
			DrawCenteredText(_draw, _min, _max, "IMG", (_max.y - _min.y) * 0.28f, IM_COL32(160, 160, 170, 255));
		}
		_draw->AddRect(_min, _max, IM_COL32(255, 255, 255, 40), 5.f);
	}

	void DrawIcon(ImDrawList* _draw, const ImVec2& _min, const ImVec2& _max, const std::filesystem::directory_entry& _entry, const Kind& _kind, const bool& _hovered)
	{
		switch (_kind)
		{
		case Kind::Folder:	DrawFolderIcon(_draw, _min, _max, _hovered); break;
		case Kind::Scene:	DrawBadge(_draw, _min, _max, IM_COL32(70, 140, 235, 255), IM_COL32(35, 70, 150, 255), "KA7"); break;
		case Kind::Script:	DrawBadge(_draw, _min, _max, IM_COL32(70, 185, 110, 255), IM_COL32(30, 105, 60, 255), LowerExtension(_entry.path()) == ".cpp" ? "C++" : ".h"); break;
		case Kind::Texture:	DrawTextureIcon(_draw, _min, _max, _entry.path()); break;
		case Kind::Audio:	DrawAudioIcon(_draw, _min, _max); break;
		case Kind::Font:	DrawBadge(_draw, _min, _max, IM_COL32(235, 140, 70, 255), IM_COL32(150, 70, 30, 255), "Aa"); break;
		default:			DrawFileIcon(_draw, _min, _max, LowerExtension(_entry.path())); break;
		}
	}

	std::string FileSizeText(const std::filesystem::directory_entry& _entry)
	{
		std::error_code _error;
		const double _bytes = static_cast<double>(_entry.file_size(_error));
		char _buffer[32];
		if (_error)					return "";
		if (_bytes < 1024.0)		std::snprintf(_buffer, sizeof(_buffer), "%.0f B", _bytes);
		else if (_bytes < 1048576.0)	std::snprintf(_buffer, sizeof(_buffer), "%.1f KB", _bytes / 1024.0);
		else						std::snprintf(_buffer, sizeof(_buffer), "%.1f MB", _bytes / 1048576.0);
		return _buffer;
	}

	std::filesystem::path NewFolderIn(const std::filesystem::path& _parent)
	{
		std::error_code _error;
		std::filesystem::path _folder = _parent / "New Folder";
		for (int _i = 2; std::filesystem::exists(_folder); ++_i)
			_folder = _parent / ("New Folder (" + std::to_string(_i) + ")");
		std::filesystem::create_directory(_folder, _error);
		LOG_MSG("Content: new folder " + _folder.generic_string());
		return _folder;
	}

	bool HasSubfolder(const std::filesystem::path& _path)
	{
		std::error_code _error;
		for (const auto& _entry : std::filesystem::directory_iterator(_path, _error))
			if (_entry.is_directory(_error)) return true;
		return false;
	}

	// The files the editor will not delete: engine assets and the folders every project needs.
	bool CanDelete(const std::filesystem::path& _path)
	{
		if (_path.empty() || _path == CONTENT_DIRECTORY) return false;
		if (_path.parent_path() == CONTENT_DIRECTORY && std::filesystem::is_directory(_path)) return false;	// Scripts, Scenes, Textures...
		for (const std::filesystem::path& _part : _path)
			if (_part == "_internal") return false;
		return true;
	}

	// A script is a .h and a .cpp with the same name: deleting one deletes the other, so no half class stays in the project.
	std::vector<std::filesystem::path> FilesToDelete(const std::filesystem::path& _path)
	{
		std::vector<std::filesystem::path> _files { _path };

		const std::string _extension = LowerExtension(_path);
		if (_extension == ".h" || _extension == ".cpp")
		{
			std::filesystem::path _companion = _path;
			_companion.replace_extension(_extension == ".h" ? ".cpp" : ".h");
			if (std::filesystem::exists(_companion)) _files.push_back(_companion);
		}
		return _files;
	}

	// Sends files and folders to the Windows Recycle Bin, so a mistake can be undone from there.
	bool MoveToRecycleBin(const std::vector<std::filesystem::path>& _files)
	{
		std::wstring _list;
		for (const std::filesystem::path& _file : _files)
		{
			_list += std::filesystem::absolute(_file).wstring();
			_list.push_back(L'\0');
		}
		_list.push_back(L'\0');	// the list ends with two zeros

		SHFILEOPSTRUCTW _operation { };
		_operation.wFunc = FO_DELETE;
		_operation.pFrom = _list.c_str();
		_operation.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
		return SHFileOperationW(&_operation) == 0 && !_operation.fAnyOperationsAborted;
	}

	constexpr const char* DRAG_CONTENT = "KRAMPUS_CONTENT";

	// A move asked by a drop (tile or folder tree), done after the lists are drawn.
	std::filesystem::path	g_moveSource;
	std::filesystem::path	g_moveFolder;

	bool IsWordChar(const char& _c)
	{
		return std::isalnum(static_cast<unsigned char>(_c)) || _c == '_';
	}

	// Replaces a whole identifier ("Enemy" in "Enemy::Enemy" but not in "EnemyBoss").
	std::string ReplaceWord(const std::string& _text, const std::string& _from, const std::string& _to)
	{
		std::string _result;
		size_t _position = 0;
		while (true)
		{
			const size_t _found = _text.find(_from, _position);
			if (_found == std::string::npos) break;

			const bool _startOk = _found == 0 || !IsWordChar(_text[_found - 1]);
			const bool _endOk = _found + _from.size() >= _text.size() || !IsWordChar(_text[_found + _from.size()]);
			_result.append(_text, _position, _found - _position);
			_result += (_startOk && _endOk) ? _to : _from;
			_position = _found + _from.size();
		}
		_result.append(_text, _position, std::string::npos);
		return _result;
	}

	bool IsScriptFile(const std::filesystem::path& _path)
	{
		const std::string _extension = LowerExtension(_path);
		return _extension == ".h" || _extension == ".cpp";
	}

	bool ValidFileName(const std::string& _name)
	{
		return !_name.empty() && _name.find_first_of("\/:*?\"<>|") == std::string::npos && _name.back() != '.' && _name.back() != ' ';
	}

	// Gives a new name to a file or folder. A script (.h + .cpp) is renamed as a pair and its class name is changed inside the files.
	// Returns an error text, empty if it worked.
	std::string RenameEntry(const std::filesystem::path& _path, const std::string& _newName, std::filesystem::path& _result)
	{
		const bool _isScript = IsScriptFile(_path);
		const std::string _oldStem = _path.stem().string();
		const std::string _newStem = _isScript ? _newName : std::filesystem::path(_newName).stem().string();

		if (_isScript)
		{
			if (!IsIdentifier(_newName)) return "A script name must be a valid C++ identifier";
		}
		else if (!ValidFileName(_newName))
		{
			return "This name cannot be used";
		}

		std::vector<std::pair<std::filesystem::path, std::filesystem::path>> _renames;
		if (_isScript)
		{
			for (const char* _extension : { ".h", ".cpp" })
			{
				const std::filesystem::path _old = _path.parent_path() / (_oldStem + _extension);
				if (std::filesystem::exists(_old)) _renames.push_back({ _old, _path.parent_path() / (_newStem + _extension) });
			}
		}
		else
		{
			_renames.push_back({ _path, _path.parent_path() / _newName });
		}

		for (const auto& [_from, _to] : _renames)
			if (_from != _to && std::filesystem::exists(_to)) return "\"" + _to.filename().string() + "\" already exists here";

		std::error_code _error;
		for (const auto& [_from, _to] : _renames)
		{
			if (_from == _to) continue;
			std::filesystem::rename(_from, _to, _error);
			if (_error) return "Cannot rename: " + _error.message();

			if (_isScript)
			{
				std::ifstream _in(_to, std::ios::binary);
				std::string _text((std::istreambuf_iterator<char>(_in)), std::istreambuf_iterator<char>());
				_in.close();
				std::ofstream _out(_to, std::ios::binary | std::ios::trunc);
				_out << ReplaceWord(_text, _oldStem, _newStem);
			}
		}

		_result = _renames.empty() ? _path : _renames.front().second;
		if (_isScript)
		{
			_result = _path.parent_path() / (_newStem + _path.extension().string());
			LOG_WARNING("Script renamed to " + _newStem + ": scenes that use the class " + _oldStem + " must have it added again.");
			RefreshSolution(true);
		}
		return "";
	}

	// Moves a file or folder into another folder (a script goes with its pair). Returns an error text, empty if it worked.
	std::string MoveEntry(const std::filesystem::path& _source, const std::filesystem::path& _folder)
	{
		if (!CanDelete(_source)) return "This one cannot be moved";
		if (_source.parent_path() == _folder) return "";	// already there

		std::error_code _error;
		if (std::filesystem::is_directory(_source))
		{
			const std::filesystem::path _a = std::filesystem::weakly_canonical(_source, _error);
			const std::filesystem::path _b = std::filesystem::weakly_canonical(_folder, _error);
			for (std::filesystem::path _walk = _b; !_walk.empty(); _walk = _walk.parent_path())
			{
				if (_walk == _a) return "A folder cannot be moved into itself";
				if (_walk == _walk.parent_path()) break;
			}
		}

		std::vector<std::filesystem::path> _files { _source };
		if (IsScriptFile(_source))
		{
			std::filesystem::path _pair = _source;
			_pair.replace_extension(LowerExtension(_source) == ".h" ? ".cpp" : ".h");
			if (std::filesystem::exists(_pair)) _files.push_back(_pair);
		}

		for (const std::filesystem::path& _file : _files)
			if (std::filesystem::exists(_folder / _file.filename())) return "\"" + _file.filename().string() + "\" already exists in " + _folder.generic_string();

		for (const std::filesystem::path& _file : _files)
		{
			std::filesystem::rename(_file, _folder / _file.filename(), _error);
			if (_error) return "Cannot move: " + _error.message();
		}

		if (IsScriptFile(_source))
		{
			LOG_WARNING("Script moved: #include lines that name its old folder must be updated.");
			RefreshSolution(true);
		}
		return "";
	}

	// Folder tree on the left.
	void DrawFolderTree(const std::filesystem::path& _path, std::filesystem::path& _current, const bool& _isRoot)
	{
		ImGuiTreeNodeFlags _flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
		if (_isRoot) _flags |= ImGuiTreeNodeFlags_DefaultOpen;
		if (!HasSubfolder(_path)) _flags |= ImGuiTreeNodeFlags_Leaf;
		if (_path == _current) _flags |= ImGuiTreeNodeFlags_Selected;

		const std::string _label = _isRoot ? "Content" : _path.filename().string();
		const bool _open = ImGui::TreeNodeEx(_path.string().c_str(), _flags, "%s", _label.c_str());
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			_current = _path;

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* _payload = ImGui::AcceptDragDropPayload(DRAG_CONTENT))
			{
				g_moveSource = std::filesystem::path(static_cast<const char*>(_payload->Data));
				g_moveFolder = _path;
			}
			ImGui::EndDragDropTarget();
		}

		if (_open)
		{
			std::vector<std::filesystem::path> _folders;
			std::error_code _error;
			for (const auto& _entry : std::filesystem::directory_iterator(_path, _error))
				if (_entry.is_directory(_error)) _folders.push_back(_entry.path());
			std::sort(_folders.begin(), _folders.end());

			for (const std::filesystem::path& _folder : _folders)
				DrawFolderTree(_folder, _current, false);
			ImGui::TreePop();
		}
	}
}

void Krampus::DrawContentPanel(EditorContext& _context)
{
	static std::filesystem::path	_current		= CONTENT_DIRECTORY;
	static std::filesystem::path	_selected;
	static char						_filter[64]		= "";
	static float					_tileSize		= 86.f;
	static char						_scriptName[64]	= "";
	static int						_scriptKind		= 0;
	static std::string				_scriptError;
	static std::vector<std::filesystem::path>	_pendingDelete;
	static bool						_askDelete	= false;
	static std::filesystem::path		_renameTarget;
	static char						_renameBuffer[128] = "";
	static std::string				_renameError;
	static bool						_askRename	= false;

	if (ImGui::Begin("Content"))
	{
		if (!std::filesystem::exists(_current)) _current = CONTENT_DIRECTORY;

		// ---- Top bar: up, breadcrumb, search, tile size, new script.
		ImGui::BeginDisabled(_current == CONTENT_DIRECTORY);
		if (ImGui::Button("^"))
			_current = _current.parent_path();
		ImGui::EndDisabled();

		std::filesystem::path _walk;
		for (const std::filesystem::path& _part : _current)
		{
			_walk = _walk.empty() ? _part : _walk / _part;
			ImGui::SameLine(0.f, 4.f);
			// one button per folder of the path
			{
				if (ImGui::SmallButton(_part.string().c_str()))
					_current = _walk;
			}
			if (_walk != _current)
			{
				ImGui::SameLine(0.f, 4.f);
				ImGui::TextDisabled(">");
			}
		}

		const float _right = ImGui::GetContentRegionAvail().x;
		ImGui::SameLine(std::max(ImGui::GetCursorPosX() + 20.f, ImGui::GetCursorPosX() + _right - 380.f));
		ImGui::SetNextItemWidth(150.f);
		ImGui::InputTextWithHint("##search", "Search...", _filter, sizeof(_filter));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(80.f);
		ImGui::SliderFloat("##size", &_tileSize, 56.f, 160.f, "");
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Icon size");
		ImGui::SameLine();
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

		// ---- Left: folder tree.
		if (ImGui::BeginChild("##folders", ImVec2(170.f, 0.f), ImGuiChildFlags_Borders))
			DrawFolderTree(CONTENT_DIRECTORY, _current, true);
		ImGui::EndChild();
		ImGui::SameLine();

		// ---- Right: the files of the current folder as tiles.
		if (ImGui::BeginChild("##files", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders))
		{
			std::vector<std::filesystem::directory_entry> _entries;
			std::error_code _error;
			std::string _lowerFilter = _filter;
			std::transform(_lowerFilter.begin(), _lowerFilter.end(), _lowerFilter.begin(), [](unsigned char _c) { return static_cast<char>(std::tolower(_c)); });

			for (const auto& _entry : std::filesystem::directory_iterator(_current, _error))
			{
				std::string _name = _entry.path().filename().string();
				std::transform(_name.begin(), _name.end(), _name.begin(), [](unsigned char _c) { return static_cast<char>(std::tolower(_c)); });
				if (!_lowerFilter.empty() && _name.find(_lowerFilter) == std::string::npos) continue;
				_entries.push_back(_entry);
			}
			std::sort(_entries.begin(), _entries.end(), [](const auto& _a, const auto& _b)
			{
				if (_a.is_directory() != _b.is_directory()) return _a.is_directory();
				std::string _nameA = _a.path().filename().string(), _nameB = _b.path().filename().string();
				std::transform(_nameA.begin(), _nameA.end(), _nameA.begin(), [](unsigned char _c) { return static_cast<char>(std::tolower(_c)); });
				std::transform(_nameB.begin(), _nameB.end(), _nameB.begin(), [](unsigned char _c) { return static_cast<char>(std::tolower(_c)); });
				return _nameA < _nameB;
			});

			const float _labelHeight = ImGui::GetTextLineHeight() * 2.f + 4.f;
			const float _padding = 8.f;
			const float _cellWidth = _tileSize + _padding;
			const int _columns = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / _cellWidth));

			std::filesystem::path _openFolder;
			int _index = 0;
			for (const auto& _entry : _entries)
			{
				const Kind _kind = KindOf(_entry);
				const std::string _fileName = _entry.path().filename().string();
				const bool _isSelected = _selected == _entry.path();

				if (_index % _columns != 0) ImGui::SameLine(0.f, _padding);
				++_index;

				ImGui::PushID(_fileName.c_str());
				const ImVec2 _min = ImGui::GetCursorScreenPos();
				ImGui::InvisibleButton("##tile", ImVec2(_tileSize, _tileSize + _labelHeight));
				const bool _hovered = ImGui::IsItemHovered();
				const ImVec2 _max(_min.x + _tileSize, _min.y + _tileSize + _labelHeight);

				// Drag a tile onto a folder (tile or tree) to move it.
				if (CanDelete(_entry.path()) && ImGui::BeginDragDropSource())
				{
					const std::string _dragged = _entry.path().generic_string();
					ImGui::SetDragDropPayload(DRAG_CONTENT, _dragged.c_str(), _dragged.size() + 1);
					ImGui::TextUnformatted(_fileName.c_str());
					ImGui::EndDragDropSource();
				}
				if (_kind == Kind::Folder && ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* _payload = ImGui::AcceptDragDropPayload(DRAG_CONTENT))
					{
						g_moveSource = std::filesystem::path(static_cast<const char*>(_payload->Data));
						g_moveFolder = _entry.path();
					}
					ImGui::EndDragDropTarget();
				}

				if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right))
					_selected = _entry.path();
				if (_hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					LOG(VerbosityType::Log, "Content: open " + _entry.path().generic_string());
					if (_kind == Kind::Folder)		_openFolder = _entry.path();
					else if (_kind == Kind::Scene)	OpenScene(_context, _entry.path());
					else if (_kind == Kind::Script)	OpenSolution(_entry.path());
					else							OpenWithSystem(_entry.path());
				}

				ImDrawList* _draw = ImGui::GetWindowDrawList();
				if (_isSelected)		_draw->AddRectFilled(_min, _max, IM_COL32(70, 120, 220, 90), 6.f);
				else if (_hovered)		_draw->AddRectFilled(_min, _max, IM_COL32(255, 255, 255, 22), 6.f);
				if (_isSelected)		_draw->AddRect(_min, _max, IM_COL32(90, 150, 255, 200), 6.f);

				const float _inset = 6.f;
				DrawIcon(_draw, ImVec2(_min.x + _inset, _min.y + _inset), ImVec2(_max.x - _inset, _min.y + _tileSize - _inset + 2.f), _entry, _kind, _hovered);

				// Name under the icon, up to two lines.
				const ImVec2 _textMin(_min.x + 3.f, _min.y + _tileSize);
				const ImVec2 _textMax(_max.x - 3.f, _max.y);
				_draw->PushClipRect(_textMin, _textMax, true);
				std::string _shown = _fileName;
				if (_kind != Kind::Folder && _kind != Kind::Other) _shown = _entry.path().stem().string();
				const ImVec2 _textSize = ImGui::CalcTextSize(_shown.c_str(), nullptr, false, _tileSize - 6.f);
				_draw->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(_textMin.x + std::max(0.f, (_tileSize - 6.f - _textSize.x) * 0.5f), _textMin.y),
					IM_COL32(230, 230, 235, 255), _shown.c_str(), nullptr, _tileSize - 6.f);
				_draw->PopClipRect();

				if (_hovered)
				{
					ImGui::BeginTooltip();
					ImGui::TextUnformatted(_fileName.c_str());
					ImGui::TextDisabled("%s  %s", NameOfKind(_kind), _kind == Kind::Folder ? "" : FileSizeText(_entry).c_str());
					ImGui::EndTooltip();
				}

				if (ImGui::BeginPopupContextItem("##itemMenu"))
				{
					if (ImGui::MenuItem(_kind == Kind::Folder ? "Open Folder" : "Open"))
					{
						if (_kind == Kind::Folder)		_openFolder = _entry.path();
						else if (_kind == Kind::Scene)	OpenScene(_context, _entry.path());
						else if (_kind == Kind::Script)	OpenSolution(_entry.path());
						else							OpenWithSystem(_entry.path());
					}
					if (ImGui::MenuItem("Show in Explorer")) ShowInExplorer(_entry.path());
					if (ImGui::MenuItem("Copy Path")) ImGui::SetClipboardText(_entry.path().generic_string().c_str());
					ImGui::Separator();
					if (ImGui::MenuItem("Rename", Shortcuts::Text("Rename File").c_str(), false, CanDelete(_entry.path())))
					{
						_renameTarget = _entry.path();
						std::snprintf(_renameBuffer, sizeof(_renameBuffer), "%s", (std::filesystem::is_directory(_renameTarget) ? _renameTarget.filename() : _renameTarget.stem()).string().c_str());
						_renameError.clear();
						_askRename = true;
					}
					if (ImGui::MenuItem("Delete", Shortcuts::Text("Delete File").c_str(), false, CanDelete(_entry.path())))
					{
						_pendingDelete = FilesToDelete(_entry.path());
						_askDelete = true;
					}
					ImGui::EndPopup();
				}
				ImGui::PopID();
			}

			if (_entries.empty())
				ImGui::TextDisabled(_lowerFilter.empty() ? "This folder is empty" : "Nothing matches the search");

			// F2 on the selected tile.
			if (!_selected.empty() && ImGui::IsWindowFocused() && Shortcuts::Pressed("Rename File") && CanDelete(_selected))
			{
				_renameTarget = _selected;
				std::snprintf(_renameBuffer, sizeof(_renameBuffer), "%s", (std::filesystem::is_directory(_renameTarget) ? _renameTarget.filename() : _renameTarget.stem()).string().c_str());
				_renameError.clear();
				_askRename = true;
			}

			// Delete key on the selected tile.
			if (!_selected.empty() && ImGui::IsWindowFocused() && Shortcuts::Pressed("Delete File") && CanDelete(_selected))
			{
				_pendingDelete = FilesToDelete(_selected);
				_askDelete = true;
			}

			// Click on the empty area: nothing selected. Right click: folder menu.
			if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				_selected.clear();
			if (ImGui::BeginPopupContextWindow("##folderMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
			{
				if (ImGui::MenuItem("New Folder"))
					_selected = NewFolderIn(_current);
				if (ImGui::MenuItem("Show in Explorer")) ShowInExplorer(_current);
				ImGui::EndPopup();
			}

			if (!_openFolder.empty())
			{
				_current = _openFolder;
				_selected.clear();
			}
		}
		ImGui::EndChild();

		// ---- Moves asked by a drop on a folder (tile or tree).
		if (!g_moveSource.empty())
		{
			const std::string _error = MoveEntry(g_moveSource, g_moveFolder);
			if (_error.empty())
			{
				LOG_MSG("Moved " + g_moveSource.generic_string() + " to " + g_moveFolder.generic_string());
				_selected = g_moveFolder / g_moveSource.filename();
				ClearThumbnails();
			}
			else
			{
				LOG_ERROR(_error);
			}
			g_moveSource.clear();
		}

		// ---- Rename.
		if (_askRename)
		{
			ImGui::OpenPopup("Rename");
			_askRename = false;
		}
		if (ImGui::BeginPopupModal("Rename", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			const bool _isFile = !std::filesystem::is_directory(_renameTarget);
			if (ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere();
			const bool _enter = ImGui::InputText("##name", _renameBuffer, sizeof(_renameBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
			if (_isFile)
			{
				ImGui::SameLine();
				ImGui::TextDisabled("%s", _renameTarget.extension().string().c_str());
			}
			if (IsScriptFile(_renameTarget))
				ImGui::TextDisabled("The .h and the .cpp are renamed, and the class name changes inside.");

			if (!_renameError.empty())
				ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "%s", _renameError.c_str());

			if (ImGui::Button("Rename", ImVec2(110.f, 0.f)) || _enter)
			{
				std::string _newName = _renameBuffer;
				if (_isFile && !IsScriptFile(_renameTarget)) _newName += _renameTarget.extension().string();

				std::filesystem::path _result;
				_renameError = RenameEntry(_renameTarget, _newName, _result);
				if (_renameError.empty())
				{
					LOG_MSG("Renamed " + _renameTarget.generic_string() + " to " + _result.generic_string());
					_selected = _result;
					ClearThumbnails();
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(110.f, 0.f)))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		// ---- Confirmation before anything is deleted.
		if (_askDelete)
		{
			ImGui::OpenPopup("Delete from Content");
			_askDelete = false;
		}
		if (ImGui::BeginPopupModal("Delete from Content", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::TextUnformatted("These will be moved to the Recycle Bin:");
			for (const std::filesystem::path& _file : _pendingDelete)
			{
				ImGui::BulletText("%s", _file.generic_string().c_str());
				if (std::filesystem::is_directory(_file))
					ImGui::TextDisabled("    (the folder and everything in it)");
			}
			ImGui::Spacing();

			if (ImGui::Button("Delete", ImVec2(110.f, 0.f)))
			{
				bool _scriptsDeleted = false;
				for (const std::filesystem::path& _file : _pendingDelete)
				{
					const std::string _extension = LowerExtension(_file);
					if (_extension == ".h" || _extension == ".cpp") _scriptsDeleted = true;
					if (std::filesystem::is_directory(_file)) _scriptsDeleted = true;	// may hold scripts
				}

				if (MoveToRecycleBin(_pendingDelete))
					LOG_MSG("Moved to the Recycle Bin: " + _pendingDelete.front().generic_string() + (_pendingDelete.size() > 1 ? " (+ its companion file)" : ""));
				else
					LOG_ERROR("Cannot delete " + _pendingDelete.front().generic_string() + " (file in use?)");

				_selected.clear();
				_pendingDelete.clear();
				ClearThumbnails();	// a picture may have been shown for a file that is gone
				if (_scriptsDeleted) RefreshSolution(true);	// they must leave the Visual Studio project too
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(110.f, 0.f)))
			{
				_pendingDelete.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}
	ImGui::End();
}

#include "EditorContext.h"
#include "EditorWidgets.h"
#include "EditorThumbnails.h"
#include "Shortcuts.h"
#include "Scene/Scene.h"
#include "Graphics/Shapes/ShapeAssets.h"
#include "ScriptCreator.h"
#include "FileDrop.h"
#include <imgui.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#include "ContentInternal.h"

using namespace Krampus;
using namespace Krampus::ContentInternal;

namespace
{
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
				if (_entry.is_directory(_error) && !IsInternalPath(_entry.path().filename())) _folders.push_back(_entry.path());	// _internal: engine and editor files
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
	static std::string				_scriptParent	= "Actor";
	static char						_parentFilter[64] = "";
	static std::string				_scriptError;
	static std::vector<std::filesystem::path>	_pendingDelete;
	static bool						_askDelete	= false;
	static std::filesystem::path		_renameTarget;
	static char						_renameBuffer[128] = "";
	static std::string				_renameError;
	static bool						_askRename	= false;

	// Files dropped from the Explorer (anywhere on the editor): copied into the folder shown, or into the folder tile they are dropped on.
	FileDrop::Drop					_drop;
	const bool						_dropped	= FileDrop::Take(_drop);
	std::filesystem::path			_dropFolder;

	if (ImGui::Begin("Content"))
	{
		if (!std::filesystem::exists(_current) || IsInternalPath(_current)) _current = CONTENT_DIRECTORY;	// never inside an _internal folder

		// ---- Top bar: up, breadcrumb, search, tile size, new script.
		ImGui::BeginDisabled(_current == CONTENT_DIRECTORY);
		if (ImGui::Button("^"))
			_current = _current.parent_path();
		ImGui::EndDisabled();

		// One button per folder of the path. Walk a copy and change the folder after the loop: assigning _current while
		// iterating over it invalidates the iterators (that was the crash when clicking "Content").
		const std::filesystem::path _shown = _current;
		std::filesystem::path _walk;
		std::filesystem::path _clicked;
		for (const std::filesystem::path& _part : _shown)
		{
			_walk = _walk.empty() ? _part : _walk / _part;
			ImGui::SameLine(0.f, 4.f);
			ImGui::PushID(_walk.string().c_str());
			if (ImGui::SmallButton(_part.string().c_str()))
				_clicked = _walk;
			ImGui::PopID();
			if (_walk != _shown)
			{
				ImGui::SameLine(0.f, 4.f);
				ImGui::TextDisabled(">");
			}
		}
		if (!_clicked.empty()) _current = _clicked;

		const float _right = ImGui::GetContentRegionAvail().x;
		ImGui::SameLine(std::max(ImGui::GetCursorPosX() + 20.f, ImGui::GetCursorPosX() + _right - 380.f));
		ImGui::SetNextItemWidth(150.f);
		ImGui::InputTextWithHint("##search", "Search...", _filter, sizeof(_filter));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(80.f);
		EditorUI::SliderFloat("##size", &_tileSize, 56.f, 160.f, "");
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Icon size");
		ImGui::SameLine();
		if (ImGui::Button("+ New Script"))
		{
			_scriptError.clear();
			RefreshClassNames();	// scripts may have been added or renamed since the last time
			ImGui::OpenPopup("NewScript");
		}
		if (ImGui::BeginPopup("NewScript"))
		{
			ImGui::SetNextItemWidth(260.f);
			ImGui::InputText("Class name", _scriptName, sizeof(_scriptName));

			// The parent: every class of the engine and of the game that can have children, grouped by what they are.
			const std::vector<ScriptParent> _parents = ScriptParents();
			const auto _chosen = std::find_if(_parents.begin(), _parents.end(), [&](const ScriptParent& _p) { return _p.name == _scriptParent; });
			ImGui::SetNextItemWidth(260.f);
			if (ImGui::BeginCombo("Parent class", _scriptParent.c_str(), ImGuiComboFlags_HeightLarge))
			{
				ImGui::SetNextItemWidth(-FLT_MIN);
				ImGui::InputTextWithHint("##parentFilter", "Search...", _parentFilter, sizeof(_parentFilter));
				std::string _search = _parentFilter;
				std::transform(_search.begin(), _search.end(), _search.begin(), [](unsigned char _c) { return static_cast<char>(std::tolower(_c)); });
				for (const ScriptBase _base : { ScriptBase::Actor, ScriptBase::Component, ScriptBase::Widget, ScriptBase::WidgetComponent })
				{
					ImGui::SeparatorText(NameOfBase(_base));
					for (const ScriptParent& _parent : _parents)
					{
						if (_parent.base != _base) continue;
						std::string _lower = _parent.name;
						std::transform(_lower.begin(), _lower.end(), _lower.begin(), [](unsigned char _c) { return static_cast<char>(std::tolower(_c)); });
						if (!_search.empty() && _lower.find(_search) == std::string::npos) continue;
						const std::string _label = _parent.name + (_parent.isScript ? "  (script)" : "") + (_parent.isDiegetic ? "  (diegetic)" : "");
						if (ImGui::Selectable(_label.c_str(), _parent.name == _scriptParent)) _scriptParent = _parent.name;
					}
				}
				ImGui::EndCombo();
			}

			// Where: the folder shown when it is in Content/Scripts, else Content/Scripts.
			const std::string _currentText = _current.generic_string();
			const bool _inScripts = _currentText.starts_with(SCRIPTS_DIRECTORY.generic_string());
			const std::filesystem::path _folder = _inScripts ? _current : SCRIPTS_DIRECTORY;
			ImGui::TextDisabled("Created in %s", _folder.generic_string().c_str());

			// The name is checked as it is typed: an existing class, a C++ word, a file that is already there...
			const std::string _name = _scriptName;
			const std::string _problem = _name.empty() ? std::string() : CheckNewClassName(_name);
			if (!_problem.empty())
				ImGui::TextColored(ImVec4(1.f, 0.6f, 0.3f, 1.f), "%s", _problem.c_str());

			ImGui::BeginDisabled(_name.empty() || !_problem.empty() || _chosen == _parents.end());
			if (ImGui::Button("Create"))
			{
				_scriptError = CreateScript(_name, *_chosen, _folder);
				if (_scriptError.empty())
				{
					_scriptName[0] = '\0';
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::EndDisabled();
			if (_chosen == _parents.end())
				ImGui::TextDisabled("Pick a parent class");
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
				if (IsInternalPath(_entry.path().filename())) continue;	// the engine and editor files are not shown
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
				const bool _visible = ImGui::IsItemVisible();	// a tile scrolled out of the panel draws nothing (draw lists have a vertex limit)
				const ImVec2 _max(_min.x + _tileSize, _min.y + _tileSize + _labelHeight);
				if (_dropped && _kind == Kind::Folder && _drop.position.x >= _min.x && _drop.position.x <= _max.x && _drop.position.y >= _min.y && _drop.position.y <= _max.y)
					_dropFolder = _entry.path();

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
					else if (_kind == Kind::Scene)	RequestOpenScene(_context, _entry.path());
					else if (_kind == Kind::Script)	OpenSolution(_entry.path());
						else if (_kind == Kind::Shape)	OpenShapeEditor(ShapeNameOf(_entry.path()));
						else if (_kind == Kind::Shader)	OpenShaderEditor(ShaderNameOf(_entry.path()));
					else							OpenWithSystem(_entry.path());
				}

				if (_visible)
				{
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
				}

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
						else if (_kind == Kind::Scene)	RequestOpenScene(_context, _entry.path());
						else if (_kind == Kind::Script)	OpenSolution(_entry.path());
						else if (_kind == Kind::Shape)	OpenShapeEditor(ShapeNameOf(_entry.path()));
						else if (_kind == Kind::Shader)	OpenShaderEditor(ShaderNameOf(_entry.path()));
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
						RefreshClassNames();
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
				RefreshClassNames();
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

	if (_dropped)
	{
		if (!std::filesystem::exists(_current) || IsInternalPath(_current)) _current = CONTENT_DIRECTORY;
		const std::filesystem::path _folder = _dropFolder.empty() ? _current : _dropFolder;
		const std::vector<std::filesystem::path> _imported = ImportDropped(_drop.paths, _folder);
		if (!_imported.empty())
		{
			LOG_MSG("Content: " + std::to_string(_imported.size()) + " item(s) imported into " + PrintablePath(_folder));
			_current = _folder;
			_selected = _imported.front();
			ClearThumbnails();
			ImGui::SetWindowFocus("Content");	// show where they went
		}
	}
}

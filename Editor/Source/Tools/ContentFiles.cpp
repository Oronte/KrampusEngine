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

namespace Krampus::ContentInternal
{
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
			if (_entry.is_directory(_error) && !IsInternalPath(_entry.path().filename())) return true;
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
		return !_name.empty() && _name.find_first_of("\\/:*?\"<>|") == std::string::npos && _name.back() != '.' && _name.back() != ' ';
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
			// The class takes the new name too: it must not be the name of something that exists (a case change of itself is fine).
			const bool _sameClass = std::equal(_oldStem.begin(), _oldStem.end(), _newName.begin(), _newName.end(),
				[](const char _a, const char _b) { return std::tolower(static_cast<unsigned char>(_a)) == std::tolower(static_cast<unsigned char>(_b)); });
			if (!_sameClass)
				if (const std::string _problem = CheckNewClassName(_newName); !_problem.empty()) return _problem;
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

	// Name of a path for the Console, whatever its characters (string() fails on the ones the Windows code page has not).
	std::string PrintablePath(const std::filesystem::path& _path)
	{
		const std::u8string _text = _path.generic_u8string();
		return std::string(_text.begin(), _text.end());
	}

	// _name in _folder, or "name (2).ext", "name (3).ext"... when it is taken.
	std::filesystem::path FreeNameIn(const std::filesystem::path& _folder, const std::filesystem::path& _name)
	{
		std::error_code _error;
		std::filesystem::path _target = _folder / _name;
		for (int _n = 2; std::filesystem::exists(_target, _error); ++_n)
			_target = _folder / (_name.stem().wstring() + L" (" + std::to_wstring(_n) + L")" + _name.extension().wstring());
		return _target;
	}

	// Copies files and folders dropped from the Explorer into _folder (the originals stay where they are). Returns the copies.
	std::vector<std::filesystem::path> ImportDropped(const std::vector<std::filesystem::path>& _paths, const std::filesystem::path& _folder)
	{
		std::vector<std::filesystem::path> _imported;
		bool _scripts = false;
		std::error_code _error;
		const std::filesystem::path _target = std::filesystem::weakly_canonical(_folder, _error);

		for (const std::filesystem::path& _source : _paths)
		{
			_error.clear();
			if (!std::filesystem::exists(_source, _error)) continue;
			const bool _isFolder = std::filesystem::is_directory(_source, _error);

			// A folder dropped into itself (or one of its own folders) would copy forever.
			if (_isFolder)
			{
				const std::filesystem::path _from = std::filesystem::weakly_canonical(_source, _error);
				bool _inside = false;
				for (std::filesystem::path _walk = _target; !_walk.empty(); _walk = _walk.parent_path())
				{
					if (_walk == _from) { _inside = true; break; }
					if (_walk == _walk.parent_path()) break;
				}
				if (_inside)
				{
					LOG_WARNING("Content: " + PrintablePath(_source) + " cannot be copied into itself");
					continue;
				}
			}

			const std::filesystem::path _copy = FreeNameIn(_folder, _source.filename());
			if (_isFolder) std::filesystem::copy(_source, _copy, std::filesystem::copy_options::recursive, _error);
			else std::filesystem::copy_file(_source, _copy, _error);
			if (_error)
			{
				LOG_WARNING("Content: cannot import " + PrintablePath(_source) + ": " + _error.message());
				continue;
			}
			_imported.push_back(_copy);
			if (_isFolder || IsScriptFile(_copy)) _scripts = true;
		}

		// Scripts dropped into Content/Scripts join the Visual Studio project, so that Compile builds them.
		const std::string _scriptsFolder = SCRIPTS_DIRECTORY.generic_string();
		if (_scripts && _folder.generic_string().rfind(_scriptsFolder, 0) == 0) RefreshSolution(true);
		return _imported;
	}

	// Folder tree on the left.
}

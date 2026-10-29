#include "EditorContext.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

using namespace Krampus;

#ifndef KRAMPUS_CMAKE_COMMAND
#define KRAMPUS_CMAKE_COMMAND "cmake"
#endif

namespace
{
	const wchar_t* PRESET = L"windows-vs2026";

	// Finds the generated Visual Studio solution (Build/vs2026/*.sln or *.slnx).
	std::filesystem::path FindSolution()
	{
		std::error_code _error;
		for (const auto& _entry : std::filesystem::directory_iterator("Build/vs2026", _error))
		{
			const std::string _extension = _entry.path().extension().string();
			if (_extension == ".sln" || _extension == ".slnx") return _entry.path();
		}
		return { };
	}

	// Runs a program without any window. Optionally waits for it and collects what it prints.
	bool RunHidden(std::wstring _commandLine, const bool& _wait, std::string* _output = nullptr)
	{
		SECURITY_ATTRIBUTES _security { sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
		HANDLE _read = nullptr, _write = nullptr;
		if (_output) CreatePipe(&_read, &_write, &_security, 0);
		if (_read) SetHandleInformation(_read, HANDLE_FLAG_INHERIT, 0);

		STARTUPINFOW _startup { };
		_startup.cb = sizeof(_startup);
		if (_write)
		{
			_startup.dwFlags = STARTF_USESTDHANDLES;
			_startup.hStdOutput = _write;
			_startup.hStdError = _write;
		}

		const std::wstring _directory = std::filesystem::current_path().wstring();
		PROCESS_INFORMATION _process { };
		const bool _started = CreateProcessW(nullptr, _commandLine.data(), nullptr, nullptr, _write != nullptr, CREATE_NO_WINDOW,
			nullptr, _directory.c_str(), &_startup, &_process) != 0;

		if (_write) CloseHandle(_write);
		if (!_started)
		{
			if (_read) CloseHandle(_read);
			return false;
		}

		if (_read)
		{
			char _buffer[512];
			DWORD _count = 0;
			while (ReadFile(_read, _buffer, sizeof(_buffer), &_count, nullptr) && _count > 0)
				_output->append(_buffer, _count);
			CloseHandle(_read);
		}
		if (_wait) WaitForSingleObject(_process.hProcess, 180000);

		CloseHandle(_process.hProcess);
		CloseHandle(_process.hThread);
		return true;
	}

	// Path of devenv.exe, asked to vswhere (installed with Visual Studio). Empty if not found.
	std::filesystem::path FindDevenv()
	{
		static std::filesystem::path _devenv;
		static bool _searched = false;
		if (_searched) return _devenv;
		_searched = true;

		wchar_t _programFiles[MAX_PATH];
		if (!GetEnvironmentVariableW(L"ProgramFiles(x86)", _programFiles, MAX_PATH)) return _devenv;
		const std::filesystem::path _vswhere = std::filesystem::path(_programFiles) / "Microsoft Visual Studio" / "Installer" / "vswhere.exe";
		if (!std::filesystem::exists(_vswhere)) return _devenv;

		std::string _output;
		if (!RunHidden(L"\"" + _vswhere.wstring() + L"\" -latest -prerelease -products * -property productPath", true, &_output)) return _devenv;

		const size_t _lineEnd = _output.find_first_of("\r\n");
		if (_lineEnd != std::string::npos) _output.resize(_lineEnd);

		if (!_output.empty() && std::filesystem::exists(_output)) _devenv = _output;
		return _devenv;
	}

	// Is a Visual Studio window titled with the solution name already open?
	bool StudioIsOpen(const std::wstring& _solutionName)
	{
		struct Search { const std::wstring* name; bool found; } _search { &_solutionName, false };

		EnumWindows([](HWND _window, LPARAM _parameter) -> BOOL
		{
			Search* _search = reinterpret_cast<Search*>(_parameter);
			if (!IsWindowVisible(_window)) return TRUE;

			wchar_t _title[512];
			if (GetWindowTextW(_window, _title, 512) == 0) return TRUE;
			if (std::wstring(_title).find(*_search->name) == std::wstring::npos) return TRUE;

			DWORD _processId = 0;
			GetWindowThreadProcessId(_window, &_processId);
			HANDLE _process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, _processId);
			if (!_process) return TRUE;

			wchar_t _image[MAX_PATH];
			DWORD _size = MAX_PATH;
			if (QueryFullProcessImageNameW(_process, 0, _image, &_size))
			{
				std::wstring _name = std::filesystem::path(_image).filename().wstring();
				std::transform(_name.begin(), _name.end(), _name.begin(), ::towlower);
				if (_name == L"devenv.exe") _search->found = true;
			}
			CloseHandle(_process);
			return _search->found ? FALSE : TRUE;
		}, reinterpret_cast<LPARAM>(&_search));

		return _search.found;
	}

	// True if a script of Content/Scripts is not in the Game project yet (a project only lists the files that existed when CMake ran).
	bool SolutionMissesScripts()
	{
		std::ifstream _file("Build/vs2026/Game/Game.vcxproj", std::ios::binary);
		if (!_file) return true;
		const std::string _project((std::istreambuf_iterator<char>(_file)), std::istreambuf_iterator<char>());

		std::error_code _error;
		for (const auto& _entry : std::filesystem::recursive_directory_iterator(SCRIPTS_DIRECTORY, _error))
		{
			const std::string _extension = _entry.path().extension().string();
			if (!_entry.is_regular_file() || (_extension != ".h" && _extension != ".cpp")) continue;

			std::string _relative = _entry.path().generic_string();
			std::replace(_relative.begin(), _relative.end(), '/', '\\');
			if (_project.find(_relative) == std::string::npos) return true;
		}

		// Files the project still lists but that are gone (deleted outside the editor): the build would fail on them.
		const std::string _marker = "Content\\Scripts\\";
		for (size_t _position = _project.find(_marker); _position != std::string::npos; _position = _project.find(_marker, _position))
		{
			const size_t _begin = _project.rfind('"', _position) + 1;
			const size_t _end = _project.find('"', _position);
			if (_end == std::string::npos) break;
			if (!std::filesystem::exists(_project.substr(_begin, _end - _begin))) return true;
			_position = _end;
		}
		return false;
	}

	void Regenerate(const bool& _wait)
	{
		const std::string _cmake = KRAMPUS_CMAKE_COMMAND;
		const std::wstring _command = L"\"" + std::wstring(_cmake.begin(), _cmake.end()) + L"\" --preset " + PRESET;
		LOG_MSG("Updating the Visual Studio solution (cmake --preset windows-vs2026)...");
		if (!RunHidden(_command, _wait))
			LOG_ERROR("Cannot run CMake. Run it yourself: cmake --preset windows-vs2026");
	}
}

void Krampus::RefreshSolution(const bool& _force)
{
	if (FindSolution().empty() || (!_force && !SolutionMissesScripts())) return;
	Regenerate(false);
}

void Krampus::OpenSolution(const std::filesystem::path& _file)
{
	// Make the solution, or add the scripts that were created after it was generated.
	if (FindSolution().empty() || SolutionMissesScripts())
		Regenerate(true);

	const std::filesystem::path _solution = FindSolution();
	if (_solution.empty())
	{
		LOG_ERROR("No Visual Studio solution found in Build/vs2026 and CMake could not generate it. Run: cmake --preset windows-vs2026");
		return;
	}

	const std::wstring _solutionPath = std::filesystem::absolute(_solution).wstring();
	const std::wstring _filePath = _file.empty() ? L"" : std::filesystem::absolute(_file).wstring();
	const std::filesystem::path _devenv = FindDevenv();

	if (_devenv.empty())
	{
		// No Visual Studio found by vswhere: let Windows open the files with what it has associated.
		ShellExecuteW(nullptr, L"open", _solutionPath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
		if (!_filePath.empty()) ShellExecuteW(nullptr, L"open", _filePath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	}
	else if (StudioIsOpen(_solution.stem().wstring()))
	{
		// Same Visual Studio: /Edit opens the file in the running instance. Without a file, opening the solution brings it to the front.
		if (!_filePath.empty())
			ShellExecuteW(nullptr, L"open", _devenv.c_str(), (L"/Edit \"" + _filePath + L"\"").c_str(), nullptr, SW_SHOWNORMAL);
		else
			ShellExecuteW(nullptr, L"open", _solutionPath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	}
	else
	{
		std::wstring _arguments = L"\"" + _solutionPath + L"\"";
		if (!_filePath.empty()) _arguments += L" /Edit \"" + _filePath + L"\"";
		ShellExecuteW(nullptr, L"open", _devenv.c_str(), _arguments.c_str(), nullptr, SW_SHOWNORMAL);
	}

	LOG_MSG("Opening " + _solution.string() + (_file.empty() ? std::string() : " on " + _file.string()) + ". Build EditorModule in Visual Studio to hot reload the game.");
}

void Krampus::PrepareSolutionForBuild()
{
	// A build that finds new scripts regenerates the projects in the middle of itself and leaves them out that time:
	// CMake must run first, so that the very first build after a script was added already compiles it.
	if (FindSolution().empty() || SolutionMissesScripts())
		Regenerate(true);
}

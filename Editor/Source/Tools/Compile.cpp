#include "EditorContext.h"

#include "Core/Engine.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shobjidl.h>
#include <future>
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")

using namespace Krampus;

#ifndef KRAMPUS_CMAKE_COMMAND
#define KRAMPUS_CMAKE_COMMAND "cmake"
#endif

// "Compile" button: builds the EditorModule target the way Visual Studio would (CMake first looks for new scripts, so a class
// created a minute ago is part of the build). The compiler messages go to the Console. Once the DLL is written, Editor.exe reloads it.

namespace
{
	struct CompileState
	{
		HANDLE						process		= nullptr;
		HANDLE						pipe		= nullptr;
		std::string					pending;						// output of the compiler not yet cut in lines
		std::set<std::string>		reported;						// error lines already shown (MSBuild repeats some)
		int							errors		= 0;
		int							warnings	= 0;
		bool						shipping	= false;						// the Package target (Ship configuration) instead of EditorModule
		std::filesystem::path		packageDir;									// shipping: where the game goes
	};

	CompileState g_compile;

	std::string Lower(std::string _text)
	{
		std::transform(_text.begin(), _text.end(), _text.begin(), [](unsigned char _c) { return static_cast<char>(std::tolower(_c)); });
		return _text;
	}

	void HandleLine(std::string _line)
	{
		while (!_line.empty() && (_line.back() == '\r' || _line.back() == ' ')) _line.pop_back();
		if (_line.empty()) return;

		// Strip the project path at the end of MSBuild lines: "file.cpp(12,3): error C2065: ... [C:\...\X.vcxproj]"
		const size_t _bracket = _line.rfind(" [");
		if (_bracket != std::string::npos && _line.back() == ']') _line.resize(_bracket);

		const std::string _lower = Lower(_line);
		if (_lower.find(": error") != std::string::npos || _lower.find("fatal error") != std::string::npos || _lower.find(" error ") != std::string::npos)
		{
			if (g_compile.reported.insert(_line).second)
			{
				++g_compile.errors;
				LOG_ERROR(_line);
			}
		}
		else if (_lower.find("warning") != std::string::npos)
		{
			++g_compile.warnings;
			if (_lower.find("cmake warning") != std::string::npos) LOG_WARNING(_line);	// the package script warns about a missing scene...
		}
		else if (_line.find("EditorModule.vcxproj ->") != std::string::npos || _line.find("KrampusGame.vcxproj ->") != std::string::npos)
		{
			LOG_MSG(_line);
		}
		else if (_line.rfind("-- Package", 0) == 0)
		{
			LOG_MSG(_line.substr(3));	// what the package script says
		}
	}

	void ReadOutput()
	{
		char _buffer[1024];
		DWORD _available = 0;
		while (PeekNamedPipe(g_compile.pipe, nullptr, 0, nullptr, &_available, nullptr) && _available > 0)
		{
			DWORD _count = 0;
			if (!ReadFile(g_compile.pipe, _buffer, std::min<DWORD>(_available, sizeof(_buffer)), &_count, nullptr) || _count == 0) break;
			g_compile.pending.append(_buffer, _count);
		}

		size_t _end;
		while ((_end = g_compile.pending.find('\n')) != std::string::npos)
		{
			HandleLine(g_compile.pending.substr(0, _end));
			g_compile.pending.erase(0, _end + 1);
		}
	}
}

// ---- Build lock
// A build outlives the module that started it: the module is reloaded when the DLL is written, while MSBuild still finishes its work.
// Build/vs2026/KrampusBuild.lock holds the id of the running build process, so that the reloaded module (and the editor host,
// Editor/Source/Main.cpp, which waits for it before reloading) know a build is still going and do not start a second one.

namespace
{
	const std::filesystem::path BUILD_LOCK = "Build/vs2026/KrampusBuild.lock";

	void WriteBuildLock(const DWORD _processId)
	{
		std::ofstream _file(BUILD_LOCK, std::ios::trunc);
		_file << _processId;
	}

	void RemoveBuildLock()
	{
		std::error_code _error;
		std::filesystem::remove(BUILD_LOCK, _error);
	}

	/// <returns>True when the lock names a cmake process that is still running (a stale lock is removed).</returns>
	bool LockedBuildRunning()
	{
		std::ifstream _file(BUILD_LOCK);
		DWORD _processId = 0;
		if (!_file || !(_file >> _processId) || _processId == 0) return false;
		_file.close();

		bool _running = false;
		if (HANDLE _process = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, _processId))
		{
			wchar_t _image[MAX_PATH];
			DWORD _length = MAX_PATH;
			const bool _isCmake = QueryFullProcessImageNameW(_process, 0, _image, &_length)
				&& std::filesystem::path(std::wstring(_image, _length)).filename() == L"cmake.exe";	// the id may belong to another program by now
			_running = _isCmake && WaitForSingleObject(_process, 0) == WAIT_TIMEOUT;
			CloseHandle(_process);
		}
		if (!_running) RemoveBuildLock();
		return _running;
	}
}

namespace
{
	bool Picking();		// the folder of Package Game is being chosen (below)
}

bool Krampus::IsCompiling()
{
	if (g_compile.process != nullptr || Picking()) return true;

	// A build started by the module before a hot reload: checked twice a second, not at each of the calls of a frame.
	static sf::Clock _clock;
	static bool _external = false;
	static bool _checked = false;
	if (!_checked || _clock.getElapsedTime().asSeconds() > 0.5f)
	{
		_external = LockedBuildRunning();
		_checked = true;
		_clock.restart();
	}
	return _external;
}

namespace
{
	void StartBuild(const bool& _shipping)
	{
	if (IsCompiling() || LockedBuildRunning())
	{
		LOG_WARNING("A build is already running: wait until it ends (the Console says when)");
		return;
	}

	PrepareSolutionForBuild();

#ifdef DEBUG
	const char* _editorConfig = "Debug";
#else
	const char* _editorConfig = "Release";
#endif
	// The editor builds its own module in its own configuration; a package is always made in Ship.
	const char* _config = _shipping ? "Ship" : _editorConfig;
	const char* _target = _shipping ? "Package" : "EditorModule";

	SECURITY_ATTRIBUTES _security { sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
	HANDLE _read = nullptr, _write = nullptr;
	if (!CreatePipe(&_read, &_write, &_security, 0))
	{
		LOG_ERROR("Cannot start the compilation (no pipe)");
		return;
	}
	SetHandleInformation(_read, HANDLE_FLAG_INHERIT, 0);

	STARTUPINFOW _startup { };
	_startup.cb = sizeof(_startup);
	_startup.dwFlags = STARTF_USESTDHANDLES;
	_startup.hStdOutput = _write;
	_startup.hStdError = _write;

	const std::string _cmake = KRAMPUS_CMAKE_COMMAND;
	const std::string _config2 = _config;
	std::wstring _command = L"\"" + std::wstring(_cmake.begin(), _cmake.end()) + L"\" --build \"Build/vs2026\" --config "
		+ std::wstring(_config2.begin(), _config2.end()) + L" --target " + std::wstring(_target, _target + std::strlen(_target)) + L" -- /nologo /verbosity:minimal";

	const std::wstring _directory = std::filesystem::current_path().wstring();
	PROCESS_INFORMATION _process { };
	const bool _started = CreateProcessW(nullptr, _command.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW,
		nullptr, _directory.c_str(), &_startup, &_process) != 0;
	CloseHandle(_write);

	if (!_started)
	{
		CloseHandle(_read);
		LOG_ERROR("Cannot run CMake. Is the solution generated? Run: cmake --preset windows-vs2026");
		return;
	}

	CloseHandle(_process.hThread);
	WriteBuildLock(_process.dwProcessId);
	g_compile = CompileState { };
	g_compile.process = _process.hProcess;
	g_compile.pipe = _read;
	g_compile.shipping = _shipping;
	LOG_MSG(_shipping ? "Packaging the game (Ship build: the engine and the game are compiled again and gathered with their Content and the runtime DLLs in a folder, a few minutes the first time)..."
		: std::string("Compiling EditorModule (") + _config + ")...");
	}
}

void Krampus::StartCompile()
{
	StartBuild(false);
}

namespace
{
	const std::filesystem::path DEFAULT_PACKAGE_ROOT = "Bin/windows-x64/Ship/Package";

	/// <summary>The name of the game as a file name (what CMake/Package.cmake does): no \ / : * ? " < > |, no dot or space at the end.</summary>
	std::string PackageName(const std::string& _gameName)
	{
		std::string _name;
		for (const char _c : _gameName)
			if (std::string("\\/:*?\"<>|").find(_c) == std::string::npos) _name += _c;
		while (!_name.empty() && (_name.back() == ' ' || _name.back() == '.')) _name.pop_back();
		while (!_name.empty() && _name.front() == ' ') _name.erase(_name.begin());
		return _name.empty() ? std::string("KrampusGame") : _name;
	}

	std::filesystem::path PackageRoot(const ShipSettings& _ship)
	{
		return _ship.packageFolder.empty() ? DEFAULT_PACKAGE_ROOT : std::filesystem::path(_ship.packageFolder);
	}

	/// <summary>The folder picker of Windows, opened on _start. Nothing when it is cancelled. Runs on its own thread (FolderPick):
	/// on the main thread, COM is not in the mode the dialog needs (STA) and the call never came back, the editor froze.</summary>
	std::optional<std::filesystem::path> PickFolder(HWND _owner, const std::filesystem::path& _start)
	{
		const HRESULT _init = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		std::optional<std::filesystem::path> _result;
		IFileOpenDialog* _dialog = nullptr;
		if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_dialog))))
		{
			DWORD _options = 0;
			_dialog->GetOptions(&_options);
			_dialog->SetOptions(_options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
			_dialog->SetTitle(L"Package Game: where to write the game (it goes in a folder named after it)");
			_dialog->SetOkButtonLabel(L"Package here");
			std::error_code _error;
			const std::filesystem::path _folder = std::filesystem::absolute(_start, _error);
			IShellItem* _startItem = nullptr;
			if (std::filesystem::is_directory(_folder, _error)
				&& SUCCEEDED(SHCreateItemFromParsingName(_folder.c_str(), nullptr, IID_PPV_ARGS(&_startItem))))
			{
				_dialog->SetFolder(_startItem);
				_startItem->Release();
			}
			IShellItem* _item = nullptr;
			if (SUCCEEDED(_dialog->Show(_owner)) && SUCCEEDED(_dialog->GetResult(&_item)))
			{
				PWSTR _path = nullptr;
				if (SUCCEEDED(_item->GetDisplayName(SIGDN_FILESYSPATH, &_path)))
				{
					_result = std::filesystem::path(_path);
					CoTaskMemFree(_path);
				}
				_item->Release();
			}
			_dialog->Release();
		}
		if (SUCCEEDED(_init)) CoUninitialize();
		return _result;
	}

	/// <summary>The folder picker open on its thread: the editor keeps running, PumpCompile starts the package when it closes.</summary>
	struct FolderPickState
	{
		std::future<std::optional<std::filesystem::path>>	result;
		Engine*												engine		= nullptr;
	};

	FolderPickState g_pick;

	bool Picking()
	{
		return g_pick.result.valid();
	}

	void StartPackage(Engine* _engine, std::optional<std::filesystem::path> _chosen);
}

void Krampus::StartShip(EditorContext& _context)
{
	if (Picking()) return;
	if (IsCompiling() || LockedBuildRunning())
	{
		LOG_WARNING("A build is already running: wait until it ends (the Console says when)");
		return;
	}
	const std::filesystem::path _root = PackageRoot(_context.engine->GetSettings().ship);
	std::error_code _error;
	std::filesystem::create_directories(_root, _error);	// the picker opens on it

	const HWND _owner = static_cast<HWND>(_context.engine->GetWindowRef().GetRenderWindow().getNativeHandle());
	g_pick.engine = _context.engine;
	g_pick.result = std::async(std::launch::async, [_owner, _root]() { return PickFolder(_owner, _root); });
	LOG_MSG("Package Game: choose the folder where the game goes");
}

namespace
{
void PumpFolderPick()
{
	if (!Picking() || g_pick.result.wait_for(std::chrono::seconds(0)) != std::future_status::ready) return;
	std::optional<std::filesystem::path> _chosen = g_pick.result.get();
	g_pick.result = { };
	StartPackage(g_pick.engine, std::move(_chosen));
}

void StartPackage(Engine* _engine, std::optional<std::filesystem::path> _chosen)
{
	if (!_chosen)
	{
		LOG_MSG("Package Game: cancelled");
		return;
	}
	EngineSettings& _settings = _engine->GetSettings();
	ShipSettings& _ship = _settings.ship;
	std::error_code _error;
	// The folder of the game itself was picked (an older package): its parent is meant.
	if (_chosen->filename() == std::filesystem::path(PackageName(_ship.gameName)) && std::filesystem::exists(*_chosen / "BuildInfo.txt", _error))
		_chosen = _chosen->parent_path();

	// Remembered in the settings (CMake/Package.cmake reads them): the default folder stays written as "nothing".
	const bool _isDefault = std::filesystem::equivalent(*_chosen, DEFAULT_PACKAGE_ROOT, _error);
	_ship.packageFolder = _isDefault ? std::string() : _chosen->generic_string();
	if (!_settings.Save()) LOG_WARNING("Package Game: the settings could not be saved, the package goes to " + PackageRoot(_ship).generic_string());

	StartBuild(true);
	g_compile.packageDir = PackageRoot(_ship) / PackageName(_ship.gameName);
}
}

bool Krampus::IsShipping()
{
	return g_compile.shipping;
}

std::filesystem::path Krampus::GetPackageDirectory(const EditorContext& _context)
{
	const ShipSettings& _ship = _context.engine->GetSettings().ship;
	return PackageRoot(_ship) / PackageName(_ship.gameName);
}

void Krampus::PumpCompile()
{
	PumpFolderPick();
	if (g_compile.process == nullptr && Picking()) return;
	if (!IsCompiling()) return;

	ReadOutput();
	if (WaitForSingleObject(g_compile.process, 0) != WAIT_OBJECT_0) return;

	ReadOutput();
	if (!g_compile.pending.empty()) HandleLine(g_compile.pending);

	DWORD _exitCode = 1;
	GetExitCodeProcess(g_compile.process, &_exitCode);
	CloseHandle(g_compile.process);
	CloseHandle(g_compile.pipe);
	RemoveBuildLock();

	const int _errors = g_compile.errors;
	const int _warnings = g_compile.warnings;
	const bool _shipping = g_compile.shipping;
	const std::filesystem::path _packageDir = g_compile.packageDir;
	g_compile = CompileState { };

	if (_exitCode == 0 && _shipping)
		LOG_MSG("Package ready: " + std::filesystem::absolute(_packageDir).generic_string() + " (copy the whole folder to another PC and run "
			+ _packageDir.filename().string() + ".exe: nothing to install).");
	else if (_exitCode == 0)
		LOG_MSG("Compilation succeeded (" + std::to_string(_warnings) + " warnings). The game reloads in a moment.");
	else if (_shipping)
		LOG_ERROR("Packaging FAILED (" + std::to_string(_errors) + " errors, see above).");
	else
		LOG_ERROR("Compilation FAILED (" + std::to_string(_errors) + " errors, see above). Fix them and press Compile again.");
}

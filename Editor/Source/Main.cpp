// Editor host: owns the window and the main loop, loads EditorModule.dll (engine + game + editor tools) and hot reloads it
// whenever it is rebuilt (or when F5 is pressed). It does not link the engine on purpose.
#include <SFML/Graphics.hpp>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "CrashReporter.h"

namespace fs = std::filesystem;

class GameModule
{
	using InitFn		= bool(*)(void*);
	using FrameFn		= bool(*)();
	using ShutdownFn	= void(*)();

	fs::path				source;							// EditorModule.dll as written by the linker
	fs::path				liveCopy;						// the copy that is actually loaded, so the linker can overwrite the original
	HMODULE					handle		= nullptr;
	FrameFn					frame		= nullptr;
	ShutdownFn				shutdown	= nullptr;
	fs::file_time_type		loadedStamp { };
	int						counter		= 0;

public:
	/// Every link writes a new EditorModule_<time>.pdb (see Editor/CMakeLists.txt). Keeps the newest ones, deletes the others;
	/// a PDB still in use cannot be deleted and is just kept.
	static void CleanOldPdbs(const fs::path& _directory, const size_t _keep)
	{
		std::error_code _error;
		std::vector<fs::directory_entry> _pdbs;
		for (const fs::directory_entry& _entry : fs::directory_iterator(_directory, _error))
		{
			const std::string _name = _entry.path().filename().string();
			if (_name.starts_with("EditorModule_") && _entry.path().extension() == ".pdb")
				_pdbs.push_back(_entry);
		}

		std::sort(_pdbs.begin(), _pdbs.end(), [](const fs::directory_entry& _a, const fs::directory_entry& _b) { return _a.last_write_time() > _b.last_write_time(); });
		for (size_t _i = _keep; _i < _pdbs.size(); ++_i)
			fs::remove(_pdbs[_i].path(), _error);
	}

public:
	explicit GameModule(const fs::path& _source)
		: source(_source)
	{
		// Copies left behind by a crashed session.
		std::error_code _error;
		CleanOldPdbs(source.parent_path(), 2);
		for (const fs::directory_entry& _entry : fs::directory_iterator(source.parent_path(), _error))
			if (_entry.path().filename().string().starts_with("EditorModule_live_"))
				fs::remove(_entry.path(), _error);
	}

	~GameModule()
	{
		Unload();
	}

	bool IsLoaded() const
	{
		return handle != nullptr;
	}

	/// <summary>Timestamp of EditorModule.dll on disk, or a default value if it does not exist.</summary>
	fs::file_time_type DiskStamp() const
	{
		std::error_code _error;
		const fs::file_time_type _stamp = fs::last_write_time(source, _error);
		return _error ? fs::file_time_type{ } : _stamp;
	}

	bool HasNewBuild() const
	{
		const fs::file_time_type _stamp = DiskStamp();
		return _stamp != fs::file_time_type{ } && _stamp != loadedStamp;
	}

	/// <returns>False while another program (the linker) still has EditorModule.dll open: loading it now would load half a file.</returns>
	bool IsFileReady() const
	{
		const HANDLE _file = CreateFileW(source.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (_file == INVALID_HANDLE_VALUE) return false;
		CloseHandle(_file);
		return true;
	}

	/// <summary>
	/// Loads a fresh copy of EditorModule.dll and starts the game in the host window.
	/// A failed copy or load (the linker may still be writing) is retried later; a failed GameInit is not, until the next build.
	/// </summary>
	bool Load(sf::RenderWindow& _window)
	{
		const fs::file_time_type _stamp = DiskStamp();
		if (_stamp == fs::file_time_type{ }) return false;

		std::error_code _error;
		liveCopy = source.parent_path() / ("EditorModule_live_" + std::to_string(++counter) + ".dll");
		fs::copy_file(source, liveCopy, fs::copy_options::overwrite_existing, _error);
		if (_error) return false;

		handle = LoadLibraryW(liveCopy.c_str());
		if (!handle)
		{
			fs::remove(liveCopy, _error);
			return false;
		}

		const auto _init = reinterpret_cast<InitFn>(GetProcAddress(handle, "GameInit"));
		frame = reinterpret_cast<FrameFn>(GetProcAddress(handle, "GameFrame"));
		shutdown = reinterpret_cast<ShutdownFn>(GetProcAddress(handle, "GameShutdown"));
		if (!_init || !frame || !shutdown)
		{
			std::cerr << "[Editor] EditorModule.dll does not export GameInit/GameFrame/GameShutdown" << std::endl;
			loadedStamp = _stamp;
			Unload();
			return false;
		}

		loadedStamp = _stamp;
		std::cout << "[Editor] Game loaded (" << liveCopy.filename().string() << ")" << std::endl;
		if (!_init(&_window))
		{
			Unload();
			return false;
		}
		return true;
	}

	void Unload()
	{
		if (!handle) return;

		shutdown();
		FreeLibrary(handle);
		handle = nullptr;
		frame = nullptr;
		shutdown = nullptr;

		std::error_code _error;
		fs::remove(liveCopy, _error);
	}

	/// <returns>False when the game wants to quit.</returns>
	bool Frame()
	{
		return frame();
	}
};

static fs::path ExecutableDirectory()
{
	wchar_t _buffer[MAX_PATH];
	const DWORD _length = GetModuleFileNameW(nullptr, _buffer, MAX_PATH);
	return fs::path(std::wstring(_buffer, _length)).parent_path();
}

// A build started from the editor writes the id of its cmake process in this file (Editor/Source/Tools/Compile.cpp) and removes it
// when it ends. The module is not reloaded while it runs: MSBuild still works after the DLL is written, and a module loaded in the
// middle would believe no build is running (a second Compile would then start two builds on the same files).
static bool BuildRunning()
{
	std::ifstream _file("Build/vs2026/KrampusBuild.lock");
	DWORD _processId = 0;
	if (!_file || !(_file >> _processId) || _processId == 0) return false;

	bool _running = false;
	if (HANDLE _process = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, _processId))
	{
		wchar_t _image[MAX_PATH];
		DWORD _length = MAX_PATH;
		const bool _isCmake = QueryFullProcessImageNameW(_process, 0, _image, &_length)
			&& fs::path(std::wstring(_image, _length)).filename() == L"cmake.exe";
		_running = _isCmake && WaitForSingleObject(_process, 0) == WAIT_TIMEOUT;
		CloseHandle(_process);
	}
	return _running;
}

// The engine loads "Content/..." relative to the working directory: go up until the project root is found.
static void EnterProjectRoot(const fs::path& _start)
{
	for (fs::path _dir = _start; ; _dir = _dir.parent_path())
	{
		if (fs::exists(_dir / "Content"))
		{
			fs::current_path(_dir);
			return;
		}
		if (_dir == _dir.parent_path()) return;
	}
}

int main()
{
	// Started by the crash handler of a crashed editor: only shows the report dialog.
	if (CrashReporter::RunReporterIfRequested())
		return 0;

	const fs::path _exeDir = ExecutableDirectory();
	EnterProjectRoot(_exeDir);
	CrashReporter::Install();

	sf::RenderWindow _window(sf::VideoMode({ 1600u, 900u }), "Krampus Editor");
	_window.setVerticalSyncEnabled(true);

	GameModule _game(_exeDir / "EditorModule.dll");
	const auto _setTitle = [&_window](const std::string& _state)
	{
		_window.setTitle(_state.empty() ? "Krampus Editor" : "Krampus Editor - " + _state);
	};
	if (!_game.Load(_window))
		_setTitle("the editor module could not start: fix the build (see the console), then build again or press F5");

	sf::Clock			_pollClock;
	fs::file_time_type	_lastSeen { };
	bool				_f5Held		= false;
	bool				_waitingForBuild = false;

	while (_window.isOpen())
	{
		bool _reload = false;

		const bool _f5 = _window.hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F5);
		if (_f5 && !_f5Held) _reload = true;
		_f5Held = _f5;

		// A build is reloaded once its timestamp stayed the same for a whole poll interval, i.e. the linker is done.
		if (_pollClock.getElapsedTime().asSeconds() > 0.5f)
		{
			_pollClock.restart();
			const fs::file_time_type _stamp = _game.DiskStamp();
			if (_game.HasNewBuild())
			{
				if (_stamp == _lastSeen) _reload = true;
				_lastSeen = _stamp;
			}
		}

		// Never in the middle of a build, never a DLL the linker is still writing: the reload waits.
		if (_reload && (BuildRunning() || !_game.IsFileReady()))
		{
			if (!_waitingForBuild) std::cout << "[Editor] A build is running: the reload waits for its end" << std::endl;
			_waitingForBuild = true;
			_reload = false;
			_lastSeen = { };	// once it ends, the new stamp must be seen stable again
		}
		else if (_reload)
			_waitingForBuild = false;

		if (_reload)
		{
			std::cout << "[Editor] Reloading EditorModule.dll..." << std::endl;
			// The module saves the scene being edited when it stops and opens it again when it starts (see Session.cpp).
			SetEnvironmentVariableW(L"KRAMPUS_HOT_RELOAD", L"1");
			_game.Unload();
			GameModule::CleanOldPdbs(_exeDir, 3);
			const bool _loaded = _game.Load(_window);
			SetEnvironmentVariableW(L"KRAMPUS_HOT_RELOAD", nullptr);
			_setTitle(_loaded ? "" : "the editor module could not start: fix the build (see the console), then build again or press F5");
		}

		if (_game.IsLoaded())
		{
			if (!_game.Frame()) break;
		}
		else
		{
			// Nothing to run (build failed or in progress): keep the window alive and wait for the next build.
			while (const std::optional<sf::Event> _event = _window.pollEvent())
				if (_event->is<sf::Event::Closed>())
					_window.close();

			_window.clear(sf::Color(25, 25, 28));
			_window.display();
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}

	_game.Unload();
	if (_window.isOpen()) _window.close();
	return 0;
}

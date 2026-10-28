// Editor host: owns the window and the main loop, loads EditorModule.dll (engine + game + editor tools) and hot reloads it
// whenever it is rebuilt (or when F5 is pressed). It does not link the engine on purpose.
#include <SFML/Graphics.hpp>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

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
	explicit GameModule(const fs::path& _source)
		: source(_source)
	{
		// Copies left behind by a crashed session.
		std::error_code _error;
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
	const fs::path _exeDir = ExecutableDirectory();
	EnterProjectRoot(_exeDir);

	sf::RenderWindow _window(sf::VideoMode({ 1600u, 900u }), "Krampus Editor");
	_window.setVerticalSyncEnabled(true);

	GameModule _game(_exeDir / "EditorModule.dll");
	_game.Load(_window);

	sf::Clock			_pollClock;
	fs::file_time_type	_lastSeen { };
	bool				_f5Held		= false;

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

		if (_reload)
		{
			std::cout << "[Editor] Reloading EditorModule.dll..." << std::endl;
			_game.Unload();
			_game.Load(_window);
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

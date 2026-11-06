// The game alone, without the editor: this is what a Ship build packages (see Ship/CMakeLists.txt and CMake/Package.cmake).
// It makes the engine, lets the game choose its first scene (Game/Source/GameEntry.cpp) and runs until the window is closed.

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//Core engine includes
#include "Core/AssetPack.h"
#include "Core/Engine.h"
#include "Network/NetworkManager.h"
#include "Scene/Scene.h"
#include "Core/CoreMinimal.h"

//Game
#include "GameEntry.h"

namespace
{
	// A packaged game (CMake/Package.cmake) is laid out like this:
	//     <Game>/<Game>.exe              the launcher (Ship/Launcher): starts the game below
	//     <Game>/Binaries/<Game>.exe     this program, with its DLLs
	//     <Game>/Content/<Game>.pak      the assets
	// The pak is mounted before anything loads a file (an older package has KrampusGame.pak next to the exe). The game then moves to
	// the root of the package, wherever it is started from (a shortcut, a console): its saves go to <Game>/Content/Saves.
	// In the other configurations the game is started from the project root, like the editor, and reads the files from the disk.
	void Prepare()
	{
		wchar_t _buffer[MAX_PATH];
		const DWORD _length = GetModuleFileNameW(nullptr, _buffer, MAX_PATH);
		if (_length == 0 || _length >= MAX_PATH) return;

		const std::filesystem::path _exeDirectory = std::filesystem::path(std::wstring(_buffer, _length)).parent_path();
		std::filesystem::path _root = _exeDirectory;
		std::error_code _error;
		if (std::filesystem::exists(_exeDirectory / "KrampusGame.pak", _error))
			Krampus::AssetPack::Mount(_exeDirectory / "KrampusGame.pak");
		else if (_exeDirectory.filename() == L"Binaries")
		{
			_root = _exeDirectory.parent_path();
			for (const std::filesystem::directory_entry& _entry : std::filesystem::directory_iterator(_root / "Content", _error))
				if (_entry.path().extension() == L".pak" && Krampus::AssetPack::Mount(_entry.path())) break;
		}
#ifdef SHIP
		std::filesystem::current_path(_root, _error);
#endif
	}

	void ReportError(const std::string& _message)
	{
#ifdef SHIP
		MessageBoxA(nullptr, _message.c_str(), "KrampusEngine", MB_OK | MB_ICONERROR);
#else
		std::cerr << _message << std::endl;
#endif
	}
}

int main(int argc, char** argv)
{
	Prepare();

	try
	{
		Krampus::Engine _engine;
		Krampus::GameStart(_engine);

		// Command line: --scene Name opens a scene; --host [port] / --join address[:port] start a network game (to test with two copies)
		for (int _i = 1; _i < argc; ++_i)
		{
			const std::string _argument = argv[_i];
			if (_argument == "--host")
			{
				if (_i + 1 < argc && std::isdigit(static_cast<unsigned char>(argv[_i + 1][0])))
					_engine.GetNetwork()->Host(static_cast<unsigned short>(std::atoi(argv[++_i])));
				else
					_engine.GetNetwork()->Host();
			}
			else if (_argument == "--scene" && _i + 1 < argc)
				Krampus::Scene::Open(_engine, argv[++_i]);
			else if (_argument == "--join")
			{
				if (_i + 1 < argc) _engine.GetNetwork()->Join(argv[++_i]);
				else _engine.GetNetwork()->Join();
			}
		}
		_engine.Start();
	}
	catch (const Krampus::Exception& _exception)
	{
		ReportError(_exception.What());
		return 1;
	}
	catch (const std::exception& _exception)	// the standard library and SFML throw their own (no memory, a file system error...)
	{
		ReportError(std::string("Unexpected error: ") + _exception.what());
		return 1;
	}
	return 0;
}

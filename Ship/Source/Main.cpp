// The game alone, without the editor: this is what a Ship build packages (see Ship/CMakeLists.txt and CMake/Package.cmake).
// It makes the engine, lets the game choose its first scene (Game/Source/GameEntry.cpp) and runs until the window is closed.

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//Core engine includes
#include "Core/AssetPack.h"
#include "Core/Engine.h"
#include "Utilities/KrampusInclude.h"

//Game
#include "GameEntry.h"

namespace
{
	// A packaged game has its assets in KrampusGame.pak, next to its exe: the pak is mounted before anything loads a file.
	// It also moves to the folder of its exe, wherever it is started from (a shortcut, a console), where it writes its saves.
	// In the other configurations the game is started from the project root, like the editor, and reads the files from the disk.
	void Prepare()
	{
		wchar_t _buffer[MAX_PATH];
		const DWORD _length = GetModuleFileNameW(nullptr, _buffer, MAX_PATH);
		if (_length == 0 || _length >= MAX_PATH) return;

		const std::filesystem::path _exeDirectory = std::filesystem::path(std::wstring(_buffer, _length)).parent_path();
		Krampus::AssetPack::Mount(_exeDirectory / "KrampusGame.pak");
#ifdef SHIP
		std::error_code _error;
		std::filesystem::current_path(_exeDirectory, _error);
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

int main()
{
	Prepare();

	try
	{
		Krampus::Engine _engine;
		Krampus::GameStart(_engine);
		_engine.Start();
	}
	catch (const Krampus::Exception& _exception)
	{
		ReportError(_exception.What());
		return 1;
	}
	return 0;
}

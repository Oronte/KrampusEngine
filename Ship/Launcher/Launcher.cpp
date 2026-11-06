// The launcher of a packaged game: <Game>/<Game>.exe, the only program at the root of the package (CMake/Package.cmake).
// The game itself and its DLLs are in <Game>/Binaries (Windows only finds the DLLs of a program in its own folder, so the game
// cannot sit at the root without them). The launcher starts Binaries/<its own name>.exe with the same command line, in the root
// folder, and waits for it (Steam sees the game running as long as the launcher runs). Its icon is set by PackTool --icon.
//
// No engine, no SFML, the C runtime linked in (static): it runs before anything of the game is found.

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

namespace
{
	std::wstring Folder(const std::wstring& _path)
	{
		const size_t _slash = _path.find_last_of(L"\\/");
		return _slash == std::wstring::npos ? std::wstring(L".") : _path.substr(0, _slash);
	}

	std::wstring FileName(const std::wstring& _path)
	{
		const size_t _slash = _path.find_last_of(L"\\/");
		return _slash == std::wstring::npos ? _path : _path.substr(_slash + 1);
	}

	/// <returns>The arguments of the command line, without the program (with their leading space).</returns>
	std::wstring Arguments()
	{
		const wchar_t* _line = GetCommandLineW();
		if (*_line == L'"')
		{
			++_line;
			while (*_line && *_line != L'"') ++_line;
			if (*_line) ++_line;
		}
		else
			while (*_line && *_line != L' ' && *_line != L'\t') ++_line;
		return _line;
	}

	int Fail(const std::wstring& _message)
	{
		MessageBoxW(nullptr, _message.c_str(), L"Launcher", MB_OK | MB_ICONERROR);
		return 1;
	}
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
	std::wstring _self(32768, L'\0');
	const DWORD _length = GetModuleFileNameW(nullptr, _self.data(), static_cast<DWORD>(_self.size()));
	if (_length == 0 || _length >= _self.size()) return Fail(L"Cannot find where the game is installed.");
	_self.resize(_length);

	const std::wstring _root = Folder(_self);
	const std::wstring _game = _root + L"\\Binaries\\" + FileName(_self);
	if (GetFileAttributesW(_game.c_str()) == INVALID_FILE_ATTRIBUTES)
		return Fail(L"The game is missing:\n" + _game + L"\n\nCopy the whole folder of the game, with its Binaries and Content folders.");

	std::wstring _command = L"\"" + _game + L"\"" + Arguments();
	STARTUPINFOW _startup { };
	_startup.cb = sizeof(_startup);
	PROCESS_INFORMATION _process { };
	if (!CreateProcessW(_game.c_str(), _command.data(), nullptr, nullptr, FALSE, 0, nullptr, _root.c_str(), &_startup, &_process))
		return Fail(L"The game could not start:\n" + _game);

	CloseHandle(_process.hThread);
	WaitForSingleObject(_process.hProcess, INFINITE);
	DWORD _exitCode = 0;
	GetExitCodeProcess(_process.hProcess, &_exitCode);
	CloseHandle(_process.hProcess);
	return static_cast<int>(_exitCode);
}

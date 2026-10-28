#include "EditorContext.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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
		}
		else if (_line.find("EditorModule.vcxproj ->") != std::string::npos)
		{
			LOG_MSG(_line);
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

bool Krampus::IsCompiling()
{
	return g_compile.process != nullptr;
}

void Krampus::StartCompile()
{
	if (IsCompiling()) return;

	PrepareSolutionForBuild();

#ifdef DEBUG
	const char* _config = "Debug";
#else
	const char* _config = "Release";
#endif

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
		+ std::wstring(_config2.begin(), _config2.end()) + L" --target EditorModule -- /nologo /verbosity:minimal";

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
	g_compile = CompileState { };
	g_compile.process = _process.hProcess;
	g_compile.pipe = _read;
	LOG_MSG(std::string("Compiling EditorModule (") + _config + ")...");
}

void Krampus::PumpCompile()
{
	if (!IsCompiling()) return;

	ReadOutput();
	if (WaitForSingleObject(g_compile.process, 0) != WAIT_OBJECT_0) return;

	ReadOutput();
	if (!g_compile.pending.empty()) HandleLine(g_compile.pending);

	DWORD _exitCode = 1;
	GetExitCodeProcess(g_compile.process, &_exitCode);
	CloseHandle(g_compile.process);
	CloseHandle(g_compile.pipe);

	const int _errors = g_compile.errors;
	const int _warnings = g_compile.warnings;
	g_compile = CompileState { };

	if (_exitCode == 0)
		LOG_MSG("Compilation succeeded (" + std::to_string(_warnings) + " warnings). The game reloads in a moment.");
	else
		LOG_ERROR("Compilation FAILED (" + std::to_string(_errors) + " errors, see above). Fix them and press Compile again.");
}

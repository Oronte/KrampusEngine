#include "CrashReporter.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>
#include <shellapi.h>
#include <tlhelp32.h>

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

namespace
{
	constexpr DWORD CodeTerminate	= 0xE0000001;	// our own codes, raised by the handlers below
	constexpr DWORD CodeAbort		= 0xE0000002;
	constexpr DWORD CodeInvalidArg	= 0xE0000003;

	EXCEPTION_POINTERS*	crashInfo		= nullptr;
	DWORD				crashThreadId	= 0;
	volatile LONG		crashing		= 0;
	fs::path			projectRoot;
	fs::path			reportDirectory;
	wchar_t				exePath[MAX_PATH] = { };
	char				crashMessage[512] = { };	// what() of the uncaught C++ exception, if any

	const char* ExceptionName(const DWORD _code)
	{
		switch (_code)
		{
		case EXCEPTION_ACCESS_VIOLATION:		return "Access violation";
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:	return "Array bounds exceeded";
		case EXCEPTION_DATATYPE_MISALIGNMENT:	return "Datatype misalignment";
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:		return "Float divide by zero";
		case EXCEPTION_FLT_INVALID_OPERATION:	return "Float invalid operation";
		case EXCEPTION_ILLEGAL_INSTRUCTION:		return "Illegal instruction";
		case EXCEPTION_IN_PAGE_ERROR:			return "In page error";
		case EXCEPTION_INT_DIVIDE_BY_ZERO:		return "Integer divide by zero";
		case EXCEPTION_PRIV_INSTRUCTION:		return "Privileged instruction";
		case EXCEPTION_STACK_OVERFLOW:			return "Stack overflow";
		case CodeTerminate:						return "std::terminate (uncaught C++ exception)";
		case CodeAbort:							return "abort()";
		case CodeInvalidArg:					return "Invalid argument passed to the C runtime";
		default:								return "Unhandled exception";
		}
	}

	/// <summary>Name of the module that contains an address, and the offset inside it.</summary>
	std::string ModuleOf(const DWORD64 _address)
	{
		HMODULE _module = nullptr;
		if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCWSTR>(_address), &_module))
			return "unknown module";

		char _path[MAX_PATH] = { };
		GetModuleFileNameA(_module, _path, MAX_PATH);
		const std::string _name = fs::path(_path).filename().string();
		char _buffer[MAX_PATH + 32];
		std::snprintf(_buffer, sizeof(_buffer), "%s + 0x%llX", _name.c_str(), static_cast<unsigned long long>(_address - reinterpret_cast<DWORD64>(_module)));
		return _buffer;
	}

	void WriteStack(FILE* _file, CONTEXT _context)
	{
		const HANDLE _process = GetCurrentProcess();
		const HANDLE _thread = OpenThread(THREAD_ALL_ACCESS, FALSE, crashThreadId);

		SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
		SymInitialize(_process, nullptr, TRUE);

		STACKFRAME64 _frame = { };
		_frame.AddrPC.Offset		= _context.Rip;
		_frame.AddrPC.Mode			= AddrModeFlat;
		_frame.AddrFrame.Offset		= _context.Rbp;
		_frame.AddrFrame.Mode		= AddrModeFlat;
		_frame.AddrStack.Offset		= _context.Rsp;
		_frame.AddrStack.Mode		= AddrModeFlat;

		alignas(SYMBOL_INFO) char _symbolBuffer[sizeof(SYMBOL_INFO) + 512] = { };
		SYMBOL_INFO* _symbol = reinterpret_cast<SYMBOL_INFO*>(_symbolBuffer);
		_symbol->SizeOfStruct	= sizeof(SYMBOL_INFO);
		_symbol->MaxNameLen		= 512;

		for (int _i = 0; _i < 64; ++_i)
		{
			if (!StackWalk64(IMAGE_FILE_MACHINE_AMD64, _process, _thread, &_frame, &_context, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
				break;
			if (_frame.AddrPC.Offset == 0)
				break;

			const DWORD64 _address = _frame.AddrPC.Offset;
			std::string _function = "(no symbol)";
			DWORD64 _displacement = 0;
			if (SymFromAddr(_process, _address, &_displacement, _symbol))
			{
				char _buffer[640];
				std::snprintf(_buffer, sizeof(_buffer), "%s + 0x%llX", _symbol->Name, static_cast<unsigned long long>(_displacement));
				_function = _buffer;
			}

			std::string _line;
			IMAGEHLP_LINE64 _lineInfo = { };
			_lineInfo.SizeOfStruct = sizeof(_lineInfo);
			DWORD _lineDisplacement = 0;
			if (SymGetLineFromAddr64(_process, _address, &_lineDisplacement, &_lineInfo))
				_line = std::string("  ") + _lineInfo.FileName + ":" + std::to_string(_lineInfo.LineNumber);

			std::fprintf(_file, "#%02d  0x%016llX  %s%s  [%s]\n", _i, static_cast<unsigned long long>(_address), _function.c_str(), _line.c_str(), ModuleOf(_address).c_str());
		}

		if (_thread) CloseHandle(_thread);
	}

	void WriteModules(FILE* _file)
	{
		const HANDLE _snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetCurrentProcessId());
		if (_snapshot == INVALID_HANDLE_VALUE) return;

		MODULEENTRY32W _entry = { };
		_entry.dwSize = sizeof(_entry);
		for (BOOL _ok = Module32FirstW(_snapshot, &_entry); _ok; _ok = Module32NextW(_snapshot, &_entry))
			std::fwprintf(_file, L"0x%016llX  %8lu bytes  %ls\n", static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(_entry.modBaseAddr)), _entry.modBaseSize, _entry.szExePath);
		CloseHandle(_snapshot);
	}

	void WriteReport(const fs::path& _file)
	{
		FILE* _out = nullptr;
		if (_wfopen_s(&_out, _file.c_str(), L"w") != 0 || !_out) return;

		const EXCEPTION_RECORD* _record = crashInfo->ExceptionRecord;
		const DWORD64 _address = reinterpret_cast<DWORD64>(_record->ExceptionAddress);

		std::time_t _now = std::time(nullptr);
		std::tm _local = { };
		localtime_s(&_local, &_now);
		char _date[64];
		std::strftime(_date, sizeof(_date), "%Y-%m-%d %H:%M:%S", &_local);

		// The reporter reads the "Exception:" and "Location:" lines: keep them near the top.
		std::fprintf(_out, "Krampus Editor crash report\nDate: %s\n", _date);
		std::fprintf(_out, "Exception: %s (0x%08lX)\n", ExceptionName(_record->ExceptionCode), _record->ExceptionCode);
		std::fprintf(_out, "Location: %s\n", ModuleOf(_address).c_str());
		if (crashMessage[0])
			std::fprintf(_out, "Message: %s\n", crashMessage);
		if (_record->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && _record->NumberParameters >= 2)
		{
			const ULONG_PTR _kind = _record->ExceptionInformation[0];
			std::fprintf(_out, "Access: %s address 0x%016llX\n", _kind == 0 ? "reading" : _kind == 1 ? "writing" : "executing", static_cast<unsigned long long>(_record->ExceptionInformation[1]));
		}
		std::fprintf(_out, "Thread: %lu\nProcess: %lu\nWorking directory: %ls\n", crashThreadId, GetCurrentProcessId(), projectRoot.c_str());

		std::fprintf(_out, "\n=== Call stack of the crashing thread ===\n");
		WriteStack(_out, *crashInfo->ContextRecord);

		std::fprintf(_out, "\n=== Loaded modules ===\n");
		WriteModules(_out);
		std::fclose(_out);
	}

	void WriteDump(const fs::path& _file)
	{
		const HANDLE _handle = CreateFileW(_file.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (_handle == INVALID_HANDLE_VALUE) return;

		MINIDUMP_EXCEPTION_INFORMATION _exception = { };
		_exception.ThreadId				= crashThreadId;
		_exception.ExceptionPointers	= crashInfo;
		_exception.ClientPointers		= FALSE;

		const MINIDUMP_TYPE _type = static_cast<MINIDUMP_TYPE>(MiniDumpWithDataSegs | MiniDumpWithHandleData | MiniDumpWithThreadInfo | MiniDumpWithUnloadedModules);
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), _handle, _type, &_exception, nullptr, nullptr);
		CloseHandle(_handle);
	}

	/// <summary>The undo snapshot with the highest number: the scene as it was before the last edit.</summary>
	void CopyLastUndo(const fs::path& _destination)
	{
		std::error_code _error;
		fs::path _newest;
		for (const fs::directory_entry& _entry : fs::directory_iterator(projectRoot / "Content" / "Editor" / "_undo", _error))
			if (_entry.path().extension() == ".ka7" && (_newest.empty() || _entry.path().filename() > _newest.filename()))
				_newest = _entry.path();
		if (!_newest.empty())
			fs::copy_file(_newest, _destination / "LastUndoSnapshot.ka7", fs::copy_options::overwrite_existing, _error);
	}

	DWORD WINAPI CrashThread(LPVOID)
	{
		std::error_code _error;
		std::time_t _now = std::time(nullptr);
		std::tm _local = { };
		localtime_s(&_local, &_now);
		char _name[64];
		std::strftime(_name, sizeof(_name), "Crash_%Y-%m-%d_%H-%M-%S", &_local);

		reportDirectory = projectRoot / "Content" / "Logs" / "Crashes" / _name;
		fs::create_directories(reportDirectory, _error);

		WriteReport(reportDirectory / "CrashReport.txt");
		WriteDump(reportDirectory / "Editor.dmp");
		fs::copy_file(projectRoot / "Content" / "Logs" / "log.txt", reportDirectory / "log.txt", fs::copy_options::overwrite_existing, _error);
		CopyLastUndo(reportDirectory);

		// The dialog lives in another process: this one is too damaged to show anything reliably.
		std::wstring _command = L"\"" + std::wstring(exePath) + L"\" --crash-report \"" + reportDirectory.wstring() + L"\"";
		STARTUPINFOW _startup = { };
		_startup.cb = sizeof(_startup);
		PROCESS_INFORMATION _process = { };
		if (CreateProcessW(nullptr, _command.data(), nullptr, nullptr, FALSE, 0, nullptr, projectRoot.c_str(), &_startup, &_process))
		{
			CloseHandle(_process.hThread);
			CloseHandle(_process.hProcess);
		}
		return 0;
	}

	LONG WINAPI Filter(EXCEPTION_POINTERS* _info)
	{
		// A crash while reporting the crash: give up.
		if (InterlockedExchange(&crashing, 1) != 0)
			TerminateProcess(GetCurrentProcess(), 3);

		crashInfo = _info;
		crashThreadId = GetCurrentThreadId();

		// On another thread, so a stack overflow can still be reported.
		const HANDLE _thread = CreateThread(nullptr, 1 << 20, CrashThread, nullptr, 0, nullptr);
		if (_thread) WaitForSingleObject(_thread, 30000);

		// ExitProcess would run the DLL detach code of a process in an unknown state.
		TerminateProcess(GetCurrentProcess(), 1);
		return EXCEPTION_EXECUTE_HANDLER;
	}

	/// <summary>Raises an exception on this thread so Filter gets a context and a call stack, then never returns.</summary>
	void RaiseCrash(const DWORD _code)
	{
		__try
		{
			RaiseException(_code, EXCEPTION_NONCONTINUABLE, 0, nullptr);
		}
		__except (Filter(GetExceptionInformation()))
		{
		}
	}

	void OnTerminate()
	{
		try
		{
			if (const std::exception_ptr _current = std::current_exception())
				std::rethrow_exception(_current);
		}
		catch (const std::exception& _exception)
		{
			std::snprintf(crashMessage, sizeof(crashMessage), "%s", _exception.what());
		}
		catch (...)
		{
		}
		RaiseCrash(CodeTerminate);
	}

	void OnAbort(int)
	{
		RaiseCrash(CodeAbort);
	}

	void OnInvalidParameter(const wchar_t*, const wchar_t*, const wchar_t*, unsigned int, uintptr_t)
	{
		RaiseCrash(CodeInvalidArg);
	}

	/// <summary>Value of a "Key: value" line of CrashReport.txt.</summary>
	std::wstring ReadReportLine(const fs::path& _report, const std::string& _key)
	{
		std::ifstream _in(_report);
		std::string _line;
		while (std::getline(_in, _line))
			if (_line.starts_with(_key))
				return fs::path(_line.substr(_key.size())).wstring();
		return L"?";
	}
}

bool CrashReporter::RunReporterIfRequested()
{
	int _count = 0;
	LPWSTR* _arguments = CommandLineToArgvW(GetCommandLineW(), &_count);
	if (!_arguments) return false;

	bool _isReporter = false;
	fs::path _directory;
	for (int _i = 1; _i + 1 < _count; ++_i)
		if (std::wstring(_arguments[_i]) == L"--crash-report")
		{
			_isReporter = true;
			_directory = _arguments[_i + 1];
		}
	LocalFree(_arguments);
	if (!_isReporter) return false;

	const fs::path _report = _directory / "CrashReport.txt";
	const std::wstring _text =
		L"Krampus Editor s'est arrêté de façon inattendue.\n\n"
		L"Erreur : " + ReadReportLine(_report, "Exception: ") + L"\n"
		L"Endroit : " + ReadReportLine(_report, "Location: ") + L"\n\n"
		L"Un rapport (pile d'appels, dump, log, dernière sauvegarde d'annulation) a été enregistré dans :\n" + _directory.wstring() + L"\n\n"
		L"Oui : ouvrir le dossier du rapport\n"
		L"Non : relancer l'éditeur\n"
		L"Annuler : fermer";

	for (;;)
	{
		const int _answer = MessageBoxW(nullptr, _text.c_str(), L"Krampus Editor - Crash", MB_YESNOCANCEL | MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);
		if (_answer == IDYES)
		{
			ShellExecuteW(nullptr, L"open", _directory.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
			continue;
		}
		if (_answer == IDNO)
		{
			// <root>/Content/Logs/Crashes/Crash_x -> <root>
			const fs::path _root = _directory.parent_path().parent_path().parent_path().parent_path();
			wchar_t _exe[MAX_PATH];
			GetModuleFileNameW(nullptr, _exe, MAX_PATH);
			std::wstring _command = L"\"" + std::wstring(_exe) + L"\"";
			STARTUPINFOW _startup = { };
			_startup.cb = sizeof(_startup);
			PROCESS_INFORMATION _process = { };
			if (CreateProcessW(nullptr, _command.data(), nullptr, nullptr, FALSE, 0, nullptr, _root.c_str(), &_startup, &_process))
			{
				CloseHandle(_process.hThread);
				CloseHandle(_process.hProcess);
			}
		}
		break;
	}
	return true;
}

void CrashReporter::Install()
{
	if (IsDebuggerPresent()) return;

	std::error_code _error;
	projectRoot = fs::current_path(_error);
	GetModuleFileNameW(nullptr, exePath, MAX_PATH);

	SetUnhandledExceptionFilter(Filter);
	std::set_terminate(OnTerminate);
	std::signal(SIGABRT, OnAbort);
	_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);	// no CRT message box, the report is the message
	_set_invalid_parameter_handler(OnInvalidParameter);
}

// Crash reporter of the editor host (Editor.exe only, never the game). When the editor crashes it writes a report in
// Content/Logs/Crashes/Crash_<date>/ (call stack, minidump, log, last undo snapshot) and starts Editor.exe again in "reporter" mode,
// which shows a dialog to open the report folder or restart the editor.
#pragma once

namespace CrashReporter
{
	/// <summary>
	/// If this process was started as the reporter (--crash-report "folder"), shows the dialog and returns true: main must exit.
	/// Call it first thing in main.
	/// </summary>
	bool RunReporterIfRequested();

	/// <summary>
	/// Catches SEH exceptions, uncaught C++ exceptions, abort() and invalid CRT parameters. Call it once the working directory is the project root.
	/// Does nothing while a debugger is attached to the process (the debugger gets the crash).
	/// </summary>
	void Install();
}

@echo off
rem Builds the engine and the game in Ship mode (no editor, no logs, optimized) and makes a folder that runs on any Windows PC,
rem nothing to install: <Settings > Ship > packageFolder, by default Bin\windows-x64\Ship\Package>\<gameName>
rem (the launcher <gameName>.exe, Binaries\ = the game and its DLLs, Content\ = the pak). See CMake\Package.cmake.
rem The same thing is done by File > Package Game in the editor (which also lets you pick the folder).
setlocal
cd /d "%~dp0"

cmake --preset windows-vs2026
if errorlevel 1 goto failed

cmake --build Build/vs2026 --config Ship --target Package -- /nologo /verbosity:minimal
if errorlevel 1 goto failed

echo.
echo Package ready: see the "Package ready" line above.
echo Copy the whole folder of the game to another PC and run the exe at its root.
exit /b 0

:failed
echo.
echo The Ship build failed.
exit /b 1

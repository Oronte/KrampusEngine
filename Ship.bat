@echo off
rem Builds the engine and the game in Ship mode (no editor, no logs, optimized) and makes a folder that runs on any Windows PC,
rem nothing to install: Bin\windows-x64\Ship\Package (the exe, the pak with the Content, the runtime DLLs).
rem The same thing is done by File > Package Game in the editor.
setlocal
cd /d "%~dp0"

cmake --preset windows-vs2026
if errorlevel 1 goto failed

cmake --build Build/vs2026 --config Ship --target Package -- /nologo /verbosity:minimal
if errorlevel 1 goto failed

echo.
echo Package ready: %~dp0Bin\windows-x64\Ship\Package
echo Copy this whole folder to another PC and run KrampusGame.exe.
exit /b 0

:failed
echo.
echo The Ship build failed.
exit /b 1

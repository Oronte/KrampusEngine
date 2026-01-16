@echo off
setlocal enabledelayedexpansion

echo ==================================================
echo KrampusEngine Build And Export Process Started
echo ==================================================
echo.

echo STEP 1 OF 7 - Destination folder selection started
echo A folder selection window will open
echo Please choose the final export destination
echo.

for /f "usebackq delims=" %%i in (`powershell -NoProfile -Command "Add-Type -AssemblyName System.Windows.Forms; $f = New-Object System.Windows.Forms.FolderBrowserDialog; if($f.ShowDialog() -eq 'OK') {Write-Output $f.SelectedPath}"`) do set "DEST=%%i"

if not defined DEST (
    echo ERROR - No destination folder was selected
    echo The process cannot continue
    pause
    exit /b 1
)

echo Destination folder selected successfully
echo Destination path is:
echo %DEST%
echo.

echo STEP 2 OF 7 - Searching for solution file
echo Looking for a SLN file in the project root directory
echo.

set "SOLUTION_FILE="
for %%f in (*.sln) do (
    set "SOLUTION_FILE=%%f"
    goto found_sln
)

:found_sln
if not defined SOLUTION_FILE (
    echo ERROR - No SLN file was found in the project root
    echo Make sure the script is executed from the correct directory
    pause
    exit /b 1
)

echo Solution file found
echo Solution file name is:
echo %SOLUTION_FILE%
echo.

for %%f in ("%SOLUTION_FILE%") do set "PROJECT_NAME=%%~nf"

echo Project name extracted from solution file
echo Project name is:
echo %PROJECT_NAME%
echo.

echo STEP 3 OF 7 - Locating MSBuild
echo Searching for Visual Studio installation using vswhere
echo.

set "MSBUILD_PATH="
for /f "usebackq delims=" %%i in (`powershell -NoProfile -Command "Write-Output ((& '%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe' -latest -products * -requires Microsoft.Component.MSBuild -property installationPath) + '\MSBuild\Current\Bin\MSBuild.exe')"`) do set "MSBUILD_PATH=%%i"

if not exist "%MSBUILD_PATH%" (
    echo ERROR - MSBuild executable was not found
    echo Verify that Visual Studio with MSBuild is installed
    pause
    exit /b 1
)

echo MSBuild found successfully
echo MSBuild path is:
echo %MSBUILD_PATH%
echo.

echo STEP 4 OF 7 - Building the project
echo Build configuration is Ship
echo Build platform is x64
echo Build process is starting now
echo.

"%MSBUILD_PATH%" "%SOLUTION_FILE%" /p:Configuration=Ship /p:Platform=x64

if errorlevel 1 (
    echo ERROR - Build process failed
    echo Check build output for details
    pause
    exit /b 1
)

echo Build completed successfully
echo.

echo STEP 5 OF 7 - Creating output directories
echo Preparing final export folder structure
echo.

set "FINAL_DIR=%DEST%\%PROJECT_NAME%"
set "BIN_DIR=%FINAL_DIR%\Bin"
set "CONTENT_DIR=%FINAL_DIR%\Content"

mkdir "%FINAL_DIR%" 2>nul
mkdir "%BIN_DIR%" 2>nul
mkdir "%CONTENT_DIR%\Audio" 2>nul
mkdir "%CONTENT_DIR%\Fonts" 2>nul
mkdir "%CONTENT_DIR%\Textures" 2>nul

echo Directory structure created
echo Final directory is:
echo %FINAL_DIR%
echo.

echo STEP 6 OF 7 - Copying build output files
echo Locating compiled binaries
echo.

set "SOURCE_BIN=Bin\windows-x86_64\Ship"

if not exist "%SOURCE_BIN%" (
    echo ERROR - Compiled binary directory was not found
    echo Expected directory:
    echo %SOURCE_BIN%
    pause
    exit /b 1
)

echo Binary directory found
echo Copying binaries to final directory
echo.

xcopy "%SOURCE_BIN%\*" "%BIN_DIR%\" /E /I /Y

if errorlevel 1 (
    echo ERROR - Failed to copy binary files
    pause
    exit /b 1
)

echo Binary files copied successfully
echo.

echo Copying content files if available
echo.

if exist "Content\Audio" (
    echo Copying audio files
    xcopy "Content\Audio\*" "%CONTENT_DIR%\Audio\" /E /I /Y
)

if exist "Content\Fonts" (
    echo Copying font files
    xcopy "Content\Fonts\*" "%CONTENT_DIR%\Fonts\" /E /I /Y
)

if exist "Content\Textures" (
    echo Copying texture files
    xcopy "Content\Textures\*" "%CONTENT_DIR%\Textures\" /E /I /Y
)

echo Content copy process completed
echo.

set "TARGET_EXE=%BIN_DIR%\Game\Game.exe"
set "SHORTCUT_PATH=%FINAL_DIR%\Game.lnk"

echo Attempting to create desktop shortcut
echo.

if exist "%TARGET_EXE%" (
    powershell -NoProfile -Command ^
    "$W = New-Object -ComObject WScript.Shell; " ^
    "$S = $W.CreateShortcut('%SHORTCUT_PATH%'); " ^
    "$S.TargetPath = '%TARGET_EXE%'; " ^
    "$S.WorkingDirectory = '%BIN_DIR%\Game'; " ^
    "$S.Save()"
    echo Shortcut created successfully
    echo Shortcut path:
    echo %SHORTCUT_PATH%
) else (
    echo ERROR - Executable not found
    echo Expected executable path:
    echo %TARGET_EXE%
)

echo.

echo STEP 7 OF 7 - Process completed
echo Build and export finished successfully
echo Final output directory:
echo %FINAL_DIR%
echo.

pause
exit /b 0
@echo off
:: ===========================================
:: KrampusEngine - Build & Export Script
:: ===========================================

:: --- Force l'affichage des erreurs ---
setlocal enabledelayedexpansion

:: --- Section 1 : Sélection du dossier de destination ---
echo [1/7] Sélection du dossier de destination...
:: Utilisation de PowerShell pour ouvrir une fenêtre FolderBrowserDialog
for /f "usebackq delims=" %%i in (`powershell -NoProfile -Command "Add-Type -AssemblyName System.Windows.Forms; $f = New-Object System.Windows.Forms.FolderBrowserDialog; if($f.ShowDialog() -eq 'OK') {Write-Output $f.SelectedPath}"`) do set "DEST=%%i"

if not defined DEST (
    echo ERREUR : Aucun dossier de destination sélectionné.
    pause
    exit /b 1
)
echo Dossier sélectionné : "%DEST%"

:: --- Section 2 : Détection du fichier solution (.sln) ---
echo [2/7] Recherche du fichier solution...
set "SOLUTION_FILE="
for %%f in (*.sln) do (
    set "SOLUTION_FILE=%%f"
    goto :found_sln
)
:found_sln
if not defined SOLUTION_FILE (
    echo ERREUR : Aucun fichier .sln trouvé à la racine du projet.
    pause
    exit /b 1
)
echo Fichier solution trouvé : "%SOLUTION_FILE%"

:: --- Extraire le nom du projet à partir du fichier solution ---
for %%f in ("%SOLUTION_FILE%") do set "PROJECT_NAME=%%~nf"
echo Nom du projet : "%PROJECT_NAME%"

:: --- Section 3 : Trouver MSBuild via vswhere.exe ---
echo [3/7] Recherche de MSBuild...
set "MSBUILD_PATH="
for /f "usebackq delims=" %%i in (`powershell -NoProfile -Command "Write-Output ((& '%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe' -latest -products * -requires Microsoft.Component.MSBuild -property installationPath) + '\MSBuild\Current\Bin\MSBuild.exe')"`) do set "MSBUILD_PATH=%%i"

if not exist "%MSBUILD_PATH%" (
    echo ERREUR : MSBuild introuvable.
    pause
    exit /b 1
)
echo MSBuild trouvé : "%MSBUILD_PATH%"

:: --- Section 4 : Compilation du projet en Release x64 ---
echo [4/7] Compilation du projet en Release x64...
"%MSBUILD_PATH%" "%SOLUTION_FILE%" /p:Configuration=Ship /p:Platform=x64
if errorlevel 1 (
    echo ERREUR : La compilation a échoué.
    pause
    exit /b 1
)
echo Compilation réussie.

:: --- Section 5 : Création du dossier final ---
echo [5/7] Création des dossiers de destination...
set "FINAL_DIR=%DEST%\%PROJECT_NAME%"
set "BIN_DIR=%FINAL_DIR%\Bin"
set "CONTENT_DIR=%FINAL_DIR%\Content"
mkdir "%FINAL_DIR%" 2>nul
mkdir "%BIN_DIR%" 2>nul
mkdir "%CONTENT_DIR%\Audio" 2>nul
mkdir "%CONTENT_DIR%\Fonts" 2>nul
mkdir "%CONTENT_DIR%\Textures" 2>nul

:: --- Section 6 : Copie des fichiers ---
echo [6/7] Copie des fichiers compilés...
set "SOURCE_BIN=Bin\windows-x86_64\Ship"
if not exist "%SOURCE_BIN%" (
    echo ERREUR : Dossier binaire introuvable : "%SOURCE_BIN%"
    pause
    exit /b 1
)
xcopy "%SOURCE_BIN%\*" "%BIN_DIR%\" /E /I /Y
if errorlevel 1 (
    echo ERREUR : La copie des fichiers binaires a échoué.
    pause
    exit /b 1
)
echo Fichiers Bin copiés.

echo [6/7] Copie du contenu...
if exist "Content\Audio" xcopy "Content\Audio\*" "%CONTENT_DIR%\Audio\" /E /I /Y
if exist "Content\Fonts" xcopy "Content\Fonts\*" "%CONTENT_DIR%\Fonts\" /E /I /Y
if exist "Content\Textures" xcopy "Content\Textures\*" "%CONTENT_DIR%\Textures\" /E /I /Y

set "TARGET_EXE=%BIN_DIR%\Game\Game.exe"
set "SHORTCUT_PATH=%FINAL_DIR%\Game.lnk"

if exist "%TARGET_EXE%" (
    powershell -NoProfile -Command ^
    "$W = New-Object -ComObject WScript.Shell; " ^
    "$S = $W.CreateShortcut('%SHORTCUT_PATH%'); " ^
    "$S.TargetPath = '%TARGET_EXE%'; " ^
    "$S.WorkingDirectory = '%BIN_DIR%\Game'; " ^
    "$S.Save()"
    echo Raccourci créé : "%SHORTCUT_PATH%"
) else (
    echo ERREUR : Executable introuvable pour créer le raccourci : "%TARGET_EXE%"
)
echo Contenu copié.

:: --- Section 7 : Fin et pause ---
echo [7/7] Opération terminée avec succès.
echo Dossier final : "%FINAL_DIR%"
pause
exit /b 0

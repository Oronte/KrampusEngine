# Package.cmake: makes the game folder that can be copied to another PC (called by the Package target, see Ship/CMakeLists.txt)
#   -D CONFIG=<Debug|Release|Ship>
#   -D SOURCE_DIR=<project root>
#   -D EXE=<KrampusGame.exe>
#   -D PACK_TOOL=<PackTool.exe>
#   -D LAUNCHER=<KrampusLauncher.exe>
#   -D VC_ROOT=<Visual Studio's VC folder>
#   -D DEFAULT_ROOT=<.../Package>         where the folder goes when Settings > Ship > packageFolder is empty
#
# Content/Settings/EngineSettings.cfg, section [ShipSettings], says the rest: gameName (names the folder and the exes), icon
# (a picture of Content/Textures) and packageFolder (where to write). The result, like the packaged build of an Unreal game, is a
# folder that runs on a PC where nothing is installed:
#
#   <packageFolder>/<Game>/
#       <Game>.exe                  the launcher (with the icon): starts Binaries/<Game>.exe
#       Binaries/<Game>.exe         the game, with SFML linked statically (Ship), and the icon too
#       Binaries/*.dll              the Visual C++ and Universal C runtimes ("app-local deployment": the PC needs no Visual C++
#                                   Redistributable), steam_api64.dll
#       Content/<Game>.pak          every file of Content/ in one file (see AssetPack.h); the editor files, scripts, logs,
#                                   screenshots and the developer's saves are left out
#       BuildInfo.txt
#
# The game writes its saves in <Game>/Content/Saves.

foreach(VARIABLE CONFIG SOURCE_DIR EXE PACK_TOOL LAUNCHER VC_ROOT DEFAULT_ROOT)
    if(NOT ${VARIABLE})
        message(FATAL_ERROR "Package: ${VARIABLE} is not defined")
    endif()
endforeach()

foreach(REQUIRED ${EXE} ${PACK_TOOL} ${LAUNCHER})
    if(NOT EXISTS "${REQUIRED}")
        message(FATAL_ERROR "Package: ${REQUIRED} does not exist, build it first")
    endif()
endforeach()

if(NOT CONFIG STREQUAL "Ship")
    message(WARNING "Package: this is a ${CONFIG} build: it still needs the SFML DLLs next to it. Use the Ship configuration for a game that runs alone.")
endif()

# ---- Settings > Ship (the text file of the settings: "key = value" lines under [ShipSettings])
set(GAME_NAME "")
set(ICON "")
set(PACKAGE_FOLDER "")
set(SETTINGS_FILE "${SOURCE_DIR}/Content/Settings/EngineSettings.cfg")
if(EXISTS "${SETTINGS_FILE}")
    file(STRINGS "${SETTINGS_FILE}" SETTINGS_LINES)
    set(IN_SHIP FALSE)
    foreach(LINE IN LISTS SETTINGS_LINES)
        string(STRIP "${LINE}" LINE)
        if(LINE MATCHES "^\\[(.*)\\]$")
            if(CMAKE_MATCH_1 STREQUAL "ShipSettings")
                set(IN_SHIP TRUE)
            else()
                set(IN_SHIP FALSE)
            endif()
        elseif(IN_SHIP AND LINE MATCHES "^([A-Za-z]+)[ \t]*=[ \t]*(.*)$")
            set(KEY "${CMAKE_MATCH_1}")
            string(STRIP "${CMAKE_MATCH_2}" VALUE)
            if(KEY STREQUAL "gameName")
                set(GAME_NAME "${VALUE}")
            elseif(KEY STREQUAL "icon")
                set(ICON "${VALUE}")
            elseif(KEY STREQUAL "packageFolder")
                set(PACKAGE_FOLDER "${VALUE}")
            endif()
        endif()
    endforeach()
endif()

# The name of the game becomes a file name: no \ / : * ? " < > |
string(REGEX REPLACE "[\\\\/:*?\"<>|]" "" GAME_NAME "${GAME_NAME}")
string(STRIP "${GAME_NAME}" GAME_NAME)
string(REGEX REPLACE "[. ]+$" "" GAME_NAME "${GAME_NAME}")
if(GAME_NAME STREQUAL "")
    set(GAME_NAME "KrampusGame")
endif()

if(PACKAGE_FOLDER STREQUAL "")
    set(OUT_ROOT "${DEFAULT_ROOT}")
else()
    file(TO_CMAKE_PATH "${PACKAGE_FOLDER}" OUT_ROOT)
    if(NOT IS_ABSOLUTE "${OUT_ROOT}")
        set(OUT_ROOT "${SOURCE_DIR}/${OUT_ROOT}")
    endif()
endif()
get_filename_component(OUT_ROOT "${OUT_ROOT}" ABSOLUTE)
set(OUT_DIR "${OUT_ROOT}/${GAME_NAME}")

# ---- Checks on Content
file(GLOB SCENES "${SOURCE_DIR}/Content/Scenes/*.ka7")
list(LENGTH SCENES SCENE_COUNT)
if(SCENE_COUNT EQUAL 0)
    message(WARNING "Package: there is no scene (*.ka7) in Content/Scenes: the game will start on an empty level.")
endif()

if(NOT EXISTS "${SOURCE_DIR}/Content/Textures/_internal/KrampusEngineBanner.png")
    message(WARNING "Package: Content/Textures/_internal/KrampusEngineBanner.png is missing (the splash screen needs it).")
endif()

set(ICON_FILE "")
if(ICON STREQUAL "")
    set(ICON "_internal/KrampusLogo.png")    # no icon chosen: the logo of the engine (ShipSettings::DEFAULT_ICON)
endif()
if(NOT ICON STREQUAL "")
    set(ICON_FILE "${SOURCE_DIR}/Content/Textures/${ICON}")
    if(NOT EXISTS "${ICON_FILE}")
        message(WARNING "Package: the icon Content/Textures/${ICON} (Settings > Ship > icon) does not exist: the exe keeps the Windows icon.")
        set(ICON_FILE "")
    endif()
endif()

# ---- The folder is made again every time, so nothing old stays in it. Only a folder made by this script (it holds its
# BuildInfo.txt) or an empty one is ever emptied: a wrong packageFolder never wipes the files of somebody.
function(is_package_folder FOLDER RESULT)
    set(${RESULT} FALSE PARENT_SCOPE)
    if(EXISTS "${FOLDER}/BuildInfo.txt")
        file(READ "${FOLDER}/BuildInfo.txt" INFO LIMIT 64)
        if(INFO MATCHES "^KrampusEngine game package")
            set(${RESULT} TRUE PARENT_SCOPE)
        endif()
    endif()
endfunction()

if(EXISTS "${OUT_DIR}")
    file(GLOB OLD_FILES "${OUT_DIR}/*")
    is_package_folder("${OUT_DIR}" IS_PACKAGE)
    if(OLD_FILES AND NOT IS_PACKAGE)
        message(FATAL_ERROR "Package: '${OUT_DIR}' already exists and is not a game package (no BuildInfo.txt): choose another folder, or empty it.")
    endif()
    file(REMOVE_RECURSE "${OUT_DIR}")
endif()
# An older package wrote its files straight in the default folder: they go away.
is_package_folder("${DEFAULT_ROOT}" OLD_LAYOUT)
if(OLD_LAYOUT)
    file(GLOB OLD_FILES LIST_DIRECTORIES false "${DEFAULT_ROOT}/*")
    file(REMOVE ${OLD_FILES})
    message(STATUS "Package: removed the files of an older package from ${DEFAULT_ROOT}")
endif()

file(MAKE_DIRECTORY "${OUT_DIR}/Binaries" "${OUT_DIR}/Content")

# ---- The launcher at the root, the game in Binaries, the Content in a pak
file(COPY_FILE "${LAUNCHER}" "${OUT_DIR}/${GAME_NAME}.exe")
file(COPY_FILE "${EXE}" "${OUT_DIR}/Binaries/${GAME_NAME}.exe")
message(STATUS "Package: ${GAME_NAME}.exe (launcher) and Binaries/${GAME_NAME}.exe")

if(ICON_FILE)
    execute_process(
        COMMAND "${PACK_TOOL}" --icon "${ICON_FILE}" "${OUT_DIR}/${GAME_NAME}.exe" "${OUT_DIR}/Binaries/${GAME_NAME}.exe"
        RESULT_VARIABLE ICON_RESULT
        OUTPUT_VARIABLE ICON_OUTPUT
        ERROR_VARIABLE ICON_ERROR
    )
    if(NOT ICON_RESULT EQUAL 0)
        message(WARNING "Package: the icon could not be set: ${ICON_ERROR}")
    else()
        string(STRIP "${ICON_OUTPUT}" ICON_OUTPUT)
        message(STATUS "${ICON_OUTPUT}")
    endif()
endif()

# Steam (only when the SDK is in ThirdParty): the DLL goes next to the game. steam_appid.txt is NOT shipped: Steam gives the game its AppID.
set(STEAM_DLL "${SOURCE_DIR}/ThirdParty/Steamworks/sdk/redistributable_bin/win64/steam_api64.dll")
if(EXISTS "${STEAM_DLL}")
    file(COPY_FILE "${STEAM_DLL}" "${OUT_DIR}/Binaries/steam_api64.dll")
    message(STATUS "Package: steam_api64.dll")
endif()

execute_process(
    COMMAND "${PACK_TOOL}" "${SOURCE_DIR}" "${OUT_DIR}/Content/${GAME_NAME}.pak"
    RESULT_VARIABLE PACK_RESULT
    OUTPUT_VARIABLE PACK_OUTPUT
    ERROR_VARIABLE PACK_ERROR
)
if(NOT PACK_RESULT EQUAL 0)
    message(FATAL_ERROR "Package: ${PACK_ERROR}")
endif()
string(STRIP "${PACK_OUTPUT}" PACK_OUTPUT)
message(STATUS "${PACK_OUTPUT}")

# ---- The Visual C++ runtime, next to the game. Windows looks in the folder of the exe first, so the game uses these copies
# and does not care what is (or is not) installed on the PC.
file(GLOB CRT_FOLDERS "${VC_ROOT}/Redist/MSVC/*/x64/Microsoft.VC*.CRT")
list(SORT CRT_FOLDERS COMPARE NATURAL ORDER DESCENDING)
if(CRT_FOLDERS)
    list(GET CRT_FOLDERS 0 CRT_FOLDER)
    file(GLOB CRT_DLLS "${CRT_FOLDER}/*.dll")
    foreach(DLL ${CRT_DLLS})
        file(COPY "${DLL}" DESTINATION "${OUT_DIR}/Binaries")
    endforeach()
    list(LENGTH CRT_DLLS CRT_COUNT)
    message(STATUS "Package: ${CRT_COUNT} Visual C++ runtime DLLs (from ${CRT_FOLDER})")
else()
    message(WARNING "Package: the Visual C++ runtime was not found in ${VC_ROOT}/Redist: the PC that runs the game must have the Visual C++ Redistributable installed.")
endif()

# ---- The Universal C runtime: part of Windows 10 and 11, but not of older Windows.
get_filename_component(KITS10 "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" ABSOLUTE)
set(UCRT_CANDIDATES "${KITS10}/Redist/ucrt/DLLs/x64" "C:/Program Files (x86)/Windows Kits/10/Redist/ucrt/DLLs/x64")
foreach(UCRT_FOLDER ${UCRT_CANDIDATES})
    if(EXISTS "${UCRT_FOLDER}/ucrtbase.dll")
        file(GLOB UCRT_DLLS "${UCRT_FOLDER}/*.dll")
        foreach(DLL ${UCRT_DLLS})
            file(COPY "${DLL}" DESTINATION "${OUT_DIR}/Binaries")
        endforeach()
        list(LENGTH UCRT_DLLS UCRT_COUNT)
        message(STATUS "Package: ${UCRT_COUNT} Universal C runtime DLLs")
        break()
    endif()
endforeach()

# ---- Summary
file(GLOB_RECURSE PACKAGED_FILES "${OUT_DIR}/*")
set(TOTAL_BYTES 0)
set(FILE_COUNT 0)
foreach(PACKAGED_FILE ${PACKAGED_FILES})
    file(SIZE "${PACKAGED_FILE}" FILE_BYTES)
    math(EXPR TOTAL_BYTES "${TOTAL_BYTES} + ${FILE_BYTES}")
    math(EXPR FILE_COUNT "${FILE_COUNT} + 1")
endforeach()
math(EXPR TOTAL_KB "${TOTAL_BYTES} / 1024")

string(TIMESTAMP BUILD_TIME "%Y-%m-%d %H:%M:%S")
file(WRITE "${OUT_DIR}/BuildInfo.txt"
    "KrampusEngine game package\nGame: ${GAME_NAME}\nConfiguration: ${CONFIG}\nBuilt: ${BUILD_TIME}\nScenes: ${SCENE_COUNT}\n\n"
    "Copy this whole folder to another PC and run ${GAME_NAME}.exe: nothing needs to be installed.\n")

message(STATUS "Package: ${SCENE_COUNT} scenes, ${FILE_COUNT} files, ${TOTAL_KB} KB in the folder")
message(STATUS "Package ready: ${OUT_DIR}")

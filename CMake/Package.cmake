# Package.cmake: makes the game folder that can be copied to another PC (called by the Package target, see Ship/CMakeLists.txt)
#   -D CONFIG=<Debug|Release|Ship>
#   -D SOURCE_DIR=<project root>
#   -D EXE=<KrampusGame.exe>
#   -D PACK_TOOL=<PackTool.exe>
#   -D VC_ROOT=<Visual Studio's VC folder>
#   -D OUT_DIR=<.../Package>
#
# The result, like the packaged build of an Unreal game, is a folder that runs on a PC where nothing is installed:
#
#   Package/KrampusGame.exe        the game, with SFML linked statically (Ship)
#   Package/KrampusGame.pak        every file of Content/ in one file (see AssetPack.h); the editor files, scripts, logs,
#                                  screenshots and the developer's saves are left out
#   Package/*.dll                  the Visual C++ runtime and the Universal C runtime, next to the exe ("app-local deployment"),
#                                  so that the PC does not need the Visual C++ Redistributable
#
# The game writes its saves next to itself (Package/Content/Saves).

foreach(VARIABLE CONFIG SOURCE_DIR EXE PACK_TOOL VC_ROOT OUT_DIR)
    if(NOT ${VARIABLE})
        message(FATAL_ERROR "Package: ${VARIABLE} is not defined")
    endif()
endforeach()

foreach(REQUIRED ${EXE} ${PACK_TOOL})
    if(NOT EXISTS "${REQUIRED}")
        message(FATAL_ERROR "Package: ${REQUIRED} does not exist, build it first")
    endif()
endforeach()

if(NOT CONFIG STREQUAL "Ship")
    message(WARNING "Package: this is a ${CONFIG} build: it still needs the SFML DLLs next to it. Use the Ship configuration for a game that runs alone.")
endif()

# ---- Checks on Content
file(GLOB SCENES "${SOURCE_DIR}/Content/Scenes/*.ka7")
list(LENGTH SCENES SCENE_COUNT)
if(SCENE_COUNT EQUAL 0)
    message(WARNING "Package: there is no scene (*.ka7) in Content/Scenes: the game will start on an empty level.")
endif()

if(NOT EXISTS "${SOURCE_DIR}/Content/Textures/_internal/KrampusEngineBanner.png")
    message(WARNING "Package: Content/Textures/_internal/KrampusEngineBanner.png is missing (the splash screen needs it).")
endif()

# ---- The folder is made again every time, so nothing old stays in it. Only a folder named Package is ever emptied.
get_filename_component(OUT_NAME "${OUT_DIR}" NAME)
if(NOT OUT_NAME STREQUAL "Package")
    message(FATAL_ERROR "Package: refusing to empty '${OUT_DIR}' (it is not a folder named Package)")
endif()
file(REMOVE_RECURSE "${OUT_DIR}")
file(MAKE_DIRECTORY "${OUT_DIR}")

# ---- The game, and its Content in a pak next to it
file(COPY_FILE "${EXE}" "${OUT_DIR}/KrampusGame.exe")
message(STATUS "Package: KrampusGame.exe")

execute_process(
    COMMAND "${PACK_TOOL}" "${SOURCE_DIR}" "${OUT_DIR}/KrampusGame.pak"
    RESULT_VARIABLE PACK_RESULT
    OUTPUT_VARIABLE PACK_OUTPUT
    ERROR_VARIABLE PACK_ERROR
)
if(NOT PACK_RESULT EQUAL 0)
    message(FATAL_ERROR "Package: ${PACK_ERROR}")
endif()
string(STRIP "${PACK_OUTPUT}" PACK_OUTPUT)
message(STATUS "${PACK_OUTPUT}")

# ---- The Visual C++ runtime, next to the exe. Windows looks in the folder of the exe first, so the game uses these copies
# and does not care what is (or is not) installed on the PC.
file(GLOB CRT_FOLDERS "${VC_ROOT}/Redist/MSVC/*/x64/Microsoft.VC*.CRT")
list(SORT CRT_FOLDERS COMPARE NATURAL ORDER DESCENDING)
if(CRT_FOLDERS)
    list(GET CRT_FOLDERS 0 CRT_FOLDER)
    file(GLOB CRT_DLLS "${CRT_FOLDER}/*.dll")
    foreach(DLL ${CRT_DLLS})
        file(COPY "${DLL}" DESTINATION "${OUT_DIR}")
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
            file(COPY "${DLL}" DESTINATION "${OUT_DIR}")
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
    "KrampusEngine game package\nConfiguration: ${CONFIG}\nBuilt: ${BUILD_TIME}\nScenes: ${SCENE_COUNT}\n\n"
    "Copy this whole folder to another PC and run KrampusGame.exe: nothing needs to be installed.\n")

message(STATUS "Package: ${SCENE_COUNT} scenes, ${FILE_COUNT} files, ${TOTAL_KB} KB in the folder")
message(STATUS "Package ready: ${OUT_DIR}")

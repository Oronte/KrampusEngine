cmake_minimum_required(VERSION 3.20)
project(Ship LANGUAGES CXX)

# KrampusGame: the engine and the game, without the editor.
#
# Ship configuration, like the packaged build of an Unreal game: the Package target makes a FOLDER that can be copied to any
# Windows PC and run, nothing to install:
#
#       cmake --build Build/vs2026 --config Ship --target Package        (or Ship.bat, or File > Package Game in the editor)
#       -> Bin/windows-x64/Ship/Package/
#              KrampusGame.exe     the game (SFML is linked statically: no sfml-*.dll)
#              KrampusGame.pak     everything of Content/ in one file (see Engine/Source/Core/AssetPack.h)
#              *.dll               the Visual C++ and Universal C runtimes, so that the PC needs no Redistributable
#
# Debug and Release: the dynamic libraries and the files of Content on the disk, to run the game from Visual Studio.
set(SHIP_BIN_DIR "${CMAKE_SOURCE_DIR}/Bin/${KRAMPUS_PLATFORM}-${KRAMPUS_ARCH}/$<CONFIG>/Game")

add_executable(KrampusGame Source/Main.cpp $<TARGET_OBJECTS:Game>)

target_include_directories(KrampusGame PRIVATE
    ${CMAKE_SOURCE_DIR}/ThirdParty/SFML-3.0.0/include
    ${CMAKE_SOURCE_DIR}/ThirdParty/box2d-3.1.1/include
)

target_link_directories(KrampusGame PRIVATE
    ${CMAKE_SOURCE_DIR}/ThirdParty/SFML-3.0.0/lib
    ${CMAKE_SOURCE_DIR}/ThirdParty/box2d-3.1.1/lib
)

target_link_libraries(KrampusGame PRIVATE
    Engine
    Game

    # SFML Debug libraries (dynamic)
    $<$<CONFIG:Debug>:sfml-graphics-d>
    $<$<CONFIG:Debug>:sfml-window-d>
    $<$<CONFIG:Debug>:sfml-system-d>
    $<$<CONFIG:Debug>:sfml-audio-d>
    $<$<CONFIG:Debug>:sfml-network-d>

    # SFML Release libraries (dynamic)
    $<$<CONFIG:Release>:sfml-graphics>
    $<$<CONFIG:Release>:sfml-window>
    $<$<CONFIG:Release>:sfml-system>
    $<$<CONFIG:Release>:sfml-audio>
    $<$<CONFIG:Release>:sfml-network>

    # SFML Ship libraries (static, with what they are made of)
    $<$<CONFIG:Ship>:sfml-graphics-s>
    $<$<CONFIG:Ship>:sfml-window-s>
    $<$<CONFIG:Ship>:sfml-audio-s>
    $<$<CONFIG:Ship>:sfml-network-s>
    $<$<CONFIG:Ship>:sfml-system-s>
    $<$<CONFIG:Ship>:freetype>
    $<$<CONFIG:Ship>:vorbisfile>
    $<$<CONFIG:Ship>:vorbisenc>
    $<$<CONFIG:Ship>:vorbis>
    $<$<CONFIG:Ship>:ogg>
    $<$<CONFIG:Ship>:FLAC>
    $<$<CONFIG:Ship>:winmm>
    $<$<CONFIG:Ship>:gdi32>
    $<$<CONFIG:Ship>:ws2_32>
    $<$<CONFIG:Ship>:ole32>
    $<$<CONFIG:Ship>:uuid>
    $<$<CONFIG:Ship>:advapi32>
    $<$<CONFIG:Ship>:user32>

    # box2d
    $<$<CONFIG:Debug>:box2dd>
    $<$<CONFIG:Release>:box2d>
    $<$<CONFIG:Ship>:box2d>

    opengl32
)

target_compile_definitions(KrampusGame PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:RELEASE>
    $<$<CONFIG:Ship>:RELEASE>
    $<$<CONFIG:Ship>:SHIP>
)

if(WIN32)
    target_compile_definitions(KrampusGame PRIVATE WINDOWS)
endif()

# A shipped game has no console window (main() stays the entry point).
target_link_options(KrampusGame PRIVATE
    $<$<CONFIG:Ship>:/SUBSYSTEM:WINDOWS>
    $<$<CONFIG:Ship>:/ENTRY:mainCRTStartup>
)

set_target_properties(KrampusGame PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SHIP_BIN_DIR}"
    PDB_OUTPUT_DIRECTORY "${SHIP_BIN_DIR}"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# ---------------------------------------------------------------------------------------------------------------
# PackTool: puts the files of Content in a pak (what the game mounts at start, see AssetPack.h).
# ---------------------------------------------------------------------------------------------------------------
add_executable(PackTool Tools/PackTool.cpp ${CMAKE_SOURCE_DIR}/Engine/Source/Core/AssetPack.cpp)

target_include_directories(PackTool PRIVATE ${CMAKE_SOURCE_DIR}/Engine/Source)

set_target_properties(PackTool PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Bin/${KRAMPUS_PLATFORM}-${KRAMPUS_ARCH}/$<CONFIG>/Tools"
    PDB_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Bin/${KRAMPUS_PLATFORM}-${KRAMPUS_ARCH}/$<CONFIG>/Tools"
)

# ---------------------------------------------------------------------------------------------------------------
# Package: the folder to copy to another PC, in Bin/<platform>/Ship/Package.
# ---------------------------------------------------------------------------------------------------------------
set(PACKAGE_DIR "${CMAKE_SOURCE_DIR}/Bin/${KRAMPUS_PLATFORM}-${KRAMPUS_ARCH}/$<CONFIG>/Package")

# Visual Studio's VC folder (the compiler is VC/Tools/MSVC/<version>/bin/Hostx64/x64/cl.exe): its Redist folder holds the C++
# runtime DLLs that the package carries.
get_filename_component(VC_ROOT "${CMAKE_CXX_COMPILER}/../../../../../../.." ABSOLUTE)

add_custom_target(Package
    COMMAND "${CMAKE_COMMAND}"
        -D "CONFIG=$<CONFIG>"
        -D "SOURCE_DIR=${CMAKE_SOURCE_DIR}"
        -D "EXE=$<TARGET_FILE:KrampusGame>"
        -D "PACK_TOOL=$<TARGET_FILE:PackTool>"
        -D "VC_ROOT=${VC_ROOT}"
        -D "OUT_DIR=${PACKAGE_DIR}"
        -P "${CMAKE_SOURCE_DIR}/CMake/Package.cmake"
    DEPENDS KrampusGame PackTool
    COMMENT "Packaging the game ($<CONFIG>)..."
    VERBATIM
)

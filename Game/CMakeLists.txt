cmake_minimum_required(VERSION 3.20)
project(Game LANGUAGES CXX)

file(GLOB_RECURSE GAME_HEADERS CONFIGURE_DEPENDS "Source/*.h")
file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS "Source/*.cpp")

# Game scripts live in Content/Scripts (shown in the editor's Content panel). A script added there is picked up
# by the next build; the editor then hot reloads its module.
set(SCRIPTS_DIR "${CMAKE_SOURCE_DIR}/Content/Scripts")
file(GLOB_RECURSE GAME_SCRIPT_HEADERS CONFIGURE_DEPENDS "${SCRIPTS_DIR}/*.h")
file(GLOB_RECURSE GAME_SCRIPT_SOURCES CONFIGURE_DEPENDS "${SCRIPTS_DIR}/*.cpp")
source_group(TREE "${SCRIPTS_DIR}" PREFIX "Scripts" FILES ${GAME_SCRIPT_HEADERS} ${GAME_SCRIPT_SOURCES})

# The game knows the engine only. It has no idea of the editor.
# An OBJECT library: its objects are linked as they are, so the actors registered with KACTOR are never dropped by the linker.
# The program that runs the game (Editor/EditorModule, or a shipping executable) takes these objects with $<TARGET_OBJECTS:Game>.
add_library(Game OBJECT ${GAME_HEADERS} ${GAME_SOURCES} ${GAME_SCRIPT_HEADERS} ${GAME_SCRIPT_SOURCES})

target_include_directories(Game PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
    ${SCRIPTS_DIR}
)

target_link_libraries(Game PUBLIC Engine)

target_compile_definitions(Game PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:RELEASE>
    $<$<CONFIG:Ship>:RELEASE>
    $<$<CONFIG:Ship>:SHIP>
)

if(WIN32)
    target_compile_definitions(Game PRIVATE WINDOWS)
endif()

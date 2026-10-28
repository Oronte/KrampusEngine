cmake_minimum_required(VERSION 3.20)
project(Editor LANGUAGES CXX)

set(BIN_DIR "${CMAKE_SOURCE_DIR}/Bin/${KRAMPUS_PLATFORM}-${KRAMPUS_ARCH}/$<CONFIG>/Editor")

# ---------------------------------------------------------------------------------------------------------------
# EditorModule.dll = Engine + Game + the editor tools (ImGui panels, play mode...).
# The Engine and the Game never include anything from this folder: the editor sits on top of them.
# ---------------------------------------------------------------------------------------------------------------
file(GLOB_RECURSE EDITOR_HEADERS CONFIGURE_DEPENDS "Source/Tools/*.h" "Source/Module/*.h")
file(GLOB_RECURSE EDITOR_SOURCES CONFIGURE_DEPENDS "Source/Tools/*.cpp" "Source/Module/*.cpp")

add_library(EditorModule SHARED ${EDITOR_HEADERS} ${EDITOR_SOURCES} $<TARGET_OBJECTS:Game>)

target_include_directories(EditorModule PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Tools
    ${CMAKE_SOURCE_DIR}/ThirdParty/SFML-3.0.0/include
    ${CMAKE_SOURCE_DIR}/ThirdParty/box2d-3.1.1/include
    ${CMAKE_SOURCE_DIR}/ThirdParty/ImGui/include
)

target_link_directories(EditorModule PRIVATE
    ${CMAKE_SOURCE_DIR}/ThirdParty/SFML-3.0.0/lib
    ${CMAKE_SOURCE_DIR}/ThirdParty/box2d-3.1.1/lib
    ${CMAKE_SOURCE_DIR}/ThirdParty/ImGui/lib
)

target_link_libraries(EditorModule PRIVATE
    Engine
    Game

    # SFML Debug libraries
    $<$<CONFIG:Debug>:sfml-graphics-d>
    $<$<CONFIG:Debug>:sfml-window-d>
    $<$<CONFIG:Debug>:sfml-system-d>
    $<$<CONFIG:Debug>:sfml-audio-d>
    $<$<CONFIG:Debug>:sfml-network-d>

    # SFML Release/Ship libraries
    $<$<OR:$<CONFIG:Release>,$<CONFIG:Ship>>:sfml-graphics>
    $<$<OR:$<CONFIG:Release>,$<CONFIG:Ship>>:sfml-window>
    $<$<OR:$<CONFIG:Release>,$<CONFIG:Ship>>:sfml-system>
    $<$<OR:$<CONFIG:Release>,$<CONFIG:Ship>>:sfml-audio>
    $<$<OR:$<CONFIG:Release>,$<CONFIG:Ship>>:sfml-network>

    # box2d
    $<$<CONFIG:Debug>:box2dd>
    $<$<CONFIG:Release>:box2d>

    # ImGui + ImGui-SFML
    $<$<CONFIG:Debug>:ImGuiD>
    $<$<OR:$<CONFIG:Release>,$<CONFIG:Ship>>:ImGui>

    opengl32
)

target_compile_definitions(EditorModule PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:RELEASE>
    $<$<CONFIG:Ship>:RELEASE>
    $<$<CONFIG:Ship>:SHIP>
)

if(WIN32)
    target_compile_definitions(EditorModule PRIVATE WINDOWS)
endif()

set_target_properties(EditorModule PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BIN_DIR}"
    PDB_OUTPUT_DIRECTORY "${BIN_DIR}"
)

# ---------------------------------------------------------------------------------------------------------------
# Editor.exe: thin host. It owns the window and the main loop, and loads EditorModule.dll.
# It does not link the engine, so the module can be rebuilt and reloaded while it runs.
# ---------------------------------------------------------------------------------------------------------------
add_executable(Editor Source/Main.cpp)

# Make sure a build of Editor also builds the module. Rebuilding only EditorModule triggers the hot reload.
add_dependencies(Editor EditorModule)

target_include_directories(Editor PRIVATE
    ${CMAKE_SOURCE_DIR}/ThirdParty/SFML-3.0.0/include
)

target_link_directories(Editor PRIVATE
    ${CMAKE_SOURCE_DIR}/ThirdParty/SFML-3.0.0/lib
)

target_link_libraries(Editor PRIVATE
    $<$<CONFIG:Debug>:sfml-graphics-d>
    $<$<CONFIG:Debug>:sfml-window-d>
    $<$<CONFIG:Debug>:sfml-system-d>
    $<$<OR:$<CONFIG:Release>,$<CONFIG:Ship>>:sfml-graphics>
    $<$<OR:$<CONFIG:Release>,$<CONFIG:Ship>>:sfml-window>
    $<$<OR:$<CONFIG:Release>,$<CONFIG:Ship>>:sfml-system>
)

set_target_properties(Editor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BIN_DIR}"
    PDB_OUTPUT_DIRECTORY "${BIN_DIR}"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

add_custom_command(TARGET Editor POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
        -D "CONFIG=$<CONFIG>"
        -D "SFML_BIN_DIR=${CMAKE_SOURCE_DIR}/ThirdParty/SFML-3.0.0/binaries"
        -D "OUT_DIR=${CMAKE_SOURCE_DIR}/Bin/${KRAMPUS_PLATFORM}-${KRAMPUS_ARCH}/$<CONFIG>/Editor"
        -P "${CMAKE_SOURCE_DIR}/CMake/CopyDLLs.cmake"
    COMMENT "Copying SFML DLLs ($<CONFIG>)..."
)

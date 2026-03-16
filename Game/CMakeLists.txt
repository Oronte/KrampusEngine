cmake_minimum_required(VERSION 3.20)
project(Game LANGUAGES CXX)

file(GLOB_RECURSE GAME_HEADERS CONFIGURE_DEPENDS "Source/*.h")
file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS "Source/*.cpp")

add_executable(Game ${GAME_HEADERS} ${GAME_SOURCES})

set_target_properties(Game PROPERTIES
    WIN32_EXECUTABLE "$<NOT:$<CONFIG:Debug>>"
)

target_include_directories(Game PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
    ${CMAKE_SOURCE_DIR}/ThirdParty/SFML-3.0.0/include
    ${CMAKE_SOURCE_DIR}/Engine/Source
)

target_link_directories(Game PRIVATE
    ${CMAKE_SOURCE_DIR}/ThirdParty/SFML-3.0.0/lib
)

target_link_libraries(Game PRIVATE
    Engine

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

    # OpenGL (always)
    opengl32
)

target_compile_definitions(Game PRIVATE
    PROJECT_ROOT_DIR=\"${KRAMPUS_ROOT_DIR}\"
)

target_compile_definitions(Game PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:RELEASE>
    $<$<CONFIG:Ship>:RELEASE>
    $<$<CONFIG:Ship>:SHIP>
)

if(WIN32)
    target_compile_definitions(Game PRIVATE WINDOWS)
endif()

set_target_properties(Game PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
        "${CMAKE_SOURCE_DIR}/Bin/${KRAMPUS_PLATFORM}-${KRAMPUS_ARCH}/$<CONFIG>/Game"
    PDB_OUTPUT_DIRECTORY
        "${CMAKE_SOURCE_DIR}/Bin/${KRAMPUS_PLATFORM}-${KRAMPUS_ARCH}/$<CONFIG>/Game"
)

set_target_properties(Game PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

add_custom_command(TARGET Game POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
        -D "CONFIG=$<CONFIG>"
        -D "SFML_BIN_DIR=${CMAKE_SOURCE_DIR}/ThirdParty/SFML-3.0.0/binaries"
        -D "OUT_DIR=${CMAKE_SOURCE_DIR}/Bin/${KRAMPUS_PLATFORM}-${KRAMPUS_ARCH}/$<CONFIG>/Game"
        -P "${CMAKE_SOURCE_DIR}/CMake/CopyDLLs.cmake"
    COMMENT "Copying SFML DLLs ($<CONFIG>)..."
)

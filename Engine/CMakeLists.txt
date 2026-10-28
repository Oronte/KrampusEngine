cmake_minimum_required(VERSION 3.20)
project(Engine LANGUAGES CXX)

file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS "Source/*.h")
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "Source/*.cpp")

add_library(Engine STATIC ${ENGINE_HEADERS} ${ENGINE_SOURCES})

target_precompile_headers(Engine PRIVATE Source/Utilities/Macro.h)

target_include_directories(Engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
    ${CMAKE_SOURCE_DIR}/ThirdParty/SFML-3.0.0/include
    ${CMAKE_SOURCE_DIR}/ThirdParty/box2d-3.1.1/include
)

target_link_directories(Engine PUBLIC
    ${CMAKE_SOURCE_DIR}/ThirdParty/SFML-3.0.0/lib
    ${CMAKE_SOURCE_DIR}/ThirdParty/box2d-3.1.1/lib
)

target_compile_definitions(Engine PUBLIC
    PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}"
    # A shipped game links SFML statically (see Ship/CMakeLists.txt): its headers must know it, in the Engine and in everything that includes it.
    $<$<CONFIG:Ship>:SFML_STATIC>
)

target_compile_definitions(Engine PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:RELEASE>
    $<$<CONFIG:Ship>:RELEASE>
    $<$<CONFIG:Ship>:SHIP>
)

if(WIN32)
    target_compile_definitions(Engine PRIVATE WINDOWS)
    target_compile_definitions(Engine PRIVATE
        _CRT_SECURE_NO_WARNINGS
    )
endif()

set_target_properties(Engine PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY
        "${CMAKE_SOURCE_DIR}/Bin/${KRAMPUS_PLATFORM}-${KRAMPUS_ARCH}/$<CONFIG>/Engine"
    PDB_OUTPUT_DIRECTORY
        "${CMAKE_SOURCE_DIR}/Bin/${KRAMPUS_PLATFORM}-${KRAMPUS_ARCH}/$<CONFIG>/Engine"
)

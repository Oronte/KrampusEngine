# CopyDLLs.cmake
# Appelé en post-build par : cmake -P CopyDLLs.cmake
#   -D CONFIG=<Debug|Release|Ship>
#   -D SFML_BIN_DIR=<chemin>
#   -D OUT_DIR=<chemin>

if(NOT CONFIG)
    message(FATAL_ERROR "CONFIG not defined")
endif()

if(CONFIG STREQUAL "Debug")
    set(DLLS
        sfml-audio-d-3.dll
        sfml-graphics-d-3.dll
        sfml-network-d-3.dll
        sfml-system-d-3.dll
        sfml-window-d-3.dll
    )
else() # Release ou Ship
    set(DLLS
        sfml-audio-3.dll
        sfml-graphics-3.dll
        sfml-network-3.dll
        sfml-system-3.dll
        sfml-window-3.dll
    )
endif()

foreach(DLL ${DLLS})
    set(SRC "${SFML_BIN_DIR}/${DLL}")
    set(DST "${OUT_DIR}/${DLL}")
    if(EXISTS "${SRC}")
        message(STATUS "Copying ${DLL} → ${OUT_DIR}")
        file(COPY_FILE "${SRC}" "${DST}" ONLY_IF_DIFFERENT)
    else()
        message(WARNING "DLL introuvable, copie ignorée : ${SRC}")
    endif()
endforeach()

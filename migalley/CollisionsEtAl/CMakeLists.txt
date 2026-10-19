cmake_minimum_required(VERSION 3.16)
project(CollisionsEtAl LANGUAGES CXX)

add_library(CollisionsEtAl STATIC
    # CollisionsEtAl source files
    ../3D/JIMCOL.CPP
    ../3D/PARTICLE.CPP
    ../3D/TILEMAKE.CPP
    ../Graphics/WINSMACK.CPP
    000TOOLS.CPP
)

target_link_libraries(CollisionsEtAl PRIVATE ProjectDefaults)

# Inherit common includes/defines
target_link_libraries(CollisionsEtAl PRIVATE ProjectDefaults)

# RERUN perf: TILEMAKE renders the terrain tile pipeline per frame — same
# -O2 carve-out as the other render-path TUs (see 3D/CMakeLists.txt).
set_source_files_properties(../3D/TILEMAKE.CPP
    PROPERTIES COMPILE_OPTIONS "-O2;-fno-strict-aliasing")

# Extra include dirs specific to CollisionsEtAl
target_include_directories(CollisionsEtAl
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../3D
        ${CMAKE_CURRENT_SOURCE_DIR}/../Graphics
)

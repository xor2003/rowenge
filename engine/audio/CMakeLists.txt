# rowan_audio — miniaudio engine singleton + FluidSynth MIDI renderer
# (replaces the SDL2_mixer backend inside mssw_compat.h).
cmake_minimum_required(VERSION 3.16)
project(rowan_audio LANGUAGES C CXX)

add_library(rowan_audio STATIC
    ../miniaudio/miniaudio_impl.c
    xmi2smf.c
    rowan_audio.cpp
)
target_include_directories(rowan_audio PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../miniaudio)

# FluidSynth is optional: absent on the i386 sysroot (deep dep tree);
# MIDI then degrades to silence — the same result the old SDL2_mixer
# backend produced for XMIDI (it had no XMI decoder).
find_package(PkgConfig REQUIRED)
pkg_check_modules(FLUIDSYNTH fluidsynth)
if(FLUIDSYNTH_FOUND)
    target_compile_definitions(rowan_audio PUBLIC ROWAN_HAVE_FLUIDSYNTH=1)
    target_include_directories(rowan_audio PUBLIC ${FLUIDSYNTH_INCLUDE_DIRS})
    target_link_directories(rowan_audio PUBLIC ${FLUIDSYNTH_LIBRARY_DIRS})
    target_link_libraries(rowan_audio PUBLIC ${FLUIDSYNTH_LIBRARIES})
endif()

# miniaudio needs pthread/dl/m on POSIX
if(UNIX AND NOT APPLE)
    target_link_libraries(rowan_audio PUBLIC pthread dl m)
endif()

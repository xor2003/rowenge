cmake_minimum_required(VERSION 3.16)
project(MigAlley LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Needed so clang-check/cppcheck can consume per-file flags
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ccache: recompiles whose preprocessed output is unchanged (header edits that
# a TU doesn't consume, rebuild loops, branch switches) hit the cache instead
# of re-running cc1plus.  Inert when ccache isn't installed.
find_program(ROWAN_CCACHE_PROGRAM ccache
    PATHS "$ENV{HOME}/.local/bin" "$ENV{HOME}/bin")
if(ROWAN_CCACHE_PROGRAM)
    message(STATUS "ccache: ${ROWAN_CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER   ${ROWAN_CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${ROWAN_CCACHE_PROGRAM})
endif()

option(MIG_SANITIZERS "Build with AddressSanitizer + UBSan (GCC, 32-bit)" OFF)
option(MIG_LINT "Run cppcheck during the build" OFF)
# 64-bit is the canonical build.  32-bit (-DROWAN_64BIT=OFF) is deprecated:
# kept only as a diagnostic baseline while LP64 migration issues surface.
option(ROWAN_64BIT "Build native 64-bit (drops -m32; needs amd64 deps)" ON)

# Unified repo layout: this tree lives at <repo>/migalley with siblings
# <repo>/engine (rowan shared compat) and <repo>/bob (bob-flight-sim).
# ROWAN_SYSROOT locates the extracted i386 .deb sysroot for 32-bit deps;
# it is NOT committed — CI installs the real :i386 -dev packages, and
# locally it defaults to <repo>/sysroot-i386 (symlink or copy).
# For ROWAN_64BIT builds ROWAN_SYSROOT may instead point at an amd64
# sysroot (e.g. <repo>/sysroot-amd64) holding deps not installed system-wide.
set(ROWAN_SYSROOT "$ENV{ROWAN_SYSROOT}" CACHE PATH
    "dependency sysroot (usr/include, usr/lib/<triplet>)")
if(NOT ROWAN_SYSROOT)
    if(ROWAN_64BIT)
        get_filename_component(ROWAN_SYSROOT "${CMAKE_SOURCE_DIR}/../sysroot-amd64" ABSOLUTE)
    else()
        get_filename_component(ROWAN_SYSROOT "${CMAKE_SOURCE_DIR}/../sysroot-i386" ABSOLUTE)
    endif()
endif()

# -------------------------------------------------------------------
# Dependencies via pkg-config (ensures 32-bit libs are found)
# -------------------------------------------------------------------
# The sysroot ships .pc files for deps not installed system-wide
# (SDL2_ttf/vulkan/openal on this box) — put it on pkg-config's path
# so a bare `cmake -B build64` configures without env setup.
if(IS_DIRECTORY "${ROWAN_SYSROOT}")
    file(GLOB _pc_dirs "${ROWAN_SYSROOT}/usr/lib/*/pkgconfig")
    set(ENV{PKG_CONFIG_PATH} "${_pc_dirs};${ROWAN_SYSROOT}/usr/lib/pkgconfig;$ENV{PKG_CONFIG_PATH}")
endif()
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2TTF REQUIRED SDL2_ttf)
pkg_check_modules(FLUIDSYNTH fluidsynth)
pkg_check_modules(VULKAN REQUIRED vulkan)

# Common settings for all in-house targets
add_library(ProjectDefaults INTERFACE)

if(NOT MSVC)
    # -fno-delete-null-pointer-checks: this codebase's MSVC-era idiom passes
    # null "references" (e.g. `=*(CRect*)NULL` default args, `if (&ref)` guards).
    # GCC assumes references are non-null and deletes those guards, turning
    # intended "no arg" cases into null derefs (View3d::MakeInteractive crash).
    if(ROWAN_64BIT)
        message(STATUS "Building native 64-bit (no -m32)")
        target_compile_options(ProjectDefaults INTERFACE
            -fms-extensions -Wno-enum-compare -fno-delete-null-pointer-checks
            -fwrapv -msse2 -mfpmath=sse
            # surface the 32→64 porting hazards loudly
            -Wpointer-to-int-cast -Wint-to-pointer-cast -Wformat
            # legacy idiom `if(cond) stmt; stmt2;` ran stmt2 unconditionally;
            # catch misleading indentation permanently (PATHOLOGY #3)
            -Wmisleading-indentation)
        target_link_options(ProjectDefaults INTERFACE
            -static-libstdc++ -static-libgcc)
    else()
        # Force 32-bit build on GCC/Clang, and disable enum comparison warnings
        message(DEPRECATION
            "32-bit build is deprecated — the canonical target is native 64-bit "
            "(ROWAN_64BIT=ON, the default). 32-bit remains only as an LP64 "
            "diagnostic baseline.")
        message(STATUS "Forcing 32-bit build: adding -m32 to CMAKE_CXX_FLAGS")
        target_compile_options(ProjectDefaults INTERFACE
            -m32 -fms-extensions -Wno-enum-compare -fno-delete-null-pointer-checks
            # MSVC-era code assumes wrap-around signed overflow; -fwrapv keeps
            # that behaviour well-defined under optimisation.
            -fwrapv
            # -m32 defaults to i686/x87: every float op goes through the 387 stack
            # and floorf() is a library call.  SSE2 scalar math is several times
            # faster for the software rasterizer's float-heavy inner loops and is
            # present on every CPU that can run this port (x86-64 era onward).
            -msse2 -mfpmath=sse)
        target_link_options(ProjectDefaults INTERFACE -m32
            # Statically link the C++ runtime: the build compiles against the
            # compiler's own headers, but the system libstdc++.so may be newer
            # (GCC14 vs GCC13 here) with a different class layout — the vtable
            # vbase offsets then disagree with the inline-ctor offsets and the
            # first std::ofstream construction segfaults under -O2.
            -static-libstdc++ -static-libgcc)
    endif()

    if(MIG_SANITIZERS)
        message(STATUS "Sanitizers enabled: ASan + UBSan")
        target_compile_options(ProjectDefaults INTERFACE
            -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(ProjectDefaults INTERFACE
            -fsanitize=address,undefined)
    endif()

    if(MIG_LINT)
        find_program(CPPCHECK_EXE NAMES cppcheck)
        if(CPPCHECK_EXE)
            set(CMAKE_CXX_CPPCHECK ${CPPCHECK_EXE}
                --enable=warning,performance,portability
                --inline-suppr --quiet
                -I${CMAKE_SOURCE_DIR}/H
                -D__GNUC__)
        else()
            message(WARNING "MIG_LINT=ON but cppcheck not found")
        endif()
    endif()
endif()

target_include_directories(ProjectDefaults INTERFACE
    # cxxpack/ shadows a handful of C++ standard headers so they are
    # compiled at default packing — legacy headers leave pack(1) active.
    ${CMAKE_SOURCE_DIR}/cxxpack
    ${CMAKE_SOURCE_DIR}/H
    ${SDL2_INCLUDE_DIRS}
    ${SDL2TTF_INCLUDE_DIRS}
    ${FLUIDSYNTH_INCLUDE_DIRS}
    ${VULKAN_INCLUDE_DIRS}
    # multiarch headers (SDL2/_real_SDL_config.h lives in the arch dir)
    ${ROWAN_SYSROOT}/usr/include/$<$<BOOL:${ROWAN_64BIT}>:x86_64>$<$<NOT:$<BOOL:${ROWAN_64BIT}>>:i386>-linux-gnu
    # sysroot root: vulkan/vulkan.h (the BoB compat backend; the vulkan.pc
    # includedir resolves empty so VULKAN_INCLUDE_DIRS gives nothing)
    ${ROWAN_SYSROOT}/usr/include
    # sysroot SDL2 headers (SDL_ttf.h etc.) when the system dir lacks them
    ${ROWAN_SYSROOT}/usr/include/SDL2
)

# gcc's multilib config finds the 32-bit sysroot libs implicitly, but clang
# (scan-build's linker driver) needs the pkg-config lib dirs spelled out.
target_link_directories(ProjectDefaults INTERFACE
    ${SDL2_LIBRARY_DIRS}
    ${SDL2TTF_LIBRARY_DIRS}
    ${FLUIDSYNTH_LIBRARY_DIRS}
    ${VULKAN_LIBRARY_DIRS}
    # sysroot libs (SDL2_ttf, vulkan, openal dev symlinks)
    ${ROWAN_SYSROOT}/usr/lib/x86_64-linux-gnu
)

target_link_libraries(ProjectDefaults INTERFACE
    ${SDL2_LIBRARIES}
    ${SDL2TTF_LIBRARIES}
    ${FLUIDSYNTH_LIBRARIES}
    ${VULKAN_LIBRARIES}
    # XSetErrorHandler — only the versioned runtime lib exists (no -dev pkg)
    -l:libX11.so.6
    # d3dvk.h dlopens libvulkan at runtime (no-op stub on glibc>=2.34)
    ${CMAKE_DL_LIBS}
)

target_compile_definitions(ProjectDefaults INTERFACE
    NDEBUG
    $<$<CONFIG:Debug>:_DEBUG>
    # SPIR-V shaders ship precompiled in Hardware/; the Vulkan pipeline looks
    # in CWD first, then here (installed games can place them beside the exe).
    MIG_SHADER_DIR="${CMAKE_SOURCE_DIR}/Hardware"
)

# -------------------------------------------------------------------
# Vendored ENet (reliable-ordered UDP) — the DirectPlay session transport.
add_subdirectory(${CMAKE_SOURCE_DIR}/../engine/enet ${CMAKE_BINARY_DIR}/enet)
# miniaudio + FluidSynth audio backend (replaces SDL2_mixer).
add_subdirectory(${CMAKE_SOURCE_DIR}/../engine/audio ${CMAKE_BINARY_DIR}/audio)
# Vendored libsmacker — SMK video decode (replaces the ffmpeg pipes).
add_subdirectory(${CMAKE_SOURCE_DIR}/../engine/smacker ${CMAKE_BINARY_DIR}/smacker)
target_link_libraries(ProjectDefaults INTERFACE rowan_enet rowan_audio rowan_smacker)
target_include_directories(ProjectDefaults INTERFACE
    ${CMAKE_SOURCE_DIR}/../engine/enet/include
    ${CMAKE_SOURCE_DIR}/../engine/miniaudio
    ${CMAKE_SOURCE_DIR}/../engine/audio
    ${CMAKE_SOURCE_DIR}/../engine/smacker
    ${CMAKE_SOURCE_DIR}/../engine/compat)

# -------------------------------------------------------------------
# Add subprojects
# -------------------------------------------------------------------
add_subdirectory(AI)
add_subdirectory(AIRCRAFT)
add_subdirectory(3D)
add_subdirectory(Comms)
add_subdirectory(Files)
add_subdirectory(Graphics)
add_subdirectory(Hardware)
add_subdirectory(Input)
add_subdirectory(Math)
add_subdirectory(Missman)
add_subdirectory(Model)
add_subdirectory(Movecode)
add_subdirectory(Bfields)
add_subdirectory(Overlay)
add_subdirectory(CollisionsEtAl)
add_subdirectory(Assembler)
add_subdirectory(GENERAL)
add_subdirectory(MFC)
add_subdirectory(MigAlley)
enable_testing()
add_subdirectory(Bob)

add_subdirectory(tests)

cmake_minimum_required(VERSION 3.16)
project(RowanEngine LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Shared engine build: BoB leaf code compiled natively (i386), exercising real
# ported objects through the hardened harness. See specs/rowan-shared-engine.
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)            # strict c++17 (glibc keeps M_PI etc. hidden)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ROWAN_SANITIZERS "Build with ASan + UBSan (32-bit)" OFF)

set(ROWAN_ROOT  ${CMAKE_SOURCE_DIR}/..)
set(BOB_TREE    ${ROWAN_ROOT}/battle-of-britain/source-ported/bob-flight-sim)
set(MIG_TREE    ${ROWAN_ROOT}/mig-alley/source-ported/migalley)
set(SYSROOT     ${ROWAN_ROOT}/mig-alley/sysroot-i386)
set(BOB_OVERLAY ${CMAKE_BINARY_DIR}/bob_inc_overlay)

# --- flags: same proven 32-bit set as migalley ------------------------------
add_library(RowanDefaults INTERFACE)
target_compile_options(RowanDefaults INTERFACE
    -m32 -fms-extensions -fno-delete-null-pointer-checks
    # BoB raw port needs MSVC permissiveness until sites are cleaned (T108):
    -fpermissive -Wno-narrowing
    -Wno-enum-compare)
target_link_options(RowanDefaults INTERFACE -m32)
if(ROWAN_SANITIZERS)
    target_compile_options(RowanDefaults INTERFACE
        -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(RowanDefaults INTERFACE -fsanitize=address,undefined)
endif()

# --- generated include overlay (case aliases + repaired generated headers) --
add_custom_command(
    OUTPUT ${BOB_OVERLAY}/.stamp
    COMMAND ${CMAKE_SOURCE_DIR}/tools/build/make_bob_overlay.sh ${BOB_OVERLAY} "" ${CMAKE_SOURCE_DIR}/compat
    COMMAND ${CMAKE_COMMAND} -E touch ${BOB_OVERLAY}/.stamp
    DEPENDS ${CMAKE_SOURCE_DIR}/tools/build/make_bob_overlay.sh
    COMMENT "Generating BoB include overlay")
add_custom_target(bob_overlay DEPENDS ${BOB_OVERLAY}/.stamp)

# mig include overlay: per-spelling resolution into MIG_TREE/H plus
# shared-math redirection (uppercase spellings -> shared/math/*) so one
# declaration set serves both the shared TU and mig's transitive includes.
set(MIG_OVERLAY ${CMAKE_BINARY_DIR}/mig_inc_overlay)
add_custom_command(
    OUTPUT ${MIG_OVERLAY}/.stamp
    COMMAND ${CMAKE_SOURCE_DIR}/tools/build/make_mig_overlay.sh ${MIG_OVERLAY}
    COMMAND ${CMAKE_COMMAND} -E touch ${MIG_OVERLAY}/.stamp
    DEPENDS ${CMAKE_SOURCE_DIR}/tools/build/make_mig_overlay.sh
    COMMENT "Generating mig include overlay")
add_custom_target(mig_overlay DEPENDS ${MIG_OVERLAY}/.stamp)

add_library(BobIncludes INTERFACE)
add_dependencies(BobIncludes bob_overlay)
target_include_directories(BobIncludes INTERFACE
    ${BOB_OVERLAY}
    ${BOB_TREE}/h
    ${BOB_TREE}/lib3d
    ${CMAKE_SOURCE_DIR}/compat
    ${MIG_TREE}/H
    ${SYSROOT}/usr/include/SDL2
    ${SYSROOT}/usr/include/i386-linux-gnu)
target_link_libraries(BobIncludes INTERFACE RowanDefaults)

# --- shared math library (REQ-DEDUP-01, user directive): ONE implementation
# for both games. shared/math/* is BoB base (ABI-safe 2-byte ANGLES — packed
# structs HP/SaveDataLoad embed it) + mig-compat grafts (public sin_cos,
# int& Intercept/HighIntercept forwarding overloads). mig-only routines were
# dead in BOTH trees (no callers either side) — not migrated.
# matrix.h/matrix.cpp stay per-game: SetViewParams/body2screen take
# game-specific Window types — an interface boundary, not a defect.
# Canon gate still compiles the shared TU once per game environment, so
# header-env divergence (DOSDEFS types, packing) remains observable.
add_library(SharedMathIncludes INTERFACE)
target_include_directories(SharedMathIncludes INTERFACE ${CMAKE_SOURCE_DIR}/shared/math)

add_library(rowan_math STATIC shared/math/math.cpp)
target_link_libraries(rowan_math PUBLIC SharedMathIncludes BobIncludes)

# --- BoB leaf libraries: wave 1 ------------------------------------------------
# math/: _math.cpp + matrix.cpp (BoB's own math.cpp replaced by shared TU;
# mathu.cpp is a unity-build manifest, MATRASM.ASM is Watcom asm — functions
# live ported in mathasm.h).
add_library(bob_math STATIC
    ${BOB_TREE}/math/_math.cpp
    ${BOB_TREE}/math/matrix.cpp)
target_link_libraries(bob_math PUBLIC SharedMathIncludes rowan_math BobIncludes)

# --- shared shell: install detection (T111, REQ-DETECT-01/02/03) -------------
# Pure C++, no game deps — each game exe embeds it and validates the install
# dir before entering game code. No skip-validation API by design.
add_library(rowan_shell STATIC shell/detect.cpp shell/manifests.cpp)
target_include_directories(rowan_shell PUBLIC shell)
target_link_libraries(rowan_shell PUBLIC RowanDefaults)

# --- shared native compat: migalley's io_compat (T103 resolver) ---------------
# _findfirst/_findnext (FNM_CASEFOLD) + fopen_ci (component-wise case-insensitive
# resolve) — proven code, no symlink aliases needed.
add_library(rowan_io_compat STATIC ${MIG_TREE}/MFC/io_compat.cpp)
target_link_libraries(rowan_io_compat PUBLIC RowanDefaults)
target_include_directories(rowan_io_compat PUBLIC ${MIG_TREE}/H)

# files/: real TUs only — _file.cpp/filei.cpp are MSVC unity-build manifests.
add_library(bob_files STATIC
    ${BOB_TREE}/files/diskio.cpp
    ${BOB_TREE}/files/fileman.cpp
    ${BOB_TREE}/files/loadlib.cpp
    ${BOB_TREE}/files/loadlist.cpp
    ${BOB_TREE}/files/winfile.cpp)
target_link_libraries(bob_files PUBLIC BobIncludes rowan_io_compat)
target_compile_definitions(bob_files PUBLIC ROWAN_USE_MIG_CSTRING)

# --- T104: 3d/, bfields/, movecode/ -------------------------------------------
# Canonical units from mfc/bob.vcxproj: _bfie.cpp/_move.cpp are unity TUs (all
# member files ExcludedFromBuild in every config). 3d builds per-file;
# _3D.CPP's include list is commented out upstream (empty stub, not built).
# Not in the project → excluded here: particle, ui3d, cloud, symline, lstream.
# jimtri/trimaker/decomp are #included fragments of migland.cpp.
add_library(bob_3d STATIC
    ${BOB_TREE}/3d/3dcode.cpp
    ${BOB_TREE}/3d/3dcom.cpp
    ${BOB_TREE}/3d/anmcntrl.cpp
    ${BOB_TREE}/3d/anmradio.cpp
    ${BOB_TREE}/3d/btree.cpp
    ${BOB_TREE}/3d/collided.cpp
    ${BOB_TREE}/3d/imagemap.cpp
    ${BOB_TREE}/3d/impact.cpp
    ${BOB_TREE}/3d/jimcol.cpp
    ${BOB_TREE}/3d/landscap.cpp
    ${BOB_TREE}/3d/loadpcx.cpp
    ${BOB_TREE}/3d/migland.cpp
    ${BOB_TREE}/3d/monotxt.cpp
    ${BOB_TREE}/3d/overlay.cpp
    ${BOB_TREE}/3d/shapes.cpp
    ${BOB_TREE}/3d/shpbld.cpp
    ${BOB_TREE}/3d/smktrls.cpp
    ${BOB_TREE}/3d/tilemake.cpp
    ${BOB_TREE}/3d/transite.cpp
    ${BOB_TREE}/3d/viewsel.cpp
    ${BOB_TREE}/3d/weappak.cpp
    ${BOB_TREE}/3d/world.cpp)
target_link_libraries(bob_3d PUBLIC BobIncludes)
target_compile_definitions(bob_3d PUBLIC ROWAN_USE_MIG_CSTRING)
# -ffunction-sections lets test executables --gc-sections away the monolithic
# game-code bodies (AirStruc/World deps) and keep only the leaf math they call.
target_compile_options(bob_3d PRIVATE -ffunction-sections)

# -ffunction/-fdata-sections: lets leaf tests --gc-sections-link only the
# functions under test instead of stubbing the entire un-migrated call graph.
add_library(bob_bfields STATIC ${BOB_TREE}/bfields/_bfie.cpp)
target_link_libraries(bob_bfields PUBLIC BobIncludes)
target_compile_definitions(bob_bfields PUBLIC ROWAN_USE_MIG_CSTRING)
target_compile_options(bob_bfields PRIVATE -ffunction-sections -fdata-sections)

# sagmove.cpp is a separate TU in the project (not in _move.cpp's unity list).
add_library(bob_movecode STATIC
    ${BOB_TREE}/movecode/_move.cpp
    ${BOB_TREE}/movecode/sagmove.cpp)
target_link_libraries(bob_movecode PUBLIC BobIncludes)
target_compile_definitions(bob_movecode PUBLIC ROWAN_USE_MIG_CSTRING)

# --- tests -------------------------------------------------------------------
enable_testing()

# T106 canonical runners — one shared source (tests/common/runner.cpp), two
# thin executables, one per game tree. Standalone per owner directive: no
# whole-project builds — mig_runner compiles just mig's Math/MATH.CPP.
add_executable(bob_runner tests/common/runner.cpp)
target_include_directories(bob_runner PRIVATE tests/common)
target_compile_definitions(bob_runner PRIVATE ROWAN_GAME_BOB)
target_link_libraries(bob_runner PRIVATE SharedMathIncludes bob_math BobIncludes)

# One implementation, two environments: mig_runner compiles the SAME shared
# TU under mig's header env via the mig overlay (resolves every spelling to
# MIG_TREE/H except shared-math names, which redirect to shared/math/* — one
# ANGLES/MathLib decl set, no redefinition clash). The gate now detects
# header-env divergence rather than impl drift.
add_executable(mig_runner
    tests/common/runner.cpp
    shared/math/math.cpp)
add_dependencies(mig_runner mig_overlay)
target_include_directories(mig_runner PRIVATE tests/common ${MIG_OVERLAY} ${MIG_TREE}/H)
target_compile_definitions(mig_runner PRIVATE ROWAN_GAME_MIG)
target_link_libraries(mig_runner PRIVATE RowanDefaults)

add_executable(bob_leaf_tests
    tests/unit/main.cpp
    tests/unit/test_bob_mathasm.cpp
    tests/unit/test_bob_matrix.cpp
    tests/unit/test_bob_shapelayout.cpp
    tests/unit/test_bob_collision.cpp
    # collided.cpp linked directly (not via bob_3d): the test only needs
    # GroundHeight/RayHit*; -ffunction-sections + --gc-sections drops the
    # monolith's game-code bodies and their AirStruc/World dependencies.
    ${BOB_TREE}/3d/collided.cpp
    tests/bob/stub_globals.cpp)   # Timer_Code + no-op globals for static-init
target_include_directories(bob_leaf_tests PRIVATE tests)
target_compile_options(bob_leaf_tests PRIVATE -ffunction-sections)
target_link_libraries(bob_leaf_tests PRIVATE SharedMathIncludes bob_math BobIncludes)
target_link_options(bob_leaf_tests PRIVATE -Wl,--gc-sections)

add_test(NAME bob_leaf_tests COMMAND bob_leaf_tests)
set_tests_properties(bob_leaf_tests PROPERTIES
    TIMEOUT 60
    ENVIRONMENT "LD_LIBRARY_PATH=${SYSROOT}/usr/lib/i386-linux-gnu")

# bob_port_tests: leaf tests against the PORTED bob-flight-sim headers
# themselves (BobIncludes only, no SharedMathIncludes). Shared-math tests
# cover the shared header; these cases pin the overlay-resolved h/mathasm.h
# that bob_game TUs actually compile — a divergence here (e.g. SHDIVSIN's
# dividend) would silently ship wrong arithmetic.
add_executable(bob_port_tests
    tests/unit/main.cpp
    tests/unit/test_bob_mathasm_port.cpp)
target_include_directories(bob_port_tests PRIVATE tests)
target_link_libraries(bob_port_tests PRIVATE BobIncludes)

add_test(NAME bob_port_tests COMMAND bob_port_tests)
set_tests_properties(bob_port_tests PROPERTIES
    TIMEOUT 60
    ENVIRONMENT "LD_LIBRARY_PATH=${SYSROOT}/usr/lib/i386-linux-gnu")

add_executable(bob_files_tests
    tests/unit/main.cpp
    tests/bob/test_bob_files.cpp
    tests/bob/stub_globals.cpp)
target_include_directories(bob_files_tests PRIVATE tests)
target_link_directories(bob_files_tests PRIVATE
    ${SYSROOT}/usr/lib/i386-linux-gnu)
target_link_libraries(bob_files_tests PRIVATE SharedMathIncludes bob_files bob_math BobIncludes
    SDL2 pthread)

add_test(NAME bob_files_tests COMMAND bob_files_tests)
set_tests_properties(bob_files_tests PROPERTIES
    TIMEOUT 60
    ENVIRONMENT "LD_LIBRARY_PATH=${SYSROOT}/usr/lib/i386-linux-gnu:${SYSROOT}/lib")

# bob_bfields_tests: gc-sections pulls only the sections under test
# (SetLoc/GetLoc + BFieldGlobalTable + static-init Timer_Code stub).
add_executable(bob_bfields_tests
    tests/unit/main.cpp
    tests/bob/test_bob_bfields.cpp
    tests/bob/test_bob_savebin.cpp
    tests/bob/stub_globals.cpp
    tests/bob/test_bob_packages_stubs.cpp   # dead-path refs from packages.cpp
    tests/bob/test_bob_nodedata_stub.cpp    # Node_Data blob (ctor unlinked)
    # packages.cpp linked directly (not via a missman lib): SquadListRef ops +
    # SquadList::squadsizes for Profile::DecodePackage's squad-list insert;
    # gc-sections drops the rest of the monolith's game-code bodies.
    ${BOB_TREE}/missman/packages.cpp)
target_compile_options(bob_bfields_tests PRIVATE -ffunction-sections)
target_include_directories(bob_bfields_tests PRIVATE tests)
target_link_directories(bob_bfields_tests PRIVATE
    ${SYSROOT}/usr/lib/i386-linux-gnu)
target_link_options(bob_bfields_tests PRIVATE -Wl,--gc-sections)
target_link_libraries(bob_bfields_tests PRIVATE SharedMathIncludes bob_bfields bob_math BobIncludes
    SDL2 pthread)

add_test(NAME bob_bfields_tests COMMAND bob_bfields_tests)
set_tests_properties(bob_bfields_tests PROPERTIES
    TIMEOUT 60
    ENVIRONMENT "LD_LIBRARY_PATH=${SYSROOT}/usr/lib/i386-linux-gnu:${SYSROOT}/lib")

add_executable(shell_detect_tests
    tests/unit/main.cpp
    tests/unit/test_shell_detect.cpp)
target_include_directories(shell_detect_tests PRIVATE tests shell)
target_link_libraries(shell_detect_tests PRIVATE rowan_shell RowanDefaults)

add_test(NAME shell_detect_tests COMMAND shell_detect_tests)
set_tests_properties(shell_detect_tests PROPERTIES
    TIMEOUT 30)

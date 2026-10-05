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
    COMMAND ${CMAKE_SOURCE_DIR}/tools/build/make_bob_overlay.sh ${BOB_OVERLAY}
    COMMAND ${CMAKE_COMMAND} -E touch ${BOB_OVERLAY}/.stamp
    DEPENDS ${CMAKE_SOURCE_DIR}/tools/build/make_bob_overlay.sh
    COMMENT "Generating BoB include overlay")
add_custom_target(bob_overlay DEPENDS ${BOB_OVERLAY}/.stamp)

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

# --- BoB leaf libraries: wave 1 ------------------------------------------------
# math/: _math.cpp math.cpp matrix.cpp (mathu.cpp is a unity-build manifest,
# MATRASM.ASM is Watcom asm — functions live ported in h/mathasm.h).
add_library(bob_math STATIC
    ${BOB_TREE}/math/_math.cpp
    ${BOB_TREE}/math/math.cpp
    ${BOB_TREE}/math/matrix.cpp)
target_link_libraries(bob_math PUBLIC BobIncludes)

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

add_executable(bob_leaf_tests
    tests/unit/main.cpp
    tests/unit/test_bob_mathasm.cpp)
target_include_directories(bob_leaf_tests PRIVATE tests)
target_link_libraries(bob_leaf_tests PRIVATE bob_math BobIncludes)

add_test(NAME bob_leaf_tests COMMAND bob_leaf_tests)
set_tests_properties(bob_leaf_tests PROPERTIES
    TIMEOUT 60
    ENVIRONMENT "LD_LIBRARY_PATH=${SYSROOT}/usr/lib/i386-linux-gnu")

add_executable(bob_files_tests
    tests/unit/main.cpp
    tests/bob/test_bob_files.cpp
    tests/bob/stub_globals.cpp)
target_include_directories(bob_files_tests PRIVATE tests)
target_link_directories(bob_files_tests PRIVATE
    ${SYSROOT}/usr/lib/i386-linux-gnu)
target_link_libraries(bob_files_tests PRIVATE bob_files bob_math BobIncludes
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
    tests/bob/stub_globals.cpp)
target_include_directories(bob_bfields_tests PRIVATE tests)
target_link_directories(bob_bfields_tests PRIVATE
    ${SYSROOT}/usr/lib/i386-linux-gnu)
target_link_options(bob_bfields_tests PRIVATE -Wl,--gc-sections)
target_link_libraries(bob_bfields_tests PRIVATE bob_bfields bob_math BobIncludes
    SDL2 pthread)

add_test(NAME bob_bfields_tests COMMAND bob_bfields_tests)
set_tests_properties(bob_bfields_tests PROPERTIES
    TIMEOUT 60
    ENVIRONMENT "LD_LIBRARY_PATH=${SYSROOT}/usr/lib/i386-linux-gnu:${SYSROOT}/lib")

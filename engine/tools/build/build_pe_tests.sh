#!/usr/bin/env bash
# T105: build the BoB PE test runner (bob_tests_pe.exe) with a locally
# extracted mingw-w64 i686 toolchain and run it under Wine.
# Reference infra only — the release port is native, this exists so
# MFC-coupled/Windows-semantics code can be exercised in a Windows CRT env.
set -euo pipefail

RE="$(cd "$(dirname "$0")/../../.."; pwd)/engine"
BOB="$(cd "$(dirname "$0")/../../.."; pwd)/bob"
TC=${MINGW_I686:-$HOME/games/bob/toolchains/mingw-i686/prefix/usr}
OVL=${BOB_OVERLAY:-$RE/build/bob_inc_overlay}
UT=$BOB/UnitTest++
PE=$RE/tests/pe
OUT=${PE_OUT:-$RE/build/pe}
mkdir -p "$OUT"

export PATH="$TC/bin:$PATH"
CXX=i686-w64-mingw32-g++
CXXFLAGS="-fms-extensions -fpermissive -Wno-narrowing -Wno-enum-compare \
-fno-delete-null-pointer-checks -ffunction-sections -fdata-sections"

# BoB objects needed by the test TU (matrix/_math/math provide the math lib;
# pe_compat.h supplies MFC DEBUG_NEW placement-new overloads mingw lacks).
for src in math/matrix.cpp math/math.cpp math/_math.cpp; do
    obj=$OUT/$(basename ${src%.cpp}).o
    echo "CXX $src"
    $CXX $CXXFLAGS -include "$PE/pe_compat.h" \
        -I"$OVL" -I"$BOB/h" -I"$BOB/lib3d" -c "$BOB/$src" -o "$obj"
done

# UnitTest++ (upstream bundled copy, Win32 TimeHelpers)
mkdir -p "$OUT/utpp"
for src in "$UT"/src/*.cpp "$UT"/src/Win32/TimeHelpers.cpp; do
    obj=$OUT/utpp/$(basename ${src%.cpp}).o
    [ -f "$obj" ] || $CXX -I"$UT/src" -c "$src" -o "$obj"
done
ar="$TC/bin/i686-w64-mingw32-ar"; [ -x "$ar" ] || ar=i686-w64-mingw32-ar
$ar rcs "$OUT/libunittestpp.a" "$OUT"/utpp/*.o

echo "CXX test_bob_pe.cpp"
$CXX $CXXFLAGS -include "$PE/pe_compat.h" \
    -I"$OVL" -I"$BOB/h" -I"$BOB/lib3d" -I"$UT/src" \
    -c "$PE/test_bob_pe.cpp" -o "$OUT/test_bob_pe.o"
$CXX -ffunction-sections -I"$UT/src" -c "$PE/runner_main.cpp" -o "$OUT/runner_main.o"

echo "LINK bob_tests_pe.exe"
$CXX -static -Wl,--gc-sections \
    "$OUT/runner_main.o" "$OUT/test_bob_pe.o" \
    "$OUT/matrix.o" "$OUT/math.o" "$OUT/_math.o" \
    "$OUT/libunittestpp.a" -o "$OUT/bob_tests_pe.exe"

if [ "${1:-}" = "--run" ]; then
    wine "$OUT/bob_tests_pe.exe"
fi

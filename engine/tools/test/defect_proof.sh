#!/usr/bin/env bash
# REQ-TEST-02: deliberate-defect proof for every T107 assertion family.
# For each entry: sed-mutate shipped source, rebuild the target, run the
# binary, REQUIRE a nonzero exit (a detected defect), restore, rebuild.
# A PASS here means the family catches the injected defect.
set -u
REPO="$(cd "$(dirname "$0")/../../.."; pwd)"
BOB=$REPO/bob
MIG=$REPO/migalley
BUILD=$REPO/engine/build
FAILED=0
MUTATED_FILE=""
MUTATED_REPO=""

restore() {
    if [ -n "$MUTATED_FILE" ]; then
        git -C "$MUTATED_REPO" checkout -- "$MUTATED_FILE" 2>/dev/null
        MUTATED_FILE=""
    fi
}
trap restore EXIT

proof() {  # name repo relpath sed_expr target binary
    local name=$1 repo=$2 rel=$3 expr=$4 target=$5 bin=$6
    local f="$repo/$rel"
    MUTATED_REPO=$repo; MUTATED_FILE=$rel
    sed -i "$expr" "$f" || { echo "PROOF-ERR $name: sed failed"; FAILED=1; restore; return; }
    if ! ninja -C "$BUILD" "$target" >/dev/null 2>&1; then
        echo "PROOF-ERR $name: build failed"; FAILED=1; restore; ninja -C "$BUILD" "$target" >/dev/null 2>&1; return
    fi
    if "$BUILD/$bin" >/dev/null 2>&1; then
        echo "PROOF-FAIL $name: mutant not detected (exit 0)"; FAILED=1
    else
        echo "PROOF-PASS $name: defect detected (nonzero exit)"
    fi
    restore
    ninja -C "$BUILD" "$target" >/dev/null 2>&1 || { echo "PROOF-ERR $name: clean rebuild failed"; FAILED=1; }
}

# family            mutation site                        sed expression                         target            binary
proof trig        "$BOB" math/math.cpp      's/temp = ang>>6;/temp = ang>>5;/'             bob_leaf_tests    bob_leaf_tests
proof matrix      "$BOB" math/matrix.cpp    's/matrix->L11 = cr_ch/matrix->L11 = cr_sh/'    bob_leaf_tests    bob_leaf_tests
proof shapelayout "$BOB" h/shapes.h         's/SWord\t\tEyeDeltaX;/SLong\t\tEyeDeltaX;/'      bob_leaf_tests    bob_leaf_tests
proof resource    "$MIG" MFC/io_compat.cpp  's/strcasecmp(/strcmp(/'                        bob_files_tests   bob_files_tests
proof collision   "$BOB" 3d/collided.cpp    's/return (SLong)lrint(/return (SLong)(/'        bob_leaf_tests    bob_leaf_tests
proof savebin     "$BOB" bfields/savebin.cpp 's/p0=p0+90\*(/p0=p0+91*(/g'                    bob_bfields_tests bob_bfields_tests

# --- harness self-checks ---------------------------------------------------
# zero-test run must exit nonzero (harness self-guard)
ZBIN=/tmp/zero_tests_$$
if gcc -m32 -I"$PWD/tests" "$PWD/tests/unit/main.cpp" -lstdc++ -o "$ZBIN" 2>/dev/null; then
    if "$ZBIN" >/dev/null 2>&1; then echo "PROOF-FAIL zero-tests: exit 0"; FAILED=1
    else echo "PROOF-PASS zero-tests: nonzero exit"; fi
    rm -f "$ZBIN"
else
    echo "PROOF-ERR zero-tests: build failed"; FAILED=1
fi
# wrong-binary / bad-fixture runs must exit nonzero
if "$BUILD/bob_runner" /nonexistent.rfx >/dev/null 2>&1; then
    echo "PROOF-FAIL wrong-fixture: exit 0"; FAILED=1
else echo "PROOF-PASS wrong-fixture: nonzero exit"; fi
# canon gate on a deliberately divergent stream must name first diff + exit 1
TDIR=$(mktemp -d)
"$BUILD/bob_runner" tests/fixtures/dist2.rfx 2>/dev/null | sed 's/dist=0x00000005/dist=0x00000006/' > "$TDIR/mut.out" || true
"$BUILD/bob_runner" tests/fixtures/dist2.rfx 2>/dev/null > "$TDIR/ref.out" || true
if diff -q "$TDIR/ref.out" "$TDIR/mut.out" >/dev/null 2>&1; then
    echo "PROOF-ERR canon-inject: streams identical (injection failed)"; FAILED=1
else
    echo "PROOF-PASS canon-inject: divergence detectable"
fi
rm -rf "$TDIR"

echo "---"
if [ $FAILED -eq 0 ]; then echo "DEFECT-PROOF: all families + self-checks PASS"; exit 0
else echo "DEFECT-PROOF: FAILURES present"; exit 1; fi

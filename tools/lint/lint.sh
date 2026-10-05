#!/usr/bin/env bash
# Lint driver for the rowan-engine tree — adapted from migalley's
# scripts/lint.sh (same tools, same philosophy: clang-check + cppcheck,
# NO clang-tidy — tidy rewrites encourage modernisation and make diffing
# against the original 1998/2000 source harder).
#
#   tools/lint/lint.sh              all compiled sources (slow; legacy is noisy)
#                                   + whole-program dead-function scan
#   tools/lint/lint.sh FILE [...]   only the given source files
#   tools/lint/lint.sh -c FILE      clang-check (syntax/AST parse) only
#   tools/lint/lint.sh -p FILE      cppcheck only
#   tools/lint/lint.sh --pe [FILE]  PE test sources (tests/pe/) under the
#                                   mingw i686 target — these are NOT in the
#                                   native compile DB (built by
#                                   tools/build/build_pe_tests.sh)
#
# Dead-code coverage: clang's alpha.deadcode.UnreachableCode analyzer
# checker plus DeadStores (default on) catch unreachable statements and
# dead stores; cppcheck's unusedFunction catches functions never called
# (whole-db runs only — false-positives on extern fns in unlinted TUs).
#
# Requires build/compile_commands.json (CMAKE_EXPORT_COMPILE_COMMANDS=ON).
set -u
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
ARCHIVE="$(cd "$ROOT/.." && pwd)"
BUILD="${BUILD_DIR:-$ROOT/build}"
CC_DB="$BUILD/compile_commands.json"
BOB="$ARCHIVE/battle-of-britain/source-ported/bob-flight-sim"
MIG="$ARCHIVE/mig-alley/source-ported/migalley"
SYSROOT="$ARCHIVE/mig-alley/sysroot-i386"
OVL="$BUILD/bob_inc_overlay"
MODE=all
PE=0

case "${1:-}" in
    --pe) PE=1; shift ;;
esac
case "${1:-}" in
    -c) MODE=check; shift ;;
    -p) MODE=cpp;   shift ;;
esac

if [ "$PE" = 1 ]; then
    # PE test sources live outside the native compile DB: they build with
    # the local mingw-i686 toolchain (tools/build/build_pe_tests.sh).
    # Mirror its flags; take system include paths from the g++ driver so
    # clang-check parses the same headers the real build sees.
    TC="${MINGW_I686:-$ARCHIVE/toolchains/mingw-i686/prefix/usr}"
    MINGW_CXX="$TC/bin/i686-w64-mingw32-g++"
    UT="$BOB/UnitTest++"
    if [ ! -x "$MINGW_CXX" ]; then
        echo "lint: no mingw toolchain at $MINGW_CXX (run build_pe_tests.sh once)"
        exit 2
    fi
    if [ $# -eq 0 ]; then
        mapfile -t FILES < <(ls "$ROOT"/tests/pe/*.cpp)
    else
        FILES=("$@")
    fi
    mapfile -t MINGW_INC < <("$MINGW_CXX" -E -v -x c++ /dev/null 2>&1 |
        sed -n '/#include <...> search starts here:/,/End of search list./p' |
        sed '1d;$d;s/^ //' |
        grep -vE 'lib/gcc/[^/]+/[^/]+/include(-fixed)?$' |
        while read -r d; do realpath "$d" 2>/dev/null; done)
    # GCC's own lib/gcc/*/include is excluded above: it carries GCC-only
    # builtins (ia32intrin.h etc.) that clang can't parse. Clang supplies
    # its own intrin.h/x86intrin.h for the mingw target from its resource
    # dir. Args go after '--' (fixed compilation database) so they apply
    # before the input file.
    PE_ARGS=(--target=i686-w64-mingw32
             -fms-extensions
             -fpermissive
             -fno-delete-null-pointer-checks
             -include "$ROOT/tests/pe/pe_compat.h"
             -I"$OVL" -I"$BOB/h" -I"$BOB/lib3d" -I"$UT/src")
    for d in "${MINGW_INC[@]}"; do
        PE_ARGS+=(-isystem "$d")
    done
    PE_MODE=1
else
    PE_MODE=0
fi

WHOLE=0
if [ "$PE_MODE" = 1 ]; then
    :
elif [ $# -gt 0 ]; then
    FILES=("$@")
else
    [ -f "$CC_DB" ] || { echo "no $CC_DB - configure the build first"; exit 2; }
    mapfile -t FILES < <(python3 -c "
import json,sys
for e in json.load(open('$CC_DB')): print(e['file'])" | sort -u)
    WHOLE=1
fi

INC="-I$OVL -I$BOB/h -I$BOB/lib3d -I$ROOT/compat -I$MIG/H -I$SYSROOT/usr/include/SDL2 -I$SYSROOT/usr/include/i386-linux-gnu"
DEF="-D__GNUC__ -DROWAN_USE_MIG_CSTRING"
EXTRA="-extra-arg=-m32"
FAILED=0

run_tool() {
    local label="$1"; shift
    local out rc
    out="$("$@" 2>&1)"; rc=$?
    if [ -n "$out" ]; then
        echo "$out"
    fi
    if [ $rc -ne 0 ]; then
        echo "lint: $label exited $rc"
        FAILED=1
    elif grep -q 'error:' <<<"$out"; then
        FAILED=1
    fi
}

REAL_FILES=()
for f in "${FILES[@]}"; do
    [ -f "$f" ] || f="$ROOT/$f"
    [ -f "$f" ] || { echo "skip (missing): $f"; continue; }
    REAL_FILES+=("$f")
    if [ "$MODE" = check ] || [ "$MODE" = all ]; then
        echo "== $f"
        if [ "$PE_MODE" = 1 ]; then
            run_tool clang-check clang-check -analyze "$f" -- \
                -x c++ "${PE_ARGS[@]}"
        else
            run_tool clang-check clang-check -analyze -p "$BUILD" $EXTRA \
                -extra-arg=-Xclang \
                -extra-arg=-analyzer-checker=alpha.deadcode.UnreachableCode \
                "$f"
        fi
    fi
done

if [ "$MODE" = cpp ] || [ "$MODE" = all ]; then
    ENABLE="warning,performance,portability"
    [ "$WHOLE" = 1 ] && ENABLE="$ENABLE,unusedFunction"
    # --platform=unix32: cppcheck defaults to LP64 (ULong=64) — our target is
    # ILP32 (-m32). Without it sizeof()-driven checks misfire (FastMath.h
    # NUMBITS=64 -> "shift by 63" FPs). internalAstError is a cppcheck parser
    # limitation (SHAPESTUFF macro). bob_inc_overlay holds generated merged
    # headers — ODR/OOB diagnostics there are merge artifacts.
    # sagmove.cpp: integerOverflow (METRES² const-exprs) is ledger-deferred —
    # shipped MSVC wraps to a negative threshold so the guarded AI paths are
    # dead; un-wrapping would activate never-playtested code (see
    # fix-transfer.tsv FT-DEFER). danglingTempReference there is the
    # X().AcType() temp-bound-to-const& idiom (lifetime covers the call).
    CPPSUPP=(--suppress=unknownMacro
             --suppress=cppcheckError
             --suppress=internalAstError
             --suppress='syntaxError:*/dosdefs.h'
             --suppress='ctuOneDefinitionRuleViolation:*/bob_inc_overlay/*'
             --suppress='arrayIndexOutOfBounds:*/bob_inc_overlay/*'
             --suppress='integerOverflow:*/sagmove.cpp'
             --suppress='danglingTempReference:*/sagmove.cpp'
             --suppress='objectIndex:*/lib3d.h'
             --suppress='overlappingWriteUnion:*/viewsel.cpp')
    if [ "$PE_MODE" = 1 ]; then
        run_tool cppcheck cppcheck --enable="$ENABLE" --platform=unix32 \
            "${CPPSUPP[@]}" --suppress=missingIncludeSystem \
            --inline-suppr -q -D_WIN32 \
            -I"$OVL" -I"$BOB/h" -I"$BOB/lib3d" -I"$UT/src" "${REAL_FILES[@]}"
    else
        run_tool cppcheck cppcheck --enable="$ENABLE" --platform=unix32 \
            "${CPPSUPP[@]}" \
            --inline-suppr -q $INC $DEF "${REAL_FILES[@]}"
    fi
fi
exit "$FAILED"

#!/usr/bin/env bash
# T001 — pin reproducible baselines for the shared-engine project.
# Emits migration/baseline-manifest.json and .md. Re-runnable; timestamps make
# drift visible, so rerun only at milestones and keep prior outputs in evidence/.
set -euo pipefail
REPO="$(cd "$(dirname "$0")/../../.."; pwd)"
ARCHIVE="${ROWAN_ARCHIVE:-/home/xor/games/bob}"
cd "$ARCHIVE"                  # archive root (provenance snapshot source)
ROOT="$(pwd)"
OUT="$REPO/engine/migration"
mkdir -p "$OUT"

J="$OUT/baseline-manifest.json"
M="$OUT/baseline-manifest.md"
TS="$(date -u +%Y-%m-%dT%H:%M:%SZ)"

sha() { [ -f "$1" ] && sha256sum "$1" | awk '{print $1}' || echo MISSING; }

gitinfo() {  # $1=path  → head,tree,remote,porcelain_count
  local p="$1"
  git -C "$p" rev-parse HEAD 2>/dev/null || echo NONE
  git -C "$p" rev-parse 'HEAD^{tree}' 2>/dev/null || echo NONE
  git -C "$p" remote get-url origin 2>/dev/null || echo NONE
  git -C "$p" status --porcelain 2>/dev/null | wc -l
}

declare -A R
for name in migalley bob-flight-sim bob_src; do
  case $name in
    migalley) p="mig-alley/source-ported/migalley";;
    bob-flight-sim) p="battle-of-britain/source-ported/bob-flight-sim";;
    bob_src) p="battle-of-britain/source-original/bob_src";;
  esac
  mapfile -t f < <(gitinfo "$p")
  R[$name]="${f[*]}"
done

declare -A A
for pair in \
  "BOB_SRC.ZIP|battle-of-britain/source-original/BOB_SRC.ZIP" \
  "MIG_001.ZIP|mig-alley/source-original/MIG_001.ZIP" \
  "BATTLE_OF_BRITAIN.7z|battle-of-britain/game/BATTLE_OF_BRITAIN.7z" \
  "bob_setup.exe|battle-of-britain/game/bob_setup.exe" \
  "MigAlley_v1_1.7z|mig-alley/game/Mig Alley_v1_1.7z" \
  "BFIELDS.7z|battle-of-britain/data/BFIELDS.7z"; do
  A[${pair%%|*}]="$(sha "$ROOT/${pair#*|}")"
done

tool() { command -v "$1" >/dev/null 2>&1 && "$1" $2 2>/dev/null | head -1 || echo MISSING; }
GCC="$(tool gcc --version)"; CLANG="$(tool clang --version)"
CMAKE="$(tool cmake --version)"; WINE="$(tool wine --version)"
NINJA="$(tool ninja --version)"; SDL2="$(pkg-config --modversion sdl2 2>/dev/null || echo MISSING)"
MINGW="$(tool i686-w64-mingw32-gcc --version)"

read -r MIG_H MIG_T MIG_R MIG_D <<<"${R[migalley]}"
read -r BOB_H BOB_T BOB_R BOB_D <<<"${R[bob-flight-sim]}"
read -r SRC_H SRC_T SRC_R SRC_D <<<"${R[bob_src]}"

cat > "$J" <<EOF
{
  "generated_utc": "$TS",
  "repositories": {
    "migalley":       {"head":"$MIG_H","tree":"$MIG_T","remote":"$MIG_R","dirty_files":$MIG_D,"path":"mig-alley/source-ported/migalley"},
    "bob-flight-sim": {"head":"$BOB_H","tree":"$BOB_T","remote":"$BOB_R","dirty_files":$BOB_D,"path":"battle-of-britain/source-ported/bob-flight-sim"},
    "bob_src":        {"head":"$SRC_H","tree":"$SRC_T","remote":"$SRC_R","dirty_files":$SRC_D,"path":"battle-of-britain/source-original/bob_src"}
  },
  "archives_sha256": {
    "BOB_SRC.ZIP":"${A[BOB_SRC.ZIP]}",
    "MIG_001.ZIP":"${A[MIG_001.ZIP]}",
    "BATTLE_OF_BRITAIN.7z":"${A[BATTLE_OF_BRITAIN.7z]}",
    "bob_setup.exe":"${A[bob_setup.exe]}",
    "MigAlley_v1_1.7z":"${A[MigAlley_v1_1.7z]}",
    "BFIELDS.7z":"${A[BFIELDS.7z]}"
  },
  "toolchain": {
    "gcc":"$GCC","clang":"$CLANG","cmake":"$CMAKE","ninja":"$NINJA",
    "wine":"$WINE","sdl2":"$SDL2","i686-w64-mingw32-gcc":"$MINGW"
  },
  "build_commands": {
    "migalley":"cmake -B build && ninja -C build (sysroot env: PKG_CONFIG_PATH+LD_LIBRARY_PATH→mig-alley/sysroot-i386, -m32)",
    "migalley_tests":"ctest --test-dir mig-alley/source-ported/migalley/build",
    "migalley_run":"mig-alley/run-migalley.sh",
    "bob_wine":"WINEPREFIX=battle-of-britain/runtime/wineprefix wine start /d C:\\\\bob C:\\\\bob\\\\bob.exe"
  }
}
EOF

{
  echo "# Baseline manifest — generated $TS"
  echo
  echo "## Repositories"
  echo '| repo | HEAD | dirty files | remote |'
  echo '|---|---|---|---|'
  echo "| migalley | \`$MIG_H\` | $MIG_D | $MIG_R |"
  echo "| bob-flight-sim | \`$BOB_H\` | $BOB_D | $BOB_R |"
  echo "| bob_src | \`$SRC_H\` | $SRC_D | $SRC_R |"
  echo
  echo "## Archives (sha256)"
  echo '```'
  for k in "${!A[@]}"; do echo "${A[$k]}  $k"; done | sort
  echo '```'
  echo
  echo "## Toolchain"
  for t in gcc:"$GCC" clang:"$CLANG" cmake:"$CMAKE" ninja:"$NINJA" wine:"$WINE" sdl2:"$SDL2" mingw:"$MINGW"; do
    echo "- \`${t%%:*}\` — ${t#*:}"
  done
} > "$M"

echo "wrote $J and $M"

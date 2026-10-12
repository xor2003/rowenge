#!/usr/bin/env bash
# bob_game_sweep.sh — syntax-check every Battle-of-Britain game TU that the
# mfc/bob.vcproj Release config actually compiles.
#
# The vcproj's authoritative source list has three kinds of entry:
#   * unity aggregations (_AI.CPP, _MODE.CPP, ... _3D.CPP) — PCH=0, they
#     #include their siblings .cpp files directly
#   * standalone TUs (nodebob.cpp, Sagmove.cpp, 3d stragglers) — PCH injected
#   * ExcludedFromBuild files — compiled only via the unity #include chain
#     (or Debug/Profile-only variants never in Release)
#
# This sweep mirrors Release: PCH=0 files compile bare; everything else gets
# `-include stdafx.h` (Bob/port/stdafx.h replicates the MSVC /Yu injection).
# Files that fail ONLY standalone are fine if their unity container is clean
# — that is how Release builds them.
set -u

REPO_ROOT="$(cd "$(dirname "$0")/../../.."; pwd)"
BOB=${BOB:-$REPO_ROOT/bob}
MIG=${MIG:-$REPO_ROOT/migalley}
SYS=${SYS:-${ROWAN_SYSROOT:-$REPO_ROOT/sysroot-i386}}
ROWAN=${ROWAN:-$REPO_ROOT/engine}
OVR=${OVR:-$MIG/build/bob_overlay}
VCPROJ=${VCPROJ:-$BOB/mfc/bob.vcproj}

CXX=${CXX:-/usr/bin/c++}
BASE="-DNDEBUG -D_DEBUG -DROWAN_USE_MIG_CSTRING -I$MIG/Bob/port -I$OVR -I$MIG/H -I$ROWAN/compat -I$SYS/usr/include -I$SYS/usr/include/SDL2 -I$SYS/usr/include/i386-linux-gnu -g -std=gnu++17 -fpermissive -m32 -fms-extensions -Wno-enum-compare -Wno-narrowing -fno-delete-null-pointer-checks -fsyntax-only"

LIST=${1:-/tmp/bob_gamefiles.txt}
NOPCH=${2:-/tmp/bob_nopch.txt}

# Regenerate the Release source list + PCH=0 set from the vcproj when absent.
if [ ! -s "$LIST" ] || [ ! -s "$NOPCH" ]; then
python3 - "$VCPROJ" "$LIST" "$NOPCH" <<'EOF'
import re, sys
xml = open(sys.argv[1]).read()
blocks = re.findall(r'<File\s+RelativePath="[^"]+"[^>]*(?:/>|>.*?</File>)', xml, re.S)
excluded, nopch = set(), set()
for b in blocks:
    rel = re.search(r'RelativePath="([^"]+)"', b).group(1).replace('\\','/')
    key = rel.lstrip('./').lower()
    cfgs = re.findall(r'<FileConfiguration\b[^>]*>', b)
    if cfgs and all('ExcludedFromBuild="true"' in c for c in cfgs):
        excluded.add(key)
    if 'UsePrecompiledHeader="0"' in b:
        nopch.add(key)
files = []
for b in blocks:
    m = re.search(r'RelativePath="([^"]+\.(?:cpp|c|cxx))"', b, re.I)
    if not m: continue
    p = m.group(1).replace('\\','/')
    if '/' not in p: continue                    # mfc-local: covered by T118
    p2 = re.sub(r'^\.\./', '', p)
    if p2.lower().startswith('mfc/'): continue   # frontend lib already built
    if p2.lower() in excluded: continue          # dead/unity-pulled files
    files.append(p2)
open(sys.argv[2],'w').write('\n'.join(files)+'\n')
open(sys.argv[3],'w').write('\n'.join(sorted(nopch))+'\n')
EOF
fi

total=0; clean=0
TSV=${TSV:-/tmp/bob_game_sweep.tsv}
> "$TSV"
while read -r f; do
  d=$(dirname "$f" | tr 'A-Z' 'a-z'); b=$(basename "$f")
  real=$(find "$BOB/$d" -maxdepth 1 -iname "$b" 2>/dev/null | head -1)
  [ -z "$real" ] && { echo -e "MISSING\t$f" >> "$TSV"; continue; }
  total=$((total+1))
  rel=$(echo "$f" | tr 'A-Z' 'a-z')
  if grep -qx "$rel" "$NOPCH"; then FLAGS="$BASE"; else FLAGS="$BASE -include stdafx.h"; fi
  errs=$(timeout 300 $CXX $FLAGS "$real" 2>&1 | grep -cE "error:")
  [ "$errs" = "0" ] && clean=$((clean+1)) || echo -e "$f\t$errs" >> "$TSV"
done < "$LIST"
echo "=== $clean/$total standalone-clean (unity-covered files may fail standalone; check their container)"
grep -v MISSING "$TSV" | sort -t$'\t' -k2 -rn | head -40
grep MISSING "$TSV" || true
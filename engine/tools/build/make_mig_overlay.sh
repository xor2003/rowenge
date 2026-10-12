#!/usr/bin/env bash
# Generate the mig include-overlay directory for the shared-math build.
#
# Like the BoB overlay (make_bob_overlay.sh) but for mig's H/ tree, plus one
# extra rule: EVERY spelling of the shared-math headers (uppercase or
# lowercase) resolves into rowan-engine/shared/math/ so the shared TU and
# mig's own headers see ONE declaration set for ANGLES/MathLib/vector —
# without it a TU could pull shared myangles.h AND mig MYANGLES.H and hit
# class-redefinition (their ANGLES differ: SWord a vs int a).
#
# Usage: make_mig_overlay.sh <overlay_dir>
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.."; pwd)"
MIG_H="$ROOT/mig-alley/source-ported/migalley/H"
SHARED="$ROOT/rowan-engine/shared/math"
OUT="${1:?usage: make_mig_overlay.sh <overlay_dir>}"
mkdir -p "$OUT"

# 1) every real mig header under its own name + an all-lowercase alias
for f in "$MIG_H/"*.[Hh] "$MIG_H/"*.[Gg]; do
  [ -f "$f" ] || continue
  b=$(basename "$f")
  ln -sf "$f" "$OUT/$b"
  ln -sf "$f" "$OUT/$(printf '%s' "$b" | tr 'A-Z' 'a-z')"
done

# 2) shared-math redirection: every spelling -> shared/math/* (overrides 1)
for s in myangles.h mathasm.h mymath.h myvector.h vector.h _math.h fastmath.h; do
  ln -sf "$SHARED/$s" "$OUT/$s"
  ln -sf "$SHARED/$s" "$OUT/$(printf '%s' "$s" | tr 'a-z' 'A-Z')"
done
ln -sf "$SHARED/myangles.h" "$OUT/myAngles.h"   # math.cpp's exact spelling

# 3) remaining include spellings used by shared/math/* resolved into the mig
#    env (case-insensitive match inside MIG_H); shared names already taken.
grep -rhoE '#include[[:space:]]+"[^"/\\]+"' "$SHARED" 2>/dev/null |
  sed -E 's/#include[[:space:]]+"([^"]+)"/\1/' | LC_ALL=C sort -u |
  while IFS= read -r inc; do
    if [ ! -e "$OUT/$inc" ]; then
      match=$(find "$MIG_H" -maxdepth 1 -iname "$inc" | head -1)
      if [ -n "$match" ]; then ln -sf "$match" "$OUT/$inc"; fi
    fi
  done

echo "mig overlay: $(ls "$OUT" | wc -l) entries -> $OUT"

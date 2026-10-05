#!/usr/bin/env bash
# Regenerate the BoB include-overlay directory used by the native build.
#
# BoB's ported tree keeps headers under h/ in mixed case while sources spell
# includes differently ("DOSDefs.h" vs dosdefs.h). Rather than editing ~400
# include spellings, the build compiles against an overlay dir of symlinks:
# every spelling the sources actually use, resolved case-insensitively to the
# real file. Files needing repair (uidvals.g) are generated here, not edited
# in the pristine bob_src repo.
#
# Usage: make_bob_overlay.sh <overlay_dir>
set -euo pipefail
BOB_ROOT="$(cd "$(dirname "$0")/../../.."; pwd)/battle-of-britain"
PORTED="$BOB_ROOT/source-ported/bob-flight-sim"
ORIG_H="$BOB_ROOT/source-original/bob_src/SRC/H"
OUT="${1:?usage: make_bob_overlay.sh <overlay_dir>}"
mkdir -p "$OUT"

# 1) every real header, under its own name
for f in "$PORTED/h/"* "$PORTED/lib3d/"*.h; do
  [ -f "$f" ] && ln -sf "$f" "$OUT/$(basename "$f")"
done

# 2) every spelling used by #include "name" across ported sources, resolved
#    case-insensitively to a real header
grep -rhoE '#include[[:space:]]+"[^"/\\]+"' "$PORTED" \
    --include='*.cpp' --include='*.h' -r 2>/dev/null |
  sed -E 's/#include[[:space:]]+"([^"]+)"/\1/' | LC_ALL=C sort -u |
  while IFS= read -r inc; do
    if [ ! -e "$OUT/$inc" ]; then
      match=$(find "$PORTED/h" "$PORTED/lib3d" -maxdepth 1 -iname "$inc" | head -1)
      if [ -n "$match" ]; then ln -sf "$match" "$OUT/$inc"; fi
    fi
  done

# 3) generated headers missing from the ported tree, repaired from bob_src.
#    UIDVALS.G has a Makebf bug: a stray ^Z truncates 'UI_56=0x27fd,' and the
#    include guard's #endif appears mid-file with an orphan #endif at the end.
python3 - "$ORIG_H/UIDVALS.G" "$OUT/uidvals.g" <<'EOF'
import sys
lines = open(sys.argv[1], 'rb').read().split(b'\n')
assert lines[449].strip() == b'#endif' and b'x27fd,' in lines[450] \
   and lines[468].strip() == b'#endif', "UIDVALS.G corruption shape changed"
lines[449] = b'\t\tUI_56=0x27fd,\t// RERUN: repaired Makebf corruption'
del lines[450]   # stray 'x27fd,' fragment
open(sys.argv[2], 'wb').write(b'\n'.join(lines).replace(b'\x1a', b''))
EOF

# 4) F_SOUNDS.G: Fmake2-generated file list missing from the ported tree
#    (FILES.G includes it when F_SOUNDS is undefined). Copy verbatim.
cp "$ORIG_H/F_SOUNDS.G" "$OUT/F_SOUNDS.G"

# 5) BFREFS.G: another Fmake2-generated list used by bfields/globrefs.cpp
# BFREFS.G was written over a longer older version: a ^Z EOF marker sits after
# the complete table (NULL};) with stale tail fragments behind it. Truncate at
# the first ^Z — stripping bytes leaves orphaned list fragments that don't parse.
python3 - "$ORIG_H/BFREFS.G" "$OUT/bfrefs.g" <<'EOF'
import sys
b = open(sys.argv[1], 'rb').read()
cut = b.find(b'\x1a')
assert cut > 0 and b[:cut].rstrip().endswith(b'NULL};'), "BFREFS.G corruption shape changed"
open(sys.argv[2], 'wb').write(b[:cut])
EOF

echo "overlay: $(ls "$OUT" | wc -l) entries -> $OUT"

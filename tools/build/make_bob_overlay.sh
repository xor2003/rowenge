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
OUT="${1:?usage: make_bob_overlay.sh <overlay_dir> [mig_h_dir] [shadow_dir ...]}"
MIG_H="${2:-}"
shift $(( $# >= 2 ? 2 : $# ))
mkdir -p "$OUT"

# Shadow dirs: include dirs that appear EARLIER on the compile -I path than
# the overlay (e.g. rowan-engine/compat, mig's H). A spelling that exists
# literally in one of these must not get an overlay link — the earlier dir
# has to win (e.g. d3dtypes.h resolves to compat/, never the vendored
# vs2005 copy; strings.h stays the system header).
SHADOW_DIRS=("$@")

# 0) mig-H precedence (optional second arg): the ported BoB tree still ships
#    the generated OLE wrapper headers (h/rbutton.h etc.) but sources must
#    see mig's ported widget declarations. Link ONLY the port surface (mig/H
#    has ~259 same-named headers — linking all of it would shadow BoB's own).
#
#    These are emitted as pack-pinning WRAPPERS, not symlinks. Mig's port
#    classes are compiled at MIXED packing: every widget .CPP sits under
#    pack(1) (RDIALOG.H pulls GLOBDEFS->DOSDEFS before the widget header),
#    while MFC_stub.h/DLGITEM.H are included BEFORE that pragma and so are
#    natural. BoB TUs must see each class exactly as its implementation TU
#    does — an include-order slip in one TU is an ODR layout split.
#    (The real killer at T123: H/<name> are stale OLE IDispatch stubs,
#    132-byte CREdtBt, while MFC/<name> are the ported classes the linked
#    code implements, 149-byte CREdtBt — ctor writes past BoBFrag's end.)
#
#    Targets: MFC/<name> when it exists (the ported impl header), else H/
#    (RTICKBOX.H/RTITLE.H live only in H/ and ARE the real defs there).
pin_hdr() { # pin_hdr <pack-level> <overlay-name> <real-header-path>
  # rm first: a stale symlink with this name would make > clobber the REAL
  # header it points at (bit us once — destroyed 16 mig headers).
  rm -f "$OUT/$2"
  # BoB's mfc/<w>.h forwarders pre-define the same <w>_INCLUDED guard the
  # MFC/ impl headers test — under the forwarder chain the guard arrives
  # defined and the real class is skipped entirely (incomplete-type errors).
  # #undef the target's own guard macro so the class always parses.
  local g
  g=$(sed -n -E 's/^#ifndef[[:space:]]+([A-Za-z_0-9]*_INCLUDED_?).*/\1/p;
                 s/^#if[[:space:]]+!defined\(([A-Za-z_0-9]*_INCLUDED_?)\).*/\1/p' \
        "$3" | head -1)
  {
    printf '#pragma pack(push,%s) /* T123: pin port class to impl-TU packing */\n' "$1"
    [ -n "$g" ] && printf '#undef %s /* T123: bob forwarder pre-defines this guard */\n' "$g"
    printf '#include "%s"\n' "$3"
    printf '#pragma pack(pop)\n'
  } > "$OUT/$2"
}
if [ -n "$MIG_H" ]; then
  MIG_H="$(cd "$MIG_H" && pwd)"
  MIG_MFC="$(cd "$MIG_H/../MFC" && pwd)"
  # Widget classes — impl TUs see them at pack(1) (post-DOSDEFS).
  for b in RBUTTON.H RCOMBO.H REDIT.H REDTBT.H RJOYCFG.H RLISTBOX.H \
           RRADIO.H RSCRLBAR.H RSPINBUT.H RSPLTBAR.H RSTATIC.H RTABS.H \
           RTICKBOX.H RTITLE.H; do
    if   [ -f "$MIG_MFC/$b" ]; then f="$MIG_MFC/$b"
    elif [ -f "$MIG_H/$b"   ]; then f="$MIG_H/$b"
    else continue; fi
    pin_hdr 1 "$b" "$f"
    pin_hdr 1 "$(echo "$b" | tr 'A-Z' 'a-z')" "$f"
  done
  # Port classes impl TUs see at NATURAL packing (included before the
  # DOSDEFS pack-1 pragma) — pin 8 makes layout include-order-proof.
  for b in MFC_stub.h WIN32_COMPAT.H DLGITEM.H MigCstring.h io_compat.h \
           timeapi.h CSTRING.H dinput_stub.h dplay_stub.h dsound_stub.h; do
    [ -f "$MIG_H/$b" ] || continue
    pin_hdr 8 "$b" "$MIG_H/$b"
  done
  # FILES.H/fileman classes — FILEMAN.CPP compiles them under DOSDEFS
  # pack(1).
  pin_hdr 1 FILES.H "$MIG_H/FILES.H"
  # lowercase spellings some TUs use (skips anything that could shadow a
  # system header — cstring.h in particular stays unwrapped).
  pin_hdr 1 files.h "$MIG_H/FILES.H"
  pin_hdr 8 dlgitem.h "$MIG_H/DLGITEM.H"
  pin_hdr 8 win32_compat.h "$MIG_H/WIN32_COMPAT.H"
  pin_hdr 8 mfc_stub.h "$MIG_H/MFC_stub.h"
  pin_hdr 8 migcstring.h "$MIG_H/MigCstring.h"
  # BoB sources say <dsound.h>; mig's surface is dsound_stub.h
  [ -f "$MIG_H/dsound_stub.h" ] && pin_hdr 8 dsound.h "$MIG_H/dsound_stub.h"
fi

# compat/d3dtypes.h pulls the real DX7 SDK type universe via <d3d7_sdk.h>;
# point that spelling at BoB's vendored copy of the Microsoft header.
ln -sf "$PORTED/h/vs2005/d3dtypes.h" "$OUT/d3d7_sdk.h"

# Header search dirs:
#   PRIMARY — BoB's own headers, mfc/ BEFORE h/: the mfc/*.h files are
#     5-line forwarders that define the *_INCLUDED guard macros BoB code
#     tests (MAINTBAR_INCLUDED gates mainfrm.h's MainToolBar decl) before
#     including "..\h\X.h". Linking h/ directly would skip the guard define.
#     The overlay sits before mig's H on the -I path so BoB game headers
#     shadow mig's same-named game headers (SHAPES.H, node.h).
#   VENDORED — h/vs2005 SDK headers. Resolved only when no earlier -I dir
#     provides the spelling literally (shadow dirs), so the compat layer's
#     replacements (d3dtypes.h) win while true-only-in-vendor headers
#     (DLS1.H, DMDLS.H) still resolve.
SEARCH_DIRS=("$PORTED/mfc" "$PORTED/h" "$PORTED/lib3d")
VENDORED_DIRS=("$PORTED/h/vs2005")

# 1) every real header, under its own name — never clobbering step-0 mig
#    links or earlier dirs in this step. mfc/ goes first so its guard-defining
#    forwarders win over h/'s real headers; h/ links every file (it also
#    carries .HPP/.INC/.ASH asm-era includes); lib3d only headers.
for d in "$PORTED/mfc" "$PORTED/h" "$PORTED/lib3d"; do
  for f in "$d/"*; do
    [ -f "$f" ] || continue
    b=$(basename "$f")
    [ -e "$OUT/$b" ] || ln -sf "$f" "$OUT/$b"
  done
done
# NB: h/vs2005 headers are NOT own-name linked — spellings resolve them in
# step 2 only when no shadow dir provides the same name (d3dtypes.h must
# resolve to the compat layer, not the vendored Direct3D header).

# 2) every spelling used by #include "name" across ported sources, resolved
#    case-insensitively to a real header (extensions match case-insensitively:
#    vendored SDK headers are .H)
grep -rhoE '#include[[:space:]]+"[^"/\\]+"' "$PORTED" \
    --include='*.[ch]' --include='*.[CH]' --include='*.cpp' --include='*.CPP' \
    --include='*.cxx' --include='*.cc' --include='*.asm' --include='*.ASM' \
    --include='*.inc' --include='*.INC' --include='*.g' --include='*.G' \
    -r 2>/dev/null |
  sed -E 's/#include[[:space:]]+"([^"]+)"/\1/' | LC_ALL=C sort -u |
  while IFS= read -r inc; do
    if [ ! -e "$OUT/$inc" ]; then
      match=$(find "${SEARCH_DIRS[@]}" -maxdepth 1 -iname "$inc" | head -1)
      if [ -z "$match" ]; then
        # vendored fallback: only when no earlier -I dir provides the
        # spelling literally — those must win (d3dtypes.h -> compat)
        shadowed=
        for d in ${SHADOW_DIRS[@]+"${SHADOW_DIRS[@]}"}; do
          [ -e "$d/$inc" ] && { shadowed=1; break; }
        done
        [ -z "$shadowed" ] && \
          match=$(find "${VENDORED_DIRS[@]}" -maxdepth 1 -iname "$inc" | head -1)
      fi
      if [ -n "$match" ]; then ln -sf "$match" "$OUT/$inc"; fi
      # NOTE: deliberately no whole-tree fallback — spellings that don't
      # resolve here (strings.h, time.h) must hit system/SDK headers.
    fi
  done

# 3) Windows-style #include "..\dir\file.h" spellings: on POSIX the backslash
#    is literal, so create links with those exact names pointing at the real
#    file (resolved relative to the including source's directory).
grep -rhoE '#include[[:space:]]+"\.\.\\[^"]+"' "$PORTED" \
    --include='*.cpp' --include='*.h' -r 2>/dev/null |
  sed -E 's/#include[[:space:]]+"([^"]+)"/\1/' | LC_ALL=C sort -u |
  while IFS= read -r inc; do
    [ -e "$OUT/$inc" ] && continue
    # "..\mfc\resource.h" -> real relative path "mfc/resource.h"
    rel=$(printf '%s' "$inc" | sed -E 's|^\.\.\\||; s|\\|/|g')
    match=$(find "$PORTED" -type f -ipath "*/$rel" | head -1)
    if [ -n "$match" ]; then ln -sf "$match" "$OUT/$inc"; fi
  done

# 5) generated headers missing from the ported tree, repaired from bob_src.
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

# 6) BFREFS.G: another Fmake2-generated list used by bfields/globrefs.cpp
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

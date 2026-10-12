#!/bin/sh
# Launch the native Linux port of Battle of Britain from the unified repo.
#   ROWAN_SYSROOT     — 32-bit dep sysroot (default ./sysroot-i386)
#   ROWAN_BOB_GAMEDIR — installed retail tree (default
#                       ./runtime/wineprefix/drive_c/bob)
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SYSROOT=${ROWAN_SYSROOT:-$HERE/sysroot-i386}/usr/lib/i386-linux-gnu
GAMEDIR=${ROWAN_BOB_GAMEDIR:-$HERE/runtime/wineprefix/drive_c/bob}

export LD_LIBRARY_PATH=$SYSROOT${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
cd "$GAMEDIR" || { echo "no game data at $GAMEDIR" >&2; exit 1; }
exec "$HERE/build/Bob/bob" "$@"

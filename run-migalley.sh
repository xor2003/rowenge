#!/bin/sh
# Launch the native Linux port of MiG Alley from the unified repo.
# Expects (env-overridable):
#   ROWAN_SYSROOT   — 32-bit dep sysroot (default ./sysroot-i386)
#   ROWAN_MIG_GAMEDIR — installed retail tree with patched roots.dir
#                     (default ./runtime/migprefix/drive_c/rowan/mig)
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SYSROOT=${ROWAN_SYSROOT:-$HERE/sysroot-i386}/usr/lib/i386-linux-gnu
GAMEDIR=${ROWAN_MIG_GAMEDIR:-$HERE/runtime/migprefix/drive_c/rowan/mig}

export LD_LIBRARY_PATH=$SYSROOT${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
cd "$GAMEDIR" || { echo "no game data at $GAMEDIR" >&2; exit 1; }
exec "$HERE/build/MigAlley/MigAlley" "$@"

#!/bin/sh
# Launch the native Linux port of Battle of Britain from the unified repo.
#   ROWAN_BUILD       — build tree to run: build64 (default if present) or build
#   ROWAN_SYSROOT     — dep sysroot (default ./sysroot-<arch> matching the build)
#   ROWAN_BOB_GAMEDIR — installed retail tree (default
#                       ./runtime/wineprefix/drive_c/bob)
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BUILD=${ROWAN_BUILD:-}
if [ -z "$BUILD" ]; then
    if [ -x "$HERE/build64/Bob/bob" ]; then BUILD=build64; else BUILD=build; fi
fi
case $BUILD in
    build64) DEFROOT=sysroot-amd64/usr/lib/x86_64-linux-gnu ;;
    *)       DEFROOT=sysroot-i386/usr/lib/i386-linux-gnu ;;
esac
SYSROOT=${ROWAN_SYSROOT:-$HERE/$DEFROOT}
GAMEDIR=${ROWAN_BOB_GAMEDIR:-$HERE/runtime/wineprefix/drive_c/bob}

export LD_LIBRARY_PATH=$SYSROOT${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
cd "$GAMEDIR" || { echo "no game data at $GAMEDIR" >&2; exit 1; }
exec "$HERE/$BUILD/Bob/bob" "$@"

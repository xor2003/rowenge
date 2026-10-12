#!/usr/bin/env bash
# T003 probe: launch MiG on private Xvfb, skip intro, then step through a
# click/key sequence (from $SEQFILE, one "key|click|wait|shot NAME ..." per
# line) saving screenshots to evidence/m0/flightprobe/. Interactive probing —
# not a gate test.
set -u
DISP=":111"
OUT="/home/xor/games/bob/rowan-engine/evidence/m0/flightprobe"
LAUNCHER="/home/xor/games/bob/mig-alley/run-migalley.sh"
SEQFILE="${1:?usage: $0 seqfile}"
mkdir -p "$OUT"

Xvfb "$DISP" -screen 0 1600x900x24 >/dev/null 2>&1 &
XVFB_PID=$!
GAME_PID=""
cleanup(){ [ -n "${GAME_PID:-}" ] && kill "$GAME_PID" 2>/dev/null; kill "$XVFB_PID" 2>/dev/null; }
trap cleanup EXIT
sleep 1

DISPLAY="$DISP" sh "$LAUNCHER" >"$OUT/game.log" 2>&1 &
GAME_PID=$!
for i in $(seq 1 60); do grep -q "OpenSmack" "$OUT/game.log" && break; kill -0 "$GAME_PID" || break; sleep 1; done
echo "intro started, click-skip"
DISPLAY="$DISP" xdotool mousemove 800 450 click 1
sleep 8
DISPLAY="$DISP" import -window root "$OUT/00_menu.png"

n=1
while IFS= read -r line; do
  [ -z "$line" ] && continue; case "$line" in \#*) continue;; esac
  set -- $line
  case "$1" in
    key)   DISPLAY="$DISP" xdotool key "$2";;
    click) DISPLAY="$DISP" xdotool mousemove "$2" "$3" click 1;;
    dblclick) DISPLAY="$DISP" xdotool mousemove "$2" "$3" click 1 click 1;;
    wait)  sleep "$2";;
    shot)  sleep "${3:-1}"; printf -v f "%s/%02d_%s.png" "$OUT" "$n" "$2"; DISPLAY="$DISP" import -window root "$f"; echo "saved $f"; n=$((n+1));;
  esac
done < "$SEQFILE"
echo "done; killing game"; kill "$GAME_PID" 2>/dev/null

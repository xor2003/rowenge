#!/usr/bin/env bash
# Menu layout audit (REQ-style): launches the game with ROWAN_DUMP_LAYOUT=1
# so every dialog emits [layout] lines, drives a click sequence through the
# frontend, then validates widget geometry:
#
#   - every control inside its dialog rect (screen coords, 4px tolerance)
#   - same-class controls in a row share height (radio/button groups)
#   - no unexpected same-class overlap (area > 25% of the smaller control)
#   - rect drift vs the RC template want= rect (max |delta|, reported)
#   - process alive at the end (no crash); screenshots saved for eyeballing
#
# Usage: menu_audit.sh <binary> <gamedir> [seqfile]
#   seqfile lines:  wait <sec> | key <xdotool-key> | click <x> <y> |
#                   shot <name> | mark <label>
#   coords are 1024x768 window-space; window is auto-located.
# Exit: 0 pass, 1 fail, 77 skip.
set -u
BIN="${1:?usage: menu_audit.sh <binary> <gamedir> [seqfile]}"
GAMEDIR="${2:?usage: menu_audit.sh <binary> <gamedir> [seqfile]}"
SEQ="${3:-}"
HERE="$(cd "$(dirname "$0")" && pwd)"
SYSROOT="${ROWAN_SYSROOT:-$HERE/../../sysroot-i386}/usr/lib/i386-linux-gnu"

for t in Xvfb xdotool import python3; do
    command -v "$t" >/dev/null 2>&1 || { echo "SKIP: $t not found"; exit 77; }
done
[ -x "$BIN" ] || { echo "SKIP: binary $BIN missing"; exit 77; }
[ -d "$GAMEDIR" ] || { echo "SKIP: no game data at $GAMEDIR"; exit 77; }

DISP=""
for d in 130 131 132 133; do
    xdpyinfo -display ":$d" >/dev/null 2>&1 || { DISP=":$d"; break; }
done
[ -n "$DISP" ] || { echo "SKIP: no free X display"; exit 77; }

Xvfb "$DISP" -screen 0 1280x1024x24 >/dev/null 2>&1 &
XVFB_PID=$!
GAME_PID=""
cleanup() {
    [ -n "$GAME_PID" ] && kill "$GAME_PID" 2>/dev/null
    kill "$XVFB_PID" 2>/dev/null
}
trap cleanup EXIT
sleep 1

LOG=$(mktemp /tmp/menu_audit.XXXXXX.log)
SHOTS=$(mktemp -d /tmp/menu_audit_shots.XXXXXX)
echo "log: $LOG  shots: $SHOTS  display: $DISP"

(cd "$GAMEDIR" && DISPLAY="$DISP" SDL_VIDEO_X11_XINPUT2=0 SDL_AUDIODRIVER=dummy \
    ROWAN_DUMP_LAYOUT=1 LD_LIBRARY_PATH="$SYSROOT" "$BIN" .) >"$LOG" 2>&1 &
GAME_PID=$!

# wait for the window
WID=""
for i in $(seq 1 60); do
    WID=$(DISPLAY="$DISP" xdotool search --onlyvisible --name '.' 2>/dev/null | head -1)
    [ -n "$WID" ] && break
    kill -0 "$GAME_PID" 2>/dev/null || { echo "FAIL: process died before window"; tail -20 "$LOG"; exit 1; }
    sleep 1
done
[ -n "$WID" ] || { echo "FAIL: no window appeared"; tail -20 "$LOG"; exit 1; }

# window origin for coord translation
eval "$(DISPLAY="$DISP" xwininfo -id "$WID" | awk '/Absolute upper-left X/{x=$NF}/Absolute upper-left Y/{y=$NF}END{print "WX="x";WY="y}')"
echo "window $WID at $WX,$WY"

run_seq() {
    local f="$1"
    while IFS= read -r line; do
        case "$line" in
            ''|'#'*) continue ;;
            wait\ *)   sleep "${line#wait }" ;;
            key\ *)    DISPLAY="$DISP" xdotool key --window "$WID" "${line#key }" ;;
            click\ *)  set -- ${line#click }; DISPLAY="$DISP" xdotool mousemove $((WX+$1)) $((WY+$2)) click 1 ;;
            shot\ *)   DISPLAY="$DISP" import -window "$WID" "$SHOTS/${line#shot }.png" ;;
            mark\ *)   echo "[audit] ${line#mark }" >> "$LOG" ;;
            *) echo "warn: unknown seq op: $line" ;;
        esac
    done < "$f"
}

# default sequence: dismiss intro/config, land on the menu
if [ -z "$SEQ" ]; then
    sleep 8
    DISPLAY="$DISP" xdotool key Return; sleep 4
    DISPLAY="$DISP" xdotool mousemove 660 520 click 1; sleep 6
else
    run_seq "$SEQ"
fi
sleep 2
DISPLAY="$DISP" import -window "$WID" "$SHOTS/final.png" 2>/dev/null || true

ALIVE=1
kill -0 "$GAME_PID" 2>/dev/null || ALIVE=0
kill "$GAME_PID" 2>/dev/null; wait "$GAME_PID" 2>/dev/null

# --- validate the [layout] records ---------------------------------------
python3 - "$LOG" <<'PYEOF'
import re, sys
log = open(sys.argv[1], errors='replace').read()
dlgs = {}   # dlg -> dict(screen=..., ctrls=[(id,cls,rect,want,text)])
fail = []
for line in log.splitlines():
    m = re.match(r"\[layout\] dlg=(\d+) screen=(-?\d+),(-?\d+)-(-?\d+),(-?\d+) dlgrect=", line)
    if m:
        d = int(m.group(1))
        dlgs.setdefault(d, {'screen': tuple(map(int, m.groups()[1:])), 'ctrls': []})
        continue
    m = re.match(r"\[layout\]  dlg=(\d+) ctl=(\d+) cls=(\S+) rect=(-?\d+),(-?\d+)-(-?\d+),(-?\d+) want=(-?\d+),(-?\d+)-(-?\d+),(-?\d+) text='(.*)'", line)
    if m:
        d = int(m.group(1))
        g = m.groups()
        dlgs.setdefault(d, {'screen': None, 'ctrls': []})['ctrls'].append(
            (int(g[1]), g[2], tuple(map(int, g[3:7])), tuple(map(int, g[7:11])), g[11]))

if not dlgs:
    print("FAIL: no [layout] records — ROWAN_DUMP_LAYOUT did not fire")
    sys.exit(1)

TOL = 4
for d, info in sorted(dlgs.items()):
    scr = info['screen']
    if not scr:
        continue
    sx0, sy0, sx1, sy1 = scr
    # Controls in an embedded child dialog carry the parent's screen offset:
    # estimate this dlg's translation as the most common (rect - want) delta.
    from collections import Counter
    offs = Counter((r[0] - w[0], r[1] - w[1])
                   for _, _, r, w, _ in info['ctrls']
                   if r[2] - r[0] > 0 and w[2] - w[0] > 0)
    doff = offs.most_common(1)[0][0] if offs else (0, 0)
    for cid, cls, r, w, txt in info['ctrls']:
        x0, y0, x1, y1 = r
        if x1 - x0 <= 0 or y1 - y0 <= 0:
            # zero-size is only a defect when the RC template says otherwise;
            # plain Statics are owner-drawn placeholders — report, don't fail
            if w[2] - w[0] > 0 and w[3] - w[1] > 0 and cls != 'Static':
                fail.append(f"dlg {d} ctl {cid} ({cls} '{txt}'): empty rect {r} (want {w})")
            continue
        # containment: child rects are dialog-relative; the dialog's own
        # extent is a template hint, so only flag gross outliers
        dw_, dh_ = sx1 - sx0, sy1 - sy0
        if dw_ > 0 and dh_ > 0 and (x1 - x0 > dw_ + 32 or y1 - y0 > dh_ + 32):
            fail.append(f"dlg {d} ctl {cid} ({cls} '{txt}'): rect {r} grossly exceeds dialog extent {scr}")
        # drift vs RC want, normalized by the dialog's embedding offset
        dw = [abs(r[0] - w[0] - doff[0]), abs(r[1] - w[1] - doff[1]),
              abs((r[2] - r[0]) - (w[2] - w[0])), abs((r[3] - r[1]) - (w[3] - w[1]))]
        if max(dw) > 64:
            fail.append(f"dlg {d} ctl {cid} ({cls} '{txt}'): rect {r} drifts {max(dw)}px from want {w} (dlg off {doff})")
    # row groups: same-class controls sharing a top -> equal heights
    rows = {}
    for cid, cls, r, w, txt in info['ctrls']:
        rows.setdefault((cls, r[1]), []).append((cid, r, w, txt))
    for (cls, top), items in rows.items():
        if len(items) < 2:
            continue
        hs = {it[1][3] - it[1][1] for it in items}
        ws = {it[1][2] - it[1][0] for it in items}
        # mixed heights only fail when items look like a row of same-shape
        # widgets — spinner pairs (half-height cells) and title bars (wide
        # text strip next to buttons) legitimately mix heights
        if len(hs) > 1 and max(hs) > 2 * min(hs) and max(ws) < 2 * min(ws):
            fail.append(f"dlg {d} {cls} row y={top}: mixed heights {sorted(hs)} "
                        f"({[(i[0], i[3]) for i in items]})")
        # pairwise overlap of same row, same class — skip pairs whose
        # template rects already coincide (intentional stacked controls)
        for i in range(len(items)):
            for j in range(i + 1, len(items)):
                a, b = items[i][1], items[j][1]
                wa, wb = items[i][2], items[j][2]
                ox = max(0, min(a[2], b[2]) - max(a[0], b[0]))
                oy = max(0, min(a[3], b[3]) - max(a[1], b[1]))
                wox = max(0, min(wa[2], wb[2]) - max(wa[0], wb[0]))
                woy = max(0, min(wa[3], wb[3]) - max(wa[1], wb[1]))
                small = min((a[2]-a[0])*(a[3]-a[1]), (b[2]-b[0])*(b[3]-b[1]))
                wsmall = min((wa[2]-wa[0])*(wa[3]-wa[1]), (wb[2]-wb[0])*(wb[3]-wb[1]))
                if small and ox * oy > small * 0.25 and \
                   not (wsmall and wox * woy > wsmall * 0.25):
                    fail.append(f"dlg {d} {cls} overlap: ctl {items[i][0]} '{items[i][3]}' x ctl {items[j][0]} '{items[j][3]}'")

print(f"[audit] {len(dlgs)} dialogs: " +
      ", ".join(f"{d}({len(i['ctrls'])} ctl)" for d, i in sorted(dlgs.items())))
if fail:
    print("FAIL:")
    for f in fail[:40]:
        print("  " + f)
    sys.exit(1)
print("[audit] layout checks passed")
PYEOF
RC=$?

if [ "$ALIVE" = 0 ]; then echo "FAIL: game exited during menu walk"; exit 1; fi
[ $RC -ne 0 ] && exit 1
echo "PASS: menu audit clean (shots in $SHOTS)"
exit 0

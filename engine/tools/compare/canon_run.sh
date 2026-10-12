#!/usr/bin/env bash
# T106 canonical equivalence gate (REQ-REPLAY-01).
#
#   canon_run.sh <fixture.rfx> [more.rfx...]
#
# For each fixture: runs bob_runner and mig_runner THREE times each.
#   * determinism: 3 runs of a runner must emit identical CANON hashes
#   * equivalence: bob's stream must equal mig's (GAME banner line excepted)
#   * on divergence: prints the first differing REC line pair (names the
#     tick and field) — the REQ-REPLAY-01 first-difference report.
#
# Exit 0 = all fixtures identical; exit 1 = any divergence; exit 2 = usage/env.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BUILD="$HERE/../../build"
BOB_RUNNER="${BOB_RUNNER:-$BUILD/bob_runner}"
MIG_RUNNER="${MIG_RUNNER:-$BUILD/mig_runner}"
RUNS=3

[ $# -ge 1 ] || { echo "usage: $0 <fixture.rfx>..."; exit 2; }
for x in "$BOB_RUNNER" "$MIG_RUNNER"; do
    [ -x "$x" ] || { echo "missing runner: $x"; exit 2; }
done

fail=0
for fix in "$@"; do
    [ -f "$fix" ] || { echo "missing fixture: $fix"; fail=2; continue; }
    name=$(basename "$fix" .rfx)

    canon_a=""; canon_b=""; diverge=0
    for i in $(seq $RUNS); do
        "$BOB_RUNNER" "$fix" > "/tmp/canon_bob_$name.out" 2>/dev/null || {
            echo "FAIL $name: bob_runner run $i exited $?"; diverge=1; break; }
        c=$(tail -1 "/tmp/canon_bob_$name.out")
        if [ -z "$canon_a" ]; then canon_a=$c;
        elif [ "$c" != "$canon_a" ]; then
            echo "FAIL $name: bob_runner nondeterministic (run1=$canon_a run$i=$c)"
            diverge=1; break
        fi
    done
    for i in $(seq $RUNS); do
        "$MIG_RUNNER" "$fix" > "/tmp/canon_mig_$name.out" 2>/dev/null || {
            echo "FAIL $name: mig_runner run $i exited $?"; diverge=1; break; }
        c=$(tail -1 "/tmp/canon_mig_$name.out")
        if [ -z "$canon_b" ]; then canon_b=$c;
        elif [ "$c" != "$canon_b" ]; then
            echo "FAIL $name: mig_runner nondeterministic (run1=$canon_b run$i=$c)"
            diverge=1; break
        fi
    done
    [ $diverge -ne 0 ] && { fail=1; continue; }

    if [ "$canon_a" = "$canon_b" ]; then
        echo "PASS $name  $canon_a  (3x deterministic, bob==mig)"
        continue
    fi

    # divergence: report first differing tick+field pair
    first=$(diff --unchanged-line-format='' --old-line-format='B %L' \
            --new-line-format='M %L' \
            <(tail -n +2 "/tmp/canon_bob_$name.out") \
            <(tail -n +2 "/tmp/canon_mig_$name.out") | grep -m2 -n "." || true)
    n_diff=$(diff <(tail -n +2 "/tmp/canon_bob_$name.out") \
                  <(tail -n +2 "/tmp/canon_mig_$name.out") | grep -c "^<" )
    echo "DIFF $name  bob=$canon_a mig=$canon_b  ($n_diff records differ)"
    echo "$first" | head -4 | sed 's/^/    /'
    fail=1
done
exit $fail

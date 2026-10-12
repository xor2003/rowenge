#!/usr/bin/env bash
# T004 — four-way provenance comparison.
# mig_orig (MIG_001.ZIP) -> mig_patched (migalley tree)
# bob_orig (bob_src tree) -> bob_patched (bob-flight-sim tree)
# plus mig_patched <-> bob_patched shared-path census.
#
# Emits, all stable-sorted:
#   migration/file-manifest-<tree>.tsv   path \t size \t raw_sha256 \t norm_sha256
#   migration/mig-orig-to-ported.tsv     path \t status \t orig_raw \t new_raw
#   migration/bob-orig-to-ported.tsv     idem
#   migration/mig-vs-bob-paths.tsv       path \t mig_raw \t bob_raw \t equal_norm
#   migration/routine-map.tsv            file-level candidate skeleton (UNKNOWN)
set -euo pipefail
REPO="$(cd "$(dirname "$0")/../../.."; pwd)"
ARCHIVE="${ROWAN_ARCHIVE:-/home/xor/games/bob}"
cd "$REPO"
OUT=engine/migration
CACHE=engine/.cache
MIGZIP="$ARCHIVE/mig-alley/source-original/MIG_001.ZIP"
MIG_ORIG="$CACHE/mig_orig"
mkdir -p "$OUT" "$CACHE"

# --- extract MiG original once ---------------------------------------------
if [ ! -d "$MIG_ORIG" ]; then
  mkdir -p "$MIG_ORIG"
  (cd "$MIG_ORIG" && unzip -qq "$OLDPWD/$MIGZIP")
fi

# .g/.m are generated/legacy include files (uidvals.g, enumbits.m) — treat as text.
TEXT_RE='\.(c|cc|cpp|cxx|h|hpp|hh|inl|asm|inc|s|g|m|txt|md|rc|def|dsp|dsw|mak|mk|in|cmake|sh|bash|json|xml|py|pl|bat|cmd|ini|cfg|spec|tsv|csv|yml|yaml|vsprops|vcxproj|filters|sln|res|bfi|stt|pif|dir|reg|doc|htm|html)$'

norm_sha() { # text => LF-normalized hash; binary => raw hash
  case "$1" in
    *$'\n'*) ;; # no-op
  esac
  if echo "$1" | grep -qiE "$TEXT_RE"; then
    tr -d '\r' < "$1" | sha256sum | awk '{print $1}'
  else
    sha256sum "$1" | awk '{print $1}'
  fi
}

manifest() { # $1=root $2=out — find files, emit tsv (lowercased relpath)
  local root="$1" out="$2" f rel raw norm size
  : > "$out"
  (cd "$root" && find . -type f ! -path './.git/*' ! -name '.git*' | sort -f) |
  while IFS= read -r f; do
    rel="${f#./}"
    [ -f "$root/$rel" ] || continue
    raw=$(sha256sum "$root/$rel" | awk '{print $1}')
    norm=$(norm_sha "$root/$rel")
    size=$(stat -c %s "$root/$rel")
    printf '%s\t%s\t%s\t%s\n' "${rel,,}" "$size" "$raw" "$norm" >> "$out"
  done
  sort -f -o "$out" "$out"
}

echo "== manifests"
manifest "$MIG_ORIG" "$OUT/file-manifest-mig_orig.tsv" &
manifest migalley "$OUT/file-manifest-mig_patched.tsv" &
manifest "$ARCHIVE/battle-of-britain/source-original/bob_src/SRC" "$OUT/file-manifest-bob_orig.tsv" &
manifest bob "$OUT/file-manifest-bob_patched.tsv" &
wait

# strip zip prefix: mig_orig files live under SRC/ — normalize to relative root
sed -i 's|^src/||' "$OUT/file-manifest-mig_orig.tsv"
sed -i '/^bob_src\//d;/^\.git/d' "$OUT/file-manifest-bob_orig.tsv"

delta() { # $1=orig.tsv $2=patched.tsv $3=out
  awk -F'\t' '
    NR==FNR { o[$1]=$3; on[$1]=$4; next }
    { p[$1]=$3; pn[$1]=$4; }
    END {
      for (k in o) {
        st = (k in p) ? ((o[k]==p[k] || on[k]==pn[k]) ? "IDENTICAL" : "MODIFIED") : "REMOVED"
        printf "%s\t%s\t%s\t%s\n", k, st, o[k], (k in p ? p[k] : "-")
      }
      for (k in p) if (!(k in o)) printf "%s\tADDED\t-\t%s\n", k, p[k]
    }' "$1" "$2" | sort -f > "$3"
}

echo "== deltas"
delta "$OUT/file-manifest-mig_orig.tsv" "$OUT/file-manifest-mig_patched.tsv" "$OUT/mig-orig-to-ported.tsv" &
delta "$OUT/file-manifest-bob_orig.tsv" "$OUT/file-manifest-bob_patched.tsv" "$OUT/bob-orig-to-ported.tsv" &
wait

echo "== mig vs bob path census"
LC_ALL=C join -t $'\t' -j 1 \
  <(LC_ALL=C sort "$OUT/file-manifest-mig_patched.tsv") \
  <(LC_ALL=C sort "$OUT/file-manifest-bob_patched.tsv") |
awk -F'\t' '{printf "%s\t%s\t%s\t%s\n", $1, $3, $6, ($4==$7 ? "YES":"no")}' |
LC_ALL=C sort > "$OUT/mig-vs-bob-paths.tsv"

echo "== routine-map skeleton (file-level candidates, UNKNOWN pending symbol pass)"
{
  printf 'candidate_id\tmig_path\tbob_path\tmig_raw\tbob_raw\tequal_norm\tclassification\tselected_impl\tdecision_status\n'
  awk -F'\t' '{printf "F%05d\t%s\t%s\t%s\t%s\t%s\tUNKNOWN\t-\tOPEN\n", NR, $1, $1, $2, $3, $4}' \
    "$OUT/mig-vs-bob-paths.tsv"
} > "$OUT/routine-map.tsv"

wc -l "$OUT"/*.tsv | sed 's|'"$OUT"'/||'
for f in mig-orig-to-ported bob-orig-to-ported; do
  echo "-- $f:"; cut -f2 "$OUT/$f.tsv" | sort | uniq -c
done
echo "-- mig-vs-bob equal_norm:"; cut -f4 "$OUT/mig-vs-bob-paths.tsv" | sort | uniq -c

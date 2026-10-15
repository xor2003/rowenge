#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Regenerate migalley/scripts/MIG_RC_ru.json from MIG_RC.json + the Russian
# translation tables (mig_ru_strings.py for UI/radio/narrative text,
# mig_ru_places.py for the Korean place-name transliteration used by the
# rail-line / highway / airfield / bridge / supply-site compound names).
#
# String IDs that have no translation fall back to the English text, so the
# generated file is always loadable even if the tables are incomplete.
import json, copy, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

from mig_ru_strings import RU          # {int id: russian text}
from mig_ru_places import translate_place  # Korean place-name transliterator

def main():
    src_path = os.path.join(HERE, "MIG_RC.json")
    out_path = os.path.join(HERE, "MIG_RC_ru.json")
    src = json.load(open(src_path, encoding="utf-8"))
    out = copy.deepcopy(src)

    table = dict(RU)
    # Auto-fill place/geometry compound names not already hand-translated.
    for t in src.get("stringtables", []):
        for e in t.get("entries", []):
            i, s = e.get("id"), e.get("text")
            if isinstance(i, int) and isinstance(s, str) and i not in table:
                tr = translate_place(s.strip())
                if tr != s.strip() and any(0x400 <= ord(c) <= 0x4FF for c in tr):
                    lead = s[: len(s) - len(s.lstrip())]
                    trail = s[len(s.rstrip()):]
                    table[i] = lead + tr + trail

    translated = 0
    for t in out.get("stringtables", []):
        for e in t.get("entries", []):
            i = e.get("id")
            if isinstance(i, int) and i in table and table[i] is not None:
                e["text"] = table[i]
                translated += 1

    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(out, f, ensure_ascii=False, indent=1)
    print("wrote %s: %d translated entries" % (out_path, translated))

if __name__ == "__main__":
    main()

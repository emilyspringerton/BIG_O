#!/usr/bin/env bash
# Regenerate the checked-in C rules from the canonical PARENA source, then the parity vectors.
# (.prn lives in the PARENA repo; generated copies are checked in here: no live cross-repo build dependency.)
set -euo pipefail
cd "$(dirname "$0")/.."
PARENA_BIN="${PARENA_BIN:-/home/fatbaby/PARENA/parena}"
PRN_DIR="${PARENA_PRN_DIR:-/home/fatbaby/PARENA/stdlib/big_o}"
PRN="$PRN_DIR/witness_rules.prn"
"$PARENA_BIN" build "$PRN" -o core/witness_rules.c
"$PARENA_BIN" build "$PRN_DIR/world_rules.prn" -o core/world_rules.c
"$PARENA_BIN" build "$PRN_DIR/world_alerts_mod.prn" -o core/world_alerts.c
"$PARENA_BIN" build "$PRN_DIR/giant_bug_brain.prn" -o core/giant_bug_brain.c
gcc -std=c99 -Wall -Wextra -Icore -Icore/runtime -DPARENA_NO_GRAPHICS tests/gen_vectors.c core/witness_rules.c -o /tmp/bigo_gen_vectors
/tmp/bigo_gen_vectors > tests/parity_vectors.txt
echo "regenerated core/witness_rules.c and tests/parity_vectors.txt ($(wc -l < tests/parity_vectors.txt) vectors)"

# day/ side (live-multiplayer primitives, separate from core/'s headless scenario-sim rules above).
"$PARENA_BIN" build "$PRN_DIR/walkie_rules.prn" -o day/packages/simulation/walkie_rules.c
echo "regenerated day/packages/simulation/walkie_rules.c"
"$PARENA_BIN" build "$PRN_DIR/item_drop_mod.prn" -o day/packages/simulation/item_drop_mod.c
"$PARENA_BIN" build "$PRN_DIR/inventory_mod.prn" -o day/packages/simulation/inventory_mod.c
echo "regenerated day/packages/simulation/item_drop_mod.c, inventory_mod.c (big_o's own forked mods, not papercraft's shared ones)"
"$PARENA_BIN" build "$PRN_DIR/party_rules.prn" -o day/packages/simulation/party_rules.c
echo "regenerated day/packages/simulation/party_rules.c"
"$PARENA_BIN" build "$PRN_DIR/chat_rules.prn" -o day/packages/simulation/chat_rules.c
echo "regenerated day/packages/simulation/chat_rules.c"
"$PARENA_BIN" build "$PRN_DIR/hoverboard_rules.prn" -o day/packages/simulation/hoverboard_rules.c
echo "regenerated day/packages/simulation/hoverboard_rules.c"
"$PARENA_BIN" build "$PRN_DIR/walkie_callsign.prn" -o day/packages/simulation/walkie_callsign.c
echo "regenerated day/packages/simulation/walkie_callsign.c"

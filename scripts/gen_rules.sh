#!/usr/bin/env bash
# Regenerate the checked-in C rules from the canonical PARENA source, then the parity vectors.
# (.prn lives in the PARENA repo; generated copies are checked in here: no live cross-repo build dependency.)
set -euo pipefail
cd "$(dirname "$0")/.."
PARENA_BIN="${PARENA_BIN:-/home/fatbaby/PARENA/parena}"
PRN="${PARENA_PRN:-/home/fatbaby/PARENA/stdlib/big_o/witness_rules.prn}"
"$PARENA_BIN" build "$PRN" -o core/witness_rules.c
gcc -std=c99 -Wall -Wextra -Icore -Icore/runtime -DPARENA_NO_GRAPHICS tests/gen_vectors.c core/witness_rules.c -o /tmp/bigo_gen_vectors
/tmp/bigo_gen_vectors > tests/parity_vectors.txt
echo "regenerated core/witness_rules.c and tests/parity_vectors.txt ($(wc -l < tests/parity_vectors.txt) vectors)"

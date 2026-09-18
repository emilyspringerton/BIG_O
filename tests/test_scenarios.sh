#!/usr/bin/env bash
# Runs every scenario (each carries `expect` assertions), checks byte-identical replay (same seed+script -> same log),
# checks that seeding actually matters, and that a bad script fails. Usage: tests/test_scenarios.sh <bigo_sim binary>
set -uo pipefail
cd "$(dirname "$0")/.."
SIM="${1:-build/bigo_sim}"
fail=0; n=0
for f in scenarios/*.txt; do
  n=$((n+1))
  if ! "$SIM" run "$f" > /tmp/bigo_a.$$ 2>&1; then echo "FAIL scenario $f"; tail -5 /tmp/bigo_a.$$; fail=$((fail+1)); continue; fi
  "$SIM" run "$f" > /tmp/bigo_b.$$ 2>&1
  if ! cmp -s /tmp/bigo_a.$$ /tmp/bigo_b.$$; then echo "FAIL nondeterministic replay: $f"; fail=$((fail+1)); fi
done
# seed sensitivity: the roll-driven scenario must differ under another seed
sed 's/^seed 1$/seed 2/' scenarios/11_seeded_noticing.txt > /tmp/bigo_s2.$$.txt
"$SIM" run scenarios/11_seeded_noticing.txt > /tmp/bigo_a.$$ 2>&1
"$SIM" run /tmp/bigo_s2.$$.txt > /tmp/bigo_b.$$ 2>&1
if cmp -s <(grep -v '^RESULT\|^> seed' /tmp/bigo_a.$$) <(grep -v '^RESULT\|^> seed' /tmp/bigo_b.$$); then echo "FAIL seed has no effect on noticing"; fail=$((fail+1)); fi
# a script with a failing expectation, an empty script, and an unknown command must all exit nonzero
printf 'seed 1\ncrew 1\nnpc public 0 50\nrelease 0 0\nexpect npc 0 state SILENCING\n' > /tmp/bigo_bad1.$$.txt
printf 'seed 1\ncrew 1\n' > /tmp/bigo_bad2.$$.txt
printf 'seed 1\ncrew 1\nfrobnicate 3\nexpect decorum 0 == 80\n' > /tmp/bigo_bad3.$$.txt
for b in /tmp/bigo_bad1.$$.txt /tmp/bigo_bad2.$$.txt /tmp/bigo_bad3.$$.txt; do
  if "$SIM" run "$b" > /dev/null 2>&1; then echo "FAIL bad script accepted: $b"; fail=$((fail+1)); fi
done
rm -f /tmp/bigo_*.$$ /tmp/bigo_*.$$.txt
if [ "$fail" -ne 0 ]; then echo "SCENARIOS FAILED ($fail)"; exit 1; fi
echo "SCENARIOS OK: $n scenarios + replay determinism + seed sensitivity + bad-script rejection"

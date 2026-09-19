#!/usr/bin/env bash
# Clean build + test. Usage: scripts/build.sh [--windows]
set -euo pipefail
cd "$(dirname "$0")/.."
VERSION="${BIGO_VERSION:-0.0.0-dev}"
CFLAGS_BASE="-std=c99 -Wall -Wextra -Werror -Icore -Icore/runtime -DPARENA_NO_GRAPHICS -DBIGO_VERSION=\"$VERSION\""
rm -rf build && mkdir -p build
echo "== C: witness rules tests (ASan+UBSan) =="
gcc $CFLAGS_BASE -g -fsanitize=address,undefined -fno-sanitize-recover=all \
    tests/test_witness_rules.c core/witness_rules.c -o build/test_witness_rules
./build/test_witness_rules tests/parity_vectors.txt
BUILD_SIM=0; [ -f core/sim.c ] && BUILD_SIM=1
if [ "$BUILD_SIM" = 1 ]; then
  echo "== C: crew sim + scenarios (ASan+UBSan) =="
  gcc $CFLAGS_BASE -g -fsanitize=address,undefined -fno-sanitize-recover=all \
      apps/bigo_sim/main.c core/sim.c core/mission.c core/witness_rules.c -o build/bigo_sim_asan
  gcc $CFLAGS_BASE -O2 apps/bigo_sim/main.c core/sim.c core/mission.c core/witness_rules.c -o build/bigo_sim
  ./build/bigo_sim --version
  bash tests/test_scenarios.sh build/bigo_sim_asan
fi
if [[ " $* " == *" --windows "* ]]; then
  echo "== Windows cross-build (mingw) =="
  if [ "$BUILD_SIM" = 1 ]; then
    x86_64-w64-mingw32-gcc $CFLAGS_BASE -O2 apps/bigo_sim/main.c core/sim.c core/mission.c core/witness_rules.c -o build/bigo_sim.exe
    file build/bigo_sim.exe | grep -q PE32
  else
    x86_64-w64-mingw32-gcc $CFLAGS_BASE -O2 -c core/witness_rules.c -o build/witness_rules_win.o
  fi
fi
echo "BUILD CLEAN"

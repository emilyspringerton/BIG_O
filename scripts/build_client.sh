#!/usr/bin/env bash
# Build the day-sandbox client (forked from PAPERCRAFT). Usage: scripts/build_client.sh [--windows SDL2_MINGW_DIR]
# Linux needs libsdl2-dev libgl-dev libglu1-mesa-dev. Windows: mingw-w64 + SDL2 2.30.10 devel (x86_64-w64-mingw32 dir).
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
SRC="day/apps/client/src/main.c day/packages/simulation/paper_fragment_mod.c day/packages/simulation/parena_runtime.c"
WORLD_OBJS="core/world.c core/world_rules.c core/world_alerts.c core/reflux_runtime.c core/sim.c core/witness_rules.c"
INC="-Iday/packages/common -Iday/packages/simulation"
# The world sim (core/) is compiled separately: it has its own PARENA runtime include dir that must not shadow the client's.
mkdir -p build/coreobj
core_objs() {  # $1 = compiler
  for f in $WORLD_OBJS; do "$1" -std=c99 -O2 -DPARENA_NO_GRAPHICS -Icore -Icore/runtime -c "$f" -o "build/coreobj/$(basename "$f" .c).o"; done
}
if [ "${1:-}" = "--windows" ]; then
  S="${2:?SDL2 mingw dir}"
  core_objs x86_64-w64-mingw32-gcc
  x86_64-w64-mingw32-gcc -std=gnu11 -O2 $SRC build/coreobj/*.o $INC -I"$S/include" -L"$S/lib" -o build/bigo_client.exe \
    -lmingw32 -lSDL2main -lSDL2 -lopengl32 -lglu32 -lws2_32 -lm
  x86_64-w64-mingw32-gcc -std=gnu11 -O2 day/apps/mapeditor/src/main.c -Iday/packages/common -o build/mapeditor.exe -lws2_32 -lm
  rm -rf build/coreobj
else
  core_objs gcc
  gcc -std=gnu11 -O2 $SRC build/coreobj/*.o $INC -o build/bigo_client_linux -lSDL2 -lGL -lGLU -lm
  gcc -std=gnu11 -O2 day/apps/mapeditor/src/main.c -Iday/packages/common -o build/mapeditor_linux -lm
  rm -rf build/coreobj
fi
echo "CLIENT BUILD CLEAN"

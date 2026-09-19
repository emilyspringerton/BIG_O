#!/usr/bin/env bash
# Build the day-sandbox client (forked from PAPERCRAFT). Usage: scripts/build_client.sh [--windows SDL2_MINGW_DIR]
# Linux needs libsdl2-dev libgl-dev libglu1-mesa-dev. Windows: mingw-w64 + SDL2 2.30.10 devel (x86_64-w64-mingw32 dir).
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
SRC="day/apps/client/src/main.c day/packages/simulation/paper_fragment_mod.c day/packages/simulation/parena_runtime.c"
INC="-Iday/packages/common -Iday/packages/simulation"
if [ "${1:-}" = "--windows" ]; then
  S="${2:?SDL2 mingw dir}"
  x86_64-w64-mingw32-gcc -std=gnu11 -O2 $SRC $INC -I"$S/include" -L"$S/lib" -o build/bigo_client.exe \
    -lmingw32 -lSDL2main -lSDL2 -lopengl32 -lglu32 -lws2_32 -lm
  x86_64-w64-mingw32-gcc -std=gnu11 -O2 day/apps/mapeditor/src/main.c -Iday/packages/common -o build/mapeditor.exe -lws2_32 -lm
else
  gcc -std=gnu11 -O2 $SRC $INC -o build/bigo_client_linux -lSDL2 -lGL -lGLU -lm
  gcc -std=gnu11 -O2 day/apps/mapeditor/src/main.c -Iday/packages/common -o build/mapeditor_linux -lm
fi
echo "CLIENT BUILD CLEAN"

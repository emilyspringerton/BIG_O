#!/usr/bin/env bash
# Build the day-sandbox server (forked from PAPERCRAFT, S504-10). No Bazel needed.
set -euo pipefail
cd "$(dirname "$0")/../day"
mkdir -p ../build
gcc -std=gnu11 -O2 -Wall -Wno-unused-function -Wno-comment -Ipackages/common -Ipackages/simulation \
  apps/server/src/main.c packages/simulation/*_mod.c packages/simulation/parena_runtime.c \
  -o ../build/bigo_day_server -lm -ldl
echo "DAY BUILD CLEAN"

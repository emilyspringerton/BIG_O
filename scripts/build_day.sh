#!/usr/bin/env bash
# Build the day-sandbox server (forked from PAPERCRAFT, S504-10). No Bazel needed.
set -euo pipefail
cd "$(dirname "$0")/../day"
mkdir -p ../build
gcc -std=gnu11 -O2 -Wall -Wno-unused-function -Wno-comment -Ipackages/common -Ipackages/simulation -I../core -I../core/runtime \
  apps/server/src/main.c packages/simulation/*_mod.c packages/simulation/parena_runtime.c \
  packages/simulation/party_rules.c \
  packages/simulation/chat_rules.c \
  packages/simulation/hoverboard_rules.c \
  packages/simulation/walkie_rules.c \
  packages/simulation/walkie_callsign.c \
  ../core/npc_archetype.c ../core/zombie_values.c ../core/humanness.c ../core/witness_rules.c \
  ../core/giant_bug_values.c ../core/giant_bug_brain.c \
  -o ../build/bigo_day_server -lm -ldl -lpthread
echo "DAY BUILD CLEAN"

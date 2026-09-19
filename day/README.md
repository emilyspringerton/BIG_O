# day/ — BIG_O day-loop sandbox (fork of PAPERCRAFT, S504-10)

Forked 2026-09-19 from `PAPERCRAFT` (server, `packages/common`, PARENA-compiled `packages/simulation` mods) as the base for
the day-cycle zombie sandbox. Build: `scripts/build_day.sh` (gcc, no Bazel). Status: **builds; not yet playable as BIG_O**.
Still PAPERCRAFT-shaped: loads the GFD urban chunk grid and verifies IDUNA connect tickets. Next: load SHANKPIT's `nextown`
level + `AI_ROLE_RELENTLESS_PURSUER` zombies, then reconcile engine deltas with SHANKPIT in both directions.

Bazel: `bazel test //:all` (rules core) and `bazel test --config=day //day/...` (21 tests) and
`bazel build --config=day //day/apps/server:papercraft_server`. All verified passing 2026-09-19.

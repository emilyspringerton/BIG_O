# day/ — BIG_O day-loop sandbox (fork of PAPERCRAFT, S504-10)

Forked 2026-09-19 from `PAPERCRAFT` (server, `packages/common`, PARENA-compiled `packages/simulation` mods) as the base for
the day-cycle zombie sandbox. Build: `scripts/build_day.sh` (gcc, no Bazel). Status: **builds; not yet playable as BIG_O**.
Still PAPERCRAFT-shaped: loads the GFD urban chunk grid and verifies IDUNA connect tickets. Next: load SHANKPIT's `nextown`
level + `AI_ROLE_RELENTLESS_PURSUER` zombies, then reconcile engine deltas with SHANKPIT in both directions.

Bazel: `bazel test //:all` (rules core) and `bazel test --config=day //day/...` (21 tests) and
`bazel build --config=day //day/apps/server:papercraft_server`. All verified passing 2026-09-19.

## Zombie animations (mannequin)
Only one model exists (UAL1 mannequin), so zombie motion is derived from its own idle/walk mocap:
`python3 tools/gen_zombie_clips.py ../SHANKPIT/assets/goldenband day/assets/goldenband` writes `zombie_{idle,walk,attack,death}`
(.gband + manifest, skeleton hash matches `mannequin_npc.gskel`; `gbtool validate` OK). walk = 1.6x-slowed shamble, arms out,
forward lean, head tilt; idle = arms hanging forward-low; attack = 36-tick arms-up-then-slam (one-shot); death = 45-tick backward
topple, holds lying. Checked numerically (arm/torso directions), **not yet seen rendered**. Not yet wired into the SHANKPIT
NPC kit loader (`gband_skel_npc_load_kit` only has idle/walk/greet/dance slots — attack/death need slots or a new API).

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

## Client + bundling (same pattern as SHANKPIT/PAPERCRAFT)
`day/apps/client` (SDL2/OpenGL, forked from PAPERCRAFT) and `day/apps/mapeditor` build via `scripts/build_client.sh`
(Linux: system SDL2) or `scripts/build_client.sh --windows <SDL2-2.30.10 mingw dir>`. CI (`client_linux`, `client_windows`)
bundles `BIG_O_Linux/` (client + PLAY.sh) and `BIG_O_Windows/` (client + `SDL2.dll` + PLAY.bat) and attaches
`bigo-linux-x86_64.tar.gz` / `bigo-windows-x86_64.zip` to each release. Both compile locally; the CI jobs are untested until pushed.
The client is still the PAPERCRAFT client (city/worldapi + IDUNA login); `bigo_sim.exe` is only the headless rules text-sim.

## The phone: every menu goes through it (S504-10)
Source spec found at `TYLER/engine/tyler_phone_mechanics.md` (Messages, Contacts, Map, Camera, Notes; banner anti-spam
"max 2 per 30s, extras batched"). PAPERCRAFT had only the banner; BIG_O has the browsable phone. `F` toggles it, `I` jumps to
Cargo, arrows/WASD navigate, Enter selects, Esc backs out (Esc quits only when the phone is closed); controller: Y toggle, D-pad, A/B.
Movement is suppressed while it is open. Logic is `day/packages/common/bigo_phone.h` (pure, unit-tested: `bigo_phone_test`, Bazel);
rendering is `draw_bigo_phone` in the client; `day/tools/phone_preview.c` renders every screen under Xvfb for visual checks.

| App | State |
|---|---|
| Messages, Notes | real (message log fed by server phone packets; notes read-only) |
| Cargo, Skills, Loadout | real: wired to the server's inventory, talent allocation, and weapon switch |
| Contacts, Map, Camera, Wardrobe | local UI only: replies/pins/photos/costume are client-side, no server effect yet; camera takes no screenshot |
| Lab | UI + splice logic work; sample counts are 0 until harvesting exists, so SPLICE is inert in a real session |
| Status | shows level/XP/position; decorum/witness readouts NOT wired (rules core isn't linked into the client) |
Verified: unit test passes; client builds Linux+Windows; all 12 screens rendered and inspected. Not verified: live play against a server.

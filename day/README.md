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
| Messages, Notes | real: message log (server phone packets + world alerts + Dr. Thorne's A1M1 brief 8 s after connecting; Enter opens a message in full); notes read-only |
| Cargo, Skills, Loadout | real: wired to the server's inventory, talent allocation, and weapon switch |
| Map | shows live zombie count per area (`Z12`) from the world feed |
| Contacts, Camera, Wardrobe | local UI only (Thorne unlocks in Contacts when his brief arrives): replies/pins/photos/costume are client-side, no server effect yet; camera takes no screenshot |
| Lab | UI + splice logic work; sample counts are 0 until harvesting exists, so SPLICE is inert in a real session |
| Status | level/XP/position + world clock/weather/outdoor sight; decorum/witness readouts NOT wired |
Verified: unit test passes; client builds Linux+Windows; all 12 screens rendered and inspected. Not verified: live play against a server.

**World feed (interim):** the client links `core/world.c` (PARENA rules + REFLUX, see `docs/B2_WORLD.md`) and runs it LOCALLY
(1 real second = 1 game minute), pushing clock, weather, zombie counts and alert messages into the phone. It is not
server-authoritative and not shared between players; it exists so the phone shows real world data until the server owns the world.
Header, Map and Status read it; nothing in the 3D scene reacts to it yet (no sky, weather, or zombies rendered).

## Sky and weather (configurable skybox)
`day/packages/common/bigo_sky.h` renders a procedural sky from the world clock + weather: time-of-day gradient with sunrise/sunset
glow, sun and moon with halos, twinkling stars, drifting cloud puffs, fog, rain streaks, storm gloom and lightning. It needs no
assets. Everything visual is data: `bigo_skycfg.h` holds palettes (day/golden/dawn/twilight/night), sun/moon/star/cloud settings and
**one profile per weather** (cover, rain, storm, darkness, grey, fog, cloud tint, grey tone); the sky eases toward the current
weather's profile. Restyle by editing a text file: copy `day/assets/skybox_default.cfg` (every key, with defaults) to
`assets/skybox.cfg` next to the client (or set `BIGO_SKYBOX=path`); **F9 reloads it live**. A bad file is rejected whole with its line
number. `assets/skybox_toxic.cfg` is a worked example (green wasteland sky). Verified: config parser/sanitizer unit test
(`bigo_skycfg_test`), and `day/tools/sky_preview.c` renders 13 time/weather shots under Xvfb (inspected). The client integration
builds on Linux and Windows but has not been run in a live session (needs a server). Sky only: world geometry is not lit by the sun.

## Launching (why "it opens then closes")
The client is still PAPERCRAFT's: at startup it fetches the city from a worldapi, logs in via IDUNA, and connects to a game server.
With no host flags it looks on the player's own machine (`localhost:7070`), so a bare launch dies instantly with `FATAL ... worldapi
localhost:7070 ... WSA error 10061 (refused)`. `PLAY.bat` / `PLAY.sh` now pass the public hosts (okemily.com: worldapi 7070, IDUNA 8080,
game UDP 7799 -- the live PAPERCRAFT servers), and the Windows launcher keeps its window open (`pause`) so errors stay readable.
`PLAY_LOCAL.*` is the bare launch for a self-hosted stack. BIG_O has no server of its own yet, so online play means being in
PAPERCRAFT's world with BIG_O's phone/sky on top. A true offline mode (no servers) is not built.

## Snapshot compression (LZ4) -- why, and how to rip it out
Founder on a phone modem saw "weak connection (170s)" and the level appearing minutes late (2026-09-19). The server's snapshot is a fixed
1436-byte struct (1464 on the wire) that is almost all zeros (16 player slots + fixed world arrays). That exceeds the 1280-1428 byte path MTU
common on mobile carriers, so it fragments, and carriers drop fragments while small packets (WELCOME, USERCMD) still pass.
`lz4mini.h` (standard LZ4 block format, tested incl. 200k hostile-input rounds under ASan/UBSan) compresses it to ~190 bytes. Clients advertise
`PC_CAP_LZ4` in one extra byte after CONNECT; old clients/servers interoperate (plain snapshots). Verified against a scratch server: LZ4 client gets
~190-byte snapshots, no-capability and old clients get plain ones, all decode. **Not yet verified on a real mobile link.**
Rip out: client `--no-lz4`, server env `PAPERCRAFT_NO_LZ4=1` (no rebuild), or delete `lz4mini.h` + the three marked blocks
(`PC_PACKET_SNAPSHOT_LZ4` in protocol/server/client).

## Network diagnosis log (mobile play) and the LZ4 decision
1. MTU/fragmentation hypothesis -> LZ4 (above). Player reported it did not fix the problem.
2. Server log showed a mobile player frozen at spawn for minutes, with no telemetry to say why. Reproduced on a scratch server (`natprobe`): after a
   source-port change (normal on cellular NAT) the server keeps streaming snapshots to the OLD port and silently ignores input from the new one, so the
   client sees no snapshots ("weak connection 170s") and cannot move until its 55s full reconnect. **Fix:** soft re-hello -- client re-sends CONNECT every
   2s after 4s of silence; the server's existing CONNECT path reclaims the slot by player_id and updates the address. Client mints a fresh ticket in the
   background every ~2.5 min so the ticket it re-sends is always valid.
3. Telemetry: client prints `[net] 5s: snapshots=.. (lz4=..) avg=..B maxgap=..ms silence=..ms soft_rehellos=..`; server prints per-slot `[net] slot N ip:port lz4=..
   usercmds_rx=.. snaps_tx=.. cmd_age_ms=..` every 10s and logs "USERCMD ignored ... NAT port change" when input arrives from an unexpected port.
**LZ4 keep/remove:** unproven either way. It is harmless (~190 B/snapshot, negotiated, `--no-lz4` / `PAPERCRAFT_NO_LZ4=1` to disable) and removes a plausible
fragmentation risk. Decide from a real session's `[net]` lines: if `avg` is ~190B with `lz4>0` and problems persist with soft_rehellos near zero, LZ4 was not the
issue and can go; if `soft_rehellos` fire and recover, the NAT fix is what mattered.

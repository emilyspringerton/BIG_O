# BIG_O — SHIP PLAN (end to end, in SHANKPIT) — 2026-10-01

Founder real-time, 2026-10-01 (in order): *"fully ship BIG_O all of it the whole thing make it work in
shankpit levels however the fuck you need to - the whole thing needs to work end to end"* · *"use rigidbody
ragdoll physics"* · *"and the models and animations available"* · *"be creative"* · *"you have full agency"* ·
*"assume this is for a DOD contract so try hard"* · *"use noc texture gen to enhance world gen"* ·
*"build all deps in PARENA first"* · *"plan it out then execute with many agents"*.

## 0. What "shipped" means here (the bar — stated up front so it can be checked, not argued)

BIG_O's own V0 bar (NORTHSTAR §4/§5-B6) closed inside SHANKPIT's engine (BIG_O tech is already first-class
SHANKPIT tech per `SHANKPIT/docs2/specs/BIGO_ENGINE_MERGE_NORTHSTAR.md`; `MODE_STORY` plays BIG_O):

> One player completes **DAY (harvest) → NIGHT (cover) → LAB (splice) → WAR (shadow war vs a bot) → DAWN**, with state
> carried across the whole turn, on real SHANKPIT levels, rendered with the real GOLDENBAND models/animations,
> with rigid-body ragdolls on deaths, world surfaces textured from NOCK-generated PARENA textures, and a
> **headless scripted end-to-end playthrough that proves the turn closes** plus live Xvfb screenshots of every
> stage. Everything below the "decision" line is PARENA first.

Not claimed (stays named, not silently dropped — NORTHSTAR §4 "Deferred"): adversarial corporations, PvP shadow war
between human crews, season-lineage persistence, mobile lab client, full pheromone-field simulation, story chapters.
Co-op (≤3) is exercised only as far as the existing server/netcode already supports it; a 2-client live run is a
stretch item, reported honestly either way.

## 1. Standing rules every work package obeys

- **Core deps are PARENA-first** (standing rule): rules/data/decision logic and any new engine dependency is written in
  PARENA (`PARENA/stdlib/big_o/*.prn`), compiled to C, tested headless; host C only owns clocks, I/O, GL. If PARENA
  cannot express something, fixing PARENA (additively, with tests) is part of the package.
- Strict flags for all tests: `-std=c99 -Wall -Wextra -pedantic -Werror` (PARENA convention). Deterministic — no wall
  clock, no `rand()`; seeded PRNG only. Bounds-checked inputs everywhere a packet or file can reach. ASan+UBSan clean.
- Never touch live shared services (no restarts/deploys, no broad `pkill` — exact PID only), never write the live IDUNA DB
  except through the named idempotent NOCK texture loader. Unique Xvfb display per agent.
- Parallel-safety: phase-1 packages **only create new files** (+ their own `mk/<pkg>.mk`), never edit shared files
  (`main.c`, `local_game.h`, `protocol.h`, `Makefile`, `CHANGELOG`, `BACKLOG`, `README`). Commit **specific paths** locally;
  the orchestrator pushes. Shared-file wiring is phase 2, serial.

## 2. Work packages

### Phase 1 — PARENA-first modules (parallel, new files only)

| Pkg | Owns | Deliverable |
|---|---|---|
| **W1 shadow_war** | the one genuinely new system | `PARENA/stdlib/big_o/shadow_war.prn`: deterministic batched army sim on a small grid; units = base(3)×trait(3)×quality; pheromone commands advance/hold/scatter; N-tick resolution; bot opponent policy; Elo update; integer-only; `SHANKPIT/packages/simulation/shadow_war_host.{c,h}` wrapper + tests (determinism, no-op symmetry, balance sanity) |
| **W2 campaign** | the turn spine | `campaign_rules.prn`: DAY→NIGHT→LAB→WAR→DAWN state machine, resource economy (samples, clones, bio-slurry), win/lose, flat-I32 save/restore; host wrapper + tests |
| **W3 harvest** | day + night rules | `harvest_rules.prn`: which vector yields which sample at what purity, sample time, extraction, evidence generation/contamination, night-hub disposal, witness implications; host wrapper + tests |
| **W4 ragdoll** | rigid-body ragdoll deps | `ragdoll_rules.prn` (impulse from weapon damage + hit direction, bone choice, settle/despawn timers) + `packages/simulation/ragdoll_pool.{c,h}` (bounded pool of the existing `RigidRagdoll`/GRB XPBD bodies; spawn-from-death, step, pose out) + tests incl. determinism and no-NaN |
| **W5 worldgen** | world generation | `worldgen.prn`: seeded procedural generator of SHANKPIT level JSON (wasteland harvest level w/ vector spawners + extraction zone; night hub: corporate block + park + zones + citizen spawners; basement lab) emitting the real `level_boxes` schema, level-chain exits; CLI + loader-verified output |
| **W6 worldtex** | NOCK texture gen for world gen | PARENA `gentexture` sources for wasteland sand/cracked earth, dry grass, asphalt, corporate carpet, office panel, lab tile, rust metal (seed/param variants), loaded into the NOCK DB through the idempotent loader, exported + `packages/render/world_tex.{c,h}` (live NOCK fetch → embedded fallback) + material-name registry; test decodes every texture |

### Phase 2 — integration (serial; each integrator owns the shared files it touches)

- **I1 sim/server:** campaign state live in `ServerState`/`local_game.h`/server tick; harvest drops on vector kills; extraction →
  level transition; evidence; decorum/regulator carried through the turn; war request/result packets (bounds-checked);
  ragdoll pool fed from deaths; levels from W5 wired into the level chain; Makefile/Bazel entries for every new module.
- **I2 client:** render ragdolls via the GOLDENBAND skeleton; phone lab app → real splice/centrifuge loop → army; shadow-war
  screen (pheromone grid, replay, result, Elo); turn HUD; world textures (W6) on materials/terrain; models/animations for
  citizens/Men/zombies/Regulators.
- **I3 assemble & script:** headless scripted end-to-end playthrough (`scripts/bigo_e2e.sh`): server + scripted client plays
  one full turn and asserts every transition; Xvfb screenshot tour.

### Phase 3 — verification (parallel, adversarial)

Independent reviewers by lens: determinism/replay · packet/input robustness (bounds, malformed, ASan/UBSan, fuzz) · gameplay
loop closes (can the turn soft-lock?) · rendering/asset sanity · docs-vs-reality (README/NORTHSTAR claims). Findings fixed,
re-verified. Then docs: BIG_O README/NORTHSTAR, SHANKPIT README/CHANGELOG, golden-doc sync, BACKLOG + Apples.

## 3. Honest risks (named now)

1. The sandbox has software GL only (Xvfb) — screenshots are real but slow/low-fidelity; no GPU perf claims.
2. Phase-1 agents share working trees: mitigated by new-files-only + per-package `mk/*.mk`; a PARENA compiler change by
   one agent can transiently affect another's build (rule: additive, tested, called out).
3. "All of it" is a multi-year game (NORTHSTAR §3.2). This plan ships the *loop*, end to end, honestly bounded — the
   report at the end states exactly what is verified vs what is only compiled.

# BIG_O — "A SHANKPIT Story" — NORTHSTAR (scoping pass, 2026-09-18, S504)

Founder pitch (verbatim, real-time, observation Apple #20148): *BIG_O: A SHANKPIT Story is a hard sci-fi,
social-stealth management sandbox built on the SHANKPIT engine. By day, navigate a sun-drenched wasteland
overrun by feral biological vectors to harvest pristine, uncorrupted genetic sequencing data. By night, put on a
suit, wash off the blood, and blend into a hyper-paranoid corporate night-society where acknowledging the
apocalypse gets you canceled by the thought police. Build an off-the-books cloning facility in your basement lab,
weaponize the local fauna, and use tactical pheromone arrays to command a subterranean army of custom-spliced
vectors to wage an algorithmic shadow war. Just remember: if they don't see it, it isn't real.*

Per THE_EMILY_WAY Principle 19 this is a big, unscoped ask: this doc investigates what exists, cuts a real V0,
and hands phased sub-tasks back to `EMILY/BACKLOG.md` (SECTION 504) — it does not start building the whole thing.

## 1. The game as three loops on one clock

| Loop | Fantasy | Player verbs | Feels like |
|---|---|---|---|
| **DAY — Harvest** | wasteland expedition | move, shoot/evade, sample vectors, extract | SHANKPIT FPS (existing) |
| **NIGHT — Cover** | corporate night-society | blend in, talk, avoid attention, launder evidence | stealth/social sim (new) |
| **BASEMENT — Lab** | off-books cloning + shadow war | splice, breed, deploy, command via pheromones | management/tactics (new) |

The clock (day → night → dawn) is the game's pacing spine: what you harvest by day is spent in the lab, what
the lab makes must be hidden at night, and night behaviour (heat) changes what tomorrow's day costs.

**The signature mechanic — "if they don't see it, it isn't real" — is the Attention/Reality system.** Every
fact in the world (a corpse, a vector, a lab, a confession about the apocalypse) has a *witnessed* state. The
society treats unwitnessed things as nonexistent; witnessed-but-unacknowledged things raise **Heat**; heat past a
threshold = "cancelled" (the thought-police fail state). Players manage *who can see what*, not just what exists.
This is the one genuinely novel system; everything else is composition of things the engine already does.
It is specified as a pure, headless, PARENA-scalar **rules module first** (same discipline as DEADWEIGHT's
`card_rules.prn`: testable, deterministic, dual-emittable, no engine needed to prove it is fun).

## 2. What already exists (checked, not assumed — SHANKPIT CHANGELOG/docs read 2026-09-18)

- Server-authoritative UDP FPS, C/SDL2 client, Go/Dragonfly persistent-world backend, season lineage (`SHANKPIT/CLAUDE.md`).
- **Level chaining** (`next_level_id`, `is_story_start`, `LevelExit`, live level transitions, works in any mode),
  **doors** (scripted + built-in proximity), **REFLUX buttons/pub-sub** (S485), **NOCK-authored characters** that spawn
  outside story mode (S480), NOCK level/widget registry in IDUNA. → the day/night *level structure* is buildable now.
- `STORY_SYSTEM_NORTHSTAR` (scriptable objects/characters/triggers), `HUMANNESS_NORTHSTAR` (MISHRI-derived jitter/mood/
  perception NPC layer — **scoped, not built**), `AI-NAV`, `AI-SOLO`, `BOT_TRAINING`, `ANTICHEAT` northstars.
- Bots + PFSP league pipeline (BRAWLPIT/DEADWEIGHT) and a proven "same server binary, fast-forward, separate league" pattern.
- PARENA mod idiom (decision logic in `.prn`, host does the work), NOCK textures, IDUNA accounts/registries (game-scoped).

**Gaps this game needs that do not exist:** a Heat/witness (Attention) model; a **sight/perception** primitive
NPCs use to *witness* things (REFLUX `LOOK_AT`/`PROXIMITY` actions are reserved but never dispatched); the humanness NPC
layer for a believable night-society; a time-of-day cycle; inventory/lab/splice data model; a swarm-command layer
(pheromones); an offline/async shadow-war sim.

## 3. Critical read of the pitch (real fork points, recommended answers — veto any)

1. **Multiplayer-first is a standing rule for from-scratch projects; this pitch reads single-player.** Recommended
   answer: a *shared persistent world* (SHANKPIT's season-lineage backend) where each player runs their own
   basement cell. Day = optional co-op expeditions (existing FPS netcode). Night = shared social hub with players as
   *other cover identities* (a player can be witnessed by another player — real multiplayer social-stealth).
   Shadow war = **asynchronous, server-simulated, deterministic 1v1 between two players' vector armies**
   (turn-batched, fast-forwardable — this is DEADWEIGHT's server/bot/league shape again, on purpose).
   Bots from day one: society NPCs (humanness layer) + rival-cell bots that fight the shadow war.
2. **Scope is a multi-year game.** V0 must be a slice that proves the *loop closes*, not a feature list (see §4).
3. **"Genetic sequencing / cloning / vectors" stays at game-abstraction level** (data cards, trait sliders, fictional
   creatures) — no real pathogen or lab-protocol content; it is a sci-fi management skin, not a bio manual.
4. **Platform is undecided.** SHANKPIT is C/SDL2 desktop. Recommended: desktop first; the *lab + shadow war* layer is
   UI/menu-shaped and can later ship to Android over the same server (DEADWEIGHT precedent), the FPS day loop cannot.
5. **Tone risk:** satire of cancel culture must land as satire — the "canceled" state should be a mechanical
   consequence of *witnessed truth*, not a political statement. Worth a founder read of the Heat copywriting.

## 4. Recommended V0 ("one full day-night-lab turn, 2 players")

- 1 wasteland level (harvest 3 sample types from scripted vector fauna) → extract.
- 1 night hub level with ~6 humanness-lite NPCs (witness + gossip) and one thought-police NPC; Heat meter; win/lose
  on Heat; carry-in "evidence" from the day (blood/samples) that must be disposed before being witnessed.
- Lab as a **menu screen**: 3 base vectors × 3 splice traits from harvested samples (data cards, deterministic).
- Pheromone array v0: 3 discrete commands (advance/hold/scatter) on a small grid.
- Shadow war v0: two players' armies auto-resolve on the server over N batched ticks; bot opponent available; Elo.
- Accounts/tracking via IDUNA (game-scoped, guest accounts — DEADWEIGHT's work reused directly).

Deferred (named, not dropped): dynamic society factions, full pheromone field simulation, cloning economy depth,
basement base-building, story chapters via the level-chain engine, mobile lab client, tournament layers.

## 5. Phased plan (→ `EMILY/BACKLOG.md` SECTION 504)

- **B0 repo hygiene:** upstream repo (founder to create `BIG_O`), CLAUDE.md, golden-index (`BIG_O-NORTH`), root repo-table row.
- **B1 Attention/Heat rules module** (PARENA scalar, C+Java emit, headless tests + parity vectors + a tiny text sim to *play* it).
- **B2 Day slice:** harvest level + vector fauna on SHANKPIT (NOCK levels/characters, level-chain exit to night).
- **B3 Night slice:** hub level + witness/perception primitive (dispatch REFLUX LOOK_AT/PROXIMITY) + humanness-lite NPCs.
- **B4 Lab:** data model + menu UI + harvested-sample → splice loop.
- **B5 Shadow war:** deterministic batched sim, server, bot pool, league reuse (DEADWEIGHT/BRAWLPIT pipeline).
- **B6 Integration + V0 bar:** one player completes day → night → lab → war; clean builds, CI releases from first commit,
  live-verified match vs bot.

Open questions for the founder (only the ones that change the plan): (a) shared-world multiplayer per §3.1 —
yes? (b) desktop-first per §3.4 — yes? (c) new repo `BIG_O` upstream — you create it (I cannot; token is read-only).

## 6. Reconciliation with the founding design conversation (added 2026-09-18, after ingesting `docs/transcript/`)

Sections 1-5 above were written from the one-paragraph pitch alone. The founding Gemini conversation (now in `docs/`)
is far more specific, and it changes several things — corrected here rather than left to drift:

- **"Vectors" are zombies, and the Attention/Heat system already has a concrete spec.** Two coupled meters: a **Decorum**
  meter (blend-in / "heresy" for talking about the apocalypse, carrying field gear) and the **Witness rule** — 1 witness =
  catatonic denial, **5+ witnesses = aggressive silencing** (they try to kill you to clean the witness list); compromised
  witnesses become accomplices. B1 (the rules module) should implement exactly this, not an abstract "Heat".
- **The engine-shaped V0 the conversation itself proposes:** a GTA3-scale small city on existing SHANKPIT primitives, one
  *corporate office block with the park across the street*, a basement lab, dirt-vs-concrete **terrain affordance flags**
  (`MAT_DIRT` fast/hidden, `MAT_CONCRETE` breachable, `MAT_REINFORCED` blocked), zombie states
  (passive-heel / subterranean-swim / wall-breach / surface-surge) driven by **pheromone balls**. That replaces §4's generic
  day/night/lab slice as the concrete B2-B3 target.
- **Costumes and social engineering are first-class** (Hitman/Codename-47 pacing): uniform matrix (lab smock, janitor
  overalls), shoulder-surfing, tailgating, vigilance profiles per NPC. This maps onto the planned humanness NPC layer.
- **Lab UI = a UNIX-style bioinformatics terminal** (isolate / align / splice with contamination %, off-target and
  nonsense-mediated-decay risk). Cheap to build, on-brand, and the same idiom as PITVIPER/JEWEL terminals.
- **Story spine (spoilers, in `docs/DESIGN_DIGEST.md`):** three-faction war (feral / underground symbionts / polite society),
  the "evil corporation was the good guy" reveal, a hyper-intelligent avian faction as the Act II escalation, and a
  final binary choice. This is campaign content: **not V0**, chain it later with the level-chain engine.
- **The conversation never addresses multiplayer.** It designs a single-player sandbox. The house rule and §3.1's shared-world
  recommendation still stand and remain the open decision for the founder.
- **Science is flavor, not a lab manual.** Terms used (gRNA off-target, cryptic splice sites, nonsense-mediated decay,
  retrotransposons, HGT, epigenetic silencing) are game vocabulary; a few claims (e.g. stress hormones "methylating" a
  population's junk DNA) are narrative licence, and should stay that way. No real protocols belong in the game.

## 7. Decision: multiplayer (founder, 2026-09-18) — co-op, up to 3, one shared world

Founder: build multiplayer from day 1, up to 3 players, "totally fine having multiplayers running around causing mayhem";
adversarial play (each player working for a different corporation) is deferred because it needs up to three onboarding
variants — later it can be a mode with **no onboarding** (or faked), and V0 focuses on co-op. Adopted design:

- **One crew, one onboarding.** 1-3 players are one lab crew (same corporation, same basement). Solo play is simply a crew of 1
  (the game must be fully playable that way — BIG_O is *built for* one player, multiplayer is additive).
- **Shared world, server-authoritative** (SHANKPIT UDP server; 3 slots + NPCs/bots). Per-player state: costume, decorum, inventory.
  Crew state: the lab, samples, clones, unlocks, the crew's witness ledger.
- **Witness rule with a crew (B1 spec decision):** an *event* (a zombie released, a body seen) is attributed to one player or,
  if the crew is seen acting together, to the crew. The 5-witness threshold counts witnesses of that event; "aggressive
  silencing" targets the attributed player(s). Denial/compromise state is per-witness-NPC, shared across the crew — one
  player compromising a witness helps everyone. Decorum stays per-player, so one player's blown cover doesn't blow the crew's.
- **Bots from day one:** society NPCs (humanness layer) always; a bot can fill an empty crew slot for testing.
- **Deferred, named:** adversarial corporations (multiple onboardings), PvP shadow war between crews (DEADWEIGHT-style async sim),
  persistent-world/season lineage integration.
- **Onboarding (founder refinement, 2026-09-18):** skipped entirely for adversarial modes; optional for co-op. Build an onboarding
  that works for co-op, but do **not** over-design it until more game systems materialize — until then it is a thin, skippable
  stub (no dedicated tutorial content, no per-role variants), and no system may *depend* on having gone through it.

## 8. Animation + AI-brain scoping pass (2026-09-20, S504) — "bring in animations ... start building the AI brain"

Founder real-time: *"bring in animations make all characters use the manequin we have and pull from the animations
library start building the AI brain attention mechanisms and values and all the mishri stuff for both 'citizens' and
also for 'the men' to a certain extent ZOMBIES need to also have their own values and attention mechanisms but they
are mofe [more] zombie values and behaving you know?"* Per Principle 19 this is a big, unscoped ask spanning two real
domains (character rendering, NPC AI); this section investigates what already exists, cuts a real V0 for each, and
names what's deferred rather than building either blind.

**Checked first, not assumed:** SHANKPIT already has a real, general-skeleton character renderer
(`packages/goldenband/gband_skel_npc.c`, "for any OTHER GOLDENBAND character asset ... used to spawn NPCs") and a
real, tested MISHRI-derived mood/timing-jitter layer (`packages/simulation/humanness.c`,
`docs/HUMANNESS_NORTHSTAR.md` Phase 1+2 DONE). BIG_O's own `core/witness_rules.c` already has a real attention
mechanism for citizens — `vigilance` (0..100, per-NPC, `docs/B1_WITNESS_RULES.md` §5) feeding `noticed()` — and a
real zombie tactic state machine (`zombie_next_state`, terrain/wall-breach logic). Neither of these needed
reinventing; the real gap was (a) BIG_O's day client had zero GOLDENBAND code at all (only 4 loose zombie animation
clips, no mesh/skeleton, no renderer), and (b) `vigilance` was a static int, never modulated by anything.

### 8a. Animation — DONE (vendoring + a real, minimal end-to-end proof)

Vendored verbatim from SHANKPIT (`packages/goldenband/{gband,gband_skel_npc,gmesh,gpose,gseq,gskel,gsync}.{c,h}`,
`sha256.h`, `packages/common/mat4.h`) into `day/packages/goldenband/` + `day/packages/common/mat4.h`, plus the
mannequin's own mesh/skeleton (`mannequin_npc.gmesh`/`.gskel`) and its generic idle/walk/dance clip set
(`UAL1_Standard_*`) into `day/assets/goldenband/`. **All characters use the mannequin** (the direct answer to "make
all characters use the manequin we have"): the zombie clips already vendored in a prior session turned out to target
the *exact same* skeleton (confirmed live — the zombie clips' own channel names, `pelvis`/`spine_01`/`thigh_l`/...,
match `mannequin_npc.gskel`'s own joint names byte-for-byte) — one shared rig, an animation-library swap per
archetype, never a separate zombie mesh. Wired a `bigo_gband_draw_skinned` callback into `day/apps/client/src/main.c`
(plain immediate-mode `GL_TRIANGLES`, reusing `cel_color3f` per-vertex — this client stays "deliberately legacy
fixed-function OpenGL", no shader/VBO pipeline pulled in) and two test NPC kits (mannequin + zombie-clip-on-mannequin)
drawn once per frame near the loaded level's own spawner, proving the real load→skin→animate→draw path end to end.

**Real, live, found-and-fixed bug along the way** (not guessed): `GSEQ_MAX_CHANNELS` was 256, but
`mannequin_npc.gskel` has 65 joints × 7 channels/joint = 455 — the mannequin's own clips silently failed to load,
in BOTH this vendored copy AND upstream SHANKPIT itself (confirmed live against SHANKPIT's own real asset library:
only `Leela`, 17 joints, actually fit under the old cap — `mannequin_npc`/`Stan`/`Mike`/`George` all silently failed,
and SHANKPIT's own kit-cycling fallback converged every player/NPC onto `Leela` regardless of intended per-id
variety). Fixed upstream (SHANKPIT `d14c7a6`) and here identically, verified live: all kits now load, `make lobby`
and this repo's own client build both clean.

**Deferred, named:** no live NPC entity system exists to drive this yet (`PC_PACKET_ENTITY_SPAWN` is an item-pickup
system only, not role/AI-shaped) — that's real, separate, not-yet-built work (§8c below), not a rendering gap.
Materials/shaders aren't parsed by `level_loader.h` yet (walls render flat-shaded — a pre-existing, already-named gap,
unrelated to this pass). Per-role clip selection (a citizen vs. The Men vs. a zombie all picking a *different* animset
off the same mannequin) needs the AI-brain work below to know which role an NPC even is.

### 8b. AI brain / values / attention — DONE (primitives + citizen/Men integration + zombie module, no live integration yet)

Same real "primitives proven in isolation first" discipline SHANKPIT's own `humanness.c` Phase 1 already used — this
pass builds and tests the brain layer, it does not yet wire it into a live server tick (§8c).

- **`core/humanness.c`/`.h`** — MISHRI's real primitives, vendored verbatim from SHANKPIT (mood enum, energy/
  fatigue/curiosity/boredom, `humanness_reaction_delay_ms`, `humanness_aim_noise`, `humanness_smooth_turn_step`,
  `humanness_tick_mood`/`humanness_get_startled`). `core/humanness_test.c`'s own 7 real MISHRI-bar behavioral tests
  (STARTLED genuinely faster than TIRED over 500 trials, etc.) re-verified passing on this copy. One real portability
  fix along the way: `M_PI` isn't exposed under this repo's own strict `-std=c99 -Werror` Bazel toolchain (SHANKPIT's
  looser Makefile build never hit it) — replaced with a literal, matching `zombie_values.c`'s own established
  convention.
- **`core/npc_archetype.h`/`.c`** — Citizens + The Men (design digest §11's "blue-collar cleanup crew" — plumbers/
  electricians/engineers/regulators). Real answer to "attention mechanisms and values": `NpcBrain` pairs a
  `HumannessState` with an archetype-differentiated `base_vigilance` (Citizen 35, The Men 85 — the docs' own
  "tired contractor" vs. "veteran guard" ends of the 0..100 scale) and `npc_brain_effective_vigilance()` modulates it
  live by mood/energy/fatigue (STARTLED +25, TIRED −15, each fatigue/low-energy point further down), clamped to
  `witness_rules.h`'s own real 0..100 scale — **the actual attention-mechanism integration point**: this is a real,
  dynamic value meant to replace `core/sim.h`'s `SimNpc.vigilance` (currently a static int, set once, never
  modulated) at the call site that feeds `noticed()`. 7 real statistical tests (`core/npc_archetype_test.c`),
  including a 500-trial run confirming The Men's average effective vigilance genuinely exceeds Citizens' across real
  mood variation, not just at init.
- **`core/zombie_values.h`/`.c`** — zombies' **own** vocabulary, per the founder's own explicit "more zombie values
  and behaving, you know?" — deliberately NOT `NpcBrain` with different numbers plugged into the human mood enum.
  `ZombieState`: hunger/aggression/decay (0.0-1.0) instead of energy/curiosity/boredom, a 4-state mood arc
  (DORMANT→AGITATED→HUNTING→FRENZIED, frenzy is a real spike from a stimulus or high sustained aggression, never a
  slow drift) instead of MISHRI's 8 states, and `decay` — a one-way physical-deterioration clock with no human
  equivalent, widening reaction delay and lunge inaccuracy over the zombie's lifetime regardless of mood. Reaction
  timing has a genuinely different shape from `humanness.c`'s STARTLED/TIRED curve: FRENZIED is faster **and**
  measurably more erratic (real, higher variance, not just a lower mean) — verified via a real 2000-trial variance
  comparison, and a real, live bug caught and fixed along the way (spread was scaling off the mood-adjusted mean
  instead of off `base_ms`, so DORMANT's own inflated mean was accidentally producing a *larger* absolute spread than
  FRENZIED — backwards from the intended "erratic" contract). Deliberately NOT wired into `core/witness_rules.c` —
  zombies are the thing citizens/The Men witness, never a witness themselves;
  `zombie_effective_alertness()` is a separate, future perception-radius hook, not a second copy of `vigilance`. 11
  real statistical tests (`core/zombie_values_test.c`).

### 8d. Live NPC entity system — DONE (2026-09-20, S504 §8c item 1)

Closes the real blocker §8c named: `PcNpcState` (x/y/z/yaw/role) added to `PcSnapshotPacket` (measured, not
estimated: 1436 → 1604 bytes), a real server-side `ServerNpc` array (`apps/server/src/main.c`) spawning 3
Citizens/1 The Men/4 zombies on a 10-unit circle around origin at startup (no level spawner coordinates available
server-side yet — `level_loader.h` is still client-only, a real, separate, still-open gap), and
`server_tick_npcs` running `npc_brain_tick`/`zombie_tick` on every real server tick — **the brains built in §8b are
now live**, ticking mood/hunger/aggression on an actually-running server, not just in a test harness. The client's
two hardcoded test-NPC draws are gone, replaced by a real loop over the live snapshot; role→kit/tint selection and
yaw→facing_rad conversion were factored into `day/packages/common/bigo_npc_visual.h` specifically so they're
testable without a live GL driver (5 real tests, `bigo_npc_visual_test.c`). Citizens and The Men currently share the
mannequin kit, tinted apart — this closes §8c's original item 3 (per-role clip/kit selection) for the
kit-vs-tint layer, though a genuinely distinct visual asset for The Men is still a real, open gap.

Verified live: the compiled server runs the full tick loop for 6 real seconds with all 8 NPCs active, no crash.
The full client↔server round-trip (server broadcasts real NPC data, client receives and renders with the correct
kit) still can't be exercised end to end in this sandbox (no real GL driver) — same standing limitation every prior
client change here already carries, not new to this pass.

### 8e. Deferred (named, phased — still not built)

1. **Wiring `npc_brain_effective_vigilance`/`zombie_effective_alertness` into `core/sim.c`** (replacing `SimNpc`'s
   static `vigilance` field) or into `apps/server/src/main.c`'s own new `ServerNpc` tick — the brains tick live now
   (§8d), but nothing yet reads their output into an actual `noticed()`/witness-state decision.
2. **Real movement/targeting.** NPCs are stationary; zombies' own `has_target` is hardcoded 0 (no player-detection
   exists), so none currently reach HUNTING/FRENZIED in practice. Needed before The Men's own dispatch/sanitize
   behavior (next item) has anything real to dispatch toward.
3. **The Men's own dispatch/sanitize behavior** (SILENCING/ENGAGE → `resolved=1` memory-wipe, `docs/B1_WITNESS_RULES.md`
   §1) as an actual decision loop, not just a faster/steadier `NpcBrain` — §8b/§8d built the *personality* and gave
   it a live tick, not the *job*.
4. **Server-side level loading.** NPCs spawn on a fixed circle around origin because the server has no level
   spawner coordinates (`level_loader.h`, S504 §8a, is client-only) — a real, separate, already-named gap from an
   earlier pass, still open.
5. **A distinct visual asset for The Men** — currently a tinted mannequin, same mesh as Citizens.
6. **PARENA-scriptable per-role personality config** — SHANKPIT's own named Phase 4, still deferred there too; real
   future home for designer-tunable archetype presets instead of the hardcoded ones in `npc_brain_init` today.

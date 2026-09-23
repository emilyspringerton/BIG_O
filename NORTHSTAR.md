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

## 9. Cloning-facility lab simulation (2026-09-20, S504-LAB) — "simulate real lab equipment as much as possible"

Founder real-time, following a gap-analysis pass that found `BP_APP_LAB`'s SPLICE screen was a client-only UI mockup
with zero server logic anywhere: *"build out all of the cloning facility simulation tech we want to simulate real
lab equipment as much as possible."* Real answer: `core/lab_sim.h`/`.c`, a headless equipment pipeline covering
every piece of gear `docs/DESIGN_DIGEST.md` §5/§6 names, with its own named realism hooks built as live mechanics
rather than flavor text — same "game vocabulary, not protocols" discipline the digest itself insists on, and the
same "primitives proven in isolation first" discipline `core/npc_archetype.c`/`core/zombie_values.c` already used.

- **Centrifuge** (`centrifuge_spin`) — purity rises on time×RPM ("spin work") with real diminishing returns;
  over-spinning past a real threshold damages integrity (pelleting/shearing) — the digest's own named trade-off.
- **PCR thermocycler** (`pcr_amplify`) — exponential read-depth amplification; cycling past a safe count both
  damages integrity (primer dimer) and creeps contamination up via nonspecific amplification.
- **Sequencer / bioinformatics terminal** (`sequencer_run`) — returns a `SequencerReadout`, a genuinely noisy
  instrument estimate of contamination (Box-Muller jitter, verified over 2000 trials to converge to ground truth on
  average without ever being handed to a caller directly) and an alignment% that degrades with both true
  contamination and low read depth — the digest's own "isolate reads, filter contamination, align" terminal, given
  a real measurement model instead of reading the sample's own ground-truth fields.
- **CRISPR splice bench** (`crispr_splice`) — a real `SpliceOutcome` enum, not a binary pass/fail: `SPLICE_SUCCESS`,
  `SPLICE_OFF_TARGET_MUTATION` (the digest's own named guide-RNA off-target risk, with a real severity value),
  `SPLICE_NONSENSE_MEDIATED_DECAY` and `SPLICE_CRYPTIC_SPLICE_FAILURE` (the digest's own named failure states — real
  dead ends, not degraded successes), and `SPLICE_UNSTABLE_LINE` (a clean edit a drifted line can still downgrade).
  A `guide_rna_specificity` and `operator_skill_0_to_1` parameter pair are the real, live integration points for two
  features that stay explicitly unbuilt here: a future guide-design choice and the digest's own named micromanipulation
  needle mini-game — this module is where a future mini-game's score plugs in, not a mini-game itself.
  `retrotransposon_jump` is a real, independent side-effect (more likely on contaminated samples, verified via a
  2000-trial rate comparison) — the digest's own named "retrotransposon-style pathogen copying" flavor hook, made a
  live, testable mechanic rather than lore.
- **Repressor/kill-switch install** (`crispr_install_repressor`) — the digest's own named sequenceable kill-switch,
  running the same splice risk model underneath; an off-target or drift-driven install carries a real, lower
  `reliability_0_to_1` rather than a bare present/absent flag — a switch can be installed and still be unreliable.
- **Breeding / genetic drift** (`clone_breed`) — a child sample averages its parents' physical stats but always
  accrues new, one-way drift on top of theirs (verified never decreasing across 20 real bred generations) — the
  digest's own named "genetic drift of repeatedly bred lines" realism hook, made a real, growing, game-mechanical
  cost of breeding a line too many times rather than a flavor claim.
- **Embryo incubation** (`incubate_embryo`) — viability derived from real integrity/contamination/drift plus the
  last splice's own outcome; an NMD or cryptic-splice-failure sample can never incubate viable regardless of how
  clean its physical stats are, and a real floor means a low-viability embryo genuinely does not take.

17 real statistical tests (`core/lab_sim_test.c`, same MISHRI-bar discipline as `zombie_values_test.c` — distribution
comparisons over 2000 trials where randomness is involved, not smoke tests), verified live via `gcc -std=c99 -Wall
-Wextra -Werror` (matching this repo's own `COPTS`) since Bazel isn't installed in this sandbox — same standing
verification pattern every prior core module in this repo already used. Wired into `BUILD.bazel` as `:lab_sim`/
`:lab_sim_test`.

**Deferred, named, not built in this pass:**

1. **No UI/server wiring.** `BP_APP_LAB`'s phone screen (samples/base/trait/SPLICE/clone list) is still the exact
   client-only mockup the gap analysis found — this pass builds the simulation core the screen would eventually call
   into, not the screen itself, and not any server-side handling of a lab action packet (none exists).
2. **No persistence.** `LabSample`/clone lines exist only as values passed between these functions in a test — no
   save/load format, no per-crew lab state, no `papercraft_persist.h`-style file.
3. **No micromanipulation mini-game UI.** `operator_skill_0_to_1` is a real, live parameter with no input source yet.
4. **No PARENA rules-module version.** This is plain C (`core/lab_sim.c`), matching `core/humanness.c`'s own
   "runs every tick, needs to be fast, not author-editable per-instance" reasoning — unlike `witness_rules.c`, lab
   equipment numbers are not currently expected to need mod-author tuning, but that could change.
5. **No connection to the harvest/day loop.** `lab_sample_init_wild_harvest`'s `contamination_pct` input has no real
   source yet — day-phase harvest doesn't produce a `LabSample` today; that's a real, separate, not-yet-built bridge.

## 10. Pheromone command tools (2026-09-20, S504-PHEROMONE) — closes half of §8e item 2

Founder-priority gap: `zombie_tick`'s `has_target` was hardcoded `0` (§8d/§8e), so zombies never reached HUNTING/
FRENZIED and The Men had nothing to dispatch toward. Real answer: `day/packages/common/bigo_pheromone.h`, a pure,
header-only targeting/steering module (`pheromone_marker_expire`, `pheromone_claim_slot`, `pheromone_find_nearest`,
`pheromone_step_toward`, 8 real tests), plus a new one-shot wire packet `PC_PACKET_PHEROMONE_THROW`
(`PcPheromoneThrowPacket`, `papercraft_protocol.h`) — the client's **G key** throws a marker at the player's own
position + camera-forward × a fixed throw distance (no real projectile arc, matching `docs/DESIGN_DIGEST.md` §6's
"paint a target" framing over actual physics), the server claims a slot in a small, bounded
`g_pheromones[BIGO_PHEROMONE_MAX]` array (30s real expiry), and `server_tick_npcs` now gives every zombie a real
`has_target` computed from proximity to an active marker instead of `0`, steering it toward the marker at
`PHEROMONE_ZOMBIE_SPEED` when locked on. Verified live: a raw UDP `PC_PACKET_PHEROMONE_THROW` sent to a running
server produces the real marker log line, and a scratch integration check (zombie starting on the real 10-unit spawn
circle, ticked the same way `server_tick_npcs` does) converges the zombie's position exactly onto the thrown marker
and escalates it to FRENZIED.

**Real, honest finding, not fixed here (scope discipline, not an oversight):** `zombie_tick` internally approximates
a flat ~1 real second of drift per call regardless of actual elapsed time (its own header's documented v0
simplification, from the original §8b pass) — `server_tick_npcs` calls it at `PC_TICK_HZ` (20Hz), so a newly
targeted zombie now reaches FRENZIED almost immediately rather than over a realistic window. This quirk pre-dates
this pass (it already ran at 20Hz with `has_target` hardcoded 0, just silently, since hunger/aggression drift alone
never mattered) — this pass is the first time it's actually visible, because `has_target` is now reachable. The real
fix (passing a real `dt_sec` into `zombie_tick`/`npc_brain_tick`) is separate work, deliberately not folded into this
pass to avoid re-touching a module whose own tests were already locked down in §8b.

**Deferred, named:**
1. **No autonomous player-detection.** A zombie with no thrown marker in range is exactly as before — stationary,
   drifting on hunger alone. This closes only the player-*commanded* half of §8e item 2; a zombie independently
   noticing a nearby player/citizen with no marker thrown is still real, separate, unbuilt work.
2. **No real projectile.** The marker lands instantly at a computed point; no arc, no travel time, no line-of-sight
   check against walls (this client's own level geometry isn't loaded server-side yet regardless, §8a).
3. **No acoustic pingers / hormone emitter** (`docs/DESIGN_DIGEST.md` §6's other two named command tools) — only the
   pheromone ball/dart was built, matching the founder's own specific "has_target" framing of the gap.
4. **Citizens/The Men never react to a commanded zombie.** This closes the zombie-side targeting gap only; nothing
   yet feeds a nearby human NPC's own witness/vigilance decision from a HUNTING/FRENZIED zombie's presence — that's
   §8e item 1 (wiring vigilance into an actual `noticed()` decision), still open at the time this section was
   written (closed in §11 below, later the same day).

## 11. Witness wiring + The Men's dispatch loop (2026-09-20, S504-DISPATCH) — closes §8e items 1 and 3

Founder-priority gap, directly following §10: a compromised/silenced citizen was possible only in `core/sim.c`'s
offline scenario harness, never live, and The Men had nothing real to dispatch toward. Real answer:
`core/witness_live.h`, a pure, header-only glue module (`bigo_zombie_is_witnessable_event`, `bigo_in_range`,
`bigo_witness_next_state_for_event`, 8 tests) wiring a HUNTING/FRENZIED zombie as a real, LOUD witnessed event
(`docs/B1_WITNESS_RULES.md` §7's own "witnessed by every NPC in the zone" rule, scaled to a live detection radius
since the server has no per-NPC zone concept) into `core/witness_rules.c`'s own real, already-tested
`npc_next_state`/`effective_witnesses`/`is_legal_transition` — exactly `core/sim.c`'s own `sim_release` semantics,
just radius-based. `apps/server/src/main.c` gained two new tick functions: `server_tick_witness` (every Citizen/The
Men NPC within `BIGO_WITNESS_DETECTION_RADIUS` of a witnessable zombie now has a real, live `witness_state`, an
added `ServerNpc` field) and `server_tick_dispatch` (The Men's own real dispatch/sanitize loop, §8e item 3: any
human NPC reaching SILENCING/ENGAGE gets the nearest idle The Men NPC assigned, travelling toward it via
`bigo_pheromone.h`'s own `pheromone_step_toward` reused verbatim, resolving the hunt to DENIAL — the digest's own
"memory-wipe spray" — on arrival). `core/witness_rules.c` is now linked into the live day server for the first time
(`scripts/build_day.sh`).

Verified live: a thrown pheromone marker escalating a zombie to FRENZIED produces real `witness_state` transitions
in the server log against the actual v0 spawn population. **Real, honest, live-found structural limit**: the v0
spawn is only 3 Citizens + 1 The Men (4 humans total) — `docs/B1_WITNESS_RULES.md`'s own 5-witness SILENCING
threshold is mathematically unreachable with a single witnessed event and only 4 humans in the whole world, so
DENIAL is the practical ceiling in live play today; SILENCING/ENGAGE and the dispatch loop's own arrival/resolve
behavior were verified instead via `core/witness_live_test.c` (pure, synthetic counts) and a larger scratch
integration harness (5 citizens + 1 The Men + 1 zombie) confirming the full loop end to end. **A second real, live
finding from that same harness**: resolving a hunt to DENIAL does not make the underlying zombie stop being a
witnessable event — if it's still HUNTING/FRENZIED and still in range on the very next witness tick, the same
humans re-witness it and the hunt reopens immediately, producing rapid dispatch/resolve churn. This is arguably
correct per the fiction (The Men clean witnesses, they don't make the zombie disappear) but it means, mechanically,
a hunt never truly ends without either the zombie itself leaving HUNTING/FRENZIED, leaving range, or being
eliminated — `core/sim.c`'s own `resolved=2` ("target eliminated/gone → UNAWARE") and `sim_los_lost` already model
exactly this second resolution path, but this pass only wires `resolved=1` (memory wipe); wiring the LOS-loss/
elimination path is real, separate, deferred work below.

**Deferred, named:**
1. **No LOS-loss/elimination resolution.** Only `resolved=1` (memory wipe) is wired; a hunt can currently only end
   by The Men arriving, never by the zombie itself losing HUNTING/FRENZIED status, leaving range, or being killed —
   the churn behavior described above is the direct, live consequence.
2. **No Corporate Service Call / Regulator escalation at max heat** (`docs/DESIGN_DIGEST.md` §11) — when every The
   Men NPC is already dispatched, a new hunt simply waits with no responder (a real, honest v0 cap), rather than
   escalating to a named, deadlier tier.
3. **No quiet-observation (costume/gear) witnessing.** Only the LOUD zombie-event path is wired; `noticed()`/
   `conspicuousness()`'s own costume/gear/decorum path (`core/sim.c`'s `sim_observe`) is a real, separate, still-
   unbuilt gap — no live decorum meter, no costume enforcement, matches the original gap-analysis finding.
4. **No accomplice/compromise mechanic live.** `effective_witnesses` is always called with `accomplices=0` — a
   player can't yet force a witness into COMPROMISED in the live game, only in `core/sim.c`'s own scenario harness.
5. **Fixed arrogance (50 for every human NPC).** Real per-NPC arrogance variety — needed for ENGAGE to ever fire
   live without every witness sharing the exact same threshold behavior — is deferred, same category as §8e item 6.
6. **The Men carry pagers** (founder real-time, 2026-09-22: "the men cary pagers"). Real, named design direction
   for the dispatch mechanic above, not built. `server_tick_dispatch`'s own current "nearest idle The Men NPC
   assigned" step is instant and omniscient — no in-fiction justification, no player-visible signal. A pager is
   the real, concrete answer to "how does a Man know to respond," and the exact insertion point: it wraps that
   existing nearest-idle-Man selection, it does not replace it. What this opens, none of it scoped or built yet:
   - **Real dispatch latency.** A gap between a hunt escalating (SILENCING/ENGAGE) and the assigned Man actually
     starting to move — an actual pager's real "message sent → Man notices the buzz → responds" delay, replacing
     today's instant assignment.
   - **A real, world-observable cue.** A buzz/beep, a lit pager on a Man's belt — something a sharp player could
     actually notice as a tell that a hunt was just dispatched toward them. This is a direct, on-theme extension
     of the signature Attention/Heat framing ("if they don't see it, it isn't real") — a pager going off IS a
     real thing that can be seen, unlike the current invisible server-side assignment.
   - **A possible future interaction surface** — intercepting, jamming, or stealing a pager. Named as the natural
     next question this mechanic raises, not scoped or committed to.
   - Where this reconciles with the SHANKPIT engine merge: `SHANKPIT/docs2/specs/BIGO_ENGINE_MERGE_NORTHSTAR.md`
     phase 7d (the still-undecided `AI_ROLE_*` roster cutover) and phase 7b's own `witness_ai.c` (SHANKPIT's own
     live population/tick loop, which explicitly does not yet have a resolution/memory-wipe loop at all) are the
     real, eventual places this would need to land on the SHANKPIT side — not built or wired there either.

## 12. Reverse port: bringing SHANKPIT's forward work back into BIG_O (2026-09-23, EMILY/BACKLOG.md SECTION 536 follow-up)

§§1-11 above are BIG_O's tech flowing INTO SHANKPIT (phases 1-7e, "BIG_O engine merge"). SHANKPIT kept building on
its own MODE_STORY after phase 7e landed — full phone-app parity/costumes/The Men, a 17-item food/cargo system
+ cake distraction, a wheelbarrow carry mechanic, giant zombie bugs (TRAPX Rogue Swarm Doctrine), and a
walkie-talkie channel/hearing primitive (Asterisk-backed) — none of which exist in this repo. This section is the
reverse direction: checked first per Principle 19 (investigate, cut a real phase 1, phase the rest), not ported
blind.

**Phase 1 (landed): walkie-talkie channel/hearing decision logic.** `PARENA/stdlib/big_o/walkie_rules.prn`
(logic identical to SHANKPIT's own `stdlib/shankpit/walkie_rules.prn`, renamed into this repo's own `big-o/`
module namespace, same "copied, logic identical" precedent §1/§2 above already established in reverse) generates
`day/packages/simulation/walkie_rules.c`. New `day/packages/common/bigo_walkie_talkie.h` — pure, header-only host
wrapper (caller owns all state, unlike SHANKPIT's own version which kept a hidden `g_players[]` array — matches
this repo's own `bigo_pheromone.h` precedent instead). `scripts/gen_rules.sh` extended to regenerate it. 5/5 real
assertions pass (`bazel test //day/packages/simulation:bigo_walkie_talkie_test`), verified against the real,
live PARENA-compiled logic, not a mock.

**Real, honest finding, checked before wiring further (bigger than walkie-talkie itself):** BIG_O's v0 has
exactly ONE shared crew/world (§7's own "one crew, one onboarding" decision) — there is no team/crew concept for
players at all. Every player would resolve to the same team_id, so `walkie-can-hear` degenerates to "always
true" and the channel decision is moot. This module is therefore a real, standalone, tested primitive with NO
LIVE CONSUMER yet — the same "correct primitive, no consumer" pattern this repo's own phases 2/3/5 already
established (§§ above), just running in the other direction. Real wiring needs a real team/crew-assignment
system first (PvP teams? Rival cleanup crews? Undecided) — not guessed at here.

**Real, unrelated bug found mid-pass, then independently fixed by a concurrent commit:**
`day/packages/common/http_client.h` failed to build (`struct addrinfo`/`struct timeval` used without their POSIX
feature-test macros) under this sandbox's toolchain, breaking `//day/packages/common:http_client_test` and
`//day/packages/common:level_loader_test`. Pre-existing (confirmed via `git diff --stat` — untouched by this
pass), unrelated to walkie-talkie. A same-day upstream commit (`4e53e09`, "fix Bazel C99 build") fixed it before
this port's own commit landed — rebased cleanly onto it; `bazel test //...` is now 34/34 green, including both
previously-broken targets.

**Queued, not started — each needs its own real investigation pass, not a blind port (Principle 19):**
1. **17-item food/cargo system + cake distraction.** SHANKPIT's `food_pickup.c` hand-places pickups around a
   hardcoded MODE_STORY VOXWORLD landmark and heals the player via `PlayerState`'s 0..100 health scale. BIG_O's
   `day/` world is structurally different on both counts: it's a real, persisted single-node world fetched from
   a live worldapi (chunk grid, map-editor-authored `PcWorldObjectFile`), not hardcoded landmark coordinates —
   and BIG_O's `PlayerSlot` has no health/HP field anywhere (only destructible world-object fragment HP exists).
   BIG_O *does* already have a more general, better-fitting item substrate than SHANKPIT's own hand-placed spots
   (`g_entities[]` + `papercraft_inventory.h`'s real stacking pickup path, currently fed only by GTA3-style drops
   from destroyed world objects) — the real question is whether food items should drop from destruction (reuse
   as-is) or need a new hand-authored spawn mechanism (a real map-editor content pass), and "eat for heal" is
   blocked on a player-health system that doesn't exist yet. Real scoping pass needed before any code.
2. **Wheelbarrow carry mechanic.** Depends on the same hardcoded lab-trespass-circle delivery target SHANKPIT's
   version uses — BIG_O has no equivalent authored landmark in its persisted world yet either.
3. **Giant zombie bugs (TRAPX Rogue Swarm Doctrine).** `giant_bug_values.{h,c}` extends `zombie_values.h`, which
   BIG_O already has natively (`core/zombie_values.h` — SHANKPIT's own copy was ported FROM this repo), so this
   is likely the next-best-fitting port after walkie-talkie: same foundation, no world-model mismatch. Not
   investigated in this pass — queued.
4. **Full phone-app UI parity + costumes + The Men.** Real, player-visible client rendering work (home grid,
   Wardrobe screen) — BIG_O's own `bigo_phone.h` already has the same app roster (`BP_APP_CARGO` etc.) but no
   on-screen UI either; this is a real, separate client-rendering scoping pass on both repos' own already-named
   gaps, not something to guess at here.

session: sess-20260920-1908-24cb3558.

## 13. Reverse port phase 2: giant zombie bugs (2026-09-23, EMILY/BACKLOG.md SECTION 536 follow-up)

§12 named giant zombie bugs as the next-best-fitting reverse-port candidate after walkie-talkie, since
`giant_bug_values.c` extends `zombie_values.h` (BIG_O's own field, SHANKPIT's copy was ported FROM this repo in
§3) rather than something BIG_O's own world model has diverged from. Confirmed live: field-for-field identical
`ZombieState` in both repos, so this ported with zero adaptation.

**Landed:** `PARENA/stdlib/big_o/giant_bug_brain.prn` (logic identical to SHANKPIT's own, renamed into this
repo's own `big-o/` module namespace) generates `core/giant_bug_brain.c` — the real "8 inputs × 8 hidden units =
64 hand-picked weights" feedforward net (see the `.prn` file's own doc comment for the honest scope of "64 layer
hand written llm"). New `core/giant_bug_values.{h,c}` — alien sense-vocabulary state (hunger, swarm_density,
heat_scent, molt_pressure, ground_vibration, pain, hive_signal, light_aversion) sharing nothing with
`humanness.h`/`zombie_values.h`'s own vocabularies, plus real, permanent strength/speed growth on
`giant_bug_eat_zombie` (aggression → strength proxy, inverse `zombie_reaction_delay_ms` → speed proxy, both
already-live `zombie_values.h` fields, not invented). `scripts/gen_rules.sh` extended to regenerate the brain.
6/6 real assertions pass (`bazel test //:giant_bug_values_test`); full suite `bazel test //...` is 35/35 green
with zero regressions.

**Real, honest finding, checked before wiring further:** no live consumer yet, same pattern §12's walkie-talkie
established — but for a different, more concrete reason here. BIG_O's live `day/apps/server` `ServerNpc` array
is `PC_NPC_MAX=8`, already fully populated by `server_spawn_npcs` (3 Citizens, 1 The Men, 4 zombies) — there is
no free slot for a 4th role without growing that array. SHANKPIT's own live version also names a real,
analogous gate not built here either: `witness_ai_bug_command_authorized` ("Men hold the key" — a spawned bug
only hunts/eats while ≥1 live The Men NPC is active, TRAPX Rogue Swarm Doctrine) and the leela-kit-reused
2.5×-scaled-and-tinted visual. All real, separate, named follow-up work for a phase 3 (grow `PC_NPC_MAX`,
add `PC_NPC_ROLE_GIANT_BUG`, wire the spawn/tick/eat loop and the Men-hold-the-key gate into
`server_tick_npcs`) — not guessed at or half-built here.

session: sess-20260920-1908-24cb3558.

## 14. Reverse port phase 3: giant zombie bugs go live (2026-09-23, EMILY/BACKLOG.md SECTION 536 follow-up)

§13 named phase 3 as "grow the roster, wire the Men-hold-the-key gate" — checked first, and the growing part
turned out to be the wrong move: `PC_NPC_MAX=8` is baked into `PcNpcState[PC_NPC_MAX]`'s own wire snapshot size
(`papercraft_protocol.h`), so resizing it is a real network-protocol change, not a cheap one. SHANKPIT's own live
version (`witness_ai.c`) already made the same call for the same reason — a genuinely separate array
(`g_giant_bugs[]`), not a new `PC_NPC_ROLE_*`. This phase copies that same real shape into BIG_O's own live
server rather than the riskier roster-growth path §13 originally named.

**Landed, live, in the real day server:** `day/apps/server/src/main.c` gained `ServerGiantBug
g_giant_bugs[BIGO_GIANT_BUG_MAX]` (8 slots, separate from `g_npcs`), `server_spawn_giant_bugs` (one bug spawned
adjacent to a real zombie NPC's own spawn point, close enough to verify the eat loop live without needing any
movement AI — neither bugs nor zombies move in this v0), `server_giant_bug_command_authorized` (the real "Men
hold the key" gate — brought back from SHANKPIT's `witness_ai_bug_command_authorized`, ported verbatim: >=1 live
`PC_NPC_ROLE_THE_MEN` NPC required), and `server_tick_giant_bugs` (per-tick: if authorized, eat the nearest
zombie NPC within `BIGO_GIANT_BUG_EAT_RADIUS`, else just drift hunger via `giant_bug_tick`). Wired into the real
tick loop and startup spawn alongside `server_tick_npcs`/`server_spawn_npcs`. `scripts/build_day.sh` extended
with the two new source files.

**Live-verified, not just compiled:** ran the real `bigo_day_server` binary (isolated `--port`/`--save-dir`/
`--world-file`/`--damage-file`, pointed at the real, shared, already-running worldapi on `:7070` read-only, same
as PAPERCRAFT/WEAKNIGHT_BEDROCK_RACERS' own live instances) — real log output: `S536-BUG: spawned 1 real giant
zombie bug...` then `S536-BUG: bug0 ate npc4 (zombie) -- strength=1.00 speed=1.13`. Strength stayed at baseline
(correct: a freshly-spawned zombie has 0.0 aggression, so `giant_bug_eat_zombie`'s real strength-gain formula
correctly contributes nothing) while speed grew (a dormant zombie's own reaction delay is still finite, so the
speed proxy is always positive) — exactly the documented "eat a strong/fast zombie, get stronger/faster"
mechanic behaving correctly on real, live data, not a scripted test. `bazel test //...` 35/35 green, zero
regressions; `scripts/build.sh` (the real CI ASan/UBSan path) also clean.

**Real, honest, deliberately NOT built here (named, not silently dropped):**
1. **No bug movement.** Matches zombies' own existing "stationary in v0" scope cut — real movement/hunting AI
   for either is separate, deferred work.
2. **No network broadcast / client visual.** `g_giant_bugs` is server-side simulation only, verified via log
   output — same "prove it live in the log first" precedent `server_tick_witness`'s own live-verification
   already established. No wire packet, no rendering, no leela-kit-reused 2.5×-scaled-and-tinted visual
   (SHANKPIT's own real answer, not replicated here).
3. **No eaten-NPC despawn broadcast.** `prey->active = 0` is real and correct server-side state, but nothing
   tells a connected client the eaten NPC is gone — moot until #2 above gives bugs (and the wider NPC roster)
   any client presence at all.
4. **TRAPX Rogue Swarm Doctrine** — real, named, deliberately not modeled, same deferral SHANKPIT's own version
   already made (GTA7's own separate faction-doctrine system, not guessed at without checking that repo first).

session: sess-20260920-1908-24cb3558.

## 15. Reverse port phase 4: wheelbarrow (2026-09-23, EMILY/BACKLOG.md SECTION 536 follow-up)

§12 originally named wheelbarrow as blocked ("no equivalent authored landmark in BIG_O's persisted world").
Re-checked: that blocker was real but smaller than it first looked -- SHANKPIT's own version doesn't use any
real level-authoring system either, it reuses one hardcoded circle (`WITNESS_AI_LAB_ZONE_*`) "well clear of the
NPC spawn footprint," the same discipline every other live-server landmark in this repo already uses
(pheromone detection radius, witness detection radius, dispatch arrival radius). This phase makes the same
call: a new, real, hardcoded circle at `(BIGO_LAB_ZONE_CX=30, BIGO_LAB_ZONE_CZ=0)`, radius 6 -- 24 units clear
of the 10-unit NPC spawn circle at origin and the giant-bug spawn point near (-9,0,1).

**Landed, live, in the real day server:** a new wire packet `PC_PACKET_WHEELBARROW_TOGGLE` (15) +
`PcWheelbarrowTogglePacket` (header-only, no payload -- the server already resolves the sender from the packet's
own source address, same `PC_PACKET_INTERACT`-style lookup, not a new identification mechanism).
`server_wheelbarrow_toggle(requester)` -- real, independently-callable decision logic (factored out of the
packet handler, same "extract the real decision" discipline `server_throw_pheromone` already established): drop
if already carrying, else pick up the nearest carryable NPC (Citizen or Zombie only, matching the founder's own
"whole zombie or citizen" wording -- The Men are never carryable) within `BIGO_WHEELBARROW_PICKUP_RADIUS`.
`server_tick_wheelbarrow()` -- trails the carried NPC 2 units behind its carrier's own real position/yaw every
tick (BIG_O's `state.yaw` is already radians, unlike SHANKPIT's degrees-with-a-conversion-factor version), then
checks delivery into the lab circle. Unlike SHANKPIT's own version (which always trails a fixed "hero," player
slot 0), this tracks WHICH connected player is carrying and trails that specific player -- a real, necessary
difference since BIG_O's own live server has no fixed-hero convention (real co-op, up to 3 players, §7).

**Live-verified, not scripted-only:** a real scratch integration harness (`#include`s `day/apps/server/src/
main.c` directly, same "call the real, unmodified functions, skip the socket/ticket layer" precedent the
pheromone/dispatch phases already used) exercises `server_spawn_npcs` → `server_wheelbarrow_toggle` →
`server_tick_wheelbarrow` end to end against real state: pickup claims the nearest real Citizen NPC, a second
toggle drops it, walking the cargo into the real lab zone despawns it and increments the real delivery counter,
and a disconnected carrier makes the tick stand down safely (cargo stays active, not silently faked as
delivered). Also re-ran the full, real `bigo_day_server` binary end to end (isolated port/paths, real shared
worldapi read-only) to confirm the new state doesn't regress startup/shutdown -- clean boot, clean tick, clean
signal-shutdown. `bazel test //...` 35/35 green; `scripts/build.sh` ASan/UBSan path clean.

**Real, honest, deliberately NOT built here (named, not silently dropped), same scope cuts SHANKPIT's own
version already made:**
1. **No literal wheelbarrow prop/model.** A real, separate art task -- the carry mechanic itself is the real
   feature, matching the founder's own framing.
2. **No client input wiring.** The real client-side E-key binding (or equivalent) that sends
   `PC_PACKET_WHEELBARROW_TOGGLE` doesn't exist yet in `day/apps/client` -- verified server-side only, same
   "prove the server logic first" precedent pheromone/witness/dispatch already used before any client wiring
   landed for them.
3. **No network broadcast of carried-NPC position to other clients**, and no despawn broadcast on delivery --
   moot until #2 gives this any client presence at all.
4. **A real, live interaction found (not a bug, named for completeness):** a carried NPC that gets eaten by a
   giant bug mid-carry (phase 3, both mechanics now touch the same `g_npcs[]`) would leave `g_carried_npc_index`
   pointing at an inactive NPC -- `server_tick_wheelbarrow`'s own `!cargo->active` check already handles this
   correctly (stands down, same as a disconnected carrier), verified by the scratch harness's own disconnect
   case exercising the identical code path.

session: sess-20260920-1908-24cb3558.

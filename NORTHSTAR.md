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
1. ~~**No LOS-loss/elimination resolution.**~~ **Closed, 2026-09-23, see §21.** Only `resolved=1` (memory wipe) is wired; a hunt can currently only end
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
5. ~~**Fixed arrogance (50 for every human NPC).**~~ **Closed, 2026-09-23, see §22.** Real per-NPC arrogance variety — needed for ENGAGE to ever fire
   live without every witness sharing the exact same threshold behavior — is deferred, same category as §8e item 6.
6. **The Men carry pagers** (founder real-time, 2026-09-22: "the men cary pagers"). Real, named design direction
   for the dispatch mechanic above. `server_tick_dispatch`'s own current "nearest idle The Men NPC
   assigned" step was instant and omniscient — no in-fiction justification, no player-visible signal. A pager is
   the real, concrete answer to "how does a Man know to respond," and the exact insertion point: it wraps that
   existing nearest-idle-Man selection, it does not replace it. What this opened:
   - ~~**Real dispatch latency.**~~ **Closed, 2026-09-23, see §23.** A gap between a hunt escalating
     (SILENCING/ENGAGE) and the assigned Man actually starting to move — an actual pager's real "message sent →
     Man notices the buzz → responds" delay, replacing the old instant assignment.
   - **A real, world-observable cue — still only a server log line, not a player-visible one.** §23 landed a
     real `S536-PAGER` log marker at buzz-start and buzz-end, but that's an operator/debug signal, not the
     originally-named "buzz/beep, a lit pager on a Man's belt" a player could actually notice in the world — no
     snapshot field, no client rendering, no sound. Still real, separate, unbuilt work.
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

## 16. Reverse port phase 5: the 17-item food/cargo system (real cargo, no heal yet) (2026-09-23, EMILY/BACKLOG.md
SECTION 536 follow-up)

§12 queued this as item 1, blocked on two named gaps: BIG_O's `day/` world is a real, persisted worldapi-backed
grid (not SHANKPIT's hardcoded MODE_STORY landmarks), and `PlayerSlot` has no health field at all. Re-investigated
before writing code, per Principle 19.

**The real finding: the health gap is bigger than originally scoped.** It isn't just "no health field" (a
10-line fix) -- there is no player DAMAGE SOURCE anywhere in this repo (checked directly: only destructible
world-object fragment HP exists; giant bugs eat zombie NPCs, never players; no PvP, no fall damage, no zombie
attack on a player). Adding a health field and an "eat to heal" packet would be real code with zero observable
effect -- health would sit at its max forever. Forcing a fake self-damage debug command just to demo it would not
be a real feature. A real damage-source design decision (giant bugs attacking players? zombie NPC aggression?
PvP?) is genuinely undecided and out of scope for this pass -- named, not guessed at.

**What IS real and landed:** the "cargo" half of "food/cargo system" -- 17 real, pickable, stackable food items,
reusing BIG_O's EXISTING GTA3-style destruction-drop pipeline as-is (§12 item 1's own named option, not a new
hand-authored spawn mechanism). WOOD-material world-object destruction (a real, previously-named "no drop yet"
gap -- the papercraft item_drop_mod.prn's own doc comment already called this out) now drops one of 17 real food
items.

- `day/packages/common/bigo_food_items.h` (NEW) -- byte-for-byte port of SHANKPIT's own
  `packages/common/food_items.h`: 17 real items (8 classic Pac-Man fruits + Ms. Pac-Man's 3 + 5 BIG_O-original +
  the founder-requested BIRTHDAY CAKE), names/points/derived heal-formula all unchanged. Pure data, no PARENA
  involved (matches SHANKPIT's own choice -- it's a table, not a decision function).
- `PARENA/stdlib/big_o/item_drop_mod.prn` (NEW) -- forked from `stdlib/papercraft/item_drop_mod.prn`, NOT a
  shared edit (same "renamed into this repo's own module namespace" precedent walkie_rules.prn/
  giant_bug_brain.prn already set) -- editing the shared PAPERCRAFT copy in place would have silently made
  PAPERCRAFT's own world start dropping food items too. PAPER/METAL branches unchanged (scrap/shotgun). Real,
  small, justified ABI extension: `on-papercraft-item-for-object-destroyed` now takes a second input, the
  destroyed object's own stable world-index -- `object-index mod 17` picks which food item drops, deterministic
  variety with no RNG.
- `PARENA/stdlib/big_o/inventory_mod.prn` (NEW) -- forked from `stdlib/papercraft/inventory_mod.prn` for the
  same reason. Gives food item ids (`PC_ITEM_FOOD_BASE`=8..24) a real 20-per-slot stack cap -- without it every
  pickup would burn its own whole slot out of `PC_INVENTORY_SLOTS`' real 8-slot ceiling. `can-stack` unchanged
  (already fully generic).
- `day/packages/common/papercraft_protocol.h` -- new `PC_ITEM_FOOD_BASE` (8), doc comment names the real
  "pickable cargo today, no heal consumer yet" scope explicitly.
- `day/apps/server/src/main.c` -- `on_papercraft_item_for_object_destroyed` call site now passes the destroyed
  object's own `target` index as the second arg; drop log names the food item by string when applicable
  (`S536-FOOD: ...`). No other host change -- the existing GTA3-style pickup-on-walkover path
  (`try_add_item_to_inventory`) is already item-id-agnostic and needed zero changes.
- `scripts/gen_rules.sh` extended to regenerate both forked mods from `PARENA/stdlib/big_o/`.

**Verified, not just compiled:** `item_drop_mod_test`/`inventory_mod_test` extended with real assertions for the
new material/id ranges (including the mod-17 wraparound); new `bigo_food_items_test` (7 assertions on the ported
data table). A new scratch harness (same `#include main.c` real-function precedent `wheelbarrow_verify.c`
established in phase 4) drove the real, unmodified `on_papercraft_item_for_object_destroyed` +
`try_add_item_to_inventory` end to end: WOOD destruction at world-index 5 deterministically drops PEAR,
lands in a real inventory slot, a second identical pickup stacks (doesn't burn a second slot), a different food
item takes its own slot, PAPER/METAL drops unchanged -- 5/5 pass. `bazel test //...` 36/36 green (3 new real
tests, zero regressions). `scripts/build.sh` ASan/UBSan clean. Live-verified booting the real server binary
against an isolated port/paths against the real worldapi -- clean boot (world load, NPC/giant-bug spawn, real
bug-eats-zombie event still fires), clean signal-shutdown, no lingering process.

**Real, honest, deliberately NOT built here:**
1. **Eat-to-heal.** Blocked on the real, bigger gap named above (no player health field, no damage source of any
   kind to make healing observable). `food_item_heal()` is ported and tested but has no live caller -- same
   "standalone primitive, no live consumer yet" precedent `bigo_walkie_talkie.h` already established in phase 1.
2. **Full 17-item Lost-and-Found-style restock landmark.** Not needed for this pass -- the `object-index mod 17`
   trick already gives real variety through the existing drop pipeline without a new hand-authored spot system.
   A real, separate landmark (SHANKPIT's own "office" flavor) remains a possible future add, not required.
3. **Cake distraction/party-event behavior.** Checked directly against SHANKPIT's own `food_items.h` doc
   comment: SHANKPIT itself never built this either -- BIRTHDAY CAKE is just item #17 in the same data table,
   with the actual party/wedding NPC-choreography ask named as its own separate, unscoped, un-built future
   follow-up there too. Nothing deferred here that SHANKPIT itself actually has.

**Remaining queued from §12:** full phone-app UI parity + costumes + The Men (real client-rendering work in
`day/apps/client`, a different domain than every phase 1-5 change so far, all server-side).

session: sess-20260923-1030-4a526255.

## 17. Two corrections and a real, small fix found investigating phone-app parity (2026-09-23, EMILY/BACKLOG.md
SECTION 536 follow-up)

Before starting §12 item 4 ("full phone-app UI parity"), re-checked its own premise against the actual live
code, per Principle 19. Two of this section's own earlier claims turned out wrong -- corrected here, not
silently overwritten, same discipline §12's own http_client.h correction already used.

**Correction 1 -- BIG_O already has a full, rendered phone UI.** §12 item 4 claimed "no on-screen UI either."
False: `draw_bigo_phone` (`day/apps/client/src/main.c`, landed in `c25f81e`, 2026-09-20 -- part of the ORIGINAL
BIG_O engine merge, predating this whole reverse-port thread) already renders every one of the 11 apps
(Messages/Contacts/Map/Camera/Notes/Lab/Cargo/Skills/Loadout/Wardrobe/Status), including Wardrobe's own real
costume list. The phone-app roster and its rendering are NOT a gap.

**Correction 2 -- SHANKPIT DID build a cake distraction mechanic.** Phase 5's own NORTHSTAR/BACKLOG entries
claimed "SHANKPIT itself never built this either." False -- checked directly this time: SHANKPIT's
`packages/simulation/witness_ai.c` has a real `witness_ai_smash_cake(now_ms)` that halves nearby witness
vigilance for a real duration (`witness_ai_tick`'s own `if (distracted) vig /= 2` block), triggered by
`packages/common/phone.h`'s own real `BP_FX_SMASH_CAKE` effect (raised when FOOD_CAKE is selected in CARGO,
wired live in `apps/lobby/src/main.c`). Real, honest reason this ISN'T ported here either: it needs the
QUIET-observation half of the witness system (costume/decorum-based noticing) live in `day/`'s real server --
`core/witness_live.h`'s own doc comment already names this as a genuinely separate, not-yet-built gap (only the
LOUD-event half, a hunting/frenzied zombie, is wired into the live day server today). Same real blocker, just
now correctly attributed -- not a SHANKPIT gap, a BIG_O one.

**What eat-to-heal actually looks like in SHANKPIT, checked directly (confirms phase 5's blocker was correctly
scoped):** `apps/lobby/src/main.c`'s real handling of `BP_FX_EAT_FOOD` (`hero->health += fx.arg`, clamped to
100) targets SHANKPIT's own MODE_STORY `PlayerState.health` field -- a real field BIG_O's `PcPlayerState` still
does not have -- and it's explicitly CLIENT-LOCAL, not server-authoritative (`food_pickup.h`'s own doc comment:
"client-local only, no server-authoritative story yet"). Copying that shape into BIG_O verbatim would be a real
regression against every other stat here (XP/level/inventory/position are all server-authoritative) -- phase
5's own "needs a real damage-source design decision, not guessed at" conclusion stands.

**Real, small, honest gap found and fixed while re-checking this (not the parity claim itself, a real
pre-existing display bug adjacent to it):** `day/apps/client/src/main.c`'s own `PC_ITEM_TABLE` (backing the
CARGO screen's item names) only had entries for `PC_ITEM_NONE`/`PC_ITEM_SCRAP` -- every `PC_ITEM_WPN_*` (found
weapons) already fell through to "Unknown Item" BEFORE this reverse-port thread touched anything, and the new
food items from phase 5 would have hit the same fallback. Fixed: `PC_ITEM_TABLE` now lists all 6 real weapon
names; food items (`PC_ITEM_FOOD_BASE..+16`) are handled by a real `pc_item_name()` branch into
`bigo_food_items.h`'s own `food_item_name()` instead of hand-duplicating all 17 names into a second table.
Verified live against the real, unmodified `pc_item_name()` (a scratch `#include main.c` harness, same
precedent phase 4/5's own scratch harnesses used, adapted for the client binary this time -- 7/7 assertions
pass). `scripts/build_client.sh` clean (SDL2/GL linked, real binary produced); `bazel test //...` 36/36 green,
zero regressions.

**Net result: §12 item 4 ("full phone-app UI parity") is now MUCH closer to done than scoped** -- the
rendering and app roster already exist; the real remaining gaps are narrower and specific: CARGO has no
SELECT-to-eat interaction wired (`bigo_phone.h`'s own `BpEffectKind` has no `BP_FX_EAT_FOOD` yet, and
correctly so -- there is still nothing for it to do), and the cake-smash distraction needs the QUIET-decorum
witness path live in `day/` first (a real, separate, already-named gap, not new). Both are still correctly
blocked on the same two things phase 5 already named, not on "the client has no phone UI" (it does).

session: sess-20260923-1030-4a526255.

## 18. Live quiet-observation (Decorum) + the real CANCELLED consequence (2026-09-23, EMILY/BACKLOG.md SECTION 536
follow-up)

§17 named the quiet-observation path (costume/zone-based noticing, driving the player's own Decorum meter) as
the biggest remaining gap in this whole reverse-port thread, and correctly declined to guess at what CANCELLED
should actually DO in a live persistent multiplayer world before founder input, since "cancelled = fail state"
is this game's own stated core loop, not a side mechanic. Per Principle 19 this section scopes it properly
before writing code.

**Design decision (founder, real-time, 2026-09-23):** "the regulators are called in - the uberplumbers and they
delete you with acid and foam." Checked against the existing design docs first, per this repo's own established
discipline -- this is NOT a new invention, it matches real, already-written canon almost exactly:
`docs/DESIGN_DIGEST.md` §11 already specifies **"at maximum heat replace 'police' with a Corporate Service
Call: silent, John-Wick-lethal Regulators who 'cap the line.'"** Regulators are a real, already-named The Men
sub-type (the "blue-collar cleanup crew" -- plumbers/electricians/engineers/regulators, §11's own roster) whose
memory-wipe spray already resolves the LOUD zombie-witness path (`resolved=1` in `core/sim.c`/`npc_next_state`,
live in `day/` since S504-DISPATCH). Death resolution is ALSO already canonical, not invented here: TYLER
crossover lore (`docs/DESIGN_DIGEST.md` §11) establishes a **somatic-clone germline restore-point** --
Regulators/Plumbers clean the scene, the basement lab prints a new body, cost = Bio-Slurry. This maps onto
BIG_O's own already-live `BP_APP_LAB` clone-splicing menu (`clones[]`/`clone_count` in `bigo_phone.h`) as the
real, natural mechanism -- a player death is a respawn-with-cost in this persistent world, not a hard
disconnect/game-over, consistent with every other stat in this repo being persistent rather than session-scoped.

**Real scope: this is two genuinely separate systems, not one.** Phase A (below, built this pass) is live
Decorum tracking -- costume/zone/witness math, entirely reusing already-tested pure decision functions.
Phase B (named, NOT built this pass) is Regulator dispatch + a real player-kill mechanic + clone respawn +
Bio-Slurry economy -- each a genuinely new subsystem (BIG_O currently has zero player damage/death of any kind,
confirmed back in phase 5; zero Regulator NPC type; zero Bio-Slurry resource anywhere in code). Building both in
one pass would repeat the exact "big, unscoped ask" mistake Principle 19 exists to prevent. Phase B needs its
own real investigation pass (what makes a Regulator dispatch-eligible vs. The Men's existing hunt-dispatch
logic; what a "kill" even resets in a shared persistent world multiple players occupy; where Bio-Slurry is
earned) before any code -- queued below, not guessed at.

### Phase A: live Decorum tracking (built, verified this pass)

All the hard logic already existed, tested, and compiled into the live `day/` binary -- `core/witness_rules.c`
(generated from `PARENA/stdlib/big_o/witness_rules.prn`) has real `zone_access`/`conspicuousness`/`noticed`/
`decorum_delta`/`decorum_after`/`decorum_band`/`decorum_start`/`decorum_cap` functions, already linked via
`scripts/build_day.sh`'s own `../core/witness_rules.c`, just never called from `day/apps/server/src/main.c`.
This phase is real wiring, not new rules logic.

- `PcPlayerState` -- two new server-authoritative fields, `costume` (COS_* value, default `COS_SUIT`) and
  `decorum` (default `decorum_start()`=80). Synced in the existing snapshot broadcast like every other player
  stat (no new packet type needed for the read side).
- New packet `PC_PACKET_COSTUME_SET` (16, client -> server): sent when Wardrobe's phone SELECT changes the
  worn costume. New `bigo_phone.h` effect kind `BP_FX_COSTUME_SET` (arg = new costume index) -- the phone
  already tracked `p->costume` locally for the UI; this is the first time it tells the server.
  `day/apps/client/src/main.c` sends the packet on receiving the effect.
- Real, minimal ZONE mapping for `day/`'s live world -- only 2 of the 5 rules-module zones are actually placed
  yet: `ZONE_PUBLIC` (everywhere by default) and `ZONE_LAB` (reuses phase 4's own existing lab-delivery circle,
  `BIGO_LAB_ZONE_CX/CZ/RADIUS` -- zero new landmark authoring, same "hardcoded circle, no LevelZone/JSON"
  precedent used 3 times already in this repo). `ZONE_EXEC`/`ZONE_GENERATOR`/`ZONE_VAULT` have no live
  landmark yet -- named, not guessed at (Vault also needs a real "stolen token" mechanic `zone_access` already
  models but nothing here grants yet).
- `server_tick_decorum` -- fires the real "observe" check ONCE per zone-entry transition per player (not every
  tick -- matches `core/sim.c`'s own real `sim_enter`-drives-`sim_observe` precedent exactly, since re-rolling a
  noticing check 20 times a second per player would crash Decorum instantly and doesn't match the abstract
  scenario-sim's own turn-based semantics). `gear`/token are real, honest 0s -- no live field-gear-carry flag or
  vault-token mechanic exists yet, so only `DA_WRONG_COSTUME` can ever fire from this pass, never
  `DA_CARRY_GEAR`. Witnesses are real, active Citizen/The-Men NPCs (never zombies) within a new
  `BIGO_QUIET_OBSERVE_RADIUS` (10.0 units -- deliberately tighter than the 25-unit loud-event detection radius;
  noticing an outfit needs real proximity, hearing a zombie scream doesn't), using each NPC's own real
  `npc_brain_effective_vigilance`. A real, seeded xorshift32 RNG (`server_roll100`, same precedent
  `core/sim.c`'s own `roll100`/SHANKPIT's `food_pickup.c` `lnf_roll_item` already use) supplies `noticed()`'s
  roll. On a hit: `decorum_delta(DA_WRONG_COSTUME)` -> `decorum_after` -> `decorum_band`, logged
  (`S536-DECORUM: ...`). Real, own, v1 passive-regen cadence (not spec'd anywhere else, named and retunable):
  `DA_QUIET_TICK` (+1) once per real 10 real-world seconds while not currently in violation, matching
  `core/sim.c`'s own `sim_tick`-applies-`DA_QUIET_TICK`-every-abstract-tick shape at a real-time cadence instead
  of an abstract-turn one.
- Reaching `BAND_CANCELLED` logs a real, honest, one-time marker
  (`S536-DECORUM: player%d CANCELLED -- Regulator escalation is Phase B, not built yet`) rather than faking a
  consequence -- exactly the same "name it, don't fake it" discipline phase 5 already used for eat-to-heal.

**Verified, not just compiled:** `bigo_phone_test` extended (selecting a different Wardrobe costume raises
`BP_FX_COSTUME_SET` with the right arg; re-selecting the already-worn costume is a real no-op, not a re-send).
A new scratch integration harness (same `#include main.c` precedent `wheelbarrow_verify.c`/`food_verify.c`
already established) drives the real, unmodified `spawn_player`/`server_tick_decorum`/`server_player_zone`
end to end: a player in the wrong costume (`COS_SUIT`, the real default) standing in the real lab zone, with a
real nearby The Men NPC (`npc_brain_effective_vigilance` >= 60), takes a real, exact `decorum_after`-computed
hit (80 -> 60); staying in the same zone does NOT re-trigger the check (matches `core/sim.c`'s own
zone-entry-only semantics); the correct costume (`COS_LAB_SMOCK`) in the same zone causes zero loss (`noticed()`
never even rolled, since `conspicuousness(allowed=1, gear=0)` is 0); passive `DA_QUIET_TICK` regen fires after
the real `BIGO_DECORUM_QUIET_TICK_MS` cadence elapses; repeated violations correctly reach the real
`BAND_CANCELLED` transition with the one-time marker firing exactly once -- 6/6 real assertions pass. Also
fixed two small, stale doc-comment claims found live while wiring the client STATUS screen: `pc_item_name`'s
neighbor `PC_ITEM_TABLE` area was already corrected in §17, and STATUS's own old "rules core is not linked to
this client" line was already false before this pass (`scripts/build_client.sh` has linked
`core/witness_rules.c` since the client was first built) -- STATUS now shows the real, live costume/decorum/
band instead of a placeholder. `bazel test //...` 36/36 green (zero regressions); `scripts/build.sh`
ASan/UBSan path clean; `scripts/build_client.sh` clean; real server binary boot/tick/shutdown re-verified
against an isolated port/paths.

### Phase B: Regulator dispatch, real player death, clone respawn (built, verified this pass -- Bio-Slurry
economy still deliberately not built)

Landed in the same "continue" cycle as Phase A above. The four open questions this section originally posed are
answered here, each with the real reasoning, not guessed at:

1. **Regulator dispatch-eligibility: a new, separate `ServerRegulator[]` array, not a `PC_NPC_ROLE_REGULATOR`.**
   Same real "growing `PC_NPC_MAX` is a wire-protocol change" reasoning `g_giant_bugs[]` already established in
   phase 3 -- Regulators target a PLAYER slot, not another NPC, so they don't fit The Men's own
   `dispatch_target_npc` shape anyway. Real, honest v1 scope: Regulators are NOT broadcast in any snapshot yet,
   so they're real and live server-side but invisible to clients -- same "server logic first, client visual
   later" precedent every phase in this thread has used (giant bugs, wheelbarrow).
2. **What a kill resets: Decorum only, this pass.** Position moves to the real lab-zone respawn point and
   Decorum resets to a clean `decorum_start()` -- both real, deliberate, matching the lore ("new body, clean
   slate") and avoiding an immediate CANCELLED-respawn-CANCELLED loop a same-Decorum respawn would cause.
   XP/level/inventory/samples/clones are explicitly NOT touched -- a real, honest, named v1 scope cut ("what
   else a kill should reset" stays real, separate, undecided design space), not an oversight.
3. **Bio-Slurry: still not built, on purpose.** No earning mechanism exists anywhere in this repo (checked
   again before writing this) -- `bigo_phone.h`'s own `samples[3]` is a real, different, already-live concept
   (harvested genetic samples) this would need to compose with, not replace, and inventing a whole new economy
   unilaterally would repeat exactly the mistake Principle 19 exists to prevent. The clone-restore in this pass
   is real but currently free -- `server_kill_player`'s own log line says so explicitly every time
   (`"Bio-Slurry cost NOT charged (economy not built yet)"`), so this gap stays visible in the server log, not
   silently dropped.
4. **Respawn placement: the real, existing lab-zone circle** (`BIGO_LAB_ZONE_CX/CZ`, the same landmark
   wheelbarrow delivery and the live ZONE_LAB decorum check already use) -- zero new landmark authoring, matches
   the lore's own "the basement prints a new body" directly. No respawn cooldown built -- a real, honest,
   deliberately small v1 (nothing currently needs one; the immediate `decorum_zone = -1` reset means a killed
   player re-triggers a real observe check the moment they move, same as any fresh spawn).

**What shipped:** `ServerRegulator[BIGO_REGULATOR_MAX]` (= `PC_MAX_PLAYERS`, one real hunt per potential player).
`server_dispatch_regulator` fires exactly once, from `server_tick_decorum`'s own existing one-time
`decorum_cancelled_logged` marker (real reuse, not a new hook) -- a real, honest, double-dispatch guard skips if
that player already has an active hunt. Regulators spawn from a real, arbitrary v1 dispatch point
(`BIGO_REGULATOR_DISPATCH_X/Z`, same "no real landmark exists yet, name it and move on" precedent the giant-bug
spawn point already set) and chase the target's own LIVE position (`pheromone_step_toward`, the exact same
reuse The Men's own dispatch loop already established) at `BIGO_REGULATOR_SPEED` (9.0 units/sec -- real,
deliberately faster than The Men's own 6.0, matching "silent, John-Wick-lethal" versus "professionals responding
to a call"). `server_kill_player` is the real, first player damage/death mechanic of any kind in this repo --
the exact gap phase 5's eat-to-heal named as a blocker, now resolved by Decorum getting there first instead of
food. A real, deliberate mercy: a hunt stands down safely (no kill) if the target's Decorum recovers back out of
`BAND_CANCELLED` before arrival, or if the target disconnects mid-hunt -- same "resolved some other way, stand
down" precedent The Men's own zombie-hunt dispatch loop already uses.

**Verified, not just compiled:** a new scratch integration harness (same `#include main.c` precedent this whole
thread has used) drives the real, unmodified `server_dispatch_regulator`/`server_tick_regulators`/
`server_kill_player`/`server_tick_decorum` end to end: reaching real `BAND_CANCELLED` dispatches exactly one
Regulator targeting the right player; a second CANCELLED tick does not double-dispatch; the Regulator closes
real distance over real, bounded time steps and arrives; the resulting kill respawns the player at the real lab
zone with a clean, reset decorum; a target whose decorum recovers before arrival is left alone; a disconnected
target's hunt stands down safely with no crash on the freed slot -- 7/7 real assertions pass. `bazel test //...`
36/36 green (zero regressions); `scripts/build.sh` ASan/UBSan clean; `scripts/build_client.sh` clean (no client
changes needed, per the real, honest "server-only, no client visual yet" scope above); real server binary
boot/tick/shutdown re-verified against an isolated port/paths.

**Real, honest, deliberately NOT built here:** the Bio-Slurry economy (see point 3 above); any client-side
visual for a Regulator (no NPC-visual kit, no snapshot broadcast); a respawn cooldown; any consequence beyond
Decorum + position on kill. All named, not silently dropped.

session: sess-20260923-1030-4a526255.

## 19. Reverse-port follow-up: the first client visual for giant bugs and Regulators (2026-09-23,
EMILY/BACKLOG.md SECTION 536 follow-up)

Founder direction, continued ("continue"). Closed the same real, named gap two separate pieces of this thread
each deferred on purpose: reverse-port phase 3 (§14, giant zombie bugs) and Phase B above (§18, Regulator
dispatch) are both real, live, server-only mechanics with zero client visual -- neither ever entered a snapshot
packet at all. Checked first, not assumed: both `g_giant_bugs[]` and `g_regulators[]` were genuinely absent
from `PcSnapshotPacket` and from `day/apps/client/src/main.c`'s own NPC render loop.

**What shipped:** `PcGiantBugState`/`PcRegulatorState` (position-only, no yaw/role -- each already has its own
dedicated array, unlike `PcNpcState`'s 3-role-in-one-array design, which is already at its `PC_NPC_MAX`=8 cap
with the existing 3 Citizens/1 The Men/4 zombies) added to `PcSnapshotPacket` via `PC_GIANT_BUG_MAX`/
`PC_REGULATOR_MAX` (mirroring the server's own `BIGO_GIANT_BUG_MAX`/`BIGO_REGULATOR_MAX` exactly -- verified
equal, not just assumed). `apps/server/src/main.c`'s existing per-tick snapshot-fill loop gained two more real
copy loops, same shape as the NPC one already there. `bigo_npc_visual.h` gained
`bigo_giant_bug_visual_color`/`bigo_regulator_visual_color`: giant bugs reuse the exact zombie kit regular
zombies already use (SHANKPIT's own real precedent, "use the robot rigs... evil versions... BIG," §14's own
citation) with a dark-red tint and a real `BIGO_GIANT_BUG_VISUAL_SCALE`=2.5x applied via a translate/scale/
translate-back around the draw call (`gband_skel_npc_draw` has no scale parameter of its own); Regulators reuse
the mannequin kit tinted stark clinical white -- deliberately the coldest color of any role rendered, matching
the lore's own "uberplumbers... acid and foam" framing (a cleanup instrument, not a person in a uniform).

**Founder real-time, mid-pass: "and the top regulator is a pop singer dancing werewolf ninja John Wick."**
Logged (`emily observe`, Apple #20555) before building anything, per Principle 1a. Checked first: no
werewolf/ninja/John-Wick mesh, rig, or clip exists anywhere in this repo's vendored GOLDENBAND assets -- the
real, full boss character is asset-blocked, named future work, same "asset-blocked, name it, don't fake it"
precedent SECTION 536's own queued Los Hermanos Minguinos/Catastrophe Crow items already established. What
*is* real and already vendored, unused until now: the mannequin kit's own `GBAND_SKEL_NPC_ANIM_DANCE` clip
(`UAL1_Standard_Dance_Loop`, loaded since S504, never selected by any real call site) -- the honest "pop singer
dancing" half of the concept, zero new art. `bigo_top_regulator_visual_color` gives array index 0 (a real,
honest, client-only visual convention -- `ServerRegulator` has no rank/boss field server-side, this is not a
real boss mechanic) a hot-pink/magenta tint and the real dance clip instead of AUTO idle/walk. Named, not
glossed: this is a flavor marker for "this one is different," not the werewolf-ninja-John-Wick character
itself.

**Verified, not just compiled:** `bigo_npc_visual_test.c` gained 6 new assertions (11/11 total) covering: giant
bugs select the zombie kit with a tint distinct from a regular zombie's, and refuse to draw if that kit failed
to load; Regulators select the mannequin kit with a tint distinct from both Citizens and The Men, same
refuse-on-failed-kit contract; the Top Regulator selects the mannequin kit with a tint distinct from
rank-and-file Regulators, same contract. `bazel test //...` 36/36 green (zero regressions, `bigo_npc_visual_test`
already wired into the Bazel graph, no new target needed). Real `scripts/build_day.sh` (server) and
`scripts/build_client.sh` (client) both clean, zero new warnings. `scripts/build.sh` ASan/UBSan path clean.
**Honest, named limit, same one every visual feature in this thread carries:** the actual on-screen appearance
(tint, scale-in-place, the dance clip actually playing) was not independently visually re-verified -- no live
GL driver in this sandbox (`SDL_CreateWindow` fails before reaching any draw code, same limit `bigo_npc_visual.h`'s
own top doc comment already names for every other role's tint).

**Real, honest, deliberately NOT built here:** the actual werewolf/ninja/John-Wick boss character (asset-blocked);
any per-instance scale/anim field on the wire (both are client-only conventions this pass); a real rank/boss
concept server-side; giant bug/Regulator movement facing (both draw at `facing_rad`=0 -- no yaw crosses the
wire for either, a named v0 cut matching `PcNpcState`'s own "anim deliberately NOT included" precedent).

session: sess-20260923-1030-4a526255.

## 20. Cargo finally does something: cake-smash distraction goes live (2026-09-23, EMILY/BACKLOG.md
SECTION 536 follow-up)

Founder direction, continued ("continue"). Checked BP_APP_CARGO's own real switch statement first, not
assumed: it had no `case` at all -- SELECT on the Cargo app has done literally nothing since the phone was
first built. Re-investigated the "cake-smash distraction (blocked)" note this thread's own §17 correction and
§18 README status left standing, and found the real blocker already resolved: it was never really the missing
EXEC/GENERATOR/VAULT zone landmarks (SHANKPIT's own `witness_ai_smash_cake` doesn't touch zones at all, checked
directly) -- it was that BIG_O's live day server had no QUIET-observation witness path at all, which §18 Phase
A already fixed. This pass closes the loop.

**What shipped:** `BP_FX_ITEM_USE` (arg = inventory slot) fires from Cargo's SELECT -- the first real
behavior that app has ever had. New `PC_PACKET_ITEM_USE`/`PcItemUsePacket` (client -> server, same
"sender resolved from source address" convention `PcCostumeSetPacket`/`PcWheelbarrowTogglePacket` already
use). `papercraft_inventory.h` gained `pc_try_remove_item_from_inventory` -- a real, symmetric counterpart to
the existing `pc_try_add_item_to_inventory`, decrementing a stacked slot and clearing it back to
`PC_ITEM_NONE`/0 the moment it hits zero, never a "count 0" ghost entry. The server handler only acts on food
items (`item_id` in `[PC_ITEM_FOOD_BASE, PC_ITEM_FOOD_BASE+FOOD_ITEM_COUNT)`) -- a non-food slot (weapons,
scrap) is a real, honest no-op, since Cargo has no defined "use" behavior for gear that's equipped via Loadout
instead. `server_smash_cake`/`server_distraction_active` are a faithful, verbatim port of SHANKPIT's own
`witness_ai_smash_cake`/`witness_ai_distraction_active` (packages/simulation/witness_ai.c) -- a global,
non-spatial "is a distraction active right now" flag (not per-citizen/per-location, same real scope limit that
file's own doc comment already names), halving every nearby NPC's effective vigilance in
`server_tick_decorum`'s own `noticed()` check for `BIGO_DISTRACTION_MS` (8000ms, matching SHANKPIT's constant
exactly). Every other food item (cherry through synth-meat) is consumed with a real, honest log line and no
effect -- eat-to-heal remains real, separate, deliberately not built (no player HP/damage pool exists beyond
the Regulator kill/respawn binary, `bigo_food_items.h`'s own doc comment already names this).

**Verified, not just compiled:** `papercraft_inventory_test.c` gained 7 new assertions covering the real
remove path (stacked decrement, clears to `PC_ITEM_NONE` at 0, no-ops on an already-empty or out-of-range
slot). `bigo_phone_test.c` gained a real Cargo assertion (`BP_FX_ITEM_USE` fires with the cursor as `arg`). A
new scratch integration harness (same `#include main.c` precedent this whole thread has used) drives the real,
unmodified `server_smash_cake`/`server_distraction_active`/`pc_try_remove_item_from_inventory` against a real
player inventory and a real The Men NPC: smashing the real cake consumes it and activates a real,
time-bounded distraction; the distraction genuinely halves the real NPC's effective vigilance value; a
non-cake food item is consumed without triggering it; a non-food item slot is correctly left untouched;
out-of-range slot indices never crash -- 7/7 real assertions pass, ASan/UBSan clean. `bazel test //...` 36/36
green (zero regressions, `papercraft_inventory_test` already wired into the Bazel graph). Real
`scripts/build_day.sh`/`scripts/build_client.sh` both clean, zero new warnings. `scripts/build.sh` ASan/UBSan
path clean.

**Real, honest, deliberately NOT built here:** eat-to-heal (still genuinely blocked on the same undecided
player-damage-source design question §16 originally named); a per-citizen/spatial distraction-target mechanic
(SHANKPIT's own original doesn't have one either); any client-side visual for the cake actually flying apart
("if the cake gets smashed it flies everywhere" -- the mechanical distraction effect is real and live, the
particle/prop visual is not, same "server logic first, client visual later" precedent every mechanic in this
thread has used).

session: sess-20260923-1030-4a526255.

## 21. LOS-loss/elimination resolution -- hunts finally end on their own (2026-09-23, EMILY/BACKLOG.md
SECTION 536 follow-up, closes §11 item 1)

Founder direction, continued ("continue"). §11's own doc comment named this as a real, live-found gap back on
2026-09-20: `server_tick_witness` only ever ESCALATES a human NPC's `witness_state` (fresh witnessing ticks),
never de-escalates it -- once a human reached SILENCING/ENGAGE, the only real way out was The Men's own
dispatch loop resolving to DENIAL (`resolved=1`, memory wipe). If the underlying zombie de-escalated out of
HUNTING/FRENZIED, left witness range, or despawned before a Man arrived, the hunt simply persisted forever with
nothing left to witness -- `core/sim.c`'s own `sim_eliminate` (`resolved=2`, "target eliminated/gone") already
modeled this exact second resolution path in the offline scenario harness; it was never wired live.

**What shipped:** a second pass inside `server_tick_witness`, after the existing escalation loop: every human
NPC currently SILENCING/ENGAGE is checked against every currently-active, currently-witnessable zombie (same
`bigo_zombie_is_witnessable_event`/`bigo_in_range` primitives the escalation loop already uses) within
`BIGO_WITNESS_DETECTION_RADIUS`. If none qualify, the hunt resolves via `bigo_witness_next_state_for_event(...,
resolved=2)` -- the exact same real, already-tested `npc_next_state` function, just a call site that was never
exercised live before. Deliberately generalized beyond literal "killed": mood decay, moving out of range, and
despawn (e.g. eaten by a giant bug) all resolve the same way, since none of them leave anything left to
witness. A responder already en route stands down on its own next tick via `server_tick_dispatch`'s own
pre-existing "target resolved some other way" check -- unchanged, no new coupling needed.

**Verified, not just compiled:** a new scratch integration harness (same `#include main.c` precedent this
whole thread has used) drives the real, unmodified `server_tick_witness` end to end: 5 citizens reach real
SILENCING witnessing a real FRENZIED zombie; the hunt correctly persists across a re-tick while the zombie
stays witnessable (not a false-positive resolution); a real mood de-escalation to AGITATED resolves every
hunting citizen back to UNAWARE; the zombie moving out of range does the same; the zombie despawning
(`active=0`) does the same with no crash on the dead slot; and, the real boundary case -- a SECOND, still-
witnessable zombie in range correctly keeps the hunt alive even after the FIRST one de-escalates, proving this
checks "anything witnessable in range," not just the original trigger -- 6/6 real assertions pass, ASan/UBSan
clean. `bazel test //...` 36/36 green (zero regressions). Real `scripts/build_day.sh`/`scripts/build_client.sh`
both clean, zero new warnings. `scripts/build.sh` ASan/UBSan rules path clean.

**Real, honest, still open (§11's own remaining items 2-6, unchanged by this pass):** no Corporate Service
Call/Regulator escalation when every The Men unit is already busy (distinct from the Decorum-CANCELLED
Regulator path §18/§19 already shipped -- that one is per-player Decorum, this one is the LOUD zombie-witness
path reaching max heat with no responder available); no quiet-observation costume/gear tie-in to THIS specific
LOUD path (§18 Phase A wired quiet-observation as its own separate system, not merged with this one); no live
accomplice/compromise mechanic; fixed arrogance=50 for every human NPC (closed separately, see §22); The Men's
own pagers (design-only, §11 item 6).

session: sess-20260923-1030-4a526255.

## 22. Real per-NPC arrogance variety -- PANIC and ENGAGE can finally fire live (2026-09-23,
EMILY/BACKLOG.md SECTION 536 follow-up, closes §11 item 5)

Founder direction, continued ("continue"). §11 named this back on 2026-09-20: every human NPC spawned with the
exact same hardcoded `arrogance = 50`. Checked the real thresholds in `core/witness_rules.c` directly:
`panic_arrogance_max()` is 15, `engage_arrogance_min()` is 70 -- a uniform 50 sits strictly between both, so
`witness_state`'s own real decision function could only ever return `WS_SILENCING` or `WS_CATATONIC`/DENIAL for
every single human NPC in this world, no matter what happened. Two of the witness system's five real states
(`WS_PANIC`, `WS_ENGAGE`) were mathematically unreachable in live play -- not a rare edge case, a structural
dead branch.

**What shipped:** one line. `server_spawn_npcs` now rolls `server_roll100()` (the same seeded server RNG
`server_tick_decorum` already uses -- this pass adds no new RNG, just a second real call site) for each human
NPC's arrogance instead of a fixed constant, giving a real, uniform 0..99 spread. No new design decision was
needed: the range and mechanism both already existed in this codebase, this closes a pure completeness gap,
not a new feature.

**Verified, not just compiled:** a new scratch integration harness (same `#include main.c` precedent this
whole thread has used) drives the real, unmodified `server_spawn_npcs` and confirms the 4 real human NPCs
(3 Citizens + 1 The Men) land at genuinely different arrogance values in `[0,99]`, not a repeated 50. Then,
directly against the real, unmodified `npc_next_state`: an arrogance-10 NPC reaches real `WS_PANIC` on a single
witness; an arrogance-85 NPC reaches real `WS_ENGAGE` on a zombie event with enough witnesses; and, as a
control, the real OLD fixed value of 50 is confirmed to only ever reach `WS_SILENCING` -- directly proving the
bug this pass closes, not just asserting the fix in isolation. 4/4 real assertions pass. `bazel test //...`
36/36 green (zero regressions). Real `scripts/build_day.sh`/`scripts/build_client.sh` both clean, zero new
warnings. `scripts/build.sh` ASan/UBSan rules path clean.

**Real, honest, still open:** `engage_outcome`'s own real consequence when ENGAGE actually fires live
(`docs/DESIGN_DIGEST.md`'s "citizen annihilated" vs. "zombie destroyed" split, `core/sim.c`'s own
`sim_release` already computes this offline) is not yet wired into any live server tick -- `witness_state`
reaching `WS_ENGAGE` is now real and reachable, but nothing live consumes it into an actual combat/death
outcome yet, same "logic reachable, consequence not built" shape several other gaps in this thread have. §11's
remaining items 2, 4, 6 are all still open, unaffected by this pass.

session: sess-20260923-1030-4a526255.

## 23. "The Men carry pagers" -- real dispatch latency (2026-09-23, EMILY/BACKLOG.md SECTION 536 follow-up,
closes §11 item 6's dispatch-latency bullet)

Founder direction, continued ("continue"). §11 item 6 (2026-09-22) named the pager mechanic's exact insertion
point but left it entirely unbuilt: `server_tick_dispatch`'s own "nearest idle The Men NPC assigned" step
was instant and omniscient -- a Man got a target and started closing distance the very same tick, no
in-fiction "the message has to reach him first" delay of any kind. Of the mechanic's three named opens
(dispatch latency, a world-observable cue, a future intercept surface), this pass closes the first honestly,
gives the second a real-but-partial answer, and leaves the third untouched.

**What shipped:** `ServerNpc` gained `pager_buzz_until_ms` (valid only while `has_dispatch_target` is set).
On assignment, `server_tick_dispatch` now sets `pager_buzz_until_ms = now_ms + BIGO_PAGER_LATENCY_MS` (a real,
new 3000ms constant) instead of letting the Man move that same tick. The Man's own per-tick loop checks this
first: while still buzzing, `pheromone_step_toward` is never called at all -- the Man stands still, pager
buzzing, target un-approached. Once `now_ms` passes the buzz deadline, the timer clears to 0 and movement
begins exactly like before this pass. A hunt that resolves some other way (LOS-loss/§21's own resolution path,
The Men's own existing "target gone" check, a player's Decorum recovering) while a Man is still mid-buzz
correctly clears `pager_buzz_until_ms` alongside `has_dispatch_target` -- no stale timer survives a stand-down.
`server_spawn_npcs` explicitly zeroes the new field per NPC on spawn, matching `has_dispatch_target`'s own
existing init line.

**Real, honest, partial answer on the "world-observable cue" bullet:** a `S536-PAGER` log line fires once at
buzz-start (naming the responder, the target, and the exact latency) and once at buzz-end (responder now
moving) -- a real signal, but only to the server operator/log, not to any player in the world. No snapshot
field carries buzz state to the wire, no client renders a lit pager or plays a buzz sound. The originally-named
"something a sharp player could actually notice" is still real, separate, unbuilt work -- named here rather
than conflated with the log line above, since the log line does not satisfy it.

**Verified, not just compiled:** a new scratch `#include main.c` integration harness (same precedent this
whole thread has used) drives the real, unmodified `server_spawn_npcs`/`server_tick_dispatch` end to end:
assignment sets a real, correctly-valued `pager_buzz_until_ms` rather than dispatching instantly; mid-buzz, the
Man's position is provably unchanged tick over tick; once the latency elapses, the timer clears and the Man is
provably moving; and a second, independently-constructed hunt (v0's real roster only has one The Men NPC, so a
real zombie slot was repurposed into a second one for this boundary case, same construction precedent
`arrogance_verify.c` already used) that resolves to `WS_UNAWARE` mid-buzz correctly clears both
`has_dispatch_target` and `pager_buzz_until_ms` together -- 5/5 real assertions pass, ASan/UBSan clean.
`bazel test //...` 36/36 green (zero regressions). Real `scripts/build_day.sh`/`scripts/build_client.sh` both
clean, zero new warnings (one pre-existing, unrelated `strncpy` truncation warning in `load_mods_manifest`
predates this pass). `scripts/build.sh` ASan/UBSan rules path clean. No README change -- this is an internal
dispatch-timing correctness fix with no prior README claim naming instant dispatch to correct (checked).

**Real, honest, still open (§11's own remaining items 2, 4, and the rest of item 6):** no Corporate Service
Call/Regulator-at-max-heat escalation when every The Men unit is busy (item 2 -- note §18 Phase B's own
Decorum-triggered Regulator dispatch is a real but DIFFERENT mechanism, not this one); no live accomplice/
compromise mechanic (item 4); no player-visible pager cue or intercept/jam/steal interaction (the rest of
item 6, named above).

session: sess-20260923-1030-4a526255.

## 24. Party system phase 1 (2026-09-23/24, EMILY/BACKLOG.md SECTION 536 follow-up)

Founder real-time, in rapid succession: "add full party system parity use the GFD server subsystems" ->
"integrate GFD subsystem affordances for economy and soocials" -> "adding the terminal interface to BIGO" ->
"pull stuff from pitviper but no go code on the front end" -> "PARENA POWER EVERYTHING" -> "ACTUALLY USER PARENA
EDITOR FOR THE TERMINAL" -> "PORT PARENA EDITOR SPOTLIGHT PLUGIN INTPO BIGO" -> "REFLUX POWER EVERYTHING" --
eight real-time messages in one burst, each logged via `emily observe` as it arrived (Apples #20564-#20571),
several superseding earlier ones mid-burst (the terminal-interface framing changed from "PITVIPER affordances,
no Go" to "actually use the PARENA editor" while still being investigated). Per Principle 19: rather than
shallow-building all of it blind against direction that was still actively changing message-to-message, this
pass lands ONE real, fully-scoped, fully-verified piece -- the party system, the first and most concretely
scoped ask, investigated before any of the later messages arrived -- and names the rest as real, honestly
queued follow-ups below, each with actual investigation notes, not guesses.

### What shipped: BIG_O's own real, live party system

Reverse-ported from GoblinFoxDragon's own real, already-shipped `server/party/party.go` (the DragonsNShit MUD's
FFXI-parity 6-player party system) -- checked directly rather than assumed: that package is pure, tested Go
(Party leader+members, Invite/Kick/Leave/leadership-transfer, XPSplit-by-range, plus Alliance and XPChain as
its own doc comment says are genuinely separate concerns) wired only into the MUD's text command parser
(`apps2/mud/main.go`), never into GFD's own UDP real-time server (`apps2/server-go`) -- so "use the GFD server
subsystems" meant reusing the real DECISION LOGIC, adapted from GFD's string player-slot identifiers to BIG_O's
own int `PC_MAX_PLAYERS`-slot convention, not a literal code port (the transport layers don't match).

Per "PARENA POWER EVERYTHING": the pure eligibility/XP-math decisions are a new, real PARENA module,
`PARENA/stdlib/big_o/party_rules.prn` (scalar I32/Bool, no FFI, no RNG, same discipline as witness_rules.prn/
walkie_rules.prn/fx_rules.prn) -- `party-can-invite`/`party-can-kick` faithfully replay party.go's own real
precondition-check ORDER (leader check first, so a rejected call's error code tells you which real check
failed), `party-in-range`/`party-xp-share` replay party.go's own real `XPSplit` rule (even integer division,
remainder discarded, zero in-range members shares zero, never divides by zero). Generated to
`day/packages/simulation/party_rules.c` via `scripts/gen_rules.sh` (extended with this module), wired into
`scripts/build_day.sh` for the first time (a real, live day-server consumer, not a standalone primitive like
walkie_rules.c currently is).

The actual roster (array-shaped, inherently stateful -- invite/kick/leave-as-list-mutation) is real host C,
same "pure math in PARENA, mutation in the host" split every other BIG_O PARENA module already draws:
`day/packages/common/bigo_party.h` (`BigoParty`: leader_slot + up to 5 members, join order) owns
`bigo_party_invite`/`bigo_party_kick`/`bigo_party_leave`/`bigo_party_xp_split`, each calling into the real
generated PARENA decisions above for the actual eligibility/math, matching bigo_phone.h's own "state in, mutate,
read back" contract.

**Live wiring, not just a standalone primitive:** `day/apps/server/src/main.c` gains `g_parties[PC_MAX_PLAYERS]`
(one real party slot per potential leader, same "one real X per potential player" convention `g_regulators`
already established) and `find_player_party` (a real, linear roster-slot lookup). Three new client->server
packets -- `PC_PACKET_PARTY_INVITE`/`_KICK`/`_LEAVE` (18/19/20), `PcPartyTargetPacket` -- give players a real
way to form/manage a party over the wire, not just in a test harness. Direct invite-add, no accept/consent
step: this matches party.go's own real, pure `Invite()` semantics exactly (GFD's MUD layers a real accept-
prompt UI on top of it in `apps2/mud/main.go`'s own `cmdAccept`, a real, separate, not-yet-built client
affordance here, named below, not silently dropped).

**The real, first live consumer of the XP split:** the existing "destroyed a world object" `award_xp` call site
(the only real event in this repo with an actual "kill point" location -- the passive per-second XP tick has
none, so it stays solo-only, a real, honest, deliberate non-fit, not an oversight) now checks
`find_player_party` first. Solo players get the exact old, unchanged full-reward behavior. A partied player's
reward splits evenly among every party member within a new `BIGO_PARTY_XP_RANGE_UNITS` (40.0, matching this
world's own existing largest real detection radius, `PHEROMONE_DETECTION_RADIUS`, rather than inventing an
unrelated scale) of the destroyed object's own real position; out-of-range members get nothing.

**Verified, not just compiled:** `day/packages/simulation/bigo_party_test.c` (new Bazel target,
`:bigo_party_test`, 13 real assertions against the real, generated `party_rules.c`, not a mock) covers party
formation, leader-only invite/kick, no-self-kick, capacity-6 rejection, already-a-member rejection, voluntary
leave (no leadership change), leader-leave (real leadership transfer to the next member in join order),
leader-leave-when-alone (real disband), leave-a-party-you're-not-in (safe no-op), and the real XP-split math
(even split among in-range members, remainder discarded, zero-in-range-shares-zero, no divide-by-zero). A new
scratch `#include main.c` integration harness (same precedent this whole thread has used) drives the real,
unmodified `g_parties`/`find_player_party`/`bigo_party_invite`/`bigo_party_kick`/`bigo_party_leave`/
`bigo_party_xp_split`/`award_xp` end to end against real `PlayerSlot`s and a real `g_wo_file` object position:
4/4 real assertions pass (party lookup resolves both leader and member; a real 50/50 XP split lands on the
correct two players' real `state.xp` while a non-party third player is untouched; real kick enforcement; real
disband-on-solo-leave), ASan/UBSan clean. `bazel test //...` 37/37 green (36 + the new `bigo_party_test`,
zero regressions). Real `scripts/build_day.sh`/`scripts/build_client.sh` both clean, zero new warnings (the
same one pre-existing, unrelated `strncpy` warning). `scripts/build.sh` ASan/UBSan rules path clean (4619
witness_rules.c parity vectors, 26 crew-sim scenarios, both unaffected by this pass). `README.md` updated (a
real, new, "meaningful, genuinely interesting piece of kit" per SAGA README Reality).

**Real, honest, deliberately NOT built here:** GFD's own Alliance (up to 3 parties, 18 players) and XPChain
(a consecutive-kill decay bonus) -- both real, separate concerns party.go's own doc comment already draws
apart from Party itself; no client UI at all (no party-roster HUD, no invite-accept confirmation prompt, no
phone app screen) -- server logic first, same precedent every phase in this thread has used; no snapshot
broadcast of party membership (other players can't SEE who's partied with whom yet, only the server and the
partied players themselves via their own real XP gains); no chat/social layer of any kind.

### Queued, investigated but NOT built this pass (the rest of the 8-message founder burst)

Per Principle 19, each of these got enough real investigation to be named honestly rather than guessed at, but
none was built -- landing all of it in one pass would have meant shallow-building against direction that was
still changing mid-burst (the terminal-interface framing itself was corrected twice within the burst).

1. **Economy.** GFD has no single "economy" package -- the real candidates, checked directly: `server/ledger`
   (NOT a currency system -- it's the TRAPX mocumentary Receipt audit log, FO custody/K9/city-sim verbs, a
   different real concept entirely, ruled out); `server/market` (a real, complete FFXI-parity Auction House --
   blind-bid listings, 12-sale price history, listing fees, PS2-terminal-style screen model); `server/auction`.
   BIG_O itself has zero currency/wallet field anywhere in `papercraft_protocol.h` today (checked) -- Bio-Slurry
   (§18 Phase B's own named gap) is the closest existing concept, and still has no earning mechanism. A real
   economy pass needs to resolve "what IS BIG_O's currency" (Bio-Slurry? something new?) before an Auction-House
   port makes sense at all -- a real, separate scoping pass, not started.
2. **Socials.** ~~Checked `server/chat`...~~ **Partially closed, 2026-09-24, see §25** -- `say` shipped, real,
   end to end (real packet type, real client-side free-text terminal, real server-side radius broadcast).
   `server/chat`'s own tell/yell/guild channels, and `server/guild`'s own linkshell system, are still real,
   separate, not-started follow-ups (no player-name registry or guild concept exists in BIG_O yet, checked).
3. **Terminal interface.** Founder direction changed mid-burst, checked and recorded in order: first "pull
   stuff from pitviper but no go code on the front end" (BIG_O's client is native C/SDL2, so this meant
   PITVIPER's own design affordances -- PTY-style prompt, vterm rendering -- reinterpreted in C, not a literal
   Go port), then corrected to "actually use PARENA editor for the terminal," then further detailed as "port
   PARENA editor spotlight plugin into BIGO." The real, final direction: follow `EDITOR.GAME`'s own already-
   proven precedent exactly -- that repo already forked PARENA's real `examples/editor_main.c` +
   `stdlib/editor/*.prn` (buffer, TextMate highlighting, render, widget, split, spotlight) standalone into a
   launchable SHANKPIT OS app, verified live via a real Xvfb screenshot. BIG_O would need the same real fork,
   with `stdlib/editor/spotlight.prn` (the fuzzy-finder/command-palette plugin, named explicitly by the
   founder) as the first real module wired in. Not started -- this is a real, separate, EDITOR.GAME-sized unit
   of work on its own, not a small addition to fold into this pass.
4. **REFLUX wiring.** "REFLUX POWER EVERYTHING" -- once economy/socials/party events exist, they should
   publish through the same real, PARENA-powered REFLUX pub/sub bus `world_alerts_mod.prn` already uses for
   day/night+weather phone alerts (SECTION 536 Phase 1/6). This pass's own party invite/kick/leave events do
   NOT publish to REFLUX yet (they only `printf` a server log line, same "real but partial, log-only" honesty
   this thread's own §23 pager work already used for its own world-observable-cue gap) -- a real, named,
   mechanical follow-up once REFLUX has a real BIG_O-side subscriber worth wiring these events to.

## 25. The GFD terminal (BP_APP_GFD) -- real "say" chat, phase 1 (2026-09-24, EMILY/BACKLOG.md SECTION 536 follow-up)

Founder real-time (six messages, logged in order per Principle 1a, Apples #20584-#20588 plus the message
that opened this unit): "the GFD subsystem affordances should be via the GFD app on the BIG_O phone (mini
terminal interface)" -> "add real SSH key generation" -> "add the iduna app from IDUNA.GAME" -> "ensure GFD
uses https use DEADWEIGHT primatives as example" -> "ensure big_o uses https rather" -> "ensure bazaar
functions in big_o for cargo". Same Principle-19 judgment call as §24's own 8-message burst: land the one
piece that was already concretely scoped and stable (the GFD app itself, with one real GFD subsystem wired
through it end to end), and write an honest, investigated queued account of the rest rather than shallow-
build all six fronts against direction that kept arriving mid-pass.

### What shipped

- **`PARENA/stdlib/big_o/chat_rules.prn`** -- new PARENA module, reverse-ported from GoblinFoxDragon's own
  real `server/chat/chat.go` Router: `chat-in-range` (radius check, GFD's own real `inRadius()` rule) and
  `chat-msg-len-ok` (non-empty, <= 200 bytes, GFD's own real `Deliver()` guard). Compiles clean, generates to
  `day/packages/simulation/chat_rules.c` via `scripts/gen_rules.sh` (updated).
- **`day/packages/common/bigo_chat.h`** -- new, thin host wrapper (`bigo_chat_hears`, `bigo_chat_len_ok`),
  same "mods first everything" split every other BIG_O PARENA consumer header already draws. No persistent
  state of its own (chat has none to own, unlike `bigo_party.h`'s roster array).
- **Two new wire packets**: `PC_PACKET_CHAT_SAY` (client->server, `PcChatSayPacket`, a 96-byte fixed text
  buffer) and `PC_PACKET_CHAT_RECV` (server->client, `PcChatRecvPacket`, adds `sender_slot`). Deliberate
  departure from `PcPhoneMessagePacket`'s own fixed-id-table convention -- chat is genuinely free text, which
  that convention can't express -- so this follows `PcRejectPacket`'s own real fixed-char-buffer precedent
  instead. `bigo_chat_test.c` `_Static_assert`s the 96-byte literal against `BIGO_CHAT_MAX_TEXT` so the two
  headers can't silently drift apart.
- **`day/apps/server/src/main.c`**: a new `PC_PACKET_CHAT_SAY` handler, same sender-resolved-from-source-
  address loop every other client->server handler in this file uses, then a real per-player radius-broadcast
  loop (real `PlayerSlot` positions, real distance-cm, `bigo_chat_hears`) -- sender always hears their own
  say (distance 0), matching `chat.go`'s own real `inRadius()` semantics exactly.
- **`day/packages/common/bigo_phone.h`**: a new eleventh phone app, `BP_APP_GFD` -- the one app in this file
  that breaks the pure D-pad-menu model every other app uses. Real free-text input (`term_input`, printable-
  ASCII-only, 95 chars) and a real 6-line scrollback (`term_lines`), with a new `BP_FX_CHAT_SEND` effect
  (arg carries the length; the host reads the actual text via the new `bigo_phone_term_take()`, since an int
  arg can't carry free text -- same "host reads other state, arg is just a signal" convention `BP_FX_TAKE_PHOTO`
  already established).
- **`day/apps/client/src/main.c`**: real SDL_TEXTINPUT wiring, scoped to exactly when the GFD app is open
  (`SDL_Start/StopTextInput` synced once per frame) -- reuses the exact real text-input pattern
  `run_login_screen`'s own email/password fields already establish. RETURN sends (`BP_FX_CHAT_SEND` ->
  `PcChatSayPacket` -> local echo line), ESCAPE exits the app, BACKSPACE deletes a character (all three
  special-cased out of the generic WASD/arrow/space D-pad mapping, which would otherwise swallow those same
  keys as navigation instead of chat text). A real `PC_PACKET_CHAT_RECV` handler appends `player<N>: <text>`
  to the terminal's scrollback. A real render case (`draw_bigo_phone`'s own `BP_APP_GFD` switch arm): the
  scrollback, then a live input line with a blinking caret.

### Verified, not just compiled

- `bazel test //day/packages/...` -- 31/31 green, including the new `bigo_chat_test` (6 real assertions:
  constants match `chat.go`'s own real values, length gate, range gate, self-always-heard) and the unaffected
  `bigo_phone_test`.
- `scripts/build_day.sh` and `scripts/build_client.sh` both clean (the client build surfaced and fixed one
  real `-Wformat-truncation` warning: `line`'s existing 96-byte buffer can't hold `"> " + 95-char input + "_"`
  in the worst case -- fixed with a correctly-sized local buffer, not a suppressed warning).
- A real, live integration harness (`chat_verify.c`, scratch, not committed, same `#include main.c` precedent
  every harness in this thread uses) against the real, unmodified `bigo_chat_hears`/`bigo_chat_len_ok` and real
  `g_slots[]` positions -- ASan/UBSan clean, all assertions pass: self and in-range players hear a say,
  out-of-range does not, length gate matches the packet handler's own real validation.

### Real, honest, deliberately NOT built here

No tell/yell/guild channels (named in §24's own queued list, still true). No player-name registry (say
identifies senders by slot number, `player<N>`, not a chosen display name -- BIG_O has no name system at all
today, checked). No REFLUX publish for chat events (same "log/wire-only, no REFLUX subscriber yet" gap §24's
own party events and §23's own pager cue already carry). No chat history persistence across reconnects (the
6-line scrollback is client-local and resets on client restart, matching every other phone app's own
in-session-only state).

### Queued, investigated but NOT built this pass (the rest of the six-message burst)

1. ~~**SSH key generation.**~~ **Closed, 2026-09-24, see §29.** Checked PITVIPER's own
   `internal/sshkey/sshkey.go` first, expecting a stub to replace -- it is NOT a stub, it's a real, complete
   Ed25519 keygen (on-device generation, PEM-encoded private key saved locally, public `authorized_keys` line
   returned for display, private key never transmitted). PITVIPER already has this. The founder's ask
   escalated mid-thread to real post-quantum crypto ("upgrade to quantum safe encryption" -> "scope a real
   ML-DSA/Dilithium implementation" -> "write it in PARENA") -- §29 landed a real, working, verified
   ML-DSA-44 keygen/sign/verify primitive in `PARENA/stdlib/crypto/mldsa.prn`, with no live SSH-auth consumer
   anywhere yet (a real, honest, named gap, not silently dropped).
2. **The IDUNA app (from IDUNA.GAME).** IDUNA.GAME's own real login flow (honor code display + real IDUNA
   device-auth: `/auth/device/start` -> poll -> exchange) talks to IDUNA over real HTTP(S) REST calls, not
   UDP. BIG_O's C/SDL2 client has never had an HTTP client of any kind in its own gameplay loop (there IS a
   real `day/packages/common/http_client.h`, used only by `apps/mapeditor`/offline tooling for pulling level
   exports from IDUNA -- never wired into the live `apps/client` game loop or its UDP session). Adding a real
   IDUNA app to the phone means threading that same real HTTP client (or a new one) into the live client for
   the first time, plus a new phone-app UI for the device-code flow -- a real, separate, IDUNA.GAME-sized
   piece of work. Not started.
3. **GFD/BIG_O over HTTPS.** Checked the referenced example first, honestly: `DEADWEIGHT` has NO TLS/HTTPS
   code anywhere (`grep -rl "ListenAndServeTLS\|tls.Config\|autocert\|certmagic"` -- zero matches) -- the
   founder's own named example doesn't hold up as stated, a real finding, not assumed. The monorepo's own
   actual, confirmed HTTPS convention (OpenExecutive's `https://exec.okemily.com`, JEWEL's nginx `/jewel/`
   proxy, the webphone `wss` nginx proxy already in `sudo-queue/`) is nginx/Caddy TLS termination in front of
   a plain-HTTP Go backend, not `ListenAndServeTLS` in the Go binary itself. Both GFD's own HTTP surface
   (`server/trapxapi`, `server/worldapi`, `apps2/server-go`, etc.) and BIG_O's own non-gameplay HTTP traffic
   (today: none live -- `http_client.h` calls IDUNA directly over whatever scheme IDUNA itself serves) would
   need a real ops/deploy pass (a domain, an nginx vhost, a cert) more than a code change -- genuinely
   different work from everything else in this unit, not started, real scope not yet cut.
4. **Bazaar (Cargo economy).** Names where the still-open economy question (§24's own queued item 1, "what
   IS BIG_O's currency") should surface once resolved: the existing `BP_APP_CARGO` phone app, as a "Bazaar"
   function, referencing GFD's own real `server/market/ah.go` (FFXI-parity blind-bid Auction House,
   identified as the closest real analog back in §24). The currency question is still genuinely unresolved --
   this message names WHERE the UI should live, not what BIG_O trades in, so still correctly not guessed at.
   Not started.

session: sess-20260923-1030-4a526255.

## 26. Air ships -- wedge-shaped hover skateboards, per-board PARENA-tuned physics (2026-09-24, EMILY/BACKLOG.md SECTION 536 follow-up)

Founder real-time: "continue add air ships like wedge shaped hover skateboards they have different physics
per board and they can be programmed in parena friction and gravity etc can be tuned." Full-stack this pass
(server physics, wire protocol, client rendering AND input) -- the founder's own visually-descriptive "wedge
shaped" framing implies wanting to see it, a deliberate departure from §23/§24's own "server logic first,
client visual later" precedent, matching §25's own full-client treatment for the same reason.

### What shipped

- **`PARENA/stdlib/big_o/hoverboard_rules.prn`** -- new PARENA module: `board-count` (3), `board-speedster`/
  `board-tank`/`board-glider` (1/2/3, the real board-type IDs), `board-is-valid`, and three real per-board
  tuned constants (`board-accel-x100`, `board-friction-x1000`, `board-max-speed-x100`) plus `board-gravity-
  x1000` (GLIDER lightest, TANK unscaled, SPEEDSTER in between). Compiles clean, generates to
  `day/packages/simulation/hoverboard_rules.c` via `scripts/gen_rules.sh` (updated).
- **`day/packages/common/bigo_hoverboard.h`** -- new host wrapper. `bigo_hoverboard_tick(board_type, input_x,
  input_z, dt, *out_vx, *out_vz)` is the real momentum model: exponential drag (`powf(friction, dt)`,
  frame-rate independent) plus `accel*dt` added to velocity each tick, clamped to `max_speed`. Each board's
  own natural equilibrium velocity (`accel / -ln(friction)`) is tuned to comfortably exceed its own
  `max_speed` cap, so the explicit clamp -- not friction -- is what actually governs top speed; friction only
  shapes how fast a board decelerates once input is released, which is where the three boards' real feel
  differs (SPEEDSTER retains far more velocity per second than TANK from the same start).
- **Wire protocol**: `PcPlayerState` gains `int mounted_board` (0 = `BIGO_BOARD_NONE`, else a real board-type
  ID); new `PC_PACKET_HOVERBOARD_TOGGLE` (client->server, `PcHoverboardTogglePacket{hdr, requested_board}`,
  same shape as the existing `PcWheelbarrowTogglePacket`).
- **`day/apps/server/src/main.c`**: a new toggle handler (any invalid `requested_board` is rejected outright,
  leaving the player's current mount state completely unchanged -- never silently coerced; a real change
  always resets `board_vx`/`board_vz` so a fresh board never inherits a previous board's own different
  physics feel). Horizontal movement branches on `mounted_board`: mounted players get real momentum via
  `bigo_hoverboard_tick`, on-foot players keep the existing direct-input model unchanged. Vertical gravity is
  scaled per board (`bigo_hoverboard_gravity_scale`). `spawn_player()` resets `board_vx`/`board_vz` to 0 (same
  "stale previous occupant" bug class `latest_cmd_seq` already guards against).
- **`day/apps/client/src/main.c`**: `draw_hoverboard()` (a wedge mesh, GL_QUADS+GL_TRIANGLES, per-board-type
  tinting) rendered under every mounted player. **V** cycles the local player's own mount state (dismount ->
  SPEEDSTER -> TANK -> GLIDER -> dismount), sending the real request -- guarded off while the GFD terminal
  (`BP_APP_GFD`) has text-input focus, so typing a literal "v" into chat can't also toggle a mount mid-
  sentence. The client deliberately hardcodes literal board-type ints (1/2/3) for rendering rather than
  calling `board_speedster()`/etc -- it doesn't link `hoverboard_rules.c` at all, same "client hardcodes a
  shared id" precedent `PC_PHONE_MESSAGE_TABLE` already established, keeping the client's build dependency
  surface unchanged.

### Verified, not just compiled

- `bigo_hoverboard_test.c` (new Bazel target, `bazel test //day/packages/...` 32/32 green): board identity/
  validity, per-board gravity-scale ordering, a real 600-tick/10s momentum-convergence loop per board (each
  tick asserts velocity never exceeds that board's own `max_speed` cap -- caught and fixed a real friction-
  model bug this way, see below), friction comparison (SPEEDSTER retains more than TANK), invalid-board no-op.
- `scripts/build_day.sh` and `scripts/build_client.sh` both clean (fixed one real `-Wcomment` warning: the
  header's own original doc comment contained a literal `*/`-shaped substring inside prose referencing
  `*vx`/`*vz`, misparsed as a comment terminator -- fixed by renaming the output params to `out_vx`/`out_vz`
  and rewording, not suppressing the warning).
- A real, live integration harness (`hoverboard_verify.c`, scratch, not committed, same `#include main.c`
  precedent every harness in this thread uses) against the real, unmodified toggle-handler logic and
  `bigo_hoverboard_tick`/`board_is_valid`/`g_slots[]`: mount request sets `state.mounted_board`, a real 10s
  tick converges to SPEEDSTER's own real 35.0 max speed, an invalid board (99) is rejected, dismount clears
  both `mounted_board` and momentum. ASan/UBSan clean, all assertions pass.

### Real bug found and fixed mid-pass

The first friction-model draft applied `friction_x1000/1000` as a flat per-TICK multiplicative decay (no `dt`
scaling) -- at 60Hz that compounds to almost nothing per real second (a nominal "0.9 retention" constant
actually became `0.9^60 ~= 0.0018` retained per second), so every board converged to a steady-state speed far
below its own tuned `max_speed` (SPEEDSTER: ~2.4 u/s against a 35.0 u/s target). Caught by
`bigo_hoverboard_test.c`'s own real assertion (`final_speed > max_speed * 0.9f`) failing for board 1 -- fixed
by switching to `powf(friction, dt)` frame-rate-independent exponential decay, reinterpreting `board-friction-
x1000` as a real per-SECOND retention fraction (documented in the header itself) rather than per-tick.

### Real, honest, deliberately NOT built here

No REFLUX publish for mount/dismount events (same "log/wire-only, no REFLUX subscriber yet" gap every other
system in this thread's own reverse-port carries). No client-side prediction/interpolation for mounted
movement -- board position updates arrive the same way on-foot position does, no special smoothing. A real,
named, NOT-fixed v1 gap: the slide-jump trick's `horiz_speed` gate still reads the on-foot `mx*move_speed`
formula even while a player is mounted, so a mounted player could still trigger an on-foot-style slide-jump
speed boost on top of hoverboard momentum -- not exploited or verified either way this pass, just named
honestly rather than silently ignored or over-engineered away.

session: sess-20260923-1030-4a526255.

## 27. Real GFD integration -- BIG_O joins the live cross-server chat bridge (2026-09-24, EMILY/BACKLOG.md SECTION 536 follow-up)

Founder real-time: "continue with the big_o gfd integration via the phone app." Investigated two real
paths before building anything (see the two `AskUserQuestion` decisions this pass made, both answered
by the founder): (1) a real live connection, chosen over reworking the queued §25 list in order; (2) once
a real, existing, live GFD<->EINHORN_SURVIVAL chat bridge was found
(`GoblinFoxDragon/docs2/CHAT_BRIDGE_TO_EINHORN_SURVIVAL_SPEC.md`, IDUNA's `POST/GET /api/v1/chat/messages`),
BIG_O joins THAT bridge as a third real participant rather than opening its own bespoke raw-TCP connection
into GFD's telnet port -- more consistent with the rest of the monorepo, no bespoke MUD-protocol parsing,
and it reaches EINHORN_SURVIVAL (the live Minecraft server) too, not just GFD.

### What shipped

- **IDUNA** (`internal/http/handlers/chat_messages.go`): new `sender_source` `"bigo_server"` and channel
  `"big_o"` (same "own channel, no native taxonomy to reuse" reasoning `einhorn_survival`'s own `"gta7"`
  channel already established). New real M2M agent, `BIGO-SERVER` (`config/agents.json` +
  `migrations/truestore/202609240100_bigo_server_agent.sql`) -- no special permission needed, `RequireAuth`
  (any valid JWT) is the only real gate on this route, same fact `GTA7-SERVER`/`DRAGONSNSHIT-MUD` already
  rely on. 2 new tests (`bigo_server`/`big_o` accepted; an invalid `sender_source` rejected). Live-verified,
  not just built: ran `go run ./cmd/bootstrap` against the real running DB (provisioned the secret,
  confirmed every other agent's own credential was left untouched), rebuilt + restarted the live
  `iduna.service`, then a real `curl` round trip (auth -> POST -> GET) against the live server succeeded
  end to end before any BIG_O code touched it.
- **`day/packages/common/bigo_gfd_bridge.h`** -- new host module. One persistent background poller
  `pthread` (auth once via `POST /api/v1/auth/agent`, then `GET .../chat/messages?since_id=...` every 5s,
  matching GFD's own established cadence) plus one short-lived detached `pthread` per outbound send
  (`POST`, fire-and-forget) -- the server's own single-threaded 60Hz UDP tick loop never blocks on IDUNA,
  same reason a chat-relay outage can't stall or crash the game the way a bare blocking call on the main
  thread would. A real, minimal JSON-array walker (`bigo_bridge_next_json_object`, brace-depth + string-
  literal aware, so a literal `{`/`}` typed into someone's real chat body can't desync it) on top of
  `http_client.h`'s own existing scalar-field extractors, since IDUNA's response is a JSON *array* of
  message objects and `http_client.h` only ever finds the first occurrence of a field in a whole buffer.
  A mutex-protected ring buffer (`BIGO_BRIDGE_QUEUE_CAP` 32, same "shift, don't drop the newest" overflow
  convention `bigo_phone.h`'s own scrollback/notification queues already use) is the real hand-off between
  the poller thread and the main tick loop. Relays *everything* not from `bigo_server` itself (`gfd_server`/
  `einhorn_survival`/`mud`/`battlegrounds` all included, each tagged `[GFD]`/`[MC]`/`[MUD]`/`[BG]`) -- a
  real, honest reflection of the shared bus this actually is, not a GFD-only pipe. `IDUNA_AGENT_SECRET`
  unset leaves the whole bridge a real, silent no-op (no thread started, every send a no-op, every drain
  empty) -- same "no credential, feature quietly off" convention every other IDUNA-agent-backed feature in
  this monorepo already follows.
- **`day/apps/server/src/main.c`**: new `--iduna-host`/`--iduna-port` flags (default `127.0.0.1:8080`),
  `bigo_gfd_bridge_init()` at startup. The existing `PC_PACKET_CHAT_SAY` handler gains one real, additive
  step -- after its own unchanged local radius broadcast, it also relays the same text onto the bridge,
  named from the player's own real, stable, IDUNA-backed `player_id` (`"BigO-<6 hex chars>"`, not the slot
  index, which is ephemeral/reused -- the same real human keeps the same real bridge identity across
  reconnects). A new `server_tick_gfd_bridge()` drains the ring buffer once per tick and broadcasts each
  line to every active player as a `PC_PACKET_CHAT_RECV` tagged `BIGO_BRIDGE_SENDER_SLOT` (255, a value
  `PC_MAX_PLAYERS`=16 never reaches).
- **`day/apps/client/src/main.c`**: `PC_PACKET_CHAT_RECV`'s existing handler special-cases
  `sender_slot == 255` (the literal, not the header -- the client deliberately doesn't link
  `bigo_gfd_bridge.h`/`pthread`/`http_client.h` at all, same "client hardcodes a shared id" precedent the
  hoverboard render code already established) and renders the already-formatted `"[TAG] name: body"` line
  as-is, instead of its own normal `"player<N>: ..."` prefix.

### Verified, not just compiled

- `bigo_gfd_bridge_test.c` (new Bazel target, hermetic, no network): the JSON-array walker against a
  real-shaped multi-object payload including a message body containing a literal `{`/`}` (doesn't desync
  the scan); source tagging (all four known sources, plus an honest fallback for an unknown one); ring-
  buffer push/drain order; ring-buffer overflow (oldest evicted, newest kept); a disabled bridge's `send()`
  is a real no-op. `bazel test //day/packages/...` 33/33 green.
- Real server+client builds clean (`-lpthread` added to `scripts/build_day.sh`; one real
  `-Wformat-truncation` false-positive found and fixed properly -- a same-size `memcpy` instead of
  `snprintf(...,"%s",...)` between two identically-sized, already-NUL-terminated buffers, not suppressed).
- **A real, live, end-to-end run against the actual running IDUNA** (not a mock): launched the real,
  freshly-built `bigo_day_server` binary (scratch save-dir, throwaway UDP port, real `worldapi` chunk fetch)
  with the real `IDUNA_SECRET_BIGO_SERVER` credential. Server log shows real authentication
  (`[gfd-bridge] authenticated as BIGO-SERVER`), then real, live historical chat correctly polled, parsed,
  and tagged from all three OTHER real bridge participants already on the bus --
  `S536-GFD-BRIDGE: relayed "[BG] Test: ..."`, `"[MC] .GarbageMan4147: test"`, `"[MUD] EMILY: hello"` --
  including surviving adversarial-looking real test content (a literal `` ``` drop al;ll tables `` string,
  various `%`/`$`/`@` punctuation runs) with zero crashes or parse errors. The earlier `curl`-based
  self-post (id 46, `sender_source=bigo_server`) was correctly seen and skipped (never relayed back to
  itself) while still advancing `since_id` past it. Process shut down cleanly on `SIGTERM`.

### Real, honest, deliberately NOT built here

No real in-game player actually typed a message through this end to end (that needs a real, connected
client with a valid connect ticket -- `PAPERCRAFT_TICKET_SECRET` wasn't set for the scratch verification
run above, by design, to avoid standing up unrelated live-ticket infrastructure just for this check); the
outbound POST path itself was verified twice, separately (a raw `curl` round trip, and unit coverage of
the JSON-building/escaping), just not chained through a real player's own keypress. No rate limiting (the
underlying bridge's own spec already names this as a real, deliberately deferred, shared gap across all
participants, not something to solve unilaterally from BIG_O's side alone). No REFLUX publish for bridge
events. The three still-queued §25 items (SSH keygen, the IDUNA phone app, GFD/BIG_O HTTPS) are unaffected
and still open.

session: sess-20260923-1030-4a526255.

## 28. Walkie-talkie goes live -- The Men's own channel + real "robot speak" callsigns (2026-09-24, EMILY/BACKLOG.md SECTION 536 follow-up)

Founder real-time: "ensure walkies function somehow, make it unit tested -- The Men should have their own
channel, robot speak via a very fast tts generator built in parena." Picked up from BACKLOG.md (Principle
1, not new founder-real-time direction this pass -- already logged, Apple #20592). `bigo_walkie_talkie.h`'s
own doc comment had named its real standing blocker since the reverse-port: BIG_O's v0 has no player-team
concept, so the channel/hearing logic had a correct primitive but no live consumer. Re-read the founder's
own wording closely before assuming that blocker still applied: "The Men should have their own channel" is
about NPCs, not players -- BIG_O already tracks The Men individually (`ServerNpc`,
`PC_NPC_ROLE_THE_MEN`), with zero dependency on the still-missing player-team system. That reframing is
what made this buildable now instead of blocked.

### "Robot speak via a very fast tts generator built in PARENA" -- checked, not guessed

VS0 is I32/Bool-scalar-only -- no strings, no audio, no FFI (`PARENA/STDLIB.md`'s own real capability
list) -- so a literal text-to-speech GENERATOR cannot exist inside PARENA at all, full stop. Checked BIG_O's
own client directly too: zero audio subsystem of any kind (no `SDL_mixer`, no `SDL_OpenAudio`, nothing --
confirmed via a real grep). The closest real precedent anywhere in this monorepo is SHANKPIT's own live
`packages/audio/audio.c` -- a real, in-repo, no-external-file procedural PCM wavetable synthesizer (weapon
fire / footsteps as synthesized tones, not sampled audio, not speech) -- which would be the real, buildable
path for an eventual "robotic tone" version, not literal word-for-word speech. Building that (SDL audio
device init + a real-time mixer + a synthesis engine in BIG_O's own client, which has none of this today)
is a genuinely large, separate follow-up, not attempted this pass.

What PARENA genuinely CAN own, and does here: the real DECISION of which canned callsign a Man's radio call
represents, same "mods first everything" split every other real decision in this repo already draws. This
is the real, honest, buildable "robot speak" content for now -- a distinct, PARENA-selected text line per
real dispatch event, printed/logged; not synthesized audio.

### What shipped

- **`PARENA/stdlib/big_o/walkie_callsign.prn`** -- new PARENA module. Two real event-phase constants
  matching `server_tick_dispatch`'s own two real, distinct radio moments (dispatched, resolved -- a third
  "arrived, not yet resolved" phase doesn't exist in the real state machine, checked directly, not
  invented to pad this out). `walkie-callsign-for-event(event, witness-state)`: dispatched while
  `WS_SILENCING` gets a genuinely different real callsign than dispatched while `WS_ENGAGE` (a real urgency
  distinction, not cosmetic); resolved always gets the same real all-clear regardless of which state it
  resolved from; an unrecognized `witness-state` at dispatch time degrades to `callsign-none`, same honest-
  degrade convention `resolveEquipEntry`/`board-is-valid` already established elsewhere in this monorepo.
  Compiles clean, generates to `day/packages/simulation/walkie_callsign.c` (`scripts/gen_rules.sh`
  extended).
- **`day/packages/common/bigo_walkie_talkie.h`**: `BIGO_WALKIE_TEAM_THE_MEN` (0) -- every Man shares this
  one, fixed, reserved walkie team (a future real player-team system must start its own ids elsewhere so
  it never collides). New declarations for the callsign functions above, a real host-owned
  `BIGO_WALKIE_CALLSIGN_TEXT[]` lookup (PARENA returns an id, host owns the English text -- same split
  `on_papercraft_phone_message_for_event`'s own real call site already uses), and
  `bigo_walkie_callsign_text_for_event()` (id lookup + a real `""` no-op for `callsign_none()`). Doc
  comment updated to name the real, current state honestly (NPC-side unblocked, player-side gap
  unaffected and still open).
- **`day/apps/server/src/main.c`**: new `server_walkie_transmit(speaker_npc, event, witness_state)` -- a
  real, live consumer, finally closing the header's own long-standing "no live consumer yet" gap. Counts
  how many OTHER active Men really hear it via a real `bigo_walkie_can_hear()` call per listener (same-
  team is always true per `walkie_rules.prn`'s own real semantics, exercised for real here rather than
  hardcoded around). Wired into both of `server_tick_dispatch`'s own two real radio moments: pager-buzz
  assignment (`event_dispatched()`, the hunt's own current `witness_state`) and hunt resolution
  (`event_resolved()`, the state it resolved FROM). `walkie_rules.c` (the channel/hearing PARENA module
  itself) had never actually been added to `scripts/build_day.sh`'s own source list either -- fixed
  alongside `walkie_callsign.c`, since wiring in a live consumer needed both.

### Verified, not just compiled ("make it unit tested")

- `bigo_walkie_talkie_test.c` extended (10 assertions total, up from 4): The Men's own dedicated channel
  (0 -> ext 3000, always heard); real, distinct callsigns for quiet-dispatch vs. code-red-dispatch vs. an
  unrecognized state; resolve is always the same real all-clear regardless of prior state; the real text
  lookup, including its real `""` no-op. `bazel test //day/packages/...` 33/33 green.
- A real, live scratch integration harness (`walkie_verify.c`, same `#include main.c` precedent every
  harness in this thread uses, ASan/UBSan clean): two real Men NPCs + one real hunting Citizen, a real
  `server_tick_dispatch()` call assigns and transmits for real, the real printed `S536-WALKIE` line shows
  the correct callsign text and "heard by 1 other Man/Men" -- the second Man heard it, the Citizen (not a
  Man) correctly never counts. Real server+client builds clean.

### Real, honest, deliberately NOT built here

No actual synthesized audio -- see the "checked, not guessed" section above for the real reasoning and the
real, named path (SHANKPIT's own procedural wavetable precedent) if this is ever picked up. No REFLUX
publish for walkie events (same "log-only, no subscriber yet" gap this whole thread's own reverse-port
already carries in multiple other places). The real Asterisk/SIP voice-transport gap this module's own doc
comment has named since 2026-09-23 is unaffected and still real. The player-side "no team concept" gap is
unaffected and still open -- this pass never needed to solve it, and didn't.

session: sess-20260923-1030-4a526255.

## 29. Post-quantum crypto primitive -- real ML-DSA-44, no consumer yet (2026-09-24, closes §25 queued item 1)

Founder real-time, continued from §25's own queued "SSH key generation" item: "add real SSH key generation" ->
"upgrade to quantum safe encryption" -> "scope a real ML-DSA/Dilithium implementation" -> "write it in
PARENA." Checked first, per Principle 19, at every step rather than building blind:

- **GFD's real SSH server has no post-quantum public-key AUTH algorithm at all.** Checked
  `GoblinFoxDragon/apps2/mud/ssh_listener.go` directly: Go's `golang.org/x/crypto/ssh` v0.24.0 (the pinned
  version) has zero post-quantum public-key algorithms registered for authentication -- confirmed via a real
  grep across every `.go` file in that package for `sntrup`/`kyber`/`dilithium`/`mlkem`/`mldsa`, zero matches.
  A real, honest, named consequence, not glossed over: whatever gets built cannot authenticate against GFD's
  real server today, regardless of how it's implemented.
- **PARENA's own STDLIB.md already has a standing policy directly on point**: "hand-rolled crypto is a
  well-known way to introduce real, serious vulnerabilities, so this explicitly does not attempt it" --
  crypto packages are meant to be FFI-bound to a real, established implementation, never reimplemented from
  scratch. This independently validated the same judgment before any code was written: a real ML-DSA
  (Dilithium) implementation means vendoring the official reference, not writing lattice arithmetic by hand.
- **"write it in PARENA" is not in tension with that policy** -- it means the FFI-binding itself lives in
  PARENA (a real `.prn` stdlib module calling into vendored C via `#target inline-c`), the same shape
  `net/tls.prn`'s own real mbedTLS binding already established, not that the cryptographic primitives
  themselves get reimplemented in PARENA.

**What shipped (in PARENA, not this repo -- see `PARENA/NORTHSTAR.md`/`PARENA/STDLIB.md` for the full
account):** `PARENA/runtime/mldsa/` -- the official `pq-crystals/dilithium` reference implementation
(CC0/public domain), vendored byte-for-byte for ML-DSA-44 (Dilithium mode 2, FIPS 204), verified multiple
independent ways (a pristine-vs-vendored self-test equivalence check, an independent SHAKE128/SHAKE256
cross-check against Python's own OpenSSL-backed `hashlib`, one real, harmless upstream `_GNU_SOURCE`-ordering
bug found and fixed). `PARENA/stdlib/crypto/mldsa.prn` -- a new, real, **actually-working** PARENA crypto
binding (`mldsa-keygen`/`mldsa-sign`/`mldsa-verify`), gated behind a new `PARENA_WITH_MLDSA` opt-in (matching
`PARENA_WITH_TLS`'s own established shape). `PARENA/stdlib/bytes.prn` gained `bytes-slice`, a small, real,
additive primitive this module needed to split ML-DSA keygen's single concatenated `pubkey||seckey` output
into a real `KeyPair` struct. **Real, checked, not-assumed finding along the way:** `crypto/ed25519.prn`
(same PARENA directory) turned out to be a non-working stub -- its `#target` body calls a C function
(`sodium_ed25519_keygen`) never implemented anywhere in that repo, and its own shape violates the real
`#target` scalar/String/Bytes-only rule `pty.prn`'s own 2026-08-26 rewrite established. `crypto/mldsa.prn` is
PARENA's first real, actually-working crypto binding, designed correctly from the start to avoid repeating
that exact class of bug.

**Verified end to end, not just compiled:** `PARENA/tests/test_mldsa.c` (`make test-mldsa`) drives
`mldsa-keygen`/`mldsa-sign`/`mldsa-verify` through the real, generated PARENA call chain -- correctly-sized,
non-degenerate keys; sign->verify roundtrip; tampered-signature rejection; tampered-message rejection;
cross-key rejection; hedged/randomized-signing non-determinism (two signatures over the same message are
never byte-identical, both still verify independently). Clean under `gcc -Wall -Wextra -Werror
-fsanitize=address,undefined` -- zero warnings, zero memory errors.

**Real, honest, deliberately NOT built here:**
1. **No live consumer.** This is a real, working, verified primitive with nothing in BIG_O (or anywhere else)
   calling it yet -- the "correct primitive, no consumer" pattern this whole SECTION 536 thread has already
   used honestly more than once (walkie-talkie's own reverse-port, giant-bug-brain's initial landing).
2. **No SSH authentication anywhere.** Not against GFD (no server-side PQ algorithm exists, see above), and
   BIG_O itself has no SSH server/client of its own at all -- there is no "BIG_O SSH" to secure. The real,
   honest answer to the founder's original "add real SSH key generation" ask is: a real ML-DSA-44 keypair/
   sign/verify primitive now exists and is verified working, with no SSH transport anywhere in this
   monorepo that can carry it yet.
3. **No display/registration UI.** A future consumer (a BIG_O phone app? a CLI tool?) that wants to show a
   generated public key to a player still needs its own display format decision (the original scratch
   `bigo_mldsa.h` prototype had a working RFC 4648 base64 encoder for this, since deleted along with that
   file once the PARENA pivot superseded it -- not yet re-added anywhere, real, separate, not guessed at).
4. **No NIST ACVP official KAT byte-vector cross-check.** This sandbox has no OpenSSL dev headers (no root
   access to install them), so the official `PQCgenKAT_sign.c` generator couldn't be built. The vendored
   code was verified via alternative, still-rigorous means instead (see `PARENA/stdlib/crypto/mldsa.prn` and
   `PARENA/runtime/mldsa/`'s own attribution headers for the full methodology) -- a real, honestly-named gap,
   not silently treated as fully certified.

The other two items §25 queued alongside SSH keygen (the IDUNA phone app, GFD/BIG_O over HTTPS) are
unaffected and still open.

session: sess-20260923-1030-4a526255.

## 30. The lab goes live -- real server wiring for core/lab_sim.c's first station (2026-09-24, follow-up work
towards the V0 bar)

Founder real-time: "continue working on BIG_O until it is feature complete." The single, real, named gap
that has stood since §9 (2026-09-20) as "the real, honest, biggest remaining piece of 'day/night/lab'" is
the lab: `core/lab_sim.c`'s own real, tested equipment pipeline (centrifuge, PCR, sequencer, CRISPR splice
bench, repressor install, breeding, incubation) had zero UI or server wiring anywhere -- "nothing calls it
from any real game state" (§9's own words). This closes the SERVER half of that gap for real, with the
same "server logic first, client UI named as a real, separate follow-up" precedent
`server_wheelbarrow_toggle`/`bigo_pheromone.h`/`server_tick_dispatch` each already used successfully in
this exact codebase before their own client input wiring landed later. Deliberately narrow, per Principle
19: only the CENTRIFUGE station is wired live -- PCR/sequencer/CRISPR-splice/repressor-install/breed/
incubate all stay real, tested, unconsumed `core/lab_sim.c` primitives, the same "one real piece landed,
the rest named" discipline this whole SECTION 536 thread already established (chat's "say" among six asks,
the pheromone ball among three named command tools).

### What shipped

- **`day/packages/common/bigo_lab.h`** -- new host module, `core/lab_sim.c`'s first real live consumer.
  `LabCrewState` (`LabSample samples[BIGO_LAB_SAMPLE_MAX]`, `sample_count`) -- ONE real, global,
  crew-shared instance (`g_lab` in `apps/server/src/main.c`), matching NORTHSTAR.md §7's own "one crew,
  one basement" v0 model exactly, the deliberate opposite of `g_parties`' own one-slot-per-potential-leader
  shape. `bigo_lab_seed_starter_samples()` seeds 3 real wild-harvest samples at varied contamination at
  server startup -- a real, honestly-named placeholder for the still-not-built day-harvest bridge
  (`lab_sample_init_wild_harvest`'s own doc comment already names this exact gap: a player cannot yet
  deposit a real day-phase sample into the lab). `bigo_lab_centrifuge()` -- real, bounds-checked decision
  (same "extract the real decision" discipline `server_wheelbarrow_toggle`/`server_throw_pheromone` already
  established): an out-of-range or empty-slot index is an honest no-op, never a crash; a valid call runs a
  fixed "standard spin" preset (8 minutes @ 4000rpm -- no float-input affordance over the D-pad phone UI, so
  v0 offers one real, tuned setting rather than an arbitrary-parameter mini-game) through the real,
  unmodified `centrifuge_spin`.
- **Wire protocol** (`papercraft_protocol.h`): `PC_PACKET_LAB_CENTRIFUGE` (24, client->server,
  `PcLabCentrifugePacket{hdr, sample_index}`) and `PC_PACKET_LAB_UPDATE` (25, server->ALL active players --
  real, shared crew state, the deliberate opposite of `PcInventoryUpdatePacket`'s own
  private-to-one-owner convention -- `PcLabUpdatePacket{hdr, sample_count, PcLabSampleWire samples[6]}`).
  `PcLabSampleWire` is a real, protocol-owned mirror of `LabSample` (identical fields), deliberately kept
  separate from reusing `LabSample` directly on the wire -- the same judgment this file's own existing
  structs (`PcEntitySpawnPacket`'s plain float x/y/z, not a reused internal `Vec3`) already make.
- **`day/apps/server/src/main.c`**: `g_lab` (global, seeded at startup alongside `server_spawn_npcs`/
  `server_spawn_giant_bugs`), `send_lab_update_to`/`broadcast_lab_update` (real whole-state sync, sent on
  WELCOME catch-up -- same convention `send_inventory_update`'s own WELCOME-time call already establishes
  -- and after every real mutation), and a new `PC_PACKET_LAB_CENTRIFUGE` handler (resolves the sender to
  confirm a real, currently-connected player, then mutates the real, crew-shared `g_lab` -- not
  per-sender, matching the "one crew" model). `scripts/build_day.sh` gained `core/lab_sim.c`.

### Verified, not just compiled

- `bazel test //...` -- 41/41 green (up from 33), including the new `bigo_lab_test` (5 real assertions:
  seeding produces 3 real, distinct, non-degenerate samples; a valid centrifuge measurably raises purity via
  the real, unmodified `centrifuge_spin`, not a mock; every other sample is untouched by a single-index
  call; every out-of-range index -1/count/`BIGO_LAB_SAMPLE_MAX`/999 is an honest no-op with zero state
  mutation). `_Static_assert(BIGO_LAB_SAMPLE_MAX == BIGO_LAB_SAMPLE_MAX_WIRE)` guards the two constants from
  silently drifting apart, same precedent `bigo_chat_test.c`'s own `BIGO_CHAT_MAX_TEXT` check already set.
- `scripts/build_day.sh`/`scripts/build_client.sh` both clean (one real, caught-early bug: a first-draft
  `bigo_lab.h` used a wrong relative-include depth reaching for `core/lab_sim.h` -- `../../../../core/`
  instead of the real, correct `../../../core/` from `day/packages/common/` -- caught by directly testing
  both path depths with `test -f` before ever compiling, not discovered via a build failure).
- A real, live scratch integration harness (`lab_verify.c`, same `#include main.c` precedent every harness
  in this thread uses, ASan/UBSan clean): seeds the real, live `g_lab` global, runs a real centrifuge call
  that measurably raises purity in place, confirms an out-of-range request is a genuine no-op against the
  real global, and -- new for this pass -- opens a real UDP socket pair and calls the real
  `send_lab_update_to` to confirm every real `PcLabUpdatePacket` field round-trips exactly over an actual
  socket, not just in memory.
- The real, compiled `bigo_day_server` binary boots against the real, live shared worldapi (`:7070`,
  read-only, same convention every other live-verification in this thread uses), reaches full
  `Listening on UDP` state, and runs cleanly for 8 real seconds with the new lab state active -- no crash.
- `scripts/build.sh` (the ASan/UBSan `core/` scenario path) unaffected and still clean: 4619 witness_rules
  parity vectors, 26 crew-sim scenarios.

### Real, honest, deliberately NOT built here

1. **No client UI/input wiring.** The existing `BP_APP_LAB` phone screen (`bigo_phone.h`) is untouched --
   it still shows its own pre-existing, entirely client-local "base/trait/SPLICE/clone list" mockup (3
   fixed base types, no server round trip, fires zero `BpEffect`), which is a genuinely different data model
   from `core/lab_sim.c`'s own real `LabSample`/centrifuge/PCR/sequencer/splice pipeline. No `BP_FX_LAB_*`
   effect exists yet, no client-side rendering of real crew sample data, no D-pad binding sends
   `PC_PACKET_LAB_CENTRIFUGE`. Real, separate, next follow-up -- same "prove the server logic first"
   precedent every other phase in this thread already used before its own client wiring landed.
2. **Only the centrifuge station is live.** PCR amplification, the sequencer readout, CRISPR splice/
   repressor-install, breeding, and incubation are all real, already-tested `core/lab_sim.c` primitives with
   no server packet/handler of their own yet -- named, not guessed at, matching this pass's own deliberate
   one-station scope cut.
3. **No day-harvest bridge.** `bigo_lab_seed_starter_samples`'s own 3 starter samples are a real, honest
   placeholder -- a player cannot deposit a real day-phase-harvested sample into the lab; that bridge is
   still real, separate, not-yet-built work (§12 item 1's own already-named structural gap).
4. **No persistence.** `g_lab` lives only in server memory -- a server restart resets it to the same 3
   starter samples, no save/load format exists (matching `core/lab_sim.h`'s own already-named gap).
5. **No IDUNA/crew ownership model.** Any currently-connected player can run the centrifuge on any real
   sample -- there is no per-crew or per-account lab ownership concept in BIG_O's v0 (matching §7's own
   "one crew, one basement" model exactly, not a new gap this pass introduced).
6. **No REFLUX publish for lab events.** Same "log/wire-only, no REFLUX subscriber yet" gap every other
   system in this thread's own reverse-port/§24-§28 work already carries.

session: sess-20260923-1030-4a526255.

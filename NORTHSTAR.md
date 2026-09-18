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

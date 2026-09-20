# BIG_O: A SHANKPIT Story

> *If they don't see it, it isn't real.*

**BIG_O** is a hard sci-fi, social-stealth management sandbox built on the [SHANKPIT](https://github.com/emilyspringerton/SHANKPIT) engine.

By day, navigate a sun-drenched wasteland overrun by feral biological vectors to harvest pristine, uncorrupted genetic
sequencing data. By night, put on a suit, wash off the blood, and blend into a hyper-paranoid corporate night-society
where acknowledging the apocalypse gets you canceled by the thought police. Build an off-the-books cloning facility in
your basement lab, weaponize the local fauna, and use tactical pheromone arrays to command a subterranean army of
custom-spliced vectors to wage an algorithmic shadow war.

> **Status: early code, not yet a playable game.** Working and tested: the witness/decorum rules core (PARENA -> C),
> a headless co-op crew sim with scenario tests, the Act I Mission 1 rulebook (`core/mission.c`, scenarios 20-23), and the world sim (day/night clock, weather, day zombies + harvest; PARENA rules, REFLUX hooks for mods, scenarios 30-35, [`docs/B2_WORLD.md`](docs/B2_WORLD.md)).
> A `day/` fork of PAPERCRAFT's SDL2 client + server builds (Linux/Windows, CI-bundled) with an in-game phone holding every
> menu. The client can now load a NOCK-authored SHANKPIT level (`--level-id`, `day/packages/common/level_loader.h`) as an
> alternative to the PAPERCRAFT city — verified live against IDUNA's `nextown` (80 walls, 1 spawner, 1 exit) — but it
> defaults to the PAPERCRAFT city still, since no BIG_O-specific level is authored yet, walls render flat-shaded (materials
> aren't parsed yet), and the server side of this fetch isn't wired at all (client-only, so far). Movement now has a real
> walk/sprint split (hold Left Shift to run) — a first step in unifying PAPERCRAFT's own dt-scaled walking pace (4
> units/sec) with SHANKPIT's own real, measured run speed (~59.4 units/sec, converted from its per-tick `MAX_SPEED`),
> since a SHANKPIT-scale level like `nextown` is far too large to cross on foot at PAPERCRAFT's walking pace alone.
> The GOLDENBAND mannequin + animation library are now vendored and wired in (`day/packages/goldenband/`,
> `gband_skel_npc.c`) — the client draws two real, live test NPCs (one human-clip mannequin, one zombie-clip mannequin,
> same shared mesh/skeleton) near the loaded level's spawner, proving the load→skin→animate→draw path end to end.
> No live NPC entity system drives this yet (server has no role-bearing NPCs to spawn), so it's a proof of concept,
> not a populated world. A real, matching AI-brain layer exists alongside it but isn't wired to anything live either:
> `core/humanness.c` (MISHRI-derived mood/timing-jitter primitives, vendored from SHANKPIT), `core/npc_archetype.c`
> (Citizens vs. The Men — a real, dynamic, mood-modulated vigilance feeding `core/witness_rules.c`'s own attention
> mechanism), and `core/zombie_values.c` (zombies' own hunger/aggression/decay vocabulary, deliberately not a human
> mood reskin). See `NORTHSTAR.md` §8 for the full scope and what's deferred. Nothing here has been played end to end
> against a live server.
> Plan: [`NORTHSTAR.md`](NORTHSTAR.md), [`docs/A1M1_PLAN.md`](docs/A1M1_PLAN.md); design sources in [`docs/`](docs/).

## The game in one page

You are a compromised lead geneticist. A routine job goes wrong, the dead get up, and the city reorganizes itself
around pretending it didn't happen. You keep a secret lab in the basement.

| Phase | What you do | Feels like |
|---|---|---|
| **Day** | The sun drives society behind steel shutters; the streets belong to feral zombies. Scavenge raw genetic samples. | survival / scavenging |
| **Twilight & night** | Suit up. The shutters open on a neon mirage of polite society that has agreed the monsters are not real. Clock in, blend in, steal what your lab needs. | Hitman-style social stealth: costumes, shoulder-surfing, tailgating |
| **Basement** | Process samples, splice, clone, tune, and command your own vectors. Your boss cannot replace you. | lab simulation / management |

### The signature mechanic: witnessing

Reality in polite society is whatever has been *socially acknowledged*. The horror is not that you released a zombie, it
is that people *saw it* and can no longer deny it, and admitting they saw it gets them canceled.

- **One witness** goes into catatonic denial: looks away, stares at a phone, talks louder about quarterly numbers.
- **Five or more witnesses** can no longer pretend, so the group turns to *aggressive silencing*: they try to kill you
  to clean the witness list. (Snooty citizens routinely overestimate themselves against a zombie and get annihilated.)
- A **witness you compromise** is an asset: once someone has seen it, they will help cover it up to protect themselves.

### Your vectors

- **Spawning:** clone embryos need soil. Early on you deploy in the park across the street.
- **Command:** pheromone balls (in the spirit of Half-Life 2's antlion pheromone pods) paint a target; your vectors
  path to it. Terrain matters: dirt is fast and hidden ("land sharks" swimming through the earth), concrete is a wall to
  chew through, reinforced vaults are a hard stop until you out-tech them.
- **Upgrades:** splice traits at the bench, at the risk of the science pushing back (off-target edits, unstable lines,
  failed embryos).

## Design pillars

1. **Systemic first.** Depth from simple, interacting rules (witnesses, terrain flags, costumes), not scripted set pieces.
2. **Slow, deliberate, Hitman pacing** in the night phase; the day phase is where things get loud.
3. **The engine is the character.** BIG_O exists to show off what SHANKPIT's primitives can do: dynamic NPC behaviour,
   scriptable levels, level-chained stories.
4. **Multiplayer-first, bots from day one** (house rule for every from-scratch project here). See the plan.

## What's in this repo

| Path | What |
|---|---|
| [`NORTHSTAR.md`](NORTHSTAR.md) | Scoping pass: capability audit against SHANKPIT, the V0 cut, phased plan, open decisions |
| [`docs/DESIGN_DIGEST.md`](docs/DESIGN_DIGEST.md) | Structured digest of the founding design conversation (world, systems, factions) |
| [`docs/transcript/`](docs/transcript/) | The design conversations, reformatted into readable Markdown (4 parts: research, concept/systems, lore/title, espionage/cleanup crew/TYLER crossover) |
| [`docs/a1m1.md`](docs/a1m1.md) | Act I, Mission 1 technical brief |
| [`docs/B1_WITNESS_RULES.md`](docs/B1_WITNESS_RULES.md) | Spec for the implemented witness/decorum/terrain rules |
| [`docs/reviews/`](docs/reviews/) | Reviews of external feedback (e.g. GitHub issue #1) |
| [`docs/source/`](docs/source/) | The untouched raw paste, for the record |
| [`CLAUDE.md`](CLAUDE.md) | Working agreement for AI-assisted development in this repo |

> **Spoilers:** the campaign's factions, the twist, and the endgame choice live in `docs/`. This README stays spoiler-free.

## Built on

- **SHANKPIT** for the engine: level chaining, doors, buttons (REFLUX pub/sub), NOCK-authored levels and characters.
- **PARENA** for rules modules (deterministic decision logic, testable headless), the same pattern as DEADWEIGHT.
- **IDUNA** for accounts, registries and match/session tracking; **NOCK** for level and texture authoring.

## Build

```bash
scripts/gen_rules.sh          # regenerate core/witness_rules.c + parity vectors from PARENA/stdlib/big_o/witness_rules.prn
scripts/build.sh              # ASan+UBSan rules tests (oracle + property + parity vectors), crew sim + scenarios
scripts/build.sh --windows    # + mingw cross-build
```

CI (`.github/workflows/ci.yml`) builds and tests every push; every green push to `main` publishes a GitHub Release.
The rules module is scalar-only PARENA (no FFI), emitted to C (and Java-clean). See [`docs/B1_WITNESS_RULES.md`](docs/B1_WITNESS_RULES.md).

## Naming

"BIG_O" is Big-O notation (an infection that scales out of control), the *Origin of replication* in genomics, and the
hollow zero of monolithic corporate branding. Marketing form: **BIG_O: A SHANKPIT Story**.

## Contributing / process

This repo follows the Emily Way: log founder direction (`emily observe`) before acting, backlog it in `EMILY/BACKLOG.md`,
clean builds first, CI releases from the first commit once code exists, and every commit carries the session trailer.

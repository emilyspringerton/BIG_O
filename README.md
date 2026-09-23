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
> `gband_skel_npc.c`), and the server now spawns a real, live NPC population (3 Citizens, 1 The Men, 4 zombies) and
> broadcasts them in every snapshot — the client renders whichever kit each role calls for (Citizens/The Men share
> the mannequin, tinted apart; zombies get the zombie-clip kit on the same shared mesh). Their brains
> (`core/humanness.c` — MISHRI-derived mood/timing-jitter primitives, vendored from SHANKPIT; `core/npc_archetype.c`
> for Citizens/The Men; `core/zombie_values.c` for zombies' own hunger/aggression/decay vocabulary) tick live on the
> real server now too. Still a proof of concept, not a populated world: NPCs are stationary (no movement/targeting
> yet), spawn on a fixed circle since the server can't load a NOCK level's own spawners yet (client-only,
> `level_loader.h`), and nothing yet feeds a brain's output into an actual witness/attention decision. See
> `NORTHSTAR.md` §8 for the full scope and what's deferred. Nothing here has been played end to end against a live
> server.
> The cloning-facility lab now has a real, tested equipment simulation (`core/lab_sim.c`, 17 statistical tests):
> a centrifuge (spin-time/RPM purity trade-off with real over-spin damage), a PCR thermocycler (amplification with
> compounding cycle damage/contamination creep), a sequencer readout (a genuinely noisy instrument estimate, not
> ground truth), a CRISPR splice bench (guide-RNA off-target mutation, nonsense-mediated decay, and cryptic-splice
> failure as real, distinct outcomes — not just a pass/fail roll), a repressor/kill-switch install with its own
> reliability score, breeding with one-way genetic drift across generations, and embryo incubation viability. This
> is the simulation core only — it is not yet wired into the phone's `BP_APP_LAB` screen or persisted anywhere; that
> UI is still the client-only mockup it always was. See `NORTHSTAR.md` §9 for what's deferred.
> Zombies can now be commanded: press **G** to throw a pheromone marker (`PC_PACKET_PHEROMONE_THROW`), and any
> zombie within range locks on (`has_target` is no longer hardcoded 0) and steers toward it, escalating mood toward
> HUNTING/FRENZIED as it closes in — verified live against a running server. No autonomous player-detection yet (a
> zombie with no thrown marker stays exactly as before), no real projectile arc, and Citizens/The Men still don't
> react to a commanded zombie's presence — that gap is now half-closed (see next): a HUNTING/FRENZIED zombie is a
> real, live witnessed event, verified live producing real `witness_state` escalation in the server log, and The
> Men now have a real dispatch/response loop (nearest idle unit travels to a hunt and resolves it to DENIAL on
> arrival, a real memory-wipe). Honest, live-found limit: the v0 spawn (4 humans total) can't reach the 5-witness
> SILENCING threshold in practice (verified via tests + a larger scratch harness instead). A hunt no longer gets
> stuck forever if The Men never arrive: the moment no witnessable zombie remains in range (it calms down, wanders
> off, or despawns), every human who was hunting it resolves back to UNAWARE on its own — the real LOS-loss/
> elimination resolution path `core/sim.c`'s own offline scenario harness always modeled but this live server
> never wired until now. See `NORTHSTAR.md` §11/§21 for the full account and what's deferred.
> The QUIET-observation half of the witness system — costume/zone-based noticing, driving the player's own
> Decorum meter — is now live too, not just the loud zombie-event path above. Costume (Wardrobe's phone screen)
> is server-authoritative for the first time; standing in the wrong costume for a zone (2 of the rules module's
> 5 zones are actually placed so far: public + the lab) where a real nearby Citizen/The Men NPC notices you drops
> Decorum, using the exact same tested `core/witness_rules.c` math the loud path already uses. Reaching the
> CANCELLED band now has a real consequence: a Regulator is dispatched, chases the player's live position, and
> on arrival kills them — respawning them at the real lab zone (the lore's own "the basement prints a new body")
> with Decorum reset to a clean slate. This is the first real player damage/death mechanic in this repo. Real,
> honest, still not built: the Bio-Slurry cost that lore names for the respawn (no earning mechanism exists
> anywhere yet, so it's currently free and says so in its own server log every time), any client-side visual for
> a Regulator (server-only so far, same "logic first, visual later" precedent every mechanic here has used), and
> a respawn cooldown. See `NORTHSTAR.md` §18 for the full account, the founder's own real design decision on
> what CANCELLED means, and what's deferred.
> Giant bugs and Regulators are visible on screen for the first time -- both were real, live, server-only
> mechanics with no client render path until now. Giant bugs reuse the same zombie kit regular zombies use,
> tinted dark red and scaled 2.5x in place ("evil versions... BIG," no new art). Regulators reuse the mannequin
> kit tinted stark clinical white -- except the first one (a real, honest, client-only visual convention, not a
> server-side rank system), who gets a hot-pink tint and the mannequin kit's own already-vendored dance clip: a
> founder real-time aside ("the top regulator is a pop singer dancing werewolf ninja John Wick") landed as a
> small, honest flavor nod using zero new art, with the full werewolf/ninja/John-Wick character named as real,
> asset-blocked future work rather than guessed at. See `NORTHSTAR.md` §19.
> The Cargo phone app finally does something: selecting an item now consumes it, and smashing the cake
> triggers the real cake-smash distraction (halves nearby Citizen/The Men vigilance for 8 seconds) --
> re-investigated its old "blocked on zone landmarks" note and found the real blocker (no live
> QUIET-observation witness path) already resolved by §18. Every other food item is eaten with no effect yet
> -- eat-to-heal is still real, deliberately unbuilt (no player HP/damage pool exists). See `NORTHSTAR.md` §20.
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
scripts/gen_rules.sh          # regenerate core/witness_rules.c + parity vectors, and day/packages/simulation/{walkie_rules,item_drop_mod,inventory_mod}.c, from PARENA/stdlib/big_o/*.prn
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

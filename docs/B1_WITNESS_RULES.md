# B1 — Witness / Decorum / Terrain rules (spec)

Source of truth for `PARENA/stdlib/big_o/witness_rules.prn` (emitted to `core/witness_rules.c`). Design basis:
`../NORTHSTAR.md` §6-§7, `DESIGN_DIGEST.md` §3-§6, `transcript/02-game-concept-and-systems.md`.

**Legend:** [T] number/rule taken from the founding transcript · [M] my invention (tune freely) · all live in ONE place, the
zero-parameter constants at the top of the `.prn` (`silence-threshold`, `panic-arrogance-max`, ...) plus the small tables in
the functions they belong to.

## What the module is / is not

Pure, deterministic decisions over integers. **No engine, no vision cones, no RNG.** The host (SHANKPIT later, `core/sim.c`
now) decides *who saw what* and hands over counts and pre-rolled integers (`roll` 0..99). Same inputs → same outputs, on
every target. Scalar-only so it emits to C and Java unchanged (Java bonus verified to compile; no changes to any emitter).

## 1. Witness reactions (per NPC, per event)

`count` = **hostile witnesses of one event** (`effective-witnesses(total, accomplices)`: compromised accomplices don't count).

| State | # | When (`witness-state(count, arrogance, compromised, zombie-event)`) |
|---|---|---|
| UNAWARE | 0 | count = 0 |
| DENIAL | 1 | 1 ≤ count < 5 and not a coward [T: "1 witness → catatonic denial"] |
| COMPROMISED | 2 | forced to witness privately / flagged by host: an accomplice who now protects the crew [T] |
| SILENCING | 3 | count ≥ 5 [T: "5+ witnesses → aggressive silencing"] |
| PANIC | 4 | arrogance ≤ 15: flees instead of denying/silencing, at any count ≥ 1 [M] |
| ENGAGE | 5 | count ≥ 5, event is a zombie, arrogance ≥ 70: attacks the *zombie* instead [T: over-confident snobs] |

Precedence: COMPROMISED > (count 0 → UNAWARE) > PANIC > (count < 5 → DENIAL) > ENGAGE/SILENCING.
`npc-next-state(prev, count, arrogance, compromised, zombie-event, resolved)` (revised 2026-09-18 after BIG_O#1):
COMPROMISED is absorbing. **SILENCING/ENGAGE persist while `resolved = 0` even if `count` reaches 0** — losing line of sight
does not end a hunt. They release only on host-reported resolution: **`resolved = 1` cleanup-crew memory wipe → DENIAL**
(The Men's Regulators spray the Memory Methylation Compound; `docs/source/continuation-raw.txt`), **`resolved = 2` attributed
target eliminated/gone → UNAWARE**. A forced compromise still wins (→ COMPROMISED). `resolved` is ignored for non-hunting states.
`is-legal-transition(from, to)` is the checked contract: from COMPROMISED only to COMPROMISED; from SILENCING/ENGAGE only to
itself, UNAWARE, DENIAL or COMPROMISED (never PANIC, never SILENCING↔ENGAGE); everything else is free.
The sim host drives it with `loslost`, `wipe [zone]`, `eliminate P` events (accomplices are never wiped).
`escalation-rank`: UNAWARE/COMPROMISED 0, DENIAL/PANIC 1, SILENCING/ENGAGE 3 — **monotone in count** (tested).
`engage-outcome(arrogance, tier)`: arrogance < 70 → 0 no effect; tier ≥ 1 zombie → 1 citizen annihilated [T]; tier 0 → 2
zombie destroyed **[M — deliberately mine, not the transcript]**: an over-confident citizen kills a tier-0 clone. The
alternative (BIG_O#1 claim C) is that arrogant citizens *always* lose to any zombie ("they get annihilated", as the
transcript has it) so tier 0 → also 1. **Open founder question**; left unchanged until decided.

## 2. Crew attribution (co-op, 1-3 players — NORTHSTAR §7)

An *event* is attributed to one player, or to the whole crew if ≥ 2 crew members are in the event's zone ("seen acting together").
`silence-target-mask(attributed, seen-together, crew-mask)` → bitmask of players the silencing pack hunts. Witness/NPC state is
shared across the crew (an accomplice helps everyone); **Decorum is per player** (below). Solo = crew of 1.

## 3. Decorum meter (per player, 0..100, higher = better standing) [M: the transcript gives concept only]

Start 80. `decorum-after(d, action)` clamps to [0, 100]. Deltas:

| Action | # | Δ | Notes |
|---|---|---|---|
| SMALL_TALK | 0 | +5 | blend-in |
| SAY_APOCALYPSE | 1 | −25 | if noticed |
| CARRY_GEAR | 2 | −15 | field gear (sequencer etc.), if noticed |
| WRONG_COSTUME_NOTICED | 3 | −20 | trespass noticed |
| ATTRIBUTED_EVENT | 4 | −40 | you were the attributed subject of a witnessed zombie event |
| QUIET_TICK | 5 | +1 | recovery |

Bands (`decorum-band`): OK ≥ 60, SUSPICION 30-59, HYSTERIC 1-29, **CANCELLED ≤ 0** [T: Suspicion→Hysteric→canceled].

## 4. Costume × zone access [T for the janitor/generator, lab-coat/lab, suit/exec examples; rest M]

`zone-access(costume, zone, token)` → 1 allowed / 0 trespass. Costumes: SUIT 0, LAB_SMOCK 1, JANITOR 2, STREET 3.
Zones: PUBLIC 0, LAB 1, EXEC 2, GENERATOR 3, VAULT 4.

| | PUBLIC | LAB | EXEC | GENERATOR | VAULT |
|---|---|---|---|---|---|
| SUIT | ✓ | ✗ | ✓ | ✗ | token |
| LAB_SMOCK | ✓ | ✓ | ✗ | ✗ | token |
| JANITOR | ✓ | ✗ | ✗ | ✓ | token |
| STREET (blood/hazmat) | ✗ | ✗ | ✗ | ✗ | ✗ |

Vault needs a stolen token (keycard/PIN, host flag) **and** a non-street costume.

## 5. Noticing (vigilance)

`conspicuousness(allowed, gear)` = (0 if allowed else 40) + (20 if carrying gear) [M].
`noticed(vigilance, conspicuous, roll)` = `roll < min(100, vigilance + conspicuous)` with the host's pre-rolled `roll` 0..99.
Vigilance is per-NPC 0..100 (veteran guard high, tired contractor low) [T concept, M values].

## 6. Terrain and zombie movement [T unless marked]

Tags: DIRT 0 (speed 100%, submerged), CONCRETE 1 (blocked; 500 HP; tier-0 clones do 0, tier-1 "super" do 50/s), REINFORCED 2 (permanent block).
`move-speed-pct`, `wall-max-hp`, `breach-dps`, `wall-hp-after(hp, dps, secs)`.
`zombie-next-state(state, has-target, tag, tier, wall-hp, at-edge)`:
no target → PASSIVE_HEEL [T]; SURFACE_SURGE is sticky while a target remains [M: sprint persists]; DIRT → SUBTERRANEAN_SWIM, or
SURFACE_SURGE at the dirt edge; CONCRETE → tier 0 BLOCKED, tier ≥ 1 WALL_BREACH until HP 0 then SURFACE_SURGE; REINFORCED → BLOCKED.

## 7. Host simulation (`core/sim.c`, `apps/bigo_sim`)

Up to 3 players, N ≤ 16 NPCs, xorshift32, discrete ticks. Events: `enter`, `costume`, `say apocalypse`, `talk`, `gear`, `release
<tier>`, `force <npc>` (private forced witness → accomplice), `tick`. Visibility rule (host, not rules): a loud event
(`release`) is witnessed by every non-accomplice NPC in the same zone; quiet events are noticed per NPC via `noticed()`.
Scenarios (`scenarios/*.txt`) carry `expect` lines and run as tests; logs are byte-deterministic for a given seed+script.

## 7a. Review notes (BIG_O#1, Gemini code review, verified 2026-09-18)

A — partly valid, implemented differently (see §1: persistence + two resolutions). B — invalid: `zone-access` checks the vault/token
first, so SUIT in VAULT without a token already returns 0; no code change, but the oracle now spells out every costume × VAULT × token.
C — design disagreement, not a bug (see `engage-outcome` above). The issue's proposed constant/header names differ from ours; we
already export the same enums in `core/witness_rules.h`, so no change.

## 8. Deliberately not here

Vision/hearing geometry, pathfinding, spawn logistics, cloning/lab, economy, factions/story, networking, real RNG.

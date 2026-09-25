# BIG_O — design digest

A structured summary of the founding design conversation (Gemini chat, 2026-09-18), organized by system rather than by
turn. The full text is in [`transcript/`](transcript/) (readable) and [`source/`](source/) (raw). **Contains spoilers.**
Items marked *(proposed)* are the conversation's suggestions, not decisions; see `../NORTHSTAR.md` §6 for what we adopted.

## 1. Premise

You are a lead geneticist. While doing your job you accidentally cause a zombie outbreak. You harvest DNA, clone your own
zombies to fight the others, hack zombie genomes to improve your clones and build counters, and hunt for a **biological
nullifier** (a deadman switch, like killing a botnet's command-and-control). Along the way you make contact with something
horrifying that turns out to be an ally: a Philip K. Dick *VALIS*-style intelligence that speaks through "garbage" data.

## 2. World and clock

- **Day:** the sun-drenched wasteland. Society is locked behind steel shutters; feral zombies own the streets. The only time
  to scavenge raw, uncorrupted samples (ancestral/fossil DNA, abandoned research sites).
- **Twilight:** put on a suit, wash off the blood, clock in.
- **Night:** the neon mirage. Polite corporate society lives at night "like vampires", commutes, holds mixers, and
  collectively refuses to acknowledge the zombies. Referring to the situation is a social/legal taboo; conspiracy theorists
  are witch-hunted by a thought-police apparatus *(proposed name: Ministry of Public Harmony)*.
- **Basement lab:** off the books; your boss cannot replace you, so it is sovereign territory.
- **Scale:** GTA3-sized small city on existing SHANKPIT primitives, "slow" Hitman/Codename-47 pacing, 3D later, primitives first.

## 3. Society, witnessing and the Decorum meter

- Reality = what is socially acknowledged. "It has never been seen, so it cannot be real."
- **Decorum meter** (in place of a GTA star system): discussing biology, asking why offices have no windows, carrying a
  portable sequencer, etc. flag you as a "Hysteric".
- **Witness states:** 1 witness = *catatonic denial* (look away, stare at a phone). **5+ witnesses = aggressive silencing**:
  they can no longer deny it, so they try to kill you to clean the witness list. Citizens carry a hidden
  Arrogance/SocialStanding value and often overestimate themselves against strong zombies and get annihilated.
- **Compromised witnesses** (someone forced to see a zombie) become accomplices who lie to protect themselves.
- **Framing/blame-shifting** as a weapon: plant evidence on a rival, trigger an internal investigation that clears your heat.
- Corporate **heat tiers** *(proposed)*: 1 = suspicion, 3 = internal audit, 5 = bio-containment breach with tactical teams.

## 4. Stealth and social engineering

Costumes gate which areas are "public" vs "trespassing" (lab smock, janitor overalls, etc.); shoulder-surfing codes and
PINs; tailgating through mantrap doors; per-NPC **vigilance** profiles (veteran guard vs tired contractor); technical-jargon
dialogue checks against real geneticists; air-gapped physical terminals you must reach in person.

## 5. The lab (cloning affordances)

Interaction-grid rather than a crafting menu *(proposed)*: centrifuge (spin time trade-off), micromanipulation/needle
mini-game, a repressor/kill-switch you can sequence into a clone. A **UNIX-style bioinformatics terminal** UI (isolate reads,
filter contamination, CRISPR splice with risk read-outs). Realism hooks the conversation leans on: raw-sample
contamination and read alignment; guide-RNA off-target effects; genetic drift of repeatedly bred lines; nonsense-mediated
decay and cryptic splice sites as failure states; retrotransposon-style pathogen copying. (Game vocabulary, not protocols.)

## 6. Spawning, command and terrain

- **Deployment:** carry clones as compressed *embryo vials* thrown onto soil (grow to full size in ~3 s), or smuggle grown
  ones in a hazmat body bag dressed as a janitor. Early game **requires soil**: spawn in the park across the street.
- **Command tools:** *pheromone balls/darts* (paint a target; clones enter enraged pursuit; the Half-Life 2 antlion
  pheromone-pod idea), *acoustic pingers* (bait wandering zombies), a *hormone emitter* (clones "heel" to you).
- **Zombies vs zombies:** all zombies start as one faction; splicing an altered faction marker makes street zombies see
  your clones as invaders, starting a proxy war.
- **Land sharks:** clones burrow through soil (rippling ground deformation), later chew through concrete foundations and
  erupt into a corporate basement.
- **Terrain flags** *(proposed)*: `MAT_DIRT` (fast, submerged), `MAT_CONCRETE` (blocked until breached; e.g. 500 HP, early
  clones do 0, super-zombies 50/s), `MAT_REINFORCED` (permanent block).
- **AI states** *(proposed)*: `PASSIVE_HEEL`, `SUBTERRANEAN_SWIM`, `WALL_BREACH`, `SURFACE_SURGE`.

## 7. Underground society *(expansion)*

A subterranean "Matrix Zion but more cliché" world: constant rave; humans and zombies coexisting; zombie bouncers and
leashed "domesticated vectors"; a **Bio-Slurry** economy; low-frequency bass that keeps feral zombies docile and jams
surface surveillance. Polite society's currency is social credit.

## 8. Story spine (spoilers)

- **VALIS** is an ancient information matrix in humanity's "junk DNA" (endogenous retroviruses); it speaks through sequencer
  output and, underground, through the bassline.
- **Three factions at war:** feral daylight hordes (overwritten by the awakened code), civilized underworld symbionts, and
  polite corporate night-society (the still-repressed, paranoid elites).
- **Twist:** the evil corporation was the good guy. Forced night-life and enforced denial were a society-scale suppressor
  (stress and decorum keep the junk DNA silenced). They gaslight you throughout ("it's your fault") because that was the only
  way to hold the line. They still look evil.
- **Act II escalation:** the infection jumps species and makes *birds* hyper-intelligent, an organized avian coalition
  (crows coordinating via birdsong ciphers, dropping acoustic beacons to pull feral hordes onto you). The corporation is both
  the cause and the only fix, and VALIS is your ally. Countermeasure *(proposed)*: RNA-interference darts that knock a
  leader bird back to normal intelligence. "Glosslighting" dialogue: your supervisor turns your correct lab data against you.
  **TYLER reconciliation, done (2026-09-25):** the coalition is staffed, in-fiction, by the same watching/archival
  lineage as `TYLER/README.md`'s Eastwind Owls ("their inability to destroy records is a feature, not a bug") --
  "Bird Correction" (TYLER's own recurring end-log line) is adopted as the in-fiction name for the coalition's own
  witness ledger forcing a correction onto the world it watches. `core/avian_values.h`/`.c` gives the coalition a
  real, tested value vocabulary (vigilance/coordination/exposure, ROOSTING/SCOUTING/SIGNALING/MOBBING, a real
  `avian_beacon_strength` output) matching zombies' own `core/zombie_values.h` pattern -- see that header's own top
  doc comment for the full tie-in. Campaign wiring (a playable Act II chapter, `zombie_get_agitated` live
  consumption of the beacon) stays deferred per NORTHSTAR.md §6; only the coalition's own math is real and tested
  so far, 8 real tests in `core/avian_values_test.c`.
- **Endgame choice:** run the corporate script (methylate the population, keep free will under an authoritarian night-society)
  or surrender to the signal (dissolve individuality into one biomass).

## 9. Names and title

Candidates considered: DECORUM PROTOCOL, PUBLIC HARMONY INC., OPTICAL COMPLIANCE, SUBSTRATE SHADOWS, EXON ARCHIVE, GARBAGE
DATA, CRYPTIC SPLICE PROTOCOL, THE JUNK LOGIC ANOMALY. Locked: **BIG_O** (marketing: *BIG_O: A SHANKPIT Story*).
Boot-screen idea: a monochrome SHANKPIT core boot log ending in "BIG_O INITIALIZED".

## 10. What the conversation did not settle

Multiplayer (it designs a single-player sandbox); platform; art direction beyond primitives; economy numbers (all figures
above are illustrative); the actual level layout; how the day/night clock maps onto persistent-world play.

## 11. Additions from the continuation chats (2026-09-18 evening) — see `transcript/04-*.md`

- **Tactical Espionage tech tree** *(proposed, alongside the biology tree)*: the **Lavalier Acoustic-Detonator** (a swapped
  lapel mic that "masquerades as a sniper's bullet": the room hears a rifle crack from across the street, guards take cover
  from the window while you steal the vault token); the **Anubis/Stargate Osiris-Pattern Resuscitator** (single-use revive
  for 120 s; the revived target bleeds lore and door codes before liquefying); **Law & Order forensic sabotage** (luminol-
  masking binder, a "biometric identity patch" false flag that frames an innocent manager and resets your heat). Principle:
  gadgets hack the *social narrative* of a crime scene, not just kill.
- **The Men — the Blue-Collar Cleanup Crew** (plumbers, electricians, engineers, regulators): the muscle that keeps the illusion
  physically true. They scrub scenes back to pristine within ~45 game-minutes (the world gaslights you), keep the
  underground sealed, and at maximum heat replace "police" with a **Corporate Service Call**: silent, John-Wick-lethal
  Regulators who "cap the line". Their memory-wipe spray is what resets witnesses (drives the `resolved` patch to
  `npc-next-state`).
- **TYLER crossover.** The BIG_O world *leaks* into the TYLER documentary universe: a "THE_LEAK" pilot cold open (Detroit kitchen,
  Tyler tipping his hat in 1901 footage, a coin whose date mutates, "if you keep filming, you become part of it"; end log
  "BIRD CORRECTION PENDING"). Witnessing-as-infection is the shared rule. **Tyler dies → wakes at his 08:00 desk** via a
  somatic-clone germline restore-point (Regulators/Plumbers clean the scene, the basement prints a new body, costs Bio-Slurry).
  Cross-repo canon exists: `TYLER/` already has Hana, "Bird Correction", Layer 4 / custody; reconcile before hardening.
- **HANA — the stochastic wildcard.** Appears randomly and never predictably: a host pre-rolled 0–99 picks helpful (0–33: opens a
  vault, drops a jammer), hostile (34–66: says the forbidden thing loudly to wreck your Decorum) or neutral VALIS static
  (67–99: glitches the zone). Signature scare: she is already in the locker the frame the door swings, hands you the suit
  lanyard, and the Plumbers at the bathroom door then read you as a clean employee. Fits our rule that RNG stays in the host.
- **Act I, Mission 1** (`a1m1.md`): Dr. Thorne's reprimand-and-heist brief: get a Level-2 lab smock, keep decorum under a
  Senior Peer's "carding", shoulder-surf a shift supervisor's PIN, physically swap in `FA_REPRESSOR_LOOPS_WGS.fa`, exit via a
  janitor mop and the service elevator.
- **Engineering caveat:** the chats' code snippets are C++ pseudo-code against an invented API (`Spawn_Camera_Rig`,
  `Set_Global_Time_Clock`, ...); SHANKPIT is C, so treat them as intent, not implementation. Numbers (75/40 dps, 45 min, 120 s, 150
  slurry) are illustrative.

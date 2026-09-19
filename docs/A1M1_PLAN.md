# Act I Mission 1 — path to playable (2026-09-19)

Brief: `a1m1.md` (Dr. Thorne, "Disciplinary Reprimand"). This doc = what exists, what is missing, build order.

## Done (rules level, headless)
`core/mission.{h,c}` + scenarios 20-23 (`bigo_sim run`): the whole mission as a deterministic rulebook — night-shift clock
(23:00-03:00, 1 tick = 1 min), smock from the laundry chute, Sector-2 entry, shoulder-surfed PIN (conspicuous: goes through
`sim_observe`), supervisor login/break, flash-drive copy at 10%/min, swap at 100%, janitor mop + service elevator, upload win.
Fail paths: supervisor returns to a foreign drive; window closes; decorum collapses (cancelled) / silencing pack (hunted).
Rejections are logged and asserted (`m!`). Zone mapping is interim: hallway/elevator = GENERATOR, Sector-2 + Room 404 = LAB.

## Design deviations to confirm with the founder
- Brief says "2 Stars" security suspicion; implemented model is Decorum bands (Suspicion < 60, Hysteric < 30). The brief's
  "at 2 Stars, grab a mop and leave" is currently *optional* — nothing forces the exit. Decide: auto-trigger hysteric->must-exit?
- Room 404 uses the LAB zone (smock suffices); the PIN gates the *terminal*, not the door. A separate zone/room object is needed
  once the real level exists.
- "Delightful evening weather" social script (Senior Peer carding) is not modelled: needs an NPC dialogue/challenge event.
- Dr. Aris Thorne vs TYLER's Thorne (`TYLER/lore/eastwind_archive.md`): unverified, still open.

## Gaps for a playable mission (build order)
1. **Deliver the brief through the phone** (Messages app; new message ids for Thorne) — cheapest, uses what exists.
2. **Night level "Sector-2"**: maintenance hallway, laundry chute, Sector-2 floor, Room 404 with a terminal object, service
   elevator, basement lab. NOCK level in the IDUNA registry (doors/exits/characters per SHANKPIT CLAUDE.md), built on `nextown`
   geometry where useful.
3. **Interactables**: laundry chute (smock), terminal (insert/swap), mop, elevator — all map to `mission_*` calls server-side.
4. **Shoulder-surf mechanic**: aim/hold-camera behind the supervisor NPC for N seconds (uses the phone Camera app affordance).
5. **NPCs**: a supervisor with a login/break schedule, a vigilant Senior Peer; needs the humanness layer + witness/perception
   primitive (S504-05).
6. **Mission state on the server** (authoritative), mission HUD/objectives in the phone (Notes app pre-fill), fail/win screens.
7. **Bots** for crew slots; 2-3 player co-op check (mission is written for one player; who holds the drive / who surfs?).

# B2 — World: day/night clock, weather, day zombies (2026-09-19)

Founder direction (2026-09-19): day/night + zombies-by-day, then "weather systems, look at GFD mud for basic weather sim logic",
then "all as PARENA REFLUX powered if possible for mod makers to hook into all of it". Backlog S504-13.

## Architecture: decide in PARENA, hold state in C, announce on REFLUX
| Piece | Where | Role |
|---|---|---|
| `world_rules.prn` | `PARENA/stdlib/big_o/` -> `core/world_rules.c` | ALL decisions/numbers: phase-for-minute, weather cycle/duration/modifiers, area caps, spawn tier. Pure scalar (C+Java emit). Retune a number = edit one defn. |
| `world_alerts_mod.prn` | same -> `core/world_alerts.c` | Worked example REFLUX **subscriber**: polls the log, decides which world events raise a phone message. Never calls world code. |
| `core/world.c` | host | State (minute, weather, zombies), seeded dice, executes decisions, dispatches REFLUX events, polls subscribers. |
| `core/reflux_runtime.c` | host | Append-only `{type,a,b,c}` log, cursor-based polling (no closures in PARENA VS0). Types 101-104 (SHANKPIT's 1-4 are untouched). |

**Hooking in (mod makers):** write a PARENA mod that answers `should-react(type)` + a decision fn, poll the log from your own
cursor (`reflux_log_at`) — see `world_alerts_mod.prn`. Events today: `101 PHASE_CHANGED (old,new,day)`,
`102 WEATHER_CHANGED (old,new,zombie pct)`, `103 ZOMBIE_SPAWNED (area,tier,id)`, `104 ZOMBIE_HARVESTED (area,tier,player)`.
Honest limit: mods can *react* to events and *supply decisions the host asks for*; they cannot yet inject new host actions
(e.g. spawn their own creature type) — that needs a host "command" log the host executes (not built).

## Rules (all in `world_rules.prn`)
- Clock: 1 tick = 1 minute. DAWN 05-07, DAY 07-19, DUSK 19-21, NIGHT 21-05.
- Weather: GFD's fixed cycle Clear → Overcast → Rain → Storm → Clear, random duration per phase (GFD: 300-720 / 180-480 /
  120-360 / 60-240 real seconds; here sim minutes 90-240 / 60-180 / 45-150 / 20-90, same ordering). Mob damage bonus is GFD's
  (Storm +3, Rain +1, exposed for future combat; unused today). Zombie density (100/110/125/150 %) and outdoor sight
  (100/90/75/50 %) are invented.
- Zombies: spawn attempt per area every 10 min while under the area cap. Day caps: wasteland 12, nextown 4, park 2, office/basement 0;
  a third at dawn/dusk; night: wasteland 3, elsewhere 0 (populated areas empty at nightfall). Cap scaled by weather. Tier by roll: 75% scavenger / 20% hound / 5% brute.
- Harvest: kills one zombie in your area, banks a sample of its tier, and you carry field gear (noticed at night by the existing rules).
- Outdoor sight: weather scales NPC vigilance in the PUBLIC zone (`Sim.public_sight_pct`).

## Verified (headless): scenarios 30-35 (clock, caps day/night, weather modifiers, GFD cycle order, harvest carry-over, REFLUX alerts),
determinism + ASan/UBSan clean. **Not built:** zombies as world objects (positions, movement, AI), client/day-slice wiring, alerts -> phone table entries.

## Open tuning / questions
1. **Decorum heals +1 per minute** (B1's quiet tick predates the minute clock): a night's wait fully restores it, weakening the
   day->night heat carry-over. Options: heal per N minutes, or only while not carrying gear.
2. Are zombies in NEXTOWN/PARK by day intended (town is "sun-drenched wasteland" only)? Caps are one number each to change.
3. Weather scale (real seconds -> sim minutes) is invented; storm sight 50% is a guess.

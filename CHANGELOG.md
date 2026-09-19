# CHANGELOG

## 2026-09-19
- feat(sky): configurable procedural skybox + weather rendering (config file, per-weather profiles, F9 reload), client integration, preview tool, unit test (S504-15)
- feat(phone): world feed (clock/weather/zombie counts in header, Map, Status), alert + Thorne A1M1 messages with full-text detail view, Thorne contact; client runs the world sim locally as an interim feed (S504-14)
- feat(world): day/night clock, GFD-derived weather, day-time zombie spawner + harvest, all decided by PARENA (`world_rules.prn`), announced on a REFLUX log with a subscriber-mod example (`world_alerts_mod.prn`); scenarios 30-35; `docs/B2_WORLD.md` (S504-13)
- feat(mission): Act I Mission 1 tracker (`core/mission.{h,c}`), 4 scenarios (clean run + 3 fail paths), `docs/A1M1_PLAN.md` (S504-12)
- feat(day): in-game phone (all menus), zombie clips, Bazel, client+SDL2 bundling (S504-10)

## 2026-09-18
- Ingested the continuation chats (`docs/source/continuation-raw.txt` -> `docs/transcript/04-*.md`), cleaned the Act I Mission 1 brief, reviewed GitHub issue #1 (`docs/reviews/ISSUE_1_REVIEW.md`; one real finding fixed via a `resolved` input on `npc-next-state`), extended the design digest. (S504-09)
- BIG_O#1: hunts (SILENCING/ENGAGE) persist until resolved: memory wipe -> DENIAL, target eliminated -> UNAWARE; vault oracle; 4 scenarios (v0.7.0) (sess-20260918-1725-497f394f)
- B1: witness/decorum/terrain rules (PARENA->C), 3083 parity vectors, crew sim + bigo_sim CLI, 12 asserted scenarios, CI auto-releases (v0.4.0) (sess-20260918-1725-497f394f)
- Ingested the founding Gemini design conversation: raw paste preserved in `docs/source/`, reformatted into three readable
  Markdown parts in `docs/transcript/`, structured spoiler-marked summary in `docs/DESIGN_DIGEST.md`. (S504)
- Replaced the pasted README with a real one (pitch, game loop, witness mechanic, doc index). (S504)
- NORTHSTAR §6: reconciled the scoping pass with what the design conversation actually specifies (five-witness rule,
  terrain flags, costumes, terminal UI, story spine); multiplayer remains the open decision. (S504)

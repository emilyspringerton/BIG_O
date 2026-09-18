# BIG_O — "A SHANKPIT Story"

Scoping-only so far (2026-09-18, S504); no code. Hard sci-fi social-stealth management sandbox on the SHANKPIT
engine: day = harvest genetic data in a vector-infested wasteland (SHANKPIT FPS), night = blend into a paranoid
corporate night-society, basement = off-books lab + pheromone-commanded vector army (async shadow war). Signature
mechanic: the Attention/Heat system ("if they don't see it, it isn't real"). **Read `NORTHSTAR.md` first** — it holds
the V0 cut, the checked capability audit, and the recommended multiplayer-first answer.

## Related repos
`SHANKPIT` (engine, level chain, doors, REFLUX, NOCK characters, humanness/story northstars), `PARENA` (rules modules,
mod idiom), `DEADWEIGHT` (server/bot/league/CI/IDUNA-guest pattern to reuse), `IDUNA` (accounts, NOCK registry), `EMILY`.

## Standing rules (monorepo)
Multiplayer-first, bots from day one; CI releases from the first commit; clean builds first; no FFI added to PARENA
Java; Emily Way (observe → backlog → work → Apple → CHANGELOG → commit+push, `session:` trailer on every commit).
Founder real-time direction goes through `emily observe -s info "Founder real-time: ..."` first.

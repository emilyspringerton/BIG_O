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

## README Reality — SAGA reconciliation (standing instruction, monorepo-wide)

Founder real-time, 2026-09-18: if a change of yours **substantially changes the claim of this project's core README**,
then per SAGA protocols (`EMILY/docs/SAGA_SYSTEM_AUDIT_2026-07-18.md`, HQ-SPEC-DOC-102: intent ↔ claim ledger ↔ reality)
you **must update `README.md` in the same unit of work** so it reflects current reality. The README is the project's public
claim; it must not lag behind the code.

- **When it applies:** a capability is added or removed; status moves ("design only" → "working", "planned" → "shipped");
  the stack, build, run or install steps change; a claim in the README is now false or stale; or you add a **meaningful,
  genuinely interesting piece of kit** (a new tool, engine capability, protocol, pipeline, game system). For that last case
  especially: put it in the README — what it is, how to run it, and its honest status and limits.
- **When it does not:** ordinary fixes, refactors and small features that leave the README's claims true.
- **How:** re-read the README against what you just changed; fix or delete stale lines (including "not built yet" notes that
  are now built); verify any new claim by actually running it, and mark anything untested as untested; commit the README
  with (or immediately after) the change, and mention it in the CHANGELOG entry.

## Core Deps Are PARENA-First (standing, monorepo-wide)

Founder real-time, 2026-10-01: *"always implement core deps in PARENA — when core deps are missing
always implement the core deps in PARENA first."*

- **When a core dependency is missing** (a codec, a protocol client, an inference engine, a
  parser, a data structure — anything this repo's own functionality stands on), implement it in
  PARENA (`PARENA/stdlib/...`) **first**, before building the feature that needs it. Deps first,
  feature second.
- **If PARENA itself can't express the dep yet**, that gap is the real first task: fix or extend
  PARENA (compiler, emitter, or stdlib), with tests, then build the dep on top. Don't route around it.
- **Third-party tools/binaries are stopgaps, not the answer.** Shelling out to or FFI-binding an
  existing tool is allowed only to unblock a demo, and must be labeled as a stopgap in the code and
  in `EMILY/BACKLOG.md` with a PARENA replacement item. (Example: Piper via subprocess for
  MODE_TYLER TTS, 2026-10-01 — stopgap; the PARENA-native synthesis stack is the real work.)
- **Not a license to reimplement the OS.** Core deps = what the product's own behavior depends on.
  Compilers, kernels, system libraries and the like stay as-is; a repo's own CLAUDE.md may record a
  considered, specific exception (same standard as the LZ4 compression convention).

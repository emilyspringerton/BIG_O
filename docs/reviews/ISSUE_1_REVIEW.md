# Review of GitHub issue #1 ("feedback")

Issue #1 (opened 2026-09-18 by the founder, text authored by Gemini) is a code audit of `stdlib/big_o/witness_rules.prn`
plus a "refactored implementation" and a proposed C header. This note records what was checked and decided.

## Who is "Claire"?

Not a hallucinated source. In this monorepo **Claire** is EMILY's shadow-context persona (`EMILY/claire.md.txt`: "the
uncompressed subconscious of the EMILY system"). Gemini has evidently been given that context and is attributing the work to
it. The actual author of `witness_rules.prn` is a Claude Code session (PARENA `41d33ec`, BIG_O `af953cf`), so "Claire's
implementation file" is a misattribution, harmless but worth knowing. The issue's other references (a "franchise bible",
`lore/`, `outlines/`, `engine/`, "Layer 4: Emily OS", "iGenomics", "Mercury Retrograde") point at **TYLER-repo canon**, which
exists (`TYLER/lore/`, `TYLER/engine/`, Hana, "BIRD CORRECTION"); those parts of the issue could not be checked against this
repo and were not needed to judge the logic.

## Findings (each checked against the code as shipped)

| | Claim | Verdict | Action |
|---|---|---|---|
| **A** | `npc-next-state` drops SILENCING/ENGAGE to UNAWARE when the witness count falls to 0 (line of sight lost); once a group has seen it they can never unsee it | **Partly valid.** The step-down when the count reaches 0 is real. But the issue's patch makes the states absorbing *forever* with no exit, which contradicts the new lore: The Men's Regulators spray a memory-wipe compound that resets witnesses, and eliminating the target should release them. | Patched with an explicit `resolved` input: silencing/engage now persist even at count 0 until the host reports a memory-wipe (→ denial) or the target eliminated (→ unaware). See `B1_WITNESS_RULES.md`. |
| **B** | `zone-access` lets a SUIT into the Vault with no token ("skips the zone-4 block") | **Invalid.** Zone 4 (vault) is checked *before* the per-costume branches, so SUIT + VAULT + no token already returns 0. The issue's "Patch B" is logically identical to the shipped code. | No code change. Tests cover every costume × vault × token combination. |
| **C** | `engage-outcome` lets an over-confident citizen kill a tier-0 clone, contradicting the "Snooty Citizen Overconfidence" loop | **Design disagreement, not a bug.** Tier ≥ 1 → citizen annihilated is from the transcript; tier 0 → zombie destroyed is our choice (it gives the player a real risk to weak clones). The issue's patch makes the tier argument unused. | Unchanged; recorded as an open founder question in the spec. |
| — | Proposed `core/witness_rules.h` with `WITNESS_STATE_*` etc. | Equivalent enums already exported (`WS_*`, `DA_*`, `BAND_*`, `COS_*`, `ZONE_*`, `TAG_*`, `ZS_*`). | No change. |
| — | "Additions" to the module (`acoustic-panic-vector`, `osiris-life-span`, `maintenance-escalation-dps`, `cleanup-speed-modifier`) in the continuation chat | Design proposals, not fixes; parameters like `default-window-xyz` are unused, several numbers are invented on the spot. | Not adopted yet; the ideas are captured in `DESIGN_DIGEST.md` for a proper spec pass. |

## Takeaway

One of three audit findings was real (and improved by the new lore), one was wrong, one was a preference. The review's tone
("exceptional... required structural adjustments... will generate execution mismatches under the Shankpit Engine") overstates
the severity; nothing in it affects build or determinism. Suggested reply on the issue, if you want to answer it: *"A: fixed
via a `resolved` input rather than a permanent lock; B: already correct (vault is gated before costume; tests cover it); C:
deliberate, tracked as a design question."*

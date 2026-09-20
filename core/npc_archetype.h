#ifndef BIGO_NPC_ARCHETYPE_H
#define BIGO_NPC_ARCHETYPE_H

// npc_archetype.h -- founder real-time: "start building the AI brain attention mechanisms and
// values and all the mishri stuff for both 'citizens' and also for 'the men' to a certain
// extent." This is the real, direct integration point: core/witness_rules.c ALREADY has a real,
// tested attention mechanism -- vigilance (per-NPC 0..100, "veteran guard high, tired contractor
// low", docs/B1_WITNESS_RULES.md §5) feeding into noticed(vigilance, conspicuous, roll) -- but
// today it's a single STATIC int per NPC (core/sim.h's own SimNpc.vigilance), set once at
// sim_add_npc and never modulated. This module doesn't replace that system; it gives it a real,
// dynamic, per-tick value to read, the same "layer other systems call INTO" architecture
// SHANKPIT's own humanness.c already established (docs live at SHANKPIT/docs/
// HUMANNESS_NORTHSTAR.md, vendored here as core/humanness.h -- MISHRI's own mood/energy/fatigue
// primitives, unchanged).
//
// Two archetypes share this one struct (not two separate modules) because they're both humans
// with the same MISHRI-shaped mood vocabulary, differing only in personality bias -- the exact
// "per-role personality config" extension point SHANKPIT's own NORTHSTAR doc named as deferred
// Phase 4 work, built here as real archetype presets instead of a PARENA-scriptable config (a
// real, honest v0 scope cut, not the eventual end state). Zombies do NOT get a variant of this
// struct -- see core/zombie_values.h's own doc comment for why they need a genuinely different
// vocabulary, not just different tuning numbers.
//
// Citizens ("night-society" NPCs who witness things -- NORTHSTAR.md §1's own witness/gossip
// roster) get a LOW base vigilance and a curious/social/boredom-leaning mood bias: they're not
// paid to notice you, and they get bored and distracted. The Men (design digest §11's "blue-collar
// cleanup crew" -- plumbers/electricians/engineers/regulators, the muscle that keeps the illusion
// physically true) get a HIGH base vigilance and a focused/low-jitter bias: professionals on the
// clock, not gossiping bystanders. Real, honest, not-yet-built next step (named, not silently
// dropped): wiring npc_brain_effective_vigilance into core/sim.c in place of SimNpc's own static
// vigilance field, and into a future live server's own NPC tick -- this module is proven in
// isolation first, same discipline humanness.c's own Phase 1 already used.

#include <stdint.h>
#include "humanness.h"

typedef enum {
    NPC_ARCHETYPE_CITIZEN = 0,
    NPC_ARCHETYPE_THE_MEN = 1
} NpcArchetype;

typedef struct {
    NpcArchetype archetype;
    HumannessState humanness;
    int base_vigilance; // 0..100, witness_rules.h's own real vigilance scale -- the archetype's
                         // baseline BEFORE npc_brain_effective_vigilance's own real mood/energy
                         // modulation (below).
} NpcBrain;

// npc_brain_init -- real, archetype-differentiated defaults. Citizen: base_vigilance 35 (the
// docs' own "tired contractor" end of the 0..100 range), mood bias toward CURIOUS/SOCIAL/BORED
// (humanness_tick_mood's own real reroll picks among these; this just nudges the STARTING mood,
// not the reroll weights, which stay MISHRI's own shared/proven weighting). The Men:
// base_vigilance 85 (the docs' own "veteran guard" end), mood bias FOCUSED, near-baseline
// energy/fatigue (a professional who isn't dragging).
void npc_brain_init(NpcBrain *b, NpcArchetype archetype, uint32_t now_ms);

// npc_brain_tick -- thin real wrapper around humanness_tick_mood; kept as its own call so a
// future per-archetype tick hook (e.g. The Men's own dispatch-timer logic) has a real, obvious
// place to grow into without every caller needing to know that's humanness.c under the hood.
void npc_brain_tick(NpcBrain *b, uint32_t now_ms);

// npc_brain_get_startled -- real wrapper around humanness_get_startled; call when this NPC's own
// witness_state (core/witness_rules.h) transitions into something alarming (SILENCING/PANIC/
// ENGAGE) or the NPC takes damage -- matches humanness.h's own documented "took damage, heard a
// nearby explosion" trigger exactly.
void npc_brain_get_startled(NpcBrain *b, uint32_t now_ms);

// npc_brain_effective_vigilance -- THE real attention-mechanism integration point: base_vigilance
// modulated by this tick's real mood/energy/fatigue, clamped to witness_rules.h's own real 0..100
// scale. A tired, unfocused citizen misses things a startled one wouldn't -- exactly MISHRI's own
// real "mood modulates perception" shape, applied to vigilance instead of MISHRI's own FOV-less
// look-loop (SHANKPIT's own HUMANNESS_NORTHSTAR.md already named keeping the HOST's real
// perception model -- here, witness_rules.c's vigilance/noticed -- and only adding timing/mood
// modulation on top, not replacing it; this is that same discipline applied to BIG_O's own host
// system). Real deltas: STARTLED mood +25 (adrenaline, momentarily hyper-aware), TIRED mood -15,
// each 0.0-1.0 point of fatigue -20, each 0.0-1.0 point of (1-energy) -15 -- additive, then
// clamped to [0, 100].
int npc_brain_effective_vigilance(const NpcBrain *b);

// npc_brain_reaction_delay_ms -- pure passthrough to humanness_reaction_delay_ms. Deliberately NOT
// archetype-scaled here on top of that: humanness.h's own documented design already puts archetype/
// role-tuned timing in the CALLER's own base_ms (matching how story_ai.h's per-role next_attack_ms
// already works) -- this module hijacking base_ms with a second, hidden archetype multiplier would
// fight the caller's own tuning instead of composing with it. Exists as a thin, discoverable
// NpcBrain-namespaced wrapper, not a policy decision.
uint32_t npc_brain_reaction_delay_ms(const NpcBrain *b, uint32_t base_ms);

#endif // BIGO_NPC_ARCHETYPE_H

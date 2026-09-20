#ifndef BIGO_ZOMBIE_VALUES_H
#define BIGO_ZOMBIE_VALUES_H

// zombie_values.h -- founder real-time: "ZOMBIES need to also have their own values and
// attention mechanisms but they are mofe [more] zombie values and behaving you know?" Real,
// deliberate answer to that "you know?": zombies do NOT get an NpcBrain (core/npc_archetype.h)
// with different tuning numbers plugged into the same human mood enum. They get a genuinely
// different vocabulary -- hunger/aggression/decay instead of energy/curiosity/boredom, a 4-state
// mood arc (DORMANT/AGITATED/HUNTING/FRENZIED) instead of MISHRI's 8-state human set, reaction
// timing that gets SLOWER at rest and FASTER (with real, growing erraticism, not just speed)
// under frenzy rather than humanness.c's own startled/tired human curve, and a one-way "decay"
// value with no human equivalent at all. The underlying MATH primitives (a jittered delay
// window, Box-Muller Gaussian noise, a timer-driven mood reroll) are the same real shape
// core/humanness.c already proved -- reimplemented here with zombie-flavored inputs/curves, not
// literally shared code, because the whole point is that the values feeding them are different in
// kind, not just in magnitude.
//
// Real integration boundary, stated plainly: this module is NOT wired into core/witness_rules.c.
// Zombies are the thing citizens/The Men witness (witness_rules.h's own zombie-event flag,
// zombie_next_state's own terrain/tactic state machine), never a witness themselves -- this
// module's own zombie_effective_alertness is a real, separate "how far away can this zombie
// sense a target" analog to witness_rules' vigilance, a future perception-radius hook (not yet
// wired to anything live), not a second implementation of the same system.

#include <stdint.h>

typedef enum {
    ZOMBIE_MOOD_DORMANT = 0,  // no target, ambient shamble -- witness_rules.h's own ZS_PASSIVE_HEEL's natural mood partner
    ZOMBIE_MOOD_AGITATED,     // sensed something (nearby noise/movement), not yet locked on
    ZOMBIE_MOOD_HUNTING,      // has a sustained target, actively pursuing
    ZOMBIE_MOOD_FRENZIED      // high aggression/hunger override -- fastest, most erratic
} ZombieMood;

typedef struct {
    float hunger;      // 0.0-1.0, rises steadily while DORMANT/no target; pushes DORMANT->AGITATED
    float aggression;  // 0.0-1.0, rises while a target is held, decays when lost; drives HUNTING->FRENZIED
    float decay;       // 0.0-1.0, ONE-WAY -- never decreases across any real tick count (a physical-
                        // deterioration clock, not a mood), slowly widens this zombie's own reaction
                        // delay ceiling and lunge inaccuracy over its lifetime
    ZombieMood mood;
    uint32_t mood_change_at_ms; // next real timer-driven mood re-evaluation, same pattern humanness.c uses
} ZombieState;

// zombie_state_init -- fresh spawn: DORMANT, hunger/aggression/decay all 0.0.
void zombie_state_init(ZombieState *z, uint32_t now_ms);

// zombie_tick -- real, per-tick update. has_target (bool-ish int) drives hunger/aggression drift
// and the DORMANT<->AGITATED<->HUNTING mood arc; FRENZIED is only ever entered via
// zombie_get_agitated's own real trigger (a sudden stimulus) or aggression crossing a real, high
// threshold while already HUNTING, never a plain tick-driven drift into it -- frenzy is a spike,
// not a slow climb, matching the real "erratic override" framing above.
void zombie_tick(ZombieState *z, uint32_t now_ms, int has_target);

// zombie_get_agitated -- a real, discrete external stimulus (gunfire, an explosion, a fresh
// wound) forces at least AGITATED (escalates existing HUNTING to FRENZIED instead of downgrading
// it) and schedules a real, short re-evaluation window -- the zombie-flavored analog to
// humanness_get_startled, deliberately not shared code (see this header's own top doc comment).
void zombie_get_agitated(ZombieState *z, uint32_t now_ms);

// zombie_reaction_delay_ms -- returns a real, jittered reaction delay derived from base_ms,
// SLOWER than base_ms at DORMANT/AGITATED (sluggish shamble -- up to 2.5x), roughly base_ms at
// HUNTING, and FASTER but genuinely MORE VARIABLE at FRENZIED (mean below base_ms, real spread
// wider than any other mood -- "erratic", not just "fast"; this is the concrete, testable
// difference from humanness.c's own STARTLED, which is fast AND tight). decay widens every
// mood's own window multiplicatively (a decaying zombie is slower across the board, regardless of
// mood), never narrows it.
uint32_t zombie_reaction_delay_ms(const ZombieState *z, uint32_t base_ms);

// zombie_lunge_noise -- a real Box-Muller Gaussian sample (mean 0) added to an attack-lunge
// angle (degrees), scaled UP by (1 - aggression) (a low-aggression zombie lunges wide and
// sloppy, a high-aggression one lunges true) and further widened by decay -- the same inverse
// shape as humanness_aim_noise's own skill parameter (skill 1.0 -> ~0 noise there too), but
// driven by a genuinely different zombie-side value, not a relabeled skill.
float zombie_lunge_noise(const ZombieState *z, float aggression_skill_0_to_1);

// zombie_effective_alertness -- 0..100, this module's own real "attention" analog to
// witness_rules.h's vigilance -- NOT fed into witness_rules.c (see this header's own top doc
// comment on why). Baseline scales with mood (DORMANT lowest, FRENZIED highest) plus a real
// hunger/aggression contribution, clamped to [0,100].
int zombie_effective_alertness(const ZombieState *z);

#endif // BIGO_ZOMBIE_VALUES_H

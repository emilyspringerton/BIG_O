#ifndef BIGO_WITNESS_LIVE_H
#define BIGO_WITNESS_LIVE_H

/* witness_live.h -- pure glue functions wiring the live ServerNpc/zombie population
 * (day/apps/server/src/main.c) into core/witness_rules.c's own pure decision functions, each real
 * server tick (S504-DISPATCH). Real answer to NORTHSTAR.md §8e item 1 ("wiring
 * npc_brain_effective_vigilance/witness state into an actual noticed()/witness-state decision") --
 * The Men's own real dispatch loop (built in apps/server/src/main.c on top of this) has nothing
 * real to respond to until this exists.
 *
 * Deliberately header-only and pure -- zero ServerNpc dependency, same "extract testable logic"
 * discipline as day/packages/common/bigo_pheromone.h, just living in core/ instead of common/
 * since witness_rules.h/zombie_values.h are core/-only, server-side-only modules the client never
 * links.
 *
 * Real, deliberate scope cut: this wires the LOUD-event half of witness_rules.c only -- a
 * HUNTING/FRENZIED zombie is a "loud event", unconditionally witnessed by every human NPC within
 * range, exactly matching docs/B1_WITNESS_RULES.md §7's own "a loud event... is witnessed by every
 * non-accomplice NPC in the same zone" rule (core/sim.c's own sim_release, scaled from zone-wide
 * to radius-based since the live server has no per-NPC zone concept yet). The QUIET-observation
 * path (costume/gear noticing via noticed()/conspicuousness(), core/sim.c's own sim_observe) is a
 * real, separate, still-open gap -- it needs a live decorum/costume system this pass does not
 * build, and is NOT what this header wires. */

#include "witness_rules.h"
#include "zombie_values.h"

#define BIGO_WITNESS_DETECTION_RADIUS 25.0f /* how far a human NPC can witness a loud zombie event */
#define BIGO_DISPATCH_ARRIVAL_RADIUS 2.5f   /* how close The Men must get to resolve a hunt */

/* bigo_zombie_is_witnessable_event -- true if this zombie's CURRENT mood counts as a real,
 * noticeable "loud event". DORMANT/AGITATED zombies are just shambling -- not yet a witnessed
 * event, matching the digest's own framing that the zombies themselves aren't the horror, being
 * SEEN acting monstrously is. */
static inline int bigo_zombie_is_witnessable_event(ZombieMood mood) {
    return mood == ZOMBIE_MOOD_HUNTING || mood == ZOMBIE_MOOD_FRENZIED;
}

/* bigo_in_range -- pure flat (x,z)-plane distance check, matching this server's own real
 * movement model (y is cosmetic everywhere else in this codebase too, see bigo_pheromone.h). */
static inline int bigo_in_range(float ax, float az, float bx, float bz, float radius) {
    float dx = ax - bx, dz = az - bz;
    return (dx * dx + dz * dz) <= radius * radius;
}

/* bigo_witness_next_state_for_event -- one human NPC's own real witness-state transition to a
 * loud zombie event, a thin, explicit wrapper around witness_rules.c's own real npc_next_state
 * (zombie_event=1 always, compromised=0 always -- this pass has no forced-compromise mechanic
 * live yet, that stays core/sim.c's own scenario-only feature for now). `count` is the caller's
 * own real effective_witnesses() of every human currently in range of THIS SAME event -- computed
 * once per event, shared across every human witnessing it, exactly matching core/sim.c's own
 * sim_release. `resolved` is 0 for a fresh witnessing tick, 1 when The Men's own dispatch loop
 * resolves the hunt (memory-wipe -> DENIAL). */
static inline int bigo_witness_next_state_for_event(int prev_state, int count, int arrogance, int resolved) {
    return npc_next_state(prev_state, count, arrogance, 0, 1, resolved);
}

#endif /* BIGO_WITNESS_LIVE_H */

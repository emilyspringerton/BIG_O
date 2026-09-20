#ifndef BIGO_PHEROMONE_H
#define BIGO_PHEROMONE_H

/* bigo_pheromone.h -- pure, header-only pheromone command logic (S504-PHEROMONE, docs/
 * DESIGN_DIGEST.md §6's "pheromone balls/darts (paint a target; clones enter enraged pursuit)").
 * Factored out of apps/server/src/main.c's own server_tick_npcs specifically so the real targeting
 * and steering decisions are testable headlessly -- same "extract pure logic, no live server
 * needed" discipline bigo_npc_visual.h already established for the render side.
 *
 * Real scope, named: this closes half of NORTHSTAR.md §8e item 2 ("real movement/targeting") --
 * zombies can now be given an explicit has_target + a real steering destination by a player action
 * instead of has_target being hardcoded 0. It does NOT give zombies autonomous player-detection
 * (sensing a nearby player/citizen on their own, with no thrown marker) -- that's a separate,
 * still-open half of the same NORTHSTAR item, deliberately not conflated with this real, player-
 * driven command mechanic.
 */

#include <math.h>

#define BIGO_PHEROMONE_MAX 4 /* small, bounded cap -- matching this repo's own PC_NPC_MAX/
    PC_WO_MAX_OBJECTS precedent; a few concurrent thrown markers is plenty for a 1-3 player crew */

typedef struct {
    int active;
    float x, z; /* world-space target; y is cosmetic (marker drop height), not used for targeting */
    unsigned int expires_at_ms;
} PheromoneMarker;

/* pheromone_marker_expire -- real, one-way expiry: deactivates any marker whose expires_at_ms has
 * passed. Call once per server tick BEFORE any targeting decision reads the array, so a stale
 * marker never draws a zombie in on the same tick it expires. */
static inline void pheromone_marker_expire(PheromoneMarker markers[], int count, unsigned int now_ms) {
    for (int i = 0; i < count; i++) {
        if (markers[i].active && now_ms >= markers[i].expires_at_ms) markers[i].active = 0;
    }
}

/* pheromone_claim_slot -- finds a free (inactive) marker slot to throw a new one into; if every
 * slot is occupied, real, deliberate behavior is to evict the SOONEST-EXPIRING marker (the one
 * closest to fading on its own) rather than refusing the throw outright -- a player's live command
 * should never silently do nothing because an old, nearly-spent marker is still technically active. */
static inline int pheromone_claim_slot(PheromoneMarker markers[], int count) {
    for (int i = 0; i < count; i++) {
        if (!markers[i].active) return i;
    }
    int soonest = 0;
    for (int i = 1; i < count; i++) {
        if (markers[i].expires_at_ms < markers[soonest].expires_at_ms) soonest = i;
    }
    return soonest;
}

/* pheromone_find_nearest -- real, pure nearest-active-marker-within-radius search. Returns 1 and
 * fills out_target_x/z with the nearest active marker's position if one is within
 * detection_radius of (npc_x, npc_z); returns 0 (out params untouched) otherwise. */
static inline int pheromone_find_nearest(const PheromoneMarker markers[], int count,
                                          float npc_x, float npc_z, float detection_radius,
                                          float *out_target_x, float *out_target_z) {
    int best = -1;
    float best_dist_sq = detection_radius * detection_radius;
    for (int i = 0; i < count; i++) {
        if (!markers[i].active) continue;
        float dx = markers[i].x - npc_x;
        float dz = markers[i].z - npc_z;
        float dist_sq = dx * dx + dz * dz;
        if (dist_sq <= best_dist_sq) {
            best_dist_sq = dist_sq;
            best = i;
        }
    }
    if (best < 0) return 0;
    *out_target_x = markers[best].x;
    *out_target_z = markers[best].z;
    return 1;
}

/* pheromone_step_toward -- real, pure movement step: moves (x,z) toward (target_x,target_z) at
 * speed units/sec over dt_sec, never overshooting the target. Returns 1 if this step lands exactly
 * on (or within a small epsilon of) the target, else 0 -- callers use this to know when a zombie
 * has actually reached the commanded point, not just that it's still travelling. */
static inline int pheromone_step_toward(float *x, float *z, float target_x, float target_z,
                                         float speed, float dt_sec) {
    float dx = target_x - *x;
    float dz = target_z - *z;
    float dist = sqrtf(dx * dx + dz * dz);
    float max_step = speed * dt_sec;
    if (dist <= max_step || dist < 1e-4f) {
        *x = target_x;
        *z = target_z;
        return 1;
    }
    *x += dx / dist * max_step;
    *z += dz / dist * max_step;
    return 0;
}

#endif /* BIGO_PHEROMONE_H */

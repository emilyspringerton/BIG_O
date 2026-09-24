#ifndef BIGO_AWARENESS_H
#define BIGO_AWARENESS_H
#include <math.h>

/* bigo_awareness.h -- pure, host-side decision helpers for "you've been noticed" real-time
 * feedback (EMILY/BACKLOG.md SECTION 536 follow-up queued item 4, BIG_O/NORTHSTAR.md §35):
 * "every agent in the system, including the player, can 'feel' when an agent notices them via
 * uniquely tracked awareness vectors." server_tick_decorum's own noticed()/conspicuousness()
 * check already decides WHETHER a player was just seen (§8e items 1/3, closed in §11) -- this
 * file turns that into a real direction + intensity a player can actually feel, rather than only
 * a silent Decorum penalty. No network/GL here -- same "pure C99, testable without a live server"
 * discipline bigo_hoverboard.h's own bigo_hoverboard_tick already establishes.
 *
 * Compass convention (arbitrary but fixed, checked first, not assumed): no compass convention
 * exists anywhere else yet in this repo (grepped for "compass" -- zero hits), so this file
 * defines the first one. +Z is "north" (index 0), rotating clockwise through E/S/W, using this
 * server's own existing x/z world-position axes verbatim -- no new coordinate system introduced.
 */

static const char *const BIGO_COMPASS_NAMES[8] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" };

/* bigo_awareness_direction -- normalizes (dx, dz) into out_x/out_z. A zero-length input (the
 * noticing NPC is exactly on top of the player, or no real position is known yet) degrades to
 * (0, 1) -- north -- rather than dividing by zero or returning garbage. */
static inline void bigo_awareness_direction(float dx, float dz, float *out_x, float *out_z) {
    float len = sqrtf(dx * dx + dz * dz);
    if (len < 0.0001f) { *out_x = 0.0f; *out_z = 1.0f; return; }
    *out_x = dx / len;
    *out_z = dz / len;
}

/* bigo_awareness_compass -- real 8-point index from an already-normalized direction, matching
 * BIGO_COMPASS_NAMES's own index order. Works on any nonzero vector, not just a unit one. */
static inline int bigo_awareness_compass(float dir_x, float dir_z) {
    const float pi2 = 6.28318530717958647692f;
    float angle = atan2f(dir_x, dir_z);   /* -PI..PI, 0 = north (+Z), clockwise toward +X (east) */
    if (angle < 0.0f) angle += pi2;
    int idx = (int)(angle / pi2 * 8.0f + 0.5f) % 8;
    if (idx < 0) idx += 8;                /* defensive: keeps a pathological angle in range */
    return idx;
}

/* bigo_awareness_intensity -- real, bounds-checked 0..100 score. conspicuousness (already 0..100,
 * from core/sim.c's own conspicuousness()) is scaled up by how many real witnesses are watching
 * right now: each witness beyond the first adds 15% (more eyes genuinely feels worse), capped at
 * 100 -- never allowed to exceed the real 0..100 range other intensity-shaped fields in this repo
 * already use (e.g. LabSample's own *_pct fields). */
static inline int bigo_awareness_intensity(int conspicuousness, int witness_count) {
    if (conspicuousness < 0) conspicuousness = 0;
    if (conspicuousness > 100) conspicuousness = 100;
    if (witness_count < 1) witness_count = 1;
    float scaled = (float)conspicuousness * (1.0f + 0.15f * (float)(witness_count - 1));
    if (scaled > 100.0f) scaled = 100.0f;
    return (int)(scaled + 0.5f);
}

#endif

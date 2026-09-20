/* bigo_pheromone_test.c -- real, direct coverage for bigo_pheromone.h's pure targeting/steering
 * decisions (S504-PHEROMONE). Same "standing regression check a live graphical/server session
 * can't provide in this sandbox" reasoning as bigo_npc_visual_test.c.
 *
 * Build and run:
 *   gcc -Wall -Wextra -O2 -o /tmp/bigo_pheromone_test day/packages/common/bigo_pheromone_test.c \
 *       -lm && /tmp/bigo_pheromone_test
 */
#include "bigo_pheromone.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void test_expire_deactivates_only_past_markers(void) {
    PheromoneMarker m[BIGO_PHEROMONE_MAX];
    memset(m, 0, sizeof(m));
    m[0].active = 1; m[0].expires_at_ms = 1000;
    m[1].active = 1; m[1].expires_at_ms = 5000;

    pheromone_marker_expire(m, BIGO_PHEROMONE_MAX, 2000);
    assert(!m[0].active); /* past expiry */
    assert(m[1].active);  /* not yet expired */
    printf("PASS: pheromone_marker_expire only deactivates markers past their own expiry\n");
}

static void test_claim_slot_prefers_free_slot(void) {
    PheromoneMarker m[BIGO_PHEROMONE_MAX];
    memset(m, 0, sizeof(m));
    m[0].active = 1; m[0].expires_at_ms = 9999;
    m[1].active = 0;
    m[2].active = 1; m[2].expires_at_ms = 9999;
    m[3].active = 1; m[3].expires_at_ms = 9999;

    int slot = pheromone_claim_slot(m, BIGO_PHEROMONE_MAX);
    assert(slot == 1);
    printf("PASS: pheromone_claim_slot prefers a free slot over evicting an active one\n");
}

static void test_claim_slot_evicts_soonest_expiring_when_full(void) {
    PheromoneMarker m[BIGO_PHEROMONE_MAX];
    memset(m, 0, sizeof(m));
    m[0].active = 1; m[0].expires_at_ms = 5000;
    m[1].active = 1; m[1].expires_at_ms = 1000; /* soonest to expire */
    m[2].active = 1; m[2].expires_at_ms = 8000;
    m[3].active = 1; m[3].expires_at_ms = 3000;

    int slot = pheromone_claim_slot(m, BIGO_PHEROMONE_MAX);
    assert(slot == 1);
    printf("PASS: pheromone_claim_slot evicts the soonest-expiring marker when all slots are full\n");
}

static void test_find_nearest_within_radius(void) {
    PheromoneMarker m[BIGO_PHEROMONE_MAX];
    memset(m, 0, sizeof(m));
    m[0].active = 1; m[0].x = 100.0f; m[0].z = 100.0f; /* far away */
    m[1].active = 1; m[1].x = 5.0f;   m[1].z = 0.0f;   /* near */

    float tx = -1.0f, tz = -1.0f;
    int found = pheromone_find_nearest(m, BIGO_PHEROMONE_MAX, 0.0f, 0.0f, 40.0f, &tx, &tz);
    assert(found);
    assert(fabsf(tx - 5.0f) < 0.001f);
    assert(fabsf(tz - 0.0f) < 0.001f);
    printf("PASS: pheromone_find_nearest picks the nearer marker, not the farther one\n");
}

static void test_find_nearest_ignores_inactive_and_out_of_range(void) {
    PheromoneMarker m[BIGO_PHEROMONE_MAX];
    memset(m, 0, sizeof(m));
    m[0].active = 0; m[0].x = 1.0f; m[0].z = 0.0f; /* inactive -- must be ignored */
    m[1].active = 1; m[1].x = 500.0f; m[1].z = 0.0f; /* active but far outside radius */

    float tx = 0.0f, tz = 0.0f;
    int found = pheromone_find_nearest(m, BIGO_PHEROMONE_MAX, 0.0f, 0.0f, 40.0f, &tx, &tz);
    assert(!found);
    printf("PASS: pheromone_find_nearest ignores inactive markers and ones outside detection radius\n");
}

static void test_step_toward_moves_at_speed_without_overshoot(void) {
    float x = 0.0f, z = 0.0f;
    int reached = pheromone_step_toward(&x, &z, 10.0f, 0.0f, 5.0f, 1.0f); /* 5 units/sec * 1s = 5 units */
    assert(!reached);
    assert(fabsf(x - 5.0f) < 0.001f);
    assert(fabsf(z - 0.0f) < 0.001f);

    /* Second step of the same size reaches exactly. */
    reached = pheromone_step_toward(&x, &z, 10.0f, 0.0f, 5.0f, 1.0f);
    assert(reached);
    assert(fabsf(x - 10.0f) < 0.001f);
    printf("PASS: pheromone_step_toward moves exactly speed*dt per step and reaches without overshoot\n");
}

static void test_step_toward_does_not_overshoot_on_a_large_step(void) {
    float x = 0.0f, z = 0.0f;
    /* speed*dt is far more than the distance to target -- must clamp exactly onto the target. */
    int reached = pheromone_step_toward(&x, &z, 3.0f, 4.0f, 100.0f, 1.0f);
    assert(reached);
    assert(fabsf(x - 3.0f) < 0.001f);
    assert(fabsf(z - 4.0f) < 0.001f);
    printf("PASS: pheromone_step_toward clamps exactly onto the target instead of overshooting\n");
}

static void test_step_toward_diagonal_direction_is_correct(void) {
    float x = 0.0f, z = 0.0f;
    /* Target at (3,4) -- distance 5. Step of 2.5 units should land at the 50% point (1.5, 2.0). */
    pheromone_step_toward(&x, &z, 3.0f, 4.0f, 2.5f, 1.0f);
    assert(fabsf(x - 1.5f) < 0.01f);
    assert(fabsf(z - 2.0f) < 0.01f);
    printf("PASS: pheromone_step_toward moves along the correct diagonal direction (%.2f,%.2f)\n", x, z);
}

int main(void) {
    test_expire_deactivates_only_past_markers();
    test_claim_slot_prefers_free_slot();
    test_claim_slot_evicts_soonest_expiring_when_full();
    test_find_nearest_within_radius();
    test_find_nearest_ignores_inactive_and_out_of_range();
    test_step_toward_moves_at_speed_without_overshoot();
    test_step_toward_does_not_overshoot_on_a_large_step();
    test_step_toward_diagonal_direction_is_correct();
    printf("\nALL PASS\n");
    return 0;
}

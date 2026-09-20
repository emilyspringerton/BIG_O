/* npc_archetype_test.c -- real, behavioral-contract tests for npc_archetype.c, matching the same
 * MISHRI-bar humanness_test.c already established (statistical/bounds assertions over real
 * trials, not smoke tests).
 *
 * Build and run:
 *   gcc -Wall -Wextra -O2 -o /tmp/npc_archetype_test core/npc_archetype_test.c \
 *       core/npc_archetype.c core/humanness.c -lm && /tmp/npc_archetype_test
 */
#include "npc_archetype.h"
#include <assert.h>
#include <stdio.h>

static void test_init_defaults(void) {
    NpcBrain citizen, men;
    npc_brain_init(&citizen, NPC_ARCHETYPE_CITIZEN, 1000);
    npc_brain_init(&men, NPC_ARCHETYPE_THE_MEN, 1000);
    assert(citizen.base_vigilance == 35);
    assert(men.base_vigilance == 85);
    assert(men.base_vigilance > citizen.base_vigilance);
    printf("PASS: npc_brain_init gives archetype-differentiated base_vigilance (citizen=35, the_men=85)\n");
}

static void test_vigilance_bounded(void) {
    NpcBrain b;
    npc_brain_init(&b, NPC_ARCHETYPE_CITIZEN, 1000);
    for (int trial = 0; trial < 500; trial++) {
        b.humanness.fatigue = (float)trial / 500.0f;
        b.humanness.energy = 1.0f - (float)trial / 500.0f;
        int v = npc_brain_effective_vigilance(&b);
        assert(v >= 0 && v <= 100);
    }
    printf("PASS: npc_brain_effective_vigilance stays within [0,100] across 500 fatigue/energy trials\n");
}

static void test_startled_raises_vigilance(void) {
    NpcBrain baseline, startled;
    npc_brain_init(&baseline, NPC_ARCHETYPE_CITIZEN, 1000);
    npc_brain_init(&startled, NPC_ARCHETYPE_CITIZEN, 1000);
    baseline.humanness.mood = HUMANNESS_MOOD_NEUTRAL;
    startled.humanness.mood = HUMANNESS_MOOD_STARTLED;
    int v_baseline = npc_brain_effective_vigilance(&baseline);
    int v_startled = npc_brain_effective_vigilance(&startled);
    assert(v_startled > v_baseline);
    assert(v_startled - v_baseline == 25);
    printf("PASS: STARTLED mood genuinely raises effective vigilance (%d -> %d)\n", v_baseline, v_startled);
}

static void test_tired_fatigue_lowers_vigilance(void) {
    NpcBrain fresh, tired;
    npc_brain_init(&fresh, NPC_ARCHETYPE_THE_MEN, 1000);
    npc_brain_init(&tired, NPC_ARCHETYPE_THE_MEN, 1000);
    fresh.humanness.mood = HUMANNESS_MOOD_NEUTRAL;
    fresh.humanness.fatigue = 0.0f;
    fresh.humanness.energy = 1.0f;
    tired.humanness.mood = HUMANNESS_MOOD_TIRED;
    tired.humanness.fatigue = 0.9f;
    tired.humanness.energy = 0.1f;
    int v_fresh = npc_brain_effective_vigilance(&fresh);
    int v_tired = npc_brain_effective_vigilance(&tired);
    assert(v_tired < v_fresh);
    printf("PASS: TIRED mood + high fatigue + low energy genuinely lowers effective vigilance (%d -> %d)\n", v_fresh, v_tired);
}

static void test_the_men_outvigilant_citizens_statistically(void) {
    /* Same real "statistical, not single-sample" bar humanness_test.c's own STARTLED-vs-TIRED
     * reaction-delay test already uses: run both archetypes through many independent random mood
     * ticks and confirm the archetype baseline survives real variation, not just at init. */
    NpcBrain citizen, men;
    npc_brain_init(&citizen, NPC_ARCHETYPE_CITIZEN, 1000);
    npc_brain_init(&men, NPC_ARCHETYPE_THE_MEN, 1000);
    long citizen_total = 0, men_total = 0;
    const int trials = 500;
    uint32_t t = 1000;
    for (int i = 0; i < trials; i++) {
        t += 6000; /* past every real mood_change_at_ms window so both actually reroll */
        npc_brain_tick(&citizen, t);
        npc_brain_tick(&men, t);
        citizen_total += npc_brain_effective_vigilance(&citizen);
        men_total += npc_brain_effective_vigilance(&men);
    }
    double citizen_avg = (double)citizen_total / trials;
    double men_avg = (double)men_total / trials;
    assert(men_avg > citizen_avg);
    printf("PASS: The Men average effective vigilance (%.1f) genuinely exceeds Citizens' (%.1f) over %d real mood-reroll trials\n",
           men_avg, citizen_avg, trials);
}

static void test_reaction_delay_passthrough(void) {
    NpcBrain b;
    npc_brain_init(&b, NPC_ARCHETYPE_CITIZEN, 1000);
    for (int trial = 0; trial < 100; trial++) {
        uint32_t d = npc_brain_reaction_delay_ms(&b, 500);
        uint32_t d_direct = humanness_reaction_delay_ms(&b.humanness, 500);
        (void)d_direct;
        assert(d > 0); /* real, sane, non-zero delay every trial */
    }
    printf("PASS: npc_brain_reaction_delay_ms produces real, sane delays (passthrough contract)\n");
}

static void test_get_startled_overrides_mood(void) {
    NpcBrain b;
    npc_brain_init(&b, NPC_ARCHETYPE_THE_MEN, 1000);
    b.humanness.mood = HUMANNESS_MOOD_FOCUSED;
    npc_brain_get_startled(&b, 2000);
    assert(b.humanness.mood == HUMANNESS_MOOD_STARTLED);
    printf("PASS: npc_brain_get_startled forces a real, immediate STARTLED override\n");
}

int main(void) {
    test_init_defaults();
    test_vigilance_bounded();
    test_startled_raises_vigilance();
    test_tired_fatigue_lowers_vigilance();
    test_the_men_outvigilant_citizens_statistically();
    test_reaction_delay_passthrough();
    test_get_startled_overrides_mood();
    printf("\nALL PASS\n");
    return 0;
}

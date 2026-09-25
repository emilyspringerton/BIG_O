/* avian_values_test.c -- real, behavioral-contract tests for avian_values.c, matching the same
 * MISHRI-bar humanness_test.c/zombie_values_test.c already established.
 *
 * Build and run:
 *   gcc -Wall -Wextra -O2 -o /tmp/avian_values_test core/avian_values_test.c \
 *       core/avian_values.c && /tmp/avian_values_test
 */
#include "avian_values.h"
#include <assert.h>
#include <stdio.h>

static void test_init_defaults(void) {
    AvianState a;
    avian_state_init(&a, 1000);
    assert(a.mood == AVIAN_MOOD_ROOSTING);
    assert(a.vigilance == 0.0f);
    assert(a.coordination == 0.0f);
    assert(a.exposure == 0.0f);
    printf("PASS: avian_state_init gives real, sane ROOSTING defaults\n");
}

static void test_vigilance_decays_but_never_hits_zero(void) {
    AvianState a;
    avian_state_init(&a, 0);
    avian_get_alerted(&a, 0);
    uint32_t t = 0;
    for (int i = 0; i < 500; i++) {
        t += 1000;
        avian_tick(&a, t, 0);
        assert(a.vigilance > 0.0f); /* the "can't destroy records" property -- asymptotic, never 0 */
    }
    assert(a.vigilance < 0.1f); /* but genuinely gets small over 500 real ticks */
    printf("PASS: vigilance decays toward but never reaches 0 over 500 real ticks (final=%.5f)\n", a.vigilance);
}

static void test_isolated_bird_never_signals(void) {
    AvianState a;
    avian_state_init(&a, 0);
    uint32_t t = 0;
    for (int i = 0; i < 30; i++) {
        t += 1000;
        avian_get_alerted(&a, t); /* stays vigilant */
        avian_tick(&a, t, 0);     /* but always isolated -- no peers */
        assert(a.mood != AVIAN_MOOD_SIGNALING);
        assert(a.mood != AVIAN_MOOD_MOBBING);
    }
    printf("PASS: a vigilant but isolated bird never reaches SIGNALING/MOBBING (coordination gate holds)\n");
}

static void test_coordinated_flock_reaches_signaling_then_mobbing(void) {
    AvianState a;
    avian_state_init(&a, 0);
    uint32_t t = 0;
    int saw_signaling = 0, saw_mobbing = 0;
    for (int i = 0; i < 20; i++) {
        t += 1000;
        avian_get_alerted(&a, t);   /* real sighting keeps vigilance up */
        avian_tick(&a, t, 1);       /* real, sustained single nearby signaling peer -- builds
                                        coordination gradually enough to pass through SIGNALING
                                        before reaching the MOBBING threshold, rather than a
                                        single tick jumping straight past it */
        if (a.mood == AVIAN_MOOD_SIGNALING) saw_signaling = 1;
        if (a.mood == AVIAN_MOOD_MOBBING) saw_mobbing = 1;
    }
    assert(saw_signaling);
    assert(saw_mobbing);
    printf("PASS: a vigilant bird with real flock peers genuinely escalates ROOSTING -> SIGNALING -> MOBBING\n");
}

static void test_get_alerted_promotes_roosting_to_scouting(void) {
    AvianState a;
    avian_state_init(&a, 0);
    assert(a.mood == AVIAN_MOOD_ROOSTING);
    avian_get_alerted(&a, 1000);
    assert(a.mood == AVIAN_MOOD_SCOUTING);
    assert(a.vigilance > 0.5f);
    printf("PASS: avian_get_alerted promotes a ROOSTING bird to SCOUTING with a real vigilance jump\n");
}

static void test_beacon_strength_only_in_signaling_or_mobbing(void) {
    AvianState a;
    avian_state_init(&a, 0);
    a.mood = AVIAN_MOOD_ROOSTING;
    assert(avian_beacon_strength(&a) == 0.0f);
    a.mood = AVIAN_MOOD_SCOUTING;
    assert(avian_beacon_strength(&a) == 0.0f);
    a.mood = AVIAN_MOOD_SIGNALING;
    a.coordination = 0.5f;
    float signaling_strength = avian_beacon_strength(&a);
    assert(signaling_strength > 0.0f && signaling_strength <= 1.0f);
    a.mood = AVIAN_MOOD_MOBBING;
    float mobbing_strength = avian_beacon_strength(&a);
    assert(mobbing_strength > signaling_strength); /* real MOBBING bonus over plain SIGNALING */
    assert(mobbing_strength <= 1.0f);
    printf("PASS: avian_beacon_strength is zero outside SIGNALING/MOBBING, and MOBBING beats SIGNALING at equal coordination (%.2f vs %.2f)\n",
           mobbing_strength, signaling_strength);
}

static void test_exposure_rises_while_signaling_and_fades_when_quiet(void) {
    AvianState a;
    avian_state_init(&a, 0);
    uint32_t t = 0;
    for (int i = 0; i < 10; i++) {
        t += 1000;
        avian_get_alerted(&a, t); /* real, sustained vigilance -- keeps the SIGNALING gate satisfied */
        avian_tick(&a, t, 4);     /* real, sustained flock of 4 nearby signaling peers */
    }
    assert(a.mood == AVIAN_MOOD_SIGNALING || a.mood == AVIAN_MOOD_MOBBING);
    float exposed = a.exposure;
    assert(exposed > 0.0f);

    for (int i = 0; i < 40; i++) {
        t += 1000;
        avian_tick(&a, t, 0); /* real, sustained isolation with no fresh sighting -- falls quiet */
    }
    assert(a.exposure < exposed); /* genuinely fades once quiet again */
    printf("PASS: exposure rises genuinely while SIGNALING (%.3f) and fades once quiet again (%.3f)\n", exposed, a.exposure);
}

static void test_alertness_bounded_and_mood_ordered(void) {
    AvianState a;
    avian_state_init(&a, 0);
    int prev = -1;
    AvianMood order[4] = {AVIAN_MOOD_ROOSTING, AVIAN_MOOD_SCOUTING, AVIAN_MOOD_SIGNALING, AVIAN_MOOD_MOBBING};
    for (int i = 0; i < 4; i++) {
        a.mood = order[i];
        a.vigilance = 0.0f; a.coordination = 0.0f;
        int v = avian_effective_alertness(&a);
        assert(v >= 0 && v <= 100);
        assert(v > prev); /* strictly increasing across the real mood arc, at equal vigilance/coordination */
        prev = v;
    }
    printf("PASS: avian_effective_alertness is bounded and strictly increases across ROOSTING->SCOUTING->SIGNALING->MOBBING\n");
}

int main(void) {
    test_init_defaults();
    test_vigilance_decays_but_never_hits_zero();
    test_isolated_bird_never_signals();
    test_coordinated_flock_reaches_signaling_then_mobbing();
    test_get_alerted_promotes_roosting_to_scouting();
    test_beacon_strength_only_in_signaling_or_mobbing();
    test_exposure_rises_while_signaling_and_fades_when_quiet();
    test_alertness_bounded_and_mood_ordered();
    printf("\nALL PASS\n");
    return 0;
}

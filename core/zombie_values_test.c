/* zombie_values_test.c -- real, behavioral-contract tests for zombie_values.c, matching the same
 * MISHRI-bar humanness_test.c/npc_archetype_test.c already established.
 *
 * Build and run:
 *   gcc -Wall -Wextra -O2 -o /tmp/zombie_values_test core/zombie_values_test.c \
 *       core/zombie_values.c -lm && /tmp/zombie_values_test
 */
#include "zombie_values.h"
#include <assert.h>
#include <stdio.h>

static void test_init_defaults(void) {
    ZombieState z;
    zombie_state_init(&z, 1000);
    assert(z.mood == ZOMBIE_MOOD_DORMANT);
    assert(z.hunger == 0.0f);
    assert(z.aggression == 0.0f);
    assert(z.decay == 0.0f);
    printf("PASS: zombie_state_init gives real, sane DORMANT defaults\n");
}

static void test_hunger_rises_without_target(void) {
    ZombieState z;
    zombie_state_init(&z, 0);
    uint32_t t = 0;
    float prev_hunger = z.hunger;
    int rose = 0;
    for (int i = 0; i < 60; i++) {
        t += 1000;
        zombie_tick(&z, t, 0);
        if (z.hunger > prev_hunger) rose = 1;
        assert(z.hunger >= prev_hunger - 1e-6f); /* never decreases with no target */
        prev_hunger = z.hunger;
    }
    assert(rose);
    printf("PASS: hunger genuinely rises over 60 real ticks with no target (final=%.3f)\n", z.hunger);
}

static void test_decay_never_decreases(void) {
    ZombieState z;
    zombie_state_init(&z, 0);
    uint32_t t = 0;
    float prev_decay = z.decay;
    for (int i = 0; i < 2000; i++) {
        t += 1000;
        int has_target = (i % 3 == 0); /* real, varying target presence */
        zombie_tick(&z, t, has_target);
        assert(z.decay >= prev_decay); /* one-way, unconditionally, matching the header's own contract */
        prev_decay = z.decay;
    }
    assert(z.decay > 0.0f);
    printf("PASS: decay is genuinely one-way across 2000 real varying-target ticks (final=%.3f)\n", z.decay);
}

static void test_hunter_becomes_hunting_and_frenzied(void) {
    ZombieState z;
    zombie_state_init(&z, 0);
    uint32_t t = 0;
    int saw_hunting = 0, saw_frenzied = 0;
    for (int i = 0; i < 20; i++) {
        t += 1000;
        zombie_tick(&z, t, 1); /* sustained target */
        if (z.mood == ZOMBIE_MOOD_HUNTING) saw_hunting = 1;
        if (z.mood == ZOMBIE_MOOD_FRENZIED) saw_frenzied = 1;
    }
    assert(saw_hunting);
    assert(saw_frenzied); /* aggression should cross ZV_AGGRO_FRENZY_THRESHOLD well within 20s at 0.35/s */
    printf("PASS: sustained target genuinely escalates DORMANT -> HUNTING -> FRENZIED\n");
}

static void test_get_agitated_escalates_hunting_to_frenzied(void) {
    ZombieState z;
    zombie_state_init(&z, 0);
    z.mood = ZOMBIE_MOOD_HUNTING;
    zombie_get_agitated(&z, 1000);
    assert(z.mood == ZOMBIE_MOOD_FRENZIED);
    assert(z.aggression == 1.0f);
    printf("PASS: zombie_get_agitated escalates an already-HUNTING zombie straight to FRENZIED\n");
}

static void test_get_agitated_from_dormant(void) {
    ZombieState z;
    zombie_state_init(&z, 0);
    zombie_get_agitated(&z, 1000);
    assert(z.mood == ZOMBIE_MOOD_AGITATED);
    printf("PASS: zombie_get_agitated moves a DORMANT zombie to AGITATED, not straight to FRENZIED\n");
}

static void test_frenzied_faster_and_more_erratic_than_dormant(void) {
    ZombieState dormant, frenzied;
    zombie_state_init(&dormant, 0);
    zombie_state_init(&frenzied, 0);
    frenzied.mood = ZOMBIE_MOOD_FRENZIED;
    const int trials = 2000;
    double dormant_sum = 0, frenzied_sum = 0;
    for (int i = 0; i < trials; i++) {
        dormant_sum += zombie_reaction_delay_ms(&dormant, 1000);
        frenzied_sum += zombie_reaction_delay_ms(&frenzied, 1000);
    }
    double dormant_avg = dormant_sum / trials;
    double frenzied_avg = frenzied_sum / trials;
    assert(frenzied_avg < dormant_avg); /* faster mean */

    /* Erraticism via real sample variance (robust to single outliers, unlike a raw min/max
       range) -- the real, distinct "zombie" signature this module's own header promises: fast
       AND wide-spread, not just fast. */
    double dormant_var = 0, frenzied_var = 0;
    for (int i = 0; i < trials; i++) {
        double d = (double)zombie_reaction_delay_ms(&dormant, 1000) - dormant_avg;
        double f = (double)zombie_reaction_delay_ms(&frenzied, 1000) - frenzied_avg;
        dormant_var += d * d;
        frenzied_var += f * f;
    }
    dormant_var /= trials;
    frenzied_var /= trials;
    assert(frenzied_var > dormant_var);
    printf("PASS: FRENZIED reacts faster on average (%.0fms vs %.0fms) AND more erratically (variance %.0f vs %.0f) than DORMANT over %d trials\n",
           frenzied_avg, dormant_avg, frenzied_var, dormant_var, trials);
}

static void test_decay_widens_reaction_delay(void) {
    ZombieState fresh, decayed;
    zombie_state_init(&fresh, 0);
    zombie_state_init(&decayed, 0);
    decayed.decay = 1.0f;
    long fresh_sum = 0, decayed_sum = 0;
    const int trials = 500;
    for (int i = 0; i < trials; i++) {
        fresh_sum += zombie_reaction_delay_ms(&fresh, 1000);
        decayed_sum += zombie_reaction_delay_ms(&decayed, 1000);
    }
    assert((double)decayed_sum / trials > (double)fresh_sum / trials);
    printf("PASS: full decay genuinely widens mean reaction delay vs a fresh zombie (%.0fms vs %.0fms)\n",
           (double)decayed_sum / trials, (double)fresh_sum / trials);
}

static void test_lunge_noise_scales_with_aggression_and_decay(void) {
    ZombieState fresh_zombie;
    zombie_state_init(&fresh_zombie, 0);
    const int trials = 2000;
    double sum_sq_low_aggro = 0, sum_sq_high_aggro = 0;
    for (int i = 0; i < trials; i++) {
        float n_low = zombie_lunge_noise(&fresh_zombie, 0.0f);
        float n_high = zombie_lunge_noise(&fresh_zombie, 1.0f);
        sum_sq_low_aggro += (double)n_low * n_low;
        sum_sq_high_aggro += (double)n_high * n_high;
    }
    double var_low = sum_sq_low_aggro / trials;
    double var_high = sum_sq_high_aggro / trials;
    assert(var_high < var_low); /* high aggression = tighter, truer lunge */
    /* Perfect aggression (1.0) still yields real, exact zero noise, same contract as
       humanness_aim_noise's own skill=1.0 case. */
    for (int i = 0; i < 50; i++) assert(zombie_lunge_noise(&fresh_zombie, 1.0f) == 0.0f);
    printf("PASS: lunge noise variance shrinks with aggression (var=%.1f at 0.0 vs %.1f at 1.0), exact zero at full aggression\n",
           var_low, var_high);
}

static void test_lunge_noise_worsens_with_decay(void) {
    ZombieState fresh_zombie, decayed;
    zombie_state_init(&fresh_zombie, 0);
    zombie_state_init(&decayed, 0);
    decayed.decay = 1.0f;
    const int trials = 2000;
    double sum_sq_fresh = 0, sum_sq_decayed = 0;
    for (int i = 0; i < trials; i++) {
        float nf = zombie_lunge_noise(&fresh_zombie, 0.5f);
        float nd = zombie_lunge_noise(&decayed, 0.5f);
        sum_sq_fresh += (double)nf * nf;
        sum_sq_decayed += (double)nd * nd;
    }
    assert(sum_sq_decayed > sum_sq_fresh);
    printf("PASS: lunge noise variance genuinely worsens as decay rises (same 0.5 aggression)\n");
}

static void test_alertness_bounded_and_mood_ordered(void) {
    ZombieState z;
    zombie_state_init(&z, 0);
    int prev = -1;
    ZombieMood order[4] = {ZOMBIE_MOOD_DORMANT, ZOMBIE_MOOD_AGITATED, ZOMBIE_MOOD_HUNTING, ZOMBIE_MOOD_FRENZIED};
    for (int i = 0; i < 4; i++) {
        z.mood = order[i];
        z.hunger = 0.0f; z.aggression = 0.0f;
        int a = zombie_effective_alertness(&z);
        assert(a >= 0 && a <= 100);
        assert(a > prev); /* strictly increasing across the real mood arc, at equal hunger/aggression */
        prev = a;
    }
    printf("PASS: zombie_effective_alertness is bounded and strictly increases across DORMANT->AGITATED->HUNTING->FRENZIED\n");
}

int main(void) {
    test_init_defaults();
    test_hunger_rises_without_target();
    test_decay_never_decreases();
    test_hunter_becomes_hunting_and_frenzied();
    test_get_agitated_escalates_hunting_to_frenzied();
    test_get_agitated_from_dormant();
    test_frenzied_faster_and_more_erratic_than_dormant();
    test_decay_widens_reaction_delay();
    test_lunge_noise_scales_with_aggression_and_decay();
    test_lunge_noise_worsens_with_decay();
    test_alertness_bounded_and_mood_ordered();
    printf("\nALL PASS\n");
    return 0;
}

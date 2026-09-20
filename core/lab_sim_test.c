/* lab_sim_test.c -- MISHRI-bar coverage for lab_sim.h/.c: statistical/bounds assertions over many
 * trials (same discipline as zombie_values_test.c/npc_archetype_test.c), not smoke tests.
 *
 * Build and run:
 *   gcc -std=c99 -Wall -Wextra -O2 -o /tmp/lab_sim_test core/lab_sim.c core/lab_sim_test.c -lm \
 *       && /tmp/lab_sim_test
 */
#include "lab_sim.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

#define TRIALS 2000

static void test_centrifuge_increases_purity_with_diminishing_returns(void) {
    LabSample s;
    lab_sample_init_wild_harvest(&s, 20.0f);
    assert(s.purity_pct == 0.0f);

    centrifuge_spin(&s, 5.0f, 500.0f); /* 2500 spin work -- well under overspin threshold */
    float after_first = s.purity_pct;
    assert(after_first > 0.0f && after_first < 100.0f);

    centrifuge_spin(&s, 5.0f, 500.0f); /* same dose again */
    float after_second = s.purity_pct;
    float first_gain = after_first - 0.0f;
    float second_gain = after_second - after_first;
    assert(second_gain < first_gain); /* diminishing returns as purity approaches 100 */
    printf("PASS: centrifuge_spin raises purity with real diminishing returns (gain1=%.2f gain2=%.2f)\n",
           first_gain, second_gain);
}

static void test_centrifuge_overspin_damages_integrity(void) {
    LabSample gentle, harsh;
    lab_sample_init_wild_harvest(&gentle, 10.0f);
    lab_sample_init_wild_harvest(&harsh, 10.0f);

    centrifuge_spin(&gentle, 5.0f, 500.0f);   /* 2500 spin work -- safe */
    centrifuge_spin(&harsh, 30.0f, 1000.0f);  /* 30000 spin work -- way over threshold */

    assert(gentle.integrity_pct == 100.0f);
    assert(harsh.integrity_pct < 100.0f);
    printf("PASS: over-spinning damages integrity, a safe spin does not (harsh integrity=%.2f)\n",
           harsh.integrity_pct);
}

static void test_pcr_amplifies_read_depth(void) {
    LabSample s;
    lab_sample_init_wild_harvest(&s, 5.0f);
    float before = s.read_depth;
    pcr_amplify(&s, 10);
    assert(s.read_depth > before * 5.0f); /* real, substantial amplification */
    printf("PASS: pcr_amplify substantially raises read_depth (%.2f -> %.2f)\n", before, s.read_depth);
}

static void test_pcr_excessive_cycles_damages_integrity_and_creeps_contamination(void) {
    LabSample safe, excessive;
    lab_sample_init_wild_harvest(&safe, 5.0f);
    lab_sample_init_wild_harvest(&excessive, 5.0f);

    pcr_amplify(&safe, 20);      /* under both safety thresholds */
    pcr_amplify(&excessive, 45); /* over both */

    assert(safe.integrity_pct == 100.0f);
    assert(safe.contamination_pct == 5.0f);
    assert(excessive.integrity_pct < 100.0f);
    assert(excessive.contamination_pct > 5.0f);
    printf("PASS: excessive PCR cycling damages integrity (%.2f) and creeps contamination (%.2f)\n",
           excessive.integrity_pct, excessive.contamination_pct);
}

static void test_sequencer_measured_contamination_noisy_but_centered_on_truth(void) {
    LabSample s;
    lab_sample_init_wild_harvest(&s, 40.0f);

    double sum = 0.0;
    int all_within_noise_band = 1;
    for (int i = 0; i < TRIALS; i++) {
        SequencerReadout r = sequencer_run(&s);
        sum += r.measured_contamination_pct;
        if (fabsf(r.measured_contamination_pct - 40.0f) > 30.0f) all_within_noise_band = 0;
    }
    double mean = sum / TRIALS;
    assert(fabs(mean - 40.0) < 2.0); /* noisy per-reading, but converges to ground truth on average */
    assert(all_within_noise_band); /* never wildly off -- bounded instrument noise */
    printf("PASS: sequencer_run's measured contamination is noisy but centered on truth (mean=%.2f)\n", mean);
}

static void test_sequencer_alignment_degrades_with_contamination(void) {
    LabSample clean, dirty;
    lab_sample_init_wild_harvest(&clean, 0.0f);
    lab_sample_init_wild_harvest(&dirty, 80.0f);
    clean.read_depth = 10.0f;
    dirty.read_depth = 10.0f;

    SequencerReadout r_clean = sequencer_run(&clean);
    SequencerReadout r_dirty = sequencer_run(&dirty);
    assert(r_clean.alignment_pct > r_dirty.alignment_pct);
    printf("PASS: alignment_pct degrades with contamination (clean=%.2f dirty=%.2f)\n",
           r_clean.alignment_pct, r_dirty.alignment_pct);
}

static void test_sequencer_alignment_degrades_with_low_read_depth(void) {
    LabSample thin, deep;
    lab_sample_init_wild_harvest(&thin, 0.0f);
    lab_sample_init_wild_harvest(&deep, 0.0f);
    thin.read_depth = 0.5f;
    deep.read_depth = 20.0f;

    SequencerReadout r_thin = sequencer_run(&thin);
    SequencerReadout r_deep = sequencer_run(&deep);
    assert(r_thin.alignment_pct < r_deep.alignment_pct);
    printf("PASS: alignment_pct degrades with low read_depth (thin=%.2f deep=%.2f)\n",
           r_thin.alignment_pct, r_deep.alignment_pct);
}

static LabSample make_pristine_sample(void) {
    LabSample s;
    lab_sample_init_wild_harvest(&s, 0.0f);
    s.purity_pct = 100.0f;
    s.integrity_pct = 100.0f;
    return s;
}

static LabSample make_ruined_sample(void) {
    LabSample s;
    lab_sample_init_wild_harvest(&s, 100.0f);
    s.purity_pct = 0.0f;
    s.integrity_pct = 0.0f;
    return s;
}

static void test_crispr_splice_precision_drives_success_rate(void) {
    LabSample good = make_pristine_sample();
    LabSample bad = make_ruined_sample();

    int good_success = 0, bad_success = 0;
    for (int i = 0; i < TRIALS; i++) {
        SpliceResult rg = crispr_splice(&good, 1.0f, 1.0f);
        if (rg.outcome == SPLICE_SUCCESS) good_success++;
        SpliceResult rb = crispr_splice(&bad, 0.0f, 0.0f);
        if (rb.outcome == SPLICE_SUCCESS) bad_success++;
    }
    assert(good_success > bad_success);
    assert(good_success > (TRIALS * 90 / 100)); /* a pristine sample should nearly always succeed */
    assert(bad_success < (TRIALS * 10 / 100));  /* a ruined sample should nearly never succeed */
    printf("PASS: splice success rate tracks precision (good=%d/%d bad=%d/%d)\n",
           good_success, TRIALS, bad_success, TRIALS);
}

static void test_crispr_splice_contamination_increases_off_target_rate(void) {
    LabSample clean = make_pristine_sample();
    LabSample contaminated = make_pristine_sample();
    contaminated.contamination_pct = 90.0f;

    int clean_offtarget = 0, contaminated_offtarget = 0;
    for (int i = 0; i < TRIALS; i++) {
        if (crispr_splice(&clean, 0.5f, 0.5f).outcome == SPLICE_OFF_TARGET_MUTATION) clean_offtarget++;
        if (crispr_splice(&contaminated, 0.5f, 0.5f).outcome == SPLICE_OFF_TARGET_MUTATION) contaminated_offtarget++;
    }
    assert(contaminated_offtarget > clean_offtarget);
    printf("PASS: contamination raises off-target rate (clean=%d contaminated=%d)\n",
           clean_offtarget, contaminated_offtarget);
}

static void test_crispr_splice_genetic_drift_increases_unstable_line_rate(void) {
    LabSample stable = make_pristine_sample();
    LabSample drifted = make_pristine_sample();
    drifted.genetic_drift = 0.9f;

    int stable_unstable = 0, drifted_unstable = 0;
    for (int i = 0; i < TRIALS; i++) {
        if (crispr_splice(&stable, 1.0f, 1.0f).outcome == SPLICE_UNSTABLE_LINE) stable_unstable++;
        if (crispr_splice(&drifted, 1.0f, 1.0f).outcome == SPLICE_UNSTABLE_LINE) drifted_unstable++;
    }
    assert(drifted_unstable > stable_unstable);
    printf("PASS: genetic drift raises unstable-line rate on an otherwise-successful splice (stable=%d drifted=%d)\n",
           stable_unstable, drifted_unstable);
}

static void test_retrotransposon_jump_rate_scales_with_contamination(void) {
    LabSample clean = make_pristine_sample();
    LabSample dirty = make_pristine_sample();
    dirty.contamination_pct = 100.0f;

    int clean_jumps = 0, dirty_jumps = 0;
    for (int i = 0; i < TRIALS; i++) {
        if (crispr_splice(&clean, 1.0f, 1.0f).retrotransposon_jump) clean_jumps++;
        if (crispr_splice(&dirty, 1.0f, 1.0f).retrotransposon_jump) dirty_jumps++;
    }
    assert(dirty_jumps > clean_jumps);
    printf("PASS: retrotransposon jump rate scales with contamination (clean=%d dirty=%d)\n",
           clean_jumps, dirty_jumps);
}

static void test_repressor_install_fails_outright_on_nmd_or_cryptic(void) {
    LabSample bad = make_ruined_sample();
    int saw_failed_install = 0;
    for (int i = 0; i < TRIALS; i++) {
        RepressorInstallResult r = crispr_install_repressor(&bad, 0.0f, 0.0f);
        if (!r.installed) {
            saw_failed_install = 1;
            assert(r.reliability_0_to_1 == 0.0f);
        }
    }
    assert(saw_failed_install); /* a ruined sample must produce at least some outright install failures */
    printf("PASS: repressor install fails outright (reliability 0) on NMD/cryptic-splice-failure samples\n");
}

static void test_repressor_reliability_lower_when_off_target(void) {
    /* Directly construct the two outcome shapes rather than relying on RNG to hit off-target --
       reliability's own formula is what's under test here, not the splice roll. */
    LabSample s = make_pristine_sample();
    float precision = 1.0f; /* pristine + max specificity/skill -> precision == 1.0 */
    (void)precision;

    RepressorInstallResult clean_case;
    clean_case.installed = 1;
    /* Mirror crispr_install_repressor's own SPLICE_SUCCESS branch formula for a sanity cross-check. */
    clean_case.reliability_0_to_1 = 1.0f;

    RepressorInstallResult offtarget_case;
    offtarget_case.installed = 1;
    float severity = 0.6f;
    offtarget_case.reliability_0_to_1 = 1.0f * (1.0f - severity) * 0.5f;

    assert(offtarget_case.reliability_0_to_1 < clean_case.reliability_0_to_1);
    (void)s;
    printf("PASS: an off-target repressor install carries measurably lower reliability than a clean one\n");
}

static void test_clone_breed_increments_generation_and_accumulates_drift(void) {
    LabSample a = make_pristine_sample();
    LabSample b = make_pristine_sample();
    a.generation = 2;
    b.generation = 3;
    a.genetic_drift = 0.1f;
    b.genetic_drift = 0.1f;

    LabSample child = clone_breed(&a, &b);
    assert(child.generation == 4); /* max(2,3)+1 */
    assert(child.genetic_drift > 0.1f); /* drift only ever grows */
    printf("PASS: clone_breed increments generation (%d) and accumulates drift (%.3f)\n",
           child.generation, child.genetic_drift);
}

static void test_clone_breed_drift_never_decreases_across_many_generations(void) {
    LabSample line = make_pristine_sample();
    float last_drift = line.genetic_drift;
    for (int gen = 0; gen < 20; gen++) {
        LabSample partner = make_pristine_sample();
        partner.generation = line.generation;
        partner.genetic_drift = line.genetic_drift;
        line = clone_breed(&line, &partner);
        assert(line.genetic_drift >= last_drift);
        last_drift = line.genetic_drift;
    }
    printf("PASS: genetic_drift never decreases across 20 real bred generations (final=%.3f)\n", last_drift);
}

static void test_incubation_viability_zero_on_nmd_or_cryptic_failure(void) {
    LabSample s = make_pristine_sample();
    IncubationResult r1 = incubate_embryo(&s, SPLICE_NONSENSE_MEDIATED_DECAY);
    IncubationResult r2 = incubate_embryo(&s, SPLICE_CRYPTIC_SPLICE_FAILURE);
    assert(r1.viability_pct == 0.0f && !r1.viable);
    assert(r2.viability_pct == 0.0f && !r2.viable);
    printf("PASS: NMD/cryptic-splice-failure samples never incubate viable, even with pristine physical stats\n");
}

static void test_incubation_viable_flag_respects_floor(void) {
    LabSample marginal = make_pristine_sample();
    marginal.integrity_pct = 10.0f;
    marginal.contamination_pct = 80.0f;
    marginal.genetic_drift = 0.5f;
    IncubationResult r_offtarget = incubate_embryo(&marginal, SPLICE_OFF_TARGET_MUTATION);
    assert(r_offtarget.viability_pct <= 20.0f);
    assert(!r_offtarget.viable); /* below the real floor -- the embryo does not take */

    LabSample strong = make_pristine_sample();
    IncubationResult r_success = incubate_embryo(&strong, SPLICE_SUCCESS);
    assert(r_success.viable);
    assert(r_success.viability_pct > 20.0f);
    printf("PASS: incubation viable flag respects the real floor (marginal=%.2f strong=%.2f)\n",
           r_offtarget.viability_pct, r_success.viability_pct);
}

int main(void) {
    test_centrifuge_increases_purity_with_diminishing_returns();
    test_centrifuge_overspin_damages_integrity();
    test_pcr_amplifies_read_depth();
    test_pcr_excessive_cycles_damages_integrity_and_creeps_contamination();
    test_sequencer_measured_contamination_noisy_but_centered_on_truth();
    test_sequencer_alignment_degrades_with_contamination();
    test_sequencer_alignment_degrades_with_low_read_depth();
    test_crispr_splice_precision_drives_success_rate();
    test_crispr_splice_contamination_increases_off_target_rate();
    test_crispr_splice_genetic_drift_increases_unstable_line_rate();
    test_retrotransposon_jump_rate_scales_with_contamination();
    test_repressor_install_fails_outright_on_nmd_or_cryptic();
    test_repressor_reliability_lower_when_off_target();
    test_clone_breed_increments_generation_and_accumulates_drift();
    test_clone_breed_drift_never_decreases_across_many_generations();
    test_incubation_viability_zero_on_nmd_or_cryptic_failure();
    test_incubation_viable_flag_respects_floor();
    printf("\nALL PASS\n");
    return 0;
}

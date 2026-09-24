/* bigo_lab_test.c -- real, direct coverage for bigo_lab.h's own crew-shared lab state and real,
 * bounds-checked centrifuge decision (EMILY/BACKLOG.md SECTION 536 follow-up, BIG_O/NORTHSTAR.md
 * §30) -- core/lab_sim.c's first real live consumer. Against the real, unmodified
 * centrifuge_spin (:lab_sim), not a mock.
 */
#include "bigo_lab.h"
#include "papercraft_protocol.h"

#include <assert.h>
#include <stdio.h>

/* The real, protocol-owned wire mirror must never silently drift from the real, live
 * BIGO_LAB_SAMPLE_MAX -- same "can't silently drift apart" precedent bigo_chat_test.c's own
 * BIGO_CHAT_MAX_TEXT check already establishes. */
_Static_assert(BIGO_LAB_SAMPLE_MAX == BIGO_LAB_SAMPLE_MAX_WIRE,
               "bigo_lab.h's BIGO_LAB_SAMPLE_MAX and papercraft_protocol.h's own "
               "BIGO_LAB_SAMPLE_MAX_WIRE mirror must stay equal");

int main(void) {
    /* --- seeding: real, non-degenerate starter material. */
    {
        LabCrewState lab;
        bigo_lab_seed_starter_samples(&lab);
        assert(lab.sample_count == 3);
        for (int i = 0; i < lab.sample_count; i++) {
            assert(lab.samples[i].purity_pct == 0.0f);   /* real, fresh wild harvest -- no spin yet */
            assert(lab.samples[i].generation == 0);       /* real wild harvest, not a bred line */
            assert(lab.samples[i].genetic_drift == 0.0f); /* no breeding has happened yet */
        }
        assert(lab.samples[0].contamination_pct != lab.samples[1].contamination_pct);
        printf("PASS: bigo-lab-seed-starter-samples seeds 3 real, distinct, fresh wild-harvest samples\n");

        for (int i = lab.sample_count; i < BIGO_LAB_SAMPLE_MAX; i++) {
            assert(lab.samples[i].contamination_pct == 0.0f && lab.samples[i].purity_pct == 0.0f);
        }
        printf("PASS: unseeded slots past sample_count are honestly zeroed, not garbage\n");
    }

    /* --- centrifuge: a real, valid spin measurably raises purity via the real, unmodified
     * lab_sim.c centrifuge_spin -- this proves the wrapper is actually calling into it, not just
     * bounds-checking and returning success. */
    {
        LabCrewState lab;
        bigo_lab_seed_starter_samples(&lab);
        float purity_before = lab.samples[0].purity_pct;

        int ok = bigo_lab_centrifuge(&lab, 0);
        assert(ok == 1);
        assert(lab.samples[0].purity_pct > purity_before);
        printf("PASS: a valid centrifuge request succeeds and measurably raises the real sample's purity\n");

        /* every OTHER sample is untouched by a single-index centrifuge call. */
        assert(lab.samples[1].purity_pct == 0.0f && lab.samples[2].purity_pct == 0.0f);
        printf("PASS: a centrifuge request on one sample leaves every other real sample untouched\n");
    }

    /* --- bounds checking: real, honest no-ops, never a crash or an out-of-bounds read/write. */
    {
        LabCrewState lab;
        bigo_lab_seed_starter_samples(&lab);

        assert(bigo_lab_centrifuge(&lab, -1) == 0);
        assert(bigo_lab_centrifuge(&lab, lab.sample_count) == 0);       /* exactly at count -- one past the real, valid range */
        assert(bigo_lab_centrifuge(&lab, BIGO_LAB_SAMPLE_MAX) == 0);    /* well past both count and the fixed array size */
        assert(bigo_lab_centrifuge(&lab, 999) == 0);

        /* real, valid samples are completely untouched by every one of the rejected calls above. */
        for (int i = 0; i < lab.sample_count; i++) {
            assert(lab.samples[i].purity_pct == 0.0f);
        }
        printf("PASS: out-of-bounds centrifuge requests are honest no-ops -- no crash, real sample data untouched\n");
    }

    printf("\nALL PASS\n");
    return 0;
}

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

    /* --- S504-LAB: every remaining station, via the real, unmodified lab_sim.c primitives. */
    {
        LabCrewState lab;
        bigo_lab_seed_starter_samples(&lab);

        float d0 = lab.samples[0].read_depth;
        assert(bigo_lab_station(&lab, BIGO_LAB_ST_PCR, 0, 0) == 1);
        assert(lab.samples[0].read_depth > d0);
        printf("PASS: PCR station raises read depth on the real sample\n");

        assert(bigo_lab_station(&lab, BIGO_LAB_ST_SEQUENCE, 0, 0) == 1);
        assert(lab.meta[0].sequenced == 1 && lab.meta[0].alignment_pct >= 0.0f && lab.meta[0].alignment_pct <= 100.0f);
        assert(lab.meta[1].sequenced == 0);
        printf("PASS: sequencer records a real readout on that sample only\n");

        assert(bigo_lab_station(&lab, BIGO_LAB_ST_SPLICE, 1, 0) == 1);
        assert(lab.meta[1].spliced == 1 && lab.meta[1].last_splice_outcome >= SPLICE_SUCCESS &&
               lab.meta[1].last_splice_outcome <= SPLICE_UNSTABLE_LINE);
        printf("PASS: CRISPR splice records its outcome\n");

        assert(bigo_lab_station(&lab, BIGO_LAB_ST_REPRESSOR, 1, 0) == 1);
        assert(lab.meta[1].repressor_reliability >= 0.0f && lab.meta[1].repressor_reliability <= 1.0f);
        printf("PASS: repressor install records installed/reliability\n");

        int before = lab.sample_count;
        assert(bigo_lab_station(&lab, BIGO_LAB_ST_BREED, 0, 1) == 1);
        assert(lab.sample_count == before + 1);
        assert(lab.samples[before].generation == 1 && lab.samples[before].genetic_drift > 0.0f);
        assert(lab.meta[before].sequenced == 0 && lab.meta[before].spliced == 0);
        printf("PASS: breeding appends a generation-1 child with drift and a clean meta\n");

        assert(bigo_lab_station(&lab, BIGO_LAB_ST_BREED, 0, 0) == 0);      /* with itself */
        assert(bigo_lab_station(&lab, BIGO_LAB_ST_BREED, 0, 99) == 0);     /* bad parent */
        while (lab.sample_count < BIGO_LAB_SAMPLE_MAX) assert(bigo_lab_station(&lab, BIGO_LAB_ST_BREED, 0, 1) == 1);
        assert(bigo_lab_station(&lab, BIGO_LAB_ST_BREED, 0, 1) == 0);      /* full lab */
        printf("PASS: breed rejects self-pairing, bad parent and a full lab\n");

        assert(bigo_lab_station(&lab, BIGO_LAB_ST_INCUBATE, 0, 0) == 1);
        assert(lab.meta[0].incubated == 1 && lab.meta[0].viability_pct >= 0.0f && lab.meta[0].viability_pct <= 100.0f);
        printf("PASS: incubation records viability\n");

        assert(bigo_lab_station(&lab, -1, 0, 0) == 0);
        assert(bigo_lab_station(&lab, BIGO_LAB_ST_COUNT, 0, 0) == 0);
        assert(bigo_lab_station(&lab, BIGO_LAB_ST_PCR, -1, 0) == 0);
        assert(bigo_lab_station(&lab, BIGO_LAB_ST_PCR, lab.sample_count, 0) == 0);
        printf("PASS: bad station / sample index are honest no-ops\n");
    }

    printf("\nALL PASS\n");
    return 0;
}

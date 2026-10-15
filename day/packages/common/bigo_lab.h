/* bigo_lab.h -- the real, first live consumer of core/lab_sim.c (EMILY/BACKLOG.md SECTION 536
 * follow-up, BIG_O/NORTHSTAR.md §9/§30). That module's own doc comment has named the real gap
 * since it landed: "does not wire into the day/ server's BP_APP_LAB UI or IDUNA persistence
 * yet." This closes the SERVER half of that gap -- a real, live, crew-shared sample inventory the
 * day server owns and mutates, plus the first real equipment station (the centrifuge) wired to an
 * actual network packet. Same "server logic first, client UI wiring named as a real, separate
 * follow-up" precedent `server_wheelbarrow_toggle`/`bigo_pheromone.h`/`server_tick_dispatch` each
 * already established successfully in this exact codebase before their own client input landed.
 *
 * Real, honest, deliberate v0 scope: BIG_O's v0 is one shared crew, one basement (NORTHSTAR.md
 * §7) -- so this is ONE real, global, crew-shared LabCrewState, not per-player, matching §7's own
 * "the lab, samples, clones... crew state" line directly. Seeded at startup with a handful of
 * wild-harvest samples -- a real, honest, named placeholder for the still-not-built day-harvest
 * bridge (`lab_sample_init_wild_harvest`'s own real doc comment already names this exact gap; a
 * player cannot yet deposit a real day-phase sample into the lab). Only the centrifuge station is
 * wired live here -- PCR/sequencer/CRISPR-splice/repressor-install/breed/incubate all stay real,
 * tested, unconsumed primitives in lab_sim.c, the same "one real piece landed, the rest named"
 * discipline this whole SECTION 536 thread already uses (chat's "say" among six asks, the
 * pheromone ball among three named command tools, etc.) -- not attempted blind in one shot.
 */
#ifndef BIGO_LAB_H
#define BIGO_LAB_H

#include <string.h>

#include "../../../core/lab_sim.h"

#define BIGO_LAB_SAMPLE_MAX 6

/* A fixed, real "standard spin" preset -- the phone UI has no float-input affordance (D-pad
 * menus only, matching every other BIG_O app except the free-text GFD terminal), so v0 offers one
 * real, tuned setting rather than a mini-game of arbitrary minutes/rpm. Chosen to sit comfortably
 * under lab_sim.c's own real over-spin integrity-damage threshold on a fresh sample, while still
 * moving purity meaningfully in one real application -- a future preset menu (quick/standard/
 * aggressive) is real, separate, additive follow-up, not built here. */
#define BIGO_LAB_CENTRIFUGE_MINUTES 8.0f
#define BIGO_LAB_CENTRIFUGE_RPM 4000.0f

/* S504-LAB: the remaining lab_sim.c stations (PCR, sequencer, CRISPR splice, repressor install,
 * breeding, incubation) are wired live too -- one generic server-authoritative station call
 * (bigo_lab_station). Their results are not LabSample fields, so each slot carries a real
 * LabSampleMeta alongside it (same index). Fixed presets again -- the phone is a D-pad UI, no
 * float input: PCR 10 cycles, guide specificity 0.8, operator skill 0.5 (the micromanipulation
 * mini-game's own future plug-in point, per lab_sim.h). */
#define BIGO_LAB_PCR_CYCLES 10
#define BIGO_LAB_GUIDE_SPECIFICITY 0.8f
#define BIGO_LAB_OPERATOR_SKILL 0.5f

typedef enum {
    BIGO_LAB_ST_CENTRIFUGE = 0,
    BIGO_LAB_ST_PCR,
    BIGO_LAB_ST_SEQUENCE,
    BIGO_LAB_ST_SPLICE,
    BIGO_LAB_ST_REPRESSOR,
    BIGO_LAB_ST_BREED,     /* sample_index x aux_index -> a new child sample appended */
    BIGO_LAB_ST_INCUBATE,
    BIGO_LAB_ST_COUNT
} BigoLabStation;

typedef struct {
    int sequenced;                    /* 1 once the sequencer has read this sample */
    float measured_contamination_pct; /* sequencer's noisy estimate, valid when sequenced */
    float alignment_pct;
    int spliced;                      /* 1 once any CRISPR splice was attempted */
    int last_splice_outcome;          /* SpliceOutcome */
    int retrotransposon_jump;         /* last splice's side-effect flag */
    int repressor_installed;
    float repressor_reliability;
    int incubated;                    /* 1 once incubation was run */
    int viable;
    float viability_pct;
} LabSampleMeta;

typedef struct {
    LabSample samples[BIGO_LAB_SAMPLE_MAX];
    LabSampleMeta meta[BIGO_LAB_SAMPLE_MAX];
    int sample_count;
} LabCrewState;

/* bigo_lab_seed_starter_samples -- real, honest v0 placeholder for the not-yet-built day-harvest
 * bridge (named above): three fresh wild-harvest samples at varied contamination, so the
 * centrifuge station has real, non-trivial material to act on from server startup rather than an
 * empty crew inventory nothing can ever fill. */
static inline void bigo_lab_seed_starter_samples(LabCrewState *lab) {
    lab->sample_count = 3;
    lab_sample_init_wild_harvest(&lab->samples[0], 12.0f);
    lab_sample_init_wild_harvest(&lab->samples[1], 35.0f);
    lab_sample_init_wild_harvest(&lab->samples[2], 60.0f);
    for (int i = lab->sample_count; i < BIGO_LAB_SAMPLE_MAX; i++) {
        memset(&lab->samples[i], 0, sizeof(LabSample));
    }
    memset(lab->meta, 0, sizeof(lab->meta));
}

/* bigo_lab_centrifuge -- real, bounds-checked decision, independently callable and testable
 * (same "extract the real decision" discipline server_wheelbarrow_toggle/server_throw_pheromone
 * already established): an out-of-range or empty-slot index is an honest no-op (returns 0),
 * never a crash or undefined read. On success, mutates the real sample in place via lab_sim.c's
 * own real, already-tested centrifuge_spin and returns 1. */
static inline int bigo_lab_centrifuge(LabCrewState *lab, int sample_index) {
    if (sample_index < 0 || sample_index >= lab->sample_count) return 0;
    centrifuge_spin(&lab->samples[sample_index], BIGO_LAB_CENTRIFUGE_MINUTES, BIGO_LAB_CENTRIFUGE_RPM);
    return 1;
}

/* bigo_lab_station -- real, bounds-checked dispatch to every lab_sim.c station (returns 1 on a
 * real mutation, 0 on an honest no-op: bad index/station, empty slot, breed with itself or a full
 * lab). Each call mutates the real sample/meta in place via the real, unmodified primitives.
 *  - an embryo that has been spliced/incubated keeps its record in meta; breeding resets the
 *    child's meta (a fresh line has no splice/sequencing history of its own).
 *  - splice/repressor results are recorded, never gated: a failure is a real outcome the crew
 *    must read off the sample, not an error. */
static inline int bigo_lab_station(LabCrewState *lab, int station, int sample_index, int aux_index) {
    if (station < 0 || station >= BIGO_LAB_ST_COUNT) return 0;
    if (sample_index < 0 || sample_index >= lab->sample_count) return 0;
    LabSample *s = &lab->samples[sample_index];
    LabSampleMeta *m = &lab->meta[sample_index];
    switch (station) {
    case BIGO_LAB_ST_CENTRIFUGE:
        centrifuge_spin(s, BIGO_LAB_CENTRIFUGE_MINUTES, BIGO_LAB_CENTRIFUGE_RPM);
        return 1;
    case BIGO_LAB_ST_PCR:
        pcr_amplify(s, BIGO_LAB_PCR_CYCLES);
        return 1;
    case BIGO_LAB_ST_SEQUENCE: {
        SequencerReadout r = sequencer_run(s);
        m->sequenced = 1;
        m->measured_contamination_pct = r.measured_contamination_pct;
        m->alignment_pct = r.alignment_pct;
        return 1;
    }
    case BIGO_LAB_ST_SPLICE: {
        SpliceResult r = crispr_splice(s, BIGO_LAB_GUIDE_SPECIFICITY, BIGO_LAB_OPERATOR_SKILL);
        m->spliced = 1;
        m->last_splice_outcome = (int)r.outcome;
        m->retrotransposon_jump = r.retrotransposon_jump;
        return 1;
    }
    case BIGO_LAB_ST_REPRESSOR: {
        RepressorInstallResult r = crispr_install_repressor(s, BIGO_LAB_GUIDE_SPECIFICITY, BIGO_LAB_OPERATOR_SKILL);
        m->repressor_installed = r.installed;
        m->repressor_reliability = r.reliability_0_to_1;
        return 1;
    }
    case BIGO_LAB_ST_BREED: {
        if (aux_index < 0 || aux_index >= lab->sample_count || aux_index == sample_index) return 0;
        if (lab->sample_count >= BIGO_LAB_SAMPLE_MAX) return 0;
        lab->samples[lab->sample_count] = clone_breed(&lab->samples[sample_index], &lab->samples[aux_index]);
        memset(&lab->meta[lab->sample_count], 0, sizeof(LabSampleMeta));
        lab->sample_count++;
        return 1;
    }
    case BIGO_LAB_ST_INCUBATE: {
        IncubationResult r = incubate_embryo(s, (SpliceOutcome)m->last_splice_outcome);
        m->incubated = 1;
        m->viable = r.viable;
        m->viability_pct = r.viability_pct;
        return 1;
    }
    }
    return 0;
}

#endif /* BIGO_LAB_H */

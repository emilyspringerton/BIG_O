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

typedef struct {
    LabSample samples[BIGO_LAB_SAMPLE_MAX];
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

#endif /* BIGO_LAB_H */

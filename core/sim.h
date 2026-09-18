/* BIG_O crew simulation host (B1). Hand-written C around the PARENA-generated rules (witness_rules.h). Owns the things
 * the rules module deliberately does not: who is in which zone, who witnesses what, the seeded RNG (rolls are drawn
 * here and passed into the pure rules), and the deterministic event log. Spec: docs/B1_WITNESS_RULES.md section 7. */
#ifndef BIGO_SIM_H
#define BIGO_SIM_H
#include <stdint.h>
#include <stdio.h>

#define SIM_MAX_PLAYERS 3
#define SIM_MAX_NPCS 16

typedef struct { int present, zone, costume, token, gear, decorum, hunted, cancelled; } SimPlayer;
typedef struct { int zone, vigilance, arrogance, state, accomplice, witnessed_event; } SimNpc;

typedef struct {
    uint32_t rng;
    int tick, event_id, nplayers, nnpcs;
    SimPlayer p[SIM_MAX_PLAYERS];
    SimNpc n[SIM_MAX_NPCS];
    FILE *out; /* deterministic event log sink */
} Sim;

void sim_init(Sim *s, uint32_t seed, int nplayers, FILE *out);
int sim_add_npc(Sim *s, int zone, int vigilance, int arrogance);      /* returns npc id or -1 */
int sim_enter(Sim *s, int pl, int zone);
int sim_set_costume(Sim *s, int pl, int costume);
int sim_set_gear(Sim *s, int pl, int gear);
int sim_set_token(Sim *s, int pl, int token);
int sim_observe(Sim *s, int pl);                                       /* noticing check for the current costume/zone/gear */
int sim_say_apocalypse(Sim *s, int pl);
int sim_talk(Sim *s, int pl);
int sim_release(Sim *s, int pl, int tier);                             /* loud zombie event, attributed to pl or the crew */
int sim_force_witness(Sim *s, int npc, int pl);                        /* private forced witness -> accomplice */
void sim_tick(Sim *s, int n);
void sim_print_state(const Sim *s, FILE *out);

const char *sim_ws_name(int state);
const char *sim_band_name(int band);
const char *sim_zone_name(int zone);
const char *sim_costume_name(int costume);
#endif

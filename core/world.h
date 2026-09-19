/* World layer (B2 groundwork): host state for the day/night clock, weather and the day-time zombie population. Every
 * decision is a call into PARENA-generated rules (world_rules.h); this file only holds state, rolls dice, executes, and
 * announces what happened on the REFLUX log so mods can hook in (see reflux_runtime.h, PARENA/stdlib/big_o/world_alerts_mod.prn).
 * One sim tick = one minute. Deterministic (own seeded RNG). Spec: docs/B2_WORLD.md. */
#ifndef BIGO_WORLD_H
#define BIGO_WORLD_H
#include <stdint.h>
#include "sim.h"
#include "reflux_runtime.h"

typedef enum { PH_DAWN = 0, PH_DAY, PH_DUSK, PH_NIGHT } Phase;
typedef enum { WX_CLEAR = 0, WX_OVERCAST, WX_RAIN, WX_STORM } Weather;
enum { AREA_WASTELAND = 0, AREA_NEXTOWN, AREA_OFFICE, AREA_PARK, AREA_BASEMENT, AREA_COUNT };

#define WORLD_MAX_ZOMBIES 64
#define WORLD_MAX_ALERTS 16

typedef struct { int area, tier, alive; } Zombie;

typedef struct {
    uint32_t rng;
    int minutes, start_minute;
    Weather weather; int weather_ends;            /* absolute world minute the current weather expires */
    Zombie z[WORLD_MAX_ZOMBIES]; int nz;
    int area[SIM_MAX_PLAYERS];
    int samples[SIM_MAX_PLAYERS][3];
    int spawned_total, culled_total;
    RefluxLog reflux;                             /* the hook point for mods */
    int alert_cursor;                             /* world_alerts_mod's own subscriber cursor */
    int alerts[WORLD_MAX_ALERTS]; int nalerts;    /* phone message ids raised by the alerts mod, oldest first */
} World;

void world_init(World *w, Sim *s, uint32_t seed, int start_hour);
int world_minute_of_day(const World *w);
int world_day(const World *w);
Phase world_phase(const World *w);
int world_area_cap(const World *w, int area);
int world_zombies_in(const World *w, int area);
void world_tick(World *w, Sim *s, int minutes);
int world_set_area(World *w, Sim *s, int pl, int area);
int world_harvest(World *w, Sim *s, int pl);
void world_force_weather(World *w, Sim *s, Weather wx);   /* GM/test hook, like GFD's ForcePhase */
int world_pop_alert(World *w);                            /* next raised phone message id, 0 if none */
const char *world_phase_name(Phase p);
const char *world_weather_name(Weather wx);
const char *world_area_name(int area);
#endif

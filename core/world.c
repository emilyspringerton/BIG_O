#include <stdio.h>
#include <string.h>
#include "world.h"
#include "world_rules.h"

static const char *const AREA_NAMES[AREA_COUNT] = { "WASTELAND", "NEXTOWN", "OFFICE", "PARK", "BASEMENT" };
static const char *const PHASE_NAMES[4] = { "DAWN", "DAY", "DUSK", "NIGHT" };
static const char *const WX_NAMES[4] = { "CLEAR", "OVERCAST", "RAIN", "STORM" };

static int wroll(World *w) {
    w->rng ^= w->rng << 13; w->rng ^= w->rng >> 17; w->rng ^= w->rng << 5;
    return (int)(w->rng % 100u);
}

static void schedule_weather(World *w) { w->weather_ends = w->minutes + weather_duration((int)w->weather, wroll(w)); }

static void apply_weather_to_sim(World *w, Sim *s) { s->public_sight_pct = weather_sight_pct((int)w->weather); }

void world_init(World *w, Sim *s, uint32_t seed, int start_hour) {
    memset(w, 0, sizeof(*w));
    w->rng = seed ? seed * 2654435761u | 1u : 0x9E3779B9u;
    w->start_minute = ((start_hour % 24) + 24) % 24 * 60;
    for (int p = 0; p < SIM_MAX_PLAYERS; p++) w->area[p] = AREA_OFFICE;
    w->weather = WX_CLEAR;
    schedule_weather(w);
    apply_weather_to_sim(w, s);
}

int world_minute_of_day(const World *w) { return (w->start_minute + w->minutes) % 1440; }
int world_day(const World *w) { return 1 + (w->start_minute + w->minutes) / 1440; }
Phase world_phase(const World *w) { return (Phase)phase_for_minute(world_minute_of_day(w)); }
int world_area_cap(const World *w, int area) { return area_cap(area, (int)world_phase(w), (int)w->weather); }

int world_zombies_in(const World *w, int area) {
    int n = 0;
    for (int i = 0; i < w->nz; i++) if (w->z[i].alive && w->z[i].area == area) n++;
    return n;
}

/* Poll the log from the alerts mod's own cursor: the mod decides whether an event deserves a phone message and which. */
static void poll_alerts(World *w, Sim *s) {
    if (w->alert_cursor < reflux_log_oldest(&w->reflux)) w->alert_cursor = reflux_log_oldest(&w->reflux);
    for (; w->alert_cursor < w->reflux.total_dispatched; w->alert_cursor++) {
        const RefluxAction *x = reflux_log_at(&w->reflux, w->alert_cursor);
        if (!x || !alert_should_react(x->action_type)) continue;
        int msg = alert_message_id(x->action_type, x->a, x->b);
        if (msg <= 0) continue;
        if (w->nalerts < WORLD_MAX_ALERTS) w->alerts[w->nalerts++] = msg;
        fprintf(s->out, "WORLD alert -> phone message %d\n", msg);
    }
}

int world_pop_alert(World *w) {
    if (w->nalerts == 0) return 0;
    int m = w->alerts[0];
    memmove(w->alerts, w->alerts + 1, sizeof(int) * (size_t)(--w->nalerts));
    return m;
}

static void spawn(World *w, Sim *s, int area) {
    int tier = spawn_tier(wroll(w)), slot = -1;
    for (int i = 0; i < w->nz; i++) if (!w->z[i].alive) { slot = i; break; }
    if (slot < 0) { if (w->nz >= WORLD_MAX_ZOMBIES) return; slot = w->nz++; }
    w->z[slot].area = area; w->z[slot].tier = tier; w->z[slot].alive = 1;
    w->spawned_total++;
    fprintf(s->out, "WORLD spawn zombie tier=%d in %s (day %d %02d:%02d, %s)\n", tier, AREA_NAMES[area], world_day(w),
            world_minute_of_day(w) / 60, world_minute_of_day(w) % 60, WX_NAMES[w->weather]);
    reflux_log_dispatch(&w->reflux, BIGO_EVT_ZOMBIE_SPAWNED, area, tier, slot);
}

static void cull_to_cap(World *w, Sim *s, int area, int cap) {
    for (int i = w->nz - 1; i >= 0 && world_zombies_in(w, area) > cap; i--)
        if (w->z[i].alive && w->z[i].area == area) {
            w->z[i].alive = 0; w->culled_total++;
            fprintf(s->out, "WORLD zombie tier=%d gone from %s\n", w->z[i].tier, AREA_NAMES[area]);
        }
}

static void set_weather(World *w, Sim *s, Weather wx) {
    Weather old = w->weather;
    w->weather = wx;
    schedule_weather(w);
    apply_weather_to_sim(w, s);
    fprintf(s->out, "WORLD weather %s -> %s (lasts until minute %d)\n", WX_NAMES[old], WX_NAMES[wx], w->weather_ends);
    reflux_log_dispatch(&w->reflux, BIGO_EVT_WEATHER_CHANGED, (int)old, (int)wx, weather_zombie_pct((int)wx));
}

void world_force_weather(World *w, Sim *s, Weather wx) { set_weather(w, s, wx); poll_alerts(w, s); }

void world_tick(World *w, Sim *s, int minutes) {
    for (int m = 0; m < minutes; m++) {
        Phase before = world_phase(w);
        w->minutes++;
        Phase ph = world_phase(w);
        if (ph != before) {
            fprintf(s->out, "WORLD phase %s -> %s (day %d)\n", PHASE_NAMES[before], PHASE_NAMES[ph], world_day(w));
            reflux_log_dispatch(&w->reflux, BIGO_EVT_PHASE_CHANGED, (int)before, (int)ph, world_day(w));
        }
        if (w->minutes >= w->weather_ends) set_weather(w, s, (Weather)weather_next((int)w->weather));
        for (int a = 0; a < AREA_COUNT; a++) {
            int cap = world_area_cap(w, a);
            cull_to_cap(w, s, a, cap);
            if (w->minutes % spawn_period_minutes() == 0 && world_zombies_in(w, a) < cap) spawn(w, s, a);
        }
        poll_alerts(w, s);
    }
    sim_tick(s, minutes);
}

int world_set_area(World *w, Sim *s, int pl, int area) {
    if (pl < 0 || pl >= s->nplayers || !s->p[pl].present || area < 0 || area >= AREA_COUNT) return -1;
    w->area[pl] = area;
    fprintf(s->out, "WORLD p%d goes to %s\n", pl, AREA_NAMES[area]);
    return 0;
}

int world_harvest(World *w, Sim *s, int pl) {
    if (pl < 0 || pl >= s->nplayers || !s->p[pl].present) return -1;
    for (int i = 0; i < w->nz; i++) {
        if (w->z[i].alive && w->z[i].area == w->area[pl]) {
            w->z[i].alive = 0;
            w->samples[pl][w->z[i].tier]++;
            sim_set_gear(s, pl, 1);
            fprintf(s->out, "WORLD p%d harvested a tier-%d sample in %s (now carrying field gear)\n", pl, w->z[i].tier, AREA_NAMES[w->area[pl]]);
            reflux_log_dispatch(&w->reflux, BIGO_EVT_ZOMBIE_HARVESTED, w->area[pl], w->z[i].tier, pl);
            return w->z[i].tier;
        }
    }
    fprintf(s->out, "WORLD p%d harvest: nothing to harvest in %s\n", pl, AREA_NAMES[w->area[pl]]);
    return -1;
}

const char *world_phase_name(Phase p) { return (p >= 0 && p < 4) ? PHASE_NAMES[p] : "?"; }
const char *world_weather_name(Weather x) { return (x >= 0 && x < 4) ? WX_NAMES[x] : "?"; }
const char *world_area_name(int a) { return (a >= 0 && a < AREA_COUNT) ? AREA_NAMES[a] : "?"; }

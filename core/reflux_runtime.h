/* REFLUX for BIG_O: a shared, append-only action log. Dispatchers append {type,a,b,c}; subscribers (PARENA mods, or native
 * code) poll it from their own cursor. No callbacks: PARENA VS0 has no closures. Same design as SHANKPIT/ECOWAR REFLUX
 * (their action types 1-4 belong to SHANKPIT's level objects; BIG_O's world events live in the 100s so both can coexist).
 * Payload is three raw I32 scalars, per-type documented below. */
#ifndef BIGO_REFLUX_H
#define BIGO_REFLUX_H
#define BIGO_EVT_PHASE_CHANGED    101 /* a=old phase b=new phase c=day number */
#define BIGO_EVT_WEATHER_CHANGED  102 /* a=old weather b=new weather c=zombie density pct */
#define BIGO_EVT_ZOMBIE_SPAWNED   103 /* a=area b=tier c=zombie id */
#define BIGO_EVT_ZOMBIE_HARVESTED 104 /* a=area b=tier c=player */
#define REFLUX_LOG_CAPACITY 256
typedef struct { int action_type, a, b, c; } RefluxAction;
typedef struct { RefluxAction actions[REFLUX_LOG_CAPACITY]; int total_dispatched; } RefluxLog;
void reflux_log_reset(RefluxLog *log);
void reflux_log_dispatch(RefluxLog *log, int type, int a, int b, int c);
/* Cursor API: total_dispatched is monotonic; a subscriber keeps its own `cursor` (start 0) and reads [cursor, total). If it
 * fell more than CAPACITY behind, reflux_log_at returns NULL for the overwritten range: skip to reflux_log_oldest(). */
int reflux_log_oldest(const RefluxLog *log);
const RefluxAction *reflux_log_at(const RefluxLog *log, int abs_index);
#endif

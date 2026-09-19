/* Declarations for the PARENA-generated world rules (core/world_rules.c from PARENA/stdlib/big_o/world_rules.prn,
 * core/world_alerts.c from world_alerts_mod.prn) -- do not edit the .c files by hand. */
#ifndef BIGO_WORLD_RULES_H
#define BIGO_WORLD_RULES_H
int phase_for_minute(int);
int weather_next(int);
int weather_min_minutes(int);
int weather_max_minutes(int);
int weather_duration(int, int);
int weather_zombie_pct(int);
int weather_sight_pct(int);
int weather_mob_damage_bonus(int);
int day_cap(int);
int night_cap(int);
int area_cap(int, int, int);
int spawn_period_minutes(void);
int spawn_tier(int);
int alert_should_react(int);
int alert_message_id(int, int, int);
#endif

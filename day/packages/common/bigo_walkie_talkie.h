/* bigo_walkie_talkie.h -- real host half of the walkie-talkie channel/hearing decision logic.
 * Brought back from SHANKPIT (EMILY/BACKLOG.md SECTION 536, "bring SHANKPIT stuff back into
 * BIG_O" follow-up) -- built there natively, forward of the original BIG_O engine merge, founder
 * real-time, 2026-09-23: "add walkie talkie voice coms asterisk based (we have a real asterisk
 * server) parena powered". Wraps PARENA-generated walkie_rules.c's own two pure decision
 * functions (channel assignment, hearing gate) -- pure, header-only, caller-owns-state, same
 * "extract testable logic, no hidden global state" discipline bigo_pheromone.h already
 * established (unlike SHANKPIT's own version, which kept a hidden g_players[] array internally).
 *
 * REAL, HONEST, NAMED GAP #1: this is channel/permission logic only, NOT audio transport. BIG_O
 * has zero microphone capture, codec, or SIP/RTP client code anywhere -- building real voice is a
 * genuinely large, separate follow-up, same real gap SHANKPIT's own version named.
 *
 * REAL, HONEST, NAMED GAP #2, found during this port (bigger than gap #1): BIG_O's v0 has exactly
 * ONE shared crew/world (NORTHSTAR.md §7's own "one crew, one onboarding" decision) -- there is no
 * team/crew concept for players at all yet. Every player would resolve to the same team_id, so
 * walkie-can-hear degenerates to "always true" and walkie-team-channel-ext always resolves to the
 * same channel. This module is therefore a real, standalone, tested primitive with NO LIVE
 * CONSUMER yet -- same "correct primitive, no consumer" pattern SHANKPIT's own phases 2/3/5 of
 * the original BIG_O engine merge already established, just running in the other direction. Real
 * wiring needs a real team/crew-assignment system first; not guessed at or half-built here.
 */
#ifndef BIGO_WALKIE_TALKIE_H
#define BIGO_WALKIE_TALKIE_H

#define BIGO_WALKIE_OVERHEAR_RADIUS_M 8.0f /* matches walkie_rules.prn's own 800cm distance-cm gate */
#define BIGO_WALKIE_NO_TEAM (-1)

/* PARENA-generated (day/packages/simulation/walkie_rules.c, from
 * PARENA/stdlib/big_o/walkie_rules.prn). Do not edit that file by hand -- regenerate via
 * `parena build`, same convention scripts/gen_rules.sh already establishes for core/. */
int walkie_team_channel_ext(int team_id);
int walkie_can_hear(int listener_team, int speaker_team, int distance_cm);

/* bigo_walkie_channel_for_team -- the channel a given team transmits/listens on (2999 for
 * BIGO_WALKIE_NO_TEAM, 3000+team_id otherwise). */
static inline int bigo_walkie_channel_for_team(int team_id) {
    return walkie_team_channel_ext(team_id);
}

/* bigo_walkie_can_hear -- would a listener on listener_team hear a live transmission from a
 * speaker on speaker_team at the given real, non-negative distance in meters? Same team -> always.
 * Different team -> only within BIGO_WALKIE_OVERHEAR_RADIUS_M (a real walkie-talkie speaker
 * leaking sound into the world). */
static inline int bigo_walkie_can_hear(int listener_team, int speaker_team, float distance_m) {
    int distance_cm = (int)(distance_m * 100.0f);
    if (distance_cm < 0) distance_cm = 0;
    return walkie_can_hear(listener_team, speaker_team, distance_cm);
}

#endif

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
 * REAL, HONEST, NAMED GAP #2 (found during this port, UPDATED 2026-09-24): BIG_O's v0 has exactly
 * ONE shared crew/world (NORTHSTAR.md §7's own "one crew, one onboarding" decision) -- there is
 * still no team/crew concept for PLAYERS at all. That original blocker is still real and still
 * unsolved for players. It stopped being this whole module's blocker, though: founder real-time,
 * 2026-09-24, "The Men should have their own channel" -- The Men are NPCs, not players, and this
 * repo already tracks them individually (ServerNpc, PC_NPC_ROLE_THE_MEN) with no dependency on a
 * player-team system at all. BIGO_WALKIE_TEAM_THE_MEN (below) gives every Man a real, live,
 * shared channel; day/apps/server/src/main.c's own server_walkie_transmit is this module's real,
 * live day-server consumer as of that date -- see BIG_O/NORTHSTAR.md §28 for the full account.
 * The player-side gap is unaffected and still open, named honestly, not solved here.
 */
#ifndef BIGO_WALKIE_TALKIE_H
#define BIGO_WALKIE_TALKIE_H

#define BIGO_WALKIE_OVERHEAR_RADIUS_M 8.0f /* matches walkie_rules.prn's own 800cm distance-cm gate */
#define BIGO_WALKIE_NO_TEAM (-1)

/* BIGO_WALKIE_TEAM_THE_MEN -- 2026-09-24 real live consumer, closing this header's own
 * previously-named "no live consumer yet" gap (see the doc comment above): The Men NPCs
 * (PC_NPC_ROLE_THE_MEN, day/apps/server/src/main.c) all share this one, fixed walkie team so
 * they can hear each other's real dispatch radio calls, independent of the still-missing player-
 * team system this module was originally scoped against. A future real player-team system must
 * pick its own team ids starting elsewhere (e.g. >= 1) so it never collides with this reserved
 * NPC-only channel. */
#define BIGO_WALKIE_TEAM_THE_MEN 0

/* PARENA-generated (day/packages/simulation/walkie_rules.c, from
 * PARENA/stdlib/big_o/walkie_rules.prn). Do not edit that file by hand -- regenerate via
 * `parena build`, same convention scripts/gen_rules.sh already establishes for core/. */
int walkie_team_channel_ext(int team_id);
int walkie_can_hear(int listener_team, int speaker_team, int distance_cm);

/* PARENA-generated (day/packages/simulation/walkie_callsign.c, from
 * PARENA/stdlib/big_o/walkie_callsign.prn) -- real decision: which canned radio-call line-id a
 * Man's own real dispatch event represents. See that .prn file's own doc comment for the full
 * real scope cut (a genuine text-to-speech GENERATOR cannot exist in PARENA at all -- VS0 is
 * I32/Bool-scalar-only, no strings/audio/FFI -- and BIG_O's client has no audio subsystem of any
 * kind yet; this owns the real decision half only, matching every other "mods first everything"
 * split in this repo). */
int event_dispatched(void);
int event_resolved(void);
int callsign_none(void);
int callsign_responding_quiet(void);
int callsign_responding_code_red(void);
int callsign_resolved(void);
int walkie_callsign_for_event(int event, int witness_state);

/* BIGO_WALKIE_CALLSIGN_TEXT -- the real, host-owned English text per callsign-*() line-id
 * (0=none..3=resolved), same "PARENA returns an id, host owns display text" split
 * BP_PHONE_MESSAGE_TABLE-style lookups already use elsewhere in this repo. This IS the real
 * "robot speak" content for now -- printed/logged, not synthesized into audio (see
 * walkie_callsign.prn's own doc comment for the real, honest, not-yet-built audio path). Indexed
 * directly by the real callsign-*() return value, not a separate mapping table. */
static const char *const BIGO_WALKIE_CALLSIGN_TEXT[4] = {
    "",
    "[ROBOT VOICE] UNIT RESPONDING. QUIET PROTOCOL.",
    "[ROBOT VOICE] UNIT RESPONDING. CODE RED.",
    "[ROBOT VOICE] SITUATION RESOLVED. STANDING DOWN.",
};

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

/* bigo_walkie_callsign_text_for_event -- the real, host-owned text line for a Man's dispatch
 * event (event_dispatched()/event_resolved(), witness_state the real live WS_* value at the
 * moment of the call). Returns "" for callsign_none() (an out-of-range/unrecognized
 * witness_state at dispatch time) so a caller can safely print it unconditionally. */
static inline const char *bigo_walkie_callsign_text_for_event(int event, int witness_state) {
    int id = walkie_callsign_for_event(event, witness_state);
    if (id < 0 || id > 3) id = 0;
    return BIGO_WALKIE_CALLSIGN_TEXT[id];
}

#endif

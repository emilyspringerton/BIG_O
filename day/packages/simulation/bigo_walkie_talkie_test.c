/* bigo_walkie_talkie_test.c -- real, direct coverage for bigo_walkie_talkie.h's pure channel/
 * hearing decisions, against the real, live PARENA-compiled walkie_rules.c (not a mock) -- same
 * real cross-package shape papercraft_inventory_test.c already establishes.
 */
#include "../common/bigo_walkie_talkie.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    assert(bigo_walkie_channel_for_team(2) == 3002);
    printf("PASS: a team-assigned player's channel is the real 3000+team_id extension\n");

    assert(bigo_walkie_channel_for_team(BIGO_WALKIE_NO_TEAM) == 2999);
    printf("PASS: an unassigned player's channel is the real shared 2999 local-chatter channel\n");

    /* Same team -- always heard, regardless of real, large distance. */
    assert(bigo_walkie_can_hear(1, 1, 500.0f));
    printf("PASS: same-team transmissions are heard for real at any real distance\n");

    /* Different team -- only within the real overhear radius. */
    assert(bigo_walkie_can_hear(1, 2, 3.0f));   /* close -- overheard */
    assert(!bigo_walkie_can_hear(1, 2, 20.0f)); /* far -- not overheard */
    printf("PASS: cross-team transmissions are only overheard within the real radius\n");

    /* A real, negative distance clamps to 0 (always heard) rather than going undefined. */
    assert(bigo_walkie_can_hear(1, 2, -5.0f));
    printf("PASS: a real negative distance clamps to 0, not undefined behavior\n");

    /* The Men's own real, fixed, dedicated channel -- SECTION 536 follow-up, 2026-09-24 (real
       live consumer, closing this header's own previously-named "no live consumer yet" gap). */
    assert(BIGO_WALKIE_TEAM_THE_MEN == 0);
    assert(bigo_walkie_channel_for_team(BIGO_WALKIE_TEAM_THE_MEN) == 3000);
    assert(bigo_walkie_can_hear(BIGO_WALKIE_TEAM_THE_MEN, BIGO_WALKIE_TEAM_THE_MEN, 500.0f));
    printf("PASS: The Men's own dedicated channel (0 -> ext 3000) is always heard by every other Man\n");

    /* walkie_callsign_for_event -- dispatched-while-WS_ENGAGE (5) and dispatched-while-
       WS_SILENCING (3) get real, genuinely different callsigns; resolved is always the same real
       all-clear regardless of the state it resolved from; an unrecognized witness_state at
       dispatch time degrades to callsign_none(), not a guess. */
    assert(walkie_callsign_for_event(event_dispatched(), 3 /* WS_SILENCING */) == callsign_responding_quiet());
    assert(walkie_callsign_for_event(event_dispatched(), 5 /* WS_ENGAGE */) == callsign_responding_code_red());
    assert(walkie_callsign_for_event(event_dispatched(), 0 /* WS_UNAWARE, real degrade */) == callsign_none());
    assert(walkie_callsign_for_event(event_resolved(), 3) == callsign_resolved());
    assert(walkie_callsign_for_event(event_resolved(), 5) == callsign_resolved());
    printf("PASS: real, distinct callsigns for quiet vs code-red dispatch; resolved is always the same all-clear\n");

    /* bigo_walkie_callsign_text_for_event -- the real host-owned text lookup, including the real
       "" no-op for callsign_none() a caller can safely check without a separate id comparison. */
    assert(strcmp(bigo_walkie_callsign_text_for_event(event_dispatched(), 5), BIGO_WALKIE_CALLSIGN_TEXT[callsign_responding_code_red()]) == 0);
    assert(bigo_walkie_callsign_text_for_event(event_dispatched(), 0)[0] == '\0');
    printf("PASS: real text lookup matches the id, real \"\" no-op for callsign_none()\n");

    printf("\nALL PASS\n");
    return 0;
}

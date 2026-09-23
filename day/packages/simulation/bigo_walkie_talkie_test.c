/* bigo_walkie_talkie_test.c -- real, direct coverage for bigo_walkie_talkie.h's pure channel/
 * hearing decisions, against the real, live PARENA-compiled walkie_rules.c (not a mock) -- same
 * real cross-package shape papercraft_inventory_test.c already establishes.
 */
#include "../common/bigo_walkie_talkie.h"

#include <assert.h>
#include <stdio.h>

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

    printf("\nALL PASS\n");
    return 0;
}

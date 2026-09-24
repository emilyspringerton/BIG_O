/* bigo_chat_test.c -- real, direct coverage for bigo_chat.h's own range/length decisions, against
 * the real, live PARENA-compiled chat_rules.c (not a mock) -- same real cross-package shape
 * bigo_party_test.c already establishes. Faithfully checks GFD's own real server/chat/chat.go
 * "say" behavior, reverse-ported.
 */
#include "../common/bigo_chat.h"
#include "../common/papercraft_protocol.h"

#include <assert.h>
#include <stdio.h>

/* papercraft_protocol.h's PcChatSayPacket/PcChatRecvPacket hardcode 96 numerically (that file is
   the lower layer, it doesn't depend on bigo_chat.h) -- this keeps the two from silently drifting
   apart, same real intent the struct's own doc comment names. */
_Static_assert(sizeof(((PcChatSayPacket *)0)->text) == BIGO_CHAT_MAX_TEXT,
               "PcChatSayPacket.text must match BIGO_CHAT_MAX_TEXT");
_Static_assert(sizeof(((PcChatRecvPacket *)0)->text) == BIGO_CHAT_MAX_TEXT,
               "PcChatRecvPacket.text must match BIGO_CHAT_MAX_TEXT");

int main(void) {
    assert(chat_say_radius_cm() == 4000);
    assert(chat_max_len() == 200);
    printf("PASS: real constants match GFD's own chat.go (200-byte cap) and this repo's own chosen 40m say radius\n");

    /* Length gate: chat.go's own real Deliver() guard -- non-empty, <= 200 bytes -- AND this
       repo's own smaller fixed wire buffer. */
    assert(!bigo_chat_len_ok(0));
    assert(!bigo_chat_len_ok(-1));
    assert(bigo_chat_len_ok(1));
    assert(bigo_chat_len_ok(BIGO_CHAT_MAX_TEXT));
    assert(!bigo_chat_len_ok(BIGO_CHAT_MAX_TEXT + 1));
    assert(!bigo_chat_len_ok(200)); /* chat.go's own cap would allow it, but it doesn't fit the wire buffer */
    printf("PASS: empty/negative/over-wire-buffer messages rejected; in-range messages accepted\n");

    /* Range: at or under the say radius hears it; strictly beyond does not. Sender always hears
       their own say (distance 0 by construction). */
    assert(bigo_chat_hears(0));
    assert(bigo_chat_hears(3999));
    assert(bigo_chat_hears(4000));
    assert(!bigo_chat_hears(4001));
    printf("PASS: say is heard at or under the real radius, not beyond it; distance 0 (self) always hears\n");

    printf("\nALL PASS\n");
    return 0;
}

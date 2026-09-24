/* bigo_gfd_bridge_test.c -- hermetic coverage for bigo_gfd_bridge.h's own pure logic: JSON-array
 * object walking, source tagging, and the ring-buffer push/drain (including overflow). No
 * network at all -- the real HTTP round trip (auth/post/poll against the actual running IDUNA)
 * was verified live separately, not here; see BIG_O/NORTHSTAR.md §27. EMILY/BACKLOG.md SECTION
 * 536 follow-up.
 */
#include "../common/bigo_gfd_bridge.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    /* Test 1: bigo_bridge_next_json_object walks a real-shaped chat_messages array, including a
       message body containing a literal '{'/'}' (must not desync the brace-depth scan). */
    {
        const char json[] =
            "[{\"id\":1,\"channel\":\"say\",\"sender_name\":\"Tyler\",\"sender_source\":\"mud\","
            "\"body\":\"hi\",\"created_at\":\"x\"},"
            "{\"id\":2,\"channel\":\"gta7\",\"sender_name\":\"Steve\",\"sender_source\":\"einhorn_survival\","
            "\"body\":\"look at this { weird } brace\",\"created_at\":\"y\"}]";
        size_t len = strlen(json);
        size_t pos = 0, os, oe;
        int found = 0;
        long long ids[4];
        while (bigo_bridge_next_json_object(json, len, &pos, &os, &oe)) {
            char buf[256];
            size_t n = oe - os;
            memcpy(buf, json + os, n);
            buf[n] = '\0';
            long long id = 0;
            assert(http_extract_json_int_field(buf, "id", &id));
            ids[found] = id;
            found++;
        }
        assert(found == 2);
        assert(ids[0] == 1 && ids[1] == 2);
        printf("PASS: bigo_bridge_next_json_object walks both objects, brace-in-body doesn't desync it\n");
    }

    /* Test 2: source tagging -- known sources get the real short tag, unknown falls back to the
       raw source string (a real, honest degrade, not a silent drop). */
    {
        char fb[32];
        assert(strcmp(bigo_bridge_source_tag("gfd_server", fb, sizeof(fb)), "GFD") == 0);
        assert(strcmp(bigo_bridge_source_tag("einhorn_survival", fb, sizeof(fb)), "MC") == 0);
        assert(strcmp(bigo_bridge_source_tag("mud", fb, sizeof(fb)), "MUD") == 0);
        assert(strcmp(bigo_bridge_source_tag("battlegrounds", fb, sizeof(fb)), "BG") == 0);
        assert(strcmp(bigo_bridge_source_tag("some_future_source", fb, sizeof(fb)), "some_future_source") == 0);
        printf("PASS: real source tagging, unknown source falls back honestly\n");
    }

    /* Test 3: ring-buffer push then drain, in order, real count. */
    {
        BigoGfdBridge b;
        memset(&b, 0, sizeof(b));
        pthread_mutex_init(&b.mu, NULL);
        bigo_bridge_push_locked(&b, "[GFD] Tyler: hi");
        bigo_bridge_push_locked(&b, "[MC] Steve: hello");
        char out[4][BIGO_BRIDGE_LINE_MAX];
        int n = bigo_gfd_bridge_drain(&b, out, 4);
        assert(n == 2);
        assert(strcmp(out[0], "[GFD] Tyler: hi") == 0);
        assert(strcmp(out[1], "[MC] Steve: hello") == 0);
        int n2 = bigo_gfd_bridge_drain(&b, out, 4);
        assert(n2 == 0);
        printf("PASS: ring-buffer push/drain preserves order, drains empty on the second call\n");
    }

    /* Test 4: overflow -- pushing past BIGO_BRIDGE_QUEUE_CAP drops the OLDEST entry, keeps the
       newest (same real "shift, don't drop the newest" convention bigo_phone.h already uses). */
    {
        BigoGfdBridge b;
        memset(&b, 0, sizeof(b));
        pthread_mutex_init(&b.mu, NULL);
        char line[BIGO_BRIDGE_LINE_MAX];
        for (int i = 0; i < BIGO_BRIDGE_QUEUE_CAP + 5; i++) {
            snprintf(line, sizeof(line), "line-%d", i);
            bigo_bridge_push_locked(&b, line);
        }
        char out[BIGO_BRIDGE_QUEUE_CAP][BIGO_BRIDGE_LINE_MAX];
        int n = bigo_gfd_bridge_drain(&b, out, BIGO_BRIDGE_QUEUE_CAP);
        assert(n == BIGO_BRIDGE_QUEUE_CAP);
        char expect[BIGO_BRIDGE_LINE_MAX];
        snprintf(expect, sizeof(expect), "line-5"); /* the first 5 of 37 pushed were evicted */
        assert(strcmp(out[0], expect) == 0);
        snprintf(expect, sizeof(expect), "line-%d", BIGO_BRIDGE_QUEUE_CAP + 4);
        assert(strcmp(out[BIGO_BRIDGE_QUEUE_CAP - 1], expect) == 0);
        printf("PASS: ring-buffer overflow evicts the oldest entries, keeps the newest\n");
    }

    /* Test 5: a disabled bridge (no secret at init) makes send a real, silent no-op -- checked
       via bigo_gfd_bridge_init itself (no network call happens for auth since enabled short-
       circuits before starting the poller thread at all). */
    {
        BigoGfdBridge b;
        bigo_gfd_bridge_init(&b, "127.0.0.1", 1, "TEST-AGENT", NULL);
        assert(b.enabled == 0);
        bigo_gfd_bridge_send(&b, "BigO-test", "should be a no-op"); /* must not crash or block */
        printf("PASS: bridge with no secret stays disabled, send() is a real no-op\n");
    }

    printf("\nALL PASS\n");
    return 0;
}

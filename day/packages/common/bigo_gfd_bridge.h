/* bigo_gfd_bridge.h -- BIG_O's own real participant on the existing cross-server chat bus
 * (IDUNA's POST/GET /api/v1/chat/messages), joining GoblinFoxDragon's own real bridge to
 * EINHORN_SURVIVAL (GoblinFoxDragon/docs2/CHAT_BRIDGE_TO_EINHORN_SURVIVAL_SPEC.md) as a third
 * real participant rather than a bespoke one-off connection. EMILY/BACKLOG.md SECTION 536
 * follow-up, BIG_O/NORTHSTAR.md §27, founder real-time (2026-09-24): "continue with the big_o
 * gfd integration via the phone app."
 *
 * Real, checked-not-assumed design decision: BIG_O posts under sender_source="bigo_server",
 * channel="big_o" (IDUNA's own internal/http/handlers/chat_messages.go, extended for this),
 * and relays back ANY message NOT from bigo_server itself (gfd_server/einhorn_survival/mud/
 * battlegrounds all included) -- this is a real, shared, multi-game bus, not a GFD-only pipe, so
 * BIG_O players genuinely see the whole mesh, tagged by real origin.
 *
 * Threading: the server's own main loop is a single-threaded 60Hz UDP tick loop -- a blocking
 * HTTP call on that thread would stall every connected player for however long IDUNA takes to
 * answer (http_client.h's own 5s timeout, worst case). Two real background pthreads instead,
 * same "server logic doesn't block on chat" posture GFD's own Go implementation gets for free
 * from goroutines: one persistent poller (auth once, then GET .../chat/messages every 5s,
 * pushing decoded lines into a mutex-protected ring buffer the main loop drains once per tick)
 * and one short-lived detached thread per outbound chat send (POST, fire-and-forget). A chat
 * relay outage (IDUNA down, network blip) never blocks or crashes the game server -- same
 * best-effort convention every other bigo_*.h/idunaclient-adjacent code in this monorepo follows.
 *
 * bigo_gfd_bridge_test.c hermetically covers the pure logic (JSON-array object walking, source
 * tagging, ring-buffer push/drain including overflow) with no network at all -- same boundary
 * papercraft_persist.h's own doc comment already draws between pure/testable and stateful host
 * plumbing. The real HTTP round trip (auth, post, poll) was verified separately, live, against
 * the actual running IDUNA instance (a real POST then GET via curl, not a unit test) -- see
 * BIG_O/NORTHSTAR.md §27.
 */
#ifndef BIGO_GFD_BRIDGE_H
#define BIGO_GFD_BRIDGE_H

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "http_client.h"

#define BIGO_BRIDGE_QUEUE_CAP 32
#define BIGO_BRIDGE_LINE_MAX 96 /* matches PcChatRecvPacket.text[96] exactly (papercraft_protocol.h) */
#define BIGO_BRIDGE_POLL_INTERVAL_SEC 5 /* same cadence GFD's own apps2/server-go poller uses */

/* Sentinel PcChatRecvPacket.sender_slot value meaning "text is already a fully-formatted line
 * from the cross-server bridge, not player<N>'s own say" -- PC_MAX_PLAYERS (16) leaves the whole
 * unsigned char range above it free; 255 is the real, obvious choice. */
#define BIGO_BRIDGE_SENDER_SLOT 255

typedef struct {
    char line[BIGO_BRIDGE_LINE_MAX];
} BigoBridgeInboundMsg;

typedef struct {
    pthread_mutex_t mu;
    BigoBridgeInboundMsg queue[BIGO_BRIDGE_QUEUE_CAP];
    int head;   /* index of the oldest queued line */
    int count;  /* how many valid entries, 0..BIGO_BRIDGE_QUEUE_CAP */
    long long last_seen_id;
    char iduna_host[64];
    int iduna_port;
    char agent_name[64];
    char agent_secret[128];
    char bearer[2048];
    int have_bearer;
    int enabled; /* 0 once agent_secret is empty at init -- the whole bridge becomes a real no-op,
                    not a crash; matches the monorepo's own "no credential, feature quietly off"
                    convention (idunaclient.go's own do(), for one). */
} BigoGfdBridge;

static BigoGfdBridge g_bigo_bridge;

/* bigo_bridge_next_json_object: scans [json, json+len) from *pos for the next top-level {...}
 * object (brace-depth tracked, string literals skipped so a literal '{'/'}' typed into a real
 * chat body can't desync the scan), writing [*obj_start, *obj_end) and advancing *pos past it.
 * Returns 1 if one was found, 0 once the array is exhausted. http_client.h's own extractors only
 * ever find the FIRST occurrence of a field in a whole buffer -- this is the real, minimal piece
 * needed on top of it to walk a JSON ARRAY of chat-message objects one at a time. */
static int bigo_bridge_next_json_object(const char *json, size_t len, size_t *pos,
                                          size_t *obj_start, size_t *obj_end) {
    size_t i = *pos;
    while (i < len && json[i] != '{') i++;
    if (i >= len) return 0;
    *obj_start = i;
    int depth = 0;
    int in_str = 0;
    for (; i < len; i++) {
        char c = json[i];
        if (in_str) {
            if (c == '\\') { i++; continue; }
            if (c == '"') in_str = 0;
            continue;
        }
        if (c == '"') { in_str = 1; continue; }
        if (c == '{') depth++;
        else if (c == '}') {
            depth--;
            if (depth == 0) { *obj_end = i + 1; *pos = i + 1; return 1; }
        }
    }
    return 0;
}

/* bigo_bridge_source_tag -- short, fixed-width bracketed origin tag for a real sender_source
 * value, so a BIG_O player can tell at a glance whether a relayed line came from GFD's own MUD,
 * EINHORN_SURVIVAL/Minecraft, the older mud<->battlegrounds bridge, or elsewhere. Falls back to
 * the raw source string itself for anything not one of the four known values (a real, honest
 * degrade if IDUNA's own validChatSources ever grows a fifth one this file hasn't been told
 * about yet, not a silent drop). */
static const char *bigo_bridge_source_tag(const char *source, char *fallback_buf, size_t fallback_len) {
    if (strcmp(source, "gfd_server") == 0) return "GFD";
    if (strcmp(source, "einhorn_survival") == 0) return "MC";
    if (strcmp(source, "mud") == 0) return "MUD";
    if (strcmp(source, "battlegrounds") == 0) return "BG";
    snprintf(fallback_buf, fallback_len, "%s", source);
    return fallback_buf;
}

/* bigo_bridge_push -- append one formatted line to the ring buffer, dropping the oldest entry if
 * full (same "shift, don't drop the newest" convention bigo_phone.h's own term-scrollback/
 * notification-overflow handling already uses -- a stale bridge line is less useful than a fresh
 * one once the buffer's real, small capacity is exceeded). Mutex must already be held by the
 * caller. */
static void bigo_bridge_push_locked(BigoGfdBridge *b, const char *line) {
    int idx;
    if (b->count < BIGO_BRIDGE_QUEUE_CAP) {
        idx = (b->head + b->count) % BIGO_BRIDGE_QUEUE_CAP;
        b->count++;
    } else {
        idx = b->head;
        b->head = (b->head + 1) % BIGO_BRIDGE_QUEUE_CAP;
    }
    snprintf(b->queue[idx].line, BIGO_BRIDGE_LINE_MAX, "%s", line);
}

/* bigo_gfd_bridge_drain -- pop up to max_lines queued inbound lines into out (each a real,
 * already-NUL-terminated <BIGO_BRIDGE_LINE_MAX line, ready to drop straight into a
 * PcChatRecvPacket.text). Called once per server tick from the main loop -- the only place this
 * header's state is read by anything other than its own background threads. Returns how many
 * lines were actually written. */
static int bigo_gfd_bridge_drain(BigoGfdBridge *b, char out[][BIGO_BRIDGE_LINE_MAX], int max_lines) {
    pthread_mutex_lock(&b->mu);
    int n = b->count < max_lines ? b->count : max_lines;
    for (int i = 0; i < n; i++) {
        int idx = (b->head + i) % BIGO_BRIDGE_QUEUE_CAP;
        memcpy(out[i], b->queue[idx].line, BIGO_BRIDGE_LINE_MAX);
    }
    b->head = (b->head + n) % BIGO_BRIDGE_QUEUE_CAP;
    b->count -= n;
    pthread_mutex_unlock(&b->mu);
    return n;
}

/* bigo_bridge_authenticate -- one real, blocking POST /api/v1/auth/agent, called only from the
 * poller thread (never the main tick loop). On success stores the bearer token; on failure
 * leaves have_bearer at whatever it was (0 on the very first attempt) so the poller's own loop
 * just retries next cycle -- an IDUNA outage at boot is a real, honest "bridge not up yet", not a
 * crash. */
static void bigo_bridge_authenticate(BigoGfdBridge *b) {
    char body[256];
    snprintf(body, sizeof(body), "{\"agent_name\":\"%s\",\"agent_secret\":\"%s\"}",
              b->agent_name, b->agent_secret);
    static char resp[8192];
    int status = 0;
    if (http_post_json(b->iduna_host, b->iduna_port, "/api/v1/auth/agent", NULL, body,
                        resp, sizeof(resp), &status) != 0 || status != 200) {
        fprintf(stderr, "[gfd-bridge] auth failed (status %d)\n", status);
        return;
    }
    char token[2048];
    if (!http_extract_json_string_field(resp, "access_token", token, sizeof(token))) {
        fprintf(stderr, "[gfd-bridge] auth response had no access_token\n");
        return;
    }
    pthread_mutex_lock(&b->mu);
    snprintf(b->bearer, sizeof(b->bearer), "%s", token);
    b->have_bearer = 1;
    pthread_mutex_unlock(&b->mu);
    fprintf(stderr, "[gfd-bridge] authenticated as %s\n", b->agent_name);
}

/* bigo_bridge_poll_once -- one real GET .../chat/messages?since_id=...&limit=50, decodes each
 * message object, skips bigo_server's own posts (a caller hearing its own bridged message back
 * would be a real, confusing echo -- IDUNA's endpoint has no exclude_source filter, same real gap
 * GFD's own poller already works around client-side), formats "[TAG] name: body", and queues it.
 * Advances last_seen_id to the highest id actually seen, even for a bigo_server message that gets
 * filtered out (otherwise the very next poll would refetch it forever). */
static void bigo_bridge_poll_once(BigoGfdBridge *b) {
    char bearer_copy[2048];
    pthread_mutex_lock(&b->mu);
    int have = b->have_bearer;
    snprintf(bearer_copy, sizeof(bearer_copy), "%s", b->bearer);
    long long since = b->last_seen_id;
    pthread_mutex_unlock(&b->mu);
    if (!have) return;

    char path[128];
    snprintf(path, sizeof(path), "/api/v1/chat/messages?since_id=%lld&limit=50", since);
    static char resp[131072];
    int status = 0;
    if (http_get_json(b->iduna_host, b->iduna_port, path, bearer_copy, resp, sizeof(resp), &status) != 0) {
        fprintf(stderr, "[gfd-bridge] poll GET failed\n");
        return;
    }
    if (status != 200) {
        fprintf(stderr, "[gfd-bridge] poll GET status %d\n", status);
        return;
    }

    size_t len = strlen(resp);
    size_t pos = 0, obj_start, obj_end;
    long long max_id = since;
    while (bigo_bridge_next_json_object(resp, len, &pos, &obj_start, &obj_end)) {
        size_t objlen = obj_end - obj_start;
        static char objbuf[640];
        if (objlen >= sizeof(objbuf)) objlen = sizeof(objbuf) - 1;
        memcpy(objbuf, resp + obj_start, objlen);
        objbuf[objlen] = '\0';

        long long id = 0;
        char source[32] = {0}, name[64] = {0}, text[512] = {0};
        if (!http_extract_json_int_field(objbuf, "id", &id)) continue;
        if (id > max_id) max_id = id;
        if (!http_extract_json_string_field(objbuf, "sender_source", source, sizeof(source))) continue;
        if (strcmp(source, "bigo_server") == 0) continue; /* real, own message echoed back -- skip */
        http_extract_json_string_field(objbuf, "sender_name", name, sizeof(name));
        http_extract_json_string_field(objbuf, "body", text, sizeof(text));

        char fallback[32];
        const char *tag = bigo_bridge_source_tag(source, fallback, sizeof(fallback));
        char line[BIGO_BRIDGE_LINE_MAX];
        snprintf(line, sizeof(line), "[%s] %s: %s", tag, name, text);

        pthread_mutex_lock(&b->mu);
        bigo_bridge_push_locked(b, line);
        pthread_mutex_unlock(&b->mu);
    }
    pthread_mutex_lock(&b->mu);
    if (max_id > b->last_seen_id) b->last_seen_id = max_id;
    pthread_mutex_unlock(&b->mu);
}

static void *bigo_bridge_poll_thread(void *arg) {
    BigoGfdBridge *b = (BigoGfdBridge *)arg;
    for (;;) {
        pthread_mutex_lock(&b->mu);
        int have = b->have_bearer;
        pthread_mutex_unlock(&b->mu);
        if (!have) bigo_bridge_authenticate(b);
        else bigo_bridge_poll_once(b);
        sleep(BIGO_BRIDGE_POLL_INTERVAL_SEC);
    }
    return NULL;
}

typedef struct {
    BigoGfdBridge *b;
    char sender_name[64];
    char body[256];
} BigoBridgeSendJob;

static void *bigo_bridge_send_thread(void *arg) {
    BigoBridgeSendJob *job = (BigoBridgeSendJob *)arg;
    BigoGfdBridge *b = job->b;
    char bearer_copy[2048];
    pthread_mutex_lock(&b->mu);
    int have = b->have_bearer;
    snprintf(bearer_copy, sizeof(bearer_copy), "%s", b->bearer);
    pthread_mutex_unlock(&b->mu);
    if (have) {
        char json_body[512];
        char esc_body[256];
        size_t j = 0;
        for (size_t i = 0; job->body[i] && j + 2 < sizeof(esc_body); i++) {
            char c = job->body[i];
            if (c == '"' || c == '\\') esc_body[j++] = '\\';
            esc_body[j++] = c;
        }
        esc_body[j] = '\0';
        snprintf(json_body, sizeof(json_body),
                  "{\"channel\":\"big_o\",\"sender_name\":\"%s\",\"sender_source\":\"bigo_server\",\"body\":\"%s\"}",
                  job->sender_name, esc_body);
        char resp[512];
        int status = 0;
        if (http_post_json(b->iduna_host, b->iduna_port, "/api/v1/chat/messages", bearer_copy,
                            json_body, resp, sizeof(resp), &status) != 0 || status != 201) {
            fprintf(stderr, "[gfd-bridge] send failed (status %d)\n", status);
        }
    }
    free(job);
    return NULL;
}

/* bigo_gfd_bridge_send -- fire-and-forget: spawns a detached thread to POST this one message,
 * never blocking the caller (the main tick loop). A no-op if the bridge isn't enabled/authed yet
 * -- never queues for later, matching the "best-effort, drop rather than buffer unboundedly"
 * posture the rest of this file already uses. */
static void bigo_gfd_bridge_send(BigoGfdBridge *b, const char *sender_name, const char *body) {
    if (!b->enabled) return;
    BigoBridgeSendJob *job = (BigoBridgeSendJob *)malloc(sizeof(BigoBridgeSendJob));
    if (!job) return;
    job->b = b;
    snprintf(job->sender_name, sizeof(job->sender_name), "%s", sender_name);
    snprintf(job->body, sizeof(job->body), "%s", body);
    pthread_t tid;
    if (pthread_create(&tid, NULL, bigo_bridge_send_thread, job) != 0) {
        free(job);
        return;
    }
    pthread_detach(tid);
}

/* bigo_gfd_bridge_init -- configures the bridge and starts its one persistent poller thread.
 * agent_secret == NULL or "" leaves the bridge disabled (enabled=0): every send is a silent
 * no-op, drain always returns 0, no thread is started at all -- a missing credential is a real,
 * honest "feature off", same as every other IDUNA-agent-backed feature in this monorepo when its
 * secret env var is unset. */
static void bigo_gfd_bridge_init(BigoGfdBridge *b, const char *iduna_host, int iduna_port,
                                   const char *agent_name, const char *agent_secret) {
    memset(b, 0, sizeof(*b));
    pthread_mutex_init(&b->mu, NULL);
    snprintf(b->iduna_host, sizeof(b->iduna_host), "%s", iduna_host);
    b->iduna_port = iduna_port;
    snprintf(b->agent_name, sizeof(b->agent_name), "%s", agent_name);
    if (!agent_secret || !agent_secret[0]) {
        fprintf(stderr, "[gfd-bridge] IDUNA_AGENT_SECRET not set -- bridge disabled\n");
        b->enabled = 0;
        return;
    }
    snprintf(b->agent_secret, sizeof(b->agent_secret), "%s", agent_secret);
    b->enabled = 1;
    pthread_t tid;
    if (pthread_create(&tid, NULL, bigo_bridge_poll_thread, b) != 0) {
        fprintf(stderr, "[gfd-bridge] failed to start poller thread -- bridge disabled\n");
        b->enabled = 0;
        return;
    }
    pthread_detach(tid);
    fprintf(stderr, "[gfd-bridge] started, polling %s:%d every %ds\n", iduna_host, iduna_port, BIGO_BRIDGE_POLL_INTERVAL_SEC);
}

#endif /* BIGO_GFD_BRIDGE_H */

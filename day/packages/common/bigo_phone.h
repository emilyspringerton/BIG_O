/* bigo_phone.h -- the in-game smartphone: the ONE way every BIG_O menu is reached.
 *
 * Source spec: TYLER/engine/tyler_phone_mechanics.md (Messages, Contacts, Map, Camera, Notes, notification
 * anti-spam "max 2 per 30s, excess batched into a summary"). PAPERCRAFT only shipped the notification banner; this
 * is the real browsable phone. BIG_O adds the apps its loops need: Lab, Cargo, Skills, Loadout, Wardrobe, Status.
 *
 * Pure C99, no GL/SDL/network: input in (abstract actions), state + "effects" out (things the host must send to
 * the server). The client renders the state and turns effects into packets. Deterministic and unit-tested.
 */
#ifndef BIGO_PHONE_H
#define BIGO_PHONE_H

#include <string.h>

typedef enum {
    BP_APP_MESSAGES = 0, BP_APP_CONTACTS, BP_APP_MAP, BP_APP_CAMERA, BP_APP_NOTES,   /* TYLER spec apps */
    BP_APP_LAB, BP_APP_CARGO, BP_APP_SKILLS, BP_APP_LOADOUT, BP_APP_WARDROBE, BP_APP_STATUS, /* BIG_O apps */
    BP_APP_GFD, /* EMILY/BACKLOG.md SECTION 536 follow-up, BIG_O/NORTHSTAR.md §24, founder real-time
                   (2026-09-24): "the GFD subsystem affordances should be via the GFD app on the
                   BIG_O phone (mini terminal interface)" -- a real, free-text terminal (the one
                   BP_APP that breaks the pure D-pad-menu model every other app here uses), talking
                   to GoblinFoxDragon-reverse-ported subsystems one command at a time. v1 ships one
                   real command: say. */
    BP_APP_COUNT
} BpApp;

typedef enum { BP_UP, BP_DOWN, BP_LEFT, BP_RIGHT, BP_SELECT, BP_BACK } BpAction;

typedef enum {
    BP_FX_NONE = 0,
    BP_FX_ALLOCATE_TALENT,  /* arg = ability index 0..4 -> PC_PACKET_ALLOCATE_TALENT */
    BP_FX_WEAPON_SWITCH,    /* arg = weapon slot -> PC_PACKET_WEAPON_SWITCH */
    BP_FX_TAKE_PHOTO,       /* host may grab a screenshot; counter already advanced */
    BP_FX_COSTUME_SET,      /* arg = new costume index (COS_*) -> PC_PACKET_COSTUME_SET, first
                                time costume becomes server-authoritative (EMILY/BACKLOG.md
                                SECTION 536 follow-up, BIG_O/NORTHSTAR.md §18 Phase A) */
    BP_FX_ITEM_USE,         /* arg = inventory slot index -> PC_PACKET_ITEM_USE, first time Cargo
                                does anything at all (EMILY/BACKLOG.md SECTION 536 follow-up,
                                BIG_O/NORTHSTAR.md §20). Fired unconditionally on SELECT -- this
                                struct carries no local inventory copy to validate against (only
                                the host's separate g_inventory does), so the server is the real,
                                only validator, same "server decides" split every other real
                                system in this file already follows. */
    BP_FX_CHAT_SEND         /* arg = length of the pending say text. The text itself lives in
                                p->term_input (NUL-terminated) -- an int arg can't carry free text,
                                so the host must call bigo_phone_term_take() to read AND clear it
                                before doing anything else with the phone (same "host reads other
                                state, arg is just a signal" convention BP_FX_TAKE_PHOTO already
                                uses). Build a real PcChatSayPacket from what comes back, send it,
                                then echo a local line via bigo_phone_term_line() so the sender
                                sees their own message immediately rather than waiting on the
                                server's own PC_PACKET_CHAT_RECV round trip. First time the GFD app
                                does anything at all (EMILY/BACKLOG.md SECTION 536 follow-up,
                                BIG_O/NORTHSTAR.md §24, reverse-ported from GoblinFoxDragon's own
                                real server/chat/chat.go "say" channel). */
} BpEffectKind;

typedef struct { BpEffectKind kind; int arg; } BpEffect;

#define BP_MAX_MESSAGES 16
#define BP_MAX_QUEUED 8
#define BP_NOTE_LINES 6
#define BP_CONTACTS 5
#define BP_CLONES 8
#define BP_INV_SLOTS 8
#define BP_ZONES 5
#define BP_COSTUMES 4
#define BP_BANNER_MS 5000
#define BP_SPAM_WINDOW_MS 30000
#define BP_SPAM_MAX 2
#define BP_TERM_INPUT_MAX 95 /* bigo_chat.h's own BIGO_CHAT_MAX_TEXT is 96; capped to 95 here to
                                 always leave room for this header's own local NUL terminator.
                                 This header stays independent of bigo_chat.h on purpose (same "no
                                 upward host-constant dependency" convention bigo_party.h already
                                 establishes) -- the relationship is checked directly, not assumed,
                                 by bigo_chat_test.c's own _Static_assert. */
#define BP_TERM_LINES 6
#define BP_TERM_LINE_LEN 80

static const char *const BP_PHASE_NAMES[4] = { "DAWN", "DAY", "DUSK", "NIGHT" };
static const char *const BP_WEATHER_NAMES[4] = { "CLEAR", "OVERCAST", "RAIN", "STORM" };
#define BP_MSG_THORNE_BRIEF 6   /* client message table id: Dr. Thorne's A1M1 reprimand; unlocks him in Contacts */

static const char *const BP_APP_NAMES[BP_APP_COUNT] = {
    "MESSAGES", "CONTACTS", "MAP", "CAMERA", "NOTES", "LAB", "CARGO", "SKILLS", "LOADOUT", "WARDROBE", "STATUS", "GFD"
};

/* Contacts: trust ladder observer -> witness -> bound -> documented (spec). Preset replies advance it. */
static const char *const BP_TRUST_NAMES[4] = { "observer", "witness", "bound", "documented" };
static const char *const BP_CONTACT_HANDLES[BP_CONTACTS] = { "CAMERA OP", "DR THORNE", "THE PRODUCER", "EASTWIND OWL", "EMILY OS" };
static const char *const BP_REPLIES[3] = { "who is this?", "i saw nothing.", "send me the file." };

/* Map zones: a faction document, deliberately imprecise (spec), not a GPS. */
static const char *const BP_ZONE_NAMES[BP_ZONES] = { "WASTELAND", "NEXTOWN", "OFFICE BLOCK", "THE PARK", "BASEMENT LAB" };

static const char *const BP_COSTUME_NAMES[BP_COSTUMES] = { "CIVILIAN SUIT", "LAB SMOCK", "JANITOR OVERALLS", "FIELD GEAR" };

/* Lab terminal: isolate -> align -> splice, base vector x trait, consuming one sample per splice. */
static const char *const BP_BASES[3] = { "SCAVENGER", "HOUND", "BRUTE" };
static const char *const BP_TRAITS[3] = { "SPEED", "ARMOR", "SCENT" };

typedef struct {
    int open;
    int app;                 /* -1 = home grid, else BpApp */
    int home_cursor;
    int cursor;              /* per-app row cursor (reset on entering an app) */
    int cursor2;             /* second axis (lab base / contact reply) */
    int lab_trait;           /* lab: selected trait */

    int messages[BP_MAX_MESSAGES]; int message_count; int unread;
    int trust[BP_CONTACTS]; int replied[BP_CONTACTS]; int contacts_met;
    int zone_current, zone_pinned, zone_alert;   /* zone_alert = zone highlighted by a server event, -1 none */
    int photos;
    int detail;              /* messages: 1 = showing the selected message in full */
    /* world feed (host-fed: clock/weather/zombies). wf_valid 0 = no world source, screens say so. */
    int wf_valid, wf_minute, wf_day, wf_phase, wf_weather, wf_zombies[BP_ZONES], wf_sight;
    char notes[BP_NOTE_LINES][48];
    int samples[3];          /* harvested sample counts per type (fed by the host; 0 until harvesting exists) */
    int clones[BP_CLONES]; int clone_count; int clone_traits[BP_CLONES];
    int costume;             /* worn costume index */
    int weapons_owned;       /* bitmask, mirrored from server */
    int current_weapon;

    /* GFD terminal (BP_APP_GFD) -- free-text input + a real scrollback, the one app that breaks
       the pure D-pad-menu model every other app here uses. term_input is always NUL-terminated. */
    char term_input[BP_TERM_INPUT_MAX + 1]; int term_input_len;
    char term_lines[BP_TERM_LINES][BP_TERM_LINE_LEN]; int term_line_count;

    /* notifications */
    int queue[BP_MAX_QUEUED]; int queue_len;
    unsigned int shown_at[BP_SPAM_MAX]; int shown_n;    /* recent banner times inside the spam window */
    int banner_id; unsigned int banner_since; int banner_batched;
} BigoPhone;

static inline void bigo_phone_init(BigoPhone *p) {
    memset(p, 0, sizeof(*p));
    p->app = -1; p->zone_alert = -1; p->zone_pinned = -1; p->zone_current = 0;
    p->contacts_met = 1;
    p->weapons_owned = 1;
    strcpy(p->notes[0], "The archive is not where you");   /* spec: pre-populated Eastwind Owls briefing */
    strcpy(p->notes[1], "think it is. Start with what the");
    strcpy(p->notes[2], "building smells like.");
    strcpy(p->term_lines[0], "GFD TERMINAL -- type, ENTER to say");
    p->term_line_count = 1;
}

static inline int bp_wrap(int v, int n) { return n <= 0 ? 0 : ((v % n) + n) % n; }

static inline void bp_enter_app(BigoPhone *p, int app) {
    p->detail = 0;
    p->app = app; p->cursor = 0; p->cursor2 = 0; p->lab_trait = 0;
    if (app == BP_APP_MESSAGES) p->unread = 0;
}

/* Rows the current app's cursor ranges over (for wrap). */
static inline int bp_rows(const BigoPhone *p) {
    switch (p->app) {
    case BP_APP_MESSAGES: return p->message_count > 0 ? p->message_count : 1;
    case BP_APP_CONTACTS: return p->contacts_met;
    case BP_APP_MAP: return BP_ZONES;
    case BP_APP_NOTES: return BP_NOTE_LINES;
    case BP_APP_LAB: return 4;       /* base, trait, SPLICE, clone list */
    case BP_APP_CARGO: return BP_INV_SLOTS;
    case BP_APP_SKILLS: return 5;
    case BP_APP_LOADOUT: return 6;
    case BP_APP_WARDROBE: return BP_COSTUMES;
    case BP_APP_GFD: return p->term_line_count > 0 ? p->term_line_count : 1;
    default: return 1;
    }
}

/* bigo_phone_term_line -- append a scrollback line to the GFD terminal (used for both the local
 * echo of a sent "say" and an incoming PC_PACKET_CHAT_RECV). Truncates to BP_TERM_LINE_LEN-1,
 * shifts out the oldest line once full -- same real "shift, don't drop the newest" convention
 * bigo_phone_notify's own messages[] overflow handling already establishes. */
static inline void bigo_phone_term_line(BigoPhone *p, const char *text) {
    char buf[BP_TERM_LINE_LEN];
    size_t n = strlen(text);
    if (n >= BP_TERM_LINE_LEN) n = BP_TERM_LINE_LEN - 1;
    memcpy(buf, text, n);
    buf[n] = '\0';
    if (p->term_line_count < BP_TERM_LINES) {
        memcpy(p->term_lines[p->term_line_count++], buf, n + 1);
    } else {
        for (int i = 0; i < BP_TERM_LINES - 1; i++) memcpy(p->term_lines[i], p->term_lines[i + 1], BP_TERM_LINE_LEN);
        memcpy(p->term_lines[BP_TERM_LINES - 1], buf, n + 1);
    }
}

/* bigo_phone_term_char -- append one printable ASCII character to the pending input, dropped
 * silently once full or if non-printable (matches SDL_TEXTINPUT's own real UTF-8 text callback --
 * multi-byte sequences are rejected byte-by-byte here rather than decoded, a real, honest v1
 * limit: no non-ASCII chat text yet). */
static inline void bigo_phone_term_char(BigoPhone *p, char c) {
    if (c < 32 || c > 126) return;
    if (p->term_input_len >= BP_TERM_INPUT_MAX) return;
    p->term_input[p->term_input_len++] = c;
    p->term_input[p->term_input_len] = '\0';
}

static inline void bigo_phone_term_backspace(BigoPhone *p) {
    if (p->term_input_len > 0) p->term_input[--p->term_input_len] = '\0';
}

/* bigo_phone_term_take -- copies out the pending input (up to out_cap bytes, NOT NUL-terminated by
 * this call -- caller owns that) and clears it. The host calls this exactly once, immediately on
 * seeing a BP_FX_CHAT_SEND effect, before any further phone input can overwrite term_input.
 * Returns the real byte count copied. */
static inline int bigo_phone_term_take(BigoPhone *p, char *out, int out_cap) {
    int n = p->term_input_len;
    if (n > out_cap) n = out_cap;
    memcpy(out, p->term_input, n);
    p->term_input_len = 0;
    p->term_input[0] = '\0';
    return n;
}

/* Notification with the spec's anti-spam rule: <= 2 banners per 30s, extras queued and later shown as a summary. */
static inline void bp_prune(BigoPhone *p, unsigned int now) {
    int k = 0;
    for (int i = 0; i < p->shown_n; i++)
        if (now - p->shown_at[i] < BP_SPAM_WINDOW_MS) p->shown_at[k++] = p->shown_at[i];
    p->shown_n = k;
}

static inline void bigo_phone_notify(BigoPhone *p, int msg_id, unsigned int now) {
    if (msg_id <= 0) return;
    if (msg_id == BP_MSG_THORNE_BRIEF && p->contacts_met < 2) p->contacts_met = 2;
    if (p->message_count < BP_MAX_MESSAGES) p->messages[p->message_count++] = msg_id;
    else { memmove(p->messages, p->messages + 1, sizeof(int) * (BP_MAX_MESSAGES - 1)); p->messages[BP_MAX_MESSAGES - 1] = msg_id; }
    if (!(p->open && p->app == BP_APP_MESSAGES)) p->unread++;
    bp_prune(p, now);
    if (p->shown_n < BP_SPAM_MAX) {
        p->shown_at[p->shown_n++] = now;
        p->banner_id = msg_id; p->banner_since = now; p->banner_batched = 0;
    } else if (p->queue_len < BP_MAX_QUEUED) {
        p->queue[p->queue_len++] = msg_id;
    }
}

/* Per-frame: expire the banner; release a batched summary once the window has room. */
static inline void bigo_phone_tick(BigoPhone *p, unsigned int now) {
    if (p->banner_id && now - p->banner_since > BP_BANNER_MS) p->banner_id = 0;
    bp_prune(p, now);
    if (!p->banner_id && p->queue_len > 0 && p->shown_n < BP_SPAM_MAX) {
        p->shown_at[p->shown_n++] = now;
        p->banner_id = p->queue[p->queue_len - 1];
        p->banner_batched = p->queue_len;
        p->banner_since = now;
        p->queue_len = 0;
    }
}

static inline void bigo_phone_set_world(BigoPhone *p, int minute_of_day, int day, int phase, int weather, const int *zombies, int sight_pct) {
    p->wf_valid = 1; p->wf_minute = minute_of_day; p->wf_day = day; p->wf_phase = phase; p->wf_weather = weather; p->wf_sight = sight_pct;
    for (int i = 0; i < BP_ZONES; i++) p->wf_zombies[i] = zombies ? zombies[i] : 0;
}

static inline void bigo_phone_toggle(BigoPhone *p) {
    p->open = !p->open;
    if (p->open) p->app = -1;
}
static inline void bigo_phone_open_app(BigoPhone *p, int app) { p->open = 1; bp_enter_app(p, app); }

/* Feed one abstract input. Returns the effect the host must carry out (BP_FX_NONE mostly). */
static inline BpEffect bigo_phone_input(BigoPhone *p, BpAction a, int unspent_points) {
    BpEffect fx = { BP_FX_NONE, 0 };
    if (!p->open) return fx;

    if (p->app < 0) {   /* home grid: 3 columns */
        const int cols = 3;
        if (a == BP_LEFT) p->home_cursor = bp_wrap(p->home_cursor - 1, BP_APP_COUNT);
        else if (a == BP_RIGHT) p->home_cursor = bp_wrap(p->home_cursor + 1, BP_APP_COUNT);
        else if (a == BP_UP) p->home_cursor = p->home_cursor - cols >= 0 ? p->home_cursor - cols : p->home_cursor;
        else if (a == BP_DOWN) p->home_cursor = p->home_cursor + cols < BP_APP_COUNT ? p->home_cursor + cols : p->home_cursor;
        else if (a == BP_SELECT) bp_enter_app(p, p->home_cursor);
        else if (a == BP_BACK) p->open = 0;
        return fx;
    }

    if (a == BP_BACK) { if (p->app == BP_APP_MESSAGES && p->detail) p->detail = 0; else p->app = -1; return fx; }
    int n = bp_rows(p);
    if (a == BP_UP) { p->cursor = bp_wrap(p->cursor - 1, n); return fx; }
    if (a == BP_DOWN) { p->cursor = bp_wrap(p->cursor + 1, n); return fx; }

    switch (p->app) {
    case BP_APP_MESSAGES:
        if (a == BP_SELECT && p->message_count > 0) p->detail = !p->detail;
        break;
    case BP_APP_CONTACTS:
        if (a == BP_LEFT || a == BP_RIGHT) p->cursor2 = bp_wrap(p->cursor2 + (a == BP_RIGHT ? 1 : -1), 3);
        else if (a == BP_SELECT && p->cursor < p->contacts_met) {   /* preset reply: the phone's primary agency mechanic */
            p->replied[p->cursor] = p->cursor2 + 1;
            if (p->trust[p->cursor] < 3) p->trust[p->cursor]++;
        }
        break;
    case BP_APP_MAP:
        if (a == BP_SELECT) p->zone_pinned = (p->zone_pinned == p->cursor) ? -1 : p->cursor;
        break;
    case BP_APP_CAMERA:
        if (a == BP_SELECT) { p->photos++; fx.kind = BP_FX_TAKE_PHOTO; fx.arg = p->photos; }
        break;
    case BP_APP_LAB:
        if (p->cursor == 0 && (a == BP_LEFT || a == BP_RIGHT)) p->cursor2 = bp_wrap(p->cursor2 + (a == BP_RIGHT ? 1 : -1), 3);
        else if (p->cursor == 1 && (a == BP_LEFT || a == BP_RIGHT)) p->lab_trait = bp_wrap(p->lab_trait + (a == BP_RIGHT ? 1 : -1), 3);
        else if (p->cursor == 2 && a == BP_SELECT) {
            int base = p->cursor2, trait = p->lab_trait;
            if (p->samples[base] > 0 && p->clone_count < BP_CLONES) {
                p->samples[base]--;
                p->clones[p->clone_count] = base;
                p->clone_traits[p->clone_count] = trait;
                p->clone_count++;
            }
        }
        break;
    case BP_APP_SKILLS:
        if (a == BP_SELECT && unspent_points > 0) { fx.kind = BP_FX_ALLOCATE_TALENT; fx.arg = p->cursor; }
        break;
    case BP_APP_LOADOUT:
        if (a == BP_SELECT && (p->cursor == 0 || (p->weapons_owned & (1 << p->cursor)))) { fx.kind = BP_FX_WEAPON_SWITCH; fx.arg = p->cursor; }
        break;
    case BP_APP_WARDROBE:
        if (a == BP_SELECT && p->cursor != p->costume) {
            p->costume = p->cursor;
            fx.kind = BP_FX_COSTUME_SET;
            fx.arg = p->costume;
        }
        break;
    case BP_APP_CARGO:
        if (a == BP_SELECT) { fx.kind = BP_FX_ITEM_USE; fx.arg = p->cursor; }
        break;
    case BP_APP_GFD:
        if (a == BP_SELECT && p->term_input_len > 0) { fx.kind = BP_FX_CHAT_SEND; fx.arg = p->term_input_len; }
        break;
    default: break;
    }
    return fx;
}

#endif

#include <stdio.h>
#include "bigo_phone.h"
#include "papercraft_protocol.h" /* BIGO_LAB_SAMPLE_MAX_WIRE -- see the BP_LAB_SAMPLES parity check below */
static int fails = 0;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)
static void go(BigoPhone *p, int app) { p->open = 1; p->app = -1; p->home_cursor = app; bigo_phone_input(p, BP_SELECT, 0); }
int main(void) {
    BigoPhone p; bigo_phone_init(&p);
    CHECK(!p.open); CHECK(bigo_phone_input(&p, BP_SELECT, 9).kind == BP_FX_NONE);   /* closed phone ignores input */
    bigo_phone_toggle(&p); CHECK(p.open && p.app == -1);
    /* home grid navigation + every app reachable */
    for (int a = 0; a < BP_APP_COUNT; a++) { go(&p, a); CHECK(p.app == a); bigo_phone_input(&p, BP_BACK, 0); CHECK(p.app == -1); }
    bigo_phone_input(&p, BP_BACK, 0); CHECK(!p.open);
    /* anti-spam: 3rd message in 30s is queued, then released as a batched summary */
    bigo_phone_init(&p);
    bigo_phone_notify(&p, 1, 1000); bigo_phone_notify(&p, 1, 2000); bigo_phone_notify(&p, 1, 3000); bigo_phone_notify(&p, 1, 4000);
    CHECK(p.banner_id == 1 && p.queue_len == 2 && p.message_count == 4 && p.unread == 4);
    bigo_phone_tick(&p, 7500); CHECK(p.banner_id == 0);            /* banner expired, window still full */
    bigo_phone_tick(&p, 32500); CHECK(p.banner_id == 1 && p.banner_batched == 2 && p.queue_len == 0);
    /* messages app clears unread */
    go(&p, BP_APP_MESSAGES); CHECK(p.unread == 0);
    /* contacts: preset reply advances trust, capped at documented */
    bigo_phone_init(&p); go(&p, BP_APP_CONTACTS);
    for (int i = 0; i < 6; i++) bigo_phone_input(&p, BP_SELECT, 0);
    CHECK(p.trust[0] == 3 && p.replied[0] == 1);
    bigo_phone_input(&p, BP_RIGHT, 0); bigo_phone_input(&p, BP_SELECT, 0); CHECK(p.replied[0] == 2);
    /* map pin toggles; camera counts photos and emits fx */
    go(&p, BP_APP_MAP); bigo_phone_input(&p, BP_DOWN, 0); bigo_phone_input(&p, BP_SELECT, 0); CHECK(p.zone_pinned == 1);
    bigo_phone_input(&p, BP_SELECT, 0); CHECK(p.zone_pinned == -1);
    go(&p, BP_APP_CAMERA); BpEffect fx = bigo_phone_input(&p, BP_SELECT, 0); CHECK(fx.kind == BP_FX_TAKE_PHOTO && p.photos == 1);
    /* skills: needs an unspent point, effect carries the ability row */
    go(&p, BP_APP_SKILLS); bigo_phone_input(&p, BP_DOWN, 1); bigo_phone_input(&p, BP_DOWN, 1);
    fx = bigo_phone_input(&p, BP_SELECT, 0); CHECK(fx.kind == BP_FX_NONE);
    fx = bigo_phone_input(&p, BP_SELECT, 1); CHECK(fx.kind == BP_FX_ALLOCATE_TALENT && fx.arg == 2);
    /* loadout: only owned weapons (knife always) */
    go(&p, BP_APP_LOADOUT); fx = bigo_phone_input(&p, BP_SELECT, 0); CHECK(fx.kind == BP_FX_WEAPON_SWITCH && fx.arg == 0);
    bigo_phone_input(&p, BP_DOWN, 0); fx = bigo_phone_input(&p, BP_SELECT, 0); CHECK(fx.kind == BP_FX_NONE);
    p.weapons_owned |= 2; fx = bigo_phone_input(&p, BP_SELECT, 0); CHECK(fx.kind == BP_FX_WEAPON_SWITCH && fx.arg == 1);
    /* wardrobe -- selecting a different costume raises BP_FX_COSTUME_SET (SECTION 536 follow-up,
       NORTHSTAR.md §18 Phase A: costume becomes server-authoritative for the first time) */
    go(&p, BP_APP_WARDROBE); bigo_phone_input(&p, BP_DOWN, 0);
    fx = bigo_phone_input(&p, BP_SELECT, 0); CHECK(p.costume == 1 && fx.kind == BP_FX_COSTUME_SET && fx.arg == 1);
    fx = bigo_phone_input(&p, BP_SELECT, 0); CHECK(fx.kind == BP_FX_NONE); /* re-selecting the SAME worn costume is a no-op, not a re-send */
    /* cargo -- SELECT fires BP_FX_ITEM_USE with the current cursor as arg (SECTION 536 follow-up,
       NORTHSTAR.md §20: Cargo's first real behavior of any kind, was a dead app before this). The
       real server is the only real validator (no local inventory copy lives in BigoPhone), so
       this fires unconditionally regardless of cursor position. */
    go(&p, BP_APP_CARGO); bigo_phone_input(&p, BP_DOWN, 0); bigo_phone_input(&p, BP_DOWN, 0);
    fx = bigo_phone_input(&p, BP_SELECT, 0); CHECK(fx.kind == BP_FX_ITEM_USE && fx.arg == 2);
    /* lab: real, host-fed crew sample list (BIG_O/NORTHSTAR.md §30 follow-up). SELECT on a real
       row fires BP_FX_LAB_CENTRIFUGE with the row index; empty lab is a real, honest no-op. */
    go(&p, BP_APP_LAB); CHECK(bp_rows(&p) == 1);   /* no samples yet -- one placeholder row, not zero */
    fx = bigo_phone_input(&p, BP_SELECT, 0); CHECK(fx.kind == BP_FX_NONE);
    p.lab_sample_count = 2; CHECK(bp_rows(&p) == 2);
    fx = bigo_phone_input(&p, BP_SELECT, 0); CHECK(fx.kind == BP_FX_LAB_CENTRIFUGE && fx.arg == 0);
    bigo_phone_input(&p, BP_DOWN, 0); fx = bigo_phone_input(&p, BP_SELECT, 0); CHECK(fx.kind == BP_FX_LAB_CENTRIFUGE && fx.arg == 1);
    CHECK(BP_LAB_SAMPLES == BIGO_LAB_SAMPLE_MAX_WIRE);
    /* world feed + Thorne unlock + message detail */
    bigo_phone_init(&p); CHECK(!p.wf_valid && p.contacts_met == 1);
    int zc[BP_ZONES] = { 12, 4, 0, 2, 0 }; bigo_phone_set_world(&p, 450, 1, 1, 2, zc, 75);
    CHECK(p.wf_valid && p.wf_minute == 450 && p.wf_zombies[0] == 12 && p.wf_weather == 2 && p.wf_sight == 75);
    bigo_phone_notify(&p, BP_MSG_THORNE_BRIEF, 5000); CHECK(p.contacts_met == 2 && p.unread == 1);
    go(&p, BP_APP_MESSAGES); CHECK(!p.detail);
    bigo_phone_input(&p, BP_SELECT, 0); CHECK(p.detail);
    bigo_phone_input(&p, BP_BACK, 0); CHECK(!p.detail && p.app == BP_APP_MESSAGES);   /* first back closes the detail */
    bigo_phone_input(&p, BP_BACK, 0); CHECK(p.app == -1);
    go(&p, BP_APP_CONTACTS); bigo_phone_input(&p, BP_DOWN, 0); bigo_phone_input(&p, BP_SELECT, 0); CHECK(p.trust[1] == 1);   /* Thorne replies work */
    /* ARPANET: same list<->detail pattern as MESSAGES, static content, no live network state */
    go(&p, BP_APP_ARPANET); CHECK(!p.detail && p.cursor == 0);
    bigo_phone_input(&p, BP_SELECT, 0); CHECK(p.detail);
    bigo_phone_input(&p, BP_BACK, 0); CHECK(!p.detail && p.app == BP_APP_ARPANET);   /* first back closes the detail */
    bigo_phone_input(&p, BP_BACK, 0); CHECK(p.app == -1);
    printf(fails ? "PHONE TEST FAILED\n" : "PHONE TEST OK\n");
    return fails != 0;
}

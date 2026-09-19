#include <stdio.h>
#include "bigo_phone.h"
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
    /* wardrobe */
    go(&p, BP_APP_WARDROBE); bigo_phone_input(&p, BP_DOWN, 0); bigo_phone_input(&p, BP_SELECT, 0); CHECK(p.costume == 1);
    /* lab: splice needs a sample; consumes exactly one; base/trait choice recorded */
    go(&p, BP_APP_LAB); bigo_phone_input(&p, BP_DOWN, 0); bigo_phone_input(&p, BP_DOWN, 0);
    bigo_phone_input(&p, BP_SELECT, 0); CHECK(p.clone_count == 0);
    p.samples[0] = 1; bigo_phone_input(&p, BP_SELECT, 0); CHECK(p.clone_count == 1 && p.samples[0] == 0 && p.clones[0] == 0);
    bigo_phone_input(&p, BP_SELECT, 0); CHECK(p.clone_count == 1);
    p.app = BP_APP_LAB; p.cursor = 0; bigo_phone_input(&p, BP_RIGHT, 0); CHECK(p.cursor2 == 1);
    p.cursor = 1; bigo_phone_input(&p, BP_RIGHT, 0); CHECK(p.lab_trait == 1);
    p.samples[1] = 1; p.cursor = 2; bigo_phone_input(&p, BP_SELECT, 0); CHECK(p.clone_count == 2 && p.clones[1] == 1 && p.clone_traits[1] == 1);
    printf(fails ? "PHONE TEST FAILED\n" : "PHONE TEST OK\n");
    return fails != 0;
}

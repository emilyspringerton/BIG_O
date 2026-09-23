/* bigo_npc_visual_test.c -- real, direct coverage for bigo_npc_visual.h's pure NPC-render
 * decisions. Exists specifically because day/apps/client/src/main.c's own live NPC render path
 * (S504 §8c) can't be exercised end to end in this sandbox (no real GL driver -- SDL_CreateWindow
 * fails before ever reaching that code) -- this is the real, standing regression check that a
 * live graphical client session can't provide, same reason papercraft_falling_test.c exists.
 *
 * Build and run:
 *   gcc -Wall -Wextra -O2 -o /tmp/bigo_npc_visual_test day/packages/common/bigo_npc_visual_test.c \
 *       -lm && /tmp/bigo_npc_visual_test
 */
#include "bigo_npc_visual.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

static void test_citizen_and_the_men_share_mannequin_kit(void) {
    int kit; float color[3];
    int ok_citizen = bigo_npc_visual_for_role(PC_NPC_ROLE_CITIZEN, 5, 9, &kit, color);
    assert(ok_citizen);
    assert(kit == 5); /* the mannequin kit index */
    int kit_men; float color_men[3];
    int ok_men = bigo_npc_visual_for_role(PC_NPC_ROLE_THE_MEN, 5, 9, &kit_men, color_men);
    assert(ok_men);
    assert(kit_men == 5); /* the SAME mannequin kit index -- one shared rig, per S504 §8a */
    /* But genuinely different tints -- distinguishable on screen despite sharing a mesh. */
    assert(color[0] != color_men[0] || color[1] != color_men[1] || color[2] != color_men[2]);
    printf("PASS: Citizen and The Men both select the mannequin kit, with distinct tints\n");
}

static void test_zombie_gets_zombie_kit(void) {
    int kit; float color[3];
    int ok = bigo_npc_visual_for_role(PC_NPC_ROLE_ZOMBIE, 5, 9, &kit, color);
    assert(ok);
    assert(kit == 9); /* the zombie kit index, NOT the mannequin one */
    printf("PASS: Zombie selects the zombie-clip kit, not the mannequin's human clips\n");
}

static void test_failed_kit_skips_draw(void) {
    int kit; float color[3];
    /* kit_mannequin passed as -1 (failed to load) -- CITIZEN/THE_MEN must both refuse to draw. */
    assert(!bigo_npc_visual_for_role(PC_NPC_ROLE_CITIZEN, -1, 9, &kit, color));
    assert(!bigo_npc_visual_for_role(PC_NPC_ROLE_THE_MEN, -1, 9, &kit, color));
    /* kit_zombie passed as -1 -- ZOMBIE must refuse, even though kit_mannequin is fine. */
    assert(!bigo_npc_visual_for_role(PC_NPC_ROLE_ZOMBIE, 5, -1, &kit, color));
    printf("PASS: a failed-to-load kit is never handed back to the caller to draw\n");
}

static void test_unknown_role_falls_back_to_citizen_look(void) {
    int kit; float color[3];
    int ok = bigo_npc_visual_for_role((unsigned char)99, 5, 9, &kit, color);
    assert(ok);
    assert(kit == 5); /* falls back to the mannequin kit, same as a real Citizen */
    printf("PASS: an unrecognized role byte falls back to the Citizen look, not a crash/garbage kit\n");
}

static void test_facing_rad_known_values(void) {
    /* yaw=0 -> draw_yaw_deg=0 -> facing_rad = 180deg in radians = pi. */
    float f0 = bigo_npc_facing_rad_from_yaw(0.0f);
    assert(fabsf(f0 - 3.14159265f) < 0.001f);

    /* yaw=pi (180deg) -> draw_yaw_deg=180 -> facing_rad = 0deg = 0 radians. */
    float f_pi = bigo_npc_facing_rad_from_yaw(3.14159265f);
    assert(fabsf(f_pi) < 0.001f);

    /* yaw=pi/2 (90deg) -> draw_yaw_deg=90 -> facing_rad = 90deg in radians = pi/2. */
    float f_half_pi = bigo_npc_facing_rad_from_yaw(3.14159265f / 2.0f);
    assert(fabsf(f_half_pi - (3.14159265f / 2.0f)) < 0.001f);

    printf("PASS: bigo_npc_facing_rad_from_yaw matches the real \"180 - degrees\" formula at 0/90/180 degrees\n");
}

static void test_giant_bug_reuses_zombie_kit_with_evil_tint(void) {
    int kit; float color[3]; float zombie_color[3];
    int ok = bigo_giant_bug_visual_color(9, &kit, color);
    assert(ok);
    assert(kit == 9); /* the SAME zombie kit index -- no new art, per SHANKPIT's own precedent */
    bigo_npc_visual_for_role(PC_NPC_ROLE_ZOMBIE, 5, 9, &kit, zombie_color);
    /* Genuinely different tint from a regular zombie -- "evil version," not identical. */
    assert(color[0] != zombie_color[0] || color[1] != zombie_color[1] || color[2] != zombie_color[2]);
    assert(BIGO_GIANT_BUG_VISUAL_SCALE > 1.0f); /* a real, visible "BIG" scale-up, not a no-op */
    printf("PASS: giant bugs reuse the zombie kit with a distinct evil tint and a real scale-up\n");
}

static void test_giant_bug_skips_draw_if_zombie_kit_failed(void) {
    int kit; float color[3];
    assert(!bigo_giant_bug_visual_color(-1, &kit, color));
    printf("PASS: a failed-to-load zombie kit is never handed back for a giant bug to draw\n");
}

static void test_regulator_reuses_mannequin_kit_with_clinical_tint(void) {
    int kit; float color[3]; float citizen_color[3]; float men_color[3];
    int ok = bigo_regulator_visual_color(5, &kit, color);
    assert(ok);
    assert(kit == 5); /* the mannequin kit -- corporate enforcer, not a feral kit */
    bigo_npc_visual_for_role(PC_NPC_ROLE_CITIZEN, 5, 9, &kit, citizen_color);
    bigo_npc_visual_for_role(PC_NPC_ROLE_THE_MEN, 5, 9, &kit, men_color);
    /* Distinguishable from every other role already rendered, not just the two closest ones. */
    assert(color[0] != citizen_color[0] || color[1] != citizen_color[1] || color[2] != citizen_color[2]);
    assert(color[0] != men_color[0] || color[1] != men_color[1] || color[2] != men_color[2]);
    printf("PASS: Regulators reuse the mannequin kit with a clinical tint distinct from Citizens/The Men\n");
}

static void test_regulator_skips_draw_if_mannequin_kit_failed(void) {
    int kit; float color[3];
    assert(!bigo_regulator_visual_color(-1, &kit, color));
    printf("PASS: a failed-to-load mannequin kit is never handed back for a Regulator to draw\n");
}

static void test_top_regulator_reuses_mannequin_kit_with_pop_star_tint(void) {
    int kit; float color[3]; float rank_and_file_color[3];
    int ok = bigo_top_regulator_visual_color(5, &kit, color);
    assert(ok);
    assert(kit == 5); /* still the mannequin kit -- no new art, per the doc comment */
    bigo_regulator_visual_color(5, &kit, rank_and_file_color);
    assert(color[0] != rank_and_file_color[0] || color[1] != rank_and_file_color[1] || color[2] != rank_and_file_color[2]);
    printf("PASS: the Top Regulator reuses the mannequin kit with a pop-star tint distinct from rank-and-file\n");
}

static void test_top_regulator_skips_draw_if_mannequin_kit_failed(void) {
    int kit; float color[3];
    assert(!bigo_top_regulator_visual_color(-1, &kit, color));
    printf("PASS: a failed-to-load mannequin kit is never handed back for the Top Regulator to draw\n");
}

int main(void) {
    test_citizen_and_the_men_share_mannequin_kit();
    test_zombie_gets_zombie_kit();
    test_failed_kit_skips_draw();
    test_unknown_role_falls_back_to_citizen_look();
    test_facing_rad_known_values();
    test_giant_bug_reuses_zombie_kit_with_evil_tint();
    test_giant_bug_skips_draw_if_zombie_kit_failed();
    test_regulator_reuses_mannequin_kit_with_clinical_tint();
    test_regulator_skips_draw_if_mannequin_kit_failed();
    test_top_regulator_reuses_mannequin_kit_with_pop_star_tint();
    test_top_regulator_skips_draw_if_mannequin_kit_failed();
    printf("\nALL PASS\n");
    return 0;
}

#ifndef BIGO_NPC_VISUAL_H
#define BIGO_NPC_VISUAL_H

/* bigo_npc_visual.h -- pure, header-only NPC render decisions (S504 §8c), factored out of
 * apps/client/src/main.c specifically so they're testable without SDL/GL -- this sandbox has no
 * live display (main.c's own doc comment at its NPC render call site), same real reason
 * papercraft_protocol.h's pc_falling_lookup_y already lives in a header instead of main.c: "the
 * real, standing regression check for it that a live graphical client session can't provide in
 * this environment." */

#include "papercraft_protocol.h"

/* bigo_npc_visual_for_role -- Citizens and The Men share the mannequin kit (tinted apart -- no
 * separate visual asset for The Men yet, a real, named, deferred gap, not an oversight); zombies
 * get the zombie-clip kit off the exact same shared mesh (S504 §8a's own "one rig, an
 * animation-library swap per archetype" design). Returns 0 (skip drawing, *kit_out untouched) if
 * the selected role's own kit failed to load (kit_mannequin/kit_zombie < 0) -- callers must never
 * pass a negative kit index into gband_skel_npc_draw. */
static inline int bigo_npc_visual_for_role(unsigned char role, int kit_mannequin, int kit_zombie,
                                            int *kit_out, float color_out[3]) {
    int kit;
    switch (role) {
        case PC_NPC_ROLE_THE_MEN:
            kit = kit_mannequin;
            color_out[0] = 0.30f; color_out[1] = 0.33f; color_out[2] = 0.38f; /* utilitarian navy/grey coverall */
            break;
        case PC_NPC_ROLE_ZOMBIE:
            kit = kit_zombie;
            color_out[0] = 0.45f; color_out[1] = 0.55f; color_out[2] = 0.35f; /* sickly green */
            break;
        case PC_NPC_ROLE_CITIZEN:
        default:
            kit = kit_mannequin;
            color_out[0] = 0.55f; color_out[1] = 0.50f; color_out[2] = 0.46f; /* plain mannequin tan */
            break;
    }
    if (kit < 0) return 0;
    *kit_out = kit;
    return 1;
}

/* bigo_giant_bug_visual_color -- SHANKPIT's own giant-bug precedent, verbatim ("use the robot
 * rigs and meshes and animations (evil versions of the other ones we already have but BIG)"):
 * the exact same zombie kit regular zombies use, no new art, just a distinct tint and a caller-
 * applied 2.5x scale (giant bugs carry no per-instance scale field on the wire -- the draw call
 * site wraps gband_skel_npc_draw in a translate/scale/translate-back, since gband_skel_npc.h's
 * own real contract has no scale parameter). Returns 0 if the zombie kit itself failed to load
 * (kit_zombie < 0) -- same "never draw garbage" contract bigo_npc_visual_for_role already sets. */
static inline int bigo_giant_bug_visual_color(int kit_zombie, int *kit_out, float color_out[3]) {
    if (kit_zombie < 0) return 0;
    *kit_out = kit_zombie;
    color_out[0] = 0.42f; color_out[1] = 0.06f; color_out[2] = 0.05f; /* dark red -- evil-version tint */
    return 1;
}
#define BIGO_GIANT_BUG_VISUAL_SCALE 2.5f

/* bigo_regulator_visual_color -- the mannequin kit (Regulators are corporate enforcers, not
 * feral), tinted stark clinical white -- deliberately the coldest, most alien-to-the-palette
 * color of any role rendered here (Citizen tan, The Men navy/grey, zombie sickly green), matching
 * the lore's own "uberplumbers... acid and foam" framing: not a person in a uniform, a cleanup
 * instrument. Returns 0 if the mannequin kit failed to load, same contract as the others above. */
static inline int bigo_regulator_visual_color(int kit_mannequin, int *kit_out, float color_out[3]) {
    if (kit_mannequin < 0) return 0;
    *kit_out = kit_mannequin;
    color_out[0] = 0.92f; color_out[1] = 0.93f; color_out[2] = 0.95f; /* stark clinical white */
    return 1;
}

/* bigo_top_regulator_visual_color -- founder real-time, 2026-09-23: "the top regulator is a pop
 * singer dancing werewolf ninja John Wick." Logged (`emily observe`, Apple #20555) and checked
 * before building anything: no werewolf/ninja/John-Wick mesh, rig, or clip exists anywhere in
 * this repo's vendored GOLDENBAND assets (`day/apps/client/assets/goldenband` -- checked
 * directly), so the real, full boss character is asset-blocked, named future work, not guessed
 * at here (same "asset-blocked, name it, don't fake it" precedent already used for Los Hermanos
 * Minguinos/Catastrophe Crow, EMILY/BACKLOG.md SECTION 536). What IS real and already vendored:
 * `gband_skel_npc.h`'s own `GBAND_SKEL_NPC_ANIM_DANCE` override, loaded for the mannequin kit
 * since S504 (`UAL1_Standard_Dance_Loop`) but never selected by any real call site until now --
 * the honest "pop singer dancing" half of the concept, with zero new art. A hot-pink/magenta
 * tint (clashing hard against the rank-and-file's clinical white) is this pass's own real, small
 * flavor marker for "this one is different," not a rank/boss field on the wire -- ServerRegulator
 * has no such field server-side; the client picks array index 0 as "the Top Regulator" purely as
 * a visual convention, honestly documented as client-only, not a real boss mechanic. */
static inline int bigo_top_regulator_visual_color(int kit_mannequin, int *kit_out, float color_out[3]) {
    if (kit_mannequin < 0) return 0;
    *kit_out = kit_mannequin;
    color_out[0] = 0.95f; color_out[1] = 0.15f; color_out[2] = 0.55f; /* hot pink/magenta -- pop-star flare */
    return 1;
}
#define BIGO_TOP_REGULATOR_SLOT 0 /* client-only visual convention, see doc comment above */

/* bigo_npc_facing_rad_from_yaw -- converts a PcNpcState's own wire yaw (radians, world-space
 * heading -- PcPlayerState's own established convention) into gband_skel_npc_draw's facing_rad,
 * applying the same "180 - degrees" correction SHANKPIT's own lobby established for this exact
 * glTF-imported mannequin rig's bind pose (SHANKPIT apps/lobby/src/main.c's own
 * draw_player_skin_mannequin) -- not independently re-verified visually here (no live GL in this
 * sandbox), same honest limitation that fix itself already carries upstream. */
static inline float bigo_npc_facing_rad_from_yaw(float yaw_rad) {
    float draw_yaw_deg = yaw_rad * (180.0f / 3.14159265f);
    return (180.0f - draw_yaw_deg) * 0.0174533f;
}

#endif // BIGO_NPC_VISUAL_H

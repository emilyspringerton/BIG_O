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

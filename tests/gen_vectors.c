/* Emits parity vectors: every rules function over a dense input domain, computed by the PARENA->C build.
 * Line format: "<fn> <args...> = <result>". test_witness_rules.c re-checks each line (and a future Java test can too). */
#include <stdio.h>
#include "witness_rules.h"

int main(void) {
    static const int ARR[] = {0, 10, 15, 16, 50, 69, 70, 100};
    printf("silence_threshold = %d\npanic_arrogance_max = %d\nengage_arrogance_min = %d\ndecorum_start = %d\ndecorum_cap = %d\n",
           silence_threshold(), panic_arrogance_max(), engage_arrogance_min(), decorum_start(), decorum_cap());
    printf("suspicion_below = %d\nhysteric_below = %d\nwall_hp_concrete = %d\nbreach_dps_super = %d\n",
           suspicion_below(), hysteric_below(), wall_hp_concrete(), breach_dps_super());
    for (int t = 0; t <= 8; t++) for (int a = 0; a <= 4; a++) printf("effective_witnesses %d %d = %d\n", t, a, effective_witnesses(t, a));
    for (int c = 0; c <= 8; c++) for (int i = 0; i < 8; i++) for (int k = 0; k <= 1; k++) for (int z = 0; z <= 1; z++)
        printf("witness_state %d %d %d %d = %d\n", c, ARR[i], k, z, witness_state(c, ARR[i], k, z));
    for (int s = 0; s <= 5; s++) printf("escalation_rank %d = %d\n", s, escalation_rank(s));
    for (int p = 0; p <= 5; p++) for (int c = 0; c <= 7; c++) for (int i = 0; i < 8; i += 2) for (int k = 0; k <= 1; k++) for (int z = 0; z <= 1; z++) for (int rz = 0; rz <= 2; rz++)
        printf("npc_next_state %d %d %d %d %d %d = %d\n", p, c, ARR[i], k, z, rz, npc_next_state(p, c, ARR[i], k, z, rz));
    for (int f = 0; f <= 5; f++) for (int t = 0; t <= 5; t++) printf("is_legal_transition %d %d = %d\n", f, t, is_legal_transition(f, t));
    for (int i = 0; i < 8; i++) for (int t = 0; t <= 2; t++) printf("engage_outcome %d %d = %d\n", ARR[i], t, engage_outcome(ARR[i], t));
    for (int a = 0; a <= 2; a++) for (int s = 0; s <= 1; s++) for (int m = 1; m <= 7; m++) printf("silence_target_mask %d %d %d = %d\n", a, s, m, silence_target_mask(a, s, m));
    for (int d = -5; d <= 110; d++) printf("clamp_decorum %d = %d\ndecorum_band %d = %d\n", d, clamp_decorum(d), d, decorum_band(d));
    for (int a = 0; a <= 6; a++) printf("decorum_delta %d = %d\n", a, decorum_delta(a));
    for (int d = 0; d <= 100; d++) for (int a = 0; a <= 5; a++) printf("decorum_after %d %d = %d\n", d, a, decorum_after(d, a));
    for (int c = 0; c <= 3; c++) for (int z = 0; z <= 4; z++) for (int k = 0; k <= 1; k++) printf("zone_access %d %d %d = %d\n", c, z, k, zone_access(c, z, k));
    for (int a = 0; a <= 1; a++) for (int g = 0; g <= 1; g++) printf("conspicuousness %d %d = %d\n", a, g, conspicuousness(a, g));
    static const int V[] = {0, 25, 50, 100}, C[] = {0, 20, 40, 60};
    for (int v = 0; v < 4; v++) for (int c = 0; c < 4; c++) for (int r = 0; r < 100; r += 3) printf("noticed %d %d %d = %d\n", V[v], C[c], r, noticed(V[v], C[c], r));
    for (int t = 0; t <= 2; t++) printf("move_speed_pct %d = %d\nwall_max_hp %d = %d\n", t, move_speed_pct(t), t, wall_max_hp(t));
    for (int t = 0; t <= 2; t++) for (int r = 0; r <= 1; r++) printf("breach_dps %d %d = %d\n", t, r, breach_dps(t, r));
    for (int h = 0; h <= 500; h += 125) for (int d = 0; d <= 50; d += 25) for (int s = 0; s <= 12; s += 4) printf("wall_hp_after %d %d %d = %d\n", h, d, s, wall_hp_after(h, d, s));
    static const int HP[] = {0, 1, 500};
    for (int s = 0; s <= 4; s++) for (int h = 0; h <= 1; h++) for (int t = 0; t <= 2; t++) for (int r = 0; r <= 1; r++) for (int w = 0; w < 3; w++) for (int e = 0; e <= 1; e++)
        printf("zombie_next_state %d %d %d %d %d %d = %d\n", s, h, t, r, HP[w], e, zombie_next_state(s, h, t, r, HP[w], e));
    return 0;
}

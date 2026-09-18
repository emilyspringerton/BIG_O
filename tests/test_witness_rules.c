/* Independent oracle + property tests for the witness rules. Expected values are HAND-TYPED from
 * docs/B1_WITNESS_RULES.md (not derived from witness_rules.c), so a regression can't grade itself. Then 100k
 * randomized inputs (ASan/UBSan build) check invariants, then every parity vector must match the C build. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "witness_rules.h"

static int fails = 0, checks = 0;
#define EQ(expr, want) do { checks++; int g_ = (expr); if (g_ != (want)) { fails++; \
    printf("FAIL %s:%d  %s = %d, want %d\n", __FILE__, __LINE__, #expr, g_, (want)); } } while (0)
#define OK(cond) do { checks++; if (!(cond)) { fails++; printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); } } while (0)

static uint32_t rs = 0x9E3779B9u;
static uint32_t rnd(void) { rs ^= rs << 13; rs ^= rs >> 17; rs ^= rs << 5; return rs; }
static int rr(int lo, int hi) { return lo + (int)(rnd() % (uint32_t)(hi - lo + 1)); }

static void oracle(void) {
    /* constants */
    EQ(silence_threshold(), 5); EQ(panic_arrogance_max(), 15); EQ(engage_arrogance_min(), 70);
    EQ(decorum_start(), 80); EQ(decorum_cap(), 100); EQ(suspicion_below(), 60); EQ(hysteric_below(), 30);
    EQ(wall_hp_concrete(), 500); EQ(breach_dps_super(), 50);
    /* witness reactions: transcript rules */
    EQ(witness_state(0, 50, 0, 1), WS_UNAWARE);
    EQ(witness_state(1, 50, 0, 1), WS_DENIAL);           /* 1 witness -> catatonic denial */
    EQ(witness_state(4, 50, 0, 1), WS_DENIAL);
    EQ(witness_state(5, 50, 0, 1), WS_SILENCING);        /* 5+ -> aggressive silencing */
    EQ(witness_state(9, 50, 0, 0), WS_SILENCING);
    EQ(witness_state(5, 70, 0, 1), WS_ENGAGE);           /* overconfident vs a zombie */
    EQ(witness_state(5, 70, 0, 0), WS_SILENCING);        /* ...but not vs a non-zombie event */
    EQ(witness_state(3, 15, 0, 1), WS_PANIC);            /* cowards flee at any count >= 1 */
    EQ(witness_state(7, 0, 0, 1), WS_PANIC);
    EQ(witness_state(0, 0, 0, 1), WS_UNAWARE);
    EQ(witness_state(3, 50, 1, 1), WS_COMPROMISED);      /* compromised beats everything */
    EQ(witness_state(0, 50, 1, 0), WS_COMPROMISED);
    EQ(effective_witnesses(5, 1), 4); EQ(effective_witnesses(2, 3), 0); EQ(effective_witnesses(0, 0), 0);
    EQ(escalation_rank(WS_UNAWARE), 0); EQ(escalation_rank(WS_COMPROMISED), 0); EQ(escalation_rank(WS_DENIAL), 1);
    EQ(escalation_rank(WS_PANIC), 1); EQ(escalation_rank(WS_SILENCING), 3); EQ(escalation_rank(WS_ENGAGE), 3);
    /* next-state (prev, count, arrogance, compromised, zombie-event, resolved) */
    EQ(npc_next_state(WS_COMPROMISED, 9, 50, 0, 1, 0), WS_COMPROMISED);  /* absorbing, even when resolved */
    EQ(npc_next_state(WS_COMPROMISED, 0, 50, 0, 0, 2), WS_COMPROMISED);
    EQ(npc_next_state(WS_DENIAL, 2, 50, 1, 0, 0), WS_COMPROMISED);       /* forced witness */
    EQ(npc_next_state(WS_SILENCING, 2, 50, 0, 0, 0), WS_SILENCING);
    EQ(npc_next_state(WS_ENGAGE, 1, 50, 0, 0, 0), WS_ENGAGE);
    EQ(npc_next_state(WS_SILENCING, 0, 50, 0, 0, 0), WS_SILENCING);      /* line of sight lost: hunt persists */
    EQ(npc_next_state(WS_ENGAGE, 0, 90, 0, 1, 0), WS_ENGAGE);
    EQ(npc_next_state(WS_SILENCING, 0, 50, 0, 0, 1), WS_DENIAL);         /* cleanup crew memory wipe */
    EQ(npc_next_state(WS_ENGAGE, 3, 90, 0, 1, 1), WS_DENIAL);
    EQ(npc_next_state(WS_SILENCING, 4, 50, 0, 0, 2), WS_UNAWARE);        /* target eliminated */
    EQ(npc_next_state(WS_ENGAGE, 0, 90, 0, 1, 2), WS_UNAWARE);
    EQ(npc_next_state(WS_SILENCING, 2, 50, 1, 0, 1), WS_COMPROMISED);    /* compromise still wins */
    EQ(npc_next_state(WS_UNAWARE, 5, 50, 0, 1, 0), WS_SILENCING);
    EQ(npc_next_state(WS_DENIAL, 5, 50, 0, 1, 0), WS_SILENCING);         /* escalation allowed */
    EQ(npc_next_state(WS_DENIAL, 0, 50, 0, 1, 0), WS_UNAWARE);           /* non-hunting states do fall with count */
    EQ(npc_next_state(WS_DENIAL, 3, 50, 0, 0, 1), WS_DENIAL);            /* resolved is ignored unless hunting */
    /* legality */
    OK(is_legal_transition(WS_COMPROMISED, WS_COMPROMISED)); OK(!is_legal_transition(WS_COMPROMISED, WS_DENIAL));
    OK(!is_legal_transition(WS_COMPROMISED, WS_UNAWARE)); OK(is_legal_transition(WS_SILENCING, WS_SILENCING));
    OK(is_legal_transition(WS_SILENCING, WS_UNAWARE)); OK(is_legal_transition(WS_SILENCING, WS_DENIAL));
    OK(is_legal_transition(WS_ENGAGE, WS_DENIAL)); OK(is_legal_transition(WS_SILENCING, WS_COMPROMISED));
    OK(!is_legal_transition(WS_SILENCING, WS_PANIC)); OK(!is_legal_transition(WS_ENGAGE, WS_PANIC));
    OK(!is_legal_transition(WS_ENGAGE, WS_SILENCING)); OK(is_legal_transition(WS_DENIAL, WS_SILENCING));
    OK(is_legal_transition(WS_UNAWARE, WS_PANIC));
    /* engage outcome */
    EQ(engage_outcome(70, 1), 1); EQ(engage_outcome(100, 2), 1); EQ(engage_outcome(70, 0), 2); EQ(engage_outcome(69, 1), 0);
    /* crew attribution */
    EQ(silence_target_mask(0, 0, 7), 1); EQ(silence_target_mask(1, 0, 7), 2); EQ(silence_target_mask(2, 0, 7), 4);
    EQ(silence_target_mask(1, 1, 3), 3); EQ(silence_target_mask(0, 1, 7), 7);
    /* decorum */
    EQ(decorum_delta(DA_SMALL_TALK), 5); EQ(decorum_delta(DA_SAY_APOCALYPSE), -25); EQ(decorum_delta(DA_CARRY_GEAR), -15);
    EQ(decorum_delta(DA_WRONG_COSTUME), -20); EQ(decorum_delta(DA_ATTRIBUTED_EVENT), -40); EQ(decorum_delta(DA_QUIET_TICK), 1);
    EQ(decorum_after(80, DA_SAY_APOCALYPSE), 55); EQ(decorum_after(98, DA_SMALL_TALK), 100); EQ(decorum_after(10, DA_ATTRIBUTED_EVENT), 0);
    EQ(decorum_band(100), BAND_OK); EQ(decorum_band(60), BAND_OK); EQ(decorum_band(59), BAND_SUSPICION);
    EQ(decorum_band(30), BAND_SUSPICION); EQ(decorum_band(29), BAND_HYSTERIC); EQ(decorum_band(1), BAND_HYSTERIC);
    EQ(decorum_band(0), BAND_CANCELLED);
    /* costume x zone (rows SUIT, SMOCK, JANITOR, STREET; cols PUBLIC LAB EXEC GEN; vault separate) */
    static const int M[4][4] = {{1,0,1,0},{1,1,0,0},{1,0,0,1},{0,0,0,0}};
    for (int c = 0; c < 4; c++) for (int z = 0; z < 4; z++) { EQ(zone_access(c, z, 0), M[c][z]); EQ(zone_access(c, z, 1), M[c][z]); }
    /* vault: every costume x token in {0,1}, hand-typed (BIG_O#1 claim B): needs token AND a non-street costume */
    EQ(zone_access(COS_SUIT, ZONE_VAULT, 0), 0);      EQ(zone_access(COS_SUIT, ZONE_VAULT, 1), 1);
    EQ(zone_access(COS_LAB_SMOCK, ZONE_VAULT, 0), 0); EQ(zone_access(COS_LAB_SMOCK, ZONE_VAULT, 1), 1);
    EQ(zone_access(COS_JANITOR, ZONE_VAULT, 0), 0);   EQ(zone_access(COS_JANITOR, ZONE_VAULT, 1), 1);
    EQ(zone_access(COS_STREET, ZONE_VAULT, 0), 0);    EQ(zone_access(COS_STREET, ZONE_VAULT, 1), 0);
    /* noticing */
    EQ(conspicuousness(1, 0), 0); EQ(conspicuousness(0, 0), 40); EQ(conspicuousness(1, 1), 20); EQ(conspicuousness(0, 1), 60);
    OK(noticed(50, 0, 49)); OK(!noticed(50, 0, 50)); OK(noticed(50, 40, 89)); OK(!noticed(50, 40, 90));
    OK(noticed(100, 60, 99)); OK(!noticed(0, 0, 0));
    /* terrain */
    EQ(move_speed_pct(TAG_DIRT), 100); EQ(move_speed_pct(TAG_CONCRETE), 0); EQ(move_speed_pct(TAG_REINFORCED), 0);
    EQ(wall_max_hp(TAG_CONCRETE), 500); EQ(wall_max_hp(TAG_DIRT), 0); EQ(wall_max_hp(TAG_REINFORCED), 0);
    EQ(breach_dps(TAG_CONCRETE, 0), 0); EQ(breach_dps(TAG_CONCRETE, 1), 50); EQ(breach_dps(TAG_REINFORCED, 1), 0);
    EQ(wall_hp_after(500, 50, 10), 0); EQ(wall_hp_after(500, 50, 4), 300); EQ(wall_hp_after(100, 0, 99), 100);
    /* zombie tree */
    EQ(zombie_next_state(ZS_SURFACE_SURGE, 0, TAG_DIRT, 1, 0, 0), ZS_PASSIVE_HEEL);
    EQ(zombie_next_state(ZS_PASSIVE_HEEL, 1, TAG_DIRT, 0, 0, 0), ZS_SUBTERRANEAN_SWIM);
    EQ(zombie_next_state(ZS_SUBTERRANEAN_SWIM, 1, TAG_DIRT, 0, 0, 1), ZS_SURFACE_SURGE);
    EQ(zombie_next_state(ZS_SUBTERRANEAN_SWIM, 1, TAG_CONCRETE, 0, 500, 0), ZS_BLOCKED);
    EQ(zombie_next_state(ZS_SUBTERRANEAN_SWIM, 1, TAG_CONCRETE, 1, 500, 0), ZS_WALL_BREACH);
    EQ(zombie_next_state(ZS_WALL_BREACH, 1, TAG_CONCRETE, 1, 0, 0), ZS_SURFACE_SURGE);
    EQ(zombie_next_state(ZS_WALL_BREACH, 1, TAG_REINFORCED, 1, 500, 0), ZS_BLOCKED);
    EQ(zombie_next_state(ZS_SURFACE_SURGE, 1, TAG_CONCRETE, 0, 500, 0), ZS_SURFACE_SURGE);  /* sprint persists */
}

static void properties(void) {
    for (int n = 0; n < 100000; n++) {
        int arr = rr(0, 100), z = rr(0, 1), c = rr(0, 12), prev = rr(0, 5), k = rr(0, 1);
        /* escalation never decreases as witnesses grow (uncompromised) */
        OK(escalation_rank(witness_state(c + 1, arr, 0, z)) >= escalation_rank(witness_state(c, arr, 0, z)));
        /* next-state is always a legal transition */
        int rz = rr(0, 2);
        int nx = npc_next_state(prev, c, arr, k, z, rz);
        OK(nx >= 0 && nx <= 5); OK(is_legal_transition(prev, nx));
        /* compromised is absorbing */
        if (prev == WS_COMPROMISED) OK(nx == WS_COMPROMISED);
        /* no silencing/engage state ever drops while resolved=0, whatever the witness count (only forced compromise exits) */
        if ((prev == WS_SILENCING || prev == WS_ENGAGE) && rz == 0 && !k) OK(nx == prev);
        /* resolution is the only way out: 1 -> DENIAL, 2 -> UNAWARE */
        if ((prev == WS_SILENCING || prev == WS_ENGAGE) && rz == 1 && !k) OK(nx == WS_DENIAL);
        if ((prev == WS_SILENCING || prev == WS_ENGAGE) && rz == 2 && !k) OK(nx == WS_UNAWARE);
        /* decorum stays in [0, cap] and band is consistent */
        int d = rr(-50, 150), act = rr(0, 5);
        int a2 = decorum_after(clamp_decorum(d), act);
        OK(a2 >= 0 && a2 <= decorum_cap());
        int b = decorum_band(a2);
        OK(b >= 0 && b <= 3); OK((b == BAND_CANCELLED) == (a2 <= 0));
        /* lower decorum never improves the band */
        OK(decorum_band(clamp_decorum(d)) >= decorum_band(clamp_decorum(d + 1)));
        /* access: street never allowed; public allowed for all non-street; token only matters in the vault */
        int cs = rr(0, 3), zn = rr(0, 4), tk = rr(0, 1);
        int acc = zone_access(cs, zn, tk);
        OK(acc == 0 || acc == 1);
        if (cs == COS_STREET) OK(acc == 0);
        if (zn == ZONE_PUBLIC && cs != COS_STREET) OK(acc == 1);
        if (zn != ZONE_VAULT) OK(acc == zone_access(cs, zn, 1 - tk));
        if (zn == ZONE_VAULT && cs != COS_STREET) OK(acc == tk);
        /* more vigilance never makes noticing less likely */
        int vg = rr(0, 100), cn = rr(0, 60), roll = rr(0, 99);
        if (noticed(vg, cn, roll)) OK(noticed(vg + 1, cn, roll));
        if (noticed(vg, cn, roll)) OK(noticed(vg, cn + 1, roll));
        /* wall hp never negative or increased */
        int hp = rr(0, 500), dps = rr(0, 60), s = rr(0, 20);
        int h2 = wall_hp_after(hp, dps, s);
        OK(h2 >= 0 && h2 <= hp);
        /* zombie state in range and no target => heel */
        int zs = zombie_next_state(rr(0, 4), rr(0, 1), rr(0, 2), rr(0, 2), rr(0, 500), rr(0, 1));
        OK(zs >= 0 && zs <= 4);
        OK(zombie_next_state(rr(0, 4), 0, rr(0, 2), rr(0, 2), rr(0, 500), rr(0, 1)) == ZS_PASSIVE_HEEL);
        /* silence mask never empty and within crew */
        int cm = rr(1, 7), at = rr(0, 2), st = rr(0, 1);
        int mk = silence_target_mask(at, st, cm);
        OK(mk != 0 && mk <= 7);
    }
}

typedef int (*F0)(void); typedef int (*F1)(int); typedef int (*F2)(int, int); typedef int (*F3)(int, int, int);
typedef int (*F4)(int, int, int, int); typedef int (*F5)(int, int, int, int, int); typedef int (*F6)(int, int, int, int, int, int);
typedef int (*F7)(int, int, int, int, int, int, int);
static const struct { const char *n; int arity; void *f; } TAB[] = {
    {"silence_threshold", 0, silence_threshold}, {"panic_arrogance_max", 0, panic_arrogance_max},
    {"engage_arrogance_min", 0, engage_arrogance_min}, {"decorum_start", 0, decorum_start}, {"decorum_cap", 0, decorum_cap},
    {"suspicion_below", 0, suspicion_below}, {"hysteric_below", 0, hysteric_below}, {"wall_hp_concrete", 0, wall_hp_concrete},
    {"breach_dps_super", 0, breach_dps_super}, {"effective_witnesses", 2, effective_witnesses}, {"witness_state", 4, witness_state},
    {"escalation_rank", 1, escalation_rank}, {"npc_next_state", 6, npc_next_state}, {"is_legal_transition", 2, is_legal_transition},
    {"engage_outcome", 2, engage_outcome}, {"silence_target_mask", 3, silence_target_mask}, {"clamp_decorum", 1, clamp_decorum},
    {"decorum_delta", 1, decorum_delta}, {"decorum_after", 2, decorum_after}, {"decorum_band", 1, decorum_band},
    {"zone_access", 3, zone_access}, {"conspicuousness", 2, conspicuousness}, {"noticed", 3, noticed},
    {"move_speed_pct", 1, move_speed_pct}, {"wall_max_hp", 1, wall_max_hp}, {"breach_dps", 2, breach_dps},
    {"wall_hp_after", 3, wall_hp_after}, {"zombie_next_state", 6, zombie_next_state},
};

static int vectors(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) { printf("FAIL cannot open %s\n", path); return 0; }
    char line[160]; int n = 0;
    while (fgets(line, sizeof line, f)) {
        char *eq = strstr(line, " = ");
        if (!eq) { fails++; printf("FAIL malformed vector: %s", line); continue; }
        int want = atoi(eq + 3); *eq = 0;
        char fn[64]; int a[6] = {0}; int got_args = sscanf(line, "%63s %d %d %d %d %d %d", fn, &a[0], &a[1], &a[2], &a[3], &a[4], &a[5]) - 1;
        int found = 0, got = 0;
        for (size_t i = 0; i < sizeof TAB / sizeof TAB[0]; i++) {
            if (strcmp(TAB[i].n, fn)) continue;
            found = 1;
            if (got_args != TAB[i].arity) { fails++; printf("FAIL arity %s\n", fn); break; }
            switch (TAB[i].arity) {
                case 0: got = ((F0)TAB[i].f)(); break; case 1: got = ((F1)TAB[i].f)(a[0]); break;
                case 2: got = ((F2)TAB[i].f)(a[0], a[1]); break; case 3: got = ((F3)TAB[i].f)(a[0], a[1], a[2]); break;
                case 4: got = ((F4)TAB[i].f)(a[0], a[1], a[2], a[3]); break;
                case 5: got = ((F5)TAB[i].f)(a[0], a[1], a[2], a[3], a[4]); break;
                default: got = ((F6)TAB[i].f)(a[0], a[1], a[2], a[3], a[4], a[5]); break;
            }
            break;
        }
        if (!found) { fails++; printf("FAIL unknown vector fn %s\n", fn); continue; }
        checks++; n++;
        if (got != want) { fails++; printf("FAIL vector %s: got %d want %d\n", fn, got, want); }
    }
    fclose(f);
    return n;
}

int main(int argc, char **argv) {
    oracle(); properties();
    int nv = vectors(argc > 1 ? argv[1] : "tests/parity_vectors.txt");
    if (nv < 2000) { fails++; printf("FAIL only %d parity vectors read\n", nv); }
    printf("test_witness_rules: %d checks (%d parity vectors), %d failures\n", checks, nv, fails);
    return fails ? 1 : 0;
}

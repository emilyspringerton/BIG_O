/* bigo_sim -- text simulation of BIG_O's witness/decorum rules for a co-op crew of 1-3.
 *   bigo_sim run <scenario.txt>     run a script; `expect` lines are assertions; exit 1 on any failure
 *   bigo_sim play [crew 1-3] [seed] interactive stdin mode (same commands, state printed after each)
 *   bigo_sim --version
 * Script format (see the scenarios directory):
 *   seed N | crew N | npc <zone> <vig> <arr> | player <p> [zone Z] [costume C] [token 0|1] [gear 0|1]
 *   loslost | wipe [zone] | eliminate P
 *   enter P Z | costume P C | gear P 0|1 | token P 0|1 | observe P | say P | talk P | release P TIER | force NPC P | tick N | state
 *   expect npc N state <NAME> | expect npc N accomplice 0|1 | expect player P band <NAME>
 *   expect decorum P <==|>=|<=|<|>> V | expect hunted P 0|1 | expect cancelled P 0|1                                        */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "sim.h"
#include "mission.h"
#include "world.h"
#include "witness_rules.h"

#ifndef BIGO_VERSION
#define BIGO_VERSION "0.0.0-dev"
#endif

typedef struct { Sim sim; Mission mis; World world; int have; uint32_t seed; int nfail, nexpect; } Ctx;

static int ieq(const char *a, const char *b) {
    for (; *a && *b; a++, b++) if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
    return *a == *b;
}
static int lookup(const char *s, const char *const *names, int n) {
    for (int i = 0; i < n; i++) if (ieq(s, names[i])) return i;
    return -1;
}
static const char *const ZN[] = {"public", "lab", "exec", "generator", "vault"};
static const char *const CN[] = {"suit", "smock", "janitor", "street"};
static const char *const SN[] = {"UNAWARE", "DENIAL", "COMPROMISED", "SILENCING", "PANIC", "ENGAGE"};
static const char *const BN[] = {"OK", "SUSPICION", "HYSTERIC", "CANCELLED"};
static int zone_of(const char *s) { int z = lookup(s, ZN, 5); return z >= 0 ? z : (isdigit((unsigned char)s[0]) ? atoi(s) : -1); }
static int costume_of(const char *s) { int c = lookup(s, CN, 4); return c >= 0 ? c : (isdigit((unsigned char)s[0]) ? atoi(s) : -1); }

static int script_fail(Ctx *c, int lineno, const char *msg) {
    c->nfail++;
    printf("EXPECT FAIL line %d: %s\n", lineno, msg);
    return 1;
}

static int player_ok(const Sim *s, int p) { return p >= 0 && p < s->nplayers; }

/* returns 0 ok, nonzero on a script error or failed expectation (counted in ctx) */
static int run_line(Ctx *c, char *line, int lineno, int echo_state) {
    char *hash = strchr(line, '#');
    if (hash) *hash = 0;
    char *tok[10];
    int nt = 0;
    for (char *t = strtok(line, " \t\r\n"); t && nt < 10; t = strtok(NULL, " \t\r\n")) tok[nt++] = t;
    if (nt == 0) return 0;
    const char *cmd = tok[0];
    if (ieq(cmd, "seed") && nt >= 2) { c->seed = (uint32_t)strtoul(tok[1], NULL, 10); return 0; }
    if (ieq(cmd, "crew") && nt >= 2) { sim_init(&c->sim, c->seed ? c->seed : 1, atoi(tok[1]), stdout); c->have = 1; return 0; }
    if (!c->have) { sim_init(&c->sim, c->seed ? c->seed : 1, 1, stdout); c->have = 1; }
    Sim *s = &c->sim;
    printf("> ");
    for (int i = 0; i < nt; i++) printf("%s%s", i ? " " : "", tok[i]);
    printf("\n");
    char msg[160];

    if (ieq(cmd, "npc") && nt >= 4) {
        int id = sim_add_npc(s, zone_of(tok[1]), atoi(tok[2]), atoi(tok[3]));
        if (id < 0) return script_fail(c, lineno, "npc: bad zone or too many NPCs");
        printf("  npc%d added\n", id);
    } else if (ieq(cmd, "player") && nt >= 2) {
        int p = atoi(tok[1]);
        if (!player_ok(s, p)) return script_fail(c, lineno, "player: no such player");
        for (int i = 2; i + 1 < nt; i += 2) {
            if (ieq(tok[i], "zone")) {
                int z = zone_of(tok[i + 1]);
                if (z < 0 || z > 4) return script_fail(c, lineno, "player: bad zone");
                s->p[p].zone = z;
            } else if (ieq(tok[i], "costume")) sim_set_costume(s, p, costume_of(tok[i + 1]));
            else if (ieq(tok[i], "token")) sim_set_token(s, p, atoi(tok[i + 1]));
            else if (ieq(tok[i], "gear")) sim_set_gear(s, p, atoi(tok[i + 1]));
            else return script_fail(c, lineno, "player: unknown attribute");
        }
    } else if (ieq(cmd, "enter") && nt >= 3) {
        if (sim_enter(s, atoi(tok[1]), zone_of(tok[2])) < 0) return script_fail(c, lineno, "enter: rejected");
    } else if (ieq(cmd, "costume") && nt >= 3) {
        if (sim_set_costume(s, atoi(tok[1]), costume_of(tok[2])) < 0) return script_fail(c, lineno, "costume: rejected");
    } else if (ieq(cmd, "gear") && nt >= 3) { sim_set_gear(s, atoi(tok[1]), atoi(tok[2]));
    } else if (ieq(cmd, "token") && nt >= 3) { sim_set_token(s, atoi(tok[1]), atoi(tok[2]));
    } else if (ieq(cmd, "observe") && nt >= 2) { sim_observe(s, atoi(tok[1]));
    } else if (ieq(cmd, "say") && nt >= 2) { sim_say_apocalypse(s, atoi(tok[1]));
    } else if (ieq(cmd, "talk") && nt >= 2) { sim_talk(s, atoi(tok[1]));
    } else if (ieq(cmd, "release") && nt >= 3) {
        if (sim_release(s, atoi(tok[1]), atoi(tok[2])) < 0) return script_fail(c, lineno, "release: rejected");
    } else if (ieq(cmd, "force") && nt >= 3) { sim_force_witness(s, atoi(tok[1]), atoi(tok[2]));
    } else if (ieq(cmd, "loslost")) { sim_los_lost(s);
    } else if (ieq(cmd, "wipe")) { sim_memory_wipe(s, nt >= 2 ? zone_of(tok[1]) : -1);
    } else if (ieq(cmd, "eliminate") && nt >= 2) { if (sim_eliminate(s, atoi(tok[1])) < 0) return script_fail(c, lineno, "eliminate: rejected");
    } else if (ieq(cmd, "world") && nt >= 2 && ieq(tok[1], "start")) { world_init(&c->world, s, c->seed ? c->seed : 1, nt >= 3 ? atoi(tok[2]) : 7);
    } else if (ieq(cmd, "weather") && nt >= 2) {
        static const char *WN[] = { "clear", "overcast", "rain", "storm" };
        int x = lookup(tok[1], WN, 4); if (x < 0) return script_fail(c, lineno, "weather: unknown");
        world_force_weather(&c->world, s, (Weather)x);
    } else if (ieq(cmd, "wtick") && nt >= 2) { world_tick(&c->world, s, atoi(tok[1]));
    } else if (ieq(cmd, "area") && nt >= 3) {
        static const char *AN[] = { "wasteland", "nextown", "office", "park", "basement" };
        int a = lookup(tok[2], AN, AREA_COUNT);
        if (a < 0 || world_set_area(&c->world, s, atoi(tok[1]), a) < 0) return script_fail(c, lineno, "area: rejected");
    } else if (ieq(cmd, "harvest") && nt >= 2) { world_harvest(&c->world, s, atoi(tok[1]));
    } else if (ieq(cmd, "mission") && nt >= 2 && ieq(tok[1], "start")) { mission_start(&c->mis); fprintf(stdout, "MISSION A1M1 started (23:00)\n");
    } else if (ieq(cmd, "mtick") && nt >= 2) { mission_tick(&c->mis, s, atoi(tok[1]));
    } else if ((ieq(cmd, "m") || ieq(cmd, "m!")) && nt >= 2) {
        Mission *m = &c->mis; int pl = nt >= 3 ? atoi(tok[2]) : 0, r = -2;
        if (ieq(tok[1], "smock")) r = mission_acquire_smock(m, s, pl);
        else if (ieq(tok[1], "sector2")) r = mission_enter_sector2(m, s, pl);
        else if (ieq(tok[1], "login")) r = mission_supervisor(m, s, 1);
        else if (ieq(tok[1], "break")) r = mission_supervisor(m, s, 0);
        else if (ieq(tok[1], "surf")) r = mission_surf(m, s, pl);
        else if (ieq(tok[1], "insert")) r = mission_insert_drive(m, s, pl);
        else if (ieq(tok[1], "swap")) r = mission_swap_drive(m, s, pl);
        else if (ieq(tok[1], "mop")) r = mission_grab_mop(m, s, pl);
        else if (ieq(tok[1], "exit")) r = mission_exit(m, s, pl);
        else if (ieq(tok[1], "upload")) r = mission_upload(m, s, pl);
        if (r == -2) return script_fail(c, lineno, "m: unknown mission action");
        if (ieq(cmd, "m") && r < 0) return script_fail(c, lineno, "m: action rejected");
        if (ieq(cmd, "m!") && r >= 0) return script_fail(c, lineno, "m!: action was accepted but should have been rejected");
    } else if (ieq(cmd, "tick") && nt >= 2) { sim_tick(s, atoi(tok[1]));
    } else if (ieq(cmd, "state")) { sim_print_state(s, stdout);
    } else if (ieq(cmd, "expect") && nt >= 4) {
        c->nexpect++;
        if (ieq(tok[1], "world") && nt >= 4) {
            World *w = &c->world; int got = -1, want = atoi(tok[nt - 1]);
            static const char *PN[] = { "DAWN", "DAY", "DUSK", "NIGHT" };
            static const char *WN2[] = { "CLEAR", "OVERCAST", "RAIN", "STORM" };
            static const char *AN2[] = { "wasteland", "nextown", "office", "park", "basement" };
            if (ieq(tok[2], "phase")) { got = (int)world_phase(w); want = lookup(tok[3], PN, 4); }
            else if (ieq(tok[2], "weather")) { got = (int)w->weather; want = lookup(tok[3], WN2, 4); }
            else if (ieq(tok[2], "day")) got = world_day(w);
            else if (ieq(tok[2], "hour")) got = world_minute_of_day(w) / 60;
            else if (ieq(tok[2], "spawned")) got = w->spawned_total;
            else if (ieq(tok[2], "sight")) got = s->public_sight_pct;
            else if (ieq(tok[2], "cap") && nt >= 5) { int a2 = lookup(tok[3], AN2, AREA_COUNT); if (a2 < 0) return script_fail(c, lineno, "expect world cap: bad area"); got = world_area_cap(w, a2); }
            else if (ieq(tok[2], "zombies") && nt >= 5) { int a2 = lookup(tok[3], AN2, AREA_COUNT); if (a2 < 0) return script_fail(c, lineno, "expect world zombies: bad area"); got = world_zombies_in(w, a2); }
            else if (ieq(tok[2], "sample") && nt >= 6) got = w->samples[atoi(tok[3])][atoi(tok[4])];
            else if (ieq(tok[2], "gear") && nt >= 5) got = s->p[atoi(tok[3])].gear;
            else if (ieq(tok[2], "alerts")) got = w->nalerts;
            else if (ieq(tok[2], "reflux")) got = w->reflux.total_dispatched;
            else return script_fail(c, lineno, "expect world: unknown field");
            if (got != want) { snprintf(msg, sizeof msg, "world %s is %d, wanted %d", tok[2], got, want); return script_fail(c, lineno, msg); }
        } else if (ieq(tok[1], "mission") && nt >= 4) {
            Mission *m = &c->mis; int got = -1, want = atoi(tok[3]);
            if (ieq(tok[2], "state")) { static const char *N[] = { "INACTIVE", "ACTIVE", "COMPLETE", "FAILED" }; int w = lookup(tok[3], N, 4); got = (int)m->state; want = w; }
            else if (ieq(tok[2], "phase")) got = mission_phase(m);
            else if (ieq(tok[2], "pct")) got = m->transfer_pct;
            else if (ieq(tok[2], "pin")) got = m->pin_known;
            else return script_fail(c, lineno, "expect mission: unknown field");
            if (got != want) { snprintf(msg, sizeof msg, "mission %s is %d, wanted %s", tok[2], got, tok[3]); return script_fail(c, lineno, msg); }
        } else if (ieq(tok[1], "npc") && nt >= 5) {
            int n = atoi(tok[2]);
            if (n < 0 || n >= s->nnpcs) return script_fail(c, lineno, "expect npc: no such npc");
            if (ieq(tok[3], "state")) {
                int want = lookup(tok[4], SN, 6);
                if (want < 0 || s->n[n].state != want) {
                    snprintf(msg, sizeof msg, "npc%d state %s, wanted %s", n, sim_ws_name(s->n[n].state), tok[4]);
                    return script_fail(c, lineno, msg);
                }
            } else if (ieq(tok[3], "accomplice")) {
                if (s->n[n].accomplice != atoi(tok[4])) {
                    snprintf(msg, sizeof msg, "npc%d accomplice %d, wanted %s", n, s->n[n].accomplice, tok[4]);
                    return script_fail(c, lineno, msg);
                }
            } else return script_fail(c, lineno, "expect npc: unknown field");
        } else if (ieq(tok[1], "player") && nt >= 5 && ieq(tok[3], "band")) {
            int p = atoi(tok[2]), want = lookup(tok[4], BN, 4);
            if (!player_ok(s, p)) return script_fail(c, lineno, "expect player: no such player");
            if (want < 0 || decorum_band(s->p[p].decorum) != want) {
                snprintf(msg, sizeof msg, "p%d band %s, wanted %s", p, sim_band_name(decorum_band(s->p[p].decorum)), tok[4]);
                return script_fail(c, lineno, msg);
            }
        } else if (ieq(tok[1], "decorum") && nt >= 5) {
            int p = atoi(tok[2]), v = atoi(tok[4]);
            if (!player_ok(s, p)) return script_fail(c, lineno, "expect decorum: no such player");
            int d = s->p[p].decorum, ok = 0;
            if (!strcmp(tok[3], "==")) ok = d == v;
            else if (!strcmp(tok[3], ">=")) ok = d >= v;
            else if (!strcmp(tok[3], "<=")) ok = d <= v;
            else if (!strcmp(tok[3], "<")) ok = d < v;
            else if (!strcmp(tok[3], ">")) ok = d > v;
            if (!ok) {
                snprintf(msg, sizeof msg, "p%d decorum %d not %s %d", p, d, tok[3], v);
                return script_fail(c, lineno, msg);
            }
        } else if (ieq(tok[1], "hunted") || ieq(tok[1], "cancelled")) {
            int p = atoi(tok[2]);
            if (!player_ok(s, p)) return script_fail(c, lineno, "expect: no such player");
            int have = ieq(tok[1], "hunted") ? s->p[p].hunted : s->p[p].cancelled;
            if (have != atoi(tok[3])) {
                snprintf(msg, sizeof msg, "p%d %s=%d, wanted %s", p, tok[1], have, tok[3]);
                return script_fail(c, lineno, msg);
            }
        } else return script_fail(c, lineno, "expect: unknown form");
        printf("  ok\n");
    } else {
        return script_fail(c, lineno, "unknown command");
    }
    if (echo_state) sim_print_state(s, stdout);
    return 0;
}

static int run_stream(FILE *in, int interactive, Ctx *c) {
    char line[256];
    int lineno = 0;
    while (1) {
        if (interactive) { printf("bigo> "); fflush(stdout); }
        if (!fgets(line, sizeof line, in)) break;
        lineno++;
        if (interactive && (line[0] == 'q' || !strncmp(line, "quit", 4))) break;
        run_line(c, line, lineno, interactive);
    }
    return c->nfail;
}

int main(int argc, char **argv) {
    Ctx c;
    memset(&c, 0, sizeof c);
    c.seed = 1;
    if (argc >= 2 && !strcmp(argv[1], "--version")) { printf("bigo_sim %s\n", BIGO_VERSION); return 0; }
    if (argc >= 3 && !strcmp(argv[1], "run")) {
        FILE *f = fopen(argv[2], "r");
        if (!f) { fprintf(stderr, "cannot open %s\n", argv[2]); return 2; }
        int fails = run_stream(f, 0, &c);
        fclose(f);
        printf("RESULT %s: %d expectations, %d failures\n", argv[2], c.nexpect, fails);
        if (c.nexpect == 0) { printf("RESULT: scenario has no expectations\n"); return 1; }
        return fails ? 1 : 0;
    }
    if (argc >= 2 && !strcmp(argv[1], "play")) {
        int crew = argc >= 3 ? atoi(argv[2]) : 1;
        if (argc >= 4) c.seed = (uint32_t)strtoul(argv[3], NULL, 10);
        sim_init(&c.sim, c.seed, crew, stdout);
        c.have = 1;
        for (int i = 0; i < 6; i++) sim_add_npc(&c.sim, ZONE_PUBLIC, 30 + 5 * i, 40 + 10 * i);
        printf("BIG_O crew sim: %d player(s), 6 NPCs in the public zone. Commands: enter P zone | costume P name | gear P 0|1 |\n"
               "say P | talk P | release P tier | force NPC P | observe P | tick N | state | quit\n"
               "(zones: public lab exec generator vault; costumes: suit smock janitor street)\n", c.sim.nplayers);
        sim_print_state(&c.sim, stdout);
        run_stream(stdin, 1, &c);
        return 0;
    }
    fprintf(stderr, "usage: bigo_sim run <scenario.txt> | play [crew] [seed] | --version\n");
    return 2;
}

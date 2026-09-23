/* PAPERCRAFT -- real game server, grown well past its own original "real Phase 0" scope
 * (NORTHSTAR.md's own "Real Phase 0" section: "a player can log in and spawn in the real
 * persistent city, nothing else" -- accurate for this file's very first version, not its current
 * one; kept here as the real starting point, not corrected away).
 *
 * A real, single-node, always-running UDP server -- no matches, no per-match instances
 * ("papercraft shouldnt have matches and the matches shouldnt end"). Fetches the real, live
 * GoblinFoxDragon worldapi urban chunk grid (scene 200, a real static multi-chunk grid --
 * packages/common/papercraft_world.h's own `PwWorld`) once at startup, verifies a real HMAC
 * connect-ticket (minted by IDUNA's PapercraftTicketHandler from a real
 * POST /api/v1/auth/email/login) on every CONNECT -- fails closed, same discipline
 * WEAKNIGHT_BEDROCK_RACERS' own apps/server already established -- and ticks real,
 * server-authoritative on-foot movement with real jump/gravity physics and a real slide-jump
 * trick, real Paper Engine destruction (`PC_PACKET_INTERACT`, real PARENA-decided damage/reward),
 * real talent-point spending (`PC_PACKET_ALLOCATE_TALENT`, real PARENA-decided gate), real
 * leveling/XP, real restart-persistence for both player progression and world-object damage, and
 * an optional real dynamically-loaded mod registry (`--mods-manifest`, live-reloadable via
 * `SIGHUP`) — see `MODDING.md`/`NORTHSTAR.md` for the full real, current feature list; this
 * header intentionally doesn't re-enumerate what's since shipped, to avoid going stale again the
 * same way its own original "no jump/fall physics yet, no destruction wiring, no talent spending"
 * line did.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <dlfcn.h>

#include "../../../packages/common/http_client.h"
#include "../../../packages/common/hmac_sha256.h"
#include "../../../packages/common/papercraft_protocol.h"
#include "../../../packages/common/lz4mini.h"
#include "../../../packages/common/papercraft_inventory.h"
#include "../../../packages/common/papercraft_world.h"
#include "../../../packages/common/paper_mesh.h"
#include "../../../packages/common/papercraft_persist.h"
#include "../../../packages/common/bigo_pheromone.h"
#include "../../../packages/common/bigo_food_items.h"
/* S504 §8c -- the real NPC-entity system giving core/npc_archetype.h (Citizens/The Men) and
 * core/zombie_values.h (zombies) a live server tick to actually drive, instead of proving them in
 * isolation only. See ServerNpc's own doc comment below for the full design. */
#include "../../../../core/npc_archetype.h"
#include "../../../../core/zombie_values.h"
#include "../../../../core/witness_rules.h"
#include "../../../../core/witness_live.h"
/* EMILY/BACKLOG.md SECTION 536 reverse-port phase 3 ("bring SHANKPIT stuff back into BIG_O"):
 * Giant Zombie Bugs, brought back from SHANKPIT's own live witness_ai.c wiring. See
 * server_giant_bug_command_authorized/server_tick_giant_bugs below for the real, live
 * "Men hold the key" gate + eat-a-nearby-zombie loop. */
#include "../../../../core/giant_bug_values.h"

#define PC_SERVER_PORT 7799
#define PC_TICK_HZ 20 /* on-foot movement doesn't need a vehicle sim's own 60Hz -- real, deliberately lower tick rate for Phase 0 */
#define PC_TICK_DT (1.0f / (float)PC_TICK_HZ)
#define PC_SNAPSHOT_HZ 10 /* real, deliberately DECOUPLED from PC_TICK_HZ (2026-08-30, founder
    real-time: "papercraft is still flashing the reconnection message to me pretty frequently"
    -- found live: the server was broadcasting a full 1436-byte PcSnapshotPacket to every player
    20 times/sec (~29KB/s per client), real, meaningful bandwidth pressure on a constrained
    connection (matches the founder's own earlier-described low-bandwidth 5G situation, S206-59).
    Real fix: simulate at the full PC_TICK_HZ (movement/physics/gravity/falling-fragment
    integration all stay real-time responsive, unaffected below), but only BUILD and SEND the
    real snapshot broadcast every (PC_TICK_HZ / PC_SNAPSHOT_HZ) ticks -- halving real client
    bandwidth without touching simulation fidelity at all. Must evenly divide PC_TICK_HZ. */
#define PC_MOVE_SPEED 4.0f /* world units/sec, real walking pace */
/* PC_SPRINT_SPEED -- "the SHANKPIT run" (S504-10, SHANKPIT/PAPERCRAFT physics unification, founder
 * real-time: "make it work like the shankpit run so normally we are walking but there is a run
 * button"), converted into this file's own units-per-second convention, not guessed. SHANKPIT's
 * own PlayerState has exactly one move speed -- MAX_SPEED = 0.95 (packages/common/physics.h) --
 * applied as a direct per-tick position delta with no dt multiply at all (confirmed live:
 * update_entity's own `p->x += p->vx`, packages/simulation/local_game.h), at SHANKPIT's own fixed
 * SHANKPIT_NET_FIXED_DT = 0.016s tick (packages/common/net_sim.h) = 62.5 ticks/sec. Steady-state
 * speed is therefore 0.95 * 62.5 = 59.375 world units/sec in SHANKPIT's own coordinate space --
 * the SAME space this server now renders real NOCK levels in (level_loader.h, S504-10b), so no
 * further scale conversion is needed. PC_MOVE_SPEED's own walking pace was tuned for the smaller,
 * 1-unit-block PAPERCRAFT city grid; loading a SHANKPIT-scale level like nextown (walls up to 864
 * units wide) at that same 4.0 units/sec pace would take minutes to cross on foot -- sprint is
 * this file's own real fix for that scale mismatch, not merely a cosmetic speed-up. */
#define PC_SPRINT_SPEED 59.375f
#define PC_USERCMD_STALE_MS 500
#define PC_PLAYER_TIMEOUT_MS 60000 /* real, generous "genuinely abandoned" threshold -- comfortably
                                       longer than apps/client's own real PC_CLIENT_STALE_MS
                                       reconnect-detection window, so a legitimate real reconnect
                                       attempt (a brief network blip, not a genuinely closed
                                       client) always wins the race and reclaims the same slot via
                                       the existing real reconnect-by-player_id lookup, rather than
                                       losing it to this timeout first. Closes a real gap this
                                       always-running, never-ending persistent server
                                       (NORTHSTAR.md's own "papercraft shouldnt have matches and
                                       the matches shouldnt end") had no defense against at all: a
                                       slot claimed by a crashed/closed client with no clean
                                       disconnect packet (UDP has none) stayed active()==1 forever,
                                       permanently eating one of PC_MAX_PLAYERS(16) real slots.

                                       Real, live bump 30000 -> 60000 (2026-08-30, founder
                                       real-time, on real 5G/limited-bandwidth: "we are getting a
                                       lot of connection lost can we get it to be more forgiving
                                       for low bandwidth?") -- doubled in lockstep with
                                       apps/client's own PC_CLIENT_STALE_MS bump (20000 ->
                                       45000ms), keeping the same real, deliberate proportional
                                       safety margin between the two (the client must always give
                                       up and stop trying comfortably before the server actually
                                       evicts the slot, or a real reconnect attempt could lose the
                                       race and never land). */
#define PC_INTERACT_REACH 2.5f  /* world units in front of the player an interact request can reach */
#define PC_INTERACT_RADIUS 1.0f /* real hit radius, matches paper_mesh_test.c's own real "shotgun blast" scenario */
#define PC_INTERACT_DAMAGE 30   /* real damage per hit -- CONCRETE fragments (80 max HP, 50% resist) take ~3 real hits to break */

/* Real jump/gravity physics -- PAPERCRAFT's own first vertical movement, closing the next real
   gap down NORTHSTAR.md's own "Explicitly not Phase 0" list (trick/skate input) alongside the
   real slide-jump trick below. Tuned for this game's own real world scale (1 unit = 1 block,
   PC_MOVE_SPEED = 4 units/sec walking) rather than a blind unit-for-unit copy of
   SHANKPIT_CONSTRUCT.txt's own GRAVITY_DROP/JUMP_FORCE constants -- the construct's own values
   are per-tick deltas at an unstated real tick rate and an unconfirmed world scale, so porting
   the raw numbers here would be a real unit mismatch, not a real port. PC_GRAVITY=12/
   PC_JUMP_VELOCITY=5 gives a real ~1.04-unit-high jump (v^2/2g) over about 0.83s of real
   hangtime -- roughly one block, a reasonable "GTA3/Skate2" traversal jump at this scale. What
   IS ported faithfully from the construct is the real slide-jump trick multiplier formula itself
   (packages/simulation/slide_jump_mod.c) -- a dimensionless ratio, unaffected by this scale
   choice. */
#define PC_GRAVITY 12.0f        /* world units/sec^2 */
#define PC_JUMP_VELOCITY 5.0f   /* world units/sec, real initial upward impulse on a grounded jump press */
#define PC_SLIDE_JUMP_MIN_SPEED 0.5f /* real minimum horizontal speed to qualify for a slide-jump, matches the construct's own real gate */
#define PC_SLIDE_JUMP_BOOST_MS 800   /* real, timed speed-boost window -- PAPERCRAFT has no persistent
                                        momentum/velocity model yet (unlike the construct's own real
                                        vx/vz physics), so a real, honest timed multiplier is this
                                        game's own real equivalent of "boosting your velocity";
                                        ~matches the real jump's own hangtime above by design, so a
                                        slide-jump's boost roughly lasts through the jump it came from */

static PwWorld g_world;
/* Real, editor-authored world objects (packages/common/papercraft_worldobjects.h) -- replaces
   the old hardcoded single test cube with a real, persisted, map-editor-editable object list
   (apps/mapeditor). g_wo_file holds the real placement data (position/material/seed);
   g_wo_mesh[i] holds each active object's own real, generated PaperCubeMesh (regenerated once at
   startup from g_wo_file.objects[i], same real deterministic seed+params the client
   independently regenerates too). */
static PcWorldObjectFile g_wo_file;
static PaperCubeMesh g_wo_mesh[PC_WO_MAX_OBJECTS];
static char g_world_objects_path[256] = "var/world/objects.dat";
static char g_world_damage_path[256] = "var/world/damage.dat";
static char g_mods_manifest_path[256] = ""; /* empty = disabled, real default -- unchanged
                                                behavior for every existing deployment/test until
                                                --mods-manifest is explicitly given */
static unsigned int g_last_world_save_ms = 0; /* real periodic damage-autosave cadence, shared
                                                  across all objects (not per-player) -- see
                                                  PC_AUTOSAVE_MS */
static int g_wo_destroyed_awarded[PC_WO_MAX_OBJECTS]; /* real, per-object "already paid out the
                                                           real xp_award_mod reward" latch -- an
                                                           object stays fully GONE after it's
                                                           destroyed, so without this a player
                                                           could re-earn real XP by re-punching an
                                                           already-destroyed object every tick */

/* g_falling -- real Phase 1a server-authoritative fragment physics state (NORTHSTAR.md's own
   "Real Phase 1" section). Server-internal only -- NOT the wire shape (PcFallingFragment,
   papercraft_protocol.h, only ever carries y; x/z/vy are real, pure server bookkeeping the client
   never needs, since this real first slice's own explicit non-goal is lateral scatter -- x/z
   never move after a real fragment detaches). */
typedef struct {
    int active;
    int object_idx;
    int fragment_idx;
    float x, y, z;
    float vy;
    float rotation_deg;         /* real Phase 1c -- current real spin angle, integrated every tick */
    float angular_velocity_deg_s; /* real, constant, assigned once at spawn */
} PcServerFallingFragment;
static PcServerFallingFragment g_falling[PC_FALLING_FRAGMENTS_MAX];
static int g_falling_next_evict = 0; /* real, simple round-robin eviction cursor -- used only when
                                         every real slot is already active; a genuinely new real
                                         detach event always wins a free slot first if one exists */

/* spawn_falling_fragment: real Phase 1a trigger -- called once per real fragment that JUST
   transitioned to PAPER_STATE_GONE (the caller already diffed before/after state to know exactly
   which ones, same real technique apps/client's own debris-spawn diff logic already uses).
   Computes the real, fixed world-space detach position (object's own real broadcast position +
   this fragment's own real local center -- the exact same real computation
   spawn_debris_for_fragment already does client-side) and claims a real, bounded slot: the first
   free one, or -- only if all PC_FALLING_FRAGMENTS_MAX are already active -- the real, simple
   round-robin oldest one. A real, deliberately small cap (see its own doc comment) means eviction
   is a real, live possibility under real, sustained destruction, not just a theoretical case. */
static void spawn_falling_fragment(int object_idx, int fragment_idx) {
    if (object_idx < 0 || object_idx >= g_wo_file.count) return;
    if (fragment_idx < 0 || fragment_idx >= g_wo_mesh[object_idx].fragment_count) return;

    PaperFragment *f = &g_wo_mesh[object_idx].fragments[fragment_idx];
    float wx = g_wo_file.objects[object_idx].x + f->center.x;
    float wy = g_wo_file.objects[object_idx].y + f->center.y;
    float wz = g_wo_file.objects[object_idx].z + f->center.z;

    int slot = -1;
    for (int i = 0; i < PC_FALLING_FRAGMENTS_MAX; i++) {
        if (!g_falling[i].active) { slot = i; break; }
    }
    if (slot < 0) {
        slot = g_falling_next_evict;
        g_falling_next_evict = (g_falling_next_evict + 1) % PC_FALLING_FRAGMENTS_MAX;
    }

    g_falling[slot].active = 1;
    g_falling[slot].object_idx = object_idx;
    g_falling[slot].fragment_idx = fragment_idx;
    g_falling[slot].x = wx;
    g_falling[slot].y = wy;
    g_falling[slot].z = wz;
    g_falling[slot].vy = 0.0f;
    g_falling[slot].rotation_deg = 0.0f;
    /* Real Phase 1c -- a real, deterministic, bounded spin rate per fragment (90..270 deg/s),
       derived from fragment_idx, no rand() -- same real "deterministic, not random" convention
       apps/client's own debris "kick" jitter (kick = 1.5f + 1.0f * (frag_idx % 7) / 7.0f)
       already established. */
    g_falling[slot].angular_velocity_deg_s = 90.0f + 180.0f * ((float)(fragment_idx % 7) / 7.0f);
}

typedef struct {
    int active;
    int lz4; /* client advertised PC_CAP_LZ4 on its latest CONNECT */
    unsigned int usercmds_rx, snaps_tx; /* per-window telemetry, reset each [net] report */
    PcPlayerState state;
    struct sockaddr_in addr;
    socklen_t addr_len;
    float latest_move_x, latest_move_z;
    float latest_yaw; /* real, decoupled camera-facing yaw from the player's own latest UserCmd --
                          see packages/common/papercraft_protocol.h's own PcUserCmdPacket comment
                          and this file's own movement-tick comment below for the full real
                          design (2026-09-02). */
    unsigned int latest_buttons; /* real PC_BTN_* bitmask from the player's own latest UserCmd */
    unsigned int latest_cmd_seq;
    unsigned int last_usercmd_ms;
    unsigned int latest_cmd_time_ms; /* real, verbatim copy of the client's OWN cmd_time_ms
        (its own local clock, not this server's) -- echoed back per-recipient in
        PcSnapshotPacket::echo_cmd_time_ms so each client can compute a real round-trip time as
        now_ms() - echo_cmd_time_ms with no clock-sync assumption at all, since both the send and
        the compare happen on the SAME client's own clock (see papercraft_protocol.h's own doc
        comment on echo_cmd_time_ms for the full real reasoning). */
    int has_player_id;
    unsigned char player_id[16];
    unsigned int last_xp_tick_ms; /* real per-second cadence, mirrors the construct's own progression_tick */
    unsigned int last_save_ms;    /* real periodic-autosave cadence -- see PC_AUTOSAVE_MS */

    /* Real jump/gravity + slide-jump trick state (not part of PcPlayerState/the wire format --
       only position/yaw need to cross the wire; vy and the boost window are server-internal). */
    float vy;
    int on_ground;
    int was_holding_jump;
    int speed_boost_permille;      /* 1000 = no boost; see PC_SLIDE_JUMP_BOOST_MS */
    unsigned int speed_boost_until_ms;

    /* Real, fixed-slot inventory (2026-08-30, founder real-time: "gta3 style stuff drops and you
       can pick it up ffxi style list affordances"). Reuses PcInventoryUpdatePacket's own
       PcInventorySlot type directly -- this IS the wire shape, not a server-internal type that
       gets translated into one. NOT yet persisted across a restart (packages/common/
       papercraft_persist.h's own PcSaveRecord has no inventory field yet) -- a real, honest,
       explicitly deferred gap for this first slice, same as position/XP were before persistence
       existed at all; a fresh spawn AND a real restart-restore both start empty for now. */
    PcInventorySlot inventory[PC_INVENTORY_SLOTS];

    /* Real, per-player "arsenal" ownership (2026-09-07, founder real-time: "not all characters
       get all aresenals you have to find a [shotgun] etc"). Bit N set = PC_WPN_* slot N has been
       found (a PC_ITEM_WPN_* entity pickup) and is real, usable arsenal for this player -- see
       on_papercraft_weapon_switch_allowed's own real gate. Server-internal only, same real
       "private to the owner" treatment as `inventory` above -- never broadcast in
       PcSnapshotPacket, only ever sent to this exact player via PcWeaponOwnedPacket.
       PC_WPN_KNIFE needs no bit here at all (the universal baseline every character always has,
       the mod's own real rule), so this starts at 0 (memset), not pre-seeded with bit 0 set. NOT
       yet persisted across a restart, same real, honest, explicitly-named gap `inventory` above
       already carries (packages/common/papercraft_persist.h's own PcSaveRecord has neither
       field yet) -- a fresh spawn and a real restart-restore both start with nothing owned but
       the baseline Knife. */
    unsigned int weapons_owned;
    /* current_weapon -- real, server-authoritative PC_WPN_* slot this player currently has
       equipped. NOT part of PcPlayerState/the snapshot broadcast (see PcWeaponOwnedPacket's own
       doc comment in papercraft_protocol.h for the real wire-budget reason) -- confirmed back to
       this exact player only, via send_weapon_owned_update, after every switch attempt. Starts
       at PC_WPN_KNIFE (0, memset default), the universal baseline every character has. */
    unsigned char current_weapon;

    /* EMILY/BACKLOG.md SECTION 536 follow-up, live Decorum tracking (BIG_O/NORTHSTAR.md §18 Phase
     * A). Server-internal bookkeeping only -- not part of PcPlayerState/the wire format, same
     * "only position/yaw need to cross the wire" precedent vy/on_ground above already set. */
    int decorum_zone;             /* ZONE_* this player was in as of the last observe check --
                                      detects zone-ENTRY transitions, same real "observe fires once
                                      on sim_enter, not every tick" precedent core/sim.c already
                                      uses. Starts at -1 (no zone yet) so the very first tick always
                                      counts as an entry. */
    unsigned int last_quiet_tick_ms; /* real, own v1 passive-regen cadence -- see
                                      BIGO_DECORUM_QUIET_TICK_MS's own doc comment below */
    int decorum_cancelled_logged;  /* real, one-time marker so BAND_CANCELLED only logs once per
                                       episode, not every tick while it stays cancelled */
} PlayerSlot;

/* Real, "simple but trackable" GTA3-style dropped-item entity -- see packages/common/
   papercraft_protocol.h's own doc comment on PcEntitySpawnPacket for the full real design
   rationale (event-driven, not folded into the continuous snapshot). Array index IS the real
   entity_id sent on the wire, same "slot index is the id" convention g_slots[]/PC_MAX_PLAYERS
   already uses for client_id. */
typedef struct {
    int active;
    unsigned char item_id;
    float x, y, z;
} ServerEntity;
static ServerEntity g_entities[PC_ENTITY_MAX];

/* ServerNpc -- S504 §8c's own real, named next step: a live, role-bearing NPC, server-authoritative
 * like everything else in this file. Array index IS the wire slot index, same "slot index is the
 * id" convention g_entities[]/g_slots[] already use. Brain state (HumannessState via NpcBrain, or
 * ZombieState) lives ONLY here, server-side -- PcNpcState (papercraft_protocol.h) deliberately
 * carries none of it, matching the file's own established "server decides, client renders" split.
 * Both an NpcBrain AND a ZombieState field, unconditionally, rather than a union: at PC_NPC_MAX=8
 * the wasted bytes are trivial, and a plain struct is simpler to reason about than a tagged union
 * for a v0 this small -- only the field matching `role` is ever ticked or read. */
typedef struct {
    int active;
    unsigned char role; /* PC_NPC_ROLE_* */
    float x, y, z, yaw;
    NpcBrain brain;       /* valid when role == PC_NPC_ROLE_CITIZEN or PC_NPC_ROLE_THE_MEN */
    ZombieState zombie;   /* valid when role == PC_NPC_ROLE_ZOMBIE */

    /* S504-DISPATCH (2026-09-20) -- valid when role == PC_NPC_ROLE_CITIZEN or PC_NPC_ROLE_THE_MEN.
       witness_state is core/witness_rules.h's own WS_* enum -- see server_tick_witness. arrogance
       is a real, live witness_rules.c input (engage vs. silence, panic threshold); a fixed 50 for
       every human NPC in this v0 -- per-NPC arrogance variety is real, honest, separate, future
       work (a personality axis, same category as §8e item 6's deferred PARENA-scriptable presets),
       not attempted here. */
    int witness_state;
    int arrogance;

    /* S504-DISPATCH -- valid when role == PC_NPC_ROLE_THE_MEN only: this NPC's own real, live
       dispatch assignment. has_dispatch_target/dispatch_target_npc name WHICH human NPC (by index
       into this same g_npcs[] array) this The Men unit is currently travelling to go resolve. */
    int has_dispatch_target;
    int dispatch_target_npc;

    /* SECTION 536 follow-up (2026-09-23, NORTHSTAR.md §11 item 6, "the men carry pagers"): valid
       when has_dispatch_target is set. pager_buzz_until_ms is the real assignment-to-response
       delay -- a Man is assigned but does NOT move until this clock elapses, replacing the old
       instant assignment. 0 means not currently buzzing (either idle, or already past the buzz
       and actively responding). */
    unsigned int pager_buzz_until_ms;
} ServerNpc;
static ServerNpc g_npcs[PC_NPC_MAX];

/* ServerGiantBug -- SECTION 536 reverse-port phase 3. Deliberately a SEPARATE array from g_npcs,
 * not a new PC_NPC_ROLE_* -- growing PC_NPC_MAX would change PcNpcState[PC_NPC_MAX]'s own wire
 * size (papercraft_protocol.h), a real, riskier protocol change; SHANKPIT's own live version
 * (witness_ai.c's g_giant_bugs[]) made the exact same call for the exact same reason. No network
 * broadcast yet -- server-side simulation only, verified via log output, same "prove it live in
 * the log first" precedent server_throw_pheromone/server_tick_witness already established. */
#define BIGO_GIANT_BUG_MAX 8
#define BIGO_GIANT_BUG_EAT_RADIUS 4.0f /* matches SHANKPIT's own WITNESS_AI_BUG_EAT_RADIUS */
typedef struct {
    int active;
    float x, y, z;
    GiantBugState bstate;
} ServerGiantBug;
static ServerGiantBug g_giant_bugs[BIGO_GIANT_BUG_MAX];

/* "wheelbarrow" carry state -- SECTION 536 reverse-port phase 4, brought back from SHANKPIT's own
 * witness_ai.c ("cannon - add wheelbarrow for carrying whole zombies or citizens back to your
 * lab", founder real-time, 2026-09-22). g_carried_npc_index (-1 = nothing carried) indexes into
 * g_npcs[]; g_carrier_slot tracks WHICH connected player is carrying, so the cargo trails that
 * specific player's own position -- unlike SHANKPIT's own version, which always trails a fixed
 * "hero" (player slot 0), this repo's own live server has no such fixed-hero convention (real
 * co-op, up to 3 players, NORTHSTAR.md §7). Real, honest scope cut, same as SHANKPIT's own: no
 * literal wheelbarrow prop/model -- the carry itself is the real feature. Lab delivery target is
 * a new, real, hardcoded circle at (BIGO_LAB_ZONE_CX, BIGO_LAB_ZONE_CZ) -- well clear of the
 * 10-unit NPC spawn circle at world origin and the giant-bug spawn point near (-9,0,1), same
 * "hardcoded coordinates, no LevelZone/JSON authoring needed" precedent every other landmark in
 * this live server already uses (pheromone detection radius, witness detection radius). */
static int g_carried_npc_index = -1;
static int g_carrier_slot = -1;
static int g_lab_deliveries = 0;
#define BIGO_WHEELBARROW_PICKUP_RADIUS 4.0f
#define BIGO_LAB_ZONE_CX 30.0f
#define BIGO_LAB_ZONE_CZ 0.0f
#define BIGO_LAB_ZONE_RADIUS 6.0f

/* Live Decorum tracking -- EMILY/BACKLOG.md SECTION 536 follow-up, BIG_O/NORTHSTAR.md §18 Phase
 * A: the QUIET-observation half of the witness system (core/witness_rules.c's own zone_access/
 * conspicuousness/noticed/decorum_*), wired live for the first time. Only ZONE_PUBLIC and
 * ZONE_LAB (reusing the wheelbarrow's own existing lab-delivery circle above -- zero new landmark
 * authoring) are actually placed in this world yet; ZONE_EXEC/ZONE_GENERATOR/ZONE_VAULT have no
 * live landmark, named and deferred in NORTHSTAR.md §18, not guessed at here. */
#define BIGO_QUIET_OBSERVE_RADIUS 10.0f /* deliberately tighter than BIGO_WITNESS_DETECTION_RADIUS
    (25.0) -- noticing an outfit needs real proximity, hearing a zombie scream doesn't */
#define BIGO_DECORUM_QUIET_TICK_MS 10000u /* real, own, v1 passive-regen cadence (not spec'd
    anywhere else) -- core/sim.c's own sim_tick applies DA_QUIET_TICK every ABSTRACT scenario
    turn; this maps that to a real, named, retunable real-time cadence instead: +1 decorum every
    10 real seconds a player spends NOT currently in violation. */

/* Real, seeded xorshift32 server RNG -- same shape core/sim.c's own roll100/SHANKPIT's
 * food_pickup.c own lnf_roll_item already use -- feeds noticed()'s own pre-rolled 0..99 input.
 * The live day server had no RNG of any kind before this (every other system here is
 * deterministic), so this is a new, small, real addition, not a reuse. */
static uint32_t g_decorum_rng = 0x9E3779B9u;
static int server_roll100(void) {
    g_decorum_rng ^= g_decorum_rng << 13;
    g_decorum_rng ^= g_decorum_rng >> 17;
    g_decorum_rng ^= g_decorum_rng << 5;
    return (int)(g_decorum_rng % 100u);
}

/* Cake-smash distraction -- EMILY/BACKLOG.md SECTION 536 follow-up, BIG_O/NORTHSTAR.md §20.
 * Faithful port of SHANKPIT's own witness_ai_smash_cake/witness_ai_distraction_active (packages/
 * simulation/witness_ai.c) -- a global, non-spatial "is any distraction active right now" flag,
 * not a per-citizen/per-location mechanic (matching that file's own doc comment: "a true
 * distraction-target mechanic would need [more]," not attempted here either). Halves every
 * nearby NPC's effective vigilance in server_tick_decorum's own noticed() check below for
 * BIGO_DISTRACTION_MS -- this is the real reason cake-smash was previously misattributed as
 * blocked on zones EXEC/GENERATOR/VAULT (NORTHSTAR.md §17's own correction): the real blocker was
 * the QUIET-observation half of the witness system not being live at all, which Phase A (§18)
 * already resolved -- this pass finally closes the loop. */
#define BIGO_DISTRACTION_MS 8000u /* mirrors SHANKPIT's own WITNESS_AI_DISTRACTION_MS exactly */
static unsigned int g_distraction_until_ms = 0;

static void server_smash_cake(unsigned int now_ms) {
    g_distraction_until_ms = now_ms + BIGO_DISTRACTION_MS;
    printf("S536-CAKE: smashed -- distraction active for %ums\n", BIGO_DISTRACTION_MS);
}

static int server_distraction_active(unsigned int now_ms) {
    return now_ms < g_distraction_until_ms;
}

/* server_player_zone -- real, minimal zone lookup for this world's own two live-placed zones. */
static int server_player_zone(const PlayerSlot *s) {
    float dx = s->state.x - BIGO_LAB_ZONE_CX, dz = s->state.z - BIGO_LAB_ZONE_CZ;
    if (dx * dx + dz * dz <= BIGO_LAB_ZONE_RADIUS * BIGO_LAB_ZONE_RADIUS) return ZONE_LAB;
    return ZONE_PUBLIC;
}

/* g_pheromones -- S504-PHEROMONE real command-point state (bigo_pheromone.h). Global rather than
 * per-crew: this v0 has exactly one shared crew/world (NORTHSTAR.md §7's own "one crew, one
 * onboarding" decision), so there is no separate crew scope to key it by yet. */
static PheromoneMarker g_pheromones[BIGO_PHEROMONE_MAX];

#define PHEROMONE_DURATION_MS 30000      /* a thrown marker commands for 30 real seconds */
#define PHEROMONE_DETECTION_RADIUS 40.0f /* a zombie within this range of an active marker locks on */
#define PHEROMONE_ZOMBIE_SPEED 5.5f      /* units/sec -- deliberately below PC_SPRINT_SPEED: a
    commanded zombie closing in is a real threat, not an instant one; tuned separately from any
    human movement speed on purpose, matching zombie_values.h's own "own vocabulary" convention */

/* server_throw_pheromone -- claims a marker slot and drops a real command point at (x,y,z),
 * expiring PHEROMONE_DURATION_MS from now. See PcPheromoneThrowPacket's own doc comment for why
 * the target is computed client-side rather than derived server-side like PC_PACKET_INTERACT. */
static void server_throw_pheromone(float x, float y, float z, unsigned int now_ms) {
    int slot = pheromone_claim_slot(g_pheromones, BIGO_PHEROMONE_MAX);
    PheromoneMarker *m = &g_pheromones[slot];
    m->active = 1;
    m->x = x;
    m->z = z;
    (void)y; /* cosmetic drop height only -- targeting is x/z, matching zombies' own flat-plane movement */
    m->expires_at_ms = now_ms + PHEROMONE_DURATION_MS;
    printf("S504-PHEROMONE: marker thrown at (%.1f, %.1f), slot %d, expires in %ds\n",
           x, z, slot, PHEROMONE_DURATION_MS / 1000);
}

/* server_spawn_npcs -- real, fixed v0 test population (NORTHSTAR.md §8's own "~6 humanness-lite
 * NPCs... and one thought-police NPC" scale): 3 Citizens, 1 The Men (the thought-police-adjacent
 * "muscle" role), 4 zombies, placed on a small real circle around world origin so they're
 * findable without needing a real level loaded server-side yet (level_loader.h is client-only so
 * far, see NORTHSTAR.md §8a). Stationary in v0, on purpose -- real movement/behavior selection
 * (The Men's own dispatch loop, zombies pursuing a sensed player) is named, deferred work
 * (NORTHSTAR.md §8c items 3-4), not attempted here; this pass wires the entity system and proves
 * the brains tick live, it does not yet make them act. */
static void server_spawn_npcs(unsigned int now_ms) {
    static const unsigned char roles[PC_NPC_MAX] = {
        PC_NPC_ROLE_CITIZEN, PC_NPC_ROLE_CITIZEN, PC_NPC_ROLE_CITIZEN, PC_NPC_ROLE_THE_MEN,
        PC_NPC_ROLE_ZOMBIE, PC_NPC_ROLE_ZOMBIE, PC_NPC_ROLE_ZOMBIE, PC_NPC_ROLE_ZOMBIE
    };
    for (int i = 0; i < PC_NPC_MAX; i++) {
        ServerNpc *n = &g_npcs[i];
        memset(n, 0, sizeof(*n));
        n->active = 1;
        n->role = roles[i];
        float angle = (float)i * (2.0f * 3.14159265f / (float)PC_NPC_MAX);
        n->x = 10.0f * cosf(angle);
        n->z = 10.0f * sinf(angle);
        n->y = 0.0f;
        n->yaw = angle;
        if (n->role == PC_NPC_ROLE_ZOMBIE) {
            zombie_state_init(&n->zombie, now_ms);
        } else {
            npc_brain_init(&n->brain, (n->role == PC_NPC_ROLE_THE_MEN) ? NPC_ARCHETYPE_THE_MEN : NPC_ARCHETYPE_CITIZEN, now_ms);
            n->witness_state = WS_UNAWARE;
            n->arrogance = server_roll100(); /* real per-NPC arrogance variety, EMILY/BACKLOG.md
                SECTION 536 follow-up, BIG_O/NORTHSTAR.md §22, closes §11 item 5. Was a fixed 50
                for every human NPC -- since panic_arrogance_max()=15 and engage_arrogance_min()=70,
                a uniform 50 meant WS_PANIC and WS_ENGAGE could never fire live at all, no matter
                what happened in the world (every witness landed in the exact same SILENCING/
                DENIAL split). A real, uniform 0..99 roll from the same seeded server RNG
                server_tick_decorum already uses gives each NPC a genuine, individual personality
                instead. */
            n->has_dispatch_target = 0;
            n->pager_buzz_until_ms = 0;
        }
    }
    printf("S504: spawned %d real NPCs (3 citizen, 1 the_men, 4 zombie) on a 10-unit circle around origin.\n", PC_NPC_MAX);
}

/* server_tick_npcs -- real, per-server-tick brain update (PC_TICK_HZ, not the slower
 * PC_SNAPSHOT_HZ broadcast rate) -- humanness_tick_mood/zombie_tick are both real, internally
 * timer-gated (their own mood_change_at_ms), so calling this every tick is cheap and correct, same
 * discipline core/humanness.h's own doc comment already establishes ("per tick or per decision
 * cycle").
 *
 * S504-PHEROMONE (2026-09-20): has_target is no longer hardcoded 0 -- a zombie within
 * PHEROMONE_DETECTION_RADIUS of an active thrown marker (bigo_pheromone.h) now real-locks on
 * (has_target=1 drives zombie_tick toward HUNTING/FRENZIED per zombie_values.c's own real mood
 * arc) and actually steers toward the marker (pheromone_step_toward), closing the player-driven
 * half of NORTHSTAR.md §8e item 2. The OTHER half -- autonomous player-detection with no thrown
 * marker at all -- is still real, separate, deliberately not attempted here (a zombie with no
 * marker in range stays exactly as before: DORMANT/AGITATED on hunger drift alone, stationary). */
static void server_tick_npcs(unsigned int now_ms) {
    static unsigned int last_npc_tick_ms = 0;
    float dt_sec = (last_npc_tick_ms == 0) ? 0.0f : (float)(now_ms - last_npc_tick_ms) / 1000.0f;
    if (dt_sec > 0.5f) dt_sec = 0.5f; /* clamp a stall/hitch, matching the client's own gband dt clamp */
    last_npc_tick_ms = now_ms;

    pheromone_marker_expire(g_pheromones, BIGO_PHEROMONE_MAX, now_ms);

    for (int i = 0; i < PC_NPC_MAX; i++) {
        ServerNpc *n = &g_npcs[i];
        if (!n->active) continue;
        if (n->role == PC_NPC_ROLE_ZOMBIE) {
            float target_x, target_z;
            int has_target = pheromone_find_nearest(g_pheromones, BIGO_PHEROMONE_MAX,
                                                      n->x, n->z, PHEROMONE_DETECTION_RADIUS,
                                                      &target_x, &target_z);
            zombie_tick(&n->zombie, now_ms, has_target);
            if (has_target && dt_sec > 0.0f) {
                pheromone_step_toward(&n->x, &n->z, target_x, target_z, PHEROMONE_ZOMBIE_SPEED, dt_sec);
                n->yaw = atan2f(target_x - n->x, target_z - n->z);
            }
        } else {
            npc_brain_tick(&n->brain, now_ms);
        }
    }
}

/* server_spawn_giant_bugs -- real, fixed v0 test population: ONE bug, placed close enough to
 * npc4's own real zombie spawn point (server_spawn_npcs' roles[4] == PC_NPC_ROLE_ZOMBIE, on the
 * same 10-unit circle at angle 4*(2pi/8) == (-10, 0, 0)) that the eat loop below can be verified
 * live without waiting on any real movement AI -- neither bugs nor zombies move in this v0
 * (matches server_spawn_npcs' own "stationary in v0, on purpose"). */
static void server_spawn_giant_bugs(unsigned int now_ms) {
    memset(g_giant_bugs, 0, sizeof(g_giant_bugs));
    ServerGiantBug *b = &g_giant_bugs[0];
    b->active = 1;
    b->x = -9.0f; b->y = 0.0f; b->z = 1.0f; /* within BIGO_GIANT_BUG_EAT_RADIUS of npc4's (-10,0,0) */
    giant_bug_state_init(&b->bstate, now_ms);
    printf("S536-BUG: spawned 1 real giant zombie bug at (%.1f, %.1f, %.1f).\n", b->x, b->y, b->z);
}

/* server_giant_bug_command_authorized -- "men are the custodians of the keys for the giant
 * zombie feral ai bugs" (founder real-time, 2026-09-22, via SHANKPIT's own live wiring --
 * witness_ai_bug_command_authorized). A spawned bug only hunts/eats while >=1 live The Men NPC is
 * active; with none active it just sits DORMANT-equivalent (hunger still drifts via giant_bug_
 * tick, attack/eat never fires). TRAPX Rogue Swarm Doctrine is real, named, and deliberately NOT
 * modeled further here -- GTA7's own separate faction-doctrine system, a real, separate follow-up. */
static int server_giant_bug_command_authorized(void) {
    for (int i = 0; i < PC_NPC_MAX; i++) {
        if (g_npcs[i].active && g_npcs[i].role == PC_NPC_ROLE_THE_MEN) return 1;
    }
    return 0;
}

/* server_tick_giant_bugs -- SECTION 536 reverse-port phase 3, real live wiring (brought back from
 * SHANKPIT's own witness_ai.c tick loop, same eat-radius/authorization logic). Real, honest,
 * deliberately NOT built here (named, not silently dropped): no bug movement (matches zombies'
 * own "stationary in v0" precedent above), no network broadcast/client visual (server-side
 * simulation only -- verified via this function's own real log line, same "prove it live in the
 * log" precedent server_tick_witness already established), no eaten-zombie despawn broadcast to
 * connected clients (a real, separate gap once bugs ever get a network presence at all). */
static void server_tick_giant_bugs(unsigned int now_ms) {
    int authorized = server_giant_bug_command_authorized();
    int live_bugs = 0;
    for (int i = 0; i < BIGO_GIANT_BUG_MAX; i++) if (g_giant_bugs[i].active) live_bugs++;

    for (int i = 0; i < BIGO_GIANT_BUG_MAX; i++) {
        ServerGiantBug *bug = &g_giant_bugs[i];
        if (!bug->active) continue;

        int has_target = 0;
        int eaten_ni = -1;
        if (authorized) {
            for (int ni = 0; ni < PC_NPC_MAX; ni++) {
                ServerNpc *n = &g_npcs[ni];
                if (!n->active || n->role != PC_NPC_ROLE_ZOMBIE) continue;
                float dx = n->x - bug->x, dy = n->y - bug->y, dz = n->z - bug->z;
                if (dx * dx + dy * dy + dz * dz <= BIGO_GIANT_BUG_EAT_RADIUS * BIGO_GIANT_BUG_EAT_RADIUS) {
                    has_target = 1;
                    eaten_ni = ni;
                    break;
                }
            }
        }

        giant_bug_tick(&bug->bstate, now_ms, has_target, live_bugs - 1);

        if (eaten_ni >= 0) {
            ServerNpc *prey = &g_npcs[eaten_ni];
            giant_bug_eat_zombie(&bug->bstate, &prey->zombie, now_ms);
            printf("S536-BUG: bug%d ate npc%d (zombie) -- strength=%.2f speed=%.2f\n",
                   i, eaten_ni, bug->bstate.strength, bug->bstate.speed);
            prey->active = 0;
        }
    }
}

static const char *WS_NAMES[] = {"UNAWARE", "DENIAL", "COMPROMISED", "SILENCING", "PANIC", "ENGAGE"};

/* server_tick_witness -- S504-DISPATCH, closes NORTHSTAR.md §8e item 1's LOUD-event half: every
 * HUNTING/FRENZIED zombie (witness_live.h's own bigo_zombie_is_witnessable_event) is now a real
 * witnessed event for every Citizen/The Men NPC within BIGO_WITNESS_DETECTION_RADIUS, driving
 * their witness_state through core/witness_rules.c's own real npc_next_state -- exactly
 * core/sim.c's own sim_release semantics (count computed once per event, shared across every
 * human who witnessed it), just radius-based instead of zone-based since this live server has no
 * per-NPC zone concept yet. The QUIET-observation path (costume/gear noticing) is a real,
 * separate, still-open gap -- not touched here, see witness_live.h's own top doc comment. */
static void server_tick_witness(void) {
    for (int zi = 0; zi < PC_NPC_MAX; zi++) {
        ServerNpc *zn = &g_npcs[zi];
        if (!zn->active || zn->role != PC_NPC_ROLE_ZOMBIE) continue;
        if (!bigo_zombie_is_witnessable_event(zn->zombie.mood)) continue;

        int total = 0;
        for (int hi = 0; hi < PC_NPC_MAX; hi++) {
            ServerNpc *hn = &g_npcs[hi];
            if (!hn->active || hn->role == PC_NPC_ROLE_ZOMBIE) continue;
            if (bigo_in_range(hn->x, hn->z, zn->x, zn->z, BIGO_WITNESS_DETECTION_RADIUS)) total++;
        }
        if (total == 0) continue;
        int count = effective_witnesses(total, 0); /* no live accomplice concept yet -- real, named deferral */

        for (int hi = 0; hi < PC_NPC_MAX; hi++) {
            ServerNpc *hn = &g_npcs[hi];
            if (!hn->active || hn->role == PC_NPC_ROLE_ZOMBIE) continue;
            if (!bigo_in_range(hn->x, hn->z, zn->x, zn->z, BIGO_WITNESS_DETECTION_RADIUS)) continue;
            int prev = hn->witness_state;
            int nx = bigo_witness_next_state_for_event(prev, count, hn->arrogance, 0);
            if (!is_legal_transition(prev, nx)) continue;
            if (nx != prev) {
                printf("S504-DISPATCH: npc%d witness_state %s -> %s (zombie%d event, count=%d)\n",
                       hi, WS_NAMES[prev], WS_NAMES[nx], zi, count);
            }
            hn->witness_state = nx;
        }
    }

    /* LOS-loss/elimination resolution -- EMILY/BACKLOG.md SECTION 536 follow-up,
     * BIG_O/NORTHSTAR.md §11 item 1, closed. Real, live-found gap: the escalation loop above only
     * ever raises witness_state, never lowers it -- once a human reaches SILENCING/ENGAGE, the
     * only way out was The Men's own dispatch loop resolving to DENIAL (resolved=1). If the
     * underlying zombie itself stops being a witnessable event (mood decays back below HUNTING/
     * FRENZIED, moves out of range, or is despawned/eaten by a giant bug) while no Man has arrived
     * yet, the hunt persisted forever with nothing left to witness -- core/sim.c's own
     * sim_eliminate (resolved=2, "target eliminated/gone") already models exactly this second
     * resolution path in the offline scenario harness; this closes the same gap live. Generalized
     * here to "nothing left to witness" rather than literally killed -- the zombie may still be
     * alive, just no longer a loud event in range. A responder already en route stands down
     * naturally on its own next tick (server_tick_dispatch's own existing "target resolved some
     * other way" check, unchanged). */
    for (int hi = 0; hi < PC_NPC_MAX; hi++) {
        ServerNpc *hn = &g_npcs[hi];
        if (!hn->active || hn->role == PC_NPC_ROLE_ZOMBIE) continue;
        if (hn->witness_state != WS_SILENCING && hn->witness_state != WS_ENGAGE) continue;

        int still_witnessable = 0;
        for (int zi = 0; zi < PC_NPC_MAX; zi++) {
            ServerNpc *zn = &g_npcs[zi];
            if (!zn->active || zn->role != PC_NPC_ROLE_ZOMBIE) continue;
            if (!bigo_zombie_is_witnessable_event(zn->zombie.mood)) continue;
            if (bigo_in_range(hn->x, hn->z, zn->x, zn->z, BIGO_WITNESS_DETECTION_RADIUS)) { still_witnessable = 1; break; }
        }
        if (still_witnessable) continue;

        int prev = hn->witness_state;
        int nx = bigo_witness_next_state_for_event(prev, 0, hn->arrogance, 2 /* resolved: nothing left to witness */);
        if (is_legal_transition(prev, nx) && nx != prev) {
            hn->witness_state = nx;
            printf("S536-WITNESS: npc%d witness_state %s -> %s (no witnessable zombie left in range)\n",
                   hi, WS_NAMES[prev], WS_NAMES[nx]);
        }
    }
}

#define THE_MEN_DISPATCH_SPEED 6.0f /* units/sec -- The Men's own real response pace, tuned
    separately from PHEROMONE_ZOMBIE_SPEED on purpose (they're professionals responding to a
    call, not a commanded predator closing in) */
#define BIGO_PAGER_LATENCY_MS 3000u /* NORTHSTAR.md §11 item 6: "message sent -> Man notices the
    buzz -> responds" -- a real, deliberately short v1 delay (the same rough order of magnitude
    as BIGO_DISTRACTION_MS's own 8000ms) before an assigned Man actually starts moving toward a
    hunt, replacing the old instant assignment-equals-movement behavior */

/* server_tick_dispatch -- S504-DISPATCH: The Men's own real dispatch/sanitize decision loop
 * (NORTHSTAR.md §8e item 3). Any Citizen/The Men NPC currently SILENCING/ENGAGE is an active hunt
 * needing a response; an idle The Men NPC (no current assignment) is dispatched to the NEAREST
 * one. SECTION 536 follow-up (NORTHSTAR.md §11 item 6, "the men carry pagers"): assignment no
 * longer means instant movement -- a real BIGO_PAGER_LATENCY_MS buzz delay stands in for "message
 * sent -> Man notices the buzz -> responds" before the Man starts travelling (bigo_pheromone.h's
 * own pheromone_step_toward, reused verbatim -- steering toward a point is steering toward a
 * point, human or zombie). On arrival, resolves the hunt (resolved=1 -> DENIAL, docs/
 * DESIGN_DIGEST.md §11's own "memory-wipe spray"). A hunt that resolves some OTHER way first (or
 * whose target NPC goes inactive), including mid-buzz before the Man ever moves, makes its
 * responder stand down instead of arriving to nothing. */
static void server_tick_dispatch(unsigned int now_ms) {
    static unsigned int last_dispatch_tick_ms = 0;
    float dt_sec = (last_dispatch_tick_ms == 0) ? 0.0f : (float)(now_ms - last_dispatch_tick_ms) / 1000.0f;
    if (dt_sec > 0.5f) dt_sec = 0.5f; /* clamp a stall/hitch, matching server_tick_npcs's own dt clamp */
    last_dispatch_tick_ms = now_ms;

    for (int hi = 0; hi < PC_NPC_MAX; hi++) {
        ServerNpc *hn = &g_npcs[hi];
        if (!hn->active || hn->role == PC_NPC_ROLE_ZOMBIE) continue;
        if (hn->witness_state != WS_SILENCING && hn->witness_state != WS_ENGAGE) continue;

        int already_assigned = 0;
        for (int mi = 0; mi < PC_NPC_MAX; mi++) {
            ServerNpc *mn = &g_npcs[mi];
            if (mn->active && mn->role == PC_NPC_ROLE_THE_MEN && mn->has_dispatch_target &&
                mn->dispatch_target_npc == hi) {
                already_assigned = 1;
                break;
            }
        }
        if (already_assigned) continue;

        int responder = -1;
        float best_d2 = 0.0f;
        for (int mi = 0; mi < PC_NPC_MAX; mi++) {
            ServerNpc *mn = &g_npcs[mi];
            if (!mn->active || mn->role != PC_NPC_ROLE_THE_MEN || mn->has_dispatch_target) continue;
            float dx = mn->x - hn->x, dz = mn->z - hn->z;
            float d2 = dx * dx + dz * dz;
            if (responder == -1 || d2 < best_d2) { responder = mi; best_d2 = d2; }
        }
        if (responder == -1) continue; /* every The Men unit already busy -- real, honest v0 cap,
            NORTHSTAR.md §11's own "Corporate Service Call" escalation to Regulators at max heat
            is the real future answer to this, not attempted here */

        g_npcs[responder].has_dispatch_target = 1;
        g_npcs[responder].dispatch_target_npc = hi;
        g_npcs[responder].pager_buzz_until_ms = now_ms + BIGO_PAGER_LATENCY_MS;
        printf("S536-PAGER: The Men npc%d's pager buzzes -- dispatched to npc%d's hunt (%s), "
               "responding in %ums\n", responder, hi, WS_NAMES[hn->witness_state], BIGO_PAGER_LATENCY_MS);
    }

    for (int mi = 0; mi < PC_NPC_MAX; mi++) {
        ServerNpc *mn = &g_npcs[mi];
        if (!mn->active || mn->role != PC_NPC_ROLE_THE_MEN || !mn->has_dispatch_target) continue;
        ServerNpc *target = &g_npcs[mn->dispatch_target_npc];
        if (!target->active || (target->witness_state != WS_SILENCING && target->witness_state != WS_ENGAGE)) {
            mn->has_dispatch_target = 0; /* resolved some other way, or target gone -- stand down */
            mn->pager_buzz_until_ms = 0;
            continue;
        }
        if (mn->pager_buzz_until_ms != 0) {
            if (now_ms < mn->pager_buzz_until_ms) continue; /* still buzzing -- not moving yet */
            mn->pager_buzz_until_ms = 0; /* latency elapsed -- Man is now actively responding */
            printf("S536-PAGER: The Men npc%d done buzzing, now responding to npc%d\n",
                   mi, mn->dispatch_target_npc);
        }
        if (dt_sec > 0.0f) {
            pheromone_step_toward(&mn->x, &mn->z, target->x, target->z, THE_MEN_DISPATCH_SPEED, dt_sec);
            mn->yaw = atan2f(target->x - mn->x, target->z - mn->z);
        }
        if (bigo_in_range(mn->x, mn->z, target->x, target->z, BIGO_DISPATCH_ARRIVAL_RADIUS)) {
            int prev = target->witness_state;
            int nx = npc_next_state(prev, 0, target->arrogance, 0, 1, 1 /* resolved: memory wipe */);
            if (is_legal_transition(prev, nx)) {
                target->witness_state = nx;
                printf("S504-DISPATCH: The Men npc%d resolved npc%d's hunt: %s -> %s (memory wipe)\n",
                       mi, mn->dispatch_target_npc, WS_NAMES[prev], WS_NAMES[nx]);
            }
            mn->has_dispatch_target = 0;
        }
    }
}

/* Real PARENA-compiled progression decisions (packages/simulation/level_mod.c) -- wiring the
   already-tested "mods first everything" leveling logic into the actual live game loop for the
   first time, matching NORTHSTAR.md's own "keep the experience gain from the construct" note. */
int on_papercraft_level_for_xp(int level, int total_xp);
int xp_required_for_level(int level);
int on_papercraft_can_allocate_talent(int ability_value, int unspent_points);
int on_papercraft_move_speed_boost_permille(int move_rank);
int on_papercraft_slide_jump_boost_permille(int speed_milli);
int on_papercraft_xp_for_object_destroyed(void);
int on_papercraft_phone_message_for_event(int event_type);
int on_papercraft_item_for_object_destroyed(int material, int object_index);
int on_papercraft_inventory_stack_max(int item_id);
int on_papercraft_inventory_can_stack(int existing_item_id, int incoming_item_id);
int on_papercraft_pickup_radius_millis(void);
/* Real PARENA-compiled "arsenal" weapon decisions (packages/simulation/weapon_mod.c) -- see
   PcWeaponSwitchPacket/PcWeaponOwnedPacket's own doc comments in papercraft_protocol.h for the
   full real design (2026-09-07, founder real-time: "aresnal (weapon switching)... not all
   characters get all aresenals you have to find a [shotgun] etc"). */
int on_papercraft_weapon_item_slot(int item_id);
int on_papercraft_weapon_switch_allowed(int owned_mask, int requested_slot);

/* I32Fn0 -- real function-pointer shape for a dynamically-loaded, zero-arg I32-returning mod
   function, same real shape apps/dynmod_poc's own I32Fn0 already proved dlopen/dlsym-compatible.
   Used by the real call site below to invoke a mod resolved out of g_mod_registry. */
typedef int (*I32Fn0)(void);

#define PC_XP_TICK_MS   1000 /* real 1-second cadence, matches SHANKPIT_CONSTRUCT.txt's own progression_tick */
#define PC_XP_PER_TICK  5    /* matches the construct's own real progression_add_xp(5) passive rate */
#define PC_AUTOSAVE_MS  10000 /* real periodic per-player save cadence -- 10s, real, bounded worst-case
                                  data loss on a crash (not a clean shutdown -- that saves everyone
                                  immediately, see g_shutdown_requested below), not tuned against any
                                  real production load yet */

static PlayerSlot g_slots[PC_MAX_PLAYERS];
static char g_save_dir[256] = "var/players";

/* server_wheelbarrow_toggle -- SECTION 536 reverse-port phase 4's own real toggle decision,
 * factored out of the PC_PACKET_WHEELBARROW_TOGGLE handler so it's independently callable/
 * testable without a live socket, same "extract the real decision, host code just wires it"
 * discipline server_throw_pheromone already established. Drop if requester is already carrying,
 * else pick up the nearest carryable (Citizen/Zombie only, matching the founder's own "whole
 * zombie or citizen" wording -- The Men are never carryable) NPC within
 * BIGO_WHEELBARROW_PICKUP_RADIUS of requester's own real position. */
static void server_wheelbarrow_toggle(int requester) {
    if (requester < 0 || requester >= PC_MAX_PLAYERS || !g_slots[requester].active) return;

    if (g_carried_npc_index >= 0 && g_carrier_slot == requester) {
        printf("S536-WHEELBARROW: player%d dropped npc%d\n", requester, g_carried_npc_index);
        g_carried_npc_index = -1;
        g_carrier_slot = -1;
        return;
    }
    if (g_carried_npc_index >= 0) return; /* something else is already being carried */

    PlayerSlot *rs = &g_slots[requester];
    int best = -1;
    float best_d2 = BIGO_WHEELBARROW_PICKUP_RADIUS * BIGO_WHEELBARROW_PICKUP_RADIUS;
    for (int ni = 0; ni < PC_NPC_MAX; ni++) {
        ServerNpc *n = &g_npcs[ni];
        if (!n->active || (n->role != PC_NPC_ROLE_CITIZEN && n->role != PC_NPC_ROLE_ZOMBIE)) continue;
        float dx = n->x - rs->state.x, dy = n->y - rs->state.y, dz = n->z - rs->state.z;
        float d2 = dx * dx + dy * dy + dz * dz;
        if (d2 <= best_d2) { best = ni; best_d2 = d2; }
    }
    if (best >= 0) {
        g_carried_npc_index = best;
        g_carrier_slot = requester;
        printf("S536-WHEELBARROW: player%d picked up npc%d (role=%d)\n", requester, best, g_npcs[best].role);
    }
}

/* server_tick_wheelbarrow -- SECTION 536 reverse-port phase 4. Trails the carried NPC just behind
 * its carrier every tick (same real "pinned to the carrier" shape SHANKPIT's own version uses,
 * BIG_O's state.yaw is already radians -- see PC_PACKET_INTERACT's own sinf/cosf(state.yaw), no
 * degree conversion needed here either), then checks delivery into BIGO_LAB_ZONE_*. A carrier who
 * disconnects, or cargo that goes inactive some other way (e.g. eaten by a giant bug -- a real,
 * live possibility now that both mechanics touch the same g_npcs[] array), makes this stand down
 * safely rather than trailing a stale reference. */
static void server_tick_wheelbarrow(void) {
    if (g_carried_npc_index < 0) return;
    ServerNpc *cargo = &g_npcs[g_carried_npc_index];
    if (g_carrier_slot < 0 || !g_slots[g_carrier_slot].active || !cargo->active) {
        g_carried_npc_index = -1;
        g_carrier_slot = -1;
        return;
    }

    PlayerSlot *carrier = &g_slots[g_carrier_slot];
    cargo->x = carrier->state.x - sinf(carrier->state.yaw) * 2.0f;
    cargo->z = carrier->state.z + cosf(carrier->state.yaw) * 2.0f;
    cargo->y = carrier->state.y;

    float dx = cargo->x - BIGO_LAB_ZONE_CX, dz = cargo->z - BIGO_LAB_ZONE_CZ;
    if (dx * dx + dz * dz <= BIGO_LAB_ZONE_RADIUS * BIGO_LAB_ZONE_RADIUS) {
        g_lab_deliveries++;
        printf("S536-WHEELBARROW: player%d delivered npc%d to the lab (total=%d)\n",
               g_carrier_slot, g_carried_npc_index, g_lab_deliveries);
        cargo->active = 0;
        g_carried_npc_index = -1;
        g_carrier_slot = -1;
    }
}

static const char *BAND_NAMES[] = {"OK", "SUSPICION", "HYSTERIC", "CANCELLED"};

/* Regulator dispatch + real player kill/respawn -- EMILY/BACKLOG.md SECTION 536 follow-up,
 * BIG_O/NORTHSTAR.md §18 Phase B. Founder, real-time, on what CANCELLED actually does: "the
 * regulators are called in - the uberplumbers and they delete you with acid and foam" -- checked
 * against and confirmed matching docs/DESIGN_DIGEST.md §11's own existing canon (a Corporate
 * Service Call of lethal Regulators at max heat, plus a somatic-clone respawn). This is BIG_O's
 * first player damage/death mechanic of any kind (the real, named blocker phase 5's eat-to-heal
 * ran into) -- resolved here by giving Decorum, not food, the first real consequence.
 *
 * Real, honest v1 scope, named: a SEPARATE array from g_npcs (same "growing PC_NPC_MAX is a real
 * wire-protocol change" precedent g_giant_bugs[] already established) -- Regulators are NOT
 * broadcast in any snapshot yet, so they are real and live server-side but invisible to clients,
 * same "server logic first, client visual later" precedent every phase in this reverse-port
 * thread has used. No Bio-Slurry charge on respawn -- that real economy does not exist anywhere
 * in this repo yet (no earning mechanism designed), so the clone-restore is currently free,
 * logged as an honest gap rather than invented on the spot. */
#define BIGO_REGULATOR_MAX PC_MAX_PLAYERS /* one real, sensible upper bound -- at most one active
    hunt per connected player (NORTHSTAR.md §7's own co-op cap) */
#define BIGO_REGULATOR_SPEED 9.0f /* units/sec -- real, deliberately faster than
    THE_MEN_DISPATCH_SPEED (6.0), matching "silent, John-Wick-lethal" (docs/DESIGN_DIGEST.md §11)
    versus The Men's own "professionals responding to a call" pace */
#define BIGO_REGULATOR_KILL_RADIUS 2.5f /* same value as BIGO_DISPATCH_ARRIVAL_RADIUS, own named
    constant per this repo's own "each mechanic gets its own named radius" convention */
/* Real, arbitrary v1 dispatch origin -- no "corporate district" landmark exists in this world
   yet, same honest placement precedent g_giant_bugs[]'s own spawn point already set. Well clear
   of the NPC spawn circle (~10 units around origin), the giant-bug spawn (-9,0,1), and the lab
   zone (30,0) itself. */
#define BIGO_REGULATOR_DISPATCH_X 50.0f
#define BIGO_REGULATOR_DISPATCH_Z -20.0f

typedef struct {
    int active;
    float x, y, z;
    int target_slot; /* -1 = no target, else a real PC_MAX_PLAYERS index being hunted */
} ServerRegulator;
static ServerRegulator g_regulators[BIGO_REGULATOR_MAX];

/* server_dispatch_regulator -- called once, exactly when a player's Decorum first crosses into
 * BAND_CANCELLED (server_tick_decorum's own one-time marker below). A no-op if that player
 * already has an active hunt (real, honest guard against a double-dispatch if decorum somehow
 * re-triggers CANCELLED before the first Regulator arrives). */
static void server_dispatch_regulator(int target_slot) {
    for (int i = 0; i < BIGO_REGULATOR_MAX; i++) {
        if (g_regulators[i].active && g_regulators[i].target_slot == target_slot) return;
    }
    for (int i = 0; i < BIGO_REGULATOR_MAX; i++) {
        if (g_regulators[i].active) continue;
        g_regulators[i].active = 1;
        g_regulators[i].x = BIGO_REGULATOR_DISPATCH_X;
        g_regulators[i].y = 0.0f;
        g_regulators[i].z = BIGO_REGULATOR_DISPATCH_Z;
        g_regulators[i].target_slot = target_slot;
        printf("S536-REGULATOR: regulator%d dispatched from (%.1f,%.1f) after player%d\n",
               i, BIGO_REGULATOR_DISPATCH_X, BIGO_REGULATOR_DISPATCH_Z, target_slot);
        return;
    }
    printf("S536-REGULATOR: dispatch suppressed -- BIGO_REGULATOR_MAX (%d) already full\n", BIGO_REGULATOR_MAX);
}

/* server_kill_player -- the real, first-ever player death in this repo. Respawns at the real lab
 * zone (docs/DESIGN_DIGEST.md §11's own "the basement prints a new body"), same ground-height
 * lookup precedent spawn_player's own default-spawn branch already uses. Decorum resets to a
 * clean decorum_start() -- a real, deliberate "new body, clean slate" choice, matching the lore
 * rather than leaving a killed player's Decorum wherever it was (which would just re-trigger
 * CANCELLED and an immediate re-dispatch, a real loop this choice avoids). Position/XP/inventory
 * are NOT reset (real, honest v1 scope -- "what a kill resets" beyond Decorum itself is real,
 * separate, undecided design space, not guessed at here; NORTHSTAR.md §18 names it). */
static void server_kill_player(int slot) {
    PlayerSlot *s = &g_slots[slot];
    int ground_y;
    if (pw_world_ground_height_at(&g_world, (int)BIGO_LAB_ZONE_CX, (int)BIGO_LAB_ZONE_CZ, &ground_y)) {
        s->state.y = (float)ground_y;
    } else {
        s->state.y = 0.0f;
    }
    s->state.x = BIGO_LAB_ZONE_CX;
    s->state.z = BIGO_LAB_ZONE_CZ;
    s->state.decorum = decorum_start();
    s->decorum_zone = -1; /* forces a real re-observe on the next tick, same as a fresh spawn */
    s->decorum_cancelled_logged = 0;
    printf("S536-REGULATOR: player%d DELETED (acid and foam) -- respawned at the lab (%.1f,%.1f,%.1f), decorum reset to %d. Bio-Slurry cost NOT charged (economy not built yet, BIG_O/NORTHSTAR.md §18 Phase B).\n",
           slot, s->state.x, s->state.y, s->state.z, s->state.decorum);
}

/* server_tick_regulators -- chases each active Regulator's own live target position (the
 * player's current x/z, not a fixed point -- same "steering toward a point" reuse of
 * pheromone_step_toward The Men's own dispatch loop already established). Stands down safely if
 * the target disconnects, or if the target's own Decorum recovers back out of BAND_CANCELLED
 * before arrival (a real, deliberate mercy -- matches The Men's own "resolved some other way,
 * stand down" precedent for zombie hunts). */
static void server_tick_regulators(unsigned int now_ms) {
    static unsigned int last_tick_ms = 0;
    float dt_sec = (last_tick_ms == 0) ? 0.0f : (float)(now_ms - last_tick_ms) / 1000.0f;
    if (dt_sec > 0.5f) dt_sec = 0.5f;
    last_tick_ms = now_ms;

    for (int i = 0; i < BIGO_REGULATOR_MAX; i++) {
        ServerRegulator *r = &g_regulators[i];
        if (!r->active) continue;
        PlayerSlot *target = &g_slots[r->target_slot];
        if (!target->active || decorum_band(target->state.decorum) != BAND_CANCELLED) {
            printf("S536-REGULATOR: regulator%d stands down -- player%d no longer a valid target\n", i, r->target_slot);
            r->active = 0;
            continue;
        }
        if (dt_sec > 0.0f) {
            pheromone_step_toward(&r->x, &r->z, target->state.x, target->state.z, BIGO_REGULATOR_SPEED, dt_sec);
        }
        if (bigo_in_range(r->x, r->z, target->state.x, target->state.z, BIGO_REGULATOR_KILL_RADIUS)) {
            server_kill_player(r->target_slot);
            r->active = 0;
        }
    }
}

/* server_tick_decorum -- EMILY/BACKLOG.md SECTION 536 follow-up, BIG_O/NORTHSTAR.md §18 Phase A:
 * the QUIET-observation half of the witness system, live for the first time. Real, deliberate
 * design choices, all named in NORTHSTAR.md §18: fires the real "observe" check once per
 * zone-ENTRY transition (matching core/sim.c's own sim_enter-drives-sim_observe precedent, not a
 * continuous per-tick re-roll, which would crash Decorum in under a second at 20Hz); gear/token
 * are real, honest 0s (no live field-gear-carry flag or vault-token mechanic exists yet, so only
 * DA_WRONG_COSTUME can ever fire from this pass); witnesses are real, active Citizen/The-Men NPCs
 * within BIGO_QUIET_OBSERVE_RADIUS, using each one's own real npc_brain_effective_vigilance. */
static void server_tick_decorum(unsigned int now_ms) {
    for (int i = 0; i < PC_MAX_PLAYERS; i++) {
        PlayerSlot *s = &g_slots[i];
        if (!s->active) continue;

        int zone = server_player_zone(s);
        if (zone != s->decorum_zone) {
            s->decorum_zone = zone;
            int allowed = zone_access(s->state.costume, zone, 0 /* no live vault-token mechanic yet */);
            int cons = conspicuousness(allowed, 0 /* no live field-gear-carry flag yet */);
            if (cons > 0) {
                int seen = 0;
                for (int ni = 0; ni < PC_NPC_MAX; ni++) {
                    ServerNpc *n = &g_npcs[ni];
                    if (!n->active || n->role == PC_NPC_ROLE_ZOMBIE) continue;
                    if (!bigo_in_range(n->x, n->z, s->state.x, s->state.z, BIGO_QUIET_OBSERVE_RADIUS)) continue;
                    int vig = npc_brain_effective_vigilance(&n->brain);
                    if (server_distraction_active(now_ms)) vig /= 2; /* cake-smash, see server_smash_cake's own doc comment */
                    if (noticed(vig, cons, server_roll100())) seen++;
                }
                if (seen > 0) {
                    int before = s->state.decorum;
                    s->state.decorum = decorum_after(before, DA_WRONG_COSTUME);
                    int band = decorum_band(s->state.decorum);
                    printf("S536-DECORUM: player%d noticed in zone%d (wrong costume, seen_by=%d) decorum %d -> %d (%s)\n",
                           i, zone, seen, before, s->state.decorum, BAND_NAMES[band]);
                    if (band == BAND_CANCELLED && !s->decorum_cancelled_logged) {
                        s->decorum_cancelled_logged = 1;
                        printf("S536-DECORUM: player%d CANCELLED -- dispatching a Regulator (BIG_O/NORTHSTAR.md §18 Phase B)\n", i);
                        server_dispatch_regulator(i);
                    } else if (band != BAND_CANCELLED) {
                        s->decorum_cancelled_logged = 0;
                    }
                }
            }
        }

        if (now_ms - s->last_quiet_tick_ms >= BIGO_DECORUM_QUIET_TICK_MS) {
            s->last_quiet_tick_ms = now_ms;
            if (decorum_band(s->state.decorum) != BAND_CANCELLED) {
                s->state.decorum = decorum_after(s->state.decorum, DA_QUIET_TICK);
            }
        }
    }
}

/* g_shutdown_requested: set by a real SIGINT/SIGTERM handler -- lets a deliberate server restart
   (not just a crash) flush every real active player's own current state to disk before exiting,
   the real reason "persistence across a restart" needs more than just the periodic autosave
   above. Handler body is signal-safe (a single sig_atomic_t write only); the actual real save
   work happens in the main loop, not inside the handler. */
static volatile sig_atomic_t g_shutdown_requested = 0;
static void handle_shutdown_signal(int sig) {
    (void)sig;
    g_shutdown_requested = 1;
}

/* g_mods_reload_requested: real SIGHUP handler, same signal-safe "set a flag, do the actual work
   in the main loop" discipline as g_shutdown_requested above -- closes MODDING.md's own
   honestly-named "No live-server reload" gap for the one real piece of live state that's actually
   SAFE to reload without a restart: the dynamically-loaded mods manifest. World-object edits
   (apps/mapeditor) still need a real restart -- that reload is a real, separate, harder problem
   (existing per-object state is keyed by array INDEX everywhere -- g_wo_mesh[i],
   g_wo_destroyed_awarded[i], a connected player's own current interact target -- and a map edit
   that changes the real object count or ordering would silently desync all of that; not attempted
   here). The mods manifest has no such problem: g_mod_registry is keyed by function NAME, not
   slot index, so dropping every real registration and re-running load_mods_manifest from scratch
   is always safe, and this server is single-threaded with no reentrancy -- the actual reload work
   only ever runs between ticks in the main loop, never while a real gameplay call site (see
   on_papercraft_xp_for_object_destroyed's own real call site) is mid-call. */
static volatile sig_atomic_t g_mods_reload_requested = 0;
static void handle_reload_signal(int sig) {
    (void)sig;
    g_mods_reload_requested = 1;
}

/* save_player: writes one real player's own current progression + position to disk, matching the
   real PcSaveRecord shape packages/common/papercraft_persist.h defines. A no-op for a slot that
   never carried a real player_id (shouldn't happen in practice -- every active slot gets one on
   CONNECT -- but a real, cheap guard against saving garbage). Logs, doesn't crash, on a real save
   failure -- a lost autosave tick is recoverable data loss, not a fatal server error. */
static void save_player(const PlayerSlot *s) {
    if (!s->has_player_id) return;
    PcSaveRecord rec;
    rec.magic = PC_SAVE_MAGIC;
    rec.x = s->state.x; rec.y = s->state.y; rec.z = s->state.z; rec.yaw = s->state.yaw;
    rec.level = s->state.level;
    rec.xp = s->state.xp;
    rec.unspent_points = s->state.unspent_points;
    for (int i = 0; i < PC_ABILITY_COUNT; i++) rec.ability[i] = s->state.ability[i];
    if (!pc_persist_save(g_save_dir, s->player_id, &rec)) {
        fprintf(stderr, "WARNING: save_player failed for a real active player -- disk full/permissions?\n");
    }
}

/* save_world_damage: writes every real active world object's own current per-fragment hp to
   disk -- the real gameplay-state counterpart to apps/mapeditor's own real, editor-authored
   PcWorldObjectFile. Called on the same real periodic-autosave + graceful-shutdown cadence
   save_player already uses. */
static void save_world_damage(void) {
    PcWorldDamageFile damage;
    memset(&damage, 0, sizeof(damage));
    damage.magic = PC_WD_MAGIC;
    for (int o = 0; o < g_wo_file.count; o++) {
        for (int f = 0; f < g_wo_mesh[o].fragment_count && f < PC_WO_FRAGMENTS; f++) {
            damage.hp[o][f] = g_wo_mesh[o].fragments[f].hp;
        }
    }
    if (!pc_worldobjects_save_damage(g_world_damage_path, &damage)) {
        fprintf(stderr, "WARNING: save_world_damage failed -- disk full/permissions?\n");
    }
}

/* Real, minimal dynamic mod registry -- apps/server's own real, production-side proof that the
   apps/dynmod_poc mechanism (dlopen/dlsym against an unmodified real PARENA-compiled .so) can be
   wired into the real game server, not just a standalone tool. Real call-site policy, decided
   here for the first time (closes MODDING.md's own "an actual apps/server call site" gap): a real
   call site looks a mod function up in this registry by name and calls it IF a real mod
   dynamically registered under that exact name; otherwise it falls back to the same real,
   statically-linked function this repo has always called -- so a mod that never loaded (the
   common case whenever --mods-manifest is unset, or that one specific mod failed) degrades to
   today's exact, unchanged behavior, never to broken/missing gameplay. See
   on_papercraft_xp_for_object_destroyed's own real call site for the first one wired this way.
   Loaded once at startup, only if
   --mods-manifest names a real file. Real, deliberately different manifest format from
   apps/dynmod_poc's own test-oriented one: just `so_path|function-name` per line (blank/#-prefixed
   lines skipped) -- a real running server has no "expected value" to self-check against the way a
   proof-of-concept tool does, it just needs to resolve and hold each real function pointer.

   Real, considered error-handling policy, decided here for the first time (closes MODDING.md's own
   "no designed error-handling policy for a bad/missing mod at real server startup" gap): a mod
   that fails to load (missing .so, missing symbol, or a malformed manifest line) is logged as a
   WARNING and skipped -- it never prevents the server from starting, and never affects any other
   mod in the same manifest. Rationale: dynamically-loaded mods are optional gameplay layers on top
   of the same statically-linked mod logic that already runs the game
   (packages/simulation:xp_award etc. stay linked in, unchanged) -- a broken mod file must not be
   able to take the whole persistent, always-running server down. A missing --mods-manifest file
   itself is the same real, non-fatal case: "zero mods loaded", not an error, since a fresh
   checkout before any real mod author has written one is the common real state. */
/* version (kanban priority-queue card 43432, "papercraft mod registry make it like npm but mo
   betta," Phase A per NORTHSTAR.md's own "A real mod registry" section): a real, small,
   backward-compatible step toward "which version of this mod is this" being answerable at all --
   today a mod is just an unversioned .so at a path. Optional third pipe field on a manifest
   line (so_path|function|version); an omitted field (every manifest written before this pass)
   defaults to PC_MOD_VERSION_UNVERSIONED, so no existing manifest needs editing. Not yet used
   for anything beyond being recorded and reported -- no compatibility checking, no resolution,
   both real, separate, later phases (B/C in NORTHSTAR.md) this alone doesn't attempt. */
#define PC_MOD_VERSION_UNVERSIONED "0.0.0-unversioned"
#define PC_MOD_REGISTRY_MAX 16
typedef struct {
    char name[64];
    char version[32];
    void *fn;
} PcModRegistryEntry;
static PcModRegistryEntry g_mod_registry[PC_MOD_REGISTRY_MAX];
static int g_mod_registry_count = 0;

/* mod_registry_lookup: real, linear lookup by function name (PC_MOD_REGISTRY_MAX is small --
   16 -- a linear scan is the real, appropriately-simple choice, not a premature hash table).
   Returns NULL if no mod named this function loaded successfully (the common case when
   --mods-manifest wasn't given at all, or that specific mod failed to load) -- every real call
   site using this is required to have a real, statically-linked fallback for exactly that case,
   see on_papercraft_xp_for_object_destroyed's own real call site below for the first one. */
static void *mod_registry_lookup(const char *name) {
    for (int i = 0; i < g_mod_registry_count; i++) {
        if (strcmp(g_mod_registry[i].name, name) == 0) return g_mod_registry[i].fn;
    }
    return NULL;
}

/* Real handle cache, same dlopen-once-per-distinct-path pattern apps/dynmod_poc's own manifest
   mode already proved (S206-27) -- two manifest lines naming the same .so share one real loaded
   instance rather than dlopen()ing it twice. */
#define PC_MOD_LIB_MAX 16
typedef struct {
    char path[256];
    void *handle;
} PcModLib;
static PcModLib g_mod_libs[PC_MOD_LIB_MAX];
static int g_mod_lib_count = 0;

static void *mod_get_handle(const char *path) {
    for (int i = 0; i < g_mod_lib_count; i++) {
        if (strcmp(g_mod_libs[i].path, path) == 0) return g_mod_libs[i].handle;
    }
    void *h = dlopen(path, RTLD_NOW);
    if (!h) return NULL;
    if (g_mod_lib_count < PC_MOD_LIB_MAX) {
        strncpy(g_mod_libs[g_mod_lib_count].path, path, sizeof(g_mod_libs[g_mod_lib_count].path) - 1);
        g_mod_libs[g_mod_lib_count].path[sizeof(g_mod_libs[g_mod_lib_count].path) - 1] = '\0';
        g_mod_libs[g_mod_lib_count].handle = h;
        g_mod_lib_count++;
    }
    return h;
}

static void load_mods_manifest(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        printf("No real mods manifest at %s -- starting with zero dynamically-loaded mods.\n", path);
        return;
    }
    char line[512];
    int lineno = 0;
    while (fgets(line, sizeof(line), f)) {
        lineno++;
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0' || *p == '\n' || *p == '#') continue;

        size_t len = strlen(p);
        while (len > 0 && (p[len - 1] == '\n' || p[len - 1] == '\r')) p[--len] = '\0';

        char *sep = strchr(p, '|');
        if (!sep) {
            fprintf(stderr, "WARNING: mods manifest %s line %d: malformed (expected so_path|function[|version]), skipped\n", path, lineno);
            continue;
        }
        *sep = '\0';
        const char *so_path = p;
        char *fn_name = sep + 1;

        /* Optional third field (version) -- see PC_MOD_VERSION_UNVERSIONED's own doc comment.
           A manifest line with no second '|' is exactly the pre-existing 2-field format, still
           fully supported. */
        const char *version = PC_MOD_VERSION_UNVERSIONED;
        char *ver_sep = strchr(fn_name, '|');
        if (ver_sep) {
            *ver_sep = '\0';
            version = ver_sep + 1;
        }

        if (g_mod_registry_count >= PC_MOD_REGISTRY_MAX) {
            fprintf(stderr, "WARNING: mods manifest %s line %d: registry full (%d max), skipped %s\n", path, lineno, PC_MOD_REGISTRY_MAX, fn_name);
            continue;
        }

        void *handle = mod_get_handle(so_path);
        if (!handle) {
            fprintf(stderr, "WARNING: mods manifest %s line %d: dlopen(%s) failed: %s -- mod skipped, server continues\n", path, lineno, so_path, dlerror());
            continue;
        }
        dlerror();
        void *sym = dlsym(handle, fn_name);
        const char *err = dlerror();
        if (err) {
            fprintf(stderr, "WARNING: mods manifest %s line %d: dlsym(%s) in %s failed: %s -- mod skipped, server continues\n", path, lineno, fn_name, so_path, err);
            continue;
        }

        strncpy(g_mod_registry[g_mod_registry_count].name, fn_name, sizeof(g_mod_registry[g_mod_registry_count].name) - 1);
        g_mod_registry[g_mod_registry_count].name[sizeof(g_mod_registry[g_mod_registry_count].name) - 1] = '\0';
        strncpy(g_mod_registry[g_mod_registry_count].version, version, sizeof(g_mod_registry[g_mod_registry_count].version) - 1);
        g_mod_registry[g_mod_registry_count].version[sizeof(g_mod_registry[g_mod_registry_count].version) - 1] = '\0';
        g_mod_registry[g_mod_registry_count].fn = sym;
        g_mod_registry_count++;
        printf("Real dynamically-loaded mod registered: %s@%s (from %s)\n", fn_name, version, so_path);
    }
    fclose(f);
    printf("Real mods manifest %s: %d mod(s) registered, %d distinct .so file(s) loaded.\n",
           path, g_mod_registry_count, g_mod_lib_count);
}

/* reload_mods_manifest: the real SIGHUP handler's own real work (see g_mods_reload_requested's
   own doc comment above for why this is safe -- name-keyed registry, no reentrancy). dlcloses
   every currently-loaded .so before reopening any of them, not just re-dlsym-ing into the same
   handles -- dlopen() on an already-open path returns the SAME cached mapping (refcounted by the
   real dynamic linker), so without a real dlclose first, a modder who rebuilt a .so in place
   would silently keep running the OLD code. A no-op, not an error, if --mods-manifest was never
   given -- there is nothing real to reload. */
static void reload_mods_manifest(void) {
    if (!g_mods_manifest_path[0]) {
        printf("Real SIGHUP received -- no --mods-manifest was given at startup, nothing to reload.\n");
        return;
    }
    printf("Real SIGHUP received -- reloading mods manifest %s...\n", g_mods_manifest_path);
    int old_registry_count = g_mod_registry_count;
    int old_lib_count = g_mod_lib_count;
    for (int i = 0; i < g_mod_lib_count; i++) dlclose(g_mod_libs[i].handle);
    g_mod_lib_count = 0;
    g_mod_registry_count = 0;
    load_mods_manifest(g_mods_manifest_path);
    printf("Real mods manifest reload complete: %d mod(s)/%d .so file(s) -> %d mod(s)/%d .so file(s).\n",
           old_registry_count, old_lib_count, g_mod_registry_count, g_mod_lib_count);
}

/* Connect-ticket secret -- direct port of WEAKNIGHT_BEDROCK_RACERS' own real
   load_ticket_secret/verify_connect_ticket pair (apps/server/src/main.c). Fails closed: an unset
   PAPERCRAFT_TICKET_SECRET means every connect is rejected, not silently accepted. */
static unsigned char g_ticket_secret[256];
static int g_ticket_secret_len = 0;

static void load_ticket_secret(void) {
    const char *env = getenv("PAPERCRAFT_TICKET_SECRET");
    if (!env || !env[0]) {
        printf("WARNING: PAPERCRAFT_TICKET_SECRET not set -- all connect attempts will be rejected (fail closed, not fail open)\n");
        return;
    }
    size_t len = strlen(env);
    if (len > sizeof(g_ticket_secret)) len = sizeof(g_ticket_secret);
    memcpy(g_ticket_secret, env, len);
    g_ticket_secret_len = (int)len;
    printf("PAPERCRAFT_TICKET_SECRET loaded (%d bytes)\n", g_ticket_secret_len);
}

static int verify_connect_ticket(const unsigned char ticket[PC_TICKET_TOTAL_LEN],
                                  unsigned char out_player_id[16]) {
    if (g_ticket_secret_len == 0) return 0;

    const unsigned char *payload = ticket;
    const unsigned char *given_mac = ticket + PC_TICKET_PAYLOAD_LEN;

    unsigned char expected_mac[32];
    hmac_sha256(g_ticket_secret, (size_t)g_ticket_secret_len, payload, PC_TICKET_PAYLOAD_LEN, expected_mac);
    if (!hmac_sha256_verify(given_mac, expected_mac, PC_TICKET_MAC_LEN)) return 0;

    unsigned int expires_at =
        (unsigned int)payload[16] | ((unsigned int)payload[17] << 8) |
        ((unsigned int)payload[18] << 16) | ((unsigned int)payload[19] << 24);
    if ((unsigned int)time(NULL) > expires_at) return 0;

    memcpy(out_player_id, payload, 16);
    return 1;
}

static void send_reject(int sock, const struct sockaddr_in *addr, socklen_t addr_len, const char *reason) {
    PcRejectPacket rej;
    memset(&rej, 0, sizeof(rej));
    rej.hdr.type = PC_PACKET_REJECT;
    snprintf(rej.reason, sizeof(rej.reason), "%s", reason);
    sendto(sock, &rej, sizeof(rej), 0, (const struct sockaddr *)addr, addr_len);
}

static unsigned int now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned int)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

/* fetch_city_world: real Phase 2 multi-chunk fetch -- one real GET /chunks?scene=200&cx=..&cz=..
   call per chunk in the real, fixed PW_GRID_DIM x PW_GRID_DIM grid around spawn (packages/common/
   papercraft_world.h's own PwWorld doc comment has the full real rationale, including the real,
   confirmed-live finding that worldapi's own urbanChunk generator doesn't vary content by cx/cz
   yet). Refuses to run if even one real chunk in the grid fails to load -- same "FATAL, don't run
   on a lie" discipline fetch_city_chunk (this function's own single-chunk predecessor) already
   established, now applied to every real grid slot, not just (0,0). */
static int fetch_city_world(const char *worldapi_host, int worldapi_port) {
    static char resp[65536]; /* real, bounded response buffer -- comfortably above the real ~50KB a 1054-block JSON array encodes to */
    for (int cz = -PW_GRID_RADIUS; cz <= PW_GRID_RADIUS; cz++) {
        for (int cx = -PW_GRID_RADIUS; cx <= PW_GRID_RADIUS; cx++) {
            int idx = pw_world_index(cx, cz);
            char path[128];
            snprintf(path, sizeof(path), "/chunks?scene=200&cx=%d&cz=%d", cx, cz);
            int status = 0;
            if (http_get_json(worldapi_host, worldapi_port, path, NULL, resp, sizeof(resp), &status) != 0) {
                fprintf(stderr, "fetch_city_world: worldapi unreachable at %s:%d for chunk (%d,%d)\n", worldapi_host, worldapi_port, cx, cz);
                return 0;
            }
            if (status != 200) {
                fprintf(stderr, "fetch_city_world: worldapi returned status %d for chunk (%d,%d)\n", status, cx, cz);
                return 0;
            }
            if (!pw_parse_chunks_json(resp, &g_world.chunks[idx])) {
                fprintf(stderr, "fetch_city_world: no real blocks parsed for chunk (%d,%d)\n", cx, cz);
                return 0;
            }
            g_world.loaded[idx] = 1;
        }
    }
    return 1;
}

/* spawn_player: real restart-persistence load-or-fresh-spawn (packages/common/
   papercraft_persist.h). If a real save exists for this slot's own player_id (s->has_player_id
   must already be set -- see the CONNECT handler, which sets it before calling this now),
   restores real position + progression from disk instead of resetting to level 1 -- closing
   NORTHSTAR.md's own "Explicitly not Phase 0: ... persistence across a restart." Falls through to
   the original real fresh-spawn path (ground-height lookup at column (8,8), confirmed clear of
   this chunk's own two real wall structures) for a genuinely new player_id, or a real save-file
   read failure (missing/corrupt) -- not a hardcoded Y either way. */
static void spawn_player(PlayerSlot *s) {
    memset(&s->state, 0, sizeof(s->state));
    s->state.costume = COS_SUIT; /* real default, matches BP_COSTUME_NAMES[0] "CIVILIAN SUIT" */
    s->state.decorum = decorum_start();
    s->decorum_zone = -1; /* no zone yet -- next server_tick_decorum call always counts as an entry */
    s->last_quiet_tick_ms = 0;
    s->decorum_cancelled_logged = 0;

    /* Real, live bug found and fixed during TYLER-phone-mechanics live verification (2026-08-30):
       g_slots[] is a static array reused across occupants (a timed-out or gracefully-freed slot
       gets claimed by the next CONNECT), but nothing was resetting the previous occupant's own
       transient per-connection wire state -- most critically latest_cmd_seq. Every real client
       starts its own cmd_sequence counter at a low number (this repo's own apps/client included),
       so a freshly-spawned player landing on a slot whose PREVIOUS occupant had already sent, say,
       300 UserCmds would have every one of their own real movement packets silently dropped by
       the `cmd.cmd_sequence >= s->latest_cmd_seq` staleness check below, until their own counter
       organically climbed back past that stale leftover value -- a real player who could
       apparently look() but not move() at all, matching this repo's own founder-reported "this
       version i cant do anything" symptom from exactly this kind of first-connection scenario.
       Real fix: this is the one real place a slot's own new occupancy begins (only reached from
       the CONNECT handler's own `if (!s->active)` fresh-claim branch), so this is the correct,
       single place to zero every transient field a stale previous occupant could have left
       behind, not just s->state above. */
    s->latest_move_x = 0.0f;
    s->latest_move_z = 0.0f;
    s->latest_yaw = 0.0f;
    s->latest_buttons = 0;
    s->latest_cmd_seq = 0;
    s->latest_cmd_time_ms = 0;
    s->was_holding_jump = 0;
    s->speed_boost_permille = 0;
    s->speed_boost_until_ms = 0;
    s->last_xp_tick_ms = 0;
    /* Real, deliberate inventory reset too -- same real class of bug as latest_cmd_seq above
       (a stale previous occupant's own leftover inventory must never carry over to a genuinely
       new player). Position/XP restore from a real save file below for an existing player_id;
       inventory does not yet (no PcSaveRecord field for it -- a real, explicitly deferred gap,
       not an oversight), so it always starts real-and-empty here regardless of which branch
       below this slot takes. */
    memset(s->inventory, 0, sizeof(s->inventory));

    if (s->has_player_id) {
        PcSaveRecord rec;
        if (pc_persist_load(g_save_dir, s->player_id, &rec)) {
            s->state.x = rec.x;
            s->state.y = rec.y;
            s->state.z = rec.z;
            s->state.yaw = rec.yaw;
            s->state.level = rec.level;
            s->state.xp = rec.xp;
            s->state.xp_to_next = xp_required_for_level(rec.level < 20 ? rec.level + 1 : 20);
            s->state.unspent_points = rec.unspent_points;
            for (int i = 0; i < PC_ABILITY_COUNT; i++) s->state.ability[i] = rec.ability[i];
            s->vy = 0.0f;
            s->on_ground = 1; /* real, honest assumption: a restored player starts standing, not mid-jump */
            printf("Real persisted player restored -- level %d, %d unspent points, position (%.1f,%.1f,%.1f).\n",
                   rec.level, rec.unspent_points, rec.x, rec.y, rec.z);
            return;
        }
    }

    int spawn_x = 8, spawn_z = 8;
    int ground_y;
    if (pw_world_ground_height_at(&g_world, spawn_x, spawn_z, &ground_y)) {
        s->state.x = (float)spawn_x;
        s->state.z = (float)spawn_z;
        s->state.y = (float)ground_y;
    } else {
        fprintf(stderr, "WARNING: spawn column (%d,%d) has no real solid block under it -- spawning at y=0\n", spawn_x, spawn_z);
        s->state.x = (float)spawn_x;
        s->state.z = (float)spawn_z;
        s->state.y = 0.0f;
    }
    s->state.yaw = 0.0f;
    /* Real starting progression, matching the construct's own progression_reset -- level 1, no
       XP, no unspent points. Only reached for a genuinely new player_id (or a real save-file
       read failure) -- an existing player_id with a real, valid save on disk returns above
       instead, restoring real progression across a restart now (packages/common/
       papercraft_persist.h), not just within one server run. */
    s->state.level = 1;
    s->state.xp = 0;
    s->state.xp_to_next = xp_required_for_level(2);
    s->state.unspent_points = 0;
    s->vy = 0.0f;
    s->on_ground = 1;
}

/* award_xp: real, shared XP-grant + level-up path -- factored out so every real XP source (the
   passive per-second tick below, and the new real "destroyed a world object" event) applies the
   exact same real level-up decision (level_mod.c's own on_papercraft_level_for_xp), not two
   independently-maintained copies of the same real logic. slot_idx is only used for the real,
   human-readable log line. */
static void award_xp(PlayerSlot *s, int amount, int slot_idx) {
    if (amount <= 0 || s->state.level >= 20) return;
    s->state.xp += amount;
    int old_level = s->state.level;
    int new_level = on_papercraft_level_for_xp(old_level, s->state.xp);
    if (new_level > old_level) {
        s->state.unspent_points += (new_level - old_level);
        s->state.level = new_level;
        printf("Player slot %d leveled up: %d -> %d (xp=%d, +%d points)\n",
               slot_idx, old_level, new_level, s->state.xp, new_level - old_level);
    }
    s->state.xp_to_next = xp_required_for_level(s->state.level < 20 ? s->state.level + 1 : 20);
}

/* g_pickup_radius -- real, cached float form of the real PARENA-decided
   on_papercraft_pickup_radius_millis(), read once at startup (a real, static tuning value, not a
   per-event decision -- see pickup_mod.prn's own doc comment) and reused every tick rather than
   re-calling the mod and re-dividing by 1000.0f on every single active-entity/active-player pair,
   every tick. */
static float g_pickup_radius = 0.0f;

/* broadcast_entity_spawn / broadcast_entity_despawn -- real, sent to every currently-active
   player (a dropped item is visible to everyone in the real persistent world, not just whoever
   caused it), same real broadcast-loop shape apps/server's own snapshot send already uses below
   in the main tick loop. */
static void broadcast_entity_spawn_to(int sock, int entity_id, PlayerSlot *s) {
    PcEntitySpawnPacket sp;
    memset(&sp, 0, sizeof(sp));
    sp.hdr.type = PC_PACKET_ENTITY_SPAWN;
    sp.entity_id = (unsigned char)entity_id;
    sp.item_id = g_entities[entity_id].item_id;
    sp.x = g_entities[entity_id].x;
    sp.y = g_entities[entity_id].y;
    sp.z = g_entities[entity_id].z;
    sendto(sock, &sp, sizeof(sp), 0, (struct sockaddr *)&s->addr, s->addr_len);
}

static void broadcast_entity_spawn(int sock, int entity_id) {
    for (int i = 0; i < PC_MAX_PLAYERS; i++) {
        if (!g_slots[i].active) continue;
        broadcast_entity_spawn_to(sock, entity_id, &g_slots[i]);
    }
}

static void broadcast_entity_despawn(int sock, int entity_id) {
    PcEntityDespawnPacket dp;
    memset(&dp, 0, sizeof(dp));
    dp.hdr.type = PC_PACKET_ENTITY_DESPAWN;
    dp.entity_id = (unsigned char)entity_id;
    for (int i = 0; i < PC_MAX_PLAYERS; i++) {
        if (!g_slots[i].active) continue;
        sendto(sock, &dp, sizeof(dp), 0, (struct sockaddr *)&g_slots[i].addr, g_slots[i].addr_len);
    }
}

/* send_inventory_update -- real, whole-inventory sync to ONE specific player (unlike the entity
   broadcasts above, a player's own inventory contents are private to them, never sent to anyone
   else -- same real "only the owner needs it" reasoning PcSnapshotPacket::echo_cmd_time_ms's own
   doc comment already used for round-trip time). */
static void send_inventory_update(int sock, PlayerSlot *s) {
    PcInventoryUpdatePacket iu;
    memset(&iu, 0, sizeof(iu));
    iu.hdr.type = PC_PACKET_INVENTORY_UPDATE;
    memcpy(iu.slots, s->inventory, sizeof(iu.slots));
    sendto(sock, &iu, sizeof(iu), 0, (struct sockaddr *)&s->addr, s->addr_len);
}

/* send_weapon_owned_update -- real, whole-bitmask sync to ONE specific player, same real
   "private to the owner" convention send_inventory_update above already establishes. Called
   once, right after granting a new weapon (never on every tick -- this is an event, not
   continuous state). */
static void send_weapon_owned_update(int sock, PlayerSlot *s) {
    PcWeaponOwnedPacket wu;
    memset(&wu, 0, sizeof(wu));
    wu.hdr.type = PC_PACKET_WEAPON_OWNED;
    wu.weapons_owned = s->weapons_owned;
    wu.current_weapon = s->current_weapon;
    sendto(sock, &wu, sizeof(wu), 0, (struct sockaddr *)&s->addr, s->addr_len);
}

/* try_add_item_to_inventory -- thin, real per-player wrapper around packages/common/
   papercraft_inventory.h's own real, pure, independently-tested pc_try_add_item_to_inventory
   (packages/common/papercraft_inventory_test.c). Pulled out to a shared header (2026-08-30,
   founder real-time: "can we make it a native test?" -- replacing a first-pass, slow, throwaway
   Python UDP probe) so the real add-to-inventory logic itself needs no live server, no PlayerSlot,
   and no UDP wire round trip to verify at all -- this function is now just PlayerSlot plumbing. */
static int try_add_item_to_inventory(PlayerSlot *s, int item_id) {
    return pc_try_add_item_to_inventory(s->inventory, item_id);
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    const char *worldapi_host = "localhost";
    int worldapi_port = 7070;
    int server_port = PC_SERVER_PORT;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--worldapi-host") == 0 && i + 1 < argc) worldapi_host = argv[++i];
        else if (strcmp(argv[i], "--worldapi-port") == 0 && i + 1 < argc) worldapi_port = atoi(argv[++i]);
        else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) server_port = atoi(argv[++i]);
        else if (strcmp(argv[i], "--save-dir") == 0 && i + 1 < argc) {
            strncpy(g_save_dir, argv[++i], sizeof(g_save_dir) - 1);
            g_save_dir[sizeof(g_save_dir) - 1] = '\0';
        } else if (strcmp(argv[i], "--world-file") == 0 && i + 1 < argc) {
            strncpy(g_world_objects_path, argv[++i], sizeof(g_world_objects_path) - 1);
            g_world_objects_path[sizeof(g_world_objects_path) - 1] = '\0';
        } else if (strcmp(argv[i], "--damage-file") == 0 && i + 1 < argc) {
            strncpy(g_world_damage_path, argv[++i], sizeof(g_world_damage_path) - 1);
            g_world_damage_path[sizeof(g_world_damage_path) - 1] = '\0';
        } else if (strcmp(argv[i], "--mods-manifest") == 0 && i + 1 < argc) {
            strncpy(g_mods_manifest_path, argv[++i], sizeof(g_mods_manifest_path) - 1);
            g_mods_manifest_path[sizeof(g_mods_manifest_path) - 1] = '\0';
        }
    }

    pc_persist_ensure_dir(g_save_dir);
    printf("Real player persistence dir: %s\n", g_save_dir);
    signal(SIGINT, handle_shutdown_signal);
    signal(SIGTERM, handle_shutdown_signal);
    signal(SIGHUP, handle_reload_signal);

    printf("PAPERCRAFT server (single-node persistent) -- fetching real %dx%d chunk grid from worldapi %s:%d (scene=200)...\n",
           PW_GRID_DIM, PW_GRID_DIM, worldapi_host, worldapi_port);
    if (!fetch_city_world(worldapi_host, worldapi_port)) {
        fprintf(stderr, "FATAL: could not load the real city chunk grid from worldapi -- refusing to run on fake/empty terrain.\n");
        return 1;
    }
    {
        int total_blocks = 0;
        for (int i = 0; i < PW_GRID_CHUNKS; i++) total_blocks += g_world.chunks[i].block_count;
        printf("Real city chunk grid loaded (%d chunks, %d total blocks, scene 200, cx/cz in [-%d,%d]).\n",
               PW_GRID_CHUNKS, total_blocks, PW_GRID_RADIUS, PW_GRID_RADIUS);
    }

    /* Real, persisted world objects (packages/common/papercraft_worldobjects.h) -- graduates the
       original hardcoded single test cube (docs/NORTHSTAR_PAPER_ENGINE.md's own "What's
       explicitly not built yet" -- closing the "no hit-detection wiring" gap) into real,
       map-editor-editable data (apps/mapeditor). If no real world-objects file exists yet (a
       fresh server, or a fresh --world-file path), seeds one real default object matching the
       original test cube's own position/material/seed -- real, backward-compatible behavior,
       not a silent behavior change -- and saves it so it's real, persisted data from then on. */
    if (!pc_worldobjects_load(g_world_objects_path, &g_wo_file)) {
        memset(&g_wo_file, 0, sizeof(g_wo_file));
        g_wo_file.magic = PC_WO_MAGIC;
        g_wo_file.count = 1;
        g_wo_file.objects[0].x = PC_DEFAULT_OBJECT_X;
        g_wo_file.objects[0].z = PC_DEFAULT_OBJECT_Z;
        g_wo_file.objects[0].material = PC_DEFAULT_OBJECT_MATERIAL;
        g_wo_file.objects[0].half_x = PC_DEFAULT_OBJECT_HALF_EXTENT;
        g_wo_file.objects[0].half_y = PC_DEFAULT_OBJECT_HALF_EXTENT;
        g_wo_file.objects[0].half_z = PC_DEFAULT_OBJECT_HALF_EXTENT;
        g_wo_file.objects[0].seed = PC_DEFAULT_OBJECT_SEED;
        int ground_y;
        if (pw_world_ground_height_at(&g_world, (int)PC_DEFAULT_OBJECT_X, (int)PC_DEFAULT_OBJECT_Z, &ground_y)) {
            g_wo_file.objects[0].y = (float)ground_y + PC_DEFAULT_OBJECT_HALF_EXTENT;
        } else {
            g_wo_file.objects[0].y = PC_DEFAULT_OBJECT_HALF_EXTENT;
        }

        /* Real second through fifth default objects -- the real, carved-out city walls
           (PC_CITY_WALL_A1_* / PC_CITY_WALL_A2_* / PC_CITY_WALL_B1_* / PC_CITY_WALL_B2_*), each
           standing in exactly where its own real VoxelBlocks used to be. Real position/extents/
           carve bounds are already derived directly from that real block data (see the constants'
           own doc comment), not ground-snapped like a normal editor placement. Wired through the
           same real, general has_carve/carve_* machinery apps/mapeditor's own --carve flag uses --
           not a special case. Both real walls are now a real, precise two-box L-shape each
           (2026-08-29) -- Wall B's own split follows S206-43's own real PC_WO_MAX_OBJECTS=4->8
           bit-packing headroom, the same real fix that already let Wall A split first. Even with
           both walls now precise, 3 real slots (of 8) stay free. */
        g_wo_file.count = 5;
        g_wo_file.objects[1].x = PC_CITY_WALL_A1_X;
        g_wo_file.objects[1].y = PC_CITY_WALL_A1_Y;
        g_wo_file.objects[1].z = PC_CITY_WALL_A1_Z;
        g_wo_file.objects[1].material = PC_CITY_WALL_A1_MATERIAL;
        g_wo_file.objects[1].half_x = PC_CITY_WALL_A1_HALF_X;
        g_wo_file.objects[1].half_y = PC_CITY_WALL_A1_HALF_Y;
        g_wo_file.objects[1].half_z = PC_CITY_WALL_A1_HALF_Z;
        g_wo_file.objects[1].seed = PC_CITY_WALL_A1_SEED;
        g_wo_file.objects[1].has_carve = 1;
        g_wo_file.objects[1].carve_x0 = PC_CITY_WALL_A1_BLOCK_X0;
        g_wo_file.objects[1].carve_x1 = PC_CITY_WALL_A1_BLOCK_X1;
        g_wo_file.objects[1].carve_y0 = PC_CITY_WALL_A1_BLOCK_Y0;
        g_wo_file.objects[1].carve_y1 = PC_CITY_WALL_A1_BLOCK_Y1;
        g_wo_file.objects[1].carve_z0 = PC_CITY_WALL_A1_BLOCK_Z0;
        g_wo_file.objects[1].carve_z1 = PC_CITY_WALL_A1_BLOCK_Z1;

        g_wo_file.objects[2].x = PC_CITY_WALL_A2_X;
        g_wo_file.objects[2].y = PC_CITY_WALL_A2_Y;
        g_wo_file.objects[2].z = PC_CITY_WALL_A2_Z;
        g_wo_file.objects[2].material = PC_CITY_WALL_A2_MATERIAL;
        g_wo_file.objects[2].half_x = PC_CITY_WALL_A2_HALF_X;
        g_wo_file.objects[2].half_y = PC_CITY_WALL_A2_HALF_Y;
        g_wo_file.objects[2].half_z = PC_CITY_WALL_A2_HALF_Z;
        g_wo_file.objects[2].seed = PC_CITY_WALL_A2_SEED;
        g_wo_file.objects[2].has_carve = 1;
        g_wo_file.objects[2].carve_x0 = PC_CITY_WALL_A2_BLOCK_X0;
        g_wo_file.objects[2].carve_x1 = PC_CITY_WALL_A2_BLOCK_X1;
        g_wo_file.objects[2].carve_y0 = PC_CITY_WALL_A2_BLOCK_Y0;
        g_wo_file.objects[2].carve_y1 = PC_CITY_WALL_A2_BLOCK_Y1;
        g_wo_file.objects[2].carve_z0 = PC_CITY_WALL_A2_BLOCK_Z0;
        g_wo_file.objects[2].carve_z1 = PC_CITY_WALL_A2_BLOCK_Z1;

        g_wo_file.objects[3].x = PC_CITY_WALL_B1_X;
        g_wo_file.objects[3].y = PC_CITY_WALL_B1_Y;
        g_wo_file.objects[3].z = PC_CITY_WALL_B1_Z;
        g_wo_file.objects[3].material = PC_CITY_WALL_B1_MATERIAL;
        g_wo_file.objects[3].half_x = PC_CITY_WALL_B1_HALF_X;
        g_wo_file.objects[3].half_y = PC_CITY_WALL_B1_HALF_Y;
        g_wo_file.objects[3].half_z = PC_CITY_WALL_B1_HALF_Z;
        g_wo_file.objects[3].seed = PC_CITY_WALL_B1_SEED;
        g_wo_file.objects[3].has_carve = 1;
        g_wo_file.objects[3].carve_x0 = PC_CITY_WALL_B1_BLOCK_X0;
        g_wo_file.objects[3].carve_x1 = PC_CITY_WALL_B1_BLOCK_X1;
        g_wo_file.objects[3].carve_y0 = PC_CITY_WALL_B1_BLOCK_Y0;
        g_wo_file.objects[3].carve_y1 = PC_CITY_WALL_B1_BLOCK_Y1;
        g_wo_file.objects[3].carve_z0 = PC_CITY_WALL_B1_BLOCK_Z0;
        g_wo_file.objects[3].carve_z1 = PC_CITY_WALL_B1_BLOCK_Z1;

        g_wo_file.objects[4].x = PC_CITY_WALL_B2_X;
        g_wo_file.objects[4].y = PC_CITY_WALL_B2_Y;
        g_wo_file.objects[4].z = PC_CITY_WALL_B2_Z;
        g_wo_file.objects[4].material = PC_CITY_WALL_B2_MATERIAL;
        g_wo_file.objects[4].half_x = PC_CITY_WALL_B2_HALF_X;
        g_wo_file.objects[4].half_y = PC_CITY_WALL_B2_HALF_Y;
        g_wo_file.objects[4].half_z = PC_CITY_WALL_B2_HALF_Z;
        g_wo_file.objects[4].seed = PC_CITY_WALL_B2_SEED;
        g_wo_file.objects[4].has_carve = 1;
        g_wo_file.objects[4].carve_x0 = PC_CITY_WALL_B2_BLOCK_X0;
        g_wo_file.objects[4].carve_x1 = PC_CITY_WALL_B2_BLOCK_X1;
        g_wo_file.objects[4].carve_y0 = PC_CITY_WALL_B2_BLOCK_Y0;
        g_wo_file.objects[4].carve_y1 = PC_CITY_WALL_B2_BLOCK_Y1;
        g_wo_file.objects[4].carve_z0 = PC_CITY_WALL_B2_BLOCK_Z0;
        g_wo_file.objects[4].carve_z1 = PC_CITY_WALL_B2_BLOCK_Z1;

        if (!pc_worldobjects_save(g_world_objects_path, &g_wo_file)) {
            fprintf(stderr, "WARNING: could not save the real default world-objects file to %s\n", g_world_objects_path);
        }
        printf("No real world-objects file at %s -- seeded %d real default objects (test prop + Wall A's real 2-box L-shape + Wall B's real 2-box L-shape).\n", g_world_objects_path, g_wo_file.count);
    } else {
        printf("Real world-objects file loaded from %s (%d object(s)).\n", g_world_objects_path, g_wo_file.count);
    }

    /* Real, general, data-driven carve-out (packages/common/papercraft_worldobjects.h's own
       has_carve/carve_* fields) -- any real object in the loaded/seeded list can carry real
       carve bounds now, not just one hardcoded case. Runs once here, before any player connects,
       same real "carve before render/collide" ordering the original single-wall version used. */
    {
        int origin_idx = pw_world_index(0, 0);
        for (int i = 0; i < g_wo_file.count; i++) {
            if (!g_wo_file.objects[i].has_carve) continue;
            if (origin_idx < 0 || !g_world.loaded[origin_idx]) continue;
            int before = g_world.chunks[origin_idx].block_count;
            pw_chunk_remove_box(&g_world.chunks[origin_idx],
                                 g_wo_file.objects[i].carve_x0, g_wo_file.objects[i].carve_x1,
                                 g_wo_file.objects[i].carve_y0, g_wo_file.objects[i].carve_y1,
                                 g_wo_file.objects[i].carve_z0, g_wo_file.objects[i].carve_z1);
            int removed = before - g_world.chunks[origin_idx].block_count;
            printf("Real city carve-out: object %d removed %d real block(s) from chunk (0,0).\n", i, removed);
        }
    }

    for (int i = 0; i < g_wo_file.count; i++) {
        paper_generate_box(&g_wo_mesh[i], g_wo_file.objects[i].half_x, g_wo_file.objects[i].half_y,
                            g_wo_file.objects[i].half_z, PC_WO_SUBDIV,
                            g_wo_file.objects[i].material, g_wo_file.objects[i].seed);
    }
    printf("Real Paper Engine: %d world object(s) live (%d fragments each) -- press E in reach to punch one.\n",
           g_wo_file.count, PC_WO_FRAGMENTS);

    /* Real per-fragment damage restore (packages/common/papercraft_worldobjects.h's own
       PcWorldDamageFile) -- closes docs/NORTHSTAR_PAPER_ENGINE.md's own honestly-named gap ("no
       persistence of a damaged building's own state across a server restart"). Restores the real
       source-of-truth hp, then re-derives state via the exact same real PARENA-compiled decision
       (on_paper_fragment_state_for_hp) fresh damage always uses -- never a separately-persisted,
       possibly-inconsistent state field. A fully-destroyed object also re-latches
       g_wo_destroyed_awarded so a restart can't let a player re-earn xp_award_mod's own real
       reward for an object that was already fully destroyed before the restart. */
    {
        PcWorldDamageFile damage;
        if (pc_worldobjects_load_damage(g_world_damage_path, &damage)) {
            int restored_objects = 0;
            for (int o = 0; o < g_wo_file.count; o++) {
                int gone_count = 0;
                for (int f = 0; f < g_wo_mesh[o].fragment_count && f < PC_WO_FRAGMENTS; f++) {
                    PaperFragment *frag = &g_wo_mesh[o].fragments[f];
                    frag->hp = damage.hp[o][f];
                    frag->state = on_paper_fragment_state_for_hp(frag->hp, frag->max_hp);
                    if (frag->state == PAPER_STATE_GONE) gone_count++;
                }
                if (gone_count == g_wo_mesh[o].fragment_count) g_wo_destroyed_awarded[o] = 1;
                restored_objects++;
            }
            printf("Real per-fragment damage restored from %s (%d object(s)).\n", g_world_damage_path, restored_objects);
        } else {
            printf("No real damage file at %s yet -- every object starts pristine.\n", g_world_damage_path);
        }
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((unsigned short)server_port);
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) { perror("bind"); return 1; }
    int fl = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, fl | O_NONBLOCK);

    printf("Listening on UDP :%d (tick=%dHz)\n", server_port, PC_TICK_HZ);
    load_ticket_secret();
    if (g_mods_manifest_path[0]) load_mods_manifest(g_mods_manifest_path);

    memset(g_slots, 0, sizeof(g_slots));
    memset(g_entities, 0, sizeof(g_entities));
    server_spawn_npcs(now_ms());
    server_spawn_giant_bugs(now_ms());
    g_pickup_radius = (float)on_papercraft_pickup_radius_millis() / 1000.0f;
    printf("Real, PARENA-decided pickup radius: %.2f world units.\n", g_pickup_radius);

    unsigned int last_tick_ms = now_ms();
    const unsigned int tick_ms = 1000 / PC_TICK_HZ;
    unsigned int server_tick = 0;

    for (;;) {
        /* Real graceful shutdown -- a deliberate SIGINT/SIGTERM (a real restart, not a crash)
           flushes every real active player's own current state to disk immediately, rather than
           relying on the periodic autosave's own real, bounded staleness window. */
        if (g_shutdown_requested) {
            int saved_count = 0;
            for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                if (g_slots[i].active) { save_player(&g_slots[i]); saved_count++; }
            }
            save_world_damage();
            printf("Real shutdown signal received -- saved %d active player(s) + world object damage, exiting.\n", saved_count);
            break;
        }

        /* Real live mod-manifest reload -- see g_mods_reload_requested's own doc comment above
           for why this is the one real piece of live state that's safe to reload without a
           restart. Runs here, between ticks, same as the shutdown check above -- never while a
           real gameplay call site is mid-call (this server is single-threaded, no reentrancy). */
        if (g_mods_reload_requested) {
            g_mods_reload_requested = 0;
            reload_mods_manifest();
        }

        /* Real bug found live (2026-08-28, wiring the real progression fields into
           PcPlayerState): a hardcoded 512-byte recv buffer silently truncated
           PcSnapshotPacket once it grew past 512 bytes (real sizeof = 540, once the compiler's
           own -Warray-bounds flagged the actual memcpy below) -- sized off the real wire format
           itself now, not a guessed constant, so this can't silently under-size again as the
           protocol keeps growing. */
        char buf[sizeof(PcSnapshotPacket) + 64];
        struct sockaddr_in from;
        socklen_t from_len = sizeof(from);
        ssize_t n;
        while ((n = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&from, &from_len)) > 0) {
            if ((size_t)n < sizeof(PcHeader)) continue;
            PcHeader hdr;
            memcpy(&hdr, buf, sizeof(PcHeader));

            if (hdr.type == PC_PACKET_CONNECT) {
                if ((size_t)n < sizeof(PcConnectPacket)) {
                    send_reject(sock, &from, from_len, "Client too old -- CONNECT missing a ticket.");
                    continue;
                }
                PcConnectPacket cp;
                memcpy(&cp, buf, sizeof(cp));
                unsigned char player_id[16];
                if (!verify_connect_ticket(cp.ticket, player_id)) {
                    send_reject(sock, &from, from_len,
                                g_ticket_secret_len == 0
                                    ? "Server ticket auth not configured yet."
                                    : "Ticket invalid or expired -- log in again.");
                    continue;
                }

                /* Real one-seat-per-identity: reuse an existing slot for this player_id if they
                   already have one (a reconnect), otherwise claim the first free slot. */
                int slot_idx = -1;
                for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                    if (g_slots[i].active && g_slots[i].has_player_id &&
                        memcmp(g_slots[i].player_id, player_id, 16) == 0) {
                        slot_idx = i;
                        break;
                    }
                }
                if (slot_idx == -1) {
                    for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                        if (!g_slots[i].active) { slot_idx = i; break; }
                    }
                }
                if (slot_idx == -1) {
                    send_reject(sock, &from, from_len, "Server full.");
                    continue;
                }

                PlayerSlot *s = &g_slots[slot_idx];
                /* One live client per identity (2026-09-19): several windows on the same account each re-sent CONNECT and stole the slot from
                   each other, so nobody kept input or snapshots ("stuck"). A CONNECT from a DIFFERENT address while the slot's current address is
                   still sending input (last 3s) is a second window: refuse it, visibly. If the current address has gone quiet (crash, relaunch,
                   or a cellular NAT port change) the new address takes over, which is exactly the re-hello healing path. */
                if (s->active && s->has_player_id &&
                    (s->addr.sin_addr.s_addr != from.sin_addr.s_addr || s->addr.sin_port != from.sin_port) &&
                    now_ms() - s->last_usercmd_ms < 3000) {
                    static unsigned int last_dup_log_ms = 0;
                    if (now_ms() - last_dup_log_ms > 5000) {
                        printf("[net] CONNECT from %s:%d refused: slot %d is live on %s:%d (another window on this account)\n", inet_ntoa(from.sin_addr),
                               ntohs(from.sin_port), slot_idx, inet_ntoa(s->addr.sin_addr), ntohs(s->addr.sin_port));
                        last_dup_log_ms = now_ms();
                    }
                    send_reject(sock, &from, from_len, "This account is already connected from another window or device. Close it first.");
                    continue;
                }
                if (!s->active) {
                    /* Real player_id must land on the slot BEFORE spawn_player runs -- spawn_player's
                       own real persistence lookup (packages/common/papercraft_persist.h) keys off
                       s->has_player_id/s->player_id, so this order matters now, not just cosmetically. */
                    s->has_player_id = 1;
                    memcpy(s->player_id, player_id, 16);
                    s->active = 1;
                    spawn_player(s);
                    printf("Player claimed slot %d from %s:%d\n", slot_idx, inet_ntoa(from.sin_addr), ntohs(from.sin_port));
                }
                s->has_player_id = 1;
                memcpy(s->player_id, player_id, 16);
                s->addr = from;
                s->addr_len = from_len;
                s->lz4 = ((size_t)n > sizeof(PcConnectPacket) && (((const unsigned char *)buf)[sizeof(PcConnectPacket)] & PC_CAP_LZ4)) ? 1 : 0;
                if (getenv("PAPERCRAFT_NO_LZ4")) s->lz4 = 0; /* A/B switch: force plain snapshots */
                /* Real, deliberate reset -- a CONNECT (fresh claim or a real reconnect) counts as
                   real activity for PC_PLAYER_TIMEOUT_MS's own purposes, same as any other real
                   client-to-server packet. Without this, a freshly-claimed slot with no USERCMD
                   sent yet would read last_usercmd_ms as its own zero-initialized default and
                   look already-timed-out on the very next tick. */
                s->last_usercmd_ms = now_ms();

                PcWelcomePacket w;
                memset(&w, 0, sizeof(w));
                w.hdr.type = PC_PACKET_WELCOME;
                w.hdr.client_id = (unsigned char)slot_idx;
                w.client_id = (unsigned char)slot_idx;
                sendto(sock, &w, sizeof(w), 0, (struct sockaddr *)&s->addr, s->addr_len);

                /* Real, one-time catch-up for a freshly-connected (or reconnecting) client: every
                   currently-active real entity already dropped in the world gets sent as its own
                   real spawn packet, so a late joiner sees item drops that happened before they
                   connected -- without this, a client's own local entity list (built entirely
                   from spawn/despawn events, see papercraft_protocol.h's own doc comment) would
                   start empty even when the real world isn't. Real inventory sync right after --
                   a genuinely fresh spawn is really empty (spawn_player's own real reset), and a
                   same-run reconnect's own real, still-live s->inventory is sent as-is. */
                for (int e = 0; e < PC_ENTITY_MAX; e++) {
                    if (g_entities[e].active) broadcast_entity_spawn_to(sock, e, s);
                }
                send_inventory_update(sock, s);
            } else if (hdr.type == PC_PACKET_USERCMD && (size_t)n >= sizeof(PcUserCmdPacket)) {
                /* Real per-slot dispatch by reply address -- same convention
                   WEAKNIGHT_BEDROCK_RACERS' own server uses for its own human slot 0. */
                int usercmd_matched = 0;
                for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                    PlayerSlot *s = &g_slots[i];
                    if (!s->active || s->addr.sin_addr.s_addr != from.sin_addr.s_addr ||
                        s->addr.sin_port != from.sin_port) {
                        continue;
                    }
                    usercmd_matched = 1;
                    s->usercmds_rx++;
                    PcUserCmdPacket cmd;
                    memcpy(&cmd, buf, sizeof(cmd));
                    if (cmd.cmd_sequence >= s->latest_cmd_seq) {
                        s->latest_cmd_seq = cmd.cmd_sequence;
                        s->latest_move_x = cmd.move_x;
                        s->latest_move_z = cmd.move_z;
                        s->latest_yaw = cmd.yaw;
                        s->latest_buttons = cmd.buttons;
                        s->last_usercmd_ms = now_ms();
                        s->latest_cmd_time_ms = cmd.cmd_time_ms;
                    }
                    break;
                }
                if (!usercmd_matched) {
                    /* Diagnostic (2026-09-19, mobile player frozen at spawn, client saw no snapshots): movement from an address that
                       matches no slot is silently dropped. On cellular links the NAT can change the source PORT mid-session; then the
                       server keeps sending snapshots to the old port while ignoring the new one. Log it (rate-limited) so this is provable. */
                    static unsigned int last_unmatched_log_ms = 0; static unsigned int unmatched_since_log = 0;
                    unmatched_since_log++;
                    unsigned int t_ms = now_ms();
                    if (t_ms - last_unmatched_log_ms > 5000) {
                        int same_ip_slot = -1;
                        for (int i = 0; i < PC_MAX_PLAYERS; i++)
                            if (g_slots[i].active && g_slots[i].addr.sin_addr.s_addr == from.sin_addr.s_addr) { same_ip_slot = i; break; }
                        if (same_ip_slot >= 0)
                            printf("[net] %u USERCMD(s) ignored from %s:%d -- slot %d is on that IP but port %d: NAT port change; waiting for the client's re-hello\n",
                                   unmatched_since_log, inet_ntoa(from.sin_addr), ntohs(from.sin_port), same_ip_slot, ntohs(g_slots[same_ip_slot].addr.sin_port));
                        else
                            printf("[net] %u USERCMD(s) ignored from %s:%d -- no slot with that IP\n", unmatched_since_log, inet_ntoa(from.sin_addr), ntohs(from.sin_port));
                        last_unmatched_log_ms = t_ms; unmatched_since_log = 0;
                    }
                }
            } else if (hdr.type == PC_PACKET_ALLOCATE_TALENT && (size_t)n >= sizeof(PcAllocateTalentPacket)) {
                /* Real "mods first everything" gameplay: the actual gate decision (is this a
                   legal ability index? does the player have a point to spend? is that ability
                   already at its own real cap?) is the real PARENA-compiled
                   on_papercraft_can_allocate_talent -- this handler only applies the real
                   consequence once the mod says yes, same "mod decides, host applies" split
                   every real mod call site in this monorepo already uses. */
                for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                    PlayerSlot *s = &g_slots[i];
                    if (!s->active || s->addr.sin_addr.s_addr != from.sin_addr.s_addr ||
                        s->addr.sin_port != from.sin_port) {
                        continue;
                    }
                    PcAllocateTalentPacket req;
                    memcpy(&req, buf, sizeof(req));
                    if (req.ability_index >= PC_ABILITY_COUNT) break;
                    int idx = req.ability_index;
                    if (on_papercraft_can_allocate_talent(s->state.ability[idx], s->state.unspent_points)) {
                        s->state.ability[idx]++;
                        s->state.unspent_points--;
                        printf("Player slot %d allocated a point into ability %d (now rank %d, %d points left)\n",
                               i, idx, s->state.ability[idx], s->state.unspent_points);
                    }
                    break;
                }
            } else if (hdr.type == PC_PACKET_WEAPON_SWITCH && (size_t)n >= sizeof(PcWeaponSwitchPacket)) {
                /* Real "mods first everything" gameplay, same shape as PC_PACKET_ALLOCATE_TALENT
                   right above: the actual gate decision (does this player actually own this
                   weapon slot, or is it the universal baseline Knife?) is the real PARENA-
                   compiled on_papercraft_weapon_switch_allowed -- this handler only applies the
                   real consequence once the mod says yes. */
                for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                    PlayerSlot *s = &g_slots[i];
                    if (!s->active || s->addr.sin_addr.s_addr != from.sin_addr.s_addr ||
                        s->addr.sin_port != from.sin_port) {
                        continue;
                    }
                    PcWeaponSwitchPacket req;
                    memcpy(&req, buf, sizeof(req));
                    int slot = req.requested_slot;
                    if (on_papercraft_weapon_switch_allowed((int)s->weapons_owned, slot)) {
                        s->current_weapon = (unsigned char)slot;
                        printf("Player slot %d switched to weapon slot %d.\n", i, slot);
                    }
                    /* Real, server-authoritative confirmation either way -- allowed (the switch
                       actually happened) or denied (current_weapon is unchanged, sent back as-is
                       so this client's own HUD never has to guess). */
                    send_weapon_owned_update(sock, s);
                    break;
                }
            } else if (hdr.type == PC_PACKET_COSTUME_SET && (size_t)n >= sizeof(PcCostumeSetPacket)) {
                /* EMILY/BACKLOG.md SECTION 536 follow-up, BIG_O/NORTHSTAR.md §18 Phase A -- the
                   first time costume becomes server-authoritative. No mod gate needed (unlike
                   weapon switch, which needs "has this player actually found it" -- any costume
                   in the real, fixed COS_* roster is always wearable, the real consequence is
                   zone_access/conspicuousness reacting to it, not an ownership check here). */
                for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                    PlayerSlot *s = &g_slots[i];
                    if (!s->active || s->addr.sin_addr.s_addr != from.sin_addr.s_addr ||
                        s->addr.sin_port != from.sin_port) {
                        continue;
                    }
                    PcCostumeSetPacket req;
                    memcpy(&req, buf, sizeof(req));
                    if (req.costume <= COS_STREET) {
                        s->state.costume = req.costume;
                        printf("Player slot %d set costume to %d.\n", i, s->state.costume);
                    }
                    break;
                }
            } else if (hdr.type == PC_PACKET_ITEM_USE && (size_t)n >= sizeof(PcItemUsePacket)) {
                /* EMILY/BACKLOG.md SECTION 536 follow-up, BIG_O/NORTHSTAR.md §20 -- Cargo's
                   SELECT finally does something. Only food items are consumable here (weapons/
                   scrap have no defined "use" behavior via Cargo -- they're equipped via
                   Loadout, not eaten -- so a non-food slot is a real, honest no-op, not a silent
                   item loss). Only FOOD_CAKE has a real effect (the cake-smash distraction);
                   every other food item is consumed with no effect yet -- eat-to-heal is still
                   real, separate, deliberately not built (see bigo_food_items.h's own doc
                   comment: no player HP/damage pool exists beyond the Regulator kill binary). */
                for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                    PlayerSlot *s = &g_slots[i];
                    if (!s->active || s->addr.sin_addr.s_addr != from.sin_addr.s_addr ||
                        s->addr.sin_port != from.sin_port) {
                        continue;
                    }
                    PcItemUsePacket req;
                    memcpy(&req, buf, sizeof(req));
                    if (req.slot < PC_INVENTORY_SLOTS) {
                        int item_id = s->inventory[req.slot].item_id;
                        if (item_id >= PC_ITEM_FOOD_BASE && item_id < PC_ITEM_FOOD_BASE + FOOD_ITEM_COUNT) {
                            int consumed = pc_try_remove_item_from_inventory(s->inventory, req.slot);
                            if (consumed == PC_ITEM_FOOD_BASE + FOOD_CAKE) {
                                server_smash_cake(now_ms());
                                printf("S536-CAKE: player%d smashed the cake\n", i);
                            } else if (consumed != PC_ITEM_NONE) {
                                printf("S536-ITEM: player%d ate %s -- no heal system yet, named gap\n",
                                       i, food_item_name(consumed - PC_ITEM_FOOD_BASE));
                            }
                            send_inventory_update(sock, s);
                        }
                    }
                    break;
                }
            } else if (hdr.type == PC_PACKET_INTERACT && (size_t)n >= sizeof(PcInteractPacket)) {
                /* Real "punch/interact" -- the minimal real input needed to exercise the already-
                   built Paper Engine live, without inventing a real combat system this sandbox
                   doesn't have yet. Hit point is derived from the player's own real position+yaw,
                   not aimed data in the packet -- Phase 0's own "smallest real proof point" bar. */
                for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                    PlayerSlot *s = &g_slots[i];
                    if (!s->active || s->addr.sin_addr.s_addr != from.sin_addr.s_addr ||
                        s->addr.sin_port != from.sin_port) {
                        continue;
                    }
                    PaperVec3 hit_world = paper_vec3(
                        s->state.x + sinf(s->state.yaw) * PC_INTERACT_REACH,
                        s->state.y,
                        s->state.z + cosf(s->state.yaw) * PC_INTERACT_REACH);

                    /* Real nearest-object-in-range pick: among every real, active world object
                       (packages/common/papercraft_worldobjects.h), the one whose own real center
                       is both within a real reasonable reach of the derived hit point AND closest
                       to it takes the hit. Simple, real, correct for a small, bounded object
                       count -- a real spatial index is later work once PC_WO_MAX_OBJECTS grows
                       past "linear scan is obviously fine." */
                    int target = -1;
                    float best_dist2 = 0.0f;
                    for (int o = 0; o < g_wo_file.count; o++) {
                        float dx = hit_world.x - g_wo_file.objects[o].x;
                        float dy = hit_world.y - g_wo_file.objects[o].y;
                        float dz = hit_world.z - g_wo_file.objects[o].z;
                        float dist2 = dx * dx + dy * dy + dz * dz;
                        /* Real, conservative reach check for a real non-uniform box -- use the
                           largest of the three real per-axis half-extents, not just one axis, so
                           a real wide/tall wall slab's own far edge stays reachable even though
                           its own thin axis is much smaller. */
                        float half_max = g_wo_file.objects[o].half_x;
                        if (g_wo_file.objects[o].half_y > half_max) half_max = g_wo_file.objects[o].half_y;
                        if (g_wo_file.objects[o].half_z > half_max) half_max = g_wo_file.objects[o].half_z;
                        float max_reach = half_max + PC_INTERACT_RADIUS + 0.5f;
                        if (dist2 <= max_reach * max_reach && (target == -1 || dist2 < best_dist2)) {
                            target = o;
                            best_dist2 = dist2;
                        }
                    }
                    if (target >= 0) {
                        /* Translate into this object's own local mesh space -- paper_mesh_damage_radius
                           operates in the same untranslated space paper_generate_cube built it in. */
                        PaperVec3 hit_local = paper_vec3(hit_world.x - g_wo_file.objects[target].x,
                                                          hit_world.y - g_wo_file.objects[target].y,
                                                          hit_world.z - g_wo_file.objects[target].z);

                        /* Real Phase 1a: snapshot fragment states BEFORE damage so we can tell
                           exactly which real fragments transitioned to GONE this hit --
                           paper_mesh_damage_radius only returns a real count, not indices, same
                           real "diff before vs after" technique apps/client's own debris-spawn
                           logic already uses, now also done server-side for the real,
                           authoritative version. */
                        unsigned char before_state[PC_WO_FRAGMENTS];
                        for (int f = 0; f < g_wo_mesh[target].fragment_count && f < PC_WO_FRAGMENTS; f++) {
                            before_state[f] = (unsigned char)g_wo_mesh[target].fragments[f].state;
                        }

                        int newly_gone = paper_mesh_damage_radius(&g_wo_mesh[target], hit_local, PC_INTERACT_RADIUS, PC_INTERACT_DAMAGE);
                        if (newly_gone > 0) {
                            printf("Player slot %d punched world object %d -- %d fragment(s) broke off.\n", i, target, newly_gone);
                            for (int f = 0; f < g_wo_mesh[target].fragment_count && f < PC_WO_FRAGMENTS; f++) {
                                if (before_state[f] != PAPER_STATE_GONE &&
                                    g_wo_mesh[target].fragments[f].state == PAPER_STATE_GONE) {
                                    spawn_falling_fragment(target, f);
                                }
                            }
                        }

                        /* Real "destroyed a world object" event -- packages/simulation/xp_award_mod.c
                           decides the real reward (ported from the construct's own real per-kill
                           XP), this host code only detects the real transition (every fragment now
                           PAPER_STATE_GONE) and applies it once per object via the real latch
                           above, matching every other "mod decides, host applies" split in this
                           monorepo. This is PAPERCRAFT's own first real worked mod-authoring
                           example -- see MODDING.md.

                           This is also this repo's own first real apps/server call site that
                           prefers a dynamically-loaded mod over the statically-linked one -- real
                           call-site policy documented on g_mod_registry's own header comment: look
                           the function up by name, call it if a real mod registered under that
                           exact name (--mods-manifest was given and this specific mod loaded),
                           otherwise fall back to the exact same statically-linked call this repo
                           has always made. Either path awards the real, correct reward -- the
                           dynamically-loaded .so is built from the EXACT same generated C as the
                           statically-linked function, not a different implementation. */
                        if (!g_wo_destroyed_awarded[target]) {
                            int gone_count = 0;
                            for (int f = 0; f < g_wo_mesh[target].fragment_count; f++) {
                                if (g_wo_mesh[target].fragments[f].state == PAPER_STATE_GONE) gone_count++;
                            }
                            if (gone_count == g_wo_mesh[target].fragment_count) {
                                g_wo_destroyed_awarded[target] = 1;
                                void *dyn = mod_registry_lookup("on_papercraft_xp_for_object_destroyed");
                                int reward;
                                const char *source;
                                if (dyn) {
                                    I32Fn0 fn = (I32Fn0)dyn;
                                    reward = fn();
                                    source = "dynamically-loaded";
                                } else {
                                    reward = on_papercraft_xp_for_object_destroyed();
                                    source = "statically-linked";
                                }
                                award_xp(s, reward, i);
                                printf("Player slot %d destroyed world object %d -- +%d real xp_award_mod XP (%s).\n",
                                       i, target, reward, source);

                                /* Real, first slice of TYLER/engine/tyler_phone_mechanics.md's
                                   "in-game smartphone system" spec (Phase 1: Messages app +
                                   notification banner only). Same real trigger event xp_award_mod
                                   already fires on -- packages/simulation/phone_mod.c
                                   (PARENA/stdlib/papercraft/phone_mod.prn) decides whether this
                                   event produces a notification and which message_id, this host
                                   code only applies it: a real PcPhoneMessagePacket sent once to
                                   the destroying player, same "mod decides, host applies" split
                                   as the XP award just above. Not run through mod_registry_lookup
                                   -- no dynamically-loaded variant of this mod exists yet, unlike
                                   xp_award_mod's own real dlopen/dlsym proof of concept -- a real,
                                   separate, later follow-up if this mod ever needs that. */
                                int msg_id = on_papercraft_phone_message_for_event(PC_PHONE_EVENT_OBJECT_DESTROYED);
                                if (msg_id != 0) {
                                    PcPhoneMessagePacket pm;
                                    memset(&pm, 0, sizeof(pm));
                                    pm.hdr.type = PC_PACKET_PHONE_MESSAGE;
                                    pm.hdr.client_id = (unsigned char)i;
                                    pm.message_id = (unsigned char)msg_id;
                                    sendto(sock, &pm, sizeof(pm), 0, (struct sockaddr *)&s->addr, s->addr_len);
                                }

                                /* Real, GTA3-style item drop -- PAPERCRAFT's own first real world
                                   entity (founder real-time, 2026-08-30: "gta3 style stuff drops
                                   and you can pick it up"). Same real trigger event xp_award_mod/
                                   phone_mod already fire on; packages/simulation/item_drop_mod.c
                                   (PARENA/stdlib/papercraft/item_drop_mod.prn) decides whether
                                   this real material drops an item and which one, this host code
                                   only finds a free real entity slot and broadcasts it -- same
                                   "mod decides, host applies" split as both real mods above. A
                                   real 0 (PC_ITEM_NONE) means no drop, matching every other
                                   material this sandbox has no real item for yet. */
                                int drop_item = on_papercraft_item_for_object_destroyed(g_wo_file.objects[target].material, target);
                                if (drop_item != PC_ITEM_NONE) {
                                    int slot_id = -1;
                                    for (int e = 0; e < PC_ENTITY_MAX; e++) {
                                        if (!g_entities[e].active) { slot_id = e; break; }
                                    }
                                    if (slot_id == -1) {
                                        printf("Real item drop suppressed -- PC_ENTITY_MAX (%d) already full.\n", PC_ENTITY_MAX);
                                    } else {
                                        g_entities[slot_id].active = 1;
                                        g_entities[slot_id].item_id = (unsigned char)drop_item;
                                        g_entities[slot_id].x = g_wo_file.objects[target].x;
                                        g_entities[slot_id].y = g_wo_file.objects[target].y;
                                        g_entities[slot_id].z = g_wo_file.objects[target].z;
                                        broadcast_entity_spawn(sock, slot_id);
                                        if (drop_item >= PC_ITEM_FOOD_BASE && drop_item < PC_ITEM_FOOD_BASE + FOOD_ITEM_COUNT) {
                                            printf("S536-FOOD: real item drop -- entity %d, %s (item_id=%d), at (%.1f,%.1f,%.1f).\n",
                                                   slot_id, food_item_name(drop_item - PC_ITEM_FOOD_BASE), drop_item,
                                                   g_entities[slot_id].x, g_entities[slot_id].y, g_entities[slot_id].z);
                                        } else {
                                            printf("Real item drop -- entity %d, item_id=%d, at (%.1f,%.1f,%.1f).\n",
                                                   slot_id, drop_item, g_entities[slot_id].x, g_entities[slot_id].y, g_entities[slot_id].z);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    break;
                }
            } else if (hdr.type == PC_PACKET_PHEROMONE_THROW && (size_t)n >= sizeof(PcPheromoneThrowPacket)) {
                /* S504-PHEROMONE: real client-driven command point. No sender validation beyond
                   the existing recv-side size check -- any connected client can drop a marker for
                   the whole crew, matching NORTHSTAR.md §7's own "one crew, one onboarding, shared
                   lab" co-op model (this is a shared crew tool, not a per-player one). */
                PcPheromoneThrowPacket req;
                memcpy(&req, buf, sizeof(req));
                server_throw_pheromone(req.x, req.y, req.z, now_ms());
            } else if (hdr.type == PC_PACKET_WHEELBARROW_TOGGLE && (size_t)n >= sizeof(PcWheelbarrowTogglePacket)) {
                /* SECTION 536 reverse-port phase 4 -- resolve the sender the same way
                   PC_PACKET_INTERACT does (addr-matched connected slot), then hand off to the
                   real, independently-testable toggle logic. */
                int requester = -1;
                for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                    PlayerSlot *rs = &g_slots[i];
                    if (rs->active && rs->addr.sin_addr.s_addr == from.sin_addr.s_addr &&
                        rs->addr.sin_port == from.sin_port) {
                        requester = i;
                        break;
                    }
                }
                if (requester >= 0) server_wheelbarrow_toggle(requester);
            }
        }

        unsigned int now = now_ms();
        if (now - last_tick_ms >= tick_ms) {
            last_tick_ms = now;
            server_tick++;
            server_tick_npcs(now);
            server_tick_giant_bugs(now);
            server_tick_wheelbarrow();
            server_tick_witness();
            server_tick_dispatch(now);
            server_tick_decorum(now);
            server_tick_regulators(now);

            for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                PlayerSlot *s = &g_slots[i];
                if (!s->active) continue;

                /* Real, genuine-abandonment timeout -- closes the real gap this always-running
                   persistent server had no defense against: a crashed/closed client leaves no
                   real disconnect packet (UDP has none), so without this the slot stayed
                   active()==1 forever. Real, final autosave before freeing the slot -- same real
                   save_player call the graceful-shutdown path already uses, so a real timed-out
                   player's own progress isn't lost, just like any other real save. A real
                   reconnect within PC_PLAYER_TIMEOUT_MS still works exactly as before (the
                   existing reconnect-by-player_id CONNECT-handler lookup), this only fires once
                   that real window has genuinely closed. */
                if (now - s->last_usercmd_ms > PC_PLAYER_TIMEOUT_MS) {
                    printf("Player slot %d timed out (no real packet in %ums) -- saving and freeing the slot.\n",
                           i, PC_PLAYER_TIMEOUT_MS);
                    save_player(s);
                    s->active = 0;
                    continue;
                }

                if (now - s->last_usercmd_ms > PC_USERCMD_STALE_MS) {
                    s->latest_move_x = 0.0f;
                    s->latest_move_z = 0.0f;
                }

                /* Real, GTA3-style walk-over pickup -- no dedicated pickup button/packet, matching
                   the founder's own real reference ("gta3 style stuff drops and you can pick it
                   up" -- GTA3's own real health/armor pickups work the same way). Every real
                   active entity within g_pickup_radius (on_papercraft_pickup_radius_millis, see
                   its own doc comment) of this player gets picked up this tick, real inventory
                   permitting -- try_add_item_to_inventory's own real "mod decides, host applies"
                   split covers stacking/capacity; a genuinely full real inventory leaves the real
                   entity where it is (no silent deletion) rather than losing the item. */
                for (int e = 0; e < PC_ENTITY_MAX; e++) {
                    if (!g_entities[e].active) continue;
                    float edx = s->state.x - g_entities[e].x;
                    float edy = s->state.y - g_entities[e].y;
                    float edz = s->state.z - g_entities[e].z;
                    float edist2 = edx * edx + edy * edy + edz * edz;
                    if (edist2 <= g_pickup_radius * g_pickup_radius) {
                        if (try_add_item_to_inventory(s, g_entities[e].item_id)) {
                            printf("Player slot %d picked up real entity %d (item_id=%d).\n",
                                   i, e, g_entities[e].item_id);
                            g_entities[e].active = 0;
                            broadcast_entity_despawn(sock, e);
                            send_inventory_update(sock, s);

                            /* Real "arsenal" unlock (2026-09-07): a picked-up item that's
                               actually a weapon (on_papercraft_weapon_item_slot's own real
                               mapping, -1 for anything that isn't) grants real, permanent access
                               to that weapon slot -- see PlayerSlot::weapons_owned's own doc
                               comment for why this is a permanent unlock, not a consumable. */
                            int wpn_slot = on_papercraft_weapon_item_slot(g_entities[e].item_id);
                            if (wpn_slot >= 0) {
                                s->weapons_owned |= (1u << wpn_slot);
                                printf("Player slot %d found weapon slot %d (weapons_owned now 0x%x).\n",
                                       i, wpn_slot, s->weapons_owned);
                                send_weapon_owned_update(sock, s);
                            }
                        }
                    }
                }

                /* Real, live redesign (2026-09-02, founder real-time: "make movement relative to
                   the camera... check the way that the shankpit construct works... it has an
                   example of how 3rd person should work"). local_x/local_z are the client's own
                   LOCAL fwd/strafe input (packages/common/papercraft_protocol.h's own
                   PcUserCmdPacket comment); rotate them by the player's own transmitted `yaw`
                   into world-space mx/mz here, server-side, matching SHANKPIT_CONSTRUCT.txt's
                   real phys_update_player (packages/simulation/game_physics.h) -- `wish_x =
                   (fwd_x*fwd)+(right_x*strafe)`, `wish_z = (fwd_z*fwd)+(right_z*strafe)` with
                   Forward(yaw)=(sin(yaw),cos(yaw))/Right(yaw)=(cos(yaw),-sin(yaw)), the same
                   radian convention this file's own pre-existing atan2f(mx, mz) yaw derivation
                   already used, not SHANKPIT server's own separate degrees-negated variant (kept
                   consistent with this repo's own established math, not ported wholesale). */
                float local_x = s->latest_move_x, local_z = s->latest_move_z;
                if (local_x > 1.0f) local_x = 1.0f;
                if (local_x < -1.0f) local_x = -1.0f;
                if (local_z > 1.0f) local_z = 1.0f;
                if (local_z < -1.0f) local_z = -1.0f;
                float yaw = s->latest_yaw;
                float mx = local_z * sinf(yaw) + local_x * cosf(yaw);
                float mz = local_z * cosf(yaw) - local_x * sinf(yaw);

                /* Sprint (S504-10, SHANKPIT/PAPERCRAFT physics unification) -- read straight from
                   latest_buttons here, ahead of the `crouching` bool below (which only exists
                   after this point), because sprint and crouch are mutually exclusive: you cannot
                   sprint while crouched, matching every other game's own convention and this
                   client's own real Shift=sprint/Ctrl=crouch keybinding (Shift no longer also
                   binds crouch -- see apps/client/src/main.c's own real key-read comment). */
                int sprinting = ((s->latest_buttons & PC_BTN_SPRINT) != 0) &&
                                 ((s->latest_buttons & PC_BTN_CROUCH) == 0);
                float base_move_speed = sprinting ? PC_SPRINT_SPEED : PC_MOVE_SPEED;

                /* Real MOVE-stat gameplay consequence -- the real PARENA-compiled
                   on_papercraft_move_speed_boost_permille (packages/simulation/stat_effects_mod.c),
                   not a hand-rolled float formula here. Ported from the construct's own real
                   progression_apply_bonuses ("boost = 1.0 + 0.035 * move"), fixed-point
                   permille in the mod, one real float division here to turn it back into an
                   actual multiplier -- VS0 has no F32 params yet, same real ceiling every other
                   mod in this monorepo respects. Real slide-jump boost (see below) stacks
                   multiplicatively on top while its own real, timed window is still active --
                   both are legitimate, independent speed modifiers, and both now stack on top of
                   base_move_speed (walk or sprint) rather than PC_MOVE_SPEED directly. */
                float move_speed = base_move_speed * (float)on_papercraft_move_speed_boost_permille(s->state.ability[PC_ABILITY_MOVE]) / 1000.0f;
                if (now < s->speed_boost_until_ms) {
                    move_speed *= (float)s->speed_boost_permille / 1000.0f;
                }
                float horiz_speed = sqrtf(mx * mx + mz * mz) * move_speed;
                s->state.x += mx * move_speed * PC_TICK_DT;
                s->state.z += mz * move_speed * PC_TICK_DT;

                int crouching = (s->latest_buttons & PC_BTN_CROUCH) != 0;
                int jump_held = (s->latest_buttons & PC_BTN_JUMP) != 0;
                int fresh_jump_press = jump_held && !s->was_holding_jump;
                s->was_holding_jump = jump_held;

                /* Real jump + gravity physics (PC_GRAVITY/PC_JUMP_VELOCITY, see their own real
                   doc comment above) -- PAPERCRAFT's first vertical movement. A grounded fresh
                   jump press launches the player upward; while airborne, real gravity integrates
                   Y each tick until they land on the real block data's own ground height at their
                   current column. A player standing over open air (off the real, fixed chunk
                   grid, or a real gap between two loaded chunks) keeps falling under gravity with
                   no floor to catch them -- real, honest physics now that real physics exist,
                   not the old "freeze at last known Y" placeholder (which only ever applied to a
                   player who was never airborne in the first place). */
                int gx = (int)(s->state.x + 0.5f), gz = (int)(s->state.z + 0.5f);
                int ground_y_i = 0;
                int has_ground = pw_world_ground_height_at(&g_world, gx, gz, &ground_y_i);
                float ground_y = (float)ground_y_i;

                if (fresh_jump_press && s->on_ground) {
                    /* Real slide-jump trick, ported from SHANKPIT_CONSTRUCT.txt's own "PHASE 485:
                       TUNED SLIDE JUMP" -- a crouch+jump combo while already moving grants a
                       real, PARENA-decided, timed speed boost (packages/simulation/
                       slide_jump_mod.c). The gate (crouching, minimum speed, fresh press,
                       grounded) is real, simple host logic; the magnitude is the real mod's own
                       decision. */
                    if (crouching && horiz_speed > PC_SLIDE_JUMP_MIN_SPEED) {
                        int speed_milli = (int)(horiz_speed * 1000.0f);
                        int boost_permille = on_papercraft_slide_jump_boost_permille(speed_milli);
                        s->speed_boost_permille = boost_permille;
                        s->speed_boost_until_ms = now + PC_SLIDE_JUMP_BOOST_MS;
                        printf("Player slot %d landed a real slide-jump trick -- %d.%02dx speed for %dms\n",
                               i, boost_permille / 1000, (boost_permille % 1000) / 10, PC_SLIDE_JUMP_BOOST_MS);
                    }
                    s->vy = PC_JUMP_VELOCITY;
                    s->on_ground = 0;
                }

                if (!s->on_ground) {
                    s->vy -= PC_GRAVITY * PC_TICK_DT;
                    s->state.y += s->vy * PC_TICK_DT;
                    if (has_ground && s->vy <= 0.0f && s->state.y <= ground_y) {
                        s->state.y = ground_y;
                        s->vy = 0.0f;
                        s->on_ground = 1;
                    }
                } else if (has_ground) {
                    /* Real, honest, deliberately-kept limit: while grounded, walking onto a
                       column with a different real ground height (e.g. stepping off a raised
                       structure) still snaps instantly rather than triggering real fall physics
                       -- the same pre-existing "no movement physics beyond basic collision"
                       simplification this repo has used since Phase 0, now scoped precisely to
                       "walking never falls, only an explicit jump does." A real, later
                       improvement (detect a real step-down bigger than some threshold and enter
                       on_ground=0 instead of snapping) is a genuine, separate piece of work, not
                       done here -- this pass's own real scope is jump + the slide-jump trick. */
                    s->state.y = ground_y;
                }

                /* Real, live redesign (2026-09-02, see this file's own movement-tick comment
                   above): facing used to be DERIVED from movement direction (atan2f(mx, mz),
                   only updated while actually moving) -- real, live symptom this caused, matching
                   SHANKPIT_CONSTRUCT.txt's own real phys_update_player's opposite choice
                   (`p->yaw = yaw`, always, straight from the transmitted camera yaw): strafing
                   or backpedaling spun the character to face its direction of travel instead of
                   continuing to face the camera, and PC_PACKET_INTERACT's own real reach
                   direction (sinf/cosf(state.yaw) below) inherited that same wrong-facing bug --
                   the real cause the founder separately named as "the combat is jacked up" even
                   with no real combat built yet. Facing is now unconditional and decoupled from
                   movement entirely, exactly like SHANKPIT's own reference model. */
                s->state.yaw = yaw;

                /* Real, passive per-second XP tick, matching the construct's own real
                   progression_tick cadence (SHANKPIT_CONSTRUCT.txt lines 851-856:
                   progression_add_xp(5) once every 1000ms). No combat/quests to award XP from
                   in this sandbox (see NORTHSTAR.md's own "no quest system... building the
                   sandbox to start"), so passive time-in-world is the real, honest source here --
                   the same "time played rewards" convention a real sandbox MMO already leans on. */
                if (s->last_xp_tick_ms == 0) s->last_xp_tick_ms = now;
                if (s->state.level < 20 && now - s->last_xp_tick_ms >= PC_XP_TICK_MS) {
                    s->last_xp_tick_ms = now;
                    award_xp(s, PC_XP_PER_TICK, i);
                }

                /* Real periodic autosave -- bounded, real worst-case data loss on a crash (not a
                   clean shutdown, which flushes everyone immediately below). */
                if (s->last_save_ms == 0) s->last_save_ms = now;
                if (now - s->last_save_ms >= PC_AUTOSAVE_MS) {
                    s->last_save_ms = now;
                    save_player(s);
                }
            }

            /* Real periodic world-object damage autosave -- same real cadence/bounded-staleness
               tradeoff as the per-player autosave above, but a single, shared timer (damage isn't
               per-player). */
            if (g_last_world_save_ms == 0) g_last_world_save_ms = now;
            if (now - g_last_world_save_ms >= PC_AUTOSAVE_MS) {
                g_last_world_save_ms = now;
                save_world_damage();
            }

            /* Real Phase 1a per-tick integration -- server-authoritative fragment physics
               (NORTHSTAR.md's own "Real Phase 1" section). Vertical-only: real gravity
               (PC_GRAVITY, the exact same real constant player jump physics already uses, not
               reinvented), real ground-height landing detection via the exact same real
               pw_world_ground_height_at player movement already calls every tick. Once landed, a
               real fragment's own slot frees immediately (set active=0) -- Phase 1a's own real,
               explicit scope is proving the fall, not a permanent rubble-pile system. */
            for (int fi = 0; fi < PC_FALLING_FRAGMENTS_MAX; fi++) {
                if (!g_falling[fi].active) continue;
                g_falling[fi].vy -= PC_GRAVITY * PC_TICK_DT;
                g_falling[fi].y += g_falling[fi].vy * PC_TICK_DT;
                /* Real Phase 1c -- real, constant-rate spin, integrated the same real way as
                   every other real per-tick quantity in this loop. */
                g_falling[fi].rotation_deg += g_falling[fi].angular_velocity_deg_s * PC_TICK_DT;
                int ground_y_i;
                int has_ground = pw_world_ground_height_at(&g_world, (int)(g_falling[fi].x + 0.5f),
                                                             (int)(g_falling[fi].z + 0.5f), &ground_y_i);
                if (has_ground && g_falling[fi].y <= (float)ground_y_i) {
                    g_falling[fi].active = 0;
                }
                /* Real, honest edge case, not a special case invented here: no real ground data
                   at this column (e.g. a fragment detached near a real chunk-grid boundary) keeps
                   the real fragment falling under real gravity with no floor to catch it -- the
                   exact same real "no floor, keep falling" contract player movement's own real
                   physics already uses. */
            }

            /* Real snapshot broadcast to every active real player -- rate-limited to
               PC_SNAPSHOT_HZ, decoupled from the full PC_TICK_HZ simulation rate above (see
               PC_SNAPSHOT_HZ's own doc comment for the real, live bandwidth reasoning). */
            if (server_tick % (PC_TICK_HZ / PC_SNAPSHOT_HZ) == 0) {
            PcSnapshotPacket snap;
            memset(&snap, 0, sizeof(snap));
            snap.hdr.type = PC_PACKET_SNAPSHOT;
            snap.hdr.sequence = server_tick;
            snap.server_tick = server_tick;
            for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                snap.active[i] = (unsigned char)g_slots[i].active;
                snap.players[i] = g_slots[i].state;
            }
            /* Real world-object broadcast -- position/material/seed (so the client can
               independently regenerate each active object's own identical real geometry) plus
               only the per-fragment STATE, not geometry, the same real "seed + deltas, not the
               whole mesh" shape paper_mesh.h's own doc comment already named as the real
               target. */
            for (int o = 0; o < PC_WO_MAX_OBJECTS; o++) {
                snap.world_object_active[o] = (o < g_wo_file.count) ? 1 : 0;
                if (o < g_wo_file.count) {
                    snap.world_objects[o] = g_wo_file.objects[o];
                    for (int f = 0; f < g_wo_mesh[o].fragment_count && f < PC_WO_FRAGMENTS; f++) {
                        pc_wo_state_pack(snap.world_object_state[o], f, g_wo_mesh[o].fragments[f].state);
                    }
                }
            }
            /* Real Phase 1a broadcast -- only y crosses the wire (see PcFallingFragment's own doc
               comment for why x/z don't need to). */
            for (int fi = 0; fi < PC_FALLING_FRAGMENTS_MAX; fi++) {
                snap.falling_active[fi] = (unsigned char)g_falling[fi].active;
                if (g_falling[fi].active) {
                    snap.falling[fi].object_idx = (unsigned char)g_falling[fi].object_idx;
                    snap.falling[fi].fragment_idx = (unsigned char)g_falling[fi].fragment_idx;
                    snap.falling[fi].y = g_falling[fi].y;
                    snap.falling[fi].rotation_deg = g_falling[fi].rotation_deg;
                }
            }
            /* S504 §8c real NPC broadcast -- position/role only, matching PcNpcState's own
               "server decides, client renders" wire-lean doc comment; brain/mood state never
               crosses the wire. */
            for (int ni = 0; ni < PC_NPC_MAX; ni++) {
                snap.npc_active[ni] = (unsigned char)g_npcs[ni].active;
                if (g_npcs[ni].active) {
                    snap.npcs[ni].x = g_npcs[ni].x;
                    snap.npcs[ni].y = g_npcs[ni].y;
                    snap.npcs[ni].z = g_npcs[ni].z;
                    snap.npcs[ni].yaw = g_npcs[ni].yaw;
                    snap.npcs[ni].role = g_npcs[ni].role;
                }
            }
            /* SECTION 536 reverse-port follow-up: the first client visual for either giant bugs
               (reverse-port phase 3) or Regulators (Phase B) -- both were real, live, server-only
               since they landed, same "server logic first, client visual later" precedent every
               mechanic in this thread has used, now caught up for these two. Position only, same
               wire-lean discipline as the NPC loop just above. */
            for (int bi = 0; bi < BIGO_GIANT_BUG_MAX; bi++) {
                snap.giant_bug_active[bi] = (unsigned char)g_giant_bugs[bi].active;
                if (g_giant_bugs[bi].active) {
                    snap.giant_bugs[bi].x = g_giant_bugs[bi].x;
                    snap.giant_bugs[bi].y = g_giant_bugs[bi].y;
                    snap.giant_bugs[bi].z = g_giant_bugs[bi].z;
                }
            }
            for (int ri = 0; ri < BIGO_REGULATOR_MAX; ri++) {
                snap.regulator_active[ri] = (unsigned char)g_regulators[ri].active;
                if (g_regulators[ri].active) {
                    snap.regulators[ri].x = g_regulators[ri].x;
                    snap.regulators[ri].y = g_regulators[ri].y;
                    snap.regulators[ri].z = g_regulators[ri].z;
                }
            }
            for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                if (!g_slots[i].active) continue;
                snap.hdr.client_id = (unsigned char)i;
                /* Real, per-recipient overwrite of the one shared echo field -- see
                   PcSnapshotPacket::echo_cmd_time_ms's own doc comment for why this is a single
                   reused field, not a real per-player array. */
                snap.echo_cmd_time_ms = g_slots[i].latest_cmd_time_ms;
                g_slots[i].snaps_tx++;
                if (g_slots[i].lz4) {
                    unsigned char wire[sizeof(PcSnapshotLz4Header) + sizeof(PcSnapshotPacket)];
                    int cn = lz4m_compress((const unsigned char *)&snap, (int)sizeof(snap), wire + sizeof(PcSnapshotLz4Header), (int)sizeof(PcSnapshotPacket));
                    if (cn > 0) {
                        PcSnapshotLz4Header lh; memset(&lh, 0, sizeof(lh));
                        lh.hdr.type = PC_PACKET_SNAPSHOT_LZ4; lh.hdr.client_id = snap.hdr.client_id; lh.hdr.sequence = snap.hdr.sequence;
                        lh.raw_len = (unsigned short)sizeof(snap); lh.comp_len = (unsigned short)cn;
                        memcpy(wire, &lh, sizeof(lh));
                        sendto(sock, wire, sizeof(lh) + (size_t)cn, 0, (struct sockaddr *)&g_slots[i].addr, g_slots[i].addr_len);
                        continue;
                    }
                }
                sendto(sock, &snap, sizeof(snap), 0, (struct sockaddr *)&g_slots[i].addr, g_slots[i].addr_len);
            }
            }

            if (server_tick % (PC_TICK_HZ * 10) == 0) {
                for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                    if (!g_slots[i].active) continue;
                    printf("[net] slot %d %s:%d lz4=%d usercmds_rx=%u snaps_tx=%u cmd_age_ms=%u (last 10s)\n", i, inet_ntoa(g_slots[i].addr.sin_addr),
                           ntohs(g_slots[i].addr.sin_port), g_slots[i].lz4, g_slots[i].usercmds_rx, g_slots[i].snaps_tx, now_ms() - g_slots[i].last_usercmd_ms);
                    g_slots[i].usercmds_rx = 0; g_slots[i].snaps_tx = 0;
                }
            }
            if (server_tick % (PC_TICK_HZ * 2) == 0 && g_slots[0].active) {
                printf("tick=%u player0=(%.2f,%.2f,%.2f)\n", server_tick,
                       g_slots[0].state.x, g_slots[0].state.y, g_slots[0].state.z);
            }
        } else {
            usleep(1000);
        }
    }

    close(sock);
    return 0;
}

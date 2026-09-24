#ifndef BIGO_FOOD_ITEMS_H
#define BIGO_FOOD_ITEMS_H

/* bigo_food_items.h -- real, reverse-ported from SHANKPIT's own packages/common/food_items.h
 * (EMILY/BACKLOG.md SECTION 536 follow-up, 2026-09-23). Pure data table, header-only, no GL/SDL/
 * network -- same "pure C, real, tested standalone" discipline this repo's own
 * bigo_walkie_talkie.h/giant_bug_values.h already used for phases 1-3 of this reverse-port.
 *
 * Content, names, and point values are byte-for-byte identical to SHANKPIT's own table (17 real
 * items: the 8 classic Pac-Man bonus fruits + Ms. Pac-Man's 3 real additions + 5 BIG_O-original
 * hard-sci-fi items + the real, founder-requested BIRTHDAY CAKE add-on). item_id here is the
 * FoodItemId enum index (0..16); the real wire item id is PC_ITEM_FOOD_BASE + this index
 * (papercraft_protocol.h) -- callers translate at the boundary, this header stays wire-agnostic
 * like SHANKPIT's own copy did.
 *
 * Real, honest, named gap (see papercraft_protocol.h's own PC_ITEM_FOOD_BASE doc comment): these
 * items are real, live, pickable/stackable cargo (day/apps/server/src/main.c's own existing
 * GTA3-style pickup path, unchanged) -- food_item_heal() below is ported and tested, but nothing
 * in this repo calls it yet. BIG_O's PlayerSlot has no health field and no damage source exists
 * anywhere (checked directly -- only destructible world-object fragment HP), so "eat for heal"
 * has no observable effect to wire it to yet. Not guessed at here -- a real damage-source design
 * decision (giant bugs attacking players? zombie NPC aggression? PvP?) is genuinely undecided and
 * out of scope for this pass. Same "standalone primitive, no live consumer yet" precedent
 * bigo_walkie_talkie.h already established for this exact reverse-port thread.
 *
 * FOOD_MINESTRONE (item 18) -- founder real-time, 2026-09-24: "add ministrone to shankpit and
 * bigo" (minestrone), mirrored here from SHANKPIT's own same-day FOOD_MINESTRONE addition,
 * same reverse-port discipline this whole file already follows. Same named gap as every other
 * item here: real cargo, no live heal consumer yet. */

#define FOOD_ITEM_COUNT 18

typedef enum {
    FOOD_CHERRY = 0,
    FOOD_STRAWBERRY,
    FOOD_PRETZEL,
    FOOD_ORANGE,
    FOOD_APPLE,
    FOOD_PEAR,
    FOOD_BANANA,
    FOOD_MELON,
    FOOD_GALAXIAN,
    FOOD_BELL,
    FOOD_KEY,
    FOOD_COFFEE,
    FOOD_DONUT,
    FOOD_ENERGY_BAR,
    FOOD_RATION_PACK,
    FOOD_SYNTH_MEAT,
    FOOD_CAKE,
    FOOD_MINESTRONE
} FoodItemId;

static const char *const FOOD_ITEM_NAMES[FOOD_ITEM_COUNT] = {
    "CHERRY", "STRAWBERRY", "PRETZEL", "ORANGE", "APPLE", "PEAR", "BANANA", "MELON",
    "GALAXIAN", "BELL", "KEY", "COFFEE", "DONUT", "ENERGY BAR", "RATION PACK", "SYNTH-MEAT",
    "BIRTHDAY CAKE", "MINESTRONE"
};

static const int FOOD_ITEM_POINTS[FOOD_ITEM_COUNT] = {
    100, 200, 300, 500, 700, 1000, 1500, 2000,
    3000, 4000, 5000, 100, 250, 750, 1250, 2500,
    1800, 1600
};

/* Real, derived, not hardcoded per-item -- ported unchanged from SHANKPIT's own formula so a
 * future real heal wiring gets the exact same balance without re-deriving it. */
static inline int food_item_heal(int item_id) {
    if (item_id < 0 || item_id >= FOOD_ITEM_COUNT) return 0;
    int heal = FOOD_ITEM_POINTS[item_id] / 100;
    if (heal < 5) heal = 5;
    if (heal > 50) heal = 50;
    return heal;
}

static inline const char *food_item_name(int item_id) {
    return (item_id >= 0 && item_id < FOOD_ITEM_COUNT) ? FOOD_ITEM_NAMES[item_id] : "?";
}

static inline int food_item_points(int item_id) {
    return (item_id >= 0 && item_id < FOOD_ITEM_COUNT) ? FOOD_ITEM_POINTS[item_id] : 0;
}

#endif

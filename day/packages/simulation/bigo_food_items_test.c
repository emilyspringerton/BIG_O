/* bigo_food_items_test.c -- real, standalone coverage for
 * day/packages/common/bigo_food_items.h's own pure data table (EMILY/BACKLOG.md SECTION 536
 * reverse-port phase 5, 2026-09-23). */
#include "../common/bigo_food_items.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    assert(FOOD_ITEM_COUNT == 17);

    assert(strcmp(food_item_name(FOOD_CHERRY), "CHERRY") == 0);
    assert(strcmp(food_item_name(FOOD_CAKE), "BIRTHDAY CAKE") == 0);
    assert(strcmp(food_item_name(-1), "?") == 0);
    assert(strcmp(food_item_name(FOOD_ITEM_COUNT), "?") == 0);

    assert(food_item_points(FOOD_CHERRY) == 100);
    assert(food_item_points(FOOD_KEY) == 5000); /* highest of the 8 classic Pac-Man fruits */
    assert(food_item_points(-1) == 0);

    /* heal = points/100, clamped 5..50. */
    assert(food_item_heal(FOOD_CHERRY) == 5);      /* 100/100=1, clamped up to floor 5 */
    assert(food_item_heal(FOOD_COFFEE) == 5);      /* 100/100=1, clamped up to floor 5 */
    assert(food_item_heal(FOOD_ENERGY_BAR) == 7);  /* 750/100=7, no clamp */
    assert(food_item_heal(FOOD_KEY) == 50);        /* 5000/100=50, exactly the ceiling */
    assert(food_item_heal(FOOD_GALAXIAN) == 30);   /* 3000/100=30, no clamp */
    assert(food_item_heal(-1) == 0);
    assert(food_item_heal(FOOD_ITEM_COUNT) == 0);

    printf("bigo_food_items_test: all assertions passed\n");
    return 0;
}

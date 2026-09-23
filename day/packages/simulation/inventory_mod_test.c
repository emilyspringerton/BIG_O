/* inventory_mod_test.c -- real test for the real PARENA-compiled inventory_mod.c
 * (packages/simulation/inventory_mod.c, generated from BIG_O's own
 * PARENA/stdlib/big_o/inventory_mod.prn -- forked from papercraft's original, EMILY/BACKLOG.md
 * SECTION 536 reverse-port phase 5, 2026-09-23).
 */
#include <assert.h>
#include <stdio.h>

int on_papercraft_inventory_stack_max(int item_id);
int on_papercraft_inventory_can_stack(int existing_item_id, int incoming_item_id);

int main(void) {
    assert(on_papercraft_inventory_stack_max(1) == 99);  /* PC_ITEM_SCRAP -> real 99-stack cap */
    assert(on_papercraft_inventory_stack_max(0) == 0);   /* PC_ITEM_NONE -> not stackable */
    assert(on_papercraft_inventory_stack_max(99) == 0);  /* unrecognized item -> not stackable */

    /* PC_ITEM_FOOD_BASE=8 .. +16=24 -> real 20-stack cap; just outside that range stays 0. */
    assert(on_papercraft_inventory_stack_max(8) == 20);  /* FOOD_CHERRY */
    assert(on_papercraft_inventory_stack_max(24) == 20); /* FOOD_CAKE, last food id */
    assert(on_papercraft_inventory_stack_max(7) == 0);   /* PC_ITEM_WPN_KATANA, just below food range */
    assert(on_papercraft_inventory_stack_max(25) == 0);  /* just above food range */

    assert(on_papercraft_inventory_can_stack(1, 1) == 1);  /* same real item -> can merge */
    assert(on_papercraft_inventory_can_stack(1, 2) == 0);  /* different items -> cannot merge */
    assert(on_papercraft_inventory_can_stack(8, 8) == 1);  /* same real food item -> can merge */
    assert(on_papercraft_inventory_can_stack(8, 9) == 0);  /* different food items -> cannot merge */
    assert(on_papercraft_inventory_can_stack(0, 0) == 0);  /* empty slot is not a "stack" -> cannot */
    assert(on_papercraft_inventory_can_stack(0, 1) == 0);  /* empty existing slot -> not a merge case */
    printf("inventory_mod_test: all assertions passed\n");
    return 0;
}

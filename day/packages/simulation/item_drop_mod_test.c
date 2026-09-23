/* item_drop_mod_test.c -- real test for the real PARENA-compiled item_drop_mod.c
 * (packages/simulation/item_drop_mod.c, generated from BIG_O's own
 * PARENA/stdlib/big_o/item_drop_mod.prn -- forked from papercraft's original, EMILY/BACKLOG.md
 * SECTION 536 reverse-port phase 5, 2026-09-23). Material ids match
 * packages/common/paper_mesh.h's own PAPER_MATERIAL_* order (paper=0, wood=1, concrete=2,
 * metal=3); PC_ITEM_SCRAP=1, PC_ITEM_WPN_SHOTGUN=5, PC_ITEM_FOOD_BASE=8 match
 * packages/common/papercraft_protocol.h.
 */
#include <assert.h>
#include <stdio.h>

int on_papercraft_item_for_object_destroyed(int material, int object_index);

int main(void) {
    assert(on_papercraft_item_for_object_destroyed(0, 0) == 1); /* PAPER -> PC_ITEM_SCRAP, regardless of object_index */
    assert(on_papercraft_item_for_object_destroyed(0, 99) == 1);
    assert(on_papercraft_item_for_object_destroyed(2, 0) == 0); /* CONCRETE -> no drop yet */
    assert(on_papercraft_item_for_object_destroyed(3, 0) == 5); /* METAL -> PC_ITEM_WPN_SHOTGUN (2026-09-07: "you have to find a shotgun") */

    /* WOOD -> a real food item, id = PC_ITEM_FOOD_BASE + (object_index mod 17) -- deterministic
       variety across the real 17-item roster, no RNG. */
    assert(on_papercraft_item_for_object_destroyed(1, 0) == 8);   /* -> FOOD_CHERRY */
    assert(on_papercraft_item_for_object_destroyed(1, 1) == 9);   /* -> FOOD_STRAWBERRY */
    assert(on_papercraft_item_for_object_destroyed(1, 16) == 24); /* -> FOOD_CAKE, last of the 17 */
    assert(on_papercraft_item_for_object_destroyed(1, 17) == 8);  /* wraps back to FOOD_CHERRY */
    assert(on_papercraft_item_for_object_destroyed(1, 34) == 8);  /* wraps again */

    printf("item_drop_mod_test: all assertions passed\n");
    return 0;
}

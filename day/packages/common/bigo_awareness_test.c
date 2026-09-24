/* bigo_awareness_test.c -- real, direct coverage for bigo_awareness.h's own direction/compass/
 * intensity decisions (EMILY/BACKLOG.md SECTION 536 follow-up queued item 4, BIG_O/NORTHSTAR.md
 * §35). Pure math, no network/GL -- same discipline bigo_hoverboard_test.c already establishes.
 */
#include "bigo_awareness.h"
#include <math.h>
#include <stdio.h>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)
#define NEAR(a, b) (fabsf((a) - (b)) < 0.01f)

int main(void) {
    float dx, dz;

    /* direction: normalizes, preserves sign/ratio */
    bigo_awareness_direction(0.0f, 5.0f, &dx, &dz); CHECK(NEAR(dx, 0.0f) && NEAR(dz, 1.0f));
    bigo_awareness_direction(3.0f, 4.0f, &dx, &dz); CHECK(NEAR(dx, 0.6f) && NEAR(dz, 0.8f));   /* 3-4-5 triangle */
    bigo_awareness_direction(0.0f, 0.0f, &dx, &dz); CHECK(NEAR(dx, 0.0f) && NEAR(dz, 1.0f));   /* zero-length -> north, no NaN */

    /* compass: the 8 cardinal/ordinal directions land where the doc comment says */
    CHECK(bigo_awareness_compass(0.0f, 1.0f) == 0);    /* N */
    CHECK(bigo_awareness_compass(1.0f, 1.0f) == 1);    /* NE */
    CHECK(bigo_awareness_compass(1.0f, 0.0f) == 2);    /* E */
    CHECK(bigo_awareness_compass(1.0f, -1.0f) == 3);   /* SE */
    CHECK(bigo_awareness_compass(0.0f, -1.0f) == 4);   /* S */
    CHECK(bigo_awareness_compass(-1.0f, -1.0f) == 5);  /* SW */
    CHECK(bigo_awareness_compass(-1.0f, 0.0f) == 6);   /* W */
    CHECK(bigo_awareness_compass(-1.0f, 1.0f) == 7);   /* NW */
    CHECK(bigo_awareness_compass(0.0f, 0.0f) >= 0 && bigo_awareness_compass(0.0f, 0.0f) < 8);  /* degenerate input stays in range */

    /* intensity: bounds-checked, more witnesses raise it, capped at 100 */
    CHECK(bigo_awareness_intensity(-10, 1) == 0);       /* clamped low */
    CHECK(bigo_awareness_intensity(200, 1) == 100);     /* clamped high */
    CHECK(bigo_awareness_intensity(40, 1) == 40);       /* single witness: no scaling */
    CHECK(bigo_awareness_intensity(40, 3) == 52);       /* 40 * 1.30 = 52 */
    CHECK(bigo_awareness_intensity(90, 5) == 100);      /* 90 * 1.60 = 144, capped */
    CHECK(bigo_awareness_intensity(50, 0) == 50);       /* witness_count < 1 treated as 1, not 0 */

    if (fails == 0) printf("bigo_awareness_test: all checks passed\n");
    else printf("bigo_awareness_test: %d FAILURES\n", fails);
    return fails != 0;
}

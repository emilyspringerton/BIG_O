// npc_archetype.c -- see npc_archetype.h.
#include "npc_archetype.h"

void npc_brain_init(NpcBrain *b, NpcArchetype archetype, uint32_t now_ms) {
    b->archetype = archetype;
    humanness_state_init(&b->humanness, now_ms);
    if (archetype == NPC_ARCHETYPE_THE_MEN) {
        b->base_vigilance = 85;
        b->humanness.mood = HUMANNESS_MOOD_FOCUSED;
        b->humanness.energy = 0.9f;   // professional, on the clock, not dragging
        b->humanness.fatigue = 0.05f;
        b->humanness.boredom = 0.0f;  // dispatched with a job, not idling
        b->humanness.curiosity = 0.1f;
    } else {
        b->base_vigilance = 35;
        b->humanness.mood = HUMANNESS_MOOD_CURIOUS;
        b->humanness.curiosity = 0.5f;
        b->humanness.boredom = 0.3f;
    }
}

void npc_brain_tick(NpcBrain *b, uint32_t now_ms) {
    humanness_tick_mood(&b->humanness, now_ms);
}

void npc_brain_get_startled(NpcBrain *b, uint32_t now_ms) {
    humanness_get_startled(&b->humanness, now_ms);
}

int npc_brain_effective_vigilance(const NpcBrain *b) {
    float v = (float)b->base_vigilance;
    if (b->humanness.mood == HUMANNESS_MOOD_STARTLED) v += 25.0f;
    if (b->humanness.mood == HUMANNESS_MOOD_TIRED) v -= 15.0f;
    v -= b->humanness.fatigue * 20.0f;
    v -= (1.0f - b->humanness.energy) * 15.0f;
    if (v < 0.0f) v = 0.0f;
    if (v > 100.0f) v = 100.0f;
    return (int)(v + 0.5f);
}

uint32_t npc_brain_reaction_delay_ms(const NpcBrain *b, uint32_t base_ms) {
    return humanness_reaction_delay_ms(&b->humanness, base_ms);
}

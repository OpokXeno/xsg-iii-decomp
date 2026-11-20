/*
 * OV02 original TU 62: 0x00a9a978..0x00a9acf0 (7 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu062", func_00A9A978);

INCLUDE_ASM("asm/nonmatchings/ov02/tu062", func_00A9AAA8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu062", func_00A9AAC0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu062", func_00A9AAE0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu062", func_00A9AB58);

INCLUDE_ASM("asm/nonmatchings/ov02/tu062", func_00A9AB70);

#include "shared.h"
/* Number of child actor slots a battle unit keeps (unit->children, 4 words from +0x2B8). */
#define UNIT_CHILD_COUNT 4
/* Actor draw-flag bit that marks an actor as fading (set by the fade start, cleared by the fade end). */
#define ACTOR_STATE_FADING 0x10000
/* Fade progress per frame: 1/15 as a single-precision word (0x3D888889). */
#define FADE_DEFAULT_STEP (1.0f / 15.0f)

#include "main/xenoActor.h"
#include "ov02/tu060.h"
#include "ov02/tu062.h"



struct FadeTask *ov01TaskClaim(void);
void ov01TaskRelease(struct FadeTask *task);
/* * unitFadeOutStep (fade-out step): progress falls by step. At 0.0 or below the * unit's actor and every child actor go to alpha 0, the task is released and * unit->fade is cleared. Otherwise the progress is written as alpha to the * unit's actor and to each child actor. */
void unitFadeOutStep(struct FadeTask *task);

/*
 * unitFadeOutStep (fade-out step): progress falls by step. At 0.0 or below the
 * unit's actor and every child actor go to alpha 0, the task is released and
 * unit->fade is cleared. Otherwise the progress is written as alpha to the
 * unit's actor and to each child actor.
 */
void unitFadeOutStep(struct FadeTask *task) {
    struct BattleUnit *unit;
    float progress;
    int slotIndex;

    unit = task->unit;
    progress = task->progress - task->step;
    task->progress = progress;
    if (progress <= 0.0f) {
        unit->unit->alpha = 0.0f;
        for (slotIndex = 0; slotIndex < UNIT_CHILD_COUNT; slotIndex++) {
            if (unit->children[slotIndex] != 0) {
                unit->children[slotIndex]->alpha = 0.0f;
            }
        }
        ov01TaskRelease(task);
        unit->fade = 0;
        return;
    }
    unit->unit->alpha = progress;
    for (slotIndex = 0; slotIndex < UNIT_CHILD_COUNT; slotIndex++) {
        if (unit->children[slotIndex] != 0) {
            unit->children[slotIndex]->alpha = task->progress;
        }
    }
}

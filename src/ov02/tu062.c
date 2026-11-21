/*
 * OV02 original TU 62: 0x00a9a978..0x00a9acf0 (7 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu062", func_00A9A978);

typedef signed char s8;
typedef int s32;

/* The counter record counterReachesLimit reads: the count is at +0x24. The record's own type is not claimed here. */
typedef struct CounterRecord {
    unsigned char unmodeled_00[0x24];
    s32 count;                          /* +0x24 */
} CounterRecord;

/* The limit record counterReachesLimit reads: the limit is at +0x08. */
typedef struct LimitRecord {
    unsigned char unmodeled_00[0x8];
    s32 limit;                          /* +0x08 */
} LimitRecord;

/* Returns 1 when the count of counter has reached the limit of limit, 0 while it is still below. */
s32 counterReachesLimit(CounterRecord *counter, LimitRecord *limit)
{
  if (counter->count < limit->limit)
  {
    return 0;
  }
  return 1;
}

s32 monsSessionTryEnd();

s32 unitTryStartPending(void) {
    return monsSessionTryEnd() != 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu062", func_00A9AAE0);

s32 func_00A9E4D8();

void unitPollBusyState(void) {
    func_00A9E4D8();
}

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



struct FadeTask *battleTaskClaim(void);
void battleTaskRelease(struct FadeTask *task);
void unitFadeOutStep(struct FadeTask *task);

/*
 * unitFadeOutStep (fade-out step): progress falls by step. At 0.0 or below the
 * unit's actor and every child actor go to alpha 0, the task is released and
 * unit->fade is cleared. Otherwise the progress is written as alpha to the
 * unit's actor and to each child actor.
 */
void unitFadeOutStep(struct FadeTask *task) {
    struct BattleUnit *battleUnit;
    float progress;
    int childIndex;

    battleUnit = task->unit;
    progress = task->progress - task->step;
    task->progress = progress;
    if (progress <= 0.0f) {
        battleUnit->motionActor->alpha = 0.0f;
        for (childIndex = 0; childIndex < UNIT_CHILD_COUNT; childIndex++) {
            if (battleUnit->children[childIndex] != 0) {
                battleUnit->children[childIndex]->alpha = 0.0f;
            }
        }
        battleTaskRelease(task);
        battleUnit->fade = 0;
        return;
    }
    battleUnit->motionActor->alpha = progress;
    for (childIndex = 0; childIndex < UNIT_CHILD_COUNT; childIndex++) {
        if (battleUnit->children[childIndex] != 0) {
            battleUnit->children[childIndex]->alpha = task->progress;
        }
    }
}

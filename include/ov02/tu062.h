/* header_harvest: hand-maintained (X3 express integration; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_OV02_TU062_H
#define INCLUDE_OV02_TU062_H

/* Types owned by ov02/tu062 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"
#include "ov02/tu060.h"

/*
 * Task record of a fade. The fields from +0xC on are the payload of the task record, a union because each kind of
 * task reads it its own way: a unit fade keeps the unit at +0xC (unitFadeTo also a target), a unit slot fade the
 * actor, a monster slot fade (monsSlotFadeInStart/OutStart) its slot owner (screen_flash.c's screen flash keeps a frame
 * counter there, see ScreenTask).
 */
typedef struct FadeTask FadeTask;

/* The 0x1C-byte task record battleTaskClaim hands out, with the fade payload at +0x0C. */
struct FadeTask {
    int flags;                          /* +0x00 */
    void (*update)(FadeTask *task);     /* +0x04: the step callback the task runs */
    void (*draw)(void);                 /* +0x08: no draw callback for a fade */
    struct BattleUnit *unit;            /* +0x0C: the unit being faded (a unit fade) */
    float progress;                     /* +0x10: moved by step each frame */
    float step;                         /* +0x14 */
    float target;                       /* +0x18: unitFadeTo only */
};

#endif

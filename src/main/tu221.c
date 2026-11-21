#include "common.h"
#include "shared.h"

#include "main/gifState.h"

extern GifStatusBlock gifStatus;
/* Scaffold data without a config name (splat names): the per-mode table of pointer-sized entries, and the
 * word the selected entry is copied into. */
extern void *gifStateEffectDraws[];
extern void *gifStateModeHook[];

void gifStateSetMode(int effectMode)
{
    if (effectMode < 6) {
        gifStateModeHook[0] = gifStateEffectDraws[effectMode];
        gifStatus.state = effectMode;
        gifStatus.ratio = 1.0f;
    }
}

void gifStateClearModeHook(void) {
    gifStateModeHook[0] = 0;
}

void gifStateNoOp(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu221", func_00258B90);

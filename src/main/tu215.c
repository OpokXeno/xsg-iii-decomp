#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu215", func_00252358);

typedef int s32;
extern s32 D_003E8B10[];
s32 func_00252358(s32 *);

void xenoActorEventTableInit(void) {
    func_00252358(D_003E8B10);
}

s32 soundSlotStop(s32);
s32 soundSlotSetPause(s32 index, s32 value);
s32 soundSlotSetVolume(s32 index, s32 level, s32 extra);

void eventViewerSilenceSound(void) {
    soundSlotSetPause(1, 0);
    soundSlotSetVolume(1, 0, 0);
    soundSlotStop(1);
}

void xglCdRingReadNoNotify(void) {
}

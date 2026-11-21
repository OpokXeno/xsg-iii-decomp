/*
 * OV02 original TU 21: 0x00a89f50..0x00a8a110 (7 functions)
 */
#include "common.h"

typedef int s32;

/* Unit record; the actor handle sits at 0x2B4. */
typedef struct SceneUnit {
    unsigned char unk_000[0x2B4];
    s32 actorHandle;
} SceneUnit;

void stateFlag1000Set(s32);
void stateFlag0040Set(s32);

void sceneEnableBlockFlags(void) {
    stateFlag0040Set(1);
    stateFlag1000Set(1);
}

void func_002ACDD0(void);

void sceneDisableBlockFlags(void) {
    stateFlag0040Set(0);
    stateFlag1000Set(0);
    func_002ACDD0();
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu021", func_00A89F98);

void func_00A89F98();

void sceneResetAndRun(s32 mode) {
    func_002ACDD0();
    func_00A89F98(mode);
}

void slotValueSetEnabled(s32, s32);

void sceneActorResetFlags(SceneUnit *sceneUnit) {
    func_002ACDD0();
    slotValueSetEnabled(sceneUnit->actorHandle, 1);
}

void sceneResetBlockFlags(void) {
    func_002ACDD0();
}

void sceneRunDefault(void) {
    func_00A89F98();
}

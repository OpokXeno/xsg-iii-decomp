/*
 * OV02 original TU 83: 0x00aace98..0x00aad1c8 (9 functions)
 */
#include "common.h"

typedef int s32;
typedef float f32;
typedef unsigned int u32;
typedef unsigned char u8;

typedef struct RoboBattleState {
    u8 unk_00[0x20];
    f32 posX;
    f32 posY;
    u8 unk_28[0x188];
    s32 battleMode;
} RoboBattleState;

INCLUDE_ASM("asm/nonmatchings/ov02/tu083", func_00AACE98);

void actorSetMode(RoboBattleState *actor, s32 battleMode) {
    actor->battleMode = battleMode;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu083", func_00AACF00);

void actorSetPosY(RoboBattleState *actor, f32 posY) {
    actor->posY = posY;
}

void actorSetPosXY(RoboBattleState *actor, f32 posX, f32 posY) {
    actor->posX = posX;
    actor->posY = posY;
}

RoboBattleState *roboBattleStateBlockGet();

s32 curActorModeIsOneOrTwo(void) {
    return (roboBattleStateBlockGet()->battleMode - 1) < 2U;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu083", func_00AACF58);

void func_00A93170();
void memset(s32 *, s32, s32);
void monsterRelease(s32);

void monsRecordDestroy(s32 *monsOwnerRecord) {
    s32 releasedSlot;

    func_00A93170();
    releasedSlot = *monsOwnerRecord;
    if (releasedSlot != 0) {
        monsterRelease(releasedSlot);
    }
    memset(monsOwnerRecord, 0, 0x1C0);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu083", func_00AAD168);

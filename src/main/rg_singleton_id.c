#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/rg_singleton_id", func_0026F370);

INCLUDE_ASM("asm/main/nonmatchings/rg_singleton_id", func_0026F700);

typedef int s32;

typedef long long s64;

typedef float f32;

s32 func_0026FA00(s32, s32, s64);

s64 func_003098D0(f32);

void movieFrameSetFade(s32 scratchQuads, s32 scratchQuadsUsed, f32 fadeAmount)
{
    func_0026FA00(scratchQuads, scratchQuadsUsed, func_003098D0(fadeAmount * 70.0f) << 0x18);
}

INCLUDE_ASM("asm/main/nonmatchings/rg_singleton_id", func_0026F9B0);

INCLUDE_ASM("asm/main/nonmatchings/rg_singleton_id", func_0026FA70);

INCLUDE_ASM("asm/main/nonmatchings/rg_singleton_id", func_0026FAD0);

INCLUDE_ASM("asm/main/nonmatchings/rg_singleton_id", func_00270028);

INCLUDE_ASM("asm/main/nonmatchings/rg_singleton_id", func_002703E8);

INCLUDE_ASM("asm/main/nonmatchings/rg_singleton_id", func_002705E0);

INCLUDE_ASM("asm/main/nonmatchings/rg_singleton_id", func_00270750);

INCLUDE_ASM("asm/main/nonmatchings/rg_singleton_id", func_00270C88);

INCLUDE_ASM("asm/main/nonmatchings/rg_singleton_id", func_00271280);

INCLUDE_ASM("asm/main/nonmatchings/rg_singleton_id", func_00271480);

typedef unsigned char u8;

extern s32 gRgSingletonIdEnabled;

void rgSingletonIDEnable(void) {
    gRgSingletonIdEnabled = 1;
}

void rgSingletonIDDisable(void) {
    gRgSingletonIdEnabled = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/rg_singleton_id", func_002714F8);

extern f32 gSingletonBobFrequency;

f32 rgSingletonWriteRect(s32 spriteRect, f32 *coords, f32 offset);

f32 sinf(f32);

typedef struct RgSingletonIDItem {
    u8 unk_00[0x20];
    f32 coords[4];
    u8 unk_30[0x8];
    f32 animationProgress;
} RgSingletonIDItem;

void rgSingletonIDUpdateBob(s32 spriteRect, RgSingletonIDItem *idSprite)
{
  f32 bobOffset;
  bobOffset = (1.0f - sinf(idSprite->animationProgress * gSingletonBobFrequency)) * ((idSprite->coords[3] - idSprite->coords[1]) * 0.5f);
  rgSingletonWriteRect(spriteRect, idSprite->coords, bobOffset);
}

void rgSingletonIDUpdateSlideIn(s32 spriteRect, RgSingletonIDItem *idSprite) {
    rgSingletonWriteRect(spriteRect, idSprite->coords, (1.0f - idSprite->animationProgress) * -35.0f);
}

INCLUDE_ASM("asm/main/nonmatchings/rg_singleton_id", func_00271638);

extern unsigned char gSingletonIdState[];

s32 singletonIdStateBuild(void *state, s32 id, void *idSource, s32 idKind);

void RgSingletonIDEntry(s32 id, void *idSource, s32 idKind) {
    singletonIdStateBuild(gSingletonIdState, id, idSource, idKind);
}

INCLUDE_ASM("asm/main/nonmatchings/rg_singleton_id", func_00271780);

extern s32 D_003E3CA0[][4];

void rgSingletonIDTableSet(s32 row, s32 column, s32 cellValue) {

    D_003E3CA0[row][column] = cellValue;
}

s32 rgSingletonIDTableGet(s32 row, s32 column) {
    return D_003E3CA0[row][column];
}

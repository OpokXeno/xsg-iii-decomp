#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu253", func_00262278);

INCLUDE_ASM("asm/main/nonmatchings/tu253", func_002623B8);

INCLUDE_ASM("asm/main/nonmatchings/tu253", func_002626D0);

INCLUDE_ASM("asm/main/nonmatchings/tu253", func_002627C0);

INCLUDE_ASM("asm/main/nonmatchings/tu253", func_002628F8);

INCLUDE_ASM("asm/main/nonmatchings/tu253", func_00262970);

INCLUDE_ASM("asm/main/nonmatchings/tu253", func_00262A10);

INCLUDE_ASM("asm/main/nonmatchings/tu253", func_00262AD0);

INCLUDE_ASM("asm/main/nonmatchings/tu253", func_00262E80);

typedef int s32;

typedef float f32;

extern s32 gSegmentPointTable;

s32 xglVectorNormal();

/* Output of segmentPlanarNormalXZ: the XZ normal of a segment, stored as x, 0, z, 1. */
typedef struct PlanarNormal {
    f32 x;                          /* 0x0 */
    s32 y;                          /* 0x4 */
    f32 z;                          /* 0x8 */
    f32 w;                          /* 0xC */
} PlanarNormal;

s32 segmentPlanarNormalXZ(PlanarNormal *segmentNormal, s32 segmentRow) {
    /* Kept as an integer offset: the typed row-pointer spellings change the emitted address code. */
    f32 (*segment)[4] = (f32 (*)[4]) (segmentRow * 0x10 + gSegmentPointTable);
    f32 direction[4];
    f32 negated_z;
    f32 x;

    __asm__ __volatile__("lqc2 vf20, 0(%0)\n\tlqc2 vf21, 0(%1)\n\tvsub.xyzw vf20, vf20, vf21\n\tsqc2 vf20, 0(%2)" :  : "r"((void *) segment[2]), "r"((void *) segment[1]), "r"(direction) : "memory");
    direction[1] = 0.0f;
    direction[3] = 1.0f;
    xglVectorNormal(&direction[0], &direction[0]);
    x = direction[0];
    negated_z = -direction[2];
    segmentNormal->w = 1.0f;
    segmentNormal->z = x;
    segmentNormal->y = 0;
    segmentNormal->x = negated_z;
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu253", func_002630F8);

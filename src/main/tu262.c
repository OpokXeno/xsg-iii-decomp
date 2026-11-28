#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu262", func_00264E60);

typedef unsigned char u8;
typedef int s32;
typedef float f32;
typedef short s16;

/* The object transform block: the fields the functions below read and write. */
typedef struct ObjectTransform {
    f32 position[4];    /* 0x00: x, y, z; objectApplyAxisFlags copies the axis values here */
    f32 pitch;          /* 0x10: written by xglVectorToAngles */
    f32 yaw;            /* 0x14: written by xglVectorToAngles */
    f32 copiedValue;   /* 0x18: copied from sourceValue */
    f32 angleScale;     /* 0x1C: set to 1.0f by xglVectorToAngles */
    u8 unk_20[0x10];    /* 0x20 */
    f32 targetPosition[4]; /* 0x30 */
    f32 pitchLimits[2]; /* 0x40 */
    f32 yawLimits[2];   /* 0x48 */
    u8 axisXEnabled;    /* 0x50 */
    u8 axisYEnabled;    /* 0x51 */
    u8 axisZEnabled;    /* 0x52 */
    u8 unk_53;          /* 0x53 */
    f32 axisX;          /* 0x54 */
    f32 axisY;          /* 0x58 */
    f32 axisZ;          /* 0x5C */
    f32 velocity[4];    /* 0x60 */
    f32 rotation[2];    /* 0x70: pitch, yaw */
    f32 sourceValue;    /* 0x78 */
    u8 unk_7C[4];       /* 0x7C */
    f32 offsetZ;        /* 0x80 */
    s16 advanceMode;    /* 0x84 */
} ObjectTransform;

/* A block whose first word says whether it is in use; the other two words are offsets from its start. */
typedef struct OffsetBlockHeader {
    s32 inUse;          /* 0x00 */
    s32 firstOffset;    /* 0x04 */
    s32 secondOffset;   /* 0x08 */
} OffsetBlockHeader;

extern unsigned char D_003E3710[];
extern unsigned char D_003E37D0[];
void *memset(void *s, int c, unsigned int n);

void cameraTablesClear(void) {
    memset(D_003E3710, 0, 0xC0);
    memset(D_003E37D0, 0, 0x10);
}

INCLUDE_ASM("asm/main/nonmatchings/tu262", func_00265208);

INCLUDE_ASM("asm/main/nonmatchings/tu262", func_002652B0);

INCLUDE_ASM("asm/main/nonmatchings/tu262", func_00265398);

INCLUDE_ASM("asm/main/nonmatchings/tu262", func_00265478);

INCLUDE_ASM("asm/main/nonmatchings/tu262", func_00265528);

INCLUDE_ASM("asm/main/nonmatchings/tu262", func_002655B0);

INCLUDE_ASM("asm/main/nonmatchings/tu262", func_00265638);

void cameraTablesClear();

void cameraTablesReset(void) {
    cameraTablesClear();
}

extern s32 cameraIndexedValueTable[];
s32 func_00265208();

void cameraIndexedApply(s32 cameraHandle, s32 cameraValue, s32 cameraValueIndex) {
    func_00265208(cameraHandle, cameraValue, cameraIndexedValueTable[cameraValueIndex]);
}

s32 func_002652B0();

void cameraIndexedSelect(s32 cameraHandle, s32 cameraValue, s32 cameraValueIndex) {
    func_002652B0(cameraHandle, cameraValue, cameraIndexedValueTable[cameraValueIndex]);
}

s32 func_00265398();

void cameraIndexedUpdate(s32 cameraHandle, s32 cameraValue, s32 cameraValueIndex) {
    func_00265398(cameraHandle, cameraValue, cameraIndexedValueTable[cameraValueIndex]);
}

s32 func_00265478();

void cameraIndexedSample(s32 cameraHandle, s32 cameraValue, s32 cameraValueIndex) {
    func_00265478(cameraHandle, cameraValue, cameraIndexedValueTable[cameraValueIndex]);
}

s32 func_00265528(s32, s32, f32);

void cameraIndexedBlend(s32 cameraHandle, s32 cameraValueIndex, f32 weight) {
    func_00265528(cameraHandle, cameraIndexedValueTable[cameraValueIndex], weight);
}

s32 func_002655B0(s32, s32, f32);

void cameraIndexedFade(s32 cameraHandle, s32 cameraValueIndex, f32 amount) {
    func_002655B0(cameraHandle, cameraIndexedValueTable[cameraValueIndex], amount);
}

extern s32 cameraActiveIndexCmd[];
s32 scratchpadSendCmd23Mode0();

void cameraActiveIndexSet(s32 activeIndex) {
    cameraActiveIndexCmd[0] = activeIndex;
    scratchpadSendCmd23Mode0(cameraActiveIndexCmd);
}

void objectUpdateAngles();

void objectApplyAxisFlags(ObjectTransform *transform) {
    f32 x;
    f32 y;
    f32 z;

    if (transform->axisXEnabled != 0) {
        x = transform->axisX;
        transform->position[0] = x;
    }
    if (transform->axisYEnabled != 0) {
        y = transform->axisY;
        transform->position[1] = y;
    }
    if (transform->axisZEnabled != 0) {
        z = transform->axisZ;
        transform->position[2] = z;
    }
    objectUpdateAngles(transform);
}

s32 cameraAnglesWrapPi(void *);
s32 xglVectorToAngles(void *angles, void *from, void *base, void *pitchLimits, void *yawLimits);
s32 func_00265878(ObjectTransform *);

void objectUpdateAngles(ObjectTransform *transform) {
    void *angles;

    angles = &transform->pitch;
    xglVectorToAngles(angles, transform->targetPosition, transform->position, transform->pitchLimits, transform->yawLimits);
    cameraAnglesWrapPi(angles);
    func_00265878(transform);
}

INCLUDE_ASM("asm/main/nonmatchings/tu262", func_00265878);

INCLUDE_ASM("asm/main/nonmatchings/tu262", func_002658B8);

INCLUDE_ASM("asm/main/nonmatchings/tu262", func_00265A78);

extern unsigned char objectAdvanceFailText[];
s32 mapProbeSpanHit(void *, s16, void *, void *, f32);
s32 xglFontDebugPrintAt(s32, s32, void *);
void objectApplyAxisFlags();

void objectAdvanceToTarget(ObjectTransform *transform) {
    f32 copiedValue;
    void *targetPosition;

    targetPosition = transform->targetPosition;
    __asm__ __volatile__("lqc2 vf20, 0(%0)\n\tlqc2 vf21, 0(%1)\n\tvadd.xyzw vf20, vf20, vf21\n\tsqc2 vf20, 0(%0)" :  : "r"(targetPosition), "r"(transform->velocity) : "memory");
    if ( mapProbeSpanHit(transform, transform->advanceMode, targetPosition, transform->rotation, transform->offsetZ) == 0) {
        xglFontDebugPrintAt(0x28, 0x32, objectAdvanceFailText);
    }
    copiedValue = transform->sourceValue;
    transform->copiedValue = copiedValue;
    objectApplyAxisFlags(transform);
}

s32 mapProbeObjectReset();

void cameraAuxReset(void) {
    mapProbeObjectReset();
}

INCLUDE_ASM("asm/main/nonmatchings/tu262", func_00265CB0);

extern unsigned char D_003E3F00[];
extern unsigned char D_005508A8[];
s32 sbNativeOwnerAdd();

void cameraNativesRegister(void) {
    sbNativeOwnerAdd(D_005508A8, D_003E3F00);
}

extern f32 cameraDefaultOffsetX;
extern f32 cameraDefaultOffsetY;
void cameraNativesRegister();
s32 func_00265E68();
void cameraOffsetSet(f32 x, f32 y);

void cameraInitModule(void) {
    cameraOffsetSet(cameraDefaultOffsetX, cameraDefaultOffsetY);
    func_00265E68();
    cameraNativesRegister();
}

INCLUDE_ASM("asm/main/nonmatchings/tu262", func_00265DC8);

INCLUDE_ASM("asm/main/nonmatchings/tu262", func_00265E68);

extern f32 cameraOffset[];

void cameraOffsetSet(f32 x, f32 y) {
    cameraOffset[0] = x;
    cameraOffset[1] = y;
}

INCLUDE_ASM("asm/main/nonmatchings/tu262", func_00265EC8);

extern s32 D_00550848;

void cameraPendingClear(void) {
    D_00550848 = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu262", func_00265FA8);

INCLUDE_ASM("asm/main/nonmatchings/tu262", func_00266008);

extern s32 cameraAllocationHandle;
s32 func_002A6FA8(s32, s32);

void cameraWorkBufferAlloc(void) {
    cameraAllocationHandle = func_002A6FA8(0xC00, -1);
}

INCLUDE_ASM("asm/main/nonmatchings/tu262", func_002660B8);

INCLUDE_ASM("asm/main/nonmatchings/tu262", func_00266208);

void *blockGetFirstEntry(OffsetBlockHeader *offsetBlock) {
    if (offsetBlock->inUse == 0) {
        return 0;
    }
    return (u8 *) offsetBlock + offsetBlock->firstOffset;
}

void *blockGetSecondEntry(OffsetBlockHeader *offsetBlock) {
    if (offsetBlock->inUse == 0) {
        return 0;
    }
    return (u8 *) offsetBlock + offsetBlock->secondOffset;
}

INCLUDE_ASM("asm/main/nonmatchings/tu262", func_00266288);

INCLUDE_ASM("asm/main/nonmatchings/tu262", func_002662B0);

INCLUDE_ASM("asm/main/nonmatchings/tu262", func_00266358);

INCLUDE_ASM("asm/main/nonmatchings/tu262", func_002664A8);

void objectAdvanceAndAim(ObjectTransform *transform) {
    void *targetPosition;

    targetPosition = transform->targetPosition;
    __asm__ __volatile__("lqc2 vf20, 0(%0)\n\tlqc2 vf21, 0(%1)\n\tvadd.xyzw vf20, vf20, vf21\n\tsqc2 vf20, 0(%0)" :  : "r"(targetPosition), "r"(transform->velocity) : "memory");
    objectUpdateAngles(transform);
}

s32 xglPointAddRotatedOffset(void *, void *, void *, f32);

void objectAdvanceRotated(ObjectTransform *transform) {
    f32 copiedValue;
    void *targetPosition;

    targetPosition = transform->targetPosition;
    __asm__ __volatile__("lqc2 vf20, 0(%0)\n\tlqc2 vf21, 0(%1)\n\tvadd.xyzw vf20, vf20, vf21\n\tsqc2 vf20, 0(%0)" :  : "r"(targetPosition), "r"(transform->velocity) : "memory");
    xglPointAddRotatedOffset(transform, targetPosition, transform->rotation, transform->offsetZ);
    copiedValue = transform->sourceValue;
    transform->copiedValue = copiedValue;
    objectApplyAxisFlags(transform);
}

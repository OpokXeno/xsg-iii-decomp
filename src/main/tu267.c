#include "common.h"

typedef int s32;
typedef float f32;
typedef signed char s8;
typedef unsigned char u8;

typedef struct { unsigned char bytes[8]; } TextTagOffset;

/* One text tag entry, 0x20 bytes, in the table at gTextTagTable (20 entries). */
struct TextTag {
    s32 key;                        /* +0x00: the caller key, 0 when the entry is free */
    s8 tagKind;                        /* +0x04 */
    unsigned char unk_05[0x3];
    s32 frames;                     /* +0x08 */
    unsigned char unk_0C[0x4];
    TextTagOffset placementOffset;           /* +0x10: copied from the caller's 8-byte placementOffset */
    unsigned char unk_18[0x8];
};

/* A rotation as two float angles: angleX at +0x00, angleY at +0x04. */
typedef struct TextTagAngles {
    f32 angleX;
    f32 angleY;
} TextTagAngles;

void textTagRotateOffset(f32 *rotatedOffset, TextTagAngles *angles, f32 distance);

void textTagPlaceOffset(void *outPosition, void *basePosition, TextTagAngles *angles, f32 distance) {
    f32 rotated[4];

    textTagRotateOffset(&rotated[0], angles, distance);
    __asm__ __volatile__("lqc2 vf20, 0(%0)" : : "r"(basePosition) : "memory");
    __asm__ __volatile__("lqc2 vf21, 0(%0)" : : "r"(rotated) : "memory");
    __asm__ __volatile__("vadd.xyzw vf20, vf20, vf21" : : : "memory");
    __asm__ __volatile__("sqc2 vf20, 0(%0)" : : "r"(outPosition) : "memory");
}

s32 xglMatrixStackMulVector();
s32 xglMatrixStackRotX(f32);
s32 xglMatrixStackRotY(f32);
s32 xglMatrixStackUnit();
/* A homogeneous point: x and y are zero, z is the input, w is 1. */
struct HomogeneousPoint {
    /* 0x00 */ s32 x;
    /* 0x04 */ s32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ f32 w;
};

void textTagRotateOffset(f32 *rotatedOffset, TextTagAngles *angles, f32 distance) {
    struct HomogeneousPoint point;

    point.z = distance;
    point.w = 1.0f;
    point.x = 0;
    point.y = 0;
    xglMatrixStackUnit();
    xglMatrixStackRotY(angles->angleY);
    xglMatrixStackRotX(angles->angleX);
    xglMatrixStackMulVector(rotatedOffset, &point.x);
}

extern struct TextTag gTextTagTable[20];
void *memset(void *destination, int value, unsigned int size);

void textTagInit(void) {
    memset(gTextTagTable, 0, 0x280U);
}

INCLUDE_ASM("asm/main/nonmatchings/tu267", func_00267D78);

void *func_00267FE0();

void textTagAdd(s32 frames, s32 tagKind, s32 key, TextTagOffset *placementOffset) {
    struct TextTag *freeTextTag;

    freeTextTag = func_00267FE0();
    if (freeTextTag != 0) {
        freeTextTag->tagKind = tagKind;
        freeTextTag->key = key;
        freeTextTag->frames = frames;
        freeTextTag->placementOffset = *placementOffset;
    }
}

/* Clears every table entry whose key equals key. */
void textTagClearKey(int key)
{
    int tagIndex;

    for (tagIndex = 0; tagIndex < 20; tagIndex++) {
        if (gTextTagTable[tagIndex].key == key) {
            memset(&gTextTagTable[tagIndex], 0, sizeof(struct TextTag));
        }
    }
}

s32 textTagCountFree(void) {
    s32 key;
    s32 usedCount;
    s32 tagIndex;
    struct TextTag *textTag;

    usedCount = 0;
    textTag = gTextTagTable;
    for (tagIndex = 0; tagIndex < 20; tagIndex++) {
        key = textTag->key;
        textTag += 1;
        if (key != 0) {
            usedCount = usedCount + 1;
        }
    }
    return 0x14 - usedCount;
}

INCLUDE_ASM("asm/main/nonmatchings/tu267", func_00267FE0);

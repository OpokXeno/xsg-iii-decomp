/* header_harvest: hand-maintained (types of the sef object split on 2026-11-24; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_MAIN_SEF_132_H
#define INCLUDE_MAIN_SEF_132_H

/* Shared by src/main/sef.c and src/main/sef_132.c (one object until the cut at 0x203B18, 2026-11-24). */

#include "shared.h"

/* One value of the scratchpad block at 0x70000000 (the effect work area). */
#define SEF_SCRATCHPAD(type, offset) (*(type *) (0x70000000 + (offset)))
typedef struct SefHermiteKey {
    unsigned char unmodeled_00[12];
    short key;                                  /* +12 */
    signed char nextKeyIndex;                            /* +14 */
    unsigned char curveFlags;                   /* +15: high nibble the ease mode, low nibble the curve kind */
} SefHermiteKey;
typedef struct SefEffectEntryRecord {
    unsigned char unmodeled_00[0x08];
    short entryId; /* +0x08 */
} SefEffectEntryRecord;
typedef struct SefEntryTableShort {
    SefEntry effectEntries[24];
    unsigned char unmodeled_20700[0x10];
    int effectEntryCount;                       /* +0x20710 */
} SefEntryTableShort;
typedef struct SefPartPlacement {
    Matrix4 matrix;                  /* +0x000 */
    unsigned char unmodeled_040[0xA0 - 0x40];
    Vector4 translation;             /* +0x0A0 */
    unsigned char unmodeled_0B0[0xC0 - 0xB0];
    Vector4 scale;                   /* +0x0C0 */
    unsigned char unmodeled_0D0[0x200 - 0xD0];
    Vector4 point;                   /* +0x200 */
} SefPartPlacement;
typedef struct SefGlobalTransform {
    Matrix4 matrix;                  /* +0x00 */
    Vector4 position;                /* +0x40 */
    float scale;                     /* +0x50 */
    short scaled;                    /* +0x54: nonzero when scale applies */
    short kind;                      /* +0x56 */
} SefGlobalTransform;
typedef signed char s8;
typedef int s32;
typedef struct SefMotionObject {
    unsigned char unmodeled_0000[0x50];
    Vector4 angles; /* +0x50 */
    unsigned char unmodeled_0060[0x70 - 0x60];
    Vector4 origin;     /* +0x70 */
    Vector4 quaternion; /* +0x80 */
    unsigned char unmodeled_0090[0xA0 - 0x90];
    Vector4 previousTarget; /* +0xA0 */
    Vector4 target;         /* +0xB0 */
    unsigned char unmodeled_00C0[0xE0 - 0xC0];
    Vector4 delta; /* +0xE0 */
    unsigned char unmodeled_00F0[0x178 - 0xF0];
    float speedA; /* +0x178 */
    unsigned char unmodeled_017C[0x1C0 - 0x17C];
    Vector4 basis; /* +0x1C0 */
    unsigned char unmodeled_01D0[0x1DC - 0x1D0];
    float time; /* +0x1DC */
    unsigned char unmodeled_01E0[0x1F8 - 0x1E0];
    int flags; /* +0x1F8 */
    unsigned char unmodeled_01FC[0x224 - 0x1FC];
    float speedB; /* +0x224 */
} SefMotionObject;
typedef struct M4 {
    AlignedVector4 r[4];
} M4;
typedef struct SefRunEntry {
    unsigned char unmodeled_00[0xC];
    short threshold;
    short payload;
} SefRunEntry;
typedef struct SefSearchOwner {
    unsigned char unmodeled_00[264];
    s32 tableOffset;
} SefSearchOwner;
typedef struct SefCameraMatrixBlock {
    M4 m0;
    M4 m40;
    AlignedVector4 v80;
    void *p90;
    int w94;
} SefCameraMatrixBlock;

#endif /* INCLUDE_MAIN_SEF_132_H */

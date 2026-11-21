/* header_harvest: hand-maintained (X3 express integration; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_MAIN_TU132_H
#define INCLUDE_MAIN_TU132_H

/* Types owned by main/tu132 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"
#include "main/tu122.h"

/* A special (X1's SdvSpecial, which also starts with its sequence): the sequence it runs, then its slot ids.
   sequence.position counts the rows reached; slotIds[i] (i < position) is the special record row i took, 0 once
   that record is freed or when no record was free. */
typedef struct SdvSpecial {
    SdvSequence sequence;     /* +0x00 */
    unsigned char unmodeled_0C[6];
} SdvSpecial;

typedef struct SefEffectEntry {
    signed char lineIndex[32];  /* +0x00: 32 line indices, negative means none (sv_get_image_item.c's view) */
    signed char sortKey[32];    /* +0x20: the second byte of each line */
    int state;
    int flags;
    short key;
    unsigned char unmodeled_04a[1];
    signed char signal;
} SefEffectEntry;

/* The request block sefGetBattleData returns (sefRequestKeyBlock). X3 layout measured from the asm of sef_134, sdv_183 and
 * tu122: the source and target actors of the request at +0x00/+0x04, the hit-wait byte at +0x20, two kind matrices
 * (kind 0x100 at +0x70, kind 0x200 at +0xB0) and, from +0x2C8, one table word per kind (kinds 1, 0, 2, 3, 4, 6; the
 * kind tables hold 64-byte matrices from +0x00, 16-byte slot vectors from +0xCC0 and flag bytes from +0x155C). */
typedef struct SefLinkedContext {
    struct XenoActor *source;        /* +0x000 */
    struct XenoActor *target;        /* +0x004 */
    unsigned char unmodeled_008[0x20 - 0x08];
    unsigned char waitHit;           /* +0x020 */
    unsigned char unmodeled_021[0x70 - 0x21];
    float kind256Matrix[4][4];       /* +0x070 */
    float kind512Matrix[4][4];       /* +0x0B0 */
    unsigned char unmodeled_0F0[0x2C4 - 0xF0];
    float kind1VectorY;              /* +0x2C4: stored into the slot vector's y when not 0 */
    int objectOffset;                /* +0x2C8: address of the kind 1 table (the linked object table) */
    int kindTableOffset[5];          /* +0x2CC: tables of kinds 0, 2, 3, 4 and 6 */
    unsigned char unmodeled_2E0[0x340 - 0x2E0];
} SefLinkedContext;

SefLinkedContext *sefGetBattleData(void);
void *func_00203990(int index);
extern Matrix4 sefCameraMatrix[1];

/* Partial local effect record: the point handles and the count of handles still held. */
typedef struct SefLocalData {
    float matrix[4][4];                              /* +0x000 */
    unsigned char unmodeled_040[0x80 - 0x40];
    Vector4 direction;                               /* +0x080 */
    unsigned char unmodeled_090[0xa0 - 0x90];
    Vector4 noise;                                   /* +0x0a0 */
    Vector4 position;                                /* +0x0b0 */
    Vector4 drift;                                   /* +0x0c0 */
    unsigned char unmodeled_0d0[0xd8 - 0xd0];
    float scaleTime;                                 /* +0x0d8 */
    unsigned char unmodeled_0dc[0xe0 - 0xdc];
    Vector4 positionValue;                           /* +0x0e0 */
    unsigned char unmodeled_0f0[0xf8 - 0xf0];
    short directionMode;                             /* +0x0f8 */
    unsigned char unmodeled_0fa[0x100 - 0xfa];
    Vector4 velocityValue;                           /* +0x100 */
    unsigned char unmodeled_110[0x118 - 0x110];
    int constantKeys;                                /* +0x118 */
    unsigned char unmodeled_11c[0x140 - 0x11c];
    Vector4 rotationValue;                           /* +0x140 */
    unsigned char unmodeled_150[0x160 - 0x150];
    Vector4 scaleValue;                              /* +0x160 */
    unsigned char unmodeled_170[0x67E - 0x170];
    short allocScanFlag;                             /* +0x67E: zero takes the one-slot check (sefAllocLocalData) */
    unsigned char unmodeled_680[0x694 - 0x680];
    int inUse;                                       /* +0x694 */
    unsigned char unmodeled_698[0x69C - 0x698];
    short pointHandles[0x100];                       /* +0x69C */
    short cursor;                                    /* +0x89C: the next slot the allocator tries */
    unsigned short pointCount;                       /* +0x89E: incremented on alloc, decremented when a handle is freed */
    unsigned char unmodeled_8A0[0x8B0 - 0x8A0];      /* the record stride is 0x8B0 (sefAllocEffectData) */
} SefLocalData;

#endif

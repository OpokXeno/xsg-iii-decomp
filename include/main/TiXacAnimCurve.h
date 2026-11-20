#ifndef INCLUDE_MAIN_TIXACANIMCURVE_H
#define INCLUDE_MAIN_TIXACANIMCURVE_H

/* Types owned by main/tu345 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"

/* One skip (face) node of a hierarchy (0x70 bytes): the limit flags at +0x4, the mode flags at +0x8 (bit 0x2 is the object
 * mode bit), the defaults at +0xC, +0x24 and +0x30, and the scale at +0x3C. */
typedef struct TiHrcSkip {
    int parent;
    unsigned int limitFlags;
    int flags;
    float translationDefault[3];
    float orient[3];
    float rotationDefault[3];
    float scaleDefault[3];
    float scale[3];
    unsigned char unmodeled_48[0x4];
    float limit[6];
    unsigned char unmodeled_64[0xC];
} TiHrcSkip;

/* One curve key header: the curve type is the low 12 bits of typeFlags, origin is the start time, and the payload at +4 holds
 * three floats (value[0] at +4, value[1] at +8, value[2] at +C) or, for some types, a u16 pair (halfwords[0] at +4, halfwords[1] at +6). */
typedef struct TiXacKey {
    u16 typeFlags;
    s16 origin;
    union {
        f32 value[3];
        u16 halfwords[6];
    } payload;
} TiXacKey;

/* A curve key table: the key count, then (time, value) float pairs. */
typedef struct XacCurveKeyTable {
    int count;
    f32 keys[1];
} XacCurveKeyTable;

#endif

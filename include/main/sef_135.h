#ifndef INCLUDE_MAIN_SEF_135_H
#define INCLUDE_MAIN_SEF_135_H

/* Types owned by main/sef_135 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"

/* The object a live effect record belongs to; only its position (+0x20, an xyz vector) is read here. */
typedef struct SefGroupObject {
    unsigned char unmodeled_00[0x20];
    Vector4 position;              /* +0x20 */
} SefGroupObject;

/*
 * One effect record of the table sefGetScheduler returns, 0x150 bytes apart (the stride the owner-group loops step by).
 * Only the fields those loops read are named.
 */
typedef struct SefEffectRecord {
    int unmodeled_00;              /* +0x00 */
    int flags;                     /* +0x04: 0x8000000 keeps the record out of the owner-group counts */
    unsigned char unmodeled_08[0x10 - 0x08];
    Vector4 unmodeled_10;          /* +0x10 */
    Vector4 unmodeled_20;          /* +0x20 */
    Vector4 unmodeled_30;          /* +0x30 */
    unsigned char unmodeled_40[0x50 - 0x40];
    int inUse;                     /* +0x50: zero for a free record */
    unsigned char unmodeled_54[0x5C - 0x54];
    struct SefGroupObject *object; /* +0x5C: the object the record belongs to */
    unsigned char unmodeled_60[0x70 - 0x60];
    int ownerKey;                  /* +0x70: owner id the group functions compare */
    unsigned char unmodeled_74[0x13C - 0x74];
    unsigned short value;          /* +0x13C: key id the group functions compare */
    unsigned char unmodeled_13e[0x150 - 0x13E];
} SefEffectRecord;

#endif

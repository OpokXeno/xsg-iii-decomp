#ifndef INCLUDE_MAIN_TIGLOBALCOLL_H
#define INCLUDE_MAIN_TIGLOBALCOLL_H

/* Types owned by main/tu335 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"

/* Collision set named by the handle: the offsets from the set to the directional and point wind headers (+0x10,
 * +0x20), three entry counts (+0x30, +0x34, +0x38), then the offsets from the set to their entry arrays (+0x3c,
 * +0x40, +0x44). */
typedef struct TiGlobalCollSet {
    unsigned char unmodeled_00[0x10];
    int windDirOffsets[4];      /* +0x10 */
    int windPointOffsets[4];    /* +0x20 */
    unsigned int sphereCount;   /* +0x30 */
    unsigned int planeCount;    /* +0x34 */
    unsigned int cylinderCount; /* +0x38 */
    int sphereEntriesOffset;    /* +0x3c */
    int planeEntriesOffset;     /* +0x40 */
    int cylinderEntriesOffset;  /* +0x44 */
} TiGlobalCollSet;

/* One cylinder of a global collision set (0x90 bytes): its radius and half height, the matrix that maps the
 * world point to the cylinder space and the matrix that maps it back. */
typedef struct TiGlobalCollCylinder {
    float radius;               /* +0x00 */
    float halfHeight;           /* +0x04 */
    unsigned char unmodeled_08[8];
    Matrix4 toWorld;            /* +0x10 */
    Matrix4 toLocal;            /* +0x50 */
} TiGlobalCollCylinder;

#endif

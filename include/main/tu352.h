#ifndef INCLUDE_MAIN_TU352_H
#define INCLUDE_MAIN_TU352_H

/* Types owned by main/tu352 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"

/* One collision triangle: three vertices, a plane vector and a kind. */
typedef struct MapRecord {
    unsigned short kind;
    short neighbour[3];                 /* +0x02, -1 marks an open edge */
    unsigned short vertexIndex[3];
    unsigned short planeIndex;
} MapRecord;

/* Partial layouts: only the offsets the map files touch are named. */
typedef struct MapRecordKind {
    int kindCode;
    unsigned char unmodeled_04[4];
} MapRecordKind;

/*
 * One entry of a block group, which is also a collision group: record count at +0x00, keep id at +0x02, vertex array at +0x04
 * (vertices[0] gives the group's Y range), planes, kinds, records from +0x10. UnduInit relocates the three pointers through
 * MapFixupEntryWords. The tag keeps its fixup-entry name because enemy_1.c and xenoActor.h name it; MapRecordGroup is the same type.
 */
typedef struct MapFixupEntry {
    unsigned short recordCount;         /* +0x00 */
    unsigned short keepId;              /* +0x02 */
    Vector4 *vertices;                  /* +0x04 */
    Vector4 *planes;                    /* +0x08 */
    MapRecordKind *kindTable;           /* +0x0C */
    MapRecord records[1];               /* +0x10, recordCount entries */
} MapFixupEntry;

typedef MapFixupEntry MapRecordGroup;

/*
 * Triangle record set; also the probe state that the mapProbe* helpers fill. Size 0x1A0: the set mapRegisterBlock builds
 * on its stack. The X3 layout is measured from the asm (mapRecordPlaneDistance, func_002A8898, func_002AA490): the vertex and plane
 * arrays at +0x60/+0x64 and the two point pointers at +0x140/+0x144; the remaining fields keep unmodeled names until read.
 */
typedef struct MapRecordSet {
    unsigned char unmodeled_00[0x60];
    Vector4 *vertices;                  /* +0x60 */
    Vector4 *planes;                    /* +0x64 */
    MapRecord *records;                 /* +0x68 */
    MapRecordKind *kindTable;           /* +0x6C */
    f32 heightTolerance;                /* +0x70 */
    f32 collisionHeight;                /* +0x74: MAP_HEIGHT_NONE until a record is hit */
    MapRecord *collisionRecord;         /* +0x78 */
    unsigned char unmodeled_7C[0x8];
    int hitKind;                        /* +0x84 */
    unsigned char unmodeled_88[0x8];
    Vector4 *edgeStart;                 /* +0x90 */
    Vector4 *edgeEnd;                   /* +0x94 */
    unsigned char unmodeled_98[0x18];
    f32 planeA[4];                      /* +0xB0 */
    f32 planeB[4];                      /* +0xC0 */
    Vector4 *triangleVertex[3];         /* +0xD0 */
    unsigned char unmodeled_DC[0x64];
    Vector4 *probeStart;                /* +0x140 */
    Vector4 *origin;                    /* +0x144 */
    unsigned char unmodeled_148[0x58];
} MapRecordSet;

int func_002A82A8(MapRecordSet *set, MapRecord *record);
int func_002A8440(MapRecordSet *set, MapRecord *record);

/*
 * Collision object, which is also the request block of the map probes: the current and previous record, flag bits (the attribute
 * mask of a probe), the block walk counts, the hit distance, its group and record set, and the hit's kind. func_002A70E8 stores
 * the defaults at +0x00..+0x2C; mapProbeScratchMove points recordSet at the scratchpad probe state. Size 0x40.
 */
typedef struct MapCollisionObject {
    MapRecord *record;                  /* +0x00: the hit record, 0 for none */
    MapRecord *previousRecord;          /* +0x04 */
    unsigned short flags;               /* +0x08: the attribute mask selecting what a probe hits */
    unsigned short bits;                /* +0x0A */
    unsigned int countWord;             /* +0x0C */
    unsigned char count;                /* +0x10 */
    unsigned char loopCount;            /* +0x11 */
    unsigned char missCount;            /* +0x12 */
    unsigned char probeEnabled;         /* +0x13 */
    float heightTolerance;              /* +0x14 */
    float distance;                     /* +0x18: MAP_HEIGHT_NONE until a hit */
    MapRecordGroup *block;              /* +0x1C */
    MapRecordSet *recordSet;            /* +0x20 */
    int kind;                           /* +0x24: the hit's kind, the attribute word of a probe */
    float unitWeight;                   /* +0x28 */
    float radius;                       /* +0x2C */
    unsigned char unmodeled_30[0x10];
} MapCollisionObject;

/* Map block header: flags at +0x00 (0x80 = fixed up, 3 = valid), group count at +0x01, group offsets from +0x04. */
typedef struct MapFixupBlock {
    unsigned char flags;                /* +0x00 */
    unsigned char groupCount;           /* +0x01 */
    unsigned short unmodeled_02;        /* +0x02 */
    unsigned int firstGroup;            /* +0x04 */
} MapFixupBlock;

#endif

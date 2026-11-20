#include "common.h"
#include "shared.h"
/* Block flags: 0x80 fixed up, 3 valid before fixup, 0x83 loaded. Group type 0x0B00 holds the keep-off entries. */
#define MAP_BLOCK_FIXED_UP 0x80
#define MAP_BLOCK_VALID 3
#define MAP_BLOCK_LOADED 0x83
#define MAP_GROUP_KEEP_OFF 0x0B00

/* Load-time view of a MapFixupEntry: its three pointer words as plain integers, which UnduInit relocates by the block address. */
typedef struct MapFixupEntryWords {
    unsigned char unmodeled_00[2];
    unsigned short keepId;              /* +0x02 */
    unsigned int pointer04;             /* +0x04 */
    unsigned int pointer08;             /* +0x08 */
    unsigned int pointer0C;             /* +0x0C */
} MapFixupEntryWords;

/* Group of entries: type word at +0x00, entry count at +0x02, relocation flag at +0x03. */
typedef struct MapFixupGroup {
    unsigned short typeWord;            /* +0x00 */
    unsigned char entryCount;           /* +0x02 */
    unsigned char relocated;            /* +0x03 */
    unsigned int firstEntry;            /* +0x04, entry array of block-relative offsets */
} MapFixupGroup;

#include "main/tu352.h"

MapFixupEntry *UnduDataGetHeader(int groupType, int keepId);
MapRecordSet *mapSetupCollisionObject(Vector4 *position, const int *delta, MapCollisionObject *object);
void mapBindAndSnapObject(Vector4 *position, MapCollisionObject *object);
extern void *mapCurrentBlock;
extern int elvKeepOffFlags[];
int printf(const char *format, ...);
void UnduInit(MapFixupBlock *block);
void mapSnapToNearestTriangle(Vector4 *position, MapCollisionObject *object);
f32 func_002AA490(Vector4 *position, const Vector4 *delta, MapCollisionObject *object);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A68E0);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A6928);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A6950);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A69D0);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A69F0);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A6A08);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A6A20);

void mapTriangleEdgePlanes(void) {
    __asm__ __volatile__(
        "vmula.xz ACC, vf19, vf20\n"
        "vmadda.xz ACC, vf16, vf21\n"
        "vmadd.xz vf4, vf17, vf23\n"
        "vmula.xz ACC, vf19, vf21\n"
        "vmadda.xz ACC, vf17, vf22\n"
        "vmadd.xz vf5, vf18, vf23\n"
        "vmula.xz ACC, vf19, vf22\n"
        "vmadda.xz ACC, vf18, vf20\n"
        "vmadd.xz vf6, vf16, vf23\n"
        "vsubz.x vf1, vf4, vf4z\n"
        "vsubz.x vf2, vf5, vf5z\n"
        "vsubz.x vf3, vf6, vf6z\n"
        "vaddx.y vf1, vf0, vf2x\n"
        "vaddx.z vf1, vf0, vf3x\n"
        : : : "memory");
}

void mapStoreEdgePlanes(Vector4 *edgePlanes)
{
    __asm__ __volatile__(
        "jal mapTriangleEdgePlanes\n\t"
        "sqc2 vf1, 0(%0)"
        : : "r"(edgePlanes) : "$31", "memory");
}

int mapPointOutsideTriangle(const Vector4 *point)
{
    int clipFlags;
    int outside;

    __asm__ __volatile__(
        "lqc2 $vf19, 0(%0)\n\t"
        "vmr32.xyzw $vf1, $vf19\n\t"
        "vmr32.xyzw $vf23, $vf1\n\t"
        : : "r"(point) : "memory");
    mapTriangleEdgePlanes();
    __asm__ __volatile__(
        "vsub.xyzw $vf2, $vf0, $vf0\n\t"
        "vclipw.xyz $vf1, $vf2w\n\t"
        "vnop\n\t"
        "vnop\n\t"
        "vnop\n\t"
        "vnop\n\t"
        "vnop\n\t"
        "cfc2 %1, $vi18\n\t"
        "nop\n\t"
        "andi %0, %1, 0x15\n\t"
        : "=r"(outside), "=r"(clipFlags) : : "memory");
    return outside;
}

/* Distance from the point origin to the plane through point with normal normal. */
f32 mapPlaneDistance(const Vector4 *origin, const Vector4 *point, const Vector4 *normal) {
    f32 result;

    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(origin) : "memory");
    __asm__ __volatile__("lqc2 vf3, 0(%0)" : : "r"(point) : "memory");
    __asm__ __volatile__("lqc2 vf4, 0(%0)" : : "r"(normal) : "memory");
    __asm__ __volatile__("vsub.xz vf1, vf3, vf2" : : : "memory");
    __asm__ __volatile__("vmul.xz vf5, vf1, vf4" : : : "memory");
    __asm__ __volatile__("vaddz.x vf1, vf5, vf5z" : : : "memory");
    __asm__ __volatile__("vaddy.x vf5, vf1, vf3y" : : : "memory");
    __asm__ __volatile__("qmfc2.ni %0, vf5" : "=r"(result) : : "memory");
    return result;
}

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A6B18);

void func_002A68E0(void);
void func_002A6B18(void);
void func_002A4DA0(void);
void func_002A4E60(void);
void func_002A5238(void);
void func_002A4E00(void);
void func_002A62A0(void);

void func_002A6F60(void) {
    func_002A6B18();
    func_002A4DA0();
    func_002A4E60();
    func_002A5238();
    func_002A4E00();
    func_002A62A0();
    func_002A68E0();
}

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A6FA8);

void func_002A70E8(MapCollisionObject *object);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A70E8);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A7158);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A72A0);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A7520);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A7798);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A78E0);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A7AF8);

#define MAP_HEIGHT_NONE -1000.0f

/* Plane distance of the probe origin to a record, or MAP_HEIGHT_NONE when the origin is outside the triangle. */
f32 mapRecordPlaneDistance(const MapRecordSet *set, const MapRecord *record)
{
    const Vector4 *origin = set->origin;
    const Vector4 *vertices = set->vertices;
    const Vector4 *vertex0 = vertices + record->vertexIndex[0];
    const Vector4 *vertex1 = vertices + record->vertexIndex[1];
    const Vector4 *vertex2 = vertices + record->vertexIndex[2];
    __asm__ __volatile__("lqc2 vf16, 0(%0)" : : "r"(vertex0) : "memory");
    __asm__ __volatile__("lqc2 vf17, 0(%0)" : : "r"(vertex1) : "memory");
    __asm__ __volatile__("lqc2 vf18, 0(%0)" : : "r"(vertex2) : "memory");
    __asm__ __volatile__("vmr32.xyzw vf1, vf16" : : : "memory");
    __asm__ __volatile__("vmr32.xyzw vf2, vf17" : : : "memory");
    __asm__ __volatile__("vmr32.xyzw vf3, vf18" : : : "memory");
    __asm__ __volatile__("vmr32.xyzw vf20, vf1" : : : "memory");
    __asm__ __volatile__("vmr32.xyzw vf21, vf2" : : : "memory");
    __asm__ __volatile__("vmr32.xyzw vf22, vf3" : : : "memory");
    if (mapPointOutsideTriangle(origin) == 0) {
        return mapPlaneDistance(origin, vertex0, set->planes + record->planeIndex);
    }
    return MAP_HEIGHT_NONE;
}

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A7DC0);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A7E98);

/* Height status of a record set probe: no hit, a hit nearer the origin height than the best so far, or the first hit. */
#define MAP_HIT_NONE 0
#define MAP_HIT_NEARER 1
#define MAP_HIT_FIRST 2

int mapCheckSubHeightCheck(MapRecordSet *set, MapRecord *record, float height)
{
    int hitStatus;
    float hitGap;
    float collisionGap;

    hitStatus = MAP_HIT_NONE;
    if (set->collisionHeight == MAP_HEIGHT_NONE) {
        hitStatus = MAP_HIT_FIRST;
    } else {
        hitGap = set->origin->y - height;
        if (hitGap < 0.0f) {
            hitGap = -hitGap;
        }
        collisionGap = set->origin->y - set->collisionHeight;
        if (collisionGap < 0.0f) {
            collisionGap = -collisionGap;
        }
        if (hitGap < collisionGap) {
            hitStatus = MAP_HIT_NEARER;
        }
    }
    switch (hitStatus) {
    case MAP_HIT_NEARER:
        if (set->origin->y + set->heightTolerance < height) {
            break;
        }
    case MAP_HIT_FIRST:
        set->collisionHeight = height;
        set->collisionRecord = record;
        break;
    default:
        break;
    }
    return hitStatus;
}

extern int D_005512FC;
/* Plane distance of the probe origin to a record, or MAP_HEIGHT_NONE when the origin is outside the triangle. */ f32 mapRecordPlaneDistance(const MapRecordSet *set, const MapRecord *record);
int func_002AA8B0(MapRecordSet *set, int flag);
int mapCheckSubHeightCheck(MapRecordSet *set, MapRecord *record, float height);

/* Walks the record set and passes each hit record to mapCheckSubHeightCheck unless its kind is 4. */
void mapCheckAll(MapRecordSet *owner)
{
    MapRecord *record;
    int recordIndex;
    int kind;
    f32 value;
    MapRecordSet *set = owner;

    record = set->records;
    if (D_005512FC != 0 && func_002AA8B0(set, 0) != 0) {
        return;
    }
    for (recordIndex = 0; recordIndex < (*(int *)&set->unmodeled_00[0x5C]); recordIndex++) {
        value = mapRecordPlaneDistance(set, record);
        if (value != MAP_HEIGHT_NONE) {
            kind = record->kind;
            if (set->kindTable != 0) {
                kind = set->kindTable[kind].kindCode;
            }
            if (kind != 4) {
                mapCheckSubHeightCheck(set, record, value);
            }
        }
        record++;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A81D0);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A82A8);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A8440);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A85D8);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A8898);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A8FE0);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A9920);

INCLUDE_ASM("asm/main/nonmatchings/undulate", mapSetupCollisionObject);

INCLUDE_ASM("asm/main/nonmatchings/undulate", mapSnapToNearestTriangle);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A9F70);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A9FD0);

void mapBindAndSnapObject(Vector4 *position, MapCollisionObject *object) {
    if (mapSetupCollisionObject(position, 0, object) != 0) {
        mapSnapToNearestTriangle(position, object);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002AA490);

/* Resets the scratchpad probe at 0x70000000, points its record set at 0x70000030 and runs the move check on it. */
f32 mapProbeScratchMove(Vector4 *position) {
    MapCollisionObject *probe = (MapCollisionObject *) 0x70000000;

    func_002A70E8(probe);
    probe->recordSet = (MapRecordSet *) 0x70000030;
    return func_002AA490(position, 0, probe);
}

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002AA8A0);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002AA8A8);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002AA8B0);

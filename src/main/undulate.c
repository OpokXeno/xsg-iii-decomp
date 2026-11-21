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
    unsigned int vertexArrayAddress;             /* +0x04 */
    unsigned int planeArrayAddress;             /* +0x08 */
    unsigned int kindTableAddress;             /* +0x0C */
} MapFixupEntryWords;

/* Group of entries: type word at +0x00, entry count at +0x02, relocation flag at +0x03. */
typedef struct MapFixupGroup {
    unsigned short groupType;            /* +0x00 */
    unsigned char blockRelativeOffsetCount;           /* +0x02 */
    unsigned char relocated;            /* +0x03 */
    unsigned int blockRelativeOffsets;            /* +0x04, entry array of block-relative offsets */
} MapFixupGroup;

#include "main/tu352.h"

MapFixupEntry *UnduDataGetHeader(int groupType, int keepId);
MapRecordSet *mapSetupCollisionObject(Vector4 *position, const int *delta, MapCollisionObject *collisionObject);
void mapBindAndSnapObject(Vector4 *position, MapCollisionObject *collisionObject);
extern void *mapCurrentBlock;
extern int elvKeepOffFlags[];
int printf(const char *format, ...);
void UnduInit(MapFixupBlock *block);
void mapSnapToNearestTriangle(Vector4 *position, MapCollisionObject *collisionObject);
f32 func_002AA490(Vector4 *position, const Vector4 *delta, MapCollisionObject *collisionObject);

typedef int s32;
extern MapCollisionObject mapStaticCollisionObject;
extern unsigned char mapStaticProbeStorage[];
/* The probe word stored at mapStaticProbeStorage by mapStaticStoreProbe. */
#define mapStaticProbeWord (*(MapRecordGroup **) mapStaticProbeStorage)
extern unsigned char gUndulateActiveFlags[];

/* View of MapRecordSet with its record count at +0x5C, which tu352.h leaves unmodeled. */
typedef struct MapRecordSetCounted {
    unsigned char unmodeled_00[0x5C];
    int triangleCount;                    /* +0x5C */
} MapRecordSetCounted;

void mapInitStaticCollisionObject(void);
MapRecordGroup *func_002A6FA8();
void func_002A70E8(MapCollisionObject *collisionObject);

void mapInitStaticCollisionObject(void) {
    func_002A70E8(&mapStaticCollisionObject);
    mapStaticCollisionObject.bits = 0x4800;
    mapStaticCollisionObject.block = func_002A6FA8(0x700, -1);
}

void mapStaticStoreProbe(s32 probeIndex) {
    MapRecordGroup *probeGroup;

    probeGroup = func_002A6FA8(0x700, probeIndex);
    mapStaticProbeWord = probeGroup;
}

s32 mapStaticObjectProbe(Vector4 *position, f32 *outDistance)
{
  f32 distance;
  u8 hitKind;
  if (mapStaticCollisionObject.block != 0)
  {
    distance = func_002AA490(position, 0, &mapStaticCollisionObject);
    if (distance != (-1000.0f))
    {
      hitKind = mapStaticCollisionObject.kind;
      *outDistance = distance;
      return hitKind + 1;
    }
  }
  return -1;
}

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A69D0);

u8 undulateFlagQuery(s32 flagNumber) {
    return gUndulateActiveFlags[flagNumber];
}

s32 undulateFlagActive(s32 flagNumber) {
    return gUndulateActiveFlags[flagNumber] & 1;
}

extern s32 D_00551598;

s32 undulateMaskTestBit(s32 bitIndex) {
    return D_00551598 & (1 << bitIndex);
}

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
    f32 planeDistance;

    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(origin) : "memory");
    __asm__ __volatile__("lqc2 vf3, 0(%0)" : : "r"(point) : "memory");
    __asm__ __volatile__("lqc2 vf4, 0(%0)" : : "r"(normal) : "memory");
    __asm__ __volatile__("vsub.xz vf1, vf3, vf2" : : : "memory");
    __asm__ __volatile__("vmul.xz vf5, vf1, vf4" : : : "memory");
    __asm__ __volatile__("vaddz.x vf1, vf5, vf5z" : : : "memory");
    __asm__ __volatile__("vaddy.x vf5, vf1, vf3y" : : : "memory");
    __asm__ __volatile__("qmfc2.ni %0, vf5" : "=r"(planeDistance) : : "memory");
    return planeDistance;
}

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A6B18);

void mapInitStaticCollisionObject(void);
void func_002A6B18(void);
void undulateSetAllFlags(void);
void undulateResetVectors(void);
void mapFlagTableReset(void);
void undulateSetAllBits(void);
void mapProbeInitDefault(void);

void mapCollisionInit(void) {
    func_002A6B18();
    undulateSetAllFlags();
    undulateResetVectors();
    mapFlagTableReset();
    undulateSetAllBits();
    mapProbeInitDefault();
    mapInitStaticCollisionObject();
}

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A6FA8);

void func_002A70E8(MapCollisionObject *collisionObject);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A70E8);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A7158);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A72A0);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A7520);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A7798);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A78E0);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A7AF8);

#define MAP_HEIGHT_NONE -1000.0f

/* Plane distance of the probe origin to a record, or MAP_HEIGHT_NONE when the origin is outside the triangle. */
f32 mapRecordPlaneDistance(const MapRecordSet *triangleSet, const MapRecord *candidateTriangle)
{
    const Vector4 *origin = triangleSet->origin;
    const Vector4 *vertices = triangleSet->vertices;
    const Vector4 *firstCorner = vertices + candidateTriangle->vertexIndex[0];
    const Vector4 *secondCorner = vertices + candidateTriangle->vertexIndex[1];
    const Vector4 *thirdCorner = vertices + candidateTriangle->vertexIndex[2];
    __asm__ __volatile__("lqc2 vf16, 0(%0)" : : "r"(firstCorner) : "memory");
    __asm__ __volatile__("lqc2 vf17, 0(%0)" : : "r"(secondCorner) : "memory");
    __asm__ __volatile__("lqc2 vf18, 0(%0)" : : "r"(thirdCorner) : "memory");
    __asm__ __volatile__("vmr32.xyzw vf1, vf16" : : : "memory");
    __asm__ __volatile__("vmr32.xyzw vf2, vf17" : : : "memory");
    __asm__ __volatile__("vmr32.xyzw vf3, vf18" : : : "memory");
    __asm__ __volatile__("vmr32.xyzw vf20, vf1" : : : "memory");
    __asm__ __volatile__("vmr32.xyzw vf21, vf2" : : : "memory");
    __asm__ __volatile__("vmr32.xyzw vf22, vf3" : : : "memory");
    if (mapPointOutsideTriangle(origin) == 0) {
        return mapPlaneDistance(origin, firstCorner, triangleSet->planes + candidateTriangle->planeIndex);
    }
    return MAP_HEIGHT_NONE;
}

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A7DC0);

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002A7E98);

/* Height status of a record set probe: no hit, a hit nearer the origin height than the best so far, or the first hit. */
#define MAP_HIT_NONE 0
#define MAP_HIT_NEARER 1
#define MAP_HIT_FIRST 2

int mapCheckSubHeightCheck(MapRecordSet *triangleSet, MapRecord *candidateTriangle, float height)
{
    int hitStatus;
    float hitGap;
    float collisionGap;

    hitStatus = MAP_HIT_NONE;
    if (triangleSet->collisionHeight == MAP_HEIGHT_NONE) {
        hitStatus = MAP_HIT_FIRST;
    } else {
        hitGap = triangleSet->origin->y - height;
        if (hitGap < 0.0f) {
            hitGap = -hitGap;
        }
        collisionGap = triangleSet->origin->y - triangleSet->collisionHeight;
        if (collisionGap < 0.0f) {
            collisionGap = -collisionGap;
        }
        if (hitGap < collisionGap) {
            hitStatus = MAP_HIT_NEARER;
        }
    }
    switch (hitStatus) {
    case MAP_HIT_NEARER:
        if (triangleSet->origin->y + triangleSet->heightTolerance < height) {
            break;
        }
    case MAP_HIT_FIRST:
        triangleSet->collisionHeight = height;
        triangleSet->collisionRecord = candidateTriangle;
        break;
    default:
        break;
    }
    return hitStatus;
}

extern int mapCheckFilterEnabled;
f32 mapRecordPlaneDistance(const MapRecordSet *triangleSet, const MapRecord *candidateTriangle);
int func_002AA8B0(MapRecordSet *triangleSet, int flag);
int mapCheckSubHeightCheck(MapRecordSet *triangleSet, MapRecord *candidateTriangle, float height);

/* Walks the record set and passes each hit record to mapCheckSubHeightCheck unless its kind is 4. */
void mapCheckAll(MapRecordSet *owner)
{
    MapRecord *candidateTriangle;
    int triangleIndex;
    int triangleKind;
    f32 planeDistance;
    MapRecordSet *triangleSet = owner;

    candidateTriangle = triangleSet->records;
    if (mapCheckFilterEnabled != 0 && func_002AA8B0(triangleSet, 0) != 0) {
        return;
    }
    for (triangleIndex = 0; triangleIndex < ((MapRecordSetCounted *) triangleSet)->triangleCount; triangleIndex++) {
        planeDistance = mapRecordPlaneDistance(triangleSet, candidateTriangle);
        if (planeDistance != MAP_HEIGHT_NONE) {
            triangleKind = candidateTriangle->kind;
            if (triangleSet->kindTable != 0) {
                triangleKind = triangleSet->kindTable[triangleKind].kindCode;
            }
            if (triangleKind != 4) {
                mapCheckSubHeightCheck(triangleSet, candidateTriangle, planeDistance);
            }
        }
        candidateTriangle++;
    }
}

f32 func_002A7E98(MapRecordSet *, MapRecord *);

void mapCheckAllByProbeDistance(MapRecordSet *triangleSet)
{
  MapRecord *candidateTriangle;
  MapRecordKind *recordKinds;
  MapRecordSet *owner;
  f32 distance;
  s32 triangleIndex;
  s32 triangleKind;
  owner = triangleSet;
  candidateTriangle = owner->records;
  if ((mapCheckFilterEnabled == 0) || (func_002AA8B0(triangleSet, 1) == 0))
  {
    triangleIndex = 0;
    if (((MapRecordSetCounted *) owner)->triangleCount > 0)
    {
      do
      {
        distance = func_002A7E98(owner, candidateTriangle);
        if (distance != MAP_HEIGHT_NONE)
        {
          recordKinds = owner->kindTable;
          triangleKind = candidateTriangle->kind;
          if (recordKinds != 0)
          {
            triangleKind = recordKinds[triangleKind].kindCode;
          }
          if (triangleKind != 4)
          {
            mapCheckSubHeightCheck(owner, candidateTriangle, distance);
          }
        }
        triangleIndex += 1;
        candidateTriangle += 1;
      }
      while (triangleIndex < ((MapRecordSetCounted *) owner)->triangleCount);
    }
  }
}

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

void mapBindAndSnapObject(Vector4 *position, MapCollisionObject *collisionObject) {
    if (mapSetupCollisionObject(position, 0, collisionObject) != 0) {
        mapSnapToNearestTriangle(position, collisionObject);
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

void mapHookNoop(void) {

}

s32 mapHookReturnZero(void) {
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/undulate", func_002AA8B0);

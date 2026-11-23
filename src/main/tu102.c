#include "common.h"

typedef short s16;

typedef int s32;

/* 6-byte row of gNativeMethodCodeTable; the table ends with a negative nativeCode. */
typedef struct NativeCodeEntry {
    s16 nativeCode;
    s16 nativeSlot;
    s16 unk_04;
} NativeCodeEntry;

extern const NativeCodeEntry gNativeMethodCodeTable[];

/* Finds the row of gNativeMethodCodeTable whose nativeCode matches (0 when absent). */
const NativeCodeEntry *nativeMethodEntryFind(s32 nativeCode) {
    const NativeCodeEntry *row = gNativeMethodCodeTable;

    while (row->nativeCode >= 0) {
        if (row->nativeCode == nativeCode) {
            return row;
        }
        row++;
    }
    return 0;
}

/* 8-byte row of gActorScanTable (the layout tu074 reads); the table ends with a negative key. */
typedef struct ActorScanEntry {
    s16 key;
    s16 scanFlagsAndIndex;
    s16 scanStep;
    s16 unk_06;
} ActorScanEntry;

extern const ActorScanEntry gActorScanTable[];

/* Finds the row of gActorScanTable whose key matches (0 when absent). */
const ActorScanEntry *actorScanEntryFind(s32 key) {
    const ActorScanEntry *row = gActorScanTable;

    while (row->key >= 0) {
        if (row->key == key) {
            return row;
        }
        row++;
    }
    return 0;
}

#include "shared.h"

typedef union ExtentVector {
    struct {
        float x;
        float y;
        float z;
        float w;
    };
    int words[4];
} ExtentVector;

/* kept: the enum type decides the emitted order of the angle stores */
typedef enum ShapeAngleCode {
    SHAPE_ANGLE_CODE_NONE,
    SHAPE_ANGLE_CODE_DEFAULT = 3
} ShapeAngleCode;

typedef struct ActorExtentShape {
    int drawEnabled;
    int unk_04[3];
    ExtentVector extents;
    ShapeAngleCode angle50;
    int angle54;
    ShapeAngleCode angle58;
    int keepsSlotFlags;
} ActorExtentShape;

typedef struct ShapeRecord {
    char *name;
    int shapeKind;
    float heightY;
    int recordAngleFirst;
    int recordAngleSecondIndex;
    int recordAngleThird;
} ShapeRecord;

typedef struct ShapeRectExtent {
    float x;
    float z;
} ShapeRectExtent;

extern float gShapeSquareExtentTable[];
extern float gShapeCircleRadiusTable[];
extern ShapeRectExtent gShapeRectExtentTable[];
extern int gShapeAngleSecondTable[];
extern ShapeRecord gShapeNameRecordTable[];
extern char D_00550A60[];
extern char D_00550A68[];
extern char D_00550A70[];
float MMathCalcLengthXZ(ExtentVector *vector);
int func_0030EBD0(const char *left, const char *right, unsigned int length);

/* Fills the extents shape of an actor from its name record: the footprint is looked up by
 * the nameRecord's shapeKind in one of three tables (square, circle, rectangle) and scaled by 5.0
 * when the name starts with 'r'. Returns 0 when the kind is unknown. */
int actorExtentShapeFromName(ActorExtentShape *shape, char *name) {
    float scale;
    ShapeRecord *nameRecord = gShapeNameRecordTable;

    shape->drawEnabled = 1;
    shape->extents.x = shape->extents.y = shape->extents.z = 1.0f;
    shape->extents.w = MMathCalcLengthXZ(&shape->extents) * 0.5f;
    shape->angle50 = SHAPE_ANGLE_CODE_DEFAULT;
    shape->angle54 = 0;
    shape->angle58 = SHAPE_ANGLE_CODE_NONE;
    shape->keepsSlotFlags = 0;
    if (name == 0) {
        return 1;
    }
    scale = name[0] == 'r' ? 5.0f : 1.0f;
    if (func_0030EBD0(name, D_00550A60, 3) == 0 || func_0030EBD0(name, D_00550A68, 3) == 0) {
        shape->keepsSlotFlags = 1;
    }
    while (nameRecord->name != 0) {
        if (func_0030EBD0(nameRecord->name, name, 7) != 0) {
            nameRecord++;
            continue;
        }
        {
            u32 shapeKind = nameRecord->shapeKind;

            if (shapeKind < 10) {
                shape->drawEnabled = 1;
                shape->extents.x = shape->extents.z = scale * gShapeSquareExtentTable[shapeKind];
                shape->extents.w = MMathCalcLengthXZ(&shape->extents) * 0.5f;
            } else if (shapeKind < 18) {
                shape->drawEnabled = 0;
                shape->extents.x = shape->extents.w = scale * gShapeCircleRadiusTable[shapeKind - 10];
            } else if (shapeKind < 25) {
                shape->drawEnabled = 1;
                shape->extents.x = scale * gShapeRectExtentTable[shapeKind - 18].x;
                shape->extents.z = scale * gShapeRectExtentTable[shapeKind - 18].z;
                shape->extents.w = MMathCalcLengthXZ(&shape->extents) * 0.5f;
            } else {
                return 0;
            }
            shape->extents.y = nameRecord->heightY;
            shape->angle58 = nameRecord->recordAngleThird;
            shape->angle50 = nameRecord->recordAngleFirst;
            shape->angle54 = gShapeAngleSecondTable[nameRecord->recordAngleSecondIndex];
            if (func_0030EBD0(name + 2, D_00550A70, 3) == 0) {
                shape->angle54 = 0;
                shape->angle58 = SHAPE_ANGLE_CODE_NONE;
            }
            return 1;
        }
    }
    return 0;
}

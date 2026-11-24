#include "common.h"
#include "shared.h"
#include "main/tu132.h"
#include "main/xgl_2.h"
#include "vu0_macro.h"
#include "main/tu118.h"
#include "main/xenoActor.h"
#include "main/sef_135.h"
#include "quadword.h"
#include "main/sef_132.h"

/* Functions of src/main/sef.c called by src/main/sef_132.c. */
struct SefLocalData;
SefEntry *sefFindEntryById(int key);
f32 sefGetEntryScale(s32 id);
SefLinkedContext *sefGetBattleData(void);
unsigned char *sevGetPtAllocator2(int allocatorIndex);
SefEffectRecord *sefGetScheduler(void);
void *sefGetActiveTableSlot(int index);
struct SefLocalData *sefGetLineAdr(int lineIndex);

typedef unsigned int Quadword __attribute__((mode(TI)));
typedef union SefQuadwordView {
    Quadword quadword;
    unsigned int wordLanes[4];
} SefQuadwordView;
typedef struct SefNoiseTable {
    f32 samples[1024];
    unsigned char unmodeled_1000[12];
    int sampleCursor;
} SefNoiseTable;
typedef struct SefLinkedObjectRecord {
    unsigned char unmodeled_000[0x155C];
    unsigned char flags;             /* +0x155C */
} SefLinkedObjectRecord;
typedef struct SefGroupTimer {
    float center[4];               /* +0x00: x, y, z, w of the group centre */
    int owner;                     /* +0x10: owner id compared by sefGetGroupCenter and sefRegisterGroupCenter */
    short remaining;               /* +0x14: frames left; zero or less means the entry is free */
    unsigned short key;            /* +0x16: second id of the group, compared as an unsigned short */
    unsigned char unmodeled_18[8]; /* +0x18..0x1F: not read by the code in this unit */
} SefGroupTimer;
struct SefRecordOwner;
void sefGetCubePosBtm(Vector4 *pos, const Vector4 *extent);
void sefGetCubePosTop(Vector4 *pos, const Vector4 *extent);
void sefNoiseInit(void);
void sefGetCirclePos(Vector4 *pos, float radius);
void sefMergeMatrixPos(Matrix4 *dest, Matrix4 *matrix, const Vector4 *position);
float sefRandAngle(void);
float xglSin(float angle);
float xglCos(float angle);
void MMathRandomSeed(int seed);
float MMathMakeRandom(void);
Matrix4 *MMathRotateMatrixZYX(Matrix4 *out, Matrix4 *matrix, const Vector4 *angles);
Matrix4 *MMathScaleMatrix(Matrix4 *out, Matrix4 *matrix, const Vector4 *scale);
/* The LCG of the seed: multiplier 0xF82D and increment 0x303B; the high 15 bits (SEF_RAND_MASK) are the sample. */
#define SEF_RAND_MULTIPLIER 0xF82Du
#define SEF_RAND_INCREMENT 0x303Bu
#define SEF_RAND_MASK 0x7FFF
#define SEF_NOISE_SCALE 32768.0f
/* LCG state of sefRandf, stored at sefRandSeed. */
extern unsigned int sefRandSeed;
/* Set to 1 once sefNoiseInit has filled the noise table. */
extern int sefNoiseReady;
/* 1024 uniform noise samples in [-0.5, 0.5) with a read index. sefNoiseNextVector reads three samples at a time and
 * can read the three floats after the 1024th, which sefNoiseInit also fills (1027 samples in all). */
extern SefNoiseTable sefNoiseTable;
extern unsigned int sefRandVectorSeeds[4];
extern unsigned int sefRandVectorIncrements[4];
Vector4 *MMathNormalizeVector2(Vector4 *destination, const Vector4 *source);
void sefMemZero(void *dest, int size);
void *memset(void *destination, int value, unsigned int size);
void func_0025A000(Vector4 *value, SefHermiteKey *key, SefHermiteKey *nextKey, float fraction);
float MMathCalcLength(const Vector4 *vector);
void rotationMatrixToQuaternion(void *output, void *input);
extern s32 sefVectorMode;
extern SefMotionObject *D_005510A4;
extern const Vector4 sefDefaultPoint;
Vector4 *MMathApplyMatrix(Vector4 *out, Matrix4 *matrix, const Vector4 *vector);
Matrix4 *MMathScaleMatrixRotS(Matrix4 destination, const Matrix4 source, float scale);
int sefDirectionToAngles(Vector4 *destination, const Vector4 *from, const Vector4 *to);
void sefGetSourceMatrix(void *matrix, const void *source);
Matrix4 *MMathMulMatrix(Matrix4 *out, Matrix4 *rhs, Matrix4 *lhs);
void vu0RotateVectorAboutX(Vector4 *destination, Vector4 *source, float scale);
void quaternionFromAngleY(Vector4 *destination, float scale);
short func_00206A50(SefRunEntry *entries);
void func_00206AA8(Matrix4 *destination, Vector4 *pointAOut, Vector4 *pointBOut, int kindA, int kindB);
extern SefEffectEntryRecord *sefCurrentEffectRecord;
Matrix4 *func_002046F8(Vector4 *dest, const Vector4 *source, int kind);
void func_00207490(AlignedVector4 *p, s32 *c, f32 rad);
void sefTransformPartPoint(SefPartPlacement *placement, Vector4 *resultPoint, int transformMode, const Vector4 *offsetVector);
float sefPlacePart(SefPartPlacement *placement, Vector4 *resultPoint, int placeMode);
void sefMemZero(void *zeroedMemory, int byteLength);
void sefMemFillWords(void *fillMemory, int fillPattern, int fillByteLength);
void sefNoiseNextVector(Vector4 *pos);
void sefGetPosition(Vector4 *resultPoint, const Vector4 *sourcePoint, int pointKind);
s32 sefFindSearchKey(SefSearchOwner *owner, s32 key);
void func_00206D78(AlignedVector4 *out, unsigned char *line, float time);
void func_00206F18(AlignedVector4 *out, unsigned char *line, float time);
f32 sefNoiseNext(void);
f32 sefRandf(void);
extern int D_005510A8;
int sefGetLinkedFlag(int linkedFlagIndex);
extern SefGlobalTransform gSdvCameraPartTransform;
extern unsigned long gCameraSelectFlags;
extern int sefEffectRecordCount;
s32 sefGetBattlePoint(AlignedVector4 *o, s32 i, s32 t);
void sefApplyMatrixVector(Vector4 *transformedVector, Matrix4 *matrix, Vector4 *vector);
extern const Matrix4 MMathUnitMatrix;
struct XglCamera;
int sefBuildEffectMatrix(const Vector4 *rotation, const Vector4 *translation, const Vector4 *scale, Matrix4 *effectMatrix);
extern SefCameraMatrixBlock sefCameraMatrixBlock;
void func_002071F8(void);
extern Vector4 sefOriginVector;
void sefTickGroupTimers(void);
void sefResetEntryFlagsAndOrigin(void);
struct XglCamera *xglStudioGetActiveCamera(void);
void sefGetGroupCenter(float *center, int owner, unsigned short key);
extern SefEntryTableShort sefEffectEntryTable;
void sefGetCameraPos(Matrix4 *dest);
struct SefLocalData;
#define sefCurrentEffectRecord (*(SefEffectRecord **)&sefCurrentEffectRecord)
void sefMemFillWords(void *dest, int value, int size);
float sefNoiseNext(void);
float sefRandf(void);

/* Stores value into each of the size / 4 words starting at dest. */
void sefMemFillWords(void *fillMemory, int fillPattern, int fillByteLength) {
    int *wordCursor;
    int wordsLeft;

    wordCursor = fillMemory;
    wordsLeft = fillByteLength >> 2;
    if (wordsLeft > 0) {
        do {
            wordsLeft -= 1;
            *wordCursor = fillPattern;
            wordCursor += 1;
        } while (wordsLeft != 0);
    }
}

/* Zeroes byteLength bytes: whole quadwords first, then the remaining words. */
void sefMemZero(void *zeroedMemory, int byteLength)
{
    SefQuadwordView *quadwordBase = zeroedMemory;
    int quadwordCount = byteLength >> 4;
    int tailWordCount = (byteLength & 0xf) >> 2;
    SefQuadwordView *tailBlock = quadwordBase + quadwordCount;
    int i;

    for (i = 0; i < quadwordCount; i++) {
        quadwordBase[i].quadword = 0;
    }
    for (i = 0; i < tailWordCount; i++) {
        tailBlock->wordLanes[i] = 0;
    }
}

/* Fills the noise table once with uniform samples in [-0.5, 0.5) and resets its read index to zero. */
void sefNoiseInit(void)
{
    f32 sample;
    f32 *cursor;
    int i;

    if (sefNoiseReady == 0) {
        sefNoiseReady = 1;
        MMathRandomSeed(0);
        sefNoiseTable.sampleCursor = 0;
        cursor = sefNoiseTable.samples;
        for (i = 0; i < 1027; i++) {
            sample = MMathMakeRandom() - 0.5f;
            *cursor = sample;
            cursor++;
        }
    }
}

/* Random float in [0, 1): one LCG step, the high 15 bits scaled by 2^-15. */
f32 sefRandf(void)
{
    int bits;

    sefRandSeed = sefRandSeed * SEF_RAND_MULTIPLIER + SEF_RAND_INCREMENT;
    bits = (sefRandSeed >> 16) & SEF_RAND_MASK;
    return bits * (1.0f / 32768.0f);
}

/* Returns the next noise sample and advances the read index of the noise table. */
f32 sefNoiseNext(void)
{
    int sampleIndex = sefNoiseTable.sampleCursor;

    sefNoiseTable.sampleCursor = (sampleIndex + 1) & 0x3FF;
    return sefNoiseTable.samples[sampleIndex];
}

/* Random float in [0.75, 1): one LCG step of sefRandSeed, the high 15 bits scaled by 2^-15, times 0.25, plus 0.75. */
f32 sefRandfUpperQuarter(void)
{
    unsigned int nextSeed;

    nextSeed = sefRandSeed * SEF_RAND_MULTIPLIER + SEF_RAND_INCREMENT;
    sefRandSeed = nextSeed;
    return (int) ((nextSeed >> 16) & SEF_RAND_MASK) * 3.0517578125e-05f * 0.25f + 0.75f;
}

/* Random angle in [0, 2*pi): the same LCG step as sefRandf, scaled by 2^-15 and then by 2*pi. */
f32 sefRandAngle(void) {
    int bits;

    sefRandSeed = sefRandSeed * SEF_RAND_MULTIPLIER + SEF_RAND_INCREMENT;
    bits = (sefRandSeed >> 16) & 0x7FFF;
    return bits * (1.0f / 32768.0f) * 6.28318531f;
}

/* Random vector: steps the x, y and z seeds of the vector LCG (times 0xF82D, then all four lanes plus the increments in one paddw), keeps the high 15 bits of each lane and converts them to floats in out. */
void sefRandVector(Vector4 *randomVector)
{
    unsigned int *laneSeeds;
    Quadword mask;                  /* the 128-bit literal of the original: 0x7FFF in x, y and z, but only 1 in w */

    laneSeeds = sefRandVectorSeeds;
    mask = 0x100007FFF00007FFF00007FFFULL;
    laneSeeds[0] *= 0xF82D;
    laneSeeds[1] *= 0xF82D;
    laneSeeds[2] *= 0xF82D;
    __asm__ __volatile__(
        "lq $8, 0(%1)\n"
        "lq $9, 0(%2)\n"
        "paddw $8, $8, $9\n"
        "sq $8, 0(%1)\n"
        "psrlw $8, $8, 16\n"
        "pand $8, $8, %3\n"
        "qmtc2 $8, vf1\n"
        "vitof0.xyzw vf1, vf1\n"
        "sqc2 vf1, 0(%0)\n"
        : : "r"(randomVector), "r"(laneSeeds), "r"(sefRandVectorIncrements), "r"(mask) : "$8", "$9", "memory");
}

/* Three random coordinates in [-0.5, 0.5), drawn in x, y, z order; w is set to 1. */
void sefNoiseNextVector(Vector4 *pos)
{
    pos->x = MMathMakeRandom() - 0.5f;
    pos->y = MMathMakeRandom() - 0.5f;
    pos->z = MMathMakeRandom() - 0.5f;
    pos->w = 1.0f;
}

/* Writes the position transformed by the matrix into dest (the position's translation merged into the matrix). */
void sefMergeMatrixPos(Matrix4 *destinationMatrix, Matrix4 *matrix, const Vector4 *position)
{
    QuadwordMergeMatrixPosition(destinationMatrix, matrix, position);
}

/* Builds the effect matrix in out (the scratchpad when out is null): the identity rows of MMathUnitMatrix are copied
 * first, then a nonzero translation becomes row 3, a nonzero rotation is applied, and a scale that is not all ones.
 * Returns how many of the three steps ran. */
int sefBuildEffectMatrix(const Vector4 *rotation, const Vector4 *translation, const Vector4 *scale, Matrix4 *effectMatrix)
{
    Matrix4 *destMatrix;
    const Matrix4 *unitMatrix;
    int stepsRun;
    int hasTranslation;
    int hasRotation;

    destMatrix = effectMatrix;
    if (destMatrix == 0) {
        destMatrix = &SEF_SCRATCHPAD(Matrix4, 0x0);
    }
    stepsRun = 0;
    unitMatrix = &MMathUnitMatrix;
    __asm__ __volatile__(
        "lqc2 $vf1, 32(%1)\n"
        "lqc2 $vf2, 16(%1)\n"
        "lqc2 $vf3, 0(%1)\n"
        "sqc2 $vf0, 48(%0)\n"
        "sqc2 $vf1, 32(%0)\n"
        "sqc2 $vf2, 16(%0)\n"
        "sqc2 $vf3, 0(%0)\n"
        :
        : "r" (destMatrix), "r" (unitMatrix)
        : "memory");
    hasTranslation = (*(int *)&translation->z | *(u64 *)translation) != 0;
    if (hasTranslation) {
        QuadwordCopyT0((*destMatrix)[3], translation);
        stepsRun = 1;
    }
    hasRotation = (*(int *)&rotation->z | *(u64 *)rotation) != 0;
    if (hasRotation) {
        MMathRotateMatrixZYX(destMatrix, destMatrix, rotation);
        stepsRun++;
    }
    if (scale != 0 && !(scale->x == 1.0f && scale->y == 1.0f && scale->z == 1.0f)) {
        MMathScaleMatrix(destMatrix, destMatrix, scale);
        stepsRun++;
    }
    return stepsRun;
}

/* Writes the camera's travel matrix with the default point merged in (into sefCameraMatrix when destMatrix is null). */
void sefGetCameraPos(Matrix4 *destMatrix)
{
    XglCamera *camera = xglStudioGetActiveCamera();

    if (destMatrix == 0) {
        destMatrix = sefCameraMatrix;
    }
    sefMergeMatrixPos(destMatrix, &camera->travel.transform.rows, &sefDefaultPoint);
}

/* VU0 product of a Vector4 and a column-major matrix: destination = matrix * vector. */
void sefApplyMatrixVector(Vector4 *transformedVector, Matrix4 *matrix, Vector4 *vector)
{
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(vector) : "memory");
    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("lqc2 vf3, 16(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("lqc2 vf4, 32(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("vmulax.xyzw ACC, vf2, vf1x" : : : "memory");
    __asm__ __volatile__("vmadday.xyzw ACC, vf3, vf1y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyzw ACC, vf4, vf1z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyzw vf1, vf0, vf1w" : : : "memory");
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(transformedVector) : "memory");
}

/* Copies one 16-byte vector through t0 with the R5900 quadword load and store. */
static inline void sefCopyQuadword(Vector4 *destVector, const void *sourceVector)
{
    __asm__ __volatile__(
        "lq $8, 0(%2)\n"
        "sq $8, 0(%1)\n"
        : "+m"(*destVector)
        : "r"(destVector), "r"(sourceVector)
        : "$8", "memory");
}

/* Copies the vector of slot of the record found by key into *destination when that slot's flag byte is set, and
 * returns bit 0 of the flag byte (0 without a record). */
int sefCopyRecordSlotVector(Vector4 *destinationVector, int key, int vectorSlot)
{
    SefEntry *foundEntry = sefFindEntryById(key);

    if (foundEntry == 0) {
        return 0;
    }
    if (foundEntry->slotFlags[vectorSlot] != 0) {
        sefCopyQuadword(destinationVector, &foundEntry->slotVectors[vectorSlot]);
    }
    return foundEntry->slotFlags[vectorSlot] & 1;
}

/* Partial view of one linked object record: its flag byte at +0x155C, bit 0 is the in-use bit. */
/* Returns bit 0 of flag byte linkIndex of the linked object table (objectOffset), or 0 without a link. */
int sefGetLinkedFlag(int linkedFlagIndex)
{
    SefLinkedContext *linkedContext = sefGetBattleData();
    int linkedObjectOffset = linkedContext->objectOffset;

    if (linkedObjectOffset == 0) {
        return 0;
    }
    return ((SefLinkedObjectRecord *)(linkedFlagIndex + linkedObjectOffset))->flags & 1;
}

void vu0DirectionToAngles(Vector4 *angles, Vector4 *direction);

/* Direction from `from` to `to` as the pair of angles applied to outVector (returns 1). When the two points
 * coincide, outVector gets vf0 instead (returns 0). */
int sefDirectionToAngles(Vector4 *outVector, const Vector4 *from, const Vector4 *to) {
    AlignedVector4 diff;
    AlignedVector4 angles;
    int isZeroDirection;
    int result;

    result = 0;
    Vu0SefSubVectorXYZ(&diff, from, to);
    isZeroDirection = ((*(int *) &diff.z | *(u64 *) &diff) == 0);
    if (isZeroDirection != 0) {
        __asm__ __volatile__("sqc2 vf0, 0(%0)" : : "r"(outVector) : "memory");
    } else {
        vu0DirectionToAngles(&angles, &diff);
        quaternionFromAngleY(outVector, angles.y);
        vu0RotateVectorAboutX(outVector, outVector, angles.x);
        result = 1;
    }
    return result;
}

/* Four 16-byte rows of a 4x4 matrix. */
typedef struct Mat4Rows {
    Vector4 r[4];
} Mat4Rows;

#define CopyUnitMatrixVu0(outMatrix, inMatrix) \
    __asm__ __volatile__( \
        "lqc2 vf1,0x20(%1)\n\tlqc2 vf2,0x10(%1)\n\tlqc2 vf3,0(%1)\n\t" \
        "sqc2 vf0,0x30+%0\n\tsqc2 vf1,0x20+%0\n\tsqc2 vf2,0x10+%0\n\tsqc2 vf3,%0" \
        : "=m"(*(Mat4Rows *)(outMatrix)) : "r"(inMatrix))

void sefCopyKindTableMatrix(Matrix4 *dest, int matrixIndex, int kind);

#define CopyMatrixFromPointer(outMatrix, inMatrix) \
    __asm__ __volatile__( \
        "lq $8,0(%1)\n\tlq $9,0x10(%1)\n\tlq $10,0x20(%1)\n\tlq $11,0x30(%1)\n\t" \
        "sq $8,0+%0\n\tsq $9,0x10+%0\n\tsq $10,0x20+%0\n\tsq $11,0x30+%0" \
        : "=m"(*(Mat4Rows *)(outMatrix)) : "r"(inMatrix) : "$8", "$9", "$10", "$11")

/* Source matrix for the kind bits in kindBits: the battle matrices for kinds 256 and 512, the kind tables for the
 * other bits, the unit matrix when no kind bit is set. */
void sefGetSourceMatrix(void *matrix, const void *kindBits) {
    SefLinkedContext *battle;
    Matrix4 *dest;
    int flags;

    dest = matrix;
    flags = (int) kindBits;
    battle = sefGetBattleData();
    if (flags == 256) {
        CopyMatrixFromPointer(dest, battle->kind256Matrix);
    } else if ((flags & 0x100) != 0) {
        sefCopyKindTableMatrix(dest, flags - 257, 1);
    } else if (flags == 512) {
        CopyMatrixFromPointer(dest, battle->kind512Matrix);
    } else if ((flags & 0x200) != 0) {
        sefCopyKindTableMatrix(dest, flags - 513, 0);
    } else if ((flags & 0x400) != 0) {
        sefCopyKindTableMatrix(dest, flags - 1025, 2);
    } else if ((flags & 0x800) != 0) {
        if (*(int *) &battle->unmodeled_008[4] == 0 && *(int *) &battle->unmodeled_008[0] != 0) {
            sefCopyKindTableMatrix(dest, flags - 2049, 2);
        } else {
            sefCopyKindTableMatrix(dest, flags - 2049, 3);
        }
    } else if ((flags & 0x1000) != 0) {
        sefCopyKindTableMatrix(dest, flags - 4097, 4);
    } else {
        CopyUnitMatrixVu0(dest, &MMathUnitMatrix);
    }
}

void sefApplySourceMatrix(void *transformedOutput, const void *sourceMatrixBlock) {
    Matrix4 matrix;

    sefGetSourceMatrix(matrix, sourceMatrixBlock);
    rotationMatrixToQuaternion(transformedOutput, matrix);
}

/* A point as a vector or as four floats. */
typedef union GbUV {
    AlignedVector4 vector;
    f32 floats[4];
} GbUV;

typedef struct GbPoints {
    s32 unmodeled_000[3];
    s8 unmodeled_00C[0xCC0 - 12];
    AlignedVector4 vectors[0x33];
    s32 unmodeled_FF0[300];
} GbPoints;

/* Partial view of the battle data read by the battle point lookup (sefGetBattlePoint). */
typedef struct GbData {
    u8 unmodeled_000[0x2C4];
    f32 battleY;
    GbPoints *points[6];
} GbData;

#define SEF_GET_BATTLE_DATA ((GbData *(*)(void))sefGetBattleData)

static inline void GbCopy(AlignedVector4 *outVector, const AlignedVector4 *inVector) {
    __asm__ __volatile__("lq $8,0(%5)\n\tsq $8,0(%4)"
                         : "=m"(outVector->x), "=m"(outVector->y), "=m"(outVector->z), "=m"(outVector->w)
                         : "r"(outVector), "r"(inVector)
                         : "$8");
}

/* Copies point slot of the battle point table into dest when its flag byte says it is set; returns the flag bit. */
#define GB_POINT(slot, EXTRA) \
    if (battle->points[slot] != 0) { \
        if (battle->points[slot]->unmodeled_00C[index + 0x1550] == 0) { \
            battle->points[slot]->unmodeled_00C[index + 0x1550] = -128; \
        } \
        GbCopy(dest, &battle->points[slot]->vectors[index]); \
        EXTRA \
        return battle->points[slot]->unmodeled_00C[index + 0x1550] & 1; \
    }

/* Battle point lookup by kind: kinds 1, 0, 2, 3, 4 and 6 select point table entries 0 to 5 (kind 1 also takes
 * the y of the battle data). Returns 0 when the point is missing. */
s32 sefGetBattlePoint(AlignedVector4 *dest, s32 index, s32 kind) {
    GbData *battle = SEF_GET_BATTLE_DATA();
    if (kind == 1) {
        GB_POINT(0, if (battle->battleY != 0.0f) ((GbUV *) dest)->floats[1] = battle->battleY;)
    } else if (kind == 0) {
        GB_POINT(1, )
    } else if (kind == 2) {
        GB_POINT(2, )
    } else if (kind == 3) {
        GB_POINT(3, )
    } else if (kind == 4) {
        GB_POINT(4, )
    } else if (kind == 6) {
        GB_POINT(5, )
    }
    return 0;
}

typedef struct SefKindTable {
    unsigned char unmodeled_000[0xFF0];
    AlignedVector4 slot[86];
    unsigned char unmodeled_1550[0x155C - 0x1550];
    unsigned char flag[1];
} SefKindTable;

typedef struct SefBattleObjectPointers {
    unsigned char unmodeled_000[0x2C8];
    int object[4];
} SefBattleObjectPointers;

/* Copies slot index of the battle object table selected by kind into outVector and returns its flag bit. Returns 0
 * when kind selects no table or the table is missing. */
int sefCopyBattleObjectSlot(AlignedVector4 *outVector, int index, int kind) {
    int table = (int) sefGetBattleData();

    if (kind == 1) {
        table = ((SefBattleObjectPointers *) table)->object[0];
    } else if (kind == 0) {
        table = ((SefBattleObjectPointers *) table)->object[1];
    } else if (kind == 2) {
        table = ((SefBattleObjectPointers *) table)->object[2];
    } else if (kind == 3) {
        table = ((SefBattleObjectPointers *) table)->object[3];
    } else {
        goto none;
    }
    if (table != 0) {
        AlignedVector4 *slot = (AlignedVector4 *) (index * 16 + table + 0xFF0);

        __asm__ __volatile__("lq $8,0(%1)\n\tsq $8,0(%0)"
                             :
                             : "r"(outVector), "r"(slot), "m"(outVector->x), "m"(outVector->y),
                               "m"(outVector->z), "m"(outVector->w)
                             : "$8", "memory");
        return ((SefKindTable *) table)->flag[index] & 1;
    }
none:
    return 0;
}

static inline void sefCopyKindMatrix(Matrix4 *dest, Matrix4 *source) {
    QuadwordCopyMatrixT0(dest, source);
}

/* Copies matrix matrixIndex of the table selected by kind into dest; the unit matrix when the table is missing. */
void sefCopyKindTableMatrix(Matrix4 *dest, int matrixIndex, int kind) {
    SefLinkedContext *linkedContext = sefGetBattleData();
    Matrix4 *matrixTable;

    if (kind == 1) {
        matrixTable = (Matrix4 *) linkedContext->objectOffset;
        if (matrixTable == 0) {
            return;
        }
        sefCopyKindMatrix(dest, &matrixTable[matrixIndex]);
        return;
    }
    if (kind == 0) {
        matrixTable = (Matrix4 *) linkedContext->kindTableOffset[0];
        if (matrixTable == 0) {
            return;
        }
        sefCopyKindMatrix(dest, &matrixTable[matrixIndex]);
        return;
    }
    if (kind == 2) {
        matrixTable = (Matrix4 *) linkedContext->kindTableOffset[1];
        if (matrixTable != 0) {
            sefCopyKindMatrix(dest, &matrixTable[matrixIndex]);
            return;
        }
    }
    if (kind == 3) {
        matrixTable = (Matrix4 *) linkedContext->kindTableOffset[2];
        if (matrixTable != 0) {
            sefCopyKindMatrix(dest, &matrixTable[matrixIndex]);
            return;
        }
    }
    if (kind == 4) {
        matrixTable = (Matrix4 *) linkedContext->kindTableOffset[3];
        if (matrixTable != 0) {
            sefCopyKindMatrix(dest, &matrixTable[matrixIndex]);
            return;
        }
    }
    if (kind == 6) {
        matrixTable = (Matrix4 *) linkedContext->kindTableOffset[4];
        if (matrixTable != 0) {
            sefCopyKindMatrix(dest, &matrixTable[matrixIndex]);
            return;
        }
    }
    Vu0CopyUnitMatrix(dest, &MMathUnitMatrix);
}

void sefMergeMatrixPos(Matrix4 *destinationMatrix, Matrix4 *matrix, const Vector4 *position);
extern short D_004749D4[6];
extern Matrix4 sefScratchMatrix;
extern Vector4 D_00475380;

/* Copies the point at inVector to outVector through $8. */
static inline void pointCopy(Vector4 *outVector, const void *inVector) {
    __asm__ __volatile__("lq $8, 0(%5)\n\tsq $8, 0(%4)"
                         : "=m"(outVector->x), "=m"(outVector->y), "=m"(outVector->z), "=m"(outVector->w)
                         : "r"(outVector), "r"(inVector), "m"(((const Vector4 *) inVector)->x), "m"(((const Vector4 *) inVector)->y),
                           "m"(((const Vector4 *) inVector)->z), "m"(((const Vector4 *) inVector)->w)
                         : "$8");
}

/* outVector = left + right in xyz, through vf1 and vf2. */
static inline void pointAdd(Vector4 *outVector, const void *left, const void *right) {
    __asm__ __volatile__("lqc2 vf1, 0(%1)\n\tlqc2 vf2, 0(%2)\n\tvadd.xyz vf1, vf1, vf2\n\tsqc2 vf1, 0(%0)"
                         :
                         : "r"(outVector), "r"(left), "r"(right)
                         : "memory");
}

/* outVector = left * right in xyz, through vf1 and vf2. */
static inline void pointMul(Vector4 *outVector, const void *left, const void *right) {
    __asm__ __volatile__("lqc2 vf1, 0(%1)\n\tlqc2 vf2, 0(%2)\n\tvmul.xyz vf1, vf1, vf2\n\tsqc2 vf1, 0(%0)"
                         :
                         : "r"(outVector), "r"(left), "r"(right)
                         : "memory");
}

/* Copies the 4x4 matrix at inMatrix to outMatrix through $8 to $11. */
static inline void matrixCopy(Matrix4 *outMatrix, const void *inMatrix) {
    __asm__ __volatile__(
        "lq $8, 0(%1)\n\tlq $9, 16(%1)\n\tlq $10, 32(%1)\n\tlq $11, 48(%1)\n\tsq $8, 0(%0)\n\tsq $9, 16(%0)\n\tsq $10, 32(%0)\n\tsq $11, 48(%0)"
        :
        : "r"(outMatrix), "r"(inMatrix)
        : "$8", "$9", "$10", "$11", "memory");
}

#define SEF_RECORD_BYTE(offset) (*(signed char *) ((char *) sefCurrentEffectRecord + (offset)))
#define SEF_BATTLE_MATRIX(offset) ((Matrix4 *) ((char *) battle + (offset)))


/* Resolves the point of kind into dest, reading the source point when one is given: camera, part, battle-matrix or
 * table point, selected by the kind range. Returns the matrix block to apply later, or 0 when dest is already final. */
Matrix4 *func_002046F8(Vector4 *dest, const Vector4 *sourcePoint, int kind) {
    extern Vector4 sefOriginVector;
    extern Vector4 *MMathApplyMatrix(Vector4 * out, Matrix4 * matrix, const Vector4 *vector);
    extern void sefTransformPartPoint(SefPartPlacement * placement, Vector4 * resultPoint, int transformMode,
                                      const Vector4 *offsetVector);
    Vector4 flatOffset;
    Vector4 shiftedOffset;
    Vector4 shiftedOffset2;
    SefLinkedContext *battle;
    Matrix4 *matrixBlock;
    Matrix4 *entryBlock;
    Matrix4 *entryBlock2;
    Vector4 *attachOffset;
    int vectorMode;
    int useScale;
    int sourceNonzero;
    int hasSourcePoint;
    int index;
    int keepSourceY;
    float length;

    matrixBlock = 0;
    useScale = 0;
    battle = sefGetBattleData();
    vectorMode = sefVectorMode;
    if (sourcePoint != 0) {
        sourceNonzero = (((int *) sourcePoint)[2] | *(long long *) sourcePoint) != 0;
        hasSourcePoint = sourceNonzero;
    } else {
        hasSourcePoint = 0;
    }
    __asm__ __volatile__("sqc2 $vf0, 0(%1)" : "=m"(*dest) : "r"(dest) : "memory");
    if (kind == 1 || kind == 4 || kind == 5 || kind == 6) {
        if (sourcePoint == 0) {
            sourcePoint = &sefDefaultPoint;
        }
        pointCopy(&flatOffset, sourcePoint);
        flatOffset.y = 0.0f;
        sefApplyMatrixVector(dest, &sefCameraMatrixBlock, &flatOffset);
        dest->y = sourcePoint->y;
        pointAdd(dest, dest, (*(Matrix4 *) &sefCameraMatrixBlock)[3]);
        return 0;
    }
    if (kind == 2) {
        struct XglCamera *camera = xglStudioGetActiveCamera();

        if (sourcePoint == 0) {
            sourcePoint = &sefDefaultPoint;
        }
        sefApplyMatrixVector(dest, &camera->travel.transform.rows, (Vector4 *) sourcePoint);
        pointAdd(dest, dest, ((Matrix4 *) &camera->travel.transform.rows)[0][3]);
        return 0;
    }
    if (kind < 9) {
        if (hasSourcePoint != 0) {
            pointCopy(dest, sourcePoint);
        }
        return 0;
    }
    if ((kind == 59 && (index = SEF_RECORD_BYTE(0x144)) >= 0) ||
        (kind == 61 && (index = SEF_RECORD_BYTE(0x145)) >= 0)) {
        if (vectorMode == 0) {
            matrixBlock = sefGetActiveTableSlot(index);
            useScale = 1;
        }
    } else if (kind == 62 && SEF_RECORD_BYTE(0x147) >= 0) {
        if (vectorMode != 0) {
            entryBlock = sefGetActiveTableSlot(SEF_RECORD_BYTE(0x149));
            pointCopy(dest, entryBlock);
        } else {
            entryBlock = sefGetActiveTableSlot(SEF_RECORD_BYTE(0x147));
            pointCopy(dest, (char *) entryBlock + 48);
            entryBlock = sefGetActiveTableSlot(SEF_RECORD_BYTE(0x144));
            sefMergeMatrixPos(&sefScratchMatrix, entryBlock, dest);
            matrixBlock = &sefScratchMatrix;
        }
    } else if (kind == 63 && SEF_RECORD_BYTE(0x148) >= 0) {
        if (sefVectorMode == -1) {
            entryBlock = sefGetActiveTableSlot(SEF_RECORD_BYTE(0x145));
            length = MMathCalcLength((Vector4 *) entryBlock);
            entryBlock2 = sefGetActiveTableSlot(SEF_RECORD_BYTE(0x148));
            pointCopy(dest, (char *) entryBlock2 + 48);
            ((Matrix4 * (*) (Matrix4 * out, Matrix4 * matrix, float scale))
                 MMathScaleMatrixRotS)(&sefScratchMatrix, entryBlock, 1.0f / length);
            sefMergeMatrixPos(&sefScratchMatrix, &sefScratchMatrix, dest);
            matrixBlock = &sefScratchMatrix;
        } else if (vectorMode != 0) {
            entryBlock = sefGetActiveTableSlot(SEF_RECORD_BYTE(0x149));
            pointCopy(dest, (char *) entryBlock + 16);
        } else {
            entryBlock = sefGetActiveTableSlot(SEF_RECORD_BYTE(0x148));
            pointCopy(dest, (char *) entryBlock + 48);
            entryBlock = sefGetActiveTableSlot(SEF_RECORD_BYTE(0x145));
            sefMergeMatrixPos(&sefScratchMatrix, entryBlock, dest);
            matrixBlock = &sefScratchMatrix;
        }
    } else if ((kind == 64 || kind == 65) && SEF_RECORD_BYTE(0x146) >= 0 && SEF_RECORD_BYTE(0x149) >= 0) {
        entryBlock = sefGetActiveTableSlot(SEF_RECORD_BYTE(0x149));
        attachOffset = (Vector4 *) ((char *) entryBlock + 32);
        entryBlock2 = sefGetActiveTableSlot(SEF_RECORD_BYTE(0x146));
        if (sourcePoint != 0) {
            pointAdd(&shiftedOffset, sourcePoint, attachOffset);
        } else {
            pointCopy(&shiftedOffset, attachOffset);
        }
        entryBlock = sefGetActiveTableSlot(SEF_RECORD_BYTE(0x148));
        pointCopy(dest, (char *) entryBlock + 48);
        sefMergeMatrixPos(&sefScratchMatrix, entryBlock2, dest);
        MMathApplyMatrix(dest, &sefScratchMatrix, &shiftedOffset);
        if (kind == 65) {
            if (sourcePoint != 0) {
                dest->y = sourcePoint->y;
            } else {
                dest->y = 0.0f;
            }
        }
        return &sefScratchMatrix;
    } else if (kind == 256) {
        if (battle->source != 0 && vectorMode == 0) {
            matrixBlock = SEF_BATTLE_MATRIX(112);
            useScale = 1;
        }
    } else if (kind == 323) {
        if (battle->source != 0 && vectorMode == 0) {
            matrixCopy(&sefScratchMatrix, SEF_BATTLE_MATRIX(112));
            if (sourcePoint != 0) {
                sefScratchMatrix[3][1] = sourcePoint->y;
            } else {
                sefScratchMatrix[3][1] = 0.0f;
            }
            matrixBlock = &sefScratchMatrix;
            useScale = 1;
        }
    } else if (kind == 512) {
        if (battle->target != 0) {
            matrixBlock = D_004749D4[0] != 0 ? SEF_BATTLE_MATRIX(496) : SEF_BATTLE_MATRIX(176);
        }
    } else if (kind & 0x100) {
        keepSourceY = 0;
        if (kind == 324) {
            keepSourceY = 1;
            kind = 260;
        }
        if (vectorMode != 0) {
            sefCopyBattleObjectSlot((AlignedVector4 *) dest, kind - 257, 1);
            if (keepSourceY) {
                if (sourcePoint != 0) {
                    dest->y = sourcePoint->y;
                } else {
                    dest->y = 0.0f;
                }
            }
        } else {
            sefGetBattlePoint(dest, kind - 257, 1);
            if (keepSourceY) {
                if (sourcePoint != 0) {
                    dest->y = sourcePoint->y;
                } else {
                    dest->y = 0.0f;
                }
                useScale = 1;
            }
            sefMergeMatrixPos(&sefScratchMatrix, SEF_BATTLE_MATRIX(112), dest);
            matrixBlock = &sefScratchMatrix;
        }
    } else if (kind & 0x200) {
        if (kind == 577 || kind == 578) {
            if (sourcePoint != 0) {
                pointAdd(&shiftedOffset2, sourcePoint, &D_00475380);
            } else {
                pointCopy(&shiftedOffset2, &D_00475380);
            }
            sefGetBattlePoint(dest, 3, 0);
            sefMergeMatrixPos(&sefScratchMatrix, SEF_BATTLE_MATRIX(496), dest);
            MMathApplyMatrix(dest, &sefScratchMatrix, &shiftedOffset2);
            if (kind == 578) {
                if (sourcePoint != 0) {
                    dest->y = sourcePoint->y;
                } else {
                    dest->y = 0.0f;
                }
            }
            return &sefScratchMatrix;
        }
        if (kind == 579) {
            if (battle->target != 0) {
                matrixCopy(&sefScratchMatrix, SEF_BATTLE_MATRIX(176));
                if (sourcePoint != 0) {
                    sefScratchMatrix[3][1] = sourcePoint->y;
                } else {
                    sefScratchMatrix[3][1] = 0.0f;
                }
            }
        } else {
            sefGetBattlePoint(dest, kind - 513, 0);
            if (D_004749D4[0] != 0) {
                sefMergeMatrixPos(&sefScratchMatrix, SEF_BATTLE_MATRIX(496), dest);
            } else {
                sefMergeMatrixPos(&sefScratchMatrix, SEF_BATTLE_MATRIX(176), dest);
            }
        }
        matrixBlock = &sefScratchMatrix;
    } else if (kind & 0x400) {
        if (vectorMode != 0) {
            sefCopyBattleObjectSlot((AlignedVector4 *) dest, kind - 1025, 2);
            if (kind != 0) {
                vectorMode = 0;
            }
        } else {
            sefGetBattlePoint(dest, kind - 1025, 2);
            sefMergeMatrixPos(&sefScratchMatrix, SEF_BATTLE_MATRIX(240), dest);
            matrixBlock = &sefScratchMatrix;
        }
    } else if (kind & 0x800) {
        if (vectorMode != 0) {
            sefCopyBattleObjectSlot((AlignedVector4 *) dest, kind - 2049, 3);
        } else {
            if (((int *) battle)[3] == 0 && ((int *) battle)[2] != 0) {
                sefGetBattlePoint(dest, kind - 2049, 2);
            } else {
                sefGetBattlePoint(dest, kind - 2049, 3);
            }
            sefMergeMatrixPos(&sefScratchMatrix, SEF_BATTLE_MATRIX(304), dest);
            matrixBlock = &sefScratchMatrix;
        }
    } else if (kind & 0x1000) {
        if (vectorMode != 0) {
            sefCopyBattleObjectSlot((AlignedVector4 *) dest, kind - 4097, 4);
        } else {
            sefGetBattlePoint(dest, kind - 4097, 4);
            sefMergeMatrixPos(&sefScratchMatrix, SEF_BATTLE_MATRIX(368), dest);
            matrixBlock = &sefScratchMatrix;
        }
    } else if (kind == 16) {
        if (((SefPartPlacement *) D_005510A4) != 0) {
            pointCopy(dest, &((SefPartPlacement *) D_005510A4)->translation);
        }
    } else if (kind == 17) {
        if (((SefPartPlacement *) D_005510A4) != 0) {
            sefTransformPartPoint(((SefPartPlacement *) D_005510A4), dest, D_005510A8, sourcePoint);
            return 0;
        }
    } else if (kind >= 0x2000) {
        return 0;
    } else if (kind == 32) {
        pointCopy(dest, &sefOriginVector);
    }
    if (matrixBlock != 0) {
        if (hasSourcePoint != 0) {
            if (useScale != 0 && sefCurrentEffectRecord != 0) {
                pointMul(dest, (char *) sefCurrentEffectRecord + 48, sourcePoint);
                MMathApplyMatrix(dest, matrixBlock, dest);
            } else {
                MMathApplyMatrix(dest, matrixBlock, sourcePoint);
            }
        } else {
            pointCopy(dest, (char *) matrixBlock + 48);
        }
    } else if (hasSourcePoint != 0) {
        pointAdd(dest, dest, sourcePoint);
    }
    return matrixBlock;
}

/* Kinds 321 and 322 are passed on as kind 256; a NULL source reads sefDefaultPoint. The w component is set to 1.0f. */
void sefGetPosition(Vector4 *resultPoint, const Vector4 *sourcePoint, int pointKind)
{
    if ((unsigned int)(pointKind - 321) < 2) {
        pointKind = 256;
    }
    if (sourcePoint == 0) {
        sourcePoint = &sefDefaultPoint;
    }
    func_002046F8(resultPoint, sourcePoint, pointKind);
    resultPoint->w = 1.0f;
}

void vu0TransformPoint(Vector4 *point);

/* Partial view of a part's transform state: the matrix rows (vf26..vf28), the vector at +0xA0 (loaded into vf29), the scale
 * at +0xC0 and the point at +0x200 that vu0TransformPoint transforms. */
/* Transforms the part's point (mode 0: transform, then scale; mode != 0: scale, then transform when mode >= 0), adds the
 * offset's w term when offset is given, and writes the result to out. */
void sefTransformPartPoint(SefPartPlacement *placement, Vector4 *resultPoint, int transformMode, const Vector4 *offsetVector)
{
    __asm__ __volatile__(
        "lqc2 vf26, 0(%0)\n"
        "lqc2 vf27, 16(%0)\n"
        "lqc2 vf28, 32(%0)\n"
        "lqc2 vf29, 0(%1)\n"
        :
        : "r"(placement), "r"(&placement->translation)
        : "memory");
    if (transformMode != 0) {
        __asm__ __volatile__(
            "lqc2 vf1, 0(%0)\n"
            "vmulx.xyzw vf26, vf26, vf1x\n"
            "vmuly.xyzw vf27, vf27, vf1y\n"
            "vmulz.xyzw vf28, vf28, vf1z\n"
            "vmulw.xyzw vf29, vf29, vf1w\n"
            :
            : "r"(&placement->scale), "m"(placement->scale));
        if (transformMode >= 0) {
            Vector4 *point = &placement->point;
            vu0TransformPoint(point);
        }
    } else {
        vu0TransformPoint(&placement->point);
        __asm__ __volatile__(
            "lqc2 vf1, 0(%0)\n"
            "vmulx.xyzw vf26, vf26, vf1x\n"
            "vmuly.xyzw vf27, vf27, vf1y\n"
            "vmulz.xyzw vf28, vf28, vf1z\n"
            "vmulw.xyzw vf29, vf29, vf1w\n"
            :
            : "r"(&placement->scale)
            : "memory");
    }
    if (offsetVector != 0) {
        __asm__ __volatile__(
            "lqc2 vf1, 0(%0)\n"
            "vmulax.xyzw ACC, vf26, vf1x\n"
            "vmadday.xyzw ACC, vf27, vf1y\n"
            "vmaddaz.xyzw ACC, vf28, vf1z\n"
            "vmaddw.xyzw vf29, vf29, vf1w\n"
            :
            : "r"(offsetVector)
            : "memory");
    }
    __asm__ __volatile__(
        "sqc2 vf29, 0(%0)\n"
        :
        : "r"(resultPoint)
        : "memory");
}

/* Part point for kind: the transform entry index (kind bit 0x100 or 0x200) applied to dest through the part's
 * allocator matrix and offset. Returns with dest unchanged when the allocator or the table is missing. */
void func_002050A8(Vector4 *dest, unsigned char *partRef, int kind, int applyScale) {
    SefLinkedContext *battle;
    unsigned char *allocator;
    unsigned char *partData;
    unsigned char *pointTable;
    Matrix4 stackMatrix;
    Vector4 stackVector;
    struct SefLocalData *lineData;
    int index;
    int lineFlags;
    int lineValue;
    int lineKind;
    float scale;

    index = -1;
    battle = sefGetBattleData();
    scale = 1.0f;
    allocator = sevGetPtAllocator2(*(short *) (partRef + 4));
    partData = *(unsigned char **) partRef;
    pointTable = partData + *(int *) (partData + 264) + 16;
    if (allocator == 0 || pointTable == 0) {
        return;
    }

    if ((kind & 0x100) != 0) {
        index = sefFindSearchKey(partData, kind - 257);
        if (applyScale != 0) {
            scale = sefGetEntryScale(((int *) battle)[0]);
        }
    } else if ((kind & 0x200) != 0) {
        index = sefFindSearchKey(partData, kind - 513);
        if (applyScale != 0) {
            scale = sefGetEntryScale(((int *) battle)[1]);
        }
    }

    if (index >= 0) {
        if ((*(int *) (allocator + 504) & 0x20) != 0) {
            __asm__ __volatile__("lqc2 $vf1, 0(%0)\n"
                                 "lqc2 $vf2, 0(%1)\n"
                                 "vadd.xyz $vf1xyz, $vf1xyz, $vf2xyz\n"
                                 "sqc2 $vf1, 0(%2)\n"
                                 :
                                 : "r"(allocator + 512),
                                   "r"(((unsigned char *) (index * 32 + (int) pointTable + 16))), "r"(dest)
                                 : "memory");
            __asm__ __volatile__("lqc2 $vf1, 0(%0)\n"
                                 "lqc2 $vf2, 0(%1)\n"
                                 "vmul.xyz $vf1xyz, $vf1xyz, $vf2xyz\n"
                                 "sqc2 $vf1, 0(%0)\n"
                                 :
                                 : "r"(dest), "r"(allocator + 192)
                                 : "memory");
        } else {
            __asm__ __volatile__("lqc2 $vf1, 0(%0)\n"
                                 "lqc2 $vf2, 0(%1)\n"
                                 "vmul.xyz $vf1xyz, $vf1xyz, $vf2xyz\n"
                                 "sqc2 $vf1, 0(%2)\n"
                                 :
                                 : "r"(((unsigned char *) (index * 32 + (int) pointTable + 16))),
                                   "r"(allocator + 192), "r"(dest)
                                 : "memory");
            __asm__ __volatile__("lqc2 $vf1, 0(%0)\n"
                                 "lqc2 $vf2, 0(%1)\n"
                                 "vadd.xyz $vf1xyz, $vf1xyz, $vf2xyz\n"
                                 "sqc2 $vf1, 0(%0)\n"
                                 :
                                 : "r"(dest), "r"(allocator + 512)
                                 : "memory");
        }
        MMathApplyMatrix(dest, (Matrix4 *) allocator, dest);

        if (applyScale != 0 && scale != 1.0f) {
            __asm__ __volatile__("lqc2 $vf1, 0(%0)\n"
                                 "mfc1 $8, %1\n"
                                 "qmtc2 $8, $vf2\n"
                                 "vmulx.xyz $vf1xyz, $vf1xyz, $vf2x\n"
                                 "sqc2 $vf1, 0(%0)\n"
                                 :
                                 : "r"(dest), "f"(scale)
                                 : "memory", "$8");
            dest->x = dest->x + *(float *) (allocator + 160);
            dest->y = dest->y + (*(float *) (allocator + 292) +
                                 (*(float *) (allocator + 164) - *(float *) (allocator + 292)) * scale);
            dest->z = dest->z + *(float *) (allocator + 168);
        } else {
            if (((int) ((((*(unsigned long long (*)[]) & gCameraSelectFlags)[0]) >> 2) & 3)) != 0) {
                lineData = sefGetLineAdr(*(short *) (allocator + 322));
                lineFlags = *(int *) (allocator + 504);
                if ((lineFlags & 0x8000) != 0) {
                    if (*(int *) ((char *) lineData + 1636) != 0) {
                        lineValue = *(short *) (*(int *) ((char *) lineData + 1636) - 4);
                    } else {
                        lineValue = 0;
                    }
                    if (*(int *) ((char *) lineData + 352) != 0) {
                        lineKind = *(short *) (*(int *) ((char *) lineData + 352) - 4);
                    } else {
                        lineKind = 0;
                    }
                    sefGetSourceMatrix(&stackMatrix, (const void *) lineValue);
                    func_002046F8(
                        (Vector4 *) ((char *) &stackMatrix + 48), 0, lineKind);
                    MMathApplyMatrix(&stackVector, (Matrix4 *) &stackMatrix, (Vector4 *) (allocator + 160));
                    __asm__ __volatile__("lqc2 $vf1, 0(%0)\n"
                                         "lqc2 $vf2, 0(%1)\n"
                                         "vadd.xyz $vf1xyz, $vf1xyz, $vf2xyz\n"
                                         "sqc2 $vf1, 0(%0)\n"
                                         :
                                         : "r"(dest), "r"(&stackVector)
                                         : "memory");
                    dest->w = 1.0f;
                    return;
                }
            }
            __asm__ __volatile__("lqc2 $vf1, 0(%0)\n"
                                 "lqc2 $vf2, 0(%1)\n"
                                 "vadd.xyz $vf1xyz, $vf1xyz, $vf2xyz\n"
                                 "sqc2 $vf1, 0(%0)\n"
                                 :
                                 : "r"(dest), "r"(allocator + 160)
                                 : "memory");
        }
        dest->w = 1.0f;
    } else {
        __asm__ __volatile__("lqc2 $vf1, 0(%0)\n"
                             "lqc2 $vf2, 0(%1)\n"
                             "vadd.xyz $vf1xyz, $vf1xyz, $vf2xyz\n"
                             "sqc2 $vf1, 0(%2)\n"
                             :
                             : "r"(allocator + 160), "r"(allocator + 512), "r"(dest)
                             : "memory");
    }
}

/* One search table entry: its key, then 28 bytes not modeled. */
typedef struct SefSearchEntry {
    s32 key;
    unsigned char unmodeled_04[28];
} SefSearchEntry;

typedef struct SefSearchTable {
    s32 count;
    unsigned char unmodeled_04[12];
    SefSearchEntry entries[1];
} SefSearchTable;

/* Index of the first entry of the owner's search table whose key equals key, or -1 when there is none. */
s32 sefFindSearchKey(SefSearchOwner *owner, s32 key) {
    SefSearchTable *table;
    SefSearchEntry *entries;
    u32 index;

    table = (SefSearchTable *) ((unsigned char *) owner + owner->tableOffset);
    entries = table->entries;
    for (index = 0; index < table->count; index++) {
        if (key == entries[index].key) {
            return index;
        }
    }
    return -1;
}

/* Random point on the circle of the given radius in the XZ plane, y zero: the seed steps once, the angle is the
 * sampled fraction of a turn. */
void sefGetCirclePos(Vector4 *pos, f32 radius) {
    f32 angle;

    sefRandSeed = sefRandSeed * SEF_RAND_MULTIPLIER + SEF_RAND_INCREMENT;
    angle = (int) ((sefRandSeed >> 16) & SEF_RAND_MASK) / SEF_NOISE_SCALE * 6.28318531f;
    Vu0SetOrigin(pos);
    pos->x = xglCos(angle) * radius;
    pos->z = xglSin(angle) * radius;
}

/* outVector = inVector with xyz multiplied by scale; w is kept. */
static inline void Scale(Vector4 *outVector, const Vector4 *inVector, float scale) {
    __asm__ __volatile__("lqc2 $vf1,0(%1)\n"
                         "mfc1 $8,%2\n"
                         "qmtc2 $8,$vf2\n"
                         "vmulx.xyz $vf1,$vf1,$vf2x\n"
                         "sqc2 $vf1,0(%0)\n"
                         :
                         : "r"(outVector), "r"(inVector), "f"(scale)
                         : "$8", "memory");
}

/* Same as Scale with the scale operand given three times. */
static inline void ScaleXyz(Vector4 *outVector, const Vector4 *inVector, float scale) {
    __asm__ __volatile__("lqc2 $vf1,0(%1)\n"
                         "mfc1 $8,%2\n"
                         "qmtc2 $8,$vf2\n"
                         "vmulx.xyz $vf1,$vf1,$vf2x\n"
                         "sqc2 $vf1,0(%0)\n"
                         :
                         : "r"(outVector), "r"(inVector), "f"(scale), "f"(scale), "f"(scale)
                         : "$8", "memory");
}

/* Random unit direction in xyz scaled by scale; w is set to 1.0 before the scale. */
void sefRandomDirection(Vector4 *vector, float scale) {
    vector->x = MMathMakeRandom() - 0.5f;
    vector->y = MMathMakeRandom() - 0.5f;
    vector->z = MMathMakeRandom() - 0.5f;
    vector->w = 1.0f;
    Scale(vector, vector, 32768.0f);
    MMathNormalizeVector2(vector, vector);
    ScaleXyz(vector, vector, scale);
}

/* Unit direction from the three floats x, y and z (w is left as it was), normalized and then scaled by scale in x, y
 * and z. */
void sefSetUnitDirectionScaled(Vector4 *vector, float x, float y, float z, float scale)
{
    vector->x = x;
    vector->y = y;
    vector->z = z;
    MMathNormalizeVector2(vector, vector);
    Vu0SefScaleVectorXYZ(vector, vector, scale);
}

/* Random point of the cube [-0.5, 0.5) x [0, 1) x [-0.5, 0.5) scaled by extent: the bottom-aligned cube. */
void sefGetCubePosBtm(Vector4 *pos, const Vector4 *extent) {
    f32 shiftedY;

    sefNoiseNextVector(pos);
    shiftedY = pos->y + 0.5f;
    pos->y = shiftedY;
    Vu0SefMulVectorXYZ(pos, pos, extent);
}

/* The top-aligned cube: sefGetCubePosBtm with y negated. */
void sefGetCubePosTop(Vector4 *pos, const Vector4 *extent)
{
    sefGetCubePosBtm(pos, extent);
    pos->y = -pos->y;
}

/* Random point of the centred cube [-0.5, 0.5)^3 scaled by the extent vector. */
void sefGetCubePosCenter(Vector4 *destinationPoint, Vector4 *scale)
{
    sefNoiseNextVector(destinationPoint);
    Vu0SefMulVectorXYZ(destinationPoint, destinationPoint, scale);
}

extern const DoublewordAlignedVector4 sefDefaultPoint_205638;

typedef struct SefViewMatrix {
    Vector4 rows[4];
} SefViewMatrix;

typedef struct SefStudioCamera {
    unsigned char unmodeled_000[0x170];
    SefViewMatrix viewMatrix;
} SefStudioCamera;

typedef struct SefMatrixRow {
    unsigned long rowWords[2];
} SefMatrixRow;

typedef union SefPartMatrix {
    XenoMatrix matrix;
    SefMatrixRow rows[4];
} SefPartMatrix;

/* Partial view of the battle data read by sefGetMatrixPoint: the gate words, the four-row blocks (kind families) rowsA
 * to rowsG and the matrix tables. */
typedef struct SefPartMatrices {
    int gate256;
    int gate579;
    int unmodeled_08;
    int gate800;
    unsigned char unmodeled_10[0x70 - 0x10];
    SefMatrixRow rowsA[4];
    SefMatrixRow rowsB[4];
    SefMatrixRow rowsC[4];
    SefMatrixRow rowsD[4];
    unsigned char unmodeled_170[0x1F0 - 0x170];
    SefMatrixRow rowsG[4];
    unsigned char unmodeled_230[0x2C8 - 0x230];
    XenoMatrix *matrixTables[4];
} SefPartMatrices;

typedef struct SefPartRecord {
    unsigned char unmodeled_00[0x30];
    Vector4 v30;
} SefPartRecord;

#define SEF_CURRENT_PART_RECORD ((SefPartRecord *)sefCurrentEffectRecord)
#define SEF_GET_PART_MATRICES ((SefPartMatrices *(*)(void))sefGetBattleData)
#define SEF_APPLY_MATRIX_VECTOR ((void(*)(Vector4*,SefViewMatrix*,Vector4*))sefApplyMatrixVector)
#define XGL_STUDIO_GET_ACTIVE_CAMERA ((SefStudioCamera *(*)(void))xglStudioGetActiveCamera)

/* outVector = left * right in xyz, through vf1 and vf2. */
static inline void sefMulVectorXyz(Vector4 *outVector, Vector4 *left, const Vector4 *right) {
    __asm__ __volatile__("lqc2 vf1, 0(%5)\n lqc2 vf2, 0(%6)\n vmul.xyz vf1, vf1, vf2\n sqc2 vf1, 0(%4)"
                         : "=m"(outVector->x), "=m"(outVector->y), "=m"(outVector->z), "=m"(outVector->w)
                         : "r"(outVector), "r"(left), "r"(right));
}

/* Matrix entry index of table: the table pointer plus index matrices. */
static inline XenoMatrix *sefMatrixAt(XenoMatrix *table, int index) {
    return table + index;
}

/* Point of kind transformed by the battle matrices (camera, studio camera, kind tables, the effect record
 * when present). Does nothing when dest or src is 0. */
void sefGetMatrixPoint(Vector4 *dest, Vector4 *src, int kind) {

    SefPartMatrices *battleMatrices = SEF_GET_PART_MATRICES();
    int vectorMode = sefVectorMode;
    SefPartMatrix recordMatrix;
    SefPartMatrix workMatrix;

    if (dest == 0 || src == 0) {
        return;
    }
    if (kind == 1 || kind == 4 || kind == 5) {
        SEF_APPLY_MATRIX_VECTOR(dest, (SefViewMatrix *) &sefCameraMatrixBlock, src);
        return;
    }
    if (kind == 2) {
        SEF_APPLY_MATRIX_VECTOR(dest, &XGL_STUDIO_GET_ACTIVE_CAMERA()->viewMatrix, src);
        return;
    }
    if (kind < 9) {
        *dest = *src;
        return;
    }
    if (kind == 256) {
        SefPartRecord *effectRecord;
        if (battleMatrices->gate256 == 0 || vectorMode != 0) {
            return;
        }
        effectRecord = SEF_CURRENT_PART_RECORD;
        if (effectRecord != 0) {
            recordMatrix.rows[0] = battleMatrices->rowsA[0];
            recordMatrix.rows[1] = battleMatrices->rowsA[1];
            *(Vector4 *) &recordMatrix.rows[3] = sefDefaultPoint_205638;
            recordMatrix.rows[2] = battleMatrices->rowsA[2];
            sefMulVectorXyz(dest, &effectRecord->v30, src);
            ((Vector4 * (*) (Vector4 *, SefPartMatrix *, Vector4 *) ) MMathApplyMatrix)(dest, &recordMatrix, src);
        } else {
            workMatrix.rows[0] = battleMatrices->rowsA[0];
            *(Vector4 *) &workMatrix.rows[3] = sefDefaultPoint_205638;
            workMatrix.rows[1] = battleMatrices->rowsA[1];
            workMatrix.rows[2] = battleMatrices->rowsA[2];
            ((Vector4 * (*) (Vector4 *, SefPartMatrix *, Vector4 *) ) MMathApplyMatrix)(dest, &workMatrix, src);
        }
        return;
    }
    if (kind & 0x100) {
        {
            int slotIndex = kind - 257;
            if (battleMatrices->matrixTables[0] == 0) {
                return;
            }
            *(XenoMatrix *) &workMatrix = *sefMatrixAt(battleMatrices->matrixTables[0], slotIndex);
            *(Vector4 *) &workMatrix.rows[3] = sefDefaultPoint_205638;
            ((Vector4 * (*) (Vector4 *, SefPartMatrix *, Vector4 *) ) MMathApplyMatrix)(dest, &workMatrix, src);
        }
        return;
    }
    if (kind & 0x200) {
        if (kind == 577 || kind == 578) {
            if (battleMatrices->matrixTables[1] == 0) {
                return;
            }
            *(XenoMatrix *) &workMatrix = battleMatrices->matrixTables[1][31];
            *(Vector4 *) &workMatrix.rows[3] = sefDefaultPoint_205638;
            ((Vector4 * (*) (Vector4 *, SefPartMatrix *, Vector4 *) ) MMathApplyMatrix)(dest, &workMatrix, src);
            workMatrix.rows[0] = battleMatrices->rowsG[0];
            *(Vector4 *) &workMatrix.rows[3] = sefDefaultPoint_205638;
            workMatrix.rows[1] = battleMatrices->rowsG[1];
            workMatrix.rows[2] = battleMatrices->rowsG[2];
            ((Vector4 * (*) (Vector4 *, SefPartMatrix *, Vector4 *) ) MMathApplyMatrix)(dest, &workMatrix, dest);
        } else if (kind == 579) {
            if (battleMatrices->gate579 == 0) {
                return;
            }
            workMatrix.rows[0] = battleMatrices->rowsB[0];
            *(Vector4 *) &workMatrix.rows[3] = sefDefaultPoint_205638;
            workMatrix.rows[1] = battleMatrices->rowsB[1];
            workMatrix.rows[2] = battleMatrices->rowsB[2];
            ((Vector4 * (*) (Vector4 *, SefPartMatrix *, Vector4 *) ) MMathApplyMatrix)(dest, &workMatrix, src);
        } else {
            if (battleMatrices->matrixTables[1] == 0) {
                return;
            }
            *(XenoMatrix *) &workMatrix = battleMatrices->matrixTables[1][31];
            *(Vector4 *) &workMatrix.rows[3] = sefDefaultPoint_205638;
            ((Vector4 * (*) (Vector4 *, SefPartMatrix *, Vector4 *) ) MMathApplyMatrix)(dest, &workMatrix, src);
            if ((*(short (*)[]) & D_004749D4)[0] != 0) {
                workMatrix.rows[0] = battleMatrices->rowsG[0];
                *(Vector4 *) &workMatrix.rows[3] = sefDefaultPoint_205638;
                workMatrix.rows[1] = battleMatrices->rowsG[1];
                workMatrix.rows[2] = battleMatrices->rowsG[2];
                ((Vector4 * (*) (Vector4 *, SefPartMatrix *, Vector4 *) ) MMathApplyMatrix)(dest, &workMatrix, dest);
            } else {
                workMatrix.rows[0] = battleMatrices->rowsB[0];
                *(Vector4 *) &workMatrix.rows[3] = sefDefaultPoint_205638;
                workMatrix.rows[1] = battleMatrices->rowsB[1];
                workMatrix.rows[2] = battleMatrices->rowsB[2];
                ((Vector4 * (*) (Vector4 *, SefPartMatrix *, Vector4 *) ) MMathApplyMatrix)(dest, &workMatrix, dest);
            }
        }
        return;
    }
    if (kind & 0x400) {
        {
            int slotIndex = kind - 1025;
            if (battleMatrices->matrixTables[2] == 0) {
                return;
            }
            *(XenoMatrix *) &workMatrix = *sefMatrixAt(battleMatrices->matrixTables[2], slotIndex);
            *(Vector4 *) &workMatrix.rows[3] = sefDefaultPoint_205638;
            ((Vector4 * (*) (Vector4 *, SefPartMatrix *, Vector4 *) ) MMathApplyMatrix)(dest, &workMatrix, dest);
            if (vectorMode == 0) {
                workMatrix.rows[0] = battleMatrices->rowsC[0];
                *(Vector4 *) &workMatrix.rows[3] = sefDefaultPoint_205638;
                workMatrix.rows[1] = battleMatrices->rowsC[1];
                workMatrix.rows[2] = battleMatrices->rowsC[2];
                ((Vector4 * (*) (Vector4 *, SefPartMatrix *, Vector4 *) ) MMathApplyMatrix)(dest, &workMatrix, dest);
            }
        }
        return;
    }
    if (kind & 0x800) {
        {
            int slotIndex = kind - 2049;
            if (battleMatrices->gate800 == 0) {
                return;
            }
            *(XenoMatrix *) &workMatrix = *sefMatrixAt(battleMatrices->matrixTables[3], slotIndex);
            *(Vector4 *) &workMatrix.rows[3] = sefDefaultPoint_205638;
            ((Vector4 * (*) (Vector4 *, SefPartMatrix *, Vector4 *) ) MMathApplyMatrix)(dest, &workMatrix, dest);
            if (vectorMode == 0) {
                workMatrix.rows[0] = battleMatrices->rowsD[0];
                *(Vector4 *) &workMatrix.rows[3] = sefDefaultPoint_205638;
                workMatrix.rows[1] = battleMatrices->rowsD[1];
                workMatrix.rows[2] = battleMatrices->rowsD[2];
                ((Vector4 * (*) (Vector4 *, SefPartMatrix *, Vector4 *) ) MMathApplyMatrix)(dest, &workMatrix, dest);
            }
        }
    }
}

/* Kind matrices of the battle request block (SefLinkedContext, include/main/tu132.h). The enable words at +0x00 are a
 * second view of that block's head, kept apart: the field types decide the emitted order. */
typedef struct SefKindMatrixBlock {
    s32 kindEnabled[5];                 /* +0x00: kinds 0x100, 0x200, 0x400, 0x800, 0x1000 */
    u8 unmodeled_014[0x70 - 0x14];
    XenoMatrix kindMatrices[5];         /* +0x70: entry n is kind 0x100 << n */
    u8 unmodeled_1B0[0x40];
    XenoMatrix kindAltMatrix;           /* +0x1F0: kind 0x200 uses it when D_004749D4 is set */
} SefKindMatrixBlock;

/* Matrix indices of the current effect record (SefEffectRecord): negative when the record has none. */
typedef struct SefEffectMatrixIndices {
    u8 unmodeled_000[0x144];
    s8 kind3BMatrixIndex;               /* +0x144: kinds 0x3B and 0x3E */
    s8 kind3DMatrixIndex;               /* +0x145: kinds 0x3D, 0x3F, 0x40 and 0x41 */
} SefEffectMatrixIndices;

#define SEF_GET_KIND_MATRIX_BLOCK ((SefKindMatrixBlock *(*)(void))sefGetBattleData)
#define SEF_KIND_MATRIX_AT(index) ((XenoMatrix *(*)(int))sefGetActiveTableSlot)(index)
#define SEF_APPLY_VECTOR_MATRIX ((void(*)(AlignedVector4*,XenoMatrix*,AlignedVector4*))sefApplyMatrixVector)
#define SEF_CURRENT_EFFECT_MATRIX_INDICES ((SefEffectMatrixIndices *)sefCurrentEffectRecord)


/* Transforms v into d by the matrix of kind k: the current record's matrix first, then the battle request's kind matrices
 * and kind 0x11; with no matrix the vector is copied. */
void func_00205DF0(AlignedVector4 *dest, AlignedVector4 *vector, s32 k) {
    SefKindMatrixBlock *block = SEF_GET_KIND_MATRIX_BLOCK();

    if ((k == 0x3B || k == 0x3E) && SEF_CURRENT_EFFECT_MATRIX_INDICES->kind3BMatrixIndex >= 0) {
        SEF_APPLY_VECTOR_MATRIX(dest, SEF_KIND_MATRIX_AT(SEF_CURRENT_EFFECT_MATRIX_INDICES->kind3BMatrixIndex), vector);
        return;
    }
    if ((k == 0x3D || k == 0x3F || k == 0x40 || k == 0x41) && SEF_CURRENT_EFFECT_MATRIX_INDICES->kind3DMatrixIndex >= 0) {
        SEF_APPLY_VECTOR_MATRIX(dest, SEF_KIND_MATRIX_AT(SEF_CURRENT_EFFECT_MATRIX_INDICES->kind3DMatrixIndex), vector);
        return;
    }
    if ((k & 0x200) && block->kindEnabled[1]) {
        if ((*(s16(*)[]) & D_004749D4)[0]) {
            SEF_APPLY_VECTOR_MATRIX(dest, &block->kindAltMatrix, vector);
        } else {
            SEF_APPLY_VECTOR_MATRIX(dest, &block->kindMatrices[1], vector);
        }
        return;
    }
    if ((k & 0x100) && block->kindEnabled[0]) {
        SEF_APPLY_VECTOR_MATRIX(dest, &block->kindMatrices[0], vector);
        return;
    }
    if ((k & 0x400) && block->kindEnabled[2]) {
        SEF_APPLY_VECTOR_MATRIX(dest, &block->kindMatrices[2], vector);
        return;
    }
    if ((k & 0x800) && block->kindEnabled[3]) {
        SEF_APPLY_VECTOR_MATRIX(dest, &block->kindMatrices[3], vector);
        return;
    }
    if ((k & 0x1000) && block->kindEnabled[4]) {
        SEF_APPLY_VECTOR_MATRIX(dest, &block->kindMatrices[4], vector);
        return;
    }
    if (k == 0x11) {
        if ((XenoMatrix *) D_005510A4) {
            SEF_APPLY_VECTOR_MATRIX(dest, (XenoMatrix *) D_005510A4, vector);
            return;
        }
    }
    __asm__ __volatile__("lq $8, 0(%5)\n\tsq $8, 0(%4)"
                         : "=m"(dest->x), "=m"(dest->y), "=m"(dest->z), "=m"(dest->w)
                         : "r"(dest), "r"(vector)
                         : "$8");
}

void sefGetCirclePos(Vector4 *pos, f32 radius);

typedef struct VuMatrix4 {
    f32 cells[4][4];
} __attribute__((aligned(16))) VuMatrix4;

extern f32 func_0025A820(const AlignedVector4*);
extern void vu0RotateCurrentX(f32);
extern void vu0RotateCurrentZ(f32);
extern VuMatrix4 vu0IdentityMatrix;

#define SEF_DEG_TO_RAD 0.0174532925f
#define SEF_NORMALIZE_VECTOR2 ((AlignedVector4 *(*)(AlignedVector4*,const AlignedVector4*))MMathNormalizeVector2)

static inline void VuZeroVector(AlignedVector4 *dest) {
    __asm__ __volatile__("sqc2 vf0, 0(%4)" : "=m"(dest->x), "=m"(dest->y), "=m"(dest->z), "=m"(dest->w) : "r"(dest));
}

static inline void VuScaleVector(AlignedVector4 *dest, AlignedVector4 *source, f32 scalar) {
    __asm__ __volatile__(
        "lqc2 $vf1, 0(%5)\n mfc1 $8, %6\n qmtc2.ni $8, $vf2\n vmulx.xyz $vf1, $vf1, $vf2x\n sqc2 $vf1, 0(%4)"
        : "=m"(dest->x), "=m"(dest->y), "=m"(dest->z), "=m"(dest->w)
        : "r"(dest), "r"(source), "f"(scalar)
        : "$8");
}

static inline void VuAddVector(AlignedVector4 *dest, AlignedVector4 *lhs, AlignedVector4 *rhs) {
    __asm__ __volatile__("lqc2 $vf1, 0(%5)\n lqc2 $vf2, 0(%6)\n vadd.xyz $vf1, $vf1, $vf2\n sqc2 $vf1, 0(%4)"
                         : "=m"(dest->x), "=m"(dest->y), "=m"(dest->z), "=m"(dest->w)
                         : "r"(dest), "r"(lhs), "r"(rhs));
}

static inline void VuSubVector(AlignedVector4 *dest, AlignedVector4 *lhs, AlignedVector4 *rhs) {
    __asm__ __volatile__("lqc2 $vf1, 0(%5)\n lqc2 $vf2, 0(%6)\n vsub.xyz $vf1, $vf1, $vf2\n sqc2 $vf1, 0(%4)"
                         : "=m"(dest->x), "=m"(dest->y), "=m"(dest->z), "=m"(dest->w)
                         : "r"(dest), "r"(lhs), "r"(rhs));
}

static inline void VuLoadCurrentMatrix(VuMatrix4 *matrix) {
    __asm__ __volatile__(
        "lqc2 $vf26, 0(%0)\n lqc2 $vf27, 16(%0)\n lqc2 $vf28, 32(%0)\n vmove.xyzw $vf29, $vf0"
        :
        : "r"(matrix));
}

#define VuApplyCurrentMatrix(dest,src) __asm__ __volatile__("lqc2 $vf1, 0(%5)\n vmulax.xyzw ACC, $vf26, $vf1x\n vmadday.xyzw ACC, $vf27, $vf1y\n vmaddaz.xyzw ACC, $vf28, $vf1z\n vmaddw.xyzw $vf20, $vf29, $vf1w\n sqc2 $vf20, 0(%4)" : "=m"((dest)->x),"=m"((dest)->y),"=m"((dest)->z),"=m"((dest)->w) : "r"(dest),"r"(src))

/* Loads the identity matrix as the VU0 current matrix and rotates it by the x and z angles (degrees) in angles. */
#define SEF_ROTATE_CURRENT_MATRIX(angles)                \
    VuLoadCurrentMatrix(&vu0IdentityMatrix);             \
    if ((angles)->y != 0.0f) {                           \
        vu0RotateCurrentX((angles)->y * SEF_DEG_TO_RAD); \
    }                                                    \
    if ((angles)->z != 0.0f) {                           \
        vu0RotateCurrentZ((angles)->z * SEF_DEG_TO_RAD); \
    }

/* Writes the emission offset d of particle kind k from the parameters o, the source direction v and random draws. */
void func_00206058(AlignedVector4 *dest, AlignedVector4 *params, AlignedVector4 *source, s32 k) {
    VuZeroVector(dest);
    if (k == 0) {
        if (params->x != 0.0f) {
            sefRandomDirection(dest, sefRandf() * params->x);
        }
    } else if (k == 1) {
        if (params->x != 0.0f) {
            sefSetUnitDirectionScaled(dest, sefNoiseNext(), sefRandf(), sefNoiseNext(), sefRandf() * params->x);
        }
    } else if (k == 2) {
        if (params->x != 0.0f) {
            sefSetUnitDirectionScaled(dest, sefNoiseNext(), -sefRandf(), sefNoiseNext(), sefRandf() * params->x);
        }
    } else if (k == 3) {
        if (params->x != 0.0f) {
            sefSetUnitDirectionScaled(dest, sefNoiseNext(), sefNoiseNext(), sefRandf(), sefRandf() * params->x);
        }
    } else if (k == 4) {
        if (params->x != 0.0f) {
            sefSetUnitDirectionScaled(dest, sefNoiseNext(), sefNoiseNext(), -sefRandf(), sefRandf() * params->x);
        }
    } else if (k == 5) {
        if (params->x != 0.0f) {
            sefSetUnitDirectionScaled(dest, -sefRandf(), sefNoiseNext(), sefNoiseNext(), sefRandf() * params->x);
        }
    } else if (k == 6) {
        if (params->x != 0.0f) {
            sefSetUnitDirectionScaled(dest, sefRandf(), sefNoiseNext(), sefNoiseNext(), sefRandf() * params->x);
        }
    } else if (k == 7) {
        dest->x = sefRandf();
        dest->y = sefRandf();
        dest->z = sefRandf();
        SEF_NORMALIZE_VECTOR2(dest, dest);
        dest->x *= sefRandf() * params->x;
        dest->y *= sefRandf() * params->y;
        dest->z *= sefRandf() * params->z;
    } else if (k == 0x17) {
        AlignedVector4 unitDir;

        SEF_NORMALIZE_VECTOR2(&unitDir, source);
        VuScaleVector(dest, &unitDir, sefRandf() * params->x);
    } else if (k == 0x1A) {
        AlignedVector4 unitDir;

        SEF_NORMALIZE_VECTOR2(&unitDir, source);
        VuScaleVector(dest, &unitDir, sefRandf() * params->x);
        if (func_0025A820(source) > func_0025A820(dest)) {
            VuAddVector(dest, source, dest);
            if (func_0025A820(dest) > params->x * params->x) {
                VuScaleVector(dest, &unitDir, params->x);
            }
        }
    } else if (k == 0x1B) {
        AlignedVector4 unitDir;

        SEF_NORMALIZE_VECTOR2(&unitDir, source);
        VuScaleVector(dest, &unitDir, sefRandf() * params->x);
        if (func_0025A820(source) < func_0025A820(dest)) {
            VuSubVector(dest, dest, source);
            if (func_0025A820(dest) > func_0025A820(source)) {
                VuZeroVector(dest);
            }
        }
    } else if (k == 8) {
        if (params->x != 0.0f) {
            sefRandomDirection(dest, params->x);
        }
    } else if (k == 9) {
        if (params->x != 0.0f) {
            sefSetUnitDirectionScaled(dest, sefNoiseNext(), sefRandf(), sefNoiseNext(), params->x);
        }
    } else if (k == 10) {
        if (params->x != 0.0f) {
            sefSetUnitDirectionScaled(dest, sefNoiseNext(), -sefRandf(), sefNoiseNext(), params->x);
        }
    } else if (k == 11) {
        if (params->x != 0.0f) {
            sefSetUnitDirectionScaled(dest, sefNoiseNext(), sefNoiseNext(), sefRandf(), params->x);
        }
    } else if (k == 12) {
        if (params->x != 0.0f) {
            sefSetUnitDirectionScaled(dest, sefNoiseNext(), sefNoiseNext(), -sefRandf(), params->x);
        }
    } else if (k == 13) {
        if (params->x != 0.0f) {
            sefSetUnitDirectionScaled(dest, -sefRandf(), sefNoiseNext(), sefNoiseNext(), params->x);
        }
    } else if (k == 14) {
        if (params->x != 0.0f) {
            sefSetUnitDirectionScaled(dest, sefRandf(), sefNoiseNext(), sefNoiseNext(), params->x);
        }
    } else if (k == 15) {
        dest->x = sefRandf();
        dest->y = sefRandf();
        dest->z = sefRandf();
        SEF_NORMALIZE_VECTOR2(dest, dest);
        dest->x *= params->x;
        dest->y *= params->y;
        dest->z *= params->z;
    } else if (k == 0x18) {
        AlignedVector4 unitDir;

        SEF_NORMALIZE_VECTOR2(&unitDir, source);
        VuScaleVector(dest, &unitDir, params->x);
    } else if (k == 0x10) {
        sefGetCubePosCenter(dest, params);
    } else if (k == 0x11) {
        sefGetCubePosTop(dest, params);
    } else if (k == 0x12) {
        sefGetCubePosBtm(dest, params);
    } else if (k == 0x13) {
        AlignedVector4 circlePoint;

        sefGetCirclePos(&circlePoint, params->x);
        SEF_ROTATE_CURRENT_MATRIX(params);
        VuApplyCurrentMatrix(dest, &circlePoint);
    } else if (k == 0x19) {
        AlignedVector4 unitDir;

        SEF_NORMALIZE_VECTOR2(&unitDir, source);
        VuScaleVector(&unitDir, &unitDir, params->x);
        SEF_ROTATE_CURRENT_MATRIX(params);
        VuApplyCurrentMatrix(dest, &unitDir);
    } else if (k == 0x14) {
        AlignedVector4 cubeCorner;

        cubeCorner.z = cubeCorner.y = cubeCorner.x = params->x * 2.0f;
        cubeCorner.w = 1.0f;
        do {
            sefGetCubePosCenter(dest, &cubeCorner);
        } while (!(func_0025A820(dest) <= params->x * params->x));
        dest->y = sefRandf() * params->y;
    } else if (k == 0x15) {
        AlignedVector4 cubeCorner;

        cubeCorner.z = cubeCorner.y = cubeCorner.x = params->x * 2.0f;
        cubeCorner.w = 1.0f;
        do {
            sefGetCubePosCenter(dest, &cubeCorner);
        } while (!(func_0025A820(dest) <= params->x * params->x));
        dest->y = sefNoiseNext() * params->y;
    }
}

/* Returns the payload of the first run entry whose threshold reaches 4096. */
short func_00206A50(SefRunEntry *entries) {
    s32 index;

    index = 0;
    while (entries[index].threshold < 4096) {
        index++;
    }
    return entries[index].payload;
}

Vector4 *MMathVectorCrossProduct(Vector4 *outVector, const Vector4 *lhs, const Vector4 *rhs);

/* Copies a 4x4 matrix row by row through the EE quadword registers. */
#define SEF_COPY_MAT4_ROWS(destination, source) \
    __asm__ __volatile__( \
        "lq $8,0+%1\n\tlq $9,0x10+%1\n\tlq $10,0x20+%1\n\tlq $11,0x30+%1\n\t" \
        "sq $8,0+%0\n\tsq $9,0x10+%0\n\tsq $10,0x20+%0\n\tsq $11,0x30+%0" \
        : "=m"(*(Mat4Rows *)(destination)) : "m"(*(const Mat4Rows *)(source)) : "$8", "$9", "$10", "$11")

/* Builds the frame dest from the points of kindA and kindB: the first axis is the normalized axis that
 * the kindB lookup returns (when there is one), the second the offset direction, the third their cross product. */
void func_00206AA8(Matrix4 *dest, Vector4 *pointAOut, Vector4 *pointBOut, int kindA, int kindB) {
    Vector4 pointA;
    Vector4 pointB;
    Vector4 pointOffset;
    Vector4 unitOffset;
    Mat4Rows frame;
    Mat4Rows *frameRows; /* kept: the alias decides the emitted order */
    Vector4 *pointBAxis;

    func_002046F8(&pointA, 0, kindA);
    pointBAxis = (Vector4 *) func_002046F8(&pointB, 0, kindB);
    Vu0SefSubVectorXYZ(&pointOffset, &pointB, &pointA);
    MMathNormalizeVector2(&unitOffset, &pointOffset);
    if (pointBAxis != 0) {
        frameRows = &frame;
        MMathNormalizeVector2(&frame.r[0], pointBAxis);
        QuadwordCopyT0(&frame.r[1], &unitOffset);
        MMathVectorCrossProduct(&frame.r[2], &frame.r[0], &frame.r[1]);
        Vu0SetOrigin(&frame.r[3]);
        frame.r[0].w = frame.r[1].w = frame.r[2].w = 0.0f;
    } else {
        frameRows = &frame;
        CopyUnitMatrixVu0(frameRows, &MMathUnitMatrix);
    }
    if (pointAOut != 0) {
        QuadwordCopyT0(pointAOut, &pointA);
    }
    if (pointBOut != 0) {
        QuadwordCopyT0(pointBOut, &pointB);
    }
    if (dest != 0) {
        SEF_COPY_MAT4_ROWS(dest, frameRows);
    }
}

/* Copies source to destination, then writes the midpoint of reference and point into its translation row. */
void func_00206C10(Matrix4 *destination, const Matrix4 *source, const Vector4 *reference,
                   const Vector4 *point) {
    Vector4 halfOffset;

    __asm__ __volatile__("lq $8, 0(%1)\n"
                         "lq $9, 16(%1)\n"
                         "lq $10, 32(%1)\n"
                         "lq $11, 48(%1)\n"
                         "sq $8, 0(%0)\n"
                         "sq $9, 16(%0)\n"
                         "sq $10, 32(%0)\n"
                         "sq $11, 48(%0)\n"
                         "lqc2 $vf1, 0(%3)\n"
                         "lqc2 $vf2, 0(%2)\n"
                         "vsub.xyz $vf1, $vf1, $vf2\n"
                         "sqc2 $vf1, 0(%4)\n"
                         ".set push\n"
                         ".set noat\n"
                         "lui $1, 0x3f00\n"
                         "mtc1 $1, $f0\n"
                         "lqc2 $vf1, 0(%4)\n"
                         ".set pop\n"
                         "mfc1 $8, $f0\n"
                         "qmtc2 $8, $vf2\n"
                         "vmulx.xyz $vf1, $vf1, $vf2x\n"
                         "sqc2 $vf1, 0(%4)\n"
                         "addiu %0, %0, 48\n"
                         "lqc2 $vf1, 0(%4)\n"
                         "lqc2 $vf2, 0(%2)\n"
                         "vadd.xyz $vf1, $vf1, $vf2\n"
                         "sqc2 $vf1, 0(%0)\n"
                         :
                         : "r"(destination), "r"(source), "r"(reference), "r"(point), "r"(&halfOffset)
                         : "$1", "$8", "$9", "$10", "$11", "$f0", "memory");
}

float MMathCalcDist(const Vector4 *first, const Vector4 *second);

/* Writes the emission offset of kind into dest from source and the distance of the two point kinds: kinds below 16
 * scale all of source by half the distance, the others only y; kind 20 is emitted as kind 21. */
void func_00206C80(Vector4 *dest, const Vector4 *source, unsigned int kind, int pointKindA, int pointKindB) {
    Vector4 pointA;
    Vector4 pointB;
    Vector4 offset;
    float pointDistance;

    func_002046F8(&pointA, 0, pointKindA);
    func_002046F8(&pointB, 0, pointKindB);
    if (kind < 16) {
        pointDistance = MMathCalcDist(&pointA, &pointB) * 0.5f;
        offset.x = pointDistance * source->x;
        offset.y = pointDistance * source->y;
        offset.z = pointDistance * source->z;
    } else {
        offset.x = source->x;
        offset.y = MMathCalcDist(&pointA, &pointB) * source->y;
        offset.z = source->z;
        if (kind == 20) {
            kind = 21;
        }
    }
    func_00206058(dest, &offset, 0, kind);
}

/* A 4x4 frame held as four aligned rows. */
typedef struct LineFrame {
    AlignedVector4 rows[4];
} LineFrame;

/* Copies the vector s to d through one EE quadword register. */
static inline void LineCopyVector(AlignedVector4 *dest, AlignedVector4 *source) {
    __asm__ __volatile__("lq $8,0(%5)\n sq $8,0(%4)"
                         : "=m"(dest->x), "=m"(dest->y), "=m"(dest->z), "=m"(dest->w)
                         : "r"(dest), "r"(source)
                         : "$8");
}

/* Adds the vectors a and b into d on the VU0 (xyz only). */
static inline void LineAddVector(AlignedVector4 *dest, AlignedVector4 *lhs, AlignedVector4 *rhs) {
    __asm__ __volatile__("lqc2 vf1,0(%4)\n lqc2 vf2,0(%5)\n vadd.xyz vf1,vf1,vf2\n sqc2 vf1,0(%3)"
                         : "=m"(dest->x), "=m"(dest->y), "=m"(dest->z)
                         : "r"(dest), "r"(lhs), "r"(rhs));
}

/* Writes the point of the line record at time into out: the value at +0x110 (interpolated before time 1) offset
 * by the point at +0x10, moved by the kind matrix when the record carries one. */
void func_00206D78(AlignedVector4 *out, unsigned char *line, float time) {
    AlignedVector4 interpolated;
    LineFrame frame;
    AlignedVector4 pointA;
    AlignedVector4 pointB;
    int kind = *(short *) (*(unsigned char **) (line + 0x164) - 0x10);
    int index = func_00206A50(*(short **) (line + 0x160));

    if (time < 1.0f) {
        func_0025A000(&interpolated, &SEF_SCRATCHPAD(AlignedVector4, 0x20), (AlignedVector4 *) (line + 0x110), time);
    } else {
        LineCopyVector(&interpolated, (AlignedVector4 *) (line + 0x110));
    }
    if (index >= 0) {
        func_00206C80(out, &interpolated, kind, (*(short **) (line + 0x160))[-2], index);
        LineAddVector(out, (AlignedVector4 *) (line + 0x10), out);
        if (!(*(int *) (line + 0x694) & 1)) {
            func_00206AA8(&frame, &pointA, &pointB, (*(short **) (line + 0x160))[-2], index);
            func_00206C10(&frame, &frame, &pointA, &pointB);
            MMathApplyMatrix(out, (Matrix4 *) &frame, out);
        }
    } else {
        func_00206058(out, &interpolated, 0, kind);
        if (sefVectorMode == 0 && !(*(int *) (line + 0x694) & 0x20000)) {
            func_00205DF0(out, out, (*(short **) (line + 0x160))[-2]);
        }
        LineAddVector(out, (AlignedVector4 *) (line + 0x10), out);
    }
}

/* The same point for the second line record of the entry: the value at +0x130 and the point at +0x30. */
void func_00206F18(AlignedVector4 *out, unsigned char *line, float time) {
    AlignedVector4 interpolated;
    LineFrame frame;
    AlignedVector4 pointA;
    AlignedVector4 pointB;
    int kind = *(short *) (*(unsigned char **) (line + 0x16C) - 0x10);
    int index = func_00206A50(*(short **) (line + 0x168));

    if (time < 1.0f) {
        func_0025A000(&interpolated, &SEF_SCRATCHPAD(AlignedVector4, 0x30), (AlignedVector4 *) (line + 0x130), time);
    } else {
        LineCopyVector(&interpolated, (AlignedVector4 *) (line + 0x130));
    }
    if (index >= 0) {
        func_00206C80(out, &interpolated, kind, (*(short **) (line + 0x168))[-2], index);
        LineAddVector(out, (AlignedVector4 *) (line + 0x30), out);
        if (!(*(int *) (line + 0x694) & 1)) {
            func_00206AA8(&frame, &pointA, &pointB, (*(short **) (line + 0x168))[-2], index);
            func_00206C10(&frame, &frame, &pointA, &pointB);
            MMathApplyMatrix(out, (Matrix4 *) &frame, out);
        }
    } else {
        func_00206058(out, &interpolated, 0, kind);
        if (sefVectorMode == 0 && !(*(int *) (line + 0x694) & 0x20000)) {
            func_00205DF0(out, out, (*(short **) (line + 0x168))[-2]);
        }
        LineAddVector(out, (AlignedVector4 *) (line + 0x30), out);
    }
}

#define sefEffectEntryTable (*(SefEntryTable *)&sefEffectEntryTable)

/* Zeroes the origin, then clears the flags of every entry and the short at +0x1592 of each. */
void sefResetEntryFlagsAndOrigin(void)
{
    SefEntry *effectEntry;
    int i;

    Vu0SetOrigin(&sefOriginVector);
    for (i = 0; i < sefEffectEntryTable.effectEntryCount; i++) {
        effectEntry = &sefEffectEntryTable.effectEntries[i];
        effectEntry->effectEntryFlags = 0;
        effectEntry->unmodeled_1592 = 0;
    }
}

/* Places the part: the combined matrix of the placement and the global transform (its translation first offset by the
 * global position when the scale applies), applied to the placement's point when mode >= 0, otherwise the matrix's
 * position row copied to out. Returns the global scale when the scale applies, else 1. */
float sefPlacePart(SefPartPlacement *placement, Vector4 *resultPoint, int placeMode)
{
    Matrix4 combined;
    float scale = 1.0f;
    AlignedVector4 localTranslation;

    if (gSdvCameraPartTransform.scaled) {
        __asm__ __volatile__(
            "lqc2 vf1, 0(%1)\n"
            "lqc2 vf2, 0(%2)\n"
            "vsub.xyz vf1, vf1, vf2\n"
            "sqc2 vf1, 0(%3)\n"
            : "=m"(localTranslation)
            : "r"(&placement->translation), "r"(&gSdvCameraPartTransform.position), "r"(&localTranslation),
              "m"(placement->translation), "m"(gSdvCameraPartTransform.position));
        sefMergeMatrixPos(&combined, &placement->matrix, &localTranslation);
        MMathMulMatrix(&combined, &gSdvCameraPartTransform.matrix, &combined);
        scale = gSdvCameraPartTransform.scale;
    } else {
        sefMergeMatrixPos(&combined, &placement->matrix, &placement->translation);
    }
    if (placeMode >= 0) {
        Vector4 *point = &placement->point;

        MMathApplyMatrix(resultPoint, &combined, point);
        return scale;
    }
    sefCopyQuadword(resultPoint, combined[3]);
    return scale;
}

/* Light-set views of the effect entry table. An entry carries the light object, the point count and the handover flag;
 * SefLightEntry and SefLightEntryPoints are kept apart: their colour and flag field types decide the emitted order. */
typedef struct SefLightSet {
    AlignedVector4 positions[7];
    AlignedVector4 colors[7];
} SefLightSet;

typedef struct SefLightObject {
    u8 unmodeled_00[0x90];
    s32 stateBits;
    u8 unmodeled_94[0xAC];
    SefLightSet lights;
} SefLightObject;

typedef union SefLightColor {
    AlignedVector4 vector;
    f32 floats[4];
    s32 ints[4];
} SefLightColor;

typedef struct SefLightEntry {
    u8 unmodeled_000[0x1320];
    AlignedVector4 points[16];
    AlignedVector4 colors[16];
    s8 pointFlags[16];
    u8 unmodeled_1530[0x20];
    SefLightObject *obj;
    u8 unmodeled_1554[0x3C];
    s16 kind;
    s16 pointCount;
    s16 unmodeled_1594;
    s16 lightHandoverFlag;
    u8 unmodeled_1598[8];
} SefLightEntry;

typedef struct SefLightEntryTable {
    SefLightEntry entries[24];
    u8 unmodeled_15A0[0x20710 - 24 * 0x15A0];
    s32 count;
} SefLightEntryTable;

typedef struct SefLightEntryPoints {
    u8 unmodeled_000[0x1320];
    AlignedVector4 points[16];
    SefLightColor colors[16];
    u8 pointFlags[16];
    u8 unmodeled_1530[0x20];
    SefLightObject *obj;
    u8 unmodeled_1554[0x3C];
    s16 kind;
    s16 pointCount;
    u8 unmodeled_1594[0xC];
} SefLightEntryPoints;

typedef struct SefLightEntryPointsTable {
    SefLightEntryPoints entries[24];
    u8 unmodeled_15A0[0x20710 - 24 * 0x15A0];
    s32 count;
} SefLightEntryPointsTable;

typedef struct SefCameraSelectFlags {
    u64 unmodeled_lo : 2;
    u64 mode : 2;
    u64 unmodeled_hi : 60;
} SefCameraSelectFlags;

#define SEF_LIGHT_ENTRY_TABLE (*(SefLightEntryTable *)&sefEffectEntryTable)
#define SEF_LIGHT_ENTRY_POINTS_TABLE (*(SefLightEntryPointsTable *)&sefEffectEntryTable)
#define SEF_LIGHT_CAMERA_SELECT_FLAGS (*(SefCameraSelectFlags *)&gCameraSelectFlags)

/* Copies one quadword. */
static inline void SefLightCopyQuad(AlignedVector4 *dest, const AlignedVector4 *source) {
    __asm__ __volatile__("lq $8,0(%5)\n\tsq $8,0(%4)"
                         : "=m"(dest->x), "=m"(dest->y), "=m"(dest->z), "=m"(dest->w)
                         : "r"(dest), "r"(source)
                         : "$8");
}

void pxySetHeaderScanEnabled(s32);
void pxyEnableFlag80000(void);

/* With camera select mode 0, every active entry with an object and kind other than 2 hands its points to the active light
 * set: an entry without points clears its handover flag; one with points copies lights 3-5 of the active set, then fills
 * light slots 3-5 with its points, the flagged ones first. */
void func_002071F8(void) {
    s32 i;
    s32 j;

    if (SEF_LIGHT_CAMERA_SELECT_FLAGS.mode == 0) {
        for (i = 0; i < SEF_LIGHT_ENTRY_TABLE.count; i++) {
            SefLightEntry *entry = &SEF_LIGHT_ENTRY_TABLE.entries[i];
            if (entry->obj != 0 && entry->kind != 2) {
                SefLightSet *lights = &entry->obj->lights;
                SefLightSet *active = sefLightSetActiveGet();
                if (entry->pointCount == 0) {
                    if (entry->lightHandoverFlag != 0) {
                        entry->lightHandoverFlag = 0;
                        for (j = 0; j < 3; j++) {
                            SefLightCopyQuad(&lights->colors[j + 3], &active->colors[j + 3]);
                            SefLightCopyQuad(&lights->positions[j + 3], &active->positions[j + 3]);
                        }
                    }
                } else {
                    s32 slot = 3;

                    pxySetHeaderScanEnabled(0);
                    pxyEnableFlag80000();
                    entry->lightHandoverFlag = 1;
                    entry->obj->stateBits |= 8;
                    for (j = 0; j < 3; j++) {
                        SefLightCopyQuad(&lights->colors[j + 3], &active->colors[j + 3]);
                        SefLightCopyQuad(&lights->positions[j + 3], &active->positions[j + 3]);
                    }
                    for (j = 0; j < entry->pointCount && slot < 6; j++) {
                        if (entry->pointFlags[j] != 0) {
                            SefLightCopyQuad(&lights->positions[slot], &entry->points[j]);
                            SefLightCopyQuad(&lights->colors[slot], &entry->colors[j]);
                            slot++;
                        }
                    }
                    if (slot < 6) {
                        for (j = 0; j < entry->pointCount && slot < 6; j++) {
                            if (entry->pointFlags[j] == 0) {
                                SefLightCopyQuad(&lights->positions[slot], &entry->points[j]);
                                SefLightCopyQuad(&lights->colors[slot], &entry->colors[j]);
                                slot++;
                            }
                        }
                    }
                }
            }
        }
    }
}


extern f32 MMathVectorDotProduct(AlignedVector4 *, AlignedVector4 *);

/* Copies one point. Kept apart from SefLightCopyQuad with the same asm: the helper count before each function decides
 * the label numbers of the object (.mdebug). */
static inline void SefLightCopyPoint(AlignedVector4 *dest, const AlignedVector4 *source) {
    __asm__ __volatile__("lq $8,0(%5)\n\tsq $8,0(%4)"
                         : "=m"(dest->x), "=m"(dest->y), "=m"(dest->z), "=m"(dest->w)
                         : "r"(dest), "r"(source)
                         : "$8");
}

/* Zeroes one vector on the VU0. */
static inline void SefLightZeroVector(AlignedVector4 *dest) {
    __asm__ __volatile__("sqc2 vf0,0(%4)" : "=m"(dest->x), "=m"(dest->y), "=m"(dest->z), "=m"(dest->w) : "r"(dest));
}

/* Subtracts b from a into d on the VU0 (xyz only). */
static inline void SefLightSubVector(AlignedVector4 *dest, AlignedVector4 *lhs, AlignedVector4 *rhs) {
    __asm__ __volatile__("lqc2 vf1,0(%3)\n\tlqc2 vf2,0(%4)\n\tvsub.xyz vf1,vf1,vf2\n\tsqc2 vf1,0(%5)"
                         : "=m"(dest->x), "=m"(dest->y), "=m"(dest->z)
                         : "r"(lhs), "r"(rhs), "r"(dest));
}

/* Adds one point per entry within rad of point: the offset from point to the entry's object, weighted by its falloff and
 * coloured by color. An entry without points first clears its light slots 3-5 and flags; a source or target object counts its flag. */
void func_00207490(AlignedVector4 *point, s32 *color, f32 rad) {
    s32 i;
    AlignedVector4 slotVector;
    AlignedVector4 offset;
    f32 radiusSquared = rad * rad;
    SefLightObject **actors = sefGetBattleData();

    for (i = 0; i < SEF_LIGHT_ENTRY_POINTS_TABLE.count; i++) {
        SefLightEntryPoints *entry = &SEF_LIGHT_ENTRY_POINTS_TABLE.entries[i];
        if (entry->obj != 0 && entry->pointCount < 16 && entry->kind != 2) {
            if (sefCopyRecordSlotVector(&slotVector, entry->obj, 3) != 0) {
                f32 distanceSquared;

                SefLightSubVector(&offset, point, &slotVector);
                distanceSquared = MMathVectorDotProduct(&offset, &offset);
                if (distanceSquared < radiusSquared) {
                    f32 falloff;
                    s32 j;
                    SefLightSet *lights;

                    lights = &entry->obj->lights;
                    falloff = (radiusSquared - distanceSquared) / radiusSquared / 128.0f;
                    if (entry->pointCount == 0) {
                        for (j = 0; j < 3; j++) {
                            SefLightZeroVector(&lights->colors[j + 3]);
                            SefLightZeroVector(&lights->positions[j + 3]);
                        }
                        memset(entry->pointFlags, 0, 16);
                    }
                    if (actors[0] == entry->obj || actors[1] == entry->obj) {
                        entry->pointFlags[entry->pointCount]++;
                    }
                    SefLightCopyPoint(&entry->points[entry->pointCount], &offset);
                    entry->colors[entry->pointCount].vector.x = (f32) color[0] * falloff;
                    entry->colors[entry->pointCount].vector.y = (f32) color[1] * falloff;
                    entry->colors[entry->pointCount].vector.z = (f32) color[2] * falloff;
                    entry->colors[entry->pointCount].ints[3] = 0;
                    entry->pointCount++;
                }
            }
        }
    }
}

#undef sefEffectEntryTable

/* <<< CHUNK sef_134 */
int sefEffectFlagTest(int mask);
void sefEffectFlagSet(int mask);

#define SEF_GROUP_ENTRY_COUNT 8

/*
 * One group timer, 0x20 bytes, eight of them in sefGroupTimers. sefRegisterGroupCenter fills it for a tracked group
 * (the centre of the group's objects, the owner id and the key that name the group) with 600 frames on the timer;
 * sefTickGroupTimers counts `remaining` down; sefGetGroupCenter reads the centre back by owner and key.
 */
extern SefGroupTimer sefGroupTimers[SEF_GROUP_ENTRY_COUNT];
/* sefEffectFlags is one flag word; sefExecEffect tests bit 1 of it through sefEffectFlagTest. */
extern int sefEffectFlags;
void sefEffectFlagClear(int mask);

/* Counts every group timer down by one frame, stopping at zero. */
void sefTickGroupTimers(void)
{
    SefGroupTimer *groupTimer;
    int timerIndex;

    groupTimer = sefGroupTimers;
    for (timerIndex = SEF_GROUP_ENTRY_COUNT - 1; timerIndex >= 0; timerIndex--) {
        if (groupTimer->remaining > 0) {
            groupTimer->remaining = groupTimer->remaining - 1;
        }
        groupTimer++;
    }
}

/*
 * Writes the origin vector (0, 0, 0, 1) to center, then copies the centre of the first table entry whose owner and key
 * match. Leaves center as the origin when no entry matches.
 */
void sefGetGroupCenter(float *center, int owner, unsigned short key)
{
    SefGroupTimer *groupTimer;
    int timerIndex;

    Vu0SetOrigin(center);
    groupTimer = sefGroupTimers;
    for (timerIndex = 0; timerIndex < SEF_GROUP_ENTRY_COUNT; timerIndex++) {
        if (groupTimer->owner == owner && groupTimer->key == key) {
            QuadwordCopyT0(center, groupTimer->center);
            return;
        }
        groupTimer++;
    }
}

#define SEF_GROUP_MEMBER_MAX 24
#define SEF_GROUP_TIMER_FRAMES 600
#define SEF_RECORD_EXCLUDE_FLAG 0x8000000

Vector4 *MMathDivVectorS(Vector4 *destination, const Vector4 *source, float scale);

/*
 * Copies a group centre into its timer entry. Both registration paths share one copy block in the original, which
 * gcc only produces when the two copies come from the same asm statement.
 */
static inline void sefGroupStoreCenter(SefGroupTimer *groupTimer, Vector4 *centre)
{
    QuadwordCopyT0(groupTimer->center, centre);
}

/*
 * Gathers the live effect records owned by owner under key. Each distinct object is counted once and its position is
 * summed into the group centre; the centre is averaged when two or more objects contributed. The centre is written to
 * the table entry for (owner, key) if one exists, otherwise to the first free entry, which gets a 600-frame timer.
 */
void sefRegisterGroupCenter(int owner, unsigned short key)
{
    int scanIndex;
    int seenCount;
    int seenIndex;
    SefEffectRecord *effectRecords;
    SefEffectRecord *effectRecord;
    SefRecordOwner *groupMember;
    SefRecordOwner *seen[SEF_GROUP_MEMBER_MAX];
    AlignedVector4 centerSum;
    Vector4 *center;
    SefGroupTimer *groupTimer;

    effectRecords = sefGetScheduler();
    center = &centerSum;
    seenCount = 0;
    Vu0SetOrigin(center);
    for (scanIndex = 0; scanIndex < sefEffectRecordCount; scanIndex++) {
        effectRecord = &effectRecords[scanIndex];
        if (effectRecord->inUse == 0 || effectRecord->ownerKey != owner) {
            continue;
        }
        groupMember = effectRecord->ownedObject;
        if (groupMember == 0 || effectRecord->groupKey != key) {
            continue;
        }
        if (effectRecord->ownerGroupFlags & SEF_RECORD_EXCLUDE_FLAG) {
            continue;
        }
        for (seenIndex = 0; seenIndex < seenCount; seenIndex++) {
            if (seen[seenIndex] == effectRecord->ownedObject) {
                break;
            }
        }
        if (seenIndex != seenCount) {
            continue;
        }
        /* center.xyz += groupMember position; no memory clobber, so the effectRecord count load stays hoisted */
        __asm__ __volatile__(
            "lqc2 vf1, 0(%1)\n"
            "lqc2 vf2, 0(%2)\n"
            "vadd.xyz vf1, vf1, vf2\n"
            "sqc2 vf1, 0(%0)\n"
            :
            : "r" (center), "r" (center), "r" (&groupMember->position));
        seen[seenCount] = groupMember;
        seenCount++;
        if (seenCount >= SEF_GROUP_MEMBER_MAX) {
            break;
        }
    }
    if (seenCount >= 2) {
        MMathDivVectorS(center, center, seenCount);
    }

    for (scanIndex = 0; scanIndex < SEF_GROUP_ENTRY_COUNT; scanIndex++) {
        groupTimer = &sefGroupTimers[scanIndex];
        if (groupTimer->owner == owner && groupTimer->key == key) {
            groupTimer->remaining = SEF_GROUP_TIMER_FRAMES;
            groupTimer->key = key;
            sefGroupStoreCenter(groupTimer, center);
            return;
        }
    }

    for (scanIndex = 0; scanIndex < SEF_GROUP_ENTRY_COUNT; scanIndex++) {
        groupTimer = &sefGroupTimers[scanIndex];
        if (groupTimer->remaining == 0) {
            groupTimer->remaining = SEF_GROUP_TIMER_FRAMES;
            groupTimer->owner = owner;
            groupTimer->key = key;
            sefGroupStoreCenter(groupTimer, center);
            return;
        }
    }
}

extern unsigned char sefOverrideFlag[];

/* Returns the fog override block (the sefOverrideFlag bytes). */
u8 *sefGetFogOverrideBlock(void) {
    return sefOverrideFlag;
}

/* Sets the mask bits of sefEffectFlags. */
void sefEffectFlagSet(s32 mask) {
    sefEffectFlags |= mask;
}

/* Clears the mask bits of sefEffectFlags. */
void sefEffectFlagClear(s32 mask) {
    sefEffectFlags &= ~mask;
}

/* Returns the mask bits of sefEffectFlags that are set. */
int sefEffectFlagTest(int mask) {
    return sefEffectFlags & mask;
}

/* Toggles the mask bits of sefEffectFlags. */
void sefFlagToggle(int mask) {
    sefEffectFlags ^= mask;
}

/* Bit 1 of sefEffectFlags follows the sign of selector: negative toggles it, positive sets it, zero clears it. */
void sefEffectFlagUpdateFromSign(int selector) {
    if (selector < 0) {
        sefFlagToggle(1);
        return;
    }
    if (selector > 0) {
        sefEffectFlagSet(1);
        return;
    }
    sefEffectFlagClear(1);
}

/* Counts the records of owner under key that the owner-group flags do not exclude. */
int sefCountOwnerGroupRecords(int owner, unsigned short key)
{
    SefEffectRecord *effectRecord;
    int scanIndex;
    int matchCount;

    matchCount = 0;
    effectRecord = sefGetScheduler();
    for (scanIndex = 0; scanIndex < sefEffectRecordCount; scanIndex++) {
        if (effectRecord->inUse != 0 && effectRecord->ownerKey == owner && effectRecord->groupKey == key) {
            if ((effectRecord->ownerGroupFlags & SEF_RECORD_EXCLUDE_FLAG) == 0) {
                matchCount += 1;
            }
        }
        effectRecord++;
    }
    return matchCount;
}

/* Clears the mask bits of the records of owner under key, except records that carry SEF_RECORD_EXCLUDE_FLAG. */
void sefClearOwnerGroupFlags(int owner, unsigned short key, int mask)
{
    SefEffectRecord *effectRecord;
    int scanIndex;
    int scanLimit;
    int recordLimit;
    int keepBit;
    int recordBits;

    effectRecord = sefGetScheduler();
    scanLimit = sefEffectRecordCount;
    scanIndex = 0;
    if (scanLimit > 0) {
        keepBit = SEF_RECORD_EXCLUDE_FLAG;
        recordLimit = scanLimit;
        do {
            if (effectRecord->inUse != 0 && effectRecord->ownerKey == owner && effectRecord->groupKey == key) {
                recordBits = effectRecord->ownerGroupFlags;
                if ((recordBits & keepBit) == 0) {
                    effectRecord->ownerGroupFlags = recordBits & ~mask;
                    scanLimit = recordLimit;
                }
            }
            scanIndex += 1;
            effectRecord++;
        } while (scanIndex < scanLimit);
    }
}

/* <<< CHUNK sef_135 */

#include "common.h"
#include "shared.h"
#include "quadword.h"
#include "vu0_macro.h"
#include "main/tu031.h"

float xglFRand(void);
int F2I(float scalar);
float I2F(int integer);
void xglVectorNormal(Vector4 *destination, const Vector4 *source);
void xglVectorScale(Vector4 *destination, const Vector4 *source, float scale);
void xglVectorInner(float *destination, const Vector4 *left, const Vector4 *right);
void xglMatrixStackUnit(void);
void xglMatrixStackRotX(float angle);
void xglMatrixStackRotY(float angle);
void xglMatrixStackRotZ(float angle);
void xglMatrixStackMulVector(Vector4 *destination, const Vector4 *source);
void xglMatrixStackTrans(const Vector4 *vector);
void xglMatrixStackScale(const Vector4 *vector);
void xglMatrixStackInverse(void);
float acosf(float);

/* Next VU0 random value as a float: vrnext advances R, and the low 32 bits of vf1 are the result. */
float xglFRand(void)
{
    __asm__ __volatile__("vrnext.x vf1x, R\n\tqmfc2 $2, vf1\n\tmtc1 $2, $f0" : : : "$2", "$f0", "memory");
}

#include "main/tu141.h"

typedef int s32;

void xglFRandSeed(f32 seed) {
    __asm__ __volatile__("qmtc2.ni %0, vf1" : : "r"(seed) : "memory");
    __asm__ __volatile__("vrinit R, vf1x" : : : "memory");
}

int F2I(register f32 scalar)
{
    register int truncated;

    __asm__ __volatile__(
        "mtc1 %1,$f0\n\t"
        "cvt.w.s $f0,$f0\n\t"
        "mfc1 %0,$f0"
        : "=r"(truncated)
        : "r"(scalar)
        : "$f0");
    return truncated;
}

f32 I2F(register int integer)
{
    return integer;
}

extern char D_20[];

float xglSin(float angle)
{
    u32 angleBits;

    __asm__("mfc1 %0, %1" : "=r"(angleBits) : "f"(angle));
    __asm__ __volatile__(
        "ctc2.i %1, $vi27\n\t"
        "vnop\n\t"
        "qmtc2 %0, $vf4\n\t"
        "vcallmsr $vi27\n\t"
        "qmfc2.i %0, $vf1\n\t"
        "mtc1 %0, $f0"
        : "+r"(angleBits) : "r"((u32)&D_20 >> 3) : "$f0", "memory");
}

extern char D_E8[];

float xglCos(float angle)
{
    u32 angleBits;

    __asm__("mfc1 %0, %1" : "=r"(angleBits) : "f"(angle));
    __asm__ __volatile__(
        "ctc2.i %1, $vi27\n\t"
        "vnop\n\t"
        "qmtc2 %0, $vf4\n\t"
        "vcallmsr $vi27\n\t"
        "qmfc2.i %0, $vf1\n\t"
        "mtc1 %0, $f0"
        : "+r"(angleBits) : "r"((u32)&D_E8 >> 3) : "$f0", "memory");
}

void func_001975C8();

void xglLightPrepare(void) {
    func_001975C8();
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_001975C8);

/* A light pack: the pack a light slot holds is its address. Only the fields the light readers use are named. */
typedef struct XglLightPack {
    u8 unmodeled_00[0x30];
    s32 lightEntryCount;         /* 0x30 */
    u8 unmodeled_34[0x8];
    s32 keyOffset;          /* 0x3C */
    u8 unmodeled_40[0xB0];
    s32 lightEntryByteOffsets[1]; /* 0xF0 */
} XglLightPack;

/* One of the 16 light slots at xglLightSlotTable: the light pack it holds (0 when free). */
typedef struct XglLightSlot {
    XglLightPack *lightPack;
    s32 unk_04;
    u8 unk_08[0x8];
} XglLightSlot;

extern XglLightSlot xglLightSlotTable[];

void xglLightSlotsClear(void) {
    s32 lightSlotNumber;
    XglLightSlot *lightSlot;

    lightSlot = xglLightSlotTable;
    for (lightSlotNumber = 0; lightSlotNumber < 16; lightSlotNumber++) {
        lightSlot->lightPack = 0;
        lightSlot->unk_04 = 0;
        lightSlot += 1;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_001978B8);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_00197970);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_00197A00);


/* One light entry of a pack: its channel types select how each of the 20 values is produced. */
/* One word of a light entry's key stream: a raw constant value or a curve header word. */
typedef union XglLightStreamWord {
    s32 word;
    f32 value;
} XglLightStreamWord;

typedef struct XglLightEntry {
    u8 unmodeled_00[0x10];
    u8 channelTypes[20];    /* 0x10 */
    u8 unmodeled_24[0xC];
    XglLightStreamWord stream[1]; /* 0x30 */
} XglLightEntry;

f32 func_00197A00(XglLightStreamWord **cursor, f32 time);

void xglLightPackEvaluate(XglLightPack *pack, f32 curveTime)
{
    s32 *lightEntryByteOffsets;
    XglLightKey *keys;
    XglLightEntry *packEntry;
    XglLightStreamWord *cursor;
    f32 *keyValues;
    s32 lightEntryIndex;
    s32 channel;

    lightEntryByteOffsets = pack->lightEntryByteOffsets;
    keys = (XglLightKey *)((u8 *)pack + pack->keyOffset);
    for (lightEntryIndex = 0; lightEntryIndex < pack->lightEntryCount; lightEntryIndex++) {
        packEntry = (XglLightEntry *)((u8 *)pack + lightEntryByteOffsets[lightEntryIndex]);
        cursor = packEntry->stream;
        keyValues = (f32 *)&keys[lightEntryIndex];
        for (channel = 0; channel < 20; channel++) {
            switch (packEntry->channelTypes[channel]) {
            case 2:
                keyValues[channel] = (cursor++)->value;
                break;
            case 3:
                keyValues[channel] = 0.0f;
                break;
            case 4:
                keyValues[channel] = 1.0f;
                break;
            case 1:
                while (((s32)cursor & 0xf) != 0) {
                    cursor++;
                }
                keyValues[channel] = func_00197A00(&cursor, curveTime);
                break;
            }
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_00197C98);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_00197DB0);

int xglLightSpotColor(Vector4 *color, Vector4 *lightDirection, XglLightKey *key, Vector4 *receiver)
{
    DoublewordAlignedVector4 coneAxis = { 0.0f, 0.0f, -1.0f, 1.0f };
    float angle;
    float halfCone;

    halfCone = key->coneAngle * 0.5f;
    Vu0SubVector(lightDirection, receiver, &key->position);
    xglVectorNormal(lightDirection, lightDirection);
    xglMatrixStackUnit();
    xglMatrixStackRotZ(key->rotation.z);
    xglMatrixStackRotY(key->rotation.y);
    xglMatrixStackRotX(key->rotation.x);
    xglMatrixStackMulVector(&coneAxis, &coneAxis);
    xglVectorInner(&angle, &coneAxis, lightDirection);
    angle = acosf(angle);
    if (angle > halfCone) {
        return 0;
    }
    QuadwordCopy(color, &key->color);
    if (halfCone - __builtin_fabsf(key->penumbra) < angle) {
        halfCone = (halfCone - angle) / __builtin_fabsf(key->penumbra);
        xglVectorScale(color, color, halfCone);
    }
    xglVectorScale(lightDirection, lightDirection, -1.0f);
    return 1;
}

int xglLightBoxColor(Vector4 *color, Vector4 *lightDirection, XglLightKey *key, Vector4 *receiver)
{
    DoublewordAlignedVector4 boxAxis = {0};
    float edgeDistance;

    boxAxis.z = 1.0f;
    edgeDistance = 1000000.0f;
    *lightDirection = *receiver;
    xglMatrixStackUnit();
    xglMatrixStackTrans(&key->position);
    if (key->rotation.z != 0.0f) {
        xglMatrixStackRotZ(key->rotation.z);
    }
    if (key->rotation.y != 0.0f) {
        xglMatrixStackRotY(key->rotation.y);
    }
    if (key->rotation.x != 0.0f) {
        xglMatrixStackRotX(key->rotation.x);
    }
    xglMatrixStackMulVector(&boxAxis, &boxAxis);
    xglMatrixStackScale(&key->extent);
    xglMatrixStackInverse();
    lightDirection->w = 1.0f;
    xglMatrixStackMulVector(lightDirection, lightDirection);
    if (lightDirection->x < -1.0f || lightDirection->x > 1.0f || lightDirection->y < -1.0f || lightDirection->y > 1.0f || lightDirection->z > 0.0f) {
        return 0;
    }
    QuadwordCopy(color, &key->color);
    if (__builtin_fabsf((-1.0f - lightDirection->x) * key->extent.x) < edgeDistance) {
        edgeDistance = __builtin_fabsf((-1.0f - lightDirection->x) * key->extent.x);
    }
    if (__builtin_fabsf((1.0f - lightDirection->x) * key->extent.x) < edgeDistance) {
        edgeDistance = __builtin_fabsf((1.0f - lightDirection->x) * key->extent.x);
    }
    if (__builtin_fabsf((-1.0f - lightDirection->y) * key->extent.y) < edgeDistance) {
        edgeDistance = __builtin_fabsf((-1.0f - lightDirection->y) * key->extent.y);
    }
    if (__builtin_fabsf((1.0f - lightDirection->y) * key->extent.y) < edgeDistance) {
        edgeDistance = __builtin_fabsf((1.0f - lightDirection->y) * key->extent.y);
    }
    if (edgeDistance < 1.0f) {
        xglVectorScale(color, color, edgeDistance);
    }
    *lightDirection = boxAxis;
    return 1;
}

void xglMatrixStackSave(Matrix4 matrix);
void xglMatrixStackLoad(Matrix4 matrix);
float xglPointLength(Vector4 *from, Vector4 *to);
extern const DoublewordAlignedVector4 xglLightConeEdgeTemplate;

static inline void xglLightConeFinish(Vector4 *color, Vector4 *lightDirection, XglLightKey *key, Matrix4 keyMatrix, float fade)
{
    xglMatrixStackLoad(keyMatrix);
    lightDirection->w = 0.0f;
    xglMatrixStackMulVector(lightDirection, lightDirection);
    xglVectorNormal(lightDirection, lightDirection);
    xglVectorScale(lightDirection, lightDirection, -1.0f);
    QuadwordCopy(color, &key->color);
    xglVectorScale(color, color, fade);
}

int xglLightConeColor(Vector4 *color, Vector4 *lightDirection, XglLightKey *key, Vector4 *receiver)
{
    Matrix4 keyMatrix;
    Vector4 coneScale;
    float innerSlope;
    float outerSlope;
    float fade;
    float coneRatio;
    DoublewordAlignedVector4 edgePoint;

    innerSlope = key->extent.x / key->extent.z;
    outerSlope = key->extent.y / key->extent.z;
    fade = 1.0f;
    coneRatio = 1.0f;
    coneScale.x = key->extent.y;
    coneScale.y = key->extent.z;
    coneScale.z = key->extent.y;
    coneScale.w = 1.0f;

    *lightDirection = *receiver;
    xglMatrixStackUnit();
    xglMatrixStackTrans(&key->position);
    if (key->rotation.z != 0.0f) {
        xglMatrixStackRotZ(key->rotation.z);
    }
    if (key->rotation.y != 0.0f) {
        xglMatrixStackRotY(key->rotation.y);
    }
    if (key->rotation.x != 0.0f) {
        xglMatrixStackRotX(key->rotation.x);
    }
    xglMatrixStackSave(keyMatrix);
    xglMatrixStackScale(&coneScale);
    xglMatrixStackInverse();
    lightDirection->w = 1.0f;
    xglMatrixStackMulVector(lightDirection, lightDirection);

    if (lightDirection->y > 1.0f || lightDirection->y < -1.0f) {
        edgePoint = xglLightConeEdgeTemplate;
        if (lightDirection->y < -1.0f) {
            edgePoint.y = -1.0f;
        }
        coneRatio = xglPointLength(lightDirection, &edgePoint) * outerSlope;
        if (coneRatio >= outerSlope) {
            return 0;
        }
        if (coneRatio > innerSlope) {
            fade = 1.0f - (coneRatio - innerSlope) / (outerSlope - innerSlope);
        }
        Vu0SubVectorXYZ(lightDirection, lightDirection, &edgePoint);
        xglLightConeFinish(color, lightDirection, key, keyMatrix, fade);
        return 1;
    } else {
        if (lightDirection->x > -1.0f && lightDirection->x < 1.0f && lightDirection->z > -1.0f && lightDirection->z < 1.0f) {
            coneRatio = __builtin_sqrtf(lightDirection->x * lightDirection->x + lightDirection->z * lightDirection->z) * outerSlope;
            if (coneRatio >= outerSlope) {
                return 0;
            }
            if (coneRatio > innerSlope) {
                fade = 1.0f - (coneRatio - innerSlope) / (outerSlope - innerSlope);
            }
            lightDirection->y = 0.0f;
            xglLightConeFinish(color, lightDirection, key, keyMatrix, fade);
            return 1;
        }
    }
    return 0;
}

extern s32 xglLightBankValid;
extern unsigned char xglLightBankStorage[];

u8 *xglLightBankGet(void) {
    if (xglLightBankValid != 0) {
        return xglLightBankStorage;
    }
    return 0;
}

void xglLightBankReset(void) {
    xglLightBankValid = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_00198570);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_00198C08);

void xglLightSlotsInit(void) {
    s32 lightSlotNumber;
    XglLightSlot *lightSlot;

    lightSlot = xglLightSlotTable;
    for (lightSlotNumber = 0; lightSlotNumber < 16; lightSlotNumber++) {
        lightSlot->lightPack = 0;
        lightSlot->unk_04 = 0;
        lightSlot += 1;
    }
}

void xglLightPackEvaluate(XglLightPack *pack, f32 curveTime);

void xglLightSlotsApply(f32 curveTime) {
    XglLightPack *lightPack;
    s32 lightSlotNumber;
    XglLightSlot *lightSlot;

    lightSlot = xglLightSlotTable;
    for (lightSlotNumber = 0; lightSlotNumber < 16; lightSlotNumber++) {
        lightPack = lightSlot->lightPack;
        lightSlot += 1;
        if (lightPack != 0) {
            xglLightPackEvaluate(lightPack, curveTime);
        }
    }
}

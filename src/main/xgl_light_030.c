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
    union {
        u64 signature;            /* 0x00: "lxy" in the low three bytes */
        u8 unmodeled_00[0x30];
    };
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

#define XGL_LIGHT_PACK_SIGNATURE 0x79786C /* "lxy" */

/* Registers a light pack in the slot table: 2 when it was registered already (one more reference), 1 when a free
 * slot took it, 0 when it is no light pack or no slot is free (X2 xglLightPackRegister). */
int xglLightPackRegister(XglLightPack *pack)
{
    int i;

    if (pack != 0 && (pack->signature & 0xFFFFFF) == XGL_LIGHT_PACK_SIGNATURE) {
        for (i = 0; i < 16; i++) {
            if (xglLightSlotTable[i].lightPack == pack) {
                xglLightSlotTable[i].unk_04 = xglLightSlotTable[i].unk_04 + 1;
                return 2;
            }
        }
        for (i = 0; i < 16; i++) {
            if (xglLightSlotTable[i].lightPack == 0) {
                xglLightSlotTable[i].lightPack = pack;
                xglLightSlotTable[i].unk_04 = 1;
                return 1;
            }
        }
    }
    return 0;
}

/* Drops one reference of a registered light pack and frees its slot at the last one: 1 when the slot was freed
 * (X2 xglLightPackRelease). */
int xglLightPackRelease(XglLightPack *pack)
{
    int i;

    if (pack != 0 && (pack->signature & 0xFFFFFF) == XGL_LIGHT_PACK_SIGNATURE) {
        for (i = 0; i < 16; i++) {
            if (xglLightSlotTable[i].lightPack == pack) {
                xglLightSlotTable[i].unk_04 = xglLightSlotTable[i].unk_04 - 1;
                if (xglLightSlotTable[i].unk_04 <= 0) {
                    xglLightSlotTable[i].lightPack = 0;
                    xglLightSlotTable[i].unk_04 = 0;
                    return 1;
                }
                return 0;
            }
        }
    }
    return 0;
}

/* One word of a light entry's key stream: a raw constant value or a curve header word. */
typedef union XglLightStreamWord {
    s32 word;
    f32 value;
} XglLightStreamWord;

/* A curve in a light entry's key stream: a header, then count key rows (time, value). The sample
 * time is held in 24.8 fixed point, clamped to [minimum, maximum]. */
typedef struct XglLightCurve {
    int count;
    int minimum;
    int maximum;
    int unknown12;
    int fixed;
    u8 unmodeled_14[0x20 - 0x14];
    Vector4 keys[1];         /* +0x20: count key rows (time, value) */
} XglLightCurve;

#define LIGHT_CURVE_FIXED_ONE 256.0f
#define LIGHT_CURVE_KEY_WEIGHT 0.2f

float func_0019FA20(int count, Vector4 *keyframes, float time, float weight);

/* XS2 xglLightPackSampleCurve: samples the curve at *cursor at time and moves *cursor past it. */
f32 func_00197A00(XglLightStreamWord **cursor, f32 time)
{
    float result = 0.0f;
    XglLightCurve *record;
    Vector4 *keys;

    record = (XglLightCurve *) *cursor;
    record->fixed = ((int) time << 8) + (int) ((time - (float) (int) time) * LIGHT_CURVE_FIXED_ONE);
    if (record->fixed > (record->maximum << 8)) {
        record->fixed = record->maximum << 8;
    }
    if (record->fixed < (record->minimum << 8)) {
        record->fixed = record->minimum << 8;
    }
    keys = record->keys;
    result = func_0019FA20(record->count, keys,
                           (float) (record->fixed / 256) + (float) (record->fixed % 256) / LIGHT_CURVE_FIXED_ONE,
                           LIGHT_CURVE_KEY_WEIGHT);
    *cursor = (XglLightStreamWord *) &keys[record->count];
    return result;
}


/* One light entry of a pack: its channel types select how each of the 20 values is produced. */
typedef struct XglLightEntry {
    u8 unmodeled_00[0x4];
    s32 entryType;          /* 0x04: XGL_LIGHT_* (below XGL_LIGHT_SLOT_TYPES: ambient and global keys) */
    u8 unmodeled_08[0x8];
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

/* One light direction slot; directions[0].weakest is the index of the weakest colour slot. */
typedef struct XglLightSlotDir {
    float x;
    float y;
    float z;
    int weakest;
} XglLightSlotDir;

/* The six strongest lights at a receiver: colours (w = strength) and directions. */
typedef struct XglLightSlots {
    Vector4 colors[6];
    XglLightSlotDir directions[6];
} XglLightSlots;

#define LIGHT_SLOT_COUNT 6

/* Replaces the weakest of the six slots when color (strength = its largest
 * channel) is stronger, stores its normalised direction (+z when zero) and finds the new weakest slot.
 * The (&array[0])[index] form is the XS2 one: plain lightSlots->colors[index] makes GCC keep &colors[0].w
 * in an extra s-register (73 instead of 70 instructions). */
void xglLightSlotsInsert(XglLightSlots *lightSlots, Vector4 *color, Vector4 *lightDir)
{
    float strength;
    int i;
    float weakestStrength;
    Vector4 *slot;

    strength = color->x;
    if (strength < color->y) {
        strength = color->y;
    }
    if (strength < color->z) {
        strength = color->z;
    }
    if ((&lightSlots->colors[0])[(&lightSlots->directions[0])->weakest].w < strength) {
        weakestStrength = 0;
        slot = &(&lightSlots->colors[0])[(&lightSlots->directions[0])->weakest];
        QuadwordCopy(slot, color);
        slot->w = strength;
        slot[LIGHT_SLOT_COUNT].x = lightDir->x;
        slot[LIGHT_SLOT_COUNT].y = lightDir->y;
        slot[LIGHT_SLOT_COUNT].z = lightDir->z;
        if (slot[LIGHT_SLOT_COUNT].x == 0.0f && slot[LIGHT_SLOT_COUNT].y == 0.0f
            && slot[LIGHT_SLOT_COUNT].z == 0.0f) {
            slot[LIGHT_SLOT_COUNT].z = 1.0f;
        }
        xglVectorNormal(&slot[LIGHT_SLOT_COUNT], &slot[LIGHT_SLOT_COUNT]);
        weakestStrength = (&lightSlots->colors[0])[0].w;
        (&lightSlots->directions[0])->weakest = 0;
        for (i = 1; i < LIGHT_SLOT_COUNT; i++) {
            if ((&lightSlots->colors[0])[i].w < weakestStrength) {
                weakestStrength = (&lightSlots->colors[0])[i].w;
                (&lightSlots->directions[0])->weakest = i;
            }
        }
    }
}

/* The key's colour at the receiver, faded linearly between extent.x and
 * extent.y; 0 (and a zero colour) when the receiver is at extent.y or farther. */
int xglLightPointColor(Vector4 *color, XglLightKey *key, Vector4 *receiver)
{
    Vector4 toLight;
    float distance;

    QuadwordZero(color);
    Vu0SubVectorXYZ(&toLight, &key->position, receiver);
    distance = toLight.x * toLight.x + toLight.y * toLight.y + toLight.z * toLight.z;
    if (key->extent.y * key->extent.y <= distance) {
        return 0;
    }
    distance = __builtin_sqrtf(distance);
    QuadwordCopy(color, &key->color);
    if (distance > key->extent.x) {
        distance = (key->extent.y - distance) / (key->extent.y - key->extent.x);
        xglVectorScale(color, color, distance);
    }
    return 1;
}

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
/* The global colour scale of the light bank (X2 xglLightColorScale). */
extern Vector4 xglLightBankStorage;

Vector4 *xglLightBankGet(void) {
    if (xglLightBankValid != 0) {
        return &xglLightBankStorage;
    }
    return 0;
}

void xglLightBankReset(void) {
    xglLightBankValid = 0;
}

/* Light key types of a light pack entry. The first group sets the ambient and global parts of the light
 * set; from XGL_LIGHT_SLOT_TYPES on a key lights the receiver through the six light slots or is one of the
 * three point lights. */
#define XGL_LIGHT_AMBIENT 1
#define XGL_LIGHT_AMBIENT_SKY 2
#define XGL_LIGHT_AMBIENT_POINT 3
#define XGL_LIGHT_AMBIENT_BOX 4
#define XGL_LIGHT_AMBIENT_SPOT 6
#define XGL_LIGHT_AMBIENT_CONE 7
#define XGL_LIGHT_DIRECTIONAL 8
#define XGL_LIGHT_COLOR_SCALE 9
#define XGL_LIGHT_SHADOW_DIRECTION 10
#define XGL_LIGHT_SHADOW_POINT 11
#define XGL_LIGHT_SLOT_TYPES 19
#define XGL_LIGHT_POINT 19
#define XGL_LIGHT_SLOT_POINT 20
#define XGL_LIGHT_SLOT_BOX 21
#define XGL_LIGHT_SLOT_SPOT 23
#define XGL_LIGHT_SLOT_CONE 24
#define XGL_LIGHT_SLOT_POINT_ALT 35
/* The light set holds three point lights; their colour is scaled to the GS range. */
#define XGL_LIGHT_POINTS 3
#define XGL_LIGHT_POINT_COLOR_SCALE 128.0f

/* X2 xglLightSetClear, inlined: clears the 18 quadwords of a light set. */
#define XglLightSetClear(lightSet) \
    __asm__ __volatile__( \
        "sq $0, 0x0(%0)\n" "sq $0, 0x10(%0)\n" "sq $0, 0x20(%0)\n" "sq $0, 0x30(%0)\n" \
        "sq $0, 0x40(%0)\n" "sq $0, 0x50(%0)\n" "sq $0, 0x60(%0)\n" "sq $0, 0x70(%0)\n" \
        "sq $0, 0x80(%0)\n" "sq $0, 0x90(%0)\n" "sq $0, 0xA0(%0)\n" "sq $0, 0xB0(%0)\n" \
        "sq $0, 0xC0(%0)\n" "sq $0, 0xD0(%0)\n" "sq $0, 0xE0(%0)\n" "sq $0, 0xF0(%0)\n" \
        "sq $0, 0x100(%0)\n" "sq $0, 0x110(%0)\n" \
        : \
        : "r" (lightSet) \
        : "memory")

/* The key's rotation (Z, Y, X; zero angles skipped) on a fresh matrix stack. */
#define XGL_LIGHT_KEY_ROTATION(key) \
    { \
        xglMatrixStackUnit(); \
        if ((key)->rotation.z != 0.0f) { \
            xglMatrixStackRotZ((key)->rotation.z); \
        } \
        if ((key)->rotation.y != 0.0f) { \
            xglMatrixStackRotY((key)->rotation.y); \
        } \
        if ((key)->rotation.x != 0.0f) { \
            xglMatrixStackRotX((key)->rotation.x); \
        } \
    }

/* X2 xglVectorCopy, inlined: one quadword through $2. */
static inline void xglVectorCopy(void *destination, void *source)
{
    QuadwordCopy(destination, source);
}

void *memset(void *destination, int value, unsigned int size);
/* The key axis (0, 0, -1, 1) a light set build starts every key from; like xglLightConeEdgeTemplate it is
 * copied from its own .rodata item (the same initialiser in C would be shared with xglLightSpotColor's). */
extern const DoublewordAlignedVector4 xglLightKeyAxisTemplate;
void pxySetColorScale(Vector4 *color);

/* Builds the light set of a receiver from the keys of a light pack. The keys
 * before XGL_LIGHT_SLOT_TYPES set the colour scale, the ambient colours, the view and shadow directions and
 * add point, cone, spot and box light to the ambient colours; the later keys fill the three point lights and
 * the six strongest light slots, which end up as the set's directions and colours. */
void xglLightPackBuildLightSet(XglLightPack *pack, XglLightSet *lightSet, Vector4 *receiver)
{
    int i;
    s32 *entryOffsets;
    XglLightSlots lightSlots;

    entryOffsets = pack->lightEntryByteOffsets;
    memset(&lightSlots, 0, sizeof(lightSlots));
    XglLightSetClear(lightSet);
    for (i = 0; i < pack->lightEntryCount; i++) {
        XglLightEntry *entry = (XglLightEntry *)((u8 *)pack + entryOffsets[i]);

        if (entry->entryType < XGL_LIGHT_SLOT_TYPES) {
            XglLightKey *key = &((XglLightKey *)((u8 *)pack + pack->keyOffset))[i];
            DoublewordAlignedVector4 axisDir = xglLightKeyAxisTemplate;
            Vector4 pointColor;
            Vector4 coneColor;
            Vector4 coneDirection;
            Vector4 spotColor;
            Vector4 spotDirection;
            Vector4 boxColor;
            Vector4 boxDirection;
            Vector4 scaledColor;
            Vector4 *lightColor;
            int lit;

            switch (entry->entryType) {
            case XGL_LIGHT_COLOR_SCALE:
                xglVectorScale(&xglLightBankStorage, &key->color, key->intensity);
                pxySetColorScale(&xglLightBankStorage);
                xglLightBankValid = 1;
                break;
            case XGL_LIGHT_AMBIENT:
                xglVectorScale(&lightSet->ambientColor, &key->color, key->intensity);
                xglVectorScale(&lightSet->ambientColorLower, &key->color, key->intensity);
                break;
            case XGL_LIGHT_AMBIENT_SKY:
                xglVectorScale(&lightSet->ambientColor, &key->color, key->intensity);
                xglVectorScale(&lightSet->ambientColorLower, &key->extent, key->intensity);
                XGL_LIGHT_KEY_ROTATION(key)
                xglMatrixStackMulVector(&axisDir, &axisDir);
                xglVectorCopy(&lightSet->viewDirection, &axisDir);
                break;
            case XGL_LIGHT_AMBIENT_POINT:
                lightColor = &pointColor;
                lit = xglLightPointColor(lightColor, key, receiver);
            addAmbientLight:
                if (!lit) {
                    break;
                }
                xglVectorScale(lightColor, lightColor, key->intensity);
                Vu0AddVectorXYZ(&lightSet->ambientColor, &lightSet->ambientColor, lightColor);
                Vu0AddVectorXYZ(&lightSet->ambientColorLower, &lightSet->ambientColorLower, lightColor);
                break;
            case XGL_LIGHT_AMBIENT_CONE:
                lightColor = &coneColor;
                lit = xglLightConeColor(lightColor, &coneDirection, key, receiver);
                goto addAmbientLight;
            case XGL_LIGHT_AMBIENT_SPOT:
                lightColor = &spotColor;
                lit = xglLightSpotColor(lightColor, &spotDirection, key, receiver);
                goto addAmbientLight;
            case XGL_LIGHT_AMBIENT_BOX:
                lightColor = &boxColor;
                lit = xglLightBoxColor(lightColor, &boxDirection, key, receiver);
                goto addAmbientLight;
            case XGL_LIGHT_DIRECTIONAL:
                axisDir.z = 1.0f;
                XGL_LIGHT_KEY_ROTATION(key)
                xglMatrixStackMulVector(&axisDir, &axisDir);
                xglVectorScale(&scaledColor, &key->color, key->intensity);
                xglLightSlotsInsert(&lightSlots, &scaledColor, &axisDir);
                break;
            case XGL_LIGHT_SHADOW_DIRECTION:
                axisDir.z = 1.0f;
                XGL_LIGHT_KEY_ROTATION(key)
                xglMatrixStackMulVector(&lightSet->shadows[0], &axisDir);
                lightSet->shadows[0].w = 1.0f;
                break;
            case XGL_LIGHT_SHADOW_POINT:
                Vu0SubVectorXYZ(&lightSet->shadows[0], &key->position, receiver);
                xglVectorNormal(&lightSet->shadows[0], &lightSet->shadows[0]);
                lightSet->shadows[0].w = 1.0f;
                break;
            }
        } else {
            break;
        }
    }
    {
        int pointCount;

        pointCount = 0;
        for (i = 0; i < pack->lightEntryCount; i++) {
            XglLightEntry *entry = (XglLightEntry *)((u8 *)pack + entryOffsets[i]);

            if (entry->entryType >= XGL_LIGHT_SLOT_TYPES) {
                XglLightKey *key = &((XglLightKey *)((u8 *)pack + pack->keyOffset))[i];
                Vector4 pointColor;
                Vector4 pointDirection;
                Vector4 coneColor;
                Vector4 coneDirection;
                Vector4 boxColor;
                Vector4 boxDirection;
                Vector4 spotColor;
                Vector4 spotDirection;

                switch (entry->entryType) {
                case XGL_LIGHT_POINT:
                    if (pointCount < XGL_LIGHT_POINTS) {
                        xglVectorCopy(lightSet->pointPositions[pointCount], &key->position);
                        xglVectorScale((Vector4 *)lightSet->pointColors[pointCount], &key->color,
                                       key->intensity * XGL_LIGHT_POINT_COLOR_SCALE);
                        lightSet->pointPositions[pointCount][3] = 1.0f;
                        pointCount++;
                    }
                    break;
                case XGL_LIGHT_SLOT_POINT:
                case XGL_LIGHT_SLOT_POINT_ALT:
                    if (!xglLightPointColor(&pointColor, key, receiver)) {
                        break;
                    }
                    Vu0SubVectorXYZ(&pointDirection, &key->position, receiver);
                    xglVectorScale(&pointColor, &pointColor, key->intensity);
                    xglLightSlotsInsert(&lightSlots, &pointColor, &pointDirection);
                    break;
                case XGL_LIGHT_SLOT_CONE:
                    if (!xglLightConeColor(&coneColor, &coneDirection, key, receiver)) {
                        break;
                    }
                    xglVectorScale(&coneColor, &coneColor, key->intensity);
                    xglLightSlotsInsert(&lightSlots, &coneColor, &coneDirection);
                    break;
                case XGL_LIGHT_SLOT_BOX:
                    if (!xglLightBoxColor(&boxColor, &boxDirection, key, receiver)) {
                        break;
                    }
                    xglVectorScale(&boxColor, &boxColor, key->intensity);
                    xglLightSlotsInsert(&lightSlots, &boxColor, &boxDirection);
                    break;
                case XGL_LIGHT_SLOT_SPOT:
                    if (!xglLightSpotColor(&spotColor, &spotDirection, key, receiver)) {
                        break;
                    }
                    xglVectorScale(&spotColor, &spotColor, key->intensity);
                    xglLightSlotsInsert(&lightSlots, &spotColor, &spotDirection);
                    break;
                }
            }
        }
    }
    {
        int slot;

        for (slot = 0; slot < LIGHT_SLOT_COUNT; slot++) {
            xglVectorCopy(lightSet->colors[slot], &lightSlots.colors[slot]);
            xglVectorCopy(lightSet->directions[slot], &lightSlots.directions[slot]);
            lightSet->colors[slot][3] = 0;
        }
    }
}

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

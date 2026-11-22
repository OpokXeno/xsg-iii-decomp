#include "common.h"
#include "shared.h"
#include "main/tu031.h"

extern const DoublewordVector4 lightDebugBoxPointTable;
extern const DoublewordVector4 lightDebugBoxPointB;
extern const DoublewordVector4 lightDebugBoxPointC;
extern const DoublewordVector4 lightDebugBoxPointD;
extern const DoublewordVector4 lightDebugBoxPointE;
void xglMatrixStackUnit(void);
void xglMatrixStackTrans(const Vector4 *vector);
void xglMatrixStackRotX(float angle);
void xglMatrixStackRotY(float angle);
void xglMatrixStackRotZ(float angle);
void xglMatrixStackScale(const Vector4 *vector);
void xglMatrixStackMulVector(Vector4 *destination, const Vector4 *source);

void xglLightPackDebugBox(XglLightKey *key)
{
    int scaled[3];
    DoublewordVector4 debugBoxPointA;
    DoublewordVector4 debugBoxPointB;
    DoublewordVector4 debugBoxPointC;
    DoublewordVector4 debugBoxPointD;
    DoublewordVector4 debugBoxPointE;

    debugBoxPointA = lightDebugBoxPointTable;
    debugBoxPointB = lightDebugBoxPointB;
    debugBoxPointC = lightDebugBoxPointC;
    debugBoxPointD = lightDebugBoxPointD;
    debugBoxPointE = lightDebugBoxPointE;
    xglMatrixStackUnit();
    xglMatrixStackTrans(&key->position);
    xglMatrixStackRotZ(key->rotation.z);
    xglMatrixStackRotY(key->rotation.y);
    xglMatrixStackRotX(key->rotation.x);
    xglMatrixStackScale(&key->extent);
    xglMatrixStackMulVector(&debugBoxPointA.vector, &debugBoxPointA.vector);
    xglMatrixStackMulVector(&debugBoxPointB.vector, &debugBoxPointB.vector);
    xglMatrixStackMulVector(&debugBoxPointC.vector, &debugBoxPointC.vector);
    xglMatrixStackMulVector(&debugBoxPointD.vector, &debugBoxPointD.vector);
    xglMatrixStackMulVector(&debugBoxPointE.vector, &debugBoxPointE.vector);
}

void xglLightPackDebugEmpty(XglLightKey *key) {

}

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_031", func_00198E48);

void xglLightPackDebugMarker(XglLightKey *key)
{
    int scaled[3];
    DoublewordAlignedVector4 copy;

    copy = key->position;
    copy.w = 0.5f;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_031", func_00199030);

void xglLightPackDebugPoint(XglLightKey *key)
{
    int scaled[3];
    DoublewordAlignedVector4 copy;

    copy = key->position;
    copy.w = key->extent.x;
    copy.w = key->extent.y;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_031", func_00199160);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_031", func_001994F0);

/* Light pack as the light-packEntry loop reads it: lightEntryCount at 0x30, keyOffset at 0x3C, lightEntryByteOffsets at 0xF0. */
typedef struct XglLightPack {
    u8 unmodeled_00[0x30];
    int lightEntryCount;                 /* 0x30 */
    u8 unmodeled_34[0x8];
    int keyOffset;                  /* 0x3C */
    u8 unmodeled_40[0xB0];
    int lightEntryByteOffsets[1];            /* 0xF0: byte offsets of the entries, relative to the pack */
} XglLightPack;

/* One packEntry of a light pack: its entryType is read at 0x04. */
typedef struct XglLightEntry {
    u8 unmodeled_00[0x4];
    int entryType;                       /* 0x04 */
} XglLightEntry;

/* One of the 16 light slots at xglLightSlotTable: the pack registered in it (0 when free). */
typedef struct XglLightPackSlot {
    XglLightPack *pack;             /* 0x00 */
    u8 unmodeled_04[0xC];
} XglLightPackSlot;

extern XglLightPackSlot xglLightSlotTable[16];

void func_001994F0(XglLightKey *key);
void func_00199030(XglLightKey *key);
void xglLightPackDebugPoint(XglLightKey *key);
void func_00198E48(XglLightKey *key);
void func_00199160(XglLightKey *key);
void xglLightPackDebugMarker(XglLightKey *key);

void xglLightPackDebugDrawAll(void)
{
    int lightSlotIndex;
    int lightEntryIndex;
    XglLightKey *key;
    int *lightEntryByteOffsets;
    XglLightEntry *packEntry;

    for (lightSlotIndex = 0; lightSlotIndex < 16; lightSlotIndex++) {
        if (xglLightSlotTable[lightSlotIndex].pack != 0) {
            key = (XglLightKey *)((u8 *)xglLightSlotTable[lightSlotIndex].pack + xglLightSlotTable[lightSlotIndex].pack->keyOffset);
            lightEntryByteOffsets = xglLightSlotTable[lightSlotIndex].pack->lightEntryByteOffsets;
            for (lightEntryIndex = 0; lightEntryIndex < xglLightSlotTable[lightSlotIndex].pack->lightEntryCount; lightEntryIndex++) {
                packEntry = (XglLightEntry *)((u8 *)xglLightSlotTable[lightSlotIndex].pack + lightEntryByteOffsets[lightEntryIndex]);
                switch (packEntry->entryType - 1) {
                case 0:
                    xglLightPackDebugEmpty(key);
                    break;
                case 1:
                    func_001994F0(key);
                    break;
                case 7:
                case 9:
                    func_00199030(key);
                    break;
                case 3:
                case 4:
                case 20:
                case 21:
                    xglLightPackDebugBox(key);
                    break;
                case 5:
                case 22:
                    func_00198E48(key);
                    break;
                case 23:
                    func_00199160(key);
                    break;
                case 10:
                case 18:
                    xglLightPackDebugMarker(key);
                    break;
                case 2:
                case 19:
                case 34:
                    xglLightPackDebugPoint(key);
                    break;
                }
                key++;
            }
        }
    }
}

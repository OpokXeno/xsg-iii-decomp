#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu287", func_0026E750);

typedef unsigned char u8;
typedef int s32;
typedef float f32;

/* Position block of an object: the origin (0x10/0x14) and the local and world coordinates (0x20-0x2C). */
struct ObjectPosition {
    u8 pad_00[0x10];
    f32 originX;
    f32 originY;
    u8 pad_18[0x8];
    f32 localX;
    f32 localY;
    f32 worldX;
    f32 worldY;
};

void movieLayerClearActive(s32 *layerFlags) {
    *layerFlags &= ~1;
}

void objectPositionSetLocal(struct ObjectPosition *position, f32 x, f32 y) {
    position->localX = x;
    position->localY = y;
    position->worldX = x + position->originX;
    position->worldY = y + position->originY;
}

extern s32 gGameLoopRuntimeStorage[];
s32 func_0026EA98();
s32 func_00273B78();

void movieLayerUpdate(s32 *layerFlags) {
    s32 layerFlagWord;

    layerFlagWord = *layerFlags;
    if (layerFlagWord & 1) {
        if (!(gGameLoopRuntimeStorage[0] & 0x4000)) {
            func_0026EA98(layerFlags);
        }
        if (*layerFlags & 1) {
            func_00273B78(layerFlags);
        }
    }
}

s32 func_0026ECA8(s32);
s32 textDrawModeSet(s32 value);

void objectRestartWithValue(s32 value) {
    textDrawModeSet(0);
    func_0026ECA8(value);
}

void objectPositionGetOrigin(f32 *outX, f32 *outY, struct ObjectPosition *position) {
    *outX = position->originX;
    *outY = position->originY;
}

s32 func_00268938(f32 *pos, s32 placeMode, s32 id);
s32 func_00272CD8(s32);

void objectPlaceForIndex(f32 *pos, s32 placeMode, s32 placeIndex) {
    s32 objectId;

    textDrawModeSet(0);
    objectId = func_00272CD8(placeIndex);
    func_00268938(pos, placeMode, objectId);
    *pos += 32.0f;
}

INCLUDE_ASM("asm/main/nonmatchings/tu287", func_0026EA98);

INCLUDE_ASM("asm/main/nonmatchings/tu287", func_0026ECA8);

INCLUDE_ASM("asm/main/nonmatchings/tu287", func_0026EE58);

INCLUDE_ASM("asm/main/nonmatchings/tu287", func_0026EE90);

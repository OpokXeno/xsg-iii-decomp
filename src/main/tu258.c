#include "common.h"

typedef signed char s8;

typedef int s32;

typedef float f32;

typedef unsigned char u8;

typedef short s16;

/* Parameter slots of the scripted camera (gScriptedCameraStorage, gScriptedCamera): two 0xC-byte slots starting at +0xB0 of the camera entry. */
typedef struct ScriptedCameraParams {
    f32 paramValue;                      /* +0x00 */
    s8 byte4;                       /* +0x04 */
    s8 byte5;                       /* +0x05 */
    s8 byte6;                       /* +0x06 */
    u8 unk_07;
    f32 paramExtra;                      /* +0x08 */
} ScriptedCameraParams;

typedef struct ScriptedCameraParamView {
    u8 unk_00[0xB0];
    ScriptedCameraParams cameraParams;        /* +0xB0 */
} ScriptedCameraParamView;

/* The scripted camera entry (gScriptedCameraStorage, gScriptedCamera): one entry of the 0x28-entry camera table from cameraGetEntry. */
typedef struct ScriptedCamera {
    s8 activationMode;                        /* 0x00: 1..5, set by cameraScriptActivate */
    u8 unk_01[3];
    f32 radius;                     /* 0x04 */
    u8 unk_08[8];
    f32 vecA[3];                    /* 0x10 */
    u8 unk_1C[0x44];
    s8 valueAEnabled;                       /* 0x60 */
    s8 valueBEnabled;                       /* 0x61 */
    s8 valueCEnabled;                       /* 0x62 */
    u8 unk_63;
    f32 valueA;                     /* 0x64 */
    f32 valueB;                     /* 0x68 */
    f32 valueC;                     /* 0x6C */
    f32 vecB[3];                    /* 0x70 */
    u8 unk_7C[0x14];
    f32 level;                     /* 0x90 */
    s16 unk_94;                     /* 0x94 */
    s8 activeState;                      /* 0x96 */
} ScriptedCamera;

/* gScriptedCameraStorage is gScriptedCamera (the camera entry the script commands edit). */
extern ScriptedCamera *gScriptedCameraStorage;

void cameraScriptSetParamSlot(s32 cameraParamSlot, s32 byte4, s32 byte5, s32 byte6, f32 paramValue, f32 paramExtra) {
    f32 valueF;
    f32 extraF;
    s32 byteValue4;
    s32 byteValue5;
    s32 byteValue6;
    ScriptedCameraParamView *paramView;
    ScriptedCameraParams *cameraParams;

    valueF = paramValue;
    extraF = paramExtra;
    byteValue4 = byte4 & 0xFF;
    byteValue5 = byte5 & 0xFF;
    byteValue6 = byte6 & 0xFF;
    if ((gScriptedCameraStorage != 0) && (cameraParamSlot >= 0) && (cameraParamSlot < 2)) {
        paramView = (ScriptedCameraParamView *) &((s32 *) gScriptedCameraStorage)[cameraParamSlot * 3];   /* 3 words = one 0xC-byte slot record */
        cameraParams = &paramView->cameraParams;
        paramView->cameraParams.paramValue = valueF;
        if (byteValue4 != 0) {
            cameraParams->byte4 = byteValue4;
        }
        if (byteValue5 != 0) {
            cameraParams->byte5 = byteValue5;
        }
        if (extraF != 0.0f) {
            cameraParams->paramExtra = extraF;
        }
        if (byteValue6 != 0) {
            cameraParams->byte6 = byteValue6;
        }
    }
}

void *cameraGetStateBlock();

void cameraSlotSetState(s32 cameraSlotIndex, s32 cameraSlotValue) {
    s8 *cameraState;

    if (cameraSlotIndex >= 0) {
        if (cameraSlotIndex < 9) {
            cameraState = cameraGetStateBlock();
            cameraState[0] = cameraSlotIndex;
            cameraState[1] = cameraSlotValue;
        }
    }
}

void *func_0025F9E8(s32, s32);

void cameraSlotSetStateById(s32 cameraStateKind, s32 id, s32 cameraSlotIndex, s32 cameraSlotValue) {
    s8 *cameraState;

    if (cameraSlotIndex >= 0) {
        if (cameraSlotIndex < 9) {
            cameraState = func_0025F9E8(cameraStateKind, id);
            if (cameraState != 0) {
                cameraState[0] = cameraSlotIndex;
                cameraState[1] = cameraSlotValue;
            }
        }
    }
}



void *cameraGetEntry(s32 index);

s32 func_00264280(void *object);

extern unsigned char gObjectPendingFlag[];

void cameraScriptActivate(s32 cameraEntryIndex, s32 activationMode) {
    ScriptedCamera *scriptedCamera;

    scriptedCamera = cameraGetEntry(cameraEntryIndex);
    gScriptedCameraStorage = scriptedCamera;
    if (scriptedCamera != 0) {
        func_00264280(scriptedCamera);
        if (activationMode > 0) {
            if (activationMode < 6) {
                gScriptedCameraStorage->activationMode = activationMode;
            }
        }
    }
}

void cameraScriptClear(void) {
    gScriptedCameraStorage = 0;
}

void objectSetScriptIndex(s32 scriptObjectIndex) {
    if (scriptObjectIndex >= 0) {
        if (scriptObjectIndex < 0x1F) {
            gScriptedCameraStorage->unk_94 = scriptObjectIndex;
        }
    }
}

void objectSetScriptCount(s32 scriptObjectCount) {
    if ((gScriptedCameraStorage != 0) && (scriptObjectCount > 0)) {
        gScriptedCameraStorage->unk_94 = scriptObjectCount;
    }
}

void objectSetVecA(f32 x, f32 y, f32 z) {
    if (gScriptedCameraStorage != 0) {
        gScriptedCameraStorage->vecA[0] = x;
        gScriptedCameraStorage->vecA[1] = y;
        gScriptedCameraStorage->vecA[2] = z;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu258", func_002640E0);

INCLUDE_ASM("asm/main/nonmatchings/tu258", func_00264110);

void cameraScriptSetLevel(f32 level) {
    if (gScriptedCameraStorage != 0) {
        gScriptedCameraStorage->level = level;
    }
}

void objectEnableValueA(f32 newValueA) {
    if (gScriptedCameraStorage != 0) {
        gScriptedCameraStorage->valueAEnabled = 1;
        gScriptedCameraStorage->valueA = newValueA;
    }
}

void objectEnableValueB(f32 newValueB) {
    if (gScriptedCameraStorage != 0) {
        gScriptedCameraStorage->valueBEnabled = 1;
        gScriptedCameraStorage->valueB = newValueB;
    }
}

void objectEnableValueC(f32 newValueC) {
    if (gScriptedCameraStorage != 0) {
        gScriptedCameraStorage->valueCEnabled = 1;
        gScriptedCameraStorage->valueC = newValueC;
    }
}

void objectSetVecB(f32 x, f32 y, f32 z) {
    if (gScriptedCameraStorage != 0) {
        gScriptedCameraStorage->vecB[0] = x;
        gScriptedCameraStorage->vecB[1] = y;
        gScriptedCameraStorage->vecB[2] = z;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu258", func_002641E8);

INCLUDE_ASM("asm/main/nonmatchings/tu258", func_00264220);

void cameraScriptSetRadius(f32 radius) {
    if (gScriptedCameraStorage != 0) {
        gScriptedCameraStorage->radius = radius;
    }
}

void objectSetActiveState(s32 activeState) {
    if (gScriptedCameraStorage != 0) {
        gScriptedCameraStorage->activeState = activeState;
    }
}

void objectMarkPending(void) {
    gObjectPendingFlag[0] = 0xFF;
}

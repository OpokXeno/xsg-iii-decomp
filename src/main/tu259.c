#include "common.h"

typedef signed char s8;
typedef int s32;
typedef short s16;
typedef float f32;
typedef unsigned char u8;

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

extern ScriptedCamera *gScriptedCameraStorage;
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

INCLUDE_ASM("asm/main/nonmatchings/tu259", func_002640E0);

INCLUDE_ASM("asm/main/nonmatchings/tu259", func_00264110);

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

INCLUDE_ASM("asm/main/nonmatchings/tu259", func_002641E8);

INCLUDE_ASM("asm/main/nonmatchings/tu259", func_00264220);

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

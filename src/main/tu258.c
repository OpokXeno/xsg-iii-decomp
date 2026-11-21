#include "common.h"

typedef signed char s8;
typedef int s32;
typedef float f32;
typedef unsigned char u8;

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

/* gScriptedCameraStorage is gScriptedCamera (the camera entry the script commands edit). */
extern s32 *gScriptedCameraStorage;

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
        paramView = (ScriptedCameraParamView *) &gScriptedCameraStorage[cameraParamSlot * 3];   /* 3 words = one 0xC-byte slot record */
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

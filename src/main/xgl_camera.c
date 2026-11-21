#include "common.h"
#include "main/xgl_2.h"

typedef int s32;
typedef float f32;

void xglCameraResetState(XglCamera *camera)
{
  camera->padPort = 1;
  camera->cameraState = 0;
  camera->active = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_camera", xglCameraScreenInit);

INCLUDE_ASM("asm/main/nonmatchings/xgl_camera", func_00193960);

INCLUDE_ASM("asm/main/nonmatchings/xgl_camera", func_00193A40);

INCLUDE_ASM("asm/main/nonmatchings/xgl_camera", func_00193A78);

INCLUDE_ASM("asm/main/nonmatchings/xgl_camera", func_00193BD0);

INCLUDE_ASM("asm/main/nonmatchings/xgl_camera", func_00193D90);

INCLUDE_ASM("asm/main/nonmatchings/xgl_camera", func_001941C0);

void xglCameraSetPadPort(XglCamera *camera, s32 port) {
    camera->padPort = port;
    if (port >= 2) {
        camera->padPort = 1;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_camera", func_001947F0);

INCLUDE_ASM("asm/main/nonmatchings/xgl_camera", func_001948E0);

INCLUDE_ASM("asm/main/nonmatchings/xgl_camera", xglCameraViewTravel);

INCLUDE_ASM("asm/main/nonmatchings/xgl_camera", xglCameraViewVolume);

void xglCameraResetState(XglCamera *camera);
s32 func_00193960(void *);
s32 xglCameraScreenInit(void *);

void xglCameraInit(XglCamera *camera) {
    xglCameraResetState(camera);
    xglCameraScreenInit(camera);
    func_00193960(camera);
    camera->defaultScale = 1.0f;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_camera", xglCameraMove);

s32 func_001948E0();
s32 xglCameraViewTravel(s32, s32);
s32 xglCameraViewVolume(s32, s32);

void xglCameraMoveOffset(s32 camera, s32 offsetVector) {
    func_001948E0();
    xglCameraViewTravel(camera, offsetVector);
    xglCameraViewVolume(camera, offsetVector);
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_camera", func_00195050);

extern f32 gCameraDefaultNearClip;
extern f32 gCameraDefaultFarClip;

void xglCameraResetClipRange(XglCamera *camera) {
    f32 nearClip;
    f32 farClip;

    nearClip = gCameraDefaultNearClip;
    farClip = gCameraDefaultFarClip;
    camera->screen.clipRange.vector.y = farClip;
    camera->screen.clipRange.vector.x = nearClip;
}

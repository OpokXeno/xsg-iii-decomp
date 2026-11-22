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

#include "quadword.h"

extern Vector4 gCameraDefaultRotation;
extern Vector4 gCameraDefaultAngleVelocity;
extern Vector4 gCameraDefaultAngleDelta;
extern Vector4 gCameraDefaultPosition;
extern Vector4 gCameraDefaultPlaceVelocity;
extern Vector4 gCameraDefaultPlaceDelta;
extern Vector4 gCameraDefaultChaseReference;
extern Vector4 gCameraDefaultChasePosition;

/* Fills the CameraTravel block at camera+0x90 with the default travel view. */
void xglCameraTravelDefaultInit(XglCamera *camera) {
    CameraTravel *travel = &camera->travel;

    travel->focus = 5.0f;
    travel->fov = 0.69813174f;
    travel->unknown_78 = 0;
    travel->fov_velocity = 0.0f;
    travel->scale = 0.0f;
    QuadwordCopy(&travel->rotation, &gCameraDefaultRotation);
    QuadwordCopy(&travel->angle_velocity, &gCameraDefaultAngleVelocity);
    QuadwordCopy(&travel->angle_delta, &gCameraDefaultAngleDelta);
    QuadwordCopy(&travel->position, &gCameraDefaultPosition);
    QuadwordCopy(&travel->place_velocity, &gCameraDefaultPlaceVelocity);
    QuadwordCopy(&travel->place_delta, &gCameraDefaultPlaceDelta);
    travel->chase_yaw = travel->rotation.y;
    travel->chase_distance = travel->position.z;
    QuadwordCopy(&travel->chase_reference, &gCameraDefaultChaseReference);
    QuadwordCopy(&travel->chase_position, &gCameraDefaultChasePosition);
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_camera", func_00193A40);

INCLUDE_ASM("asm/main/nonmatchings/xgl_camera", func_00193A78);

INCLUDE_ASM("asm/main/nonmatchings/xgl_camera", func_00193BD0);

#include "vu0_macro.h"
#include "main/tu007.h"

typedef struct CameraPadOverride {
    unsigned short held;
    unsigned short pressed;
    unsigned char unmodeled_04[4];
    signed char rightX;
    signed char rightY;
    signed char leftX;
    signed char leftY;
    unsigned int active;
} CameraPadOverride;

extern PadRecord PadData[2];
extern CameraPadOverride sdvSpecialBuf;
void func_00193A40(XglCamera *camera);
void func_00193A78(XglCamera *camera);
void func_00193BD0(XglCamera *camera);
void xglVectorScaleXYZ(Vector4 *destination, const Vector4 *source, float scale);
void xglVectorScaleAddXYZ(Vector4 *destination, const Vector4 *source, float scale, const Vector4 *addend);
void xglMatrixStackUnit(void);
void xglMatrixStackRotY(float angle);
void xglMatrixStackSave(Matrix4 matrix);

/* Pad update, free (manual) travel mode (camera state 2). */
void xglCameraPadUpdateFree(XglCamera *camera) {
    CameraTravel *travel = &camera->travel;
    Matrix axes;
    short rightX = PadData[camera->padPort].rightX;
    short leftX = PadData[camera->padPort].leftX;
    short rightY = PadData[camera->padPort].rightY;
    short leftY = PadData[camera->padPort].leftY;
    unsigned char deadZone[4];
    unsigned short held;

    held = PadData[camera->padPort].held | PadData[1].held;
    deadZone[0] = PadData[camera->padPort].axisDeadZone[0];
    deadZone[1] = PadData[camera->padPort].axisDeadZone[1];
    deadZone[2] = PadData[camera->padPort].axisDeadZone[2];
    deadZone[3] = PadData[camera->padPort].axisDeadZone[3];
    if (sdvSpecialBuf.active) {
        held = sdvSpecialBuf.held;
        rightX = sdvSpecialBuf.rightX;
        leftX = sdvSpecialBuf.leftX;
        rightY = sdvSpecialBuf.rightY;
        leftY = sdvSpecialBuf.leftY;
        deadZone[0] = PadData[0].axisDeadZone[0];
        deadZone[1] = PadData[0].axisDeadZone[1];
        deadZone[2] = PadData[0].axisDeadZone[2];
        deadZone[3] = PadData[0].axisDeadZone[3];
    }
    /* The second pad takes over when its sticks are deflected more than the first pad's (or the override's). */
    if (((rightX < 0) ? -rightX : rightX) + ((leftX < 0) ? -leftX : leftX)
            + ((rightY < 0) ? -rightY : rightY) + ((leftY < 0) ? -leftY : leftY)
        < (((PadData[1].rightX < 0) ? -PadData[1].rightX : PadData[1].rightX)
           + ((PadData[1].leftX < 0) ? -PadData[1].leftX : PadData[1].leftX)
           + ((PadData[1].rightY < 0) ? -PadData[1].rightY : PadData[1].rightY)
           + ((PadData[1].leftY < 0) ? -PadData[1].leftY : PadData[1].leftY))) {
        rightX = PadData[1].rightX;
        leftX = PadData[1].leftX;
        rightY = PadData[1].rightY;
        leftY = PadData[1].leftY;
        deadZone[0] = PadData[1].axisDeadZone[0];
        deadZone[1] = PadData[1].axisDeadZone[1];
        deadZone[2] = PadData[1].axisDeadZone[2];
        deadZone[3] = PadData[1].axisDeadZone[3];
    }

    func_00193A40(camera);
    func_00193A78(camera);
    if (rightY < 0) {
        travel->angle_delta.x = (rightY + deadZone[1]) * -6.981317e-05f;
    }
    if (rightY > 0) {
        travel->angle_delta.x = (rightY - deadZone[1]) * -6.981317e-05f;
    }
    if (rightX < 0) {
        travel->angle_delta.y = (rightX + deadZone[0]) * -6.981317e-05f;
    }
    if (rightX > 0) {
        travel->angle_delta.y = (rightX - deadZone[0]) * -6.981317e-05f;
    }
    if (held & 0x8000) {
        travel->angle_delta.z = 0.0022340214f;
    }
    if (held & 0x2000) {
        travel->angle_delta.z = -0.0022340214f;
    }
    if (leftX < 0) {
        travel->place_delta.x = (leftX + deadZone[2]) * 0.0002f;
    }
    if (leftX > 0) {
        travel->place_delta.x = (leftX - deadZone[2]) * 0.0002f;
    }
    if (held & 0x1) {
        travel->place_delta.y = -0.0064f;
    }
    if (held & 0x4) {
        travel->place_delta.y = 0.0064f;
    }
    if (leftY < 0) {
        travel->place_delta.z = (leftY + deadZone[3]) * 0.0002f;
    }
    if (leftY > 0) {
        travel->place_delta.z = (leftY - deadZone[3]) * 0.0002f;
    }

    func_00193BD0(camera);
    travel->fov_velocity = travel->fov_velocity * 0.8f + travel->scale;
    travel->fov = travel->fov + travel->fov_velocity;
    xglVectorScaleXYZ(&travel->angle_velocity, &travel->angle_velocity, 0.8f);
    xglVectorScaleXYZ(&travel->place_velocity, &travel->place_velocity, 0.8f);
    Vu0AddVectorXYZ(&travel->angle_velocity, &travel->angle_velocity, &travel->angle_delta);
    xglMatrixStackUnit();
    xglMatrixStackRotY(travel->rotation.y);
    xglMatrixStackSave(axes.rows);
    xglVectorScaleAddXYZ(&travel->place_velocity, &axes.vectors[0], travel->place_delta.x, &travel->place_velocity);
    travel->place_velocity.y = travel->place_velocity.y + travel->place_delta.y;
    xglVectorScaleAddXYZ(&travel->place_velocity, &axes.vectors[2], travel->place_delta.z, &travel->place_velocity);
    Vu0AddVectorXYZ(&travel->rotation, &travel->rotation, &travel->angle_velocity);
    Vu0AddVectorXYZ(&travel->position, &travel->position, &travel->place_velocity);
}

float xglSin(float angle);
float xglCos(float angle);
float xglPointLengthXZ(const Vector4 *first, const Vector4 *second);
void xglVectorLength(float *destination, const Vector4 *source);
void xglVectorNormal(Vector4 *destination, const Vector4 *source);
extern float atan2f(float y, float x);

/* Pad button bits of PadRecord.held and pressed that the chase camera reads. */
#define CAMERA_BTN_DOWN 0x0001
#define CAMERA_BTN_UP 0x0004
#define CAMERA_BTN_ORBIT 0x0040
#define CAMERA_BTN_MOVE_FOCUS 0x0080
#define CAMERA_BTN_LOCK 0x0800

extern float gCameraHalfPi;

#define STICK_SCALE 0.0002f /* stick units past the dead zone to camera units */
#define PAN_STEP 0.032f     /* height step per frame, as a fraction of the distance to the focus */
#define ABS(x) ((x) < 0 ? -(x) : (x))

/* Pad update, chase travel mode (camera state 3). */
void xglCameraPadUpdateChase(XglCamera *camera) {
    CameraTravel *travel = &camera->travel;
    Vector4 lookDirection;
    Vector4 forward;
    float forwardLength; /* unused by the original after xglVectorLength */
    float distance;
    float pitchScale = 1.0f;
    short rightX = PadData[camera->padPort].rightX;
    short leftX = PadData[camera->padPort].leftX;
    short rightY = PadData[camera->padPort].rightY;
    short leftY = PadData[camera->padPort].leftY;
    unsigned char deadZone[4];
    unsigned short held;
    unsigned short pressed;
    float yaw;
    float angle;

    held = PadData[camera->padPort].held | PadData[1].held;
    pressed = PadData[camera->padPort].pressed | PadData[1].pressed;
    deadZone[0] = PadData[camera->padPort].axisDeadZone[0];
    deadZone[1] = PadData[camera->padPort].axisDeadZone[1];
    deadZone[2] = PadData[camera->padPort].axisDeadZone[2];
    deadZone[3] = PadData[camera->padPort].axisDeadZone[3];
    if (sdvSpecialBuf.active) {
        held = sdvSpecialBuf.held;
        pressed = sdvSpecialBuf.pressed;
        rightX = sdvSpecialBuf.rightX;
        leftX = sdvSpecialBuf.leftX;
        rightY = sdvSpecialBuf.rightY;
        leftY = sdvSpecialBuf.leftY;
        deadZone[0] = PadData[0].axisDeadZone[0];
        deadZone[1] = PadData[0].axisDeadZone[1];
        deadZone[2] = PadData[0].axisDeadZone[2];
        deadZone[3] = PadData[0].axisDeadZone[3];
    }
    /* The second pad takes over when its sticks are deflected more than the first pad's (or the override's). */
    if (ABS(rightX) + ABS(leftX) + ABS(rightY) + ABS(leftY)
        < ABS(PadData[1].rightX) + ABS(PadData[1].leftX) + ABS(PadData[1].rightY) + ABS(PadData[1].leftY)) {
        rightX = PadData[1].rightX;
        leftX = PadData[1].leftX;
        rightY = PadData[1].rightY;
        leftY = PadData[1].leftY;
        deadZone[0] = PadData[1].axisDeadZone[0];
        deadZone[1] = PadData[1].axisDeadZone[1];
        deadZone[2] = PadData[1].axisDeadZone[2];
        deadZone[3] = PadData[1].axisDeadZone[3];
    }

    func_00193A40(camera);
    func_00193A78(camera);
    Vu0SubVectorXYZ(&forward, &travel->chase_position, &travel->position);
    xglVectorLength(&forwardLength, &forward);
    xglVectorNormal(&forward, &forward);
    distance = xglPointLengthXZ(&travel->chase_position, &travel->position);

    if (pressed & CAMERA_BTN_LOCK) {
        if (camera->lookAtLocked) {
            camera->lookAtLocked = 0;
        } else {
            camera->lookAtLocked = 1;
        }
    }
    if ((held & CAMERA_BTN_MOVE_FOCUS) || (held & CAMERA_BTN_ORBIT)) {
        if (held & CAMERA_BTN_DOWN) {
            travel->chase_position.y += distance * -PAN_STEP;
        }
        if (held & CAMERA_BTN_UP) {
            travel->chase_position.y += distance * PAN_STEP;
        }
    }
    if (!(held & CAMERA_BTN_ORBIT)) {
        if (held & CAMERA_BTN_DOWN) {
            travel->position.y += distance * -PAN_STEP;
        }
        if (held & CAMERA_BTN_UP) {
            travel->position.y += distance * PAN_STEP;
        }
    }
    if (leftX < 0) {
        travel->place_delta.x = (leftX + deadZone[2]) * STICK_SCALE;
    }
    if (leftX > 0) {
        travel->place_delta.x = (leftX - deadZone[2]) * STICK_SCALE;
    }
    if (travel->place_delta.x != 0.0f) {
        if ((held & CAMERA_BTN_MOVE_FOCUS) && !(held & CAMERA_BTN_ORBIT)) {
            float halfPi;
            yaw = atan2f(forward.x, forward.z);
            halfPi = gCameraHalfPi;
            travel->position.x += xglSin(yaw - halfPi) * travel->place_delta.x * distance;
            travel->position.z += xglCos(yaw - halfPi) * travel->place_delta.x * distance;
            travel->chase_position.x += xglSin(yaw - halfPi) * travel->place_delta.x * distance;
            travel->chase_position.z += xglCos(yaw - halfPi) * travel->place_delta.x * distance;
        }
        if (!(held & CAMERA_BTN_ORBIT) && !(held & CAMERA_BTN_MOVE_FOCUS)) {
            angle = atan2f(forward.x, forward.z);
            angle = angle + travel->place_delta.x;
            travel->position.x = travel->chase_position.x - xglSin(angle) * distance;
            travel->position.z = travel->chase_position.z - xglCos(angle) * distance;
        }
        if ((held & CAMERA_BTN_ORBIT) && !(held & CAMERA_BTN_MOVE_FOCUS)) {
            angle = atan2f(forward.x, forward.z);
            angle = angle - travel->place_delta.x;
            travel->chase_position.x = travel->position.x + xglSin(angle) * distance;
            travel->chase_position.z = travel->position.z + xglCos(angle) * distance;
        }
    }
    if (leftY < 0) {
        travel->place_delta.z = (leftY + deadZone[3]) * STICK_SCALE;
    }
    if (leftY > 0) {
        travel->place_delta.z = (leftY - deadZone[3]) * STICK_SCALE;
    }
    if ((held & CAMERA_BTN_MOVE_FOCUS) || (held & CAMERA_BTN_ORBIT)) {
        travel->chase_position.x -= forward.x * travel->place_delta.z * distance * pitchScale;
        travel->chase_position.y -= forward.y * travel->place_delta.z * distance * pitchScale;
        travel->chase_position.z -= forward.z * travel->place_delta.z * distance * pitchScale;
    }
    if (!(held & CAMERA_BTN_ORBIT)) {
        travel->position.x -= forward.x * travel->place_delta.z * distance * pitchScale;
        travel->position.y -= forward.y * travel->place_delta.z * distance * pitchScale;
        travel->position.z -= forward.z * travel->place_delta.z * distance * pitchScale;
    }

    Vu0SubVectorXYZ(&lookDirection, &travel->chase_position, &travel->position);
    xglVectorNormal(&lookDirection, &lookDirection);
    lookDirection.z = -lookDirection.z;
    travel->rotation.y = atan2f(lookDirection.x, lookDirection.z);
    travel->rotation.x = atan2f(lookDirection.y, lookDirection.x * xglSin(travel->rotation.y) + lookDirection.z * xglCos(travel->rotation.y));
    travel->rotation.y = -travel->rotation.y;
}

void xglCameraSetPadPort(XglCamera *camera, s32 port) {
    camera->padPort = port;
    if (port >= 2) {
        camera->padPort = 1;
    }
}

void sdvInitSpecialWork(void);

INCLUDE_ASM("asm/main/nonmatchings/xgl_camera", func_001947F0);

INCLUDE_ASM("asm/main/nonmatchings/xgl_camera", func_001948E0);

INCLUDE_ASM("asm/main/nonmatchings/xgl_camera", xglCameraViewTravel);

INCLUDE_ASM("asm/main/nonmatchings/xgl_camera", xglCameraViewVolume);

void xglCameraResetState(XglCamera *camera);
/* Fills the CameraTravel block at camera+0x90 with the default travel view. */
void xglCameraTravelDefaultInit(XglCamera *camera);
s32 xglCameraScreenInit(void *);

void func_001948E0();
void xglCameraViewTravel(XglCamera *camera, Vector4 *cameraShift);
void xglCameraViewVolume(XglCamera *camera, Vector4 *cameraShift);

void xglCameraInit(XglCamera *camera) {
    xglCameraResetState(camera);
    xglCameraScreenInit(camera);
    xglCameraTravelDefaultInit(camera);
    camera->defaultScale = 1.0f;
}

void xglCameraMove(XglCamera *camera) {
    Vector4 cameraShift;

    cameraShift = (Vector4){0, 0, 0, 0};
    func_001948E0(camera, &cameraShift);
    xglCameraViewTravel(camera, &cameraShift);
    xglCameraViewVolume(camera, &cameraShift);
}

void xglCameraMoveOffset(XglCamera *camera, Vector4 *cameraShift) {
    func_001948E0();
    xglCameraViewTravel(camera, cameraShift);
    xglCameraViewVolume(camera, cameraShift);
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

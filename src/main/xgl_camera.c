#include "common.h"
#include "shared.h"
#include "main/sdv.h"
#include "main/xgl_2.h"
#include "main/xgl_render.h"
#include "quadword.h"
#include "vu0_macro.h"
#include "main/tu007.h"

void sdvInitSpecialWork(void);

void *memset(void *destination, int value, unsigned int count);

typedef int s32;

extern s32 D_005511EC;

void sdvStoreValue(s32 storedValue) {
    D_005511EC = storedValue;
}

void sdvInitSpecialWork(void)
{
    memset(&sdvSpecialBuf, 0, sizeof(sdvSpecialBuf));
}

/* Sets the camera pad override: the buttons and the four stick axes the camera pad updates read
 * instead of the pads; active is stored last. */
void func_00193850(int held, int pressed, int repeated, int rightX, int rightY, int leftX, int leftY)
{
    sdvSpecialBuf.held = held;
    sdvSpecialBuf.pressed = pressed;
    sdvSpecialBuf.repeated = repeated;
    sdvSpecialBuf.rightX = rightX;
    sdvSpecialBuf.rightY = rightY;
    sdvSpecialBuf.leftX = leftX;
    sdvSpecialBuf.leftY = leftY;
    sdvSpecialBuf.active = 1;
}

typedef float f32;

void xglCameraResetState(XglCamera *camera)
{
  camera->padPort = 1;
  camera->cameraState = 0;
  camera->active = 0;
}

extern Vector4 gCameraDefaultClipRange;

extern Vector4 gCameraDefaultPortEffect;

/* The render state; the camera screen takes its frame size. */
extern XglRenderStateBlock D_004C8390;

/* Fills the XglCameraScreen block at camera+0x10 with the default screen and the frame size. */
void xglCameraScreenInit(XglCamera *camera)
{
    XglCameraScreen *screen = &camera->screen;

    screen->depthMax = 0x00FFFFFF;
    screen->depthMin = 0;
    QuadwordCopy(&screen->clipRange.vector, &gCameraDefaultClipRange);
    QuadwordCopy(&screen->portEffect, &gCameraDefaultPortEffect);
    screen->resolution.vector.x = D_004C8390.frame_width;
    screen->resolution.vector.y = D_004C8390.frame_height;
    screen->resolution.vector.z = D_004C8390.frame_width;
    screen->resolution.vector.w = D_004C8390.frame_height;
    screen->window.vector.x = 0.0f;
    screen->window.vector.y = 0.0f;
    screen->window.vector.z = D_004C8390.frame_width;
    screen->window.vector.w = D_004C8390.frame_height;
}

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
    travel->fast_travel_frames = 0;
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

extern Vector4 gCameraResetAngleDelta;

extern Vector4 gCameraResetPlaceDelta;

/* Clears the travel scale and resets the angle and place deltas. */
void func_00193A40(XglCamera *camera)
{
    camera->travel.scale = 0.0f;
    QuadwordCopy(&camera->travel.angle_delta, &gCameraResetAngleDelta);
    QuadwordCopy(&camera->travel.place_delta, &gCameraResetPlaceDelta);
}

extern PadRecord PadData[2];

/* The buttons of the focus and scale update (PS2 pad bits: R2, L1, R1, circle, square). */
#define CAMERA_FOCUS_BTN_SCALE_DOWN 0x0002
#define CAMERA_FOCUS_BTN_SHIFTS 0x000E
#define CAMERA_FOCUS_BTN_SCALE_UP 0x0008
#define CAMERA_FOCUS_BTN_IN 0x0020
#define CAMERA_FOCUS_BTN_OUT 0x0080
/* From 10 on the focus steps while the button is held, below it once per press. */
#define CAMERA_FOCUS_HELD_MIN 10.0f
#define CAMERA_FOCUS_MAX 99.0f
#define CAMERA_FOCUS_MIN 1.0f
#define CAMERA_FOCUS_STEP 1.0f
/* The travel scale the scale buttons set (in radians per frame). */
#define CAMERA_SCALE_STEP 0.0011170107f

/* Pad update of the travel focus and scale: steps the focus between 1 and 99 (unless one
 * of R2/L1/R1 is held or sdv holds the camera) and sets the scale from R1 or R2. With the
 * sdv special work active its buttons replace the pads'. */
void func_00193A78(XglCamera *camera)
{
    CameraTravel *travel = &camera->travel;
    unsigned short held;
    unsigned short pressed;

    held = PadData[1].held | PadData[camera->padPort].held;
    pressed = PadData[1].pressed | PadData[camera->padPort].pressed;
    if (sdvSpecialBuf.active) {
        held = sdvSpecialBuf.held;
        pressed = sdvSpecialBuf.pressed;
    }
    if (!(held & CAMERA_FOCUS_BTN_SHIFTS) && D_005511EC == 0) {
        if (travel->focus >= CAMERA_FOCUS_HELD_MIN) {
            if ((held & CAMERA_FOCUS_BTN_IN) && travel->focus < CAMERA_FOCUS_MAX) {
                travel->focus += CAMERA_FOCUS_STEP;
            }
            if ((held & CAMERA_FOCUS_BTN_OUT) && travel->focus > CAMERA_FOCUS_MIN) {
                travel->focus -= CAMERA_FOCUS_STEP;
            }
        } else {
            if ((pressed & CAMERA_FOCUS_BTN_IN) && travel->focus < CAMERA_FOCUS_MAX) {
                travel->focus += CAMERA_FOCUS_STEP;
            }
            if ((pressed & CAMERA_FOCUS_BTN_OUT) && travel->focus > CAMERA_FOCUS_MIN) {
                travel->focus -= CAMERA_FOCUS_STEP;
            }
        }
    }
    if (held & CAMERA_FOCUS_BTN_SCALE_UP) {
        travel->scale = CAMERA_SCALE_STEP;
    }
    if (held & CAMERA_FOCUS_BTN_SCALE_DOWN) {
        travel->scale = -CAMERA_SCALE_STEP;
    }
}

/* Cross (PS2 pad bit 0x40): the travel runs four times as fast. */
#define CAMERA_TRAVEL_BTN_FAST 0x0040
/* From focus 10 on the angle delta keeps the factor of focus 9 (episode II's largest focus). */
#define CAMERA_TRAVEL_ANGLE_FOCUS_MAX 9.0f

void xglVectorScaleXYZ(Vector4 *destination, const Vector4 *source, float scale);

/* Scales the travel scale and deltas by focus-dependent factors, x4 while cross is held. From focus 10 on the
 * angle delta keeps the factor of focus 9 and the place delta uses ten times the focus. */
void func_00193BD0(XglCamera *camera)
{
    CameraTravel *travel = &camera->travel;
    unsigned short held;

    held = PadData[1].held | PadData[camera->padPort].held;
    if (sdvSpecialBuf.active) {
        held = sdvSpecialBuf.held;
    }
    if (held & CAMERA_TRAVEL_BTN_FAST) {
        travel->scale = travel->scale * 4.0f;
        xglVectorScaleXYZ(&travel->angle_delta, &travel->angle_delta, 4.0f);
        xglVectorScaleXYZ(&travel->place_delta, &travel->place_delta, 4.0f);
        travel->fast_travel_frames = travel->fast_travel_frames + 1;
    } else {
        travel->fast_travel_frames = 0;
    }
    travel->scale = travel->scale * ((travel->focus - 5.0f) * 0.08f + 1.0f);
    if (travel->focus >= 10.0f) {
        xglVectorScaleXYZ(&travel->angle_delta, &travel->angle_delta,
                          (CAMERA_TRAVEL_ANGLE_FOCUS_MAX - 5.0f) * 0.08f + 1.0f);
        xglVectorScaleXYZ(&travel->place_delta, &travel->place_delta,
                          (travel->focus * 10.0f - 5.0f) * 0.24f + 1.0f + travel->fast_travel_frames * 0.01f);
    } else {
        xglVectorScaleXYZ(&travel->place_delta, &travel->place_delta,
                          (travel->focus - 5.0f) * 0.24f + 1.0f + travel->fast_travel_frames * 0.01f);
        xglVectorScaleXYZ(&travel->angle_delta, &travel->angle_delta, (travel->focus - 5.0f) * 0.08f + 1.0f);
    }
}

void func_00193A40(XglCamera *camera);

void func_00193A78(XglCamera *camera);

void func_00193BD0(XglCamera *camera);

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

#define PAD_TRIANGLE 0x0010
#define PAD_SELECT 0x0100

/* XglCamera.cameraState values the pad update dispatches on. */
#define CAMERA_STATE_RESET 0  /* xglCameraResetState: no pad update, the sdv special work is left alone */
#define CAMERA_STATE_STUDIO 1 /* set by xglStudioMainCameraInit */
#define CAMERA_STATE_FREE 2   /* xglCameraPadUpdateFree */
#define CAMERA_STATE_CHASE 3  /* xglCameraPadUpdateChase */
#define CAMERA_STATE_GAME 4   /* XS2 name; set by sb_natives.c, tu237.c and ov02 tu097.c */

/* Pad update of a camera: the free and the chase state run their update unless SELECT is held; SELECT
 * with TRIANGLE pressed puts the travel back to its default. The pad is the camera's port ORed with port 1,
 * or the sdv special work while it is active. */
void func_001947F0(XglCamera *camera)
{
    unsigned short held;
    unsigned short pressed;

    held = PadData[1].held | PadData[camera->padPort].held;
    pressed = PadData[1].pressed | PadData[camera->padPort].pressed;
    if (sdvSpecialBuf.active) {
        held = sdvSpecialBuf.held;
        pressed = sdvSpecialBuf.pressed;
    }
    switch (camera->cameraState) {
    case CAMERA_STATE_RESET:
        return;
    case CAMERA_STATE_STUDIO:
        break;
    case CAMERA_STATE_FREE:
        if (!(held & PAD_SELECT)) {
            xglCameraPadUpdateFree(camera);
        } else if (pressed & PAD_TRIANGLE) {
            xglCameraTravelDefaultInit(camera);
        }
        break;
    case CAMERA_STATE_CHASE:
        if (held & PAD_SELECT) {
            if (pressed & PAD_TRIANGLE) {
                xglCameraTravelDefaultInit(camera);
            }
        } else {
            xglCameraPadUpdateChase(camera);
        }
        break;
    case CAMERA_STATE_GAME:
        break;
    }
    sdvInitSpecialWork();
}

void xglVectorScale(Vector4 *scaledVector, const Vector4 *vector, float scale);

/* The GS primitive coordinate range (12.4 fixed point) and its centre. */
#define CAMERA_GS_COORD_SIZE 4096.0f
#define CAMERA_GS_COORD_CENTER 2048.0f
/* The horizontal projection scale. XS2 computes (135 / 11) / (54 / (MAGH + 1)) from DISPLAY1; XS3 keeps the
 * value for MAGH 4. */
#define CAMERA_PROJECTION_SCALE (25.0f / 22.0f)

/* The depth range of the camera screen as floats: its minimum and its extent. */
typedef struct CameraDepthRange {
    float minimum;
    float range;
} CameraDepthRange;

/* Derives the screen offset and scale, the clip range and the projection from the window, resolution and
 * port effect of the camera screen. */
void xglCameraViewScreen(XglCamera *camera, const Vector4 *offset)
{
    XglCameraScreen *screen = &camera->screen;
    float scale;
    CameraDepthRange depth;
    Vector4 scaled;
    float offsetX;
    float offsetY;

    offsetX = (screen->window.vector.z + screen->window.vector.x) * 0.5f - screen->resolution.vector.x * 0.5f;
    offsetY = (screen->window.vector.w + screen->window.vector.y) * 0.5f - screen->resolution.vector.y * 0.5f;
    scale = CAMERA_PROJECTION_SCALE;
    depth.minimum = (float) screen->depthMin;
    depth.range = (float) (screen->depthMax - screen->depthMin);
    if (screen->resolution.vector.z > screen->resolution.vector.w) {
        xglVectorScale(&scaled, &screen->resolution.vector,
                       (CAMERA_GS_COORD_SIZE - __builtin_fabsf(2.0f * offsetX)) / screen->resolution.vector.z);
    } else {
        xglVectorScale(&scaled, &screen->resolution.vector,
                       (CAMERA_GS_COORD_SIZE - __builtin_fabsf(2.0f * offsetY)) / screen->resolution.vector.w);
    }
    screen->clipRange.vector.z = screen->projection.x * (scaled.z / screen->resolution.vector.z);
    screen->clipRange.vector.w = screen->projection.y * (scaled.w / screen->resolution.vector.w);
    QuadwordCopy(&screen->projection, &screen->resolution);
    screen->projection.x = screen->projection.x * scale;
    screen->projection.z = screen->projection.z * scale;
    screen->screenOffset.x = offsetX + CAMERA_GS_COORD_CENTER;
    screen->screenOffset.y = offsetY + CAMERA_GS_COORD_CENTER;
    screen->screenOffset.z = depth.minimum + depth.range * 0.5f;
    screen->screenOffset.w = (1.0f - screen->portEffect.z) * 255.0f;
    screen->screenScale.x = (scaled.x * screen->portEffect.x) * 0.5f;
    screen->screenScale.y = -((scaled.y * screen->portEffect.y) * 0.5f);
    screen->screenScale.z = -(depth.range * 0.5f);
    screen->screenScale.w = (screen->portEffect.z * screen->portEffect.w) * 255.0f;
}

float tanf(float angle);
void xglVectorMulMat(Vector4 *transformedVector, Matrix4 matrix, const Vector4 *vector);

void xglPlaneParameter(Vector4 *plane, const Vector4 *a, const Vector4 *b, const Vector4 *c);

void xglMatrixStackMul(Matrix4 matrix);

void xglMatrixStackInverse(void);

void xglMatrixStackTrans(const Vector4 *vector);

void xglMatrixStackRotX(float angle);

void xglMatrixStackRotZ(float angle);

void xglMatrixStackFrustum(float left, float right, float bottom, float top, float nearPlane, float farPlane);

extern signed char aaiVertexIndex[6][3];

/* Builds the frustum corners and the view/inverse transforms from the screen and travel state, then saves them into
 * the camera. */
void xglCameraViewTravel(XglCamera *camera, Vector4 *offset)
{
    XglCameraScreen *screen = &camera->screen;
    CameraTravel *travel = &camera->travel;
    AlignedVector4 scale;
    AlignedVector4 aspect;
    AlignedVector4 position;

    scale.x = tanf(travel->fov * 0.5f);
    scale.y = scale.x;
    scale.z = scale.x * (screen->clipRange.vector.w / screen->projection.y);
    scale.w = scale.z;
    aspect.x = screen->projection.x / screen->projection.y;
    aspect.y = aspect.x;
    aspect.z = screen->clipRange.vector.z / screen->clipRange.vector.w;
    aspect.w = aspect.z;
    travel->frustum.corner[3].x = screen->clipRange.vector.x;
    travel->frustum.corner[3].y = screen->clipRange.vector.y;
    travel->frustum.corner[3].z = travel->frustum.corner[3].x;
    travel->frustum.corner[3].w = travel->frustum.corner[3].y;
    Vu0MulVector(&travel->frustum.corner[3], &travel->frustum.corner[3], &scale);
    xglVectorScale(&travel->frustum.corner[2], &travel->frustum.corner[3], -1.0f);
    Vu0MulVector(&travel->frustum.corner[1], &travel->frustum.corner[3], &aspect);
    xglVectorScale(&travel->frustum.corner[0], &travel->frustum.corner[1], -1.0f);
    xglMatrixStackUnit();
    Vu0SubVectorXYZ(&position, &travel->position, offset);
    xglMatrixStackTrans(&position);
    xglMatrixStackRotY(travel->rotation.y);
    xglMatrixStackRotX(travel->rotation.x);
    xglMatrixStackRotZ(travel->rotation.z);
    xglMatrixStackSave(travel->transform.rows);
    xglMatrixStackInverse();
    xglMatrixStackSave(travel->inverseTransform);
}

/* Copies the travel frustum into the camera volume, saves the view matrix and builds the volume vertices,
 * planes and plane sets. */
void xglCameraViewVolume(XglCamera *camera, Vector4 *offset) {
    XglCameraScreen *screen;
    CameraTravel *travel;
    CameraVolume *volume;
    AlignedVector4 corner;
    int x;
    int verticalRow;
    int y;
    int i;
    int j;
    int k;

    screen = &camera->screen;
    travel = &camera->travel;
    for (i = 0; i <= 0; i++) {
        volume = &camera->volume[i];
        QuadwordCopyMatrix(volume->frustum, travel->frustum.matrix);
        xglMatrixStackUnit();
        xglMatrixStackFrustum(volume->frustum[0][2], volume->frustum[1][2], volume->frustum[2][2],
                      volume->frustum[3][2], screen->clipRange.vector.x, screen->clipRange.vector.y);
        xglMatrixStackMul(travel->inverseTransform);
        xglMatrixStackSave(volume->viewMatrix);
        for (j = 0; j < 2; j++) {
            for (k = 0; k < 8; k++) {
                x = k & 1;
                verticalRow = ((k & 2) >> 1) | 2;
                y = ((k & 4) >> 2) | ((j & 1) << 1);
                corner.x = volume->frustum[x][y];
                corner.y = volume->frustum[verticalRow][y];
                corner.z = -screen->clipRange.component[y & 1];
                corner.w = 1.0f;
                xglVectorMulMat(&volume->vertex[j * 8 + k].vector, travel->transform.rows, &corner);
            }
            {
            for (k = 0; k < 6; k++) {
                xglPlaneParameter(&(volume->plane + j * 6)[k].vector,
                                  &(volume->vertex + j * 8)[aaiVertexIndex[k][0]].vector,
                                  &(volume->vertex + j * 8)[aaiVertexIndex[k][1]].vector,
                                  &(volume->vertex + j * 8)[aaiVertexIndex[k][2]].vector);
            }
            }
        }
        {
        for (j = 0; j < 4; j++) {
            for (k = 0; k < 3; k++) {
                volume->planeSet[0][j][k] = volume->plane[k].component[j];
                volume->planeSet[1][j][k] = volume->plane[k + 3].component[j];
                volume->planeSet[2][j][k] = volume->plane[k + 6].component[j];
                volume->planeSet[3][j][k] = volume->plane[k + 9].component[j];
            }
        }
        }
    }
}

void xglCameraResetState(XglCamera *camera);

/* Fills the CameraTravel block at camera+0x90 with the default travel view. */
void xglCameraTravelDefaultInit(XglCamera *camera);

void xglCameraScreenInit(XglCamera *camera);

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
    xglCameraViewScreen(camera, &cameraShift);
    xglCameraViewTravel(camera, &cameraShift);
    xglCameraViewVolume(camera, &cameraShift);
}

void xglCameraMoveOffset(XglCamera *camera, Vector4 *cameraShift) {
    xglCameraViewScreen(camera, cameraShift);
    xglCameraViewTravel(camera, cameraShift);
    xglCameraViewVolume(camera, cameraShift);
}

/* Sets the camera window to the rectangle (left, top) - (right, bottom): corners sorted, then clamped to the frame. */
void xglCameraSetWindow(XglCamera *camera, int left, int top, int right, int bottom)
{
    int swapCoordinate;
    XglCameraScreen *screen;

    if (camera == 0) {
        return;
    }
    screen = &camera->screen;
    if (right < left) {
        swapCoordinate = right;
        right = left;
        left = swapCoordinate;
    }
    if (bottom < top) {
        swapCoordinate = bottom;
        bottom = top;
        top = swapCoordinate;
    }
    if (left < 0) {
        left = 0;
    }
    if (right < 0) {
        right = 0;
    }
    if (top < 0) {
        top = 0;
    }
    if (bottom < 0) {
        bottom = 0;
    }
    if (left >= D_004C8390.frame_width) {
        left = D_004C8390.frame_width;
    }
    if (right >= D_004C8390.frame_width) {
        right = D_004C8390.frame_width;
    }
    if (top >= D_004C8390.frame_height) {
        top = D_004C8390.frame_height;
    }
    if (bottom >= D_004C8390.frame_height) {
        bottom = D_004C8390.frame_height;
    }
    screen->window.vector.x = left;
    screen->window.vector.y = top;
    screen->window.vector.z = right;
    screen->window.vector.w = bottom;
}

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

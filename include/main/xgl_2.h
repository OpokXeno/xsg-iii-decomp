/* header_harvest: hand-maintained (X3 express integration; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_MAIN_XGL_2_H
#define INCLUDE_MAIN_XGL_2_H

/* Types owned by main/tu029 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"

/* The xgl camera (0x600 bytes; xgl_studio.c keeps eight per studio). Earlier files called it StudioCamera:
 * nearClip/farClip are screen.clipRange, fov is travel.fov, the 0x60 rectangle is screen.window and the view
 * matrix _CurMatrixSet loads is volume[0].viewMatrix. */
typedef struct XglCamera XglCamera;

typedef struct XglCameraScreen XglCameraScreen;

/* The four frustum vectors of a travel view; xglCameraViewTravel copies them as one matrix into the camera volume. */
typedef union CameraFrustum {
    Vector4 corner[4];
    Matrix4 matrix;
} CameraFrustum;

typedef struct CameraTravel {
    float focus;
    float fov;
    float fov_velocity;
    float scale;
    Vector4 rotation;
    Vector4 angle_velocity;
    Vector4 angle_delta;
    Vector4 position;
    Vector4 place_velocity;
    Vector4 place_delta;
    float chase_yaw;
    float chase_distance;
    int unknown_78;
    unsigned char unmodeled_7c[4];
    Vector4 chase_reference;
    Vector4 chase_position;
    CameraFrustum frustum;
    /* +0xE0 and +0x120 of the travel view: the camera transform pair saved by xglCameraViewTravel. */
    Matrix4 transform;
    Matrix4 inverseTransform;
} CameraTravel;

/* A vector viewed as a Vector4 or as its four floats by index. The original reaches such vectors through a union: the
 * stores have alias set 0, so every store reloads the pointers (the levers named where it is used). */
typedef union Vector4Elements {
    Vector4 vector;
    float component[4];
} Vector4Elements;

typedef struct CameraVolume {
    Vector4Elements vertex[16];
    Vector4Elements plane[12];
    Matrix4 frustum;
    Matrix4 viewMatrix;
    unsigned char unmodeled_240[0x40];
    float planeSet[4][4][4];
} CameraVolume;

/* A 4x4 matrix as xgl_2.c and nml_model_set.c read it: floats, rows, words or quadwords. */
typedef unsigned int XglMatrixQuadword __attribute__((mode(TI)));

typedef union Matrix {
    float elements[16];
    Matrix4 rows;
    int words[4][4];
    XglMatrixQuadword quadwords[4];
    Vector4 vectors[4];
} Matrix;

/* One projected point as the rot-trans-pers functions store it: GS fixed-point x, y and z (the vftoi result
   of the VU0 vertex) and the float w that stays in the fourth word (the perspective scale). */
typedef struct ScreenVector4 {
    int x;
    int y;
    int z;
    float w;
} ScreenVector4;

struct XglCameraScreen {
    unsigned int depthMax;
    unsigned int depthMin;
    unsigned char unmodeled_08[8];
    Vector4Elements clipRange; /* x, y = near, far */
    Vector4 portEffect;
    Vector4 resolution;
    Vector4 projection;
    Vector4 window;
    Vector4 screenOffset;
    Vector4 screenScale;
};

struct XglCamera {
    unsigned int active;
    unsigned int state;
    int padPort;                     /* +0x8: PadData port read by the pad updates */
    unsigned char unmodeled_0c[4];
    XglCameraScreen screen;
    CameraTravel travel;
    unsigned char unmodeled_1f0[0x80];
    CameraVolume volume[1];
    int lookAtLocked;                /* +0x5F0: toggled by button 0x800 */
    float unknown_5f4;               /* +0x5F4: set to 1.0f by xglCameraInit */
    unsigned char unmodeled_5f8[8];  /* the studio keeps cameras at a 0x600 stride */
};

void xglRotTransPersN(ScreenVector4 *destination, Matrix4 matrix, const Vector4 *source, int count, int cameraId);
void func_00196200(Matrix4 destination);
struct XglCamera *xglStudioGetCamera2(int camera_index);

#endif

#include "common.h"
#include "shared.h"

#include "main/xgl_2.h"

/* Row-major 4x4 float matrix. */

void assertFailed(const char *message, const char *file, int line);





/* Camera animation record: the version float at +4 picks the layout of the curve data that starts at +0xC; the frame
 * range of its playback is at +8 and +0xA (the title scene steps through it). */
typedef struct TiCameraRecord {
    unsigned char unmodeled_00[4];
    f32 version;
    unsigned short frameStart;  /* +8 */
    unsigned short frameEnd;    /* +0xA */
    u8 curveData[0];
} TiCameraRecord;

void tiCameraReadRecord(XglCamera *camera, TiCameraRecord *cameraRecord, f32 curveParam);
void xglEulerFromRotation(Vector4 *euler, Vector4 *rotation);

f32 atanf(f32);

/* The scalar and angle readers zero two words of the camera's travel block (fov_velocity and scale of
 * XglCamera.travel) with integer stores; as float members they are stored as 0.0f, which changes the code (.text DIFF).
 * This view of travel.focus..scale keeps those two words int. */
typedef struct TiCameraChannel {
    unsigned char unmodeled_00[4];
    float fov;                /* travel.fov */
    int fovVelocity;             /* travel.fov_velocity */
    int scale;             /* travel.scale */
} TiCameraChannel;

extern const char tiCameraAssertPData[];
extern const char tiCameraSourceFile[];
extern const char tiCameraAssertPCam[];

void xacEvalJointPose(f32 *values, u8 **ppJointHeader, f32 curveParam);
void xacEvalJointValue(f32 curveParam, f32 *values, u8 **ppJointHeader);

/* Data of this TU: the assert strings, in address order. */
extern const char tiCameraAssertPData[8];

extern const char tiCameraSourceFile[24];
extern const char tiCameraAssertPCam[8];

/* Horizontal or vertical field of view in degrees from a focal length in millimetres and a frame size in inches
 * (0.03937 converts mm to inch). */
f32 tiCameraFovDegrees(f32 focal_length_mm, f32 frame_size)
{
    f32 half_frame;

    half_frame = frame_size * 0.5f;
    return 2.0f * atanf(half_frame / (focal_length_mm * 0.03937f)) * 57.29578f;
}

void tiCameraReadPose(XglCamera *camera, u8 **ppData, f32 curveParam)
{
    /* Three position values, then three rotation angles, as xacEvalJointPose writes them. */
    f32 pose[6];
    Vector4 rotation;
    /* Unused local: removing it changes the stack frame and the .text bytes. */
    Vector4 unused_frame_slot;

    if (ppData == 0) {
        assertFailed(tiCameraAssertPData, tiCameraSourceFile, 28);
    }
    if (camera == 0) {
        assertFailed(tiCameraAssertPCam, tiCameraSourceFile, 29);
    }
    xacEvalJointPose(pose, ppData, curveParam);
    camera->travel.position.x = pose[0];
    camera->travel.position.y = pose[1];
    camera->travel.position.z = pose[2];
    rotation.x = pose[3];
    rotation.y = pose[4];
    rotation.z = pose[5];
    xglEulerFromRotation(&camera->travel.rotation, &rotation);
}

void tiCameraReadScalar(XglCamera *camera, u8 **ppData, f32 curveParam)
{
    f32 sample;
    TiCameraChannel *channel;

    if (ppData == 0) {
        assertFailed(tiCameraAssertPData, tiCameraSourceFile, 56);
    }
    if (camera == 0) {
        assertFailed(tiCameraAssertPCam, tiCameraSourceFile, 57);
    }
    xacEvalJointValue(curveParam, &sample, ppData);
    channel = (TiCameraChannel *)&camera->travel;
    channel->fovVelocity = channel->scale = 0;
    channel->fov = sample;
}

void tiCameraReadAngle(XglCamera *camera, u8 **ppData, f32 curveParam)
{
    f32 sample;
    TiCameraChannel *channel;

    if (ppData == 0) {
        assertFailed(tiCameraAssertPData, tiCameraSourceFile, 76);
    }
    if (camera == 0) {
        assertFailed(tiCameraAssertPCam, tiCameraSourceFile, 77);
    }
    xacEvalJointValue(curveParam, &sample, ppData);
    channel = (TiCameraChannel *)&camera->travel;
    channel->scale = 0;
    channel->fovVelocity = 0;
    channel->fov = sample * 3.1415927f / 180.0f;
}

/* Camera animation record: three header words, then `version` picks the layout (< 1.1 uses the
 * scalar reader, otherwise the angle reader). The data cursor starts after the header. */
void tiCameraReadRecord(XglCamera *camera, TiCameraRecord *cameraRecord, f32 curveParam)
{
    u8 *stream;

    stream = cameraRecord->curveData;
    if (cameraRecord->version < 1.1f)
    {
        tiCameraReadPose(camera, &stream, curveParam);
        tiCameraReadScalar(camera, &stream, curveParam);
        return;
    }
    tiCameraReadPose(camera, &stream, curveParam);
    tiCameraReadAngle(camera, &stream, curveParam);
}

INCLUDE_ASM("asm/main/nonmatchings/TiCameraSolver", func_002941D0);

#include "common.h"
#include "shared.h"

#include "main/xgl_2.h"

/* Row-major 4x4 float matrix. */

/* GIFtag EOP bit (bit 15 of the NLOOP/EOP word). */
#define GIF_TAG_EOP 0x8000
/* GIFtag REGS values (one nibble per register) of the packets game code builds: RGBAQ/ST/XYZ2
 * as in the TiDrawJoint tags, RGBAQ + four XYZ2 and RGBAQ + four ST/XYZ2 pairs as in the shadows. */
#define GIF_REGS_RGBAQ_ST_XYZ2 0x521


void tiDrawJointScreenPoints(unsigned int pointCount, const Vector4 *inputPoints, ScreenVector4 *screenPoints);
void tiDrawJointTransformPoints(unsigned int pointCount, Matrix4 matrix, const Vector4 *inputPoints, Vector4 *outputPoints);
void xglVectorMulMat(Vector4 *destination, Matrix4 matrix, const Vector4 *vector);
void xglMatrixStackLoad(Matrix4 matrix);
void xglMatrixStackPush(void);
void xglMatrixStackPop(u32 value);
void xglMatrixStackRTPS(ScreenVector4 *destination, const Vector4 *point, Vector4 *screenScale, Vector4 *screenOffset);
void XrgQuaternionToMatrix(Matrix4 matrix, const Vector4 *quaternion);
int XrgQuaternionFromVectors(Vector4 *destination, const Vector4 *left, const Vector4 *right);
struct XglCamera;
struct XglCamera *xglStudioGetActiveCamera(void);

/* Four-word GIF tag quadword written to the scratchpad packet. */
typedef struct TiDrawJointGifTag {
    unsigned int nloop_eop;       /* NLOOP (bits 0-14) and EOP (bit 15) */
    unsigned int nreg_flg_prim;   /* NREG (28-31), FLG (26-27), PRE (14) and PRIM (15-25) */
    unsigned int regs_lo;         /* register list, one nibble per register */
    unsigned int regs_hi;
} TiDrawJointGifTag;

/* Per-joint GIF tag state: each field is one bit group of the tag. */
typedef struct TiDrawJointGifState {
    unsigned char eop;            /* +0x00 */
    unsigned char unmodeled_01[0x3];
    int nloop;                    /* +0x04 */
    unsigned char nreg;           /* +0x08 */
    unsigned char flg;            /* +0x09 */
    unsigned char pre;            /* +0x0A */
    unsigned char prim_type;      /* +0x0B */
    unsigned char prim_iip;       /* +0x0C */
    unsigned char prim_tme;       /* +0x0D */
    unsigned char prim_fge;       /* +0x0E */
    unsigned char prim_abe;       /* +0x0F */
    unsigned char prim_aa1;       /* +0x10 */
    unsigned char prim_ctxt;      /* +0x11 */
    unsigned char prim_fix;       /* +0x12 */
} TiDrawJointGifState;








extern Vector4 tiDrawJointMeshPoints[16];
extern Vector4 tiDrawJointMeshBase[];
extern Vector4 tiDrawJointRefAxis[];
/* Base of the 16-point mesh, before orientation. */
extern Vector4 tiDrawJointMeshBase[16];
extern Vector4 tiDrawJointRefAxis[1];

void tiDrawCollisionDummy(void)
{
}

/* Writes the GIF tag header for state into the packet: REGS is RGBAQ, ST, XYZ2. */
void tiDrawJointSetGifTag(TiDrawJointGifTag *tag, const TiDrawJointGifState *gifState)
{
    unsigned int nreg_prim;
    unsigned int pre_flg;

    tag->nloop_eop = gifState->eop != 0 ? gifState->nloop | GIF_TAG_EOP : gifState->nloop;
    nreg_prim = (gifState->nreg << 28)
        | ((gifState->prim_type | (gifState->prim_iip << 3) | (gifState->prim_tme << 4) | (gifState->prim_fge << 5)
           | (gifState->prim_abe << 6) | (gifState->prim_aa1 << 7) | (gifState->prim_ctxt << 9) | (gifState->prim_fix << 10)) << 15);
    pre_flg = (gifState->pre << 14) | (gifState->flg << 26);
    tag->regs_lo = GIF_REGS_RGBAQ_ST_XYZ2;
    tag->regs_hi = 0;
    tag->nreg_flg_prim = nreg_prim | pre_flg;
}

/* Transforms count points through the active camera's view matrix and screen (RTPS). */
void tiDrawJointScreenPoints(unsigned int pointCount, const Vector4 *inputPoints, ScreenVector4 *screenPoints)
{
    XglCamera *camera;
    const Vector4 *pointCursor;

    pointCursor = inputPoints;
    xglMatrixStackPush();
    camera = xglStudioGetActiveCamera();
    xglMatrixStackLoad(camera->volume[0].viewMatrix);
    while (pointCount != 0) {
        xglMatrixStackRTPS(screenPoints, pointCursor, &camera->screen.screenScale, &camera->screen.screenOffset);
        pointCursor += 1;
        screenPoints += 1;
        pointCount -= 1;
    }
    xglMatrixStackPop(1);
}

/* Transforms count points by matrix into destination; each result gets w = 1. */
void tiDrawJointTransformPoints(unsigned int pointCount, Matrix4 matrix, const Vector4 *inputPoints,
                   Vector4 *outputPoints)
{
    unsigned int pointIndex;

    pointIndex = 0;
    if (pointCount != 0) {
        do {
            xglVectorMulMat(outputPoints, matrix, inputPoints);
            outputPoints->w = 1.0f;
            pointIndex += 1;
            inputPoints += 1;
            outputPoints += 1;
        } while (pointIndex < pointCount);
    }
}

void assertFailed(const char *message, const char *file, int line);
extern const char tiDrawJointAssertSourceText[];
extern const char tiDrawJointAssertFile[];
extern const char tiDrawJointAssertColourText[];

/* Packs count points for the VIF1 direct packet: both pointers are asserted (lines 86 and 87). The original body
 * leaves no store or call after the asserts, so only the index loop over count remains in the object. */
void tiDrawJointPackPoints(unsigned int pointCount, const Vector4 *inputPoints, const unsigned int *colour, int unused_flag)
{
    unsigned int pointIndex;

    if (inputPoints == 0) {
        assertFailed(tiDrawJointAssertSourceText, tiDrawJointAssertFile, 86);
    }
    if (colour == 0) {
        assertFailed(tiDrawJointAssertColourText, tiDrawJointAssertFile, 87);
    }
    pointIndex = 0;
    if (pointCount != 0) {
        do {
            pointIndex += 2;
        } while (pointIndex < pointCount);
    }
}

extern const char tiDrawJointAssertMatrixText[];
extern Vector4 tiDrawJointAxisXPoints[2];
extern Vector4 tiDrawJointAxisYPoints[2];
extern Vector4 tiDrawJointAxisZPoints[2];
extern unsigned int tiDrawJointAxisXColour[4];
extern unsigned int tiDrawJointAxisYColour[4];
extern unsigned int tiDrawJointAxisZColour[4];

/* Draws the three axis stages (matrix asserted at line 121): each transforms two points through matrix, screens
 * them and packs them with its colour block. value is written into point 1 of each stage first. */
void tiDrawJointDrawAxes(Matrix4 matrix, float axisScale)
{
    ScreenVector4 screenPoints[2];
    Vector4 localPoints[2];

    if (matrix == 0) {
        assertFailed(tiDrawJointAssertMatrixText, tiDrawJointAssertFile, 121);
    }
    tiDrawJointAxisXPoints[1].x = axisScale;
    tiDrawJointAxisYPoints[1].y = axisScale;
    tiDrawJointAxisZPoints[1].z = axisScale;
    tiDrawJointTransformPoints(2, matrix, tiDrawJointAxisXPoints, localPoints);
    tiDrawJointScreenPoints(2, localPoints, screenPoints);
    tiDrawJointPackPoints(2, localPoints, tiDrawJointAxisXColour, 1);
    tiDrawJointTransformPoints(2, matrix, tiDrawJointAxisYPoints, localPoints);
    tiDrawJointScreenPoints(2, localPoints, screenPoints);
    tiDrawJointPackPoints(2, localPoints, tiDrawJointAxisYColour, 1);
    tiDrawJointTransformPoints(2, matrix, tiDrawJointAxisZPoints, localPoints);
    tiDrawJointScreenPoints(2, localPoints, screenPoints);
    tiDrawJointPackPoints(2, localPoints, tiDrawJointAxisZColour, 1);
}

extern const char tiDrawJointAssertMeshMatrixText[];
extern Vector4 tiDrawJointMeshTransformed[16];
extern ScreenVector4 tiDrawJointMeshScreen[16];

/* Draws the 16-point mesh tiDrawJointMeshPoints through current_matrix (asserted at line 146). Its points are
 * transformed into tiDrawJointMeshTransformed, screened into tiDrawJointMeshScreen, then packed with colour and flag 0. */
void tiDrawJointDrawMesh(Matrix4 current_matrix, const unsigned int *colour)
{
    if (current_matrix == 0) {
        assertFailed(tiDrawJointAssertMeshMatrixText, tiDrawJointAssertFile, 146);
    }
    tiDrawJointTransformPoints(16, current_matrix, tiDrawJointMeshPoints, tiDrawJointMeshTransformed);
    tiDrawJointScreenPoints(16, tiDrawJointMeshTransformed, tiDrawJointMeshScreen);
    tiDrawJointPackPoints(16, tiDrawJointMeshTransformed, colour, 0);
}

/* Normalises the axis into a Vector4 with w = 1, records its length in four table points, then
 * transforms the table: negated when XrgQuaternionFromVectors returns 0, else through the quaternion matrix. */
void tiDrawJointOrientMesh(const float *axis)
{
    Vector4 normal;
    Vector4 quaternion;
    Matrix4 matrix;
    float length;
    float inverseLength;
    unsigned int meshIndex;

    length = __builtin_sqrtf(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);
    if (length != 1.0f) {
        if (length == 0.0f) {
            length = 1.0e-7f;
        }
        inverseLength = 1.0f / length;
        normal.x = axis[0] * inverseLength;
        normal.y = axis[1] * inverseLength;
        normal.z = axis[2] * inverseLength;
    } else {
        normal.x = axis[0];
        normal.y = axis[1];
        normal.z = axis[2];
    }
    normal.w = 1.0f;
    tiDrawJointMeshBase[9].x = length;
    tiDrawJointMeshBase[11].x = length;
    tiDrawJointMeshBase[13].x = length;
    tiDrawJointMeshBase[15].x = length;
    if (XrgQuaternionFromVectors(&quaternion, tiDrawJointRefAxis, &normal) == 0) {
        for (meshIndex = 0; meshIndex < 16; meshIndex += 1) {
            tiDrawJointMeshPoints[meshIndex].x = -tiDrawJointMeshBase[meshIndex].x;
            tiDrawJointMeshPoints[meshIndex].y = -tiDrawJointMeshBase[meshIndex].y;
            tiDrawJointMeshPoints[meshIndex].z = -tiDrawJointMeshBase[meshIndex].z;
            tiDrawJointMeshPoints[meshIndex].w = 1.0f;
        }
    } else {
        XrgQuaternionToMatrix(matrix, &quaternion);
        for (meshIndex = 0; meshIndex < 16; meshIndex += 1) {
            xglVectorMulMat(&tiDrawJointMeshPoints[meshIndex], matrix, &tiDrawJointMeshBase[meshIndex]);
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/TiDrawJoint", func_00294D10);

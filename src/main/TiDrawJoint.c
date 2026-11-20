#include "common.h"
#include "shared.h"

#include "main/xgl_2.h"

/* Row-major 4x4 float matrix. */

/* GIFtag EOP bit (bit 15 of the NLOOP/EOP word). */
#define GIF_TAG_EOP 0x8000
/* GIFtag REGS values (one nibble per register) of the packets game code builds: RGBAQ/ST/XYZ2
 * as in the TiDrawJoint tags, RGBAQ + four XYZ2 and RGBAQ + four ST/XYZ2 pairs as in the shadows. */
#define GIF_REGS_RGBAQ_ST_XYZ2 0x521


void tiDrawJointScreenPoints(unsigned int count, const Vector4 *source, ScreenVector4 *destination);
void tiDrawJointTransformPoints(unsigned int count, Matrix4 matrix, const Vector4 *source, Vector4 *destination);
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
void tiDrawJointSetGifTag(TiDrawJointGifTag *tag, const TiDrawJointGifState *state)
{
    unsigned int nreg_prim;
    unsigned int pre_flg;

    tag->nloop_eop = state->eop != 0 ? state->nloop | GIF_TAG_EOP : state->nloop;
    nreg_prim = (state->nreg << 28)
        | ((state->prim_type | (state->prim_iip << 3) | (state->prim_tme << 4) | (state->prim_fge << 5)
           | (state->prim_abe << 6) | (state->prim_aa1 << 7) | (state->prim_ctxt << 9) | (state->prim_fix << 10)) << 15);
    pre_flg = (state->pre << 14) | (state->flg << 26);
    tag->regs_lo = GIF_REGS_RGBAQ_ST_XYZ2;
    tag->regs_hi = 0;
    tag->nreg_flg_prim = nreg_prim | pre_flg;
}

/* Transforms count points through the active camera's view matrix and screen (RTPS). */
void tiDrawJointScreenPoints(unsigned int count, const Vector4 *source, ScreenVector4 *destination)
{
    XglCamera *camera;
    const Vector4 *points;

    points = source;
    xglMatrixStackPush();
    camera = xglStudioGetActiveCamera();
    xglMatrixStackLoad(camera->volume[0].viewMatrix);
    while (count != 0) {
        xglMatrixStackRTPS(destination, points, &camera->screen.screenScale, &camera->screen.screenOffset);
        points += 1;
        destination += 1;
        count -= 1;
    }
    xglMatrixStackPop(1);
}

/* Transforms count points by matrix into destination; each result gets w = 1. */
void tiDrawJointTransformPoints(unsigned int count, Matrix4 matrix, const Vector4 *source,
                   Vector4 *destination)
{
    unsigned int index;

    index = 0;
    if (count != 0) {
        do {
            xglVectorMulMat(destination, matrix, source);
            destination->w = 1.0f;
            index += 1;
            source += 1;
            destination += 1;
        } while (index < count);
    }
}

void assertFailed(const char *message, const char *file, int line);
extern const char D_005427A0[];
extern const char D_005427A8[];
extern const char D_005427C0[];

/* Packs count points for the VIF1 direct packet: both pointers are asserted (lines 86 and 87). The original body
 * leaves no store or call after the asserts, so only the index loop over count remains in the object. */
void func_002948C0(unsigned int count, const ScreenVector4 *source, const unsigned int *colour, int flag)
{
    unsigned int index;

    if (source == 0) {
        assertFailed(D_005427A0, D_005427A8, 86);
    }
    if (colour == 0) {
        assertFailed(D_005427C0, D_005427A8, 87);
    }
    index = 0;
    if (count != 0) {
        do {
            index += 2;
        } while (index < count);
    }
}

extern const char D_005427C8[];
extern Vector4 D_004C9820[2];
extern Vector4 D_004C9850[2];
extern Vector4 D_004C9880[2];
extern unsigned int D_004C9840[4];
extern unsigned int D_004C9870[4];
extern unsigned int D_004C98A0[4];

/* Draws the three axis stages (matrix asserted at line 121): each transforms two points through matrix, screens
 * them and packs them with its colour block. value is written into point 1 of each stage first. */
void tiDrawJointDrawAxes(Matrix4 matrix, float value)
{
    ScreenVector4 screen[2];
    Vector4 local[2];

    if (matrix == 0) {
        assertFailed(D_005427C8, D_005427A8, 121);
    }
    D_004C9820[1].x = value;
    D_004C9850[1].y = value;
    D_004C9880[1].z = value;
    tiDrawJointTransformPoints(2, matrix, D_004C9820, local);
    tiDrawJointScreenPoints(2, local, screen);
    func_002948C0(2, local, D_004C9840, 1);
    tiDrawJointTransformPoints(2, matrix, D_004C9850, local);
    tiDrawJointScreenPoints(2, local, screen);
    func_002948C0(2, local, D_004C9870, 1);
    tiDrawJointTransformPoints(2, matrix, D_004C9880, local);
    tiDrawJointScreenPoints(2, local, screen);
    func_002948C0(2, local, D_004C98A0, 1);
}

extern const char D_005427D8[];
extern Vector4 D_008E5890[16];
extern ScreenVector4 D_008E5990[16];

/* Draws the 16-point mesh tiDrawJointMeshPoints through current_matrix (asserted at line 146). Its points are
 * transformed into D_008E5890, screened into D_008E5990, then packed with colour and flag 0. */
void tiDrawJointDrawMesh(Matrix4 current_matrix, const unsigned int *colour)
{
    if (current_matrix == 0) {
        assertFailed(D_005427D8, D_005427A8, 146);
    }
    tiDrawJointTransformPoints(16, current_matrix, tiDrawJointMeshPoints, D_008E5890);
    tiDrawJointScreenPoints(16, D_008E5890, D_008E5990);
    func_002948C0(16, D_008E5890, colour, 0);
}

/* Normalises the axis into a Vector4 with w = 1, records its length in four table points, then
 * transforms the table: negated when XrgQuaternionFromVectors returns 0, else through the quaternion matrix. */
void tiDrawJointOrientMesh(const float *axis)
{
    Vector4 normal;
    Vector4 quaternion;
    Matrix4 matrix;
    float length;
    float scale;
    unsigned int index;

    length = __builtin_sqrtf(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);
    if (length != 1.0f) {
        if (length == 0.0f) {
            length = 1.0e-7f;
        }
        scale = 1.0f / length;
        normal.x = axis[0] * scale;
        normal.y = axis[1] * scale;
        normal.z = axis[2] * scale;
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
        for (index = 0; index < 16; index += 1) {
            tiDrawJointMeshPoints[index].x = -tiDrawJointMeshBase[index].x;
            tiDrawJointMeshPoints[index].y = -tiDrawJointMeshBase[index].y;
            tiDrawJointMeshPoints[index].z = -tiDrawJointMeshBase[index].z;
            tiDrawJointMeshPoints[index].w = 1.0f;
        }
    } else {
        XrgQuaternionToMatrix(matrix, &quaternion);
        for (index = 0; index < 16; index += 1) {
            xglVectorMulMat(&tiDrawJointMeshPoints[index], matrix, &tiDrawJointMeshBase[index]);
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/TiDrawJoint", func_00294D10);

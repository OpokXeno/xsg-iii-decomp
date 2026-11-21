#include "common.h"
#include "shared.h"

#include "main/xgl_2.h"

/* Row-major 4x4 float matrix. */

void tiDrawCollisionDrawObjectRing(const Vector4 *position, const unsigned int *pCol, float scale);
void assertFailed(const char *message, const char *file, int line);


void xglMatrixStackUnit(void);








void xglCurMulRotateX(float angle);
void xglCurMulRotateZ(float angle);
void xglCurMatrixGet(Matrix4 matrix);
void xglCurApplyVector(Vector4 *destination, const Vector4 *vector);
void xglCurSetRotateY(float angle);
void xglCurSetTranslate(const Vector4 *vector);
void tiDrawJointScreenPoints(unsigned int count, const Vector4 *source, ScreenVector4 *destination);
void tiDrawJointTransformPoints(unsigned int count, Matrix4 matrix, const Vector4 *source, Vector4 *destination);
void tiDrawJointPackPoints(unsigned int count, const Vector4 *source, const unsigned int *colour, int flag);
struct XglCamera;
struct XglCamera *xglStudioGetActiveCamera(void);
float cosf(float);
float sinf(float);

/* Points of the collision ring, and the angle between two neighbouring vertices (the 24-sided ring: 2*pi/24, as the original constant). */
#define TI_COLLISION_RING_POINTS 48
#define TI_COLLISION_RING_STEP 0.26179933547973633f

/* 48 Vector4 points: the ring that tiDrawCollisionBuildRing builds at the given scale. */
extern Vector4 tiDrawCollisionRing[TI_COLLISION_RING_POINTS];
/* First transform pass of the ring (matrix applied). */
extern Vector4 tiDrawCollisionTransformed[];
/* Second pass of the ring, the input of the collision fill. */
extern ScreenVector4 tiDrawCollisionScreenPoints[];
/* Draws the ring through the object transform: transforms the ring points, projects them to screen and packs the
 * transformed points into pCol. */
void tiDrawCollisionDrawRing(Matrix4 pTransMat, const unsigned int *pCol);
extern Vector4 tiDrawCollisionTransformed[48];
extern ScreenVector4 tiDrawCollisionScreenPoints[48];

/* Builds the 48-point ring at scale: cos/sin pairs in the camera's rotated frame, two points per pass. */
void tiDrawCollisionBuildRing(float scale)
{
    XglCamera *pCamera;
    float angleStep;
    float unitW;
    float angle;
    unsigned int angleIndex;
    unsigned int nextAngleIndex;
    unsigned int firstPoint;
    Vector4 *pPoint;
    Vector4 *pNext;
    float *pSecondX, *pFirstX;
    float *pFirstY;
    float *pSecondY;

    angleIndex = 0;
    firstPoint = 0;
    pCamera = xglStudioGetActiveCamera();
    angleStep = TI_COLLISION_RING_STEP;
    unitW = 1.0f;
    xglCurSetRotateY(pCamera->travel.rotation.y);
    xglCurMulRotateX(pCamera->travel.rotation.x);
    xglCurMulRotateZ(pCamera->travel.rotation.z);
    for (; firstPoint < TI_COLLISION_RING_POINTS; firstPoint += 2) {
        angle = angleIndex * angleStep;
        pFirstX = &tiDrawCollisionRing[firstPoint].x;
        pFirstY = &tiDrawCollisionRing[firstPoint + 1].x;
        pFirstY -= 3;
        *pFirstX = cosf(angle) * scale;
        angle = angleIndex * angleStep;
        nextAngleIndex = angleIndex + 1;
        pSecondX = &tiDrawCollisionRing[firstPoint].x;
        pSecondX += 4;
        *pFirstY = sinf(angle) * scale;
        tiDrawCollisionRing[firstPoint].z = 0.0f;
        tiDrawCollisionRing[firstPoint].w = unitW;
        angle = nextAngleIndex * angleStep;
        pSecondY = &tiDrawCollisionRing[firstPoint + 1].x;
        pSecondY++;
        *pSecondX = cosf(angle) * scale;
        angle = nextAngleIndex * angleStep;
        *pSecondY = sinf(angle) * scale;
        tiDrawCollisionRing[firstPoint + 1].z = 0.0f;
        tiDrawCollisionRing[firstPoint + 1].w = unitW;
        xglCurApplyVector(&tiDrawCollisionRing[firstPoint], &tiDrawCollisionRing[firstPoint]);
        xglCurApplyVector(&tiDrawCollisionRing[firstPoint + 1], &tiDrawCollisionRing[firstPoint + 1]);
        angleIndex = nextAngleIndex;
    }
}

/* Assert strings of tiDrawCollisionDrawRing: the scaffold data at 0x542770 ("pCol"), 0x542778 (the file name)
 * and 0x542790 ("pTransMat"). */
extern const char tiDrawCollisionAssertPCol[];
extern const char tiDrawCollisionSourceFile[];
extern const char tiDrawCollisionAssertTransMat[];
/* Scaffold data after tiDrawCollisionRing (0x4C9220: the transform buffer, 0x4C9520: the screen buffer). */
extern Vector4 D_004C9220[48];
extern ScreenVector4 D_004C9520[48];

/* Draws the ring through the object transform: transforms the ring points, projects them to screen and packs the
 * transformed points into pCol. */
void tiDrawCollisionDrawRing(Matrix4 pTransMat, const unsigned int *pCol)
{
    if (pCol == 0) {
        assertFailed(tiDrawCollisionAssertPCol, tiDrawCollisionSourceFile, 51);
    }
    if (pTransMat == 0) {
        assertFailed(tiDrawCollisionAssertTransMat, tiDrawCollisionSourceFile, 52);
    }
    tiDrawJointTransformPoints(TI_COLLISION_RING_POINTS, pTransMat, tiDrawCollisionRing, D_004C9220);
    tiDrawJointScreenPoints(TI_COLLISION_RING_POINTS, D_004C9220, D_004C9520);
    tiDrawJointPackPoints(TI_COLLISION_RING_POINTS, D_004C9220, pCol, 0);
}

/* Per-frame collision draw of one object: builds the ring at scale, takes the object transform and fills pCol. */
void tiDrawCollisionDrawObjectRing(const Vector4 *position, const unsigned int *pCol, float scale)
{
    Matrix4 trans_mat;

    xglMatrixStackUnit();
    tiDrawCollisionBuildRing(scale);
    xglCurSetTranslate(position);
    xglCurMatrixGet(trans_mat);
    tiDrawCollisionDrawRing(trans_mat, pCol);
}

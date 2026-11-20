#include "common.h"
#include "shared.h"
#include "vu0_macro.h"


/*
 * One culling volume: the position, rotation and scale culling_matrix reads,
 * the transform it builds from them, and the four corners it derives.
 */
typedef struct CullingVolume {
    Vector4 position;
    Vector4 rotation;
    Vector4 scale;
    Vector4 normalizedDirection;
    Matrix4 transform;
    Matrix4 inverseTransform;
    Vector4 cullingCorners[4];
} CullingVolume;

void xglMatrixStackUnit(void);
void xglMatrixStackInverse(void);
void xglMatrixStackTrans(const Vector4 *vector);
void xglMatrixStackScale(const Vector4 *vector);
void xglMatrixStackRotX(float angle);
void xglMatrixStackRotY(float angle);
void xglMatrixStackRotZ(float angle);
void xglMatrixStackSave(Matrix4 matrix);
void xglVectorNormal(Vector4 *destination, const Vector4 *source);
extern const AlignedVector4 D_00541D50;
extern const AlignedVector4 D_00541D60;
extern const AlignedVector4 D_00541D70;
extern const AlignedVector4 D_00541D80;
extern const AlignedVector4 D_00541D90;

/* func_0019BA78: builds the volume transform from its position, rotation and scale, then the four corners. */
void func_0019BA78(CullingVolume *volume)
{
    AlignedVector4 forwardAxis;
    AlignedVector4 lowerLeft;
    AlignedVector4 lowerRight;
    AlignedVector4 upperRight;
    AlignedVector4 upperLeft;

    xglMatrixStackUnit();
    xglMatrixStackTrans(&volume->position);
    xglMatrixStackRotZ(volume->rotation.z);
    xglMatrixStackRotY(volume->rotation.y);
    xglMatrixStackRotX(volume->rotation.x);
    xglMatrixStackScale(&volume->scale);
    xglMatrixStackSave(volume->transform);
    xglMatrixStackInverse();
    xglMatrixStackSave(volume->inverseTransform);
    forwardAxis = D_00541D50;
    Vu0ApplyMatrix33XYZ(&volume->normalizedDirection, &volume->transform, &forwardAxis);
    xglVectorNormal(&volume->normalizedDirection, &volume->normalizedDirection);
    lowerLeft = D_00541D60;
    lowerRight = D_00541D70;
    upperRight = D_00541D80;
    upperLeft = D_00541D90;
    Vu0ApplyMatrixXYZ(&volume->cullingCorners[0], &volume->transform, &lowerLeft);
    Vu0ApplyMatrixXYZ(&volume->cullingCorners[1], &volume->transform, &lowerRight);
    Vu0ApplyMatrixXYZ(&volume->cullingCorners[2], &volume->transform, &upperRight);
    Vu0ApplyMatrixXYZ(&volume->cullingCorners[3], &volume->transform, &upperLeft);
}

INCLUDE_ASM("asm/main/nonmatchings/face_point", func_0019BC50);

INCLUDE_ASM("asm/main/nonmatchings/face_point", _FacePoint);

INCLUDE_ASM("asm/main/nonmatchings/face_point", _CheckLine);

INCLUDE_ASM("asm/main/nonmatchings/face_point", plane_from_points);

INCLUDE_ASM("asm/main/nonmatchings/face_point", setup_occlusion);

INCLUDE_ASM("asm/main/nonmatchings/face_point", check_occlusion);

INCLUDE_ASM("asm/main/nonmatchings/face_point", xglCullingCheck);

INCLUDE_ASM("asm/main/nonmatchings/face_point", func_0019C3B8);

INCLUDE_ASM("asm/main/nonmatchings/face_point", func_0019C3C8);

INCLUDE_ASM("asm/main/nonmatchings/face_point", func_0019C420);

INCLUDE_ASM("asm/main/nonmatchings/face_point", check_culling_map);

INCLUDE_ASM("asm/main/nonmatchings/face_point", func_0019C4F8);

INCLUDE_ASM("asm/main/nonmatchings/face_point", func_0019C538);

INCLUDE_ASM("asm/main/nonmatchings/face_point", xglCullingMapCreate);

INCLUDE_ASM("asm/main/nonmatchings/face_point", xglCullingMapDisp);

INCLUDE_ASM("asm/main/nonmatchings/face_point", func_0019C7E0);

INCLUDE_ASM("asm/main/nonmatchings/face_point", func_0019C7F0);

INCLUDE_ASM("asm/main/nonmatchings/face_point", func_0019C7F8);

INCLUDE_ASM("asm/main/nonmatchings/face_point", func_0019C800);

INCLUDE_ASM("asm/main/nonmatchings/face_point", _ApplyMatrix33);

INCLUDE_ASM("asm/main/nonmatchings/face_point", func_0019C850);

INCLUDE_ASM("asm/main/nonmatchings/face_point", _ApplyMatrix2Mat);

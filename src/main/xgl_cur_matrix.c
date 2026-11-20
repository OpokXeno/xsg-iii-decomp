#include "common.h"
#include "shared.h"

/* Row-major 4x4 float matrix. */

void XrgMatrixToQuaternion(float quat[4], Matrix4 matrix);

#include "vu0_macro.h"

void xglCurMatrixSet(Matrix4 matrix);
void xglCurMatrixGet(Matrix4 matrix);
void xglCurApplyVector(Vector4 *destination, const Vector4 *vector);
void xglCurSetRotateX(float value);
void xglCurSetRotateY(float angle);
void xglCurSetRotateZ(f32 angle);
void xglCurSetTranslate(const Vector4 *vector);
void xglCurSetScale(f32 scale);
void xglCurSetScaleXYZ(const Vector4 *scale);
void xglCurMatrixMul(Matrix4 matrix);
f32 cosf(f32);
f32 sinf(f32);
extern Matrix4 xglRotMatrixX;
extern Matrix4 xglRotMatrixY;
extern Matrix4 xglRotMatrixZ;
extern Matrix4 xglTranslateMatrix;
extern Matrix4 xglScaleMatrix;
extern Matrix4 xglScaleXYZMatrix;
void xglCurMulRotateX(float angle);
void xglCurMulRotateY(float angle);
void xglCurMulRotateZ(float angle);
void xglCurMulTranslate(float *translation);
void xglCurMulScale(f32 scale);
void xglCurMulScaleXYZ(const f32 scale[3]);
void xglCurMulInvScaleXYZ(f32 scale[3]);
extern Matrix4 xglMulRotMatrixX;
extern Matrix4 xglMulRotMatrixY;
extern Matrix4 xglMulRotMatrixZ;
extern Matrix4 xglMulTranslateMatrix;
extern Matrix4 xglMulScaleMatrix;
extern Matrix4 xglMulScaleXYZMatrix;
extern Matrix4 xglMulInvScaleMatrix;
void xglVectorAddXYZ(Vector4 *destination, const Vector4 *first, const Vector4 *second);
void tiVu0LoadIdentityMatrix(void);
/* A four-row matrix in data; its rows load into the resident VU0 matrix. */
extern Matrix4 tiVu0IdentityMatrix;

/* Quaternion from the rotation part of a row-major 4x4 matrix (Shepperd's method). */
void XrgMatrixToQuaternion(float quat[4], Matrix4 matrix) {
    float trace;
    float s;
    float two_s;
    int i;
    int j;
    int k;
    float *slot;
    int off_i;
    int off_j;
    int off_k;
    int off_larger_diag;
    int off_ji;
    int off_ij;
    int off_ki;
    int off_ik;
    int off_jk;
    int off_kj;
    int quat_off_j;
    int quat_off_k;
    /* Byte-offset float reads kept: the typed matrix[i][j] and quat[j] forms change the code (check.py). */

    trace = matrix[0][0] + matrix[1][1] + matrix[2][2] + matrix[3][3];
    if (trace >= 1.0f) {
        s = __builtin_sqrtf(trace);
        two_s = s + s;
        quat[3] = two_s * 0.25f;
        quat[0] = (matrix[1][2] - matrix[2][1]) / two_s;
        quat[1] = (matrix[2][0] - matrix[0][2]) / two_s;
        quat[2] = (matrix[0][1] - matrix[1][0]) / two_s;
    } else {
        if (matrix[1][1] < matrix[0][0]) {
            i = 0;
        } else {
            i = 1;
        }
        off_larger_diag = 20 * i;
        if (matrix[2][2] > *(float *)((char *)matrix + off_larger_diag)) {
            i = 2;
        }
        j = (i + 1) % 3;
        k = (j + 1) % 3;
        off_i = 20 * i;
        off_j = 20 * j;
        off_k = 20 * k;
        s = __builtin_sqrtf(*(float *)((char *)matrix + off_i) - *(float *)((char *)matrix + off_j) - *(float *)((char *)matrix + off_k) + 1.0f);
        two_s = s + s;
        slot = &quat[i];
        *slot = two_s * 0.25f;
        off_ji = 16 * j + 4 * i;
        off_ij = 16 * i + 4 * j;
        quat_off_j = 4 * j;
        *(float *)((char *)quat + quat_off_j) = (*(float *)((char *)matrix + off_ji) + *(float *)((char *)matrix + off_ij)) / two_s;
        off_ki = 16 * k + 4 * i;
        off_ik = 16 * i + 4 * k;
        quat_off_k = 4 * k;
        *(float *)((char *)quat + quat_off_k) = (*(float *)((char *)matrix + off_ki) + *(float *)((char *)matrix + off_ik)) / two_s;
        off_jk = 16 * j + 4 * k;
        off_kj = 16 * k + 4 * j;
        quat[3] = (*(float *)((char *)matrix + off_jk) - *(float *)((char *)matrix + off_kj)) / two_s;
    }
}

void xglVectorOuter(Vector4 *destination, const Vector4 *left, const Vector4 *right);
void xglVectorInner(float *result, const Vector4 *left, const Vector4 *right);
extern const float D_00550180;

int XrgQuaternionFromVectors(Vector4 *destination, const Vector4 *left, const Vector4 *right) {
    Vector4 cross;
    float dot;
    float length;
    float inverse;

    xglVectorOuter(&cross, left, right);
    xglVectorInner(&dot, left, right);
    length = __builtin_sqrtf((dot + 1.0f) * 2.0f);
    if (__builtin_fabsf(length) < D_00550180) {
        cross.y = 1.0f;
        cross.x = 0.0f;
        cross.z = 0.0f;
        destination->y = cross.y;
        destination->x = cross.x;
        destination->z = cross.z;
        destination->w = 0.0f;
        return 0;
    }
    inverse = 1.0f / length;
    destination->x = cross.x * inverse;
    destination->y = cross.y * inverse;
    destination->z = cross.z * inverse;
    destination->w = 0.5f * length;
    return 1;
}

/* Copies one vector through VU0 register vf2. */
void xglVectorCopy(Vector4 *destination, const Vector4 *source) {
    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(source) : "memory");
    __asm__ __volatile__("sqc2 vf2, 0(%0)" : : "r"(destination) : "memory");
}

/* Adds the xyz parts of two vectors; w is taken from the first operand. */
void xglVectorAddXYZ(Vector4 *destination, const Vector4 *first, const Vector4 *second) {
    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(first) : "memory");
    __asm__ __volatile__("lqc2 vf3, 0(%0)" : : "r"(second) : "memory");
    __asm__ __volatile__("vadd.xyz vf4, vf2, vf3" : : : "memory");
    __asm__ __volatile__("vmove.w vf4, vf2" : : : "memory");
    __asm__ __volatile__("sqc2 vf4, 0(%0)" : : "r"(destination) : "memory");
}

/* Subtracts the xyz parts of two vectors; w is taken from the minuend. */
void xglVectorSubXYZ(Vector4 *destination, const Vector4 *minuend, const Vector4 *subtrahend) {
    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(minuend) : "memory");
    __asm__ __volatile__("lqc2 vf3, 0(%0)" : : "r"(subtrahend) : "memory");
    __asm__ __volatile__("vsub.xyz vf4, vf2, vf3" : : : "memory");
    __asm__ __volatile__("vmove.w vf4, vf2" : : : "memory");
    __asm__ __volatile__("sqc2 vf4, 0(%0)" : : "r"(destination) : "memory");
}

/* Component-wise product of two vectors. */
void xglVectorMul(Vector4 *destination, const Vector4 *first, const Vector4 *second) {
    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(first) : "memory");
    __asm__ __volatile__("lqc2 vf3, 0(%0)" : : "r"(second) : "memory");
    __asm__ __volatile__("vmul.xyzw vf4, vf2, vf3" : : : "memory");
    __asm__ __volatile__("sqc2 vf4, 0(%0)" : : "r"(destination) : "memory");
}

void xglVectorNormal_002A1010(Vector4 *destination, const Vector4 *source) {
    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(source) : "memory");
    __asm__ __volatile__("vmul.xyz vf3, vf2, vf2" : : : "memory");
    __asm__ __volatile__("vaddy.x vf3, vf3, vf3y" : : : "memory");
    __asm__ __volatile__("vaddz.x vf3, vf3, vf3z" : : : "memory");
    __asm__ __volatile__("vrsqrt Q, vf0w, vf3x" : : : "memory");
    __asm__ __volatile__("vwaitq" : : : "memory");
    __asm__ __volatile__("vmulq.xyz vf2, vf2, Q" : : : "memory");
    __asm__ __volatile__("sqc2 vf2, 0(%0)" : : "r"(destination) : "memory");
}

/* Copies four vectors (a 4x4 matrix) through vf2..vf5. */
void xglMatrixCopy(Vector4 *destination, const Vector4 *source) {
    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(source) : "memory");
    __asm__ __volatile__("lqc2 vf3, 16(%0)" : : "r"(source) : "memory");
    __asm__ __volatile__("lqc2 vf4, 32(%0)" : : "r"(source) : "memory");
    __asm__ __volatile__("lqc2 vf5, 48(%0)" : : "r"(source) : "memory");
    __asm__ __volatile__("sqc2 vf2, 0(%0)" : : "r"(destination) : "memory");
    __asm__ __volatile__("sqc2 vf3, 16(%0)" : : "r"(destination) : "memory");
    __asm__ __volatile__("sqc2 vf4, 32(%0)" : : "r"(destination) : "memory");
    __asm__ __volatile__("sqc2 vf5, 48(%0)" : : "r"(destination) : "memory");
}

/* Loads the data matrix tiVu0IdentityMatrix into the VU0 current matrix (vf27..vf30). */
void tiVu0LoadIdentityMatrix(void) {
    Vu0SetCurrentMatrix(tiVu0IdentityMatrix);
}

/* Sets the VU0 current matrix (vf27-vf30) from a 4x4 matrix. */
void xglCurMatrixSet(Matrix4 matrix) {
    Vu0SetCurrentMatrix(matrix);
}

/* Current matrix = matrix * current matrix: the four rows of matrix are multiplied by vf27-vf30 and the product is kept in vf27-vf30. */
void xglCurMatrixMul(Matrix4 matrix) {
    __asm__ __volatile__(
        "lqc2 vf2, 0(%0)\n\t"
        "lqc2 vf3, 16(%0)\n\t"
        "lqc2 vf4, 32(%0)\n\t"
        "lqc2 vf5, 48(%0)\n\t"
        "vmulax.xyzw ACC, vf27, vf2x\n\t"
        "vmadday.xyzw ACC, vf28, vf2y\n\t"
        "vmaddaz.xyzw ACC, vf29, vf2z\n\t"
        "vmaddw.xyzw vf2, vf30, vf2w\n\t"
        "vmulax.xyzw ACC, vf27, vf3x\n\t"
        "vmadday.xyzw ACC, vf28, vf3y\n\t"
        "vmaddaz.xyzw ACC, vf29, vf3z\n\t"
        "vmaddw.xyzw vf3, vf30, vf3w\n\t"
        "vmulax.xyzw ACC, vf27, vf4x\n\t"
        "vmadday.xyzw ACC, vf28, vf4y\n\t"
        "vmaddaz.xyzw ACC, vf29, vf4z\n\t"
        "vmaddw.xyzw vf4, vf30, vf4w\n\t"
        "vmulax.xyzw ACC, vf27, vf5x\n\t"
        "vmadday.xyzw ACC, vf28, vf5y\n\t"
        "vmaddaz.xyzw ACC, vf29, vf5z\n\t"
        "vmaddw.xyzw vf5, vf30, vf5w\n\t"
        "vmove.xyzw vf27, vf2\n\t"
        "vmove.xyzw vf28, vf3\n\t"
        "vmove.xyzw vf29, vf4\n\t"
        "vmove.xyzw vf30, vf5"
        :
        : "r"(matrix)
        : "memory"
    );
}

/* Stores the VU0 current matrix (vf27-vf30) into matrix. */
void xglCurMatrixGet(Matrix4 matrix) {
    Vu0GetCurrentMatrix(matrix);
}

/* destination = current matrix * (vector.xyz, 1); the w component is passed through. */
void xglCurApplyVector(Vector4 *destination, const Vector4 *vector) {
    __asm__ __volatile__(
        "lqc2 vf2, 0(%1)\n\t"
        "vmulax.xyzw ACC, vf27, vf2x\n\t"
        "vmadday.xyzw ACC, vf28, vf2y\n\t"
        "vmaddaz.xyzw ACC, vf29, vf2z\n\t"
        "vmaddw.xyzw vf2, vf30, vf2w\n\t"
        "sqc2 vf2, 0(%0)"
        :
        : "r"(destination), "r"(vector)
        : "memory"
    );
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_cur_matrix", func_002A1138);

/* Empty body: no statements, no role shown. */
void xglCurDummy(void) {
}

/* Builds an X-axis rotation in xglRotMatrixX (cos at [1][1] and [2][2], sin at [1][2], -sin at [2][1]) and makes it the current matrix. */
void xglCurSetRotateX(f32 angle) {
    f32 cosine = cosf(angle);
    f32 sine = sinf(angle);

    xglRotMatrixX[1][1] = cosine;
    xglRotMatrixX[1][2] = sine;
    xglRotMatrixX[2][1] = -sine;
    xglRotMatrixX[2][2] = cosine;
    xglCurMatrixSet(xglRotMatrixX);
}

/* Builds a Y-axis rotation in xglRotMatrixY (cos at [0][0] and [2][2], sin at [2][0], -sin at [0][2]) and makes it the current matrix. */
void xglCurSetRotateY(f32 angle) {
    f32 cosine = cosf(angle);
    f32 sine = sinf(angle);

    xglRotMatrixY[0][0] = cosine;
    xglRotMatrixY[2][0] = sine;
    xglRotMatrixY[0][2] = -sine;
    xglRotMatrixY[2][2] = cosine;
    xglCurMatrixSet(xglRotMatrixY);
}

/* Builds a Z-axis rotation in xglRotMatrixZ (cos at [0][0] and [1][1], sin at [0][1], -sin at [1][0]) and makes it the current matrix. */
void xglCurSetRotateZ(f32 angle) {
    f32 cosine = cosf(angle);
    f32 sine = sinf(angle);

    xglRotMatrixZ[0][0] = cosine;
    xglRotMatrixZ[0][1] = sine;
    xglRotMatrixZ[1][0] = -sine;
    xglRotMatrixZ[1][1] = cosine;
    xglCurMatrixSet(xglRotMatrixZ);
}

/* Translation matrix in xglTranslateMatrix (x, y, z in row 3), made the current matrix. */
void xglCurSetTranslate(const Vector4 *vector) {
    xglTranslateMatrix[3][0] = vector->x;
    xglTranslateMatrix[3][1] = vector->y;
    xglTranslateMatrix[3][2] = vector->z;
    xglCurMatrixSet(xglTranslateMatrix);
}

/* Uniform scale matrix in xglScaleMatrix (scale on the diagonal), made the current matrix. */
void xglCurSetScale(f32 scale) {
    xglScaleMatrix[0][0] = scale;
    xglScaleMatrix[1][1] = scale;
    xglScaleMatrix[2][2] = scale;
    xglCurMatrixSet(xglScaleMatrix);
}

/* Per-axis scale matrix in xglScaleXYZMatrix (scale.x, scale.y, scale.z on the diagonal), made the current matrix. */
void xglCurSetScaleXYZ(const Vector4 *scale) {
    xglScaleXYZMatrix[0][0] = scale->x;
    xglScaleXYZMatrix[1][1] = scale->y;
    xglScaleXYZMatrix[2][2] = scale->z;
    xglCurMatrixSet(xglScaleXYZMatrix);
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_cur_matrix", func_002A1318);

/* Rotation about X: cosine on the [1][1] and [2][2] diagonal, sine at [1][2], -sine at [2][1]. */
void xglCurMulRotateX(f32 angle)
{
    f32 cosine;
    f32 sine;

    cosine = cosf(angle);
    sine = sinf(angle);
    xglMulRotMatrixX[2][2] = cosine;
    xglMulRotMatrixX[1][1] = cosine;
    xglMulRotMatrixX[1][2] = sine;
    xglMulRotMatrixX[2][1] = -sine;
    xglCurMatrixMul(xglMulRotMatrixX);
}

/* Rotation about Y: cosine on the [0][0] and [2][2] diagonal, sine at [2][0], -sine at [0][2]. */
void xglCurMulRotateY(f32 angle)
{
    f32 cosine;
    f32 sine;

    cosine = cosf(angle);
    sine = sinf(angle);
    xglMulRotMatrixY[2][2] = cosine;
    xglMulRotMatrixY[0][0] = cosine;
    xglMulRotMatrixY[2][0] = sine;
    xglMulRotMatrixY[0][2] = -sine;
    xglCurMatrixMul(xglMulRotMatrixY);
}

/* Rotation about Z: cosine on the [0][0] and [1][1] diagonal, sine at [0][1], -sine at [1][0]. */
void xglCurMulRotateZ(f32 angle)
{
    f32 cosine;
    f32 sine;

    cosine = cosf(angle);
    sine = sinf(angle);
    xglMulRotMatrixZ[1][1] = cosine;
    xglMulRotMatrixZ[0][0] = cosine;
    xglMulRotMatrixZ[0][1] = sine;
    xglMulRotMatrixZ[1][0] = -sine;
    xglCurMatrixMul(xglMulRotMatrixZ);
}

/* Current matrix = translation matrix * current matrix; translation fills row 3 of xglMulTranslateMatrix. */
void xglCurMulTranslate(float *translation)
{
    xglMulTranslateMatrix[3][0] = translation[0];
    xglMulTranslateMatrix[3][1] = translation[1];
    xglMulTranslateMatrix[3][2] = translation[2];
    xglCurMatrixMul(xglMulTranslateMatrix);
}

/* Current matrix = uniform scale matrix (xglMulScaleMatrix) * current matrix. */
void xglCurMulScale(f32 scale)
{
    xglMulScaleMatrix[0][0] = scale;
    xglMulScaleMatrix[1][1] = scale;
    xglMulScaleMatrix[2][2] = scale;
    xglCurMatrixMul(xglMulScaleMatrix);
}

/* Current matrix = per-axis scale matrix (xglMulScaleXYZMatrix) * current matrix. */
void xglCurMulScaleXYZ(const f32 scale[3])
{
    xglMulScaleXYZMatrix[0][0] = scale[0];
    xglMulScaleXYZMatrix[1][1] = scale[1];
    xglMulScaleXYZMatrix[2][2] = scale[2];
    xglCurMatrixMul(xglMulScaleXYZMatrix);
}

/* Writes 1/x, 1/y, 1/z on the diagonal of xglMulInvScaleMatrix for each non-zero component (zero components are skipped), then multiplies the current matrix by it. */
void xglCurMulInvScaleXYZ(f32 scale[3]) {
    if (scale[0] != 0.0f) {
        xglMulInvScaleMatrix[0][0] = 1.0f / scale[0];
    }
    if (scale[1] != 0.0f) {
        xglMulInvScaleMatrix[1][1] = 1.0f / scale[1];
    }
    if (scale[2] != 0.0f) {
        xglMulInvScaleMatrix[2][2] = 1.0f / scale[2];
    }
    xglCurMatrixMul(xglMulInvScaleMatrix);
}

#include "common.h"
#include "shared.h"

/* Row-major 4x4 float matrix. */

void xglMatrixFromAxes(Matrix4 matrix, const Vector4 *firstRowAxis, const Vector4 *secondRowAxis, const Vector4 *thirdRowAxis);
void xglVectorScaleXYZ(Vector4 *destination, const Vector4 *source, float scale);
void xglVectorNormal(Vector4 *destination, const Vector4 *source);

/* VU0 data-memory quadword address of the VU1 floating-point register window, and its quadword count. */
#define VU1_FLOAT_WINDOW 0x400

#define VU1_FLOAT_REGISTER_QUADS 32

#include "quadword.h"

/*
 * Reads the 32 quadwords of the VU1 floating-point registers through the VU1
 * register window of VU0 data memory (quadword address 0x400) with
 * post-incremented VI loads. Every value lands in one scratch quadword and is
 * discarded.
 */
void xglVu1ReadFloatRegisters(void)
{
    AlignedVector4 discardedQuad;
    int i;

    __asm__ __volatile__("ctc2 %0,$vi1" : : "r"(VU1_FLOAT_WINDOW) : "memory");
    for (i = VU1_FLOAT_REGISTER_QUADS - 1; i >= 0; i--) {
        __asm__ __volatile__("vlqi.xyzw $vf2xyzw,($vi1++)" : : : "memory");
        __asm__ __volatile__("sqc2 $vf2,0(%0)" : : "r"(&discardedQuad) : "memory");
    }
}

/* Transposes a 4x4 matrix. The source is copied first because callers pass the
 * same matrix as both arguments (xglMatrixFromAxes). */
void xglMatrixReverseInPlace(Matrix4 transposed, Matrix4 original)
{
    Matrix4 originalCopy;

    QuadwordCopyMatrix(originalCopy, original);
    transposed[0][0] = originalCopy[0][0];
    transposed[0][1] = originalCopy[1][0];
    transposed[0][2] = originalCopy[2][0];
    transposed[0][3] = originalCopy[3][0];
    transposed[1][0] = originalCopy[0][1];
    transposed[1][1] = originalCopy[1][1];
    transposed[1][2] = originalCopy[2][1];
    transposed[1][3] = originalCopy[3][1];
    transposed[2][0] = originalCopy[0][2];
    transposed[2][1] = originalCopy[1][2];
    transposed[2][2] = originalCopy[2][2];
    transposed[2][3] = originalCopy[3][2];
    transposed[3][0] = originalCopy[0][3];
    transposed[3][1] = originalCopy[1][3];
    transposed[3][2] = originalCopy[2][3];
    transposed[3][3] = originalCopy[3][3];
}

/* Builds a 4x4 matrix whose rows are the normalized negated axes, with a last row of (0, 0, 0, 1). */
void xglMatrixFromAxes(Matrix4 matrix, const Vector4 *firstRowAxis, const Vector4 *secondRowAxis, const Vector4 *thirdRowAxis)
{
    Vector4 row;
    unsigned int i;

    xglVectorScaleXYZ(&row, firstRowAxis, -1.0f);
    xglVectorNormal(&row, &row);
    for (i = 0; i < 4; i++) {
        float *matrixElement = matrix[0] + i;
        float *rowComponent = &row.x + i;

        *matrixElement = *rowComponent;
    }
    xglVectorScaleXYZ(&row, secondRowAxis, -1.0f);
    xglVectorNormal(&row, &row);
    {
        float *rowComponents = &row.x;

        for (i = 0; i < 4; i++) {
            matrix[1][i] = rowComponents[i];
        }
    }
    xglVectorScaleXYZ(&row, thirdRowAxis, -1.0f);
    xglVectorNormal(&row, &row);
    {
        float *rowComponents = &row.x;

        for (i = 0; i < 4; i++) {
            matrix[2][i] = rowComponents[i];
        }
    }
    matrix[3][3] = 1.0f;
    matrix[3][0] = matrix[3][1] = matrix[3][2] = 0.0f;

    xglMatrixReverseInPlace(matrix, matrix);
}

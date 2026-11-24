#include "common.h"
#include "shared.h"
#include "main/xgl_2.h"

/* The quaternion as four floats (elements: x, y, z, w) or one Vector4. The union view is kept: the element
 * stores decide the emitted order (a struct view does not match). */
typedef union QuaternionView {
    float elements[4];
    Vector4 vector;
} QuaternionView;

/* Rotation matrix of a quaternion on VU0: the four rows are stored into matrix, row 3 is (0, 0, 0, 1). */
Matrix *vu0QuatToMatrix(Matrix *matrix, AlignedVector4 *quaternion) {
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(quaternion) : "memory");
    __asm__ __volatile__("vadd.xyzw vf2, vf1, vf1" : : : "memory");
    __asm__ __volatile__("vaddw.xyzw vf9, vf0, vf0w" : : : "memory");
    __asm__ __volatile__("vsub.xyzw vf8, vf0, vf0" : : : "memory");
    __asm__ __volatile__("vmulz.xyzw vf5, vf2, vf1z" : : : "memory");
    __asm__ __volatile__("vmuly.xyzw vf4, vf2, vf1y" : : : "memory");
    __asm__ __volatile__("vmulx.xyzw vf3, vf2, vf1x" : : : "memory");
    __asm__ __volatile__("vsub.xyzw vf7, vf0, vf0" : : : "memory");
    __asm__ __volatile__("vsub.xyzw vf6, vf0, vf0" : : : "memory");
    __asm__ __volatile__("vaddz.y vf10, vf4, vf5z" : : : "memory");
    __asm__ __volatile__("vaddx.z vf11, vf5, vf3x" : : : "memory");
    __asm__ __volatile__("vaddy.x vf12, vf3, vf4y" : : : "memory");
    __asm__ __volatile__("vaddw.x vf8, vf5, vf4w" : : : "memory");
    __asm__ __volatile__("vsubw.x vf7, vf4, vf5w" : : : "memory");
    __asm__ __volatile__("vsuby.x vf6, vf9, vf10y" : : : "memory");
    __asm__ __volatile__("vsubw.y vf8, vf5, vf3w" : : : "memory");
    __asm__ __volatile__("vsubz.y vf7, vf9, vf11z" : : : "memory");
    __asm__ __volatile__("vaddw.y vf6, vf3, vf5w" : : : "memory");
    __asm__ __volatile__("vsubx.z vf8, vf9, vf12x" : : : "memory");
    __asm__ __volatile__("vaddw.z vf7, vf4, vf3w" : : : "memory");
    __asm__ __volatile__("vsubw.z vf6, vf3, vf4w" : : : "memory");
    __asm__ __volatile__("sqc2 vf0, 48(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("sqc2 vf8, 32(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("sqc2 vf7, 16(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("sqc2 vf6, 0(%0)" : : "r"(matrix) : "memory");
    return matrix;
}

/* Quaternion (x, y, z, w) of a rotation matrix, largest-component form: the diagonal decides which component comes
 * from the square root and the others from matrix differences. */
QuaternionView *rotationMatrixToQuaternion(QuaternionView *quaternion, Matrix *rotationMatrix) {
    float trace;
    float w;

    trace = rotationMatrix->elements[0] + rotationMatrix->elements[5] + rotationMatrix->elements[10];
    if (trace > 0.0f) {
        w = __builtin_sqrtf(trace + 1.0f) * 0.5f;
        quaternion->elements[3] = w;
        trace = 0.5f / (w + w);
        quaternion->elements[0] = (rotationMatrix->elements[6] - rotationMatrix->elements[9]) * trace;
        quaternion->elements[1] = (rotationMatrix->elements[8] - rotationMatrix->elements[2]) * trace;
        quaternion->elements[2] = (rotationMatrix->elements[1] - rotationMatrix->elements[4]) * trace;
    } else {
        int nextAxisTable[3] = {1, 2, 0};
        int largestAxis = 0;
        int nextAxis;
        int lastAxis;
        float *largestComponent;

        if (rotationMatrix->elements[0] < rotationMatrix->elements[5]) {
            largestAxis = 1;
        }
        if (rotationMatrix->elements[largestAxis * 5] < rotationMatrix->elements[10]) {
            largestAxis = 2;
        }
        nextAxis = nextAxisTable[largestAxis];
        lastAxis = nextAxisTable[nextAxis];
        largestComponent = quaternion->elements + largestAxis;
        w = __builtin_sqrtf(
                rotationMatrix->elements[largestAxis * 5] -
                (rotationMatrix->elements[nextAxis * 5] + rotationMatrix->elements[lastAxis * 5]) + 1.0f) *
            0.5f;
        *largestComponent = w;
        trace = 0.5f / (w + w);
        quaternion->elements[3] = (rotationMatrix->elements[nextAxis * 4 + lastAxis] -
                                   rotationMatrix->elements[lastAxis * 4 + nextAxis]) *
                                  trace;
        quaternion->elements[nextAxis] = (rotationMatrix->elements[largestAxis * 4 + nextAxis] +
                                          rotationMatrix->elements[nextAxis * 4 + largestAxis]) *
                                         trace;
        quaternion->elements[lastAxis] = (rotationMatrix->elements[largestAxis * 4 + lastAxis] +
                                          rotationMatrix->elements[lastAxis * 4 + largestAxis]) *
                                         trace;
    }
    return quaternion;
}

/* Normalizes the rows of source on VU0 and converts them to a quaternion with rotationMatrixToQuaternion. */
void rotationMatrixNormalizedToQuaternion(QuaternionView *quaternion, Matrix *unnormalizedRows) {
    Matrix normalizedRows;

    __asm__ __volatile__("lqc2 vf1, 0(%0)\n\t"
                         "lqc2 vf2, 16(%0)\n\t"
                         "lqc2 vf3, 32(%0)\n\t"
                         "vmul.xyz vf4, vf1, vf1\n\t"
                         "vaddy.x vf4, vf4, vf4y\n\t"
                         "vaddz.x vf4, vf4, vf4z\n\t"
                         "vrsqrt Q, vf0w, vf4x\n\t"
                         "vmul.xyz vf5, vf2, vf2\n\t"
                         "vaddy.x vf5, vf5, vf5y\n\t"
                         "vaddz.x vf5, vf5, vf5z\n\t"
                         "vwaitq\n\t"
                         "vmulq.xyz vf4, vf1, Q\n\t"
                         "vrsqrt Q, vf0w, vf5x\n\t"
                         "vmul.xyz vf6, vf3, vf3\n\t"
                         "vaddy.x vf6, vf6, vf6y\n\t"
                         "vaddz.x vf6, vf6, vf6z\n\t"
                         "vwaitq\n\t"
                         "vmulq.xyz vf5, vf2, Q\n\t"
                         "vrsqrt Q, vf0w, vf6x\n\t"
                         "vwaitq\n\t"
                         "vmulq.xyz vf6, vf3, Q\n\t"
                         "sqc2 vf4, 0(%1)\n\t"
                         "sqc2 vf5, 16(%1)\n\t"
                         "sqc2 vf6, 32(%1)"
                         :
                         : "r"(unnormalizedRows->vectors), "r"(normalizedRows.vectors)
                         : "memory");
    rotationMatrixToQuaternion(quaternion, &normalizedRows);
}

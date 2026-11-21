#include "common.h"
#include "shared.h"

/* Row-major 4x4 float matrix. */

#include "vu0_macro.h"

float srsAtan2(float x, float y);

/* The identity matrix the rotation functions start from (test_128.c defines it). */
extern const Vector4 vu0IdentityMatrix[4];

void vu0SetCurrentRotationY(float angle);
void vu0RotateCurrentX(float angle);
void vu0RotateCurrentY(float angle);
void vu0RotateCurrentZ(float angle);
void vu0RotateCurrentXYZ(Vector4 *angles);
void vu0RotateCurrentYXZ(Vector4 *angles);
void vu0RotateCurrentZYX(Vector4 *angles);
void vu0SetCurrentRotationYXZ(Vector4 *angles);
void vu0TransformPoint(Vector4 *point);
void vu0TranslateCurrent(Vector4 *offset);

int vu0SetRotationFromDirection(Matrix4 matrix, Vector4 *targetPoint, Vector4 *sourcePoint);

/* Row 3 of the matrix, which holds the direction while the rotation is built: the zero test reads
 * x and y as one doubleword. */
typedef union Vu0Direction {
    Vector4 vector;
    struct {
        unsigned long long xy;
        int z;
    } bits;
} Vu0Direction;

/* Multiplies the current matrix (vf26-vf29) by a rotation about X by angle (nothing for 0); the VU0
 * micro routines at 0xE8 and 0x20 return the cosine and the sine of vf4x in vf1x. */
void vu0RotateCurrentX(float angle)
{
    if (angle != 0.0f) {
        __asm__ __volatile__(
            "mfc1 $8, %0\n\t"
            "qmtc2.ni $8, vf4\n\t"
            "vcallms 0xE8\n\t"
            "vmove.x vf5, vf1\n\t"
            "qmtc2.ni $8, vf4\n\t"
            "vcallms 0x20\n\t"
            "vmove.x vf6, vf1\n\t"
            "vmulax.xyzw ACC, vf27, vf5x\n\t"
            "vmaddx.xyzw vf1, vf28, vf6x\n\t"
            "vmulax.xyzw ACC, vf28, vf5x\n\t"
            "vmsubx.xyzw vf28, vf27, vf6x\n\t"
            "vmove.xyzw vf27, vf1"
            : : "f"(angle) : "$8", "memory");
    }
}

/* Multiplies the current matrix (vf26-vf29) by a rotation about Y by angle (nothing for 0); the VU0
 * micro routines at 0xE8 and 0x20 return the cosine and the sine of vf4x in vf1x. */
void vu0RotateCurrentY(float angle)
{
    if (angle != 0.0f) {
        __asm__ __volatile__(
            "mfc1 $8, %0\n\t"
            "qmtc2.ni $8, vf4\n\t"
            "vcallms 0xE8\n\t"
            "vmove.x vf5, vf1\n\t"
            "qmtc2.ni $8, vf4\n\t"
            "vcallms 0x20\n\t"
            "vmove.x vf6, vf1\n\t"
            "vmulax.xyzw ACC, vf26, vf5x\n\t"
            "vmsubx.xyzw vf1, vf28, vf6x\n\t"
            "vmulax.xyzw ACC, vf28, vf5x\n\t"
            "vmaddx.xyzw vf28, vf26, vf6x\n\t"
            "vmove.xyzw vf26, vf1"
            : : "f"(angle) : "$8", "memory");
    }
}

/* Multiplies the current matrix (vf26-vf29) by a rotation about Z by angle (nothing for 0); the VU0
 * micro routines at 0xE8 and 0x20 return the cosine and the sine of vf4x in vf1x. */
void vu0RotateCurrentZ(float angle)
{
    if (angle != 0.0f) {
        __asm__ __volatile__(
            "mfc1 $8, %0\n\t"
            "qmtc2.ni $8, vf4\n\t"
            "vcallms 0xE8\n\t"
            "vmove.x vf5, vf1\n\t"
            "qmtc2.ni $8, vf4\n\t"
            "vcallms 0x20\n\t"
            "vmove.x vf6, vf1\n\t"
            "vmulax.xyzw ACC, vf26, vf5x\n\t"
            "vmaddx.xyzw vf1, vf27, vf6x\n\t"
            "vmulax.xyzw ACC, vf27, vf5x\n\t"
            "vmsubx.xyzw vf27, vf26, vf6x\n\t"
            "vmove.xyzw vf26, vf1"
            : : "f"(angle) : "$8", "memory");
    }
}

/* Euler rotation of the current matrix: X, then Y, then Z. */
void vu0RotateCurrentXYZ(Vector4 *angles)
{
    vu0RotateCurrentX(angles->x);
    vu0RotateCurrentY(angles->y);
    vu0RotateCurrentZ(angles->z);
}

/* Same three rotations in the order Y, X, Z. */
void vu0RotateCurrentYXZ(Vector4 *angles)
{
    vu0RotateCurrentY(angles->y);
    vu0RotateCurrentX(angles->x);
    vu0RotateCurrentZ(angles->z);
}

/* Same three rotations in the order Z, Y, X. */
void vu0RotateCurrentZYX(Vector4 *angles)
{
    vu0RotateCurrentZ(angles->z);
    vu0RotateCurrentY(angles->y);
    vu0RotateCurrentX(angles->x);
}

/* Sets the current matrix to a Y rotation, then rotates it by X and Z. */
void vu0SetCurrentRotationYXZ(Vector4 *angles)
{
    vu0SetCurrentRotationY(angles->y);
    vu0RotateCurrentX(angles->x);
    vu0RotateCurrentZ(angles->z);
}

/* Transforms a point by the current matrix; the result stays in vf29. */
void vu0TransformPoint(Vector4 *point)
{
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(point) : "memory");
    __asm__ __volatile__("vmulax.xyzw ACC, vf26, vf1x" : : : "memory");
    __asm__ __volatile__("vmadday.xyzw ACC, vf27, vf1y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyzw ACC, vf28, vf1z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyzw vf29, vf29, vf1w" : : : "memory");
}

/* Multiplies the current matrix by a translation: identity rows 0-2 and offset as row 3. */
void vu0TranslateCurrent(Vector4 *translation)
{
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(vu0IdentityMatrix) : "memory");
    __asm__ __volatile__("lqc2 vf2, 16(%0)" : : "r"(vu0IdentityMatrix) : "memory");
    __asm__ __volatile__("lqc2 vf3, 32(%0)" : : "r"(vu0IdentityMatrix) : "memory");
    __asm__ __volatile__("lqc2 vf4, 0(%0)" : : "r"(translation) : "memory");
    __asm__ __volatile__("vmulax.xyzw ACC, vf1, vf26x" : : : "memory");
    __asm__ __volatile__("vmadday.xyzw ACC, vf2, vf26y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyzw ACC, vf3, vf26z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyzw vf26, vf4, vf26w" : : : "memory");
    __asm__ __volatile__("vmulax.xyzw ACC, vf1, vf27x" : : : "memory");
    __asm__ __volatile__("vmadday.xyzw ACC, vf2, vf27y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyzw ACC, vf3, vf27z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyzw vf27, vf4, vf27w" : : : "memory");
    __asm__ __volatile__("vmulax.xyzw ACC, vf1, vf28x" : : : "memory");
    __asm__ __volatile__("vmadday.xyzw ACC, vf2, vf28y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyzw ACC, vf3, vf28z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyzw vf28, vf4, vf28w" : : : "memory");
    __asm__ __volatile__("vmulax.xyzw ACC, vf1, vf29x" : : : "memory");
    __asm__ __volatile__("vmadday.xyzw ACC, vf2, vf29y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyzw ACC, vf3, vf29z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyzw vf29, vf4, vf29w" : : : "memory");
}

/* Sets matrix to the rotation that turns the Z axis towards `to` - `from` (Y rotation from the
 * heading, then X from the pitch) and returns 1; for equal points it becomes the identity and the
 * result is 0. Rows 2 and 3 hold the angles and the direction until the matrix is stored. */
int vu0SetRotationFromDirection(Matrix4 matrix, Vector4 *targetPoint, Vector4 *sourcePoint)
{
    Vu0Direction *direction = (Vu0Direction *) matrix[3];
    float *angleRow;
    int isRotated = 0;
    int isZeroDirection;
    float horizontalLength;
    float pitch;

    Vu0SubVectorXYZ12(direction, targetPoint, sourcePoint);
    isZeroDirection = (direction->bits.z | direction->bits.xy) == 0;
    if (isZeroDirection) {
        Vu0SetCurrentIdentity(vu0IdentityMatrix);
    } else {
        pitch = -direction->vector.y;
        angleRow = matrix[2];
        __asm__ __volatile__(
            "lqc2 vf1, 0(%1)\n\t"
            "vmul.xz vf1, vf1, vf1\n\t"
            "vaddz.x vf1, vf1, vf1z\n\t"
            "vsqrt Q, vf1x\n\t"
            "sqc2 vf0, 0(%2)\n\t"
            "vwaitq\n\t"
            "vaddq.x vf1, vf0, Q\n\t"
            "qmfc2.ni $8, vf1\n\t"
            "mtc1 $8, %0"
            : "=f"(horizontalLength) : "r"(direction), "r"(angleRow) : "$8", "memory");
        angleRow[0] = srsAtan2(pitch, horizontalLength);
        angleRow[1] = srsAtan2(direction->vector.x, direction->vector.z);
        vu0SetCurrentRotationY(angleRow[1]);
        vu0RotateCurrentX(angleRow[0]);
        isRotated = 1;
    }
    __asm__ __volatile__(
        "sqc2 vf26, 0(%0)\n\t"
        "sqc2 vf27, 16(%0)\n\t"
        "sqc2 vf28, 32(%0)\n\t"
        "sqc2 vf29, 48(%0)"
        : : "r"(matrix) : "memory");
    return isRotated;
}

#include "common.h"
#include "shared.h"

float Get_Decimal_Surplus_for_Radius(float angle);
float Get_Distance(const Vector4 *fromPoint, const Vector4 *toPoint);
float Get_Distance3D(const Vector4 *fromPoint, const Vector4 *toPoint);
float Get_Angle(const Vector4 *fromPoint, const Vector4 *toPoint);

void Get_Point_By_AngleLength(const Vector4 *startPoint, Vector4 *endPoint, float angle, float length);

extern float sinf(float angle);
extern float cosf(float angle);
extern float atan2f(float y, float x);

/* Half and full turn of the angle values used by the Get_/unduCheck angle helpers. */
#define GET_PI 3.1415927f

#define GET_TWO_PI 6.2831855f

float Get_Distance(const Vector4 *fromPoint, const Vector4 *toPoint)
{
    float fromX = fromPoint->x;
    float fromZ = fromPoint->z;

    return __builtin_sqrtf((toPoint->x - fromX) * (toPoint->x - fromX)
                           + (toPoint->z - fromZ) * (toPoint->z - fromZ));
}

float Get_Distance3D(const Vector4 *fromPoint, const Vector4 *toPoint)
{
    float fromX = fromPoint->x;
    float fromY = fromPoint->y;
    float fromZ = fromPoint->z;

    return __builtin_sqrtf((toPoint->x - fromX) * (toPoint->x - fromX)
                           + (toPoint->y - fromY) * (toPoint->y - fromY)
                           + (toPoint->z - fromZ) * (toPoint->z - fromZ));
}

float Get_Angle(const Vector4 *fromPoint, const Vector4 *toPoint)
{
    return atan2f(toPoint->x - fromPoint->x, toPoint->z - fromPoint->z);
}

typedef int s32;

f32 Get_Angle_Difference(Vector4 *fromPoint, Vector4 *toPoint, f32 baseAngle) {
    f32 difference;

    difference = Get_Angle(fromPoint, toPoint) - baseAngle;
    if (difference < -3.1415927f) {
        difference += 6.2831855f;
    }
    if (difference > 3.1415927f) {
        difference -= 6.2831855f;
    }
    return difference;
}

float Get_Decimal_Surplus_for_Radius(float angle)
{
    if (angle <= 0.0f) {
        float negative_pi = -GET_PI;
        if (angle < negative_pi) {
            do {
                angle += GET_TWO_PI;
            } while (angle < negative_pi);
        }
    } else {
        while (GET_PI <= angle) {
            angle -= GET_TWO_PI;
        }
    }
    return angle;
}

s32 xglMatrixStackRotX(f32);
s32 xglMatrixStackRotY(f32);
s32 xglMatrixStackSave();
s32 xglMatrixStackTrans();
s32 xglMatrixStackUnit();
void Get_Point_By_Rotation(s32 *basePosition, Vector4 *rotatedPoint, f32 rotationX, f32 rotationY, f32 offsetZ) {
    f32 matrix[0x10];
    Vector4 zOffsetVector;
    f32 x;
    f32 y;
    f32 z;

    zOffsetVector.w = 1.0f;
    zOffsetVector.z = offsetZ;
    zOffsetVector.x = 0;
    zOffsetVector.y = 0;
    xglMatrixStackUnit();
    xglMatrixStackTrans(basePosition);
    xglMatrixStackRotY(rotationY);
    xglMatrixStackRotX(rotationX);
    xglMatrixStackTrans(&zOffsetVector.x);
    xglMatrixStackSave(&matrix[0]);
    x = matrix[0xC];
    y = matrix[0xD];
    z = matrix[0xE];
    rotatedPoint->z = z;
    rotatedPoint->y = y;
    rotatedPoint->x = x;
}

void Get_Point_By_AngleLength(const Vector4 *startPoint, Vector4 *endPoint,
                              float angle, float length)
{
    endPoint->x = startPoint->x + sinf(angle) * length;
    endPoint->y = startPoint->y;
    endPoint->z = startPoint->z + cosf(angle) * length;
    endPoint->w = startPoint->w;
}

void Get_Point_On_Orbit(Vector4 *orbitCenter, Vector4 *orbitPoint, Vector4 *pointOnOrbit, f32 fraction) {
    f32 radius;
    f32 circumference;
    f32 angle;

    radius = Get_Distance(orbitCenter, orbitPoint);
    circumference = 2.0f * radius * 3.1415927f;
    angle = Get_Angle(orbitCenter, orbitPoint) + ((fraction * 6.2831855f) / circumference);
    pointOnOrbit->x = orbitCenter->x + (sinf(angle) * radius);
    pointOnOrbit->y = orbitPoint->y;
    pointOnOrbit->z = orbitCenter->z + (cosf(angle) * radius);
    pointOnOrbit->w = orbitPoint->w;
}

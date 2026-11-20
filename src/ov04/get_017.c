#include "common.h"
#include "shared.h"

float Get_Decimal_Surplus_for_Radius(float angle);
float Get_Distance(const Vector4 *first, const Vector4 *second);
float Get_Distance3D(const Vector4 *first, const Vector4 *second);
float Get_Angle(const Vector4 *first, const Vector4 *second);

void Get_Point_By_AngleLength(const Vector4 *source, Vector4 *destination, float angle, float length);

extern float sinf(float angle);
extern float cosf(float angle);
extern float atan2f(float y, float x);

/* Half and full turn of the angle values used by the Get_/unduCheck angle helpers. */
#define GET_PI 3.1415927f

#define GET_TWO_PI 6.2831855f

float Get_Distance(const Vector4 *first, const Vector4 *second)
{
    float first_x = first->x;
    float first_z = first->z;

    return __builtin_sqrtf((second->x - first_x) * (second->x - first_x)
                           + (second->z - first_z) * (second->z - first_z));
}

float Get_Distance3D(const Vector4 *first, const Vector4 *second)
{
    float first_x = first->x;
    float first_y = first->y;
    float first_z = first->z;

    return __builtin_sqrtf((second->x - first_x) * (second->x - first_x)
                           + (second->y - first_y) * (second->y - first_y)
                           + (second->z - first_z) * (second->z - first_z));
}

float Get_Angle(const Vector4 *first, const Vector4 *second)
{
    return atan2f(second->x - first->x, second->z - first->z);
}

INCLUDE_ASM("asm/nonmatchings/ov04/get_017", func_00A9F7E0);

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

INCLUDE_ASM("asm/nonmatchings/ov04/get_017", func_00A9F928);

void Get_Point_By_AngleLength(const Vector4 *source, Vector4 *destination,
                              float angle, float length)
{
    destination->x = source->x + sinf(angle) * length;
    destination->y = source->y;
    destination->z = source->z + cosf(angle) * length;
    destination->w = source->w;
}

INCLUDE_ASM("asm/nonmatchings/ov04/get_017", func_00A9FA48);

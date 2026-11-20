#include "common.h"
#include "shared.h"

int Get_Rnd(int min, int max);
int Get_RndFloatRange(float minimum, float maximum);
float Get_Distance(const Vector4 *first, const Vector4 *second);
void Get_One_Step(float length, const Vector4 *source, const Vector4 *destination, Vector4 *output);
void Get_MiddlePoint(const Vector4 *start, const Vector4 *following,
                     short point_index, short point_count, Vector4 *result);
void Get_MiddlePoint_Parabora(const Vector4 *first, const Vector4 *second, int current, int total, float height,
                              Vector4 *destination);
int Check_CrossingOver(const Vector4 *first_start, const Vector4 *first_end,
                       const Vector4 *second_start, const Vector4 *second_end);
int uwamonoQuadEdgesVisible(const Vector4 *origin, const Vector4 *corner0, const Vector4 *corner1,
                            const Vector4 *corner2, const Vector4 *corner3);
unsigned short xglSRand(void);
float xglFRand(void);
int CheckPointLine(const Vector4 *first, const Vector4 *second, const Vector4 *point);
void CalcCrossPoint(const Vector4 *first, const Vector4 *second, const Vector4 *third, const Vector4 *fourth, Vector4 *cross_point);
void CalcVerticalVector(const Vector4 *first, const Vector4 *second, const Vector4 *third, Vector4 *vertical);
void xglVectorInner(float *destination, const Vector4 *left, const Vector4 *right);
void xglVectorOuter(Vector4 *destination, const Vector4 *first, const Vector4 *second);
void xglVectorLength(float *destination, const Vector4 *vector);
void xglVectorNormal(Vector4 *destination, const Vector4 *source);

#include "vu0_macro.h"

int Get_Rnd(int min, int max)
{
    return min + xglSRand() % (max - min + 1);
}

int Get_RndFloatRange(float minimum, float maximum)
{
    return (int) (minimum + (xglFRand() * (maximum - minimum)));
}

INCLUDE_ASM("asm/nonmatchings/ov04/get_019", func_00A9FED8);

void Get_MiddlePoint(const Vector4 *start, const Vector4 *following,
                     short point_index, short point_count, Vector4 *result)
{
    if (point_index == point_count) {
        result->x = following->x;
        result->y = following->y;
        result->z = following->z;
        result->w = 1.0f;
    } else {
        result->x = start->x + (following->x - start->x) * (float)point_index
                    / (float)point_count;
        result->y = start->y + (following->y - start->y) * (float)point_index
                    / (float)point_count;
        result->z = start->z + (following->z - start->z) * (float)point_index
                    / (float)point_count;
        result->w = 1.0f;
    }
}

void Get_MiddlePoint_Parabora(const Vector4 *first, const Vector4 *second,
                              int current, int total, float height,
                              Vector4 *destination)
{
    int current_value = (short)current;
    int total_value = (short)total;
    int midpoint = (int)((short)total +
                         ((unsigned int)(total << 16) >> 31)) >> 1;
    int delta = midpoint - current_value;
    float adjusted_height = height * (float)(delta * delta)
                           / (float)(midpoint * midpoint);
    height -= adjusted_height;

    if (current_value == total_value) {
        destination->x = second->x;
        destination->y = second->y;
        destination->z = second->z;
        destination->w = second->w;
    } else {
        destination->x = first->x + (second->x - first->x)
                       * (float)current_value / (float)total_value;
        destination->y = first->y + (second->y - first->y)
                       * (float)current_value / (float)total_value + height;
        destination->z = first->z + (second->z - first->z)
                       * (float)current_value / (float)total_value;
        destination->w = first->w + (second->w - first->w)
                       * (float)current_value / (float)total_value;
    }
}

float Get_ParaboraValue(int current, int total, float first, float second, float height)
{
    int current_value = (short)current;
    int total_value = (short)total;
    int midpoint = (int)((short)total + ((unsigned int)(total << 16) >> 31)) >> 1;
    int delta = midpoint - current_value;
    float adjusted_height = height * (float)(delta * delta) / (float)(midpoint * midpoint);
    height -= adjusted_height;

    if (current_value == total_value) {
        return second;
    }
    return first + (second - first) * (float)current_value / (float)total_value + height;
}

void Get_One_Step(float length, const Vector4 *source,
                  const Vector4 *destination, Vector4 *output)
{
    float x_step = destination->x - source->x;
    float z_step = destination->z - source->z;
    float inverse_distance = 1.0f / Get_Distance(source, destination);

    output->x = x_step * inverse_distance * length;
    output->y = 0.0f;
    output->z = z_step * inverse_distance * length;
    output->w = 1.0f;
}

/* soft-double helpers at their scaffold labels: double from float, double subtract, double compare */
extern double fptodp(float value);
extern double dpsub(double minuend, double subtrahend);
extern int dpcmp(double first, double second);
float Get_Decimal_Surplus_for_Radius(float angle);

#define GET_TWO_PI 6.2831855f

/* Whether angle lies in the sector from..to, after normalising the three angles with
   Get_Decimal_Surplus_for_Radius; a sector spanning a full turn or more contains every angle. */
int unduCheckAngle(float angle, float from, float to)
{
    double span = fptodp(to - from);
    int inside;

    if (dpcmp(span, 0.0) < 0) {
        span = dpsub(0.0, span);
    }
    if (dpcmp(span, GET_TWO_PI) >= 0) {
        return 1;
    }

    from = Get_Decimal_Surplus_for_Radius(from);
    to = Get_Decimal_Surplus_for_Radius(to);
    angle = Get_Decimal_Surplus_for_Radius(angle);
    if (from <= to) {
        if (from <= angle) {
            return angle <= to;
        }
        return 0;
    }
    if (from <= angle) {
        return 1;
    }
    inside = 0;
    if (angle <= to) {
        inside = 1;
    }
    return inside;
}

typedef struct Ov04Source30 {
    unsigned char unmodeled_00[0x30];
    Vector4 point;
} Ov04Source30;

void vectorCopyPoint(const Ov04Source30 *source, Vector4 *destination)
{
    float z;

    destination->x = source->point.x;
    destination->y = source->point.y;
    z = source->point.z;
    destination->w = 1.0f;
    destination->z = z;
}

typedef struct Ov04Pose {
    float matrix[4][4];
} Ov04Pose;

extern float asinf(float x);
extern float atan2f(float y, float x);
/* soft-double helpers at their scaffold labels: double sqrt, double divide, double to float */
extern double func_00304070(double x);
extern double func_0030A190(double dividend, double divisor);
extern float func_0030A4E0(double x);

void uwamonoPoseAnglesGet(const Ov04Pose *pose, Vector4 *angles)
{
    float roll;

    if (pose->matrix[1][1] >= 0.0f) {
        double sine = pose->matrix[0][1];

        roll = asinf(func_0030A4E0(func_0030A190(sine, func_00304070(1.0f - pose->matrix[2][1] * pose->matrix[2][1]))));
    } else {
        double sine = pose->matrix[0][1];

        roll = 3.1415927f - asinf(func_0030A4E0(func_0030A190(sine, func_00304070(1.0f - pose->matrix[2][1] * pose->matrix[2][1]))));
    }
    angles->x = asinf(pose->matrix[2][1]);
    angles->y = atan2f(pose->matrix[2][0], pose->matrix[2][2]);
    angles->w = 1.0f;
    angles->z = roll;
}

INCLUDE_ASM("asm/nonmatchings/ov04/get_019", func_00AA04E0);

INCLUDE_ASM("asm/nonmatchings/ov04/get_019", func_00AA0560);

/* True when the origin sees the midpoint of each edge of the quad (corner0..corner3) without that
   ray crossing another edge: the four midpoints are tested against the three other edges. */
int uwamonoQuadEdgesVisible(const Vector4 *origin, const Vector4 *corner0, const Vector4 *corner1,
                  const Vector4 *corner2, const Vector4 *corner3)
{
    Vector4 edge_midpoints[4];

    Get_MiddlePoint(corner0, corner1, 1, 2, &edge_midpoints[0]);
    Get_MiddlePoint(corner1, corner2, 1, 2, &edge_midpoints[1]);
    Get_MiddlePoint(corner2, corner3, 1, 2, &edge_midpoints[2]);
    Get_MiddlePoint(corner3, corner0, 1, 2, &edge_midpoints[3]);

    if (Check_CrossingOver(origin, &edge_midpoints[0], corner1, corner2) == 1)
        return 0;
    if (Check_CrossingOver(origin, &edge_midpoints[0], corner2, corner3) == 1)
        return 0;
    if (Check_CrossingOver(origin, &edge_midpoints[0], corner3, corner0) == 1)
        return 0;
    if (Check_CrossingOver(origin, &edge_midpoints[1], corner2, corner3) == 1)
        return 0;
    if (Check_CrossingOver(origin, &edge_midpoints[1], corner3, corner0) == 1)
        return 0;
    if (Check_CrossingOver(origin, &edge_midpoints[1], corner0, corner1) == 1)
        return 0;
    if (Check_CrossingOver(origin, &edge_midpoints[2], corner3, corner0) == 1)
        return 0;
    if (Check_CrossingOver(origin, &edge_midpoints[2], corner0, corner1) == 1)
        return 0;
    if (Check_CrossingOver(origin, &edge_midpoints[2], corner1, corner2) == 1)
        return 0;
    if (Check_CrossingOver(origin, &edge_midpoints[3], corner0, corner1) == 1)
        return 0;
    if (Check_CrossingOver(origin, &edge_midpoints[3], corner1, corner2) == 1)
        return 0;
    return Check_CrossingOver(origin, &edge_midpoints[3], corner2, corner3) != 1;
}

void CalcVerticalVector(const Vector4 *first, const Vector4 *second,
                        const Vector4 *third, Vector4 *vertical)
{
    Vector4 first_edge;
    Vector4 second_edge;
    Vector4 normal;

    Vu0SubVectorXYZ(&first_edge, second, first);
    Vu0SubVectorXYZ(&second_edge, third, first);
    xglVectorOuter(&normal, &first_edge, &second_edge);
    xglVectorOuter(vertical, &first_edge, &normal);
}

float CalcLength(const Vector4 *first, const Vector4 *second, const Vector4 *third)
{
    Vector4 vertical;
    Vector4 delta;
    float length;

    CalcVerticalVector(first, second, third, &vertical);
    xglVectorNormal(&vertical, &vertical);
    Vu0SubVectorXYZ(&delta, first, third);
    xglVectorInner(&length, &vertical, &delta);
    return length;
}

/* Side of the XZ directed line first->second that point lies on: 1 left (cross > 0), 2 right (cross < 0), 0 on the line. */
int CheckPointLine(const Vector4 *first, const Vector4 *second,
                   const Vector4 *point)
{
    Vector4 first_delta;
    Vector4 second_delta;
    float cross;
    float first_x = first->x;
    int side = 1;

    first_delta.x = second->x - first_x;
    second_delta.x = point->x - first_x;
    first_delta.z = second->z - first->z;
    second_delta.z = point->z - first->z;
    cross = first_delta.x * second_delta.z
          - first_delta.z * second_delta.x;

    if (!(0.0f < cross)) {
        if (cross < 0.0f) {
            side = 2;
        } else {
            side = 0;
        }
    }
    return side;
}

int Check_CrossingOver(const Vector4 *first_start, const Vector4 *first_end,
                       const Vector4 *second_start, const Vector4 *second_end)
{
    int first_start_side = CheckPointLine(first_start, first_end, second_start);
    int first_end_side = CheckPointLine(first_start, first_end, second_end);

    if (first_start_side != first_end_side) {
        int second_start_side = CheckPointLine(second_start, second_end, first_start);
        int second_end_side = CheckPointLine(second_start, second_end, first_end);

        if (second_start_side != second_end_side)
            return 1;
    }
    return 0;
}

void CalcCrossPoint(const Vector4 *first, const Vector4 *second,
                    const Vector4 *third, const Vector4 *fourth,
                    Vector4 *cross_point)
{
    Vector4 normal;
    /* Reused pair: [1] holds the plane length first, then the length of cross_point;
       [0] holds the dot product of the edge with the normal, then the scale ratio. */
    float intersection_scale[2];

    intersection_scale[1] = CalcLength(first, second, third);
    CalcVerticalVector(first, second, third, &normal);
    xglVectorNormal(&normal, &normal);
    Vu0SubVectorXYZ(cross_point, fourth, third);
    xglVectorInner(&intersection_scale[0], cross_point, &normal);
    intersection_scale[0] = intersection_scale[1] / intersection_scale[0];
    xglVectorLength(&intersection_scale[1], cross_point);
    xglVectorNormal(cross_point, cross_point);
    cross_point->x *= intersection_scale[0] * intersection_scale[1];
    cross_point->y *= intersection_scale[0] * intersection_scale[1];
    cross_point->z *= intersection_scale[0] * intersection_scale[1];
    /* VU0 add: the third point's position is added to the scaled offset in place. */
    __asm__ __volatile__(
        "lqc2 vf20, 0(%0)\n\t"
        "lqc2 vf21, 0(%1)\n\t"
        "vadd.xyzw vf20, vf20, vf21\n\t"
        "sqc2 vf20, 0(%0)"
        :
        : "r"(cross_point), "r"(third)
        : "memory");
}

int CheckCrossCircle(const Vector4 *start, const Vector4 *end,
                     const Vector4 *center, float radius)
{
    Vector4 middle;
    Vector4 direction;
    float projection_x;
    float half_length;
    float along;
    float x;
    float z;
    int result;

    direction.x = end->x - start->x;
    direction.y = 0.0f;
    direction.z = end->z - start->z;
    direction.w = 1.0f;
    half_length = __builtin_sqrtf(direction.x * direction.x
                                  + direction.z * direction.z) * 0.5f;
    xglVectorNormal(&direction, &direction);
    half_length += radius;
    result = 0;
    middle.x = start->x + (end->x - start->x) * 0.5f;
    middle.z = start->z + (end->z - start->z) * 0.5f;
    x = center->x - middle.x;
    z = center->z - middle.z;
    along = x * direction.x + z * direction.z;
    if (!(half_length < __builtin_fabsf(along))) {
        projection_x = along * direction.x;
        x = center->x - (projection_x + middle.x);
        z = center->z - (along * direction.z + middle.z);
        result = x * x + z * z < radius * radius;
    }
    return result;
}

#include "common.h"
#include "shared.h"

int Get_Rnd(int minimum, int maximum);
int Get_RndFloatRange(float minimum, float maximum);
float Get_Distance(const Vector4 *first, const Vector4 *second);
void Get_One_Step(float length, const Vector4 *fromPoint, const Vector4 *toPoint, Vector4 *stepVector);
void Get_MiddlePoint(const Vector4 *fromPoint, const Vector4 *toPoint,
                     short point_index, short point_count, Vector4 *interpolated);
void Get_MiddlePoint_Parabora(const Vector4 *startPoint, const Vector4 *endPoint, int arcStepIndex, int arcStepCount, float height,
                              Vector4 *arcPoint);
int Check_CrossingOver(const Vector4 *firstLineStart, const Vector4 *firstLineEnd,
                       const Vector4 *secondLineStart, const Vector4 *secondLineEnd);
int uwamonoQuadEdgesVisible(const Vector4 *origin, const Vector4 *cornerA, const Vector4 *cornerB,
                            const Vector4 *cornerC, const Vector4 *cornerD);
unsigned short xglSRand(void);
float xglFRand(void);
int CheckPointLine(const Vector4 *lineStart, const Vector4 *lineEnd, const Vector4 *point);
void CalcCrossPoint(const Vector4 *vertexA, const Vector4 *vertexB, const Vector4 *vertexC, const Vector4 *lineEnd, Vector4 *cross_point);
void CalcVerticalVector(const Vector4 *vertexA, const Vector4 *vertexB, const Vector4 *vertexC, Vector4 *vertical);
void xglVectorInner(float *destination, const Vector4 *left, const Vector4 *right);
void xglVectorOuter(Vector4 *destination, const Vector4 *first, const Vector4 *second);
void xglVectorLength(float *destination, const Vector4 *vector);
void xglVectorNormal(Vector4 *destination, const Vector4 *source);

#include "vu0_macro.h"

int Get_Rnd(int minimum, int maximum)
{
    return minimum + xglSRand() % (maximum - minimum + 1);
}

int Get_RndFloatRange(float minimum, float maximum)
{
    return minimum + (xglFRand() * (maximum - minimum));
}

typedef int s32;

/* Slot 0 of the moving-actor table whose pointer is stored in gMovingActorSlots (see ov04/tu002): the seeded range takes the fraction of pos_x + pos_z as its random draw. */
typedef struct MovingActorSlotPosition {
    unsigned char unk_00[0x10];
    f32 pos_x;             /* +0x10 */
    unsigned char unk_14[0x18 - 0x14];
    f32 pos_z;            /* +0x18 */
} MovingActorSlotPosition;

extern MovingActorSlotPosition *gMovingActorSlots;

s32 actorPositionSeededRange(f32 lowBound, f32 highBound) {
    f32 seedPositionSum;

    seedPositionSum = gMovingActorSlots->pos_x + gMovingActorSlots->pos_z;
    return lowBound + ((seedPositionSum - (f32) (s32) seedPositionSum) * (highBound - lowBound));
}

void Get_MiddlePoint(const Vector4 *fromPoint, const Vector4 *toPoint,
                     short point_index, short point_count, Vector4 *interpolated)
{
    if (point_index == point_count) {
        interpolated->x = toPoint->x;
        interpolated->y = toPoint->y;
        interpolated->z = toPoint->z;
        interpolated->w = 1.0f;
    } else {
        interpolated->x = fromPoint->x + (toPoint->x - fromPoint->x) * (float)point_index
                    / (float)point_count;
        interpolated->y = fromPoint->y + (toPoint->y - fromPoint->y) * (float)point_index
                    / (float)point_count;
        interpolated->z = fromPoint->z + (toPoint->z - fromPoint->z) * (float)point_index
                    / (float)point_count;
        interpolated->w = 1.0f;
    }
}

void Get_MiddlePoint_Parabora(const Vector4 *startPoint, const Vector4 *endPoint,
                              int arcStepIndex, int arcStepCount, float height,
                              Vector4 *arcPoint)
{
    short arcStepIndexS16 = arcStepIndex;
    short arcStepCountS16 = arcStepCount;
    int midpoint = (int)(arcStepCountS16 +
                         ((unsigned int)(arcStepCount << 16) >> 31)) >> 1;
    int delta = midpoint - arcStepIndexS16;
    float adjusted_height = height * (float)(delta * delta)
                           / (float)(midpoint * midpoint);
    height -= adjusted_height;

    if (arcStepIndexS16 == arcStepCountS16) {
        arcPoint->x = endPoint->x;
        arcPoint->y = endPoint->y;
        arcPoint->z = endPoint->z;
        arcPoint->w = endPoint->w;
    } else {
        arcPoint->x = startPoint->x + (endPoint->x - startPoint->x)
                       * (float)arcStepIndexS16 / (float)arcStepCountS16;
        arcPoint->y = startPoint->y + (endPoint->y - startPoint->y)
                       * (float)arcStepIndexS16 / (float)arcStepCountS16 + height;
        arcPoint->z = startPoint->z + (endPoint->z - startPoint->z)
                       * (float)arcStepIndexS16 / (float)arcStepCountS16;
        arcPoint->w = startPoint->w + (endPoint->w - startPoint->w)
                       * (float)arcStepIndexS16 / (float)arcStepCountS16;
    }
}

float Get_ParaboraValue(int arcStepIndex, int arcStepCount, float arcStartValue, float arcEndValue, float height)
{
    short arcStepIndexS16 = arcStepIndex;
    short arcStepCountS16 = arcStepCount;
    int midpoint = (int)(arcStepCountS16 + ((unsigned int)(arcStepCount << 16) >> 31)) >> 1;
    int delta = midpoint - arcStepIndexS16;
    float adjusted_height = height * (float)(delta * delta) / (float)(midpoint * midpoint);
    height -= adjusted_height;

    if (arcStepIndexS16 == arcStepCountS16) {
        return arcEndValue;
    }
    return arcStartValue + (arcEndValue - arcStartValue) * (float)arcStepIndexS16 / (float)arcStepCountS16 + height;
}

void Get_One_Step(float length, const Vector4 *fromPoint,
                  const Vector4 *toPoint, Vector4 *stepVector)
{
    float x_step = toPoint->x - fromPoint->x;
    float z_step = toPoint->z - fromPoint->z;
    float inverse_distance = 1.0f / Get_Distance(fromPoint, toPoint);

    stepVector->x = x_step * inverse_distance * length;
    stepVector->y = 0.0f;
    stepVector->z = z_step * inverse_distance * length;
    stepVector->w = 1.0f;
}

/* soft-double helpers at their scaffold labels: double from float, double subtract, double compare */
extern double fptodp(float value);
extern double dpsub(double minuend, double subtrahend);
extern int dpcmp(double first, double second);
float Get_Decimal_Surplus_for_Radius(float angle);

#define GET_TWO_PI 6.2831855f

/* Whether angle lies in the sector from..to, after normalising the three angles with
   Get_Decimal_Surplus_for_Radius; a sector spanning a full turn or more contains every angle. */
int unduCheckAngle(float angle, float fromAngle, float toAngle)
{
    double sectorSpan = fptodp(toAngle - fromAngle);
    int inside;

    if (dpcmp(sectorSpan, 0.0) < 0) {
        sectorSpan = dpsub(0.0, sectorSpan);
    }
    if (dpcmp(sectorSpan, GET_TWO_PI) >= 0) {
        return 1;
    }

    fromAngle = Get_Decimal_Surplus_for_Radius(fromAngle);
    toAngle = Get_Decimal_Surplus_for_Radius(toAngle);
    angle = Get_Decimal_Surplus_for_Radius(angle);
    if (fromAngle <= toAngle) {
        if (fromAngle <= angle) {
            return angle <= toAngle;
        }
        return 0;
    }
    if (fromAngle <= angle) {
        return 1;
    }
    inside = 0;
    if (angle <= toAngle) {
        inside = 1;
    }
    return inside;
}

typedef struct SourceWithPoint {
    unsigned char unmodeled_00[0x30];
    Vector4 point;
} SourceWithPoint;

void vectorCopyPoint(const SourceWithPoint *pointHolder, Vector4 *copiedPoint)
{
    float z;

    copiedPoint->x = pointHolder->point.x;
    copiedPoint->y = pointHolder->point.y;
    z = pointHolder->point.z;
    copiedPoint->w = 1.0f;
    copiedPoint->z = z;
}

typedef struct PoseMatrix {
    float matrix[4][4];
} PoseMatrix;

extern float asinf(float x);
extern float atan2f(float y, float x);
/* soft-double helpers at their scaffold labels: double sqrt, double divide, double to float */
extern double func_00304070(double x);
extern double func_0030A190(double dividend, double divisor);
extern float func_0030A4E0(double x);

void uwamonoPoseAnglesGet(const PoseMatrix *pose, Vector4 *angles)
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

s32 Check_RangeOverlap(f32 leftMin, f32 leftMax, f32 rightMin, f32 rightMax) {
    s32 overlaps;

    overlaps = 0;
    if (!(leftMax < rightMin)) {
        if (!(rightMax < leftMin)) {
            overlaps = 1;
        }
    }
    return overlaps;
}

/* True when the origin sees the midpoint of each edge of the quad (cornerA..cornerD) without that
   ray crossing another edge: the four midpoints are tested against the three other edges. */
int uwamonoQuadEdgesVisible(const Vector4 *origin, const Vector4 *cornerA, const Vector4 *cornerB,
                  const Vector4 *cornerC, const Vector4 *cornerD)
{
    Vector4 edge_midpoints[4];

    Get_MiddlePoint(cornerA, cornerB, 1, 2, &edge_midpoints[0]);
    Get_MiddlePoint(cornerB, cornerC, 1, 2, &edge_midpoints[1]);
    Get_MiddlePoint(cornerC, cornerD, 1, 2, &edge_midpoints[2]);
    Get_MiddlePoint(cornerD, cornerA, 1, 2, &edge_midpoints[3]);

    if (Check_CrossingOver(origin, &edge_midpoints[0], cornerB, cornerC) == 1)
        return 0;
    if (Check_CrossingOver(origin, &edge_midpoints[0], cornerC, cornerD) == 1)
        return 0;
    if (Check_CrossingOver(origin, &edge_midpoints[0], cornerD, cornerA) == 1)
        return 0;
    if (Check_CrossingOver(origin, &edge_midpoints[1], cornerC, cornerD) == 1)
        return 0;
    if (Check_CrossingOver(origin, &edge_midpoints[1], cornerD, cornerA) == 1)
        return 0;
    if (Check_CrossingOver(origin, &edge_midpoints[1], cornerA, cornerB) == 1)
        return 0;
    if (Check_CrossingOver(origin, &edge_midpoints[2], cornerD, cornerA) == 1)
        return 0;
    if (Check_CrossingOver(origin, &edge_midpoints[2], cornerA, cornerB) == 1)
        return 0;
    if (Check_CrossingOver(origin, &edge_midpoints[2], cornerB, cornerC) == 1)
        return 0;
    if (Check_CrossingOver(origin, &edge_midpoints[3], cornerA, cornerB) == 1)
        return 0;
    if (Check_CrossingOver(origin, &edge_midpoints[3], cornerB, cornerC) == 1)
        return 0;
    return Check_CrossingOver(origin, &edge_midpoints[3], cornerC, cornerD) != 1;
}

void CalcVerticalVector(const Vector4 *vertexA, const Vector4 *vertexB,
                        const Vector4 *vertexC, Vector4 *vertical)
{
    Vector4 first_edge;
    Vector4 second_edge;
    Vector4 normal;

    Vu0SubVectorXYZ(&first_edge, vertexB, vertexA);
    Vu0SubVectorXYZ(&second_edge, vertexC, vertexA);
    xglVectorOuter(&normal, &first_edge, &second_edge);
    xglVectorOuter(vertical, &first_edge, &normal);
}

float CalcLength(const Vector4 *vertexA, const Vector4 *vertexB, const Vector4 *vertexC)
{
    Vector4 vertical;
    Vector4 delta;
    float length;

    CalcVerticalVector(vertexA, vertexB, vertexC, &vertical);
    xglVectorNormal(&vertical, &vertical);
    Vu0SubVectorXYZ(&delta, vertexA, vertexC);
    xglVectorInner(&length, &vertical, &delta);
    return length;
}

/* Side of the XZ directed line first->second that point lies on: 1 left (cross > 0), 2 right (cross < 0), 0 on the line. */
int CheckPointLine(const Vector4 *lineStart, const Vector4 *lineEnd,
                   const Vector4 *point)
{
    Vector4 lineDelta;
    Vector4 pointDelta;
    float cross;
    float lineStartX = lineStart->x;
    int side = 1;

    lineDelta.x = lineEnd->x - lineStartX;
    pointDelta.x = point->x - lineStartX;
    lineDelta.z = lineEnd->z - lineStart->z;
    pointDelta.z = point->z - lineStart->z;
    cross = lineDelta.x * pointDelta.z
          - lineDelta.z * pointDelta.x;

    if (!(0.0f < cross)) {
        if (cross < 0.0f) {
            side = 2;
        } else {
            side = 0;
        }
    }
    return side;
}

int Check_CrossingOver(const Vector4 *firstLineStart, const Vector4 *firstLineEnd,
                       const Vector4 *secondLineStart, const Vector4 *secondLineEnd)
{
    int first_start_side = CheckPointLine(firstLineStart, firstLineEnd, secondLineStart);
    int first_end_side = CheckPointLine(firstLineStart, firstLineEnd, secondLineEnd);

    if (first_start_side != first_end_side) {
        int second_start_side = CheckPointLine(secondLineStart, secondLineEnd, firstLineStart);
        int second_end_side = CheckPointLine(secondLineStart, secondLineEnd, firstLineEnd);

        if (second_start_side != second_end_side)
            return 1;
    }
    return 0;
}

void CalcCrossPoint(const Vector4 *vertexA, const Vector4 *vertexB,
                    const Vector4 *vertexC, const Vector4 *lineEnd,
                    Vector4 *cross_point)
{
    Vector4 normal;
    /* Reused pair: [1] holds the plane length first, then the length of cross_point;
       [0] holds the dot product of the edge with the normal, then the scale ratio. */
    float intersection_scale[2];

    intersection_scale[1] = CalcLength(vertexA, vertexB, vertexC);
    CalcVerticalVector(vertexA, vertexB, vertexC, &normal);
    xglVectorNormal(&normal, &normal);
    Vu0SubVectorXYZ(cross_point, lineEnd, vertexC);
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
        : "r"(cross_point), "r"(vertexC)
        : "memory");
}

int CheckCrossCircle(const Vector4 *lineStart, const Vector4 *lineEnd,
                     const Vector4 *center, float radius)
{
    Vector4 middle;
    Vector4 direction;
    float projection_x;
    float half_length;
    float along;
    float x;
    float z;
    int crossesCircle;

    direction.x = lineEnd->x - lineStart->x;
    direction.y = 0.0f;
    direction.z = lineEnd->z - lineStart->z;
    direction.w = 1.0f;
    half_length = __builtin_sqrtf(direction.x * direction.x
                                  + direction.z * direction.z) * 0.5f;
    xglVectorNormal(&direction, &direction);
    half_length += radius;
    crossesCircle = 0;
    middle.x = lineStart->x + (lineEnd->x - lineStart->x) * 0.5f;
    middle.z = lineStart->z + (lineEnd->z - lineStart->z) * 0.5f;
    x = center->x - middle.x;
    z = center->z - middle.z;
    along = x * direction.x + z * direction.z;
    if (!(half_length < __builtin_fabsf(along))) {
        projection_x = along * direction.x;
        x = center->x - (projection_x + middle.x);
        z = center->z - (along * direction.z + middle.z);
        crossesCircle = x * x + z * z < radius * radius;
    }
    return crossesCircle;
}

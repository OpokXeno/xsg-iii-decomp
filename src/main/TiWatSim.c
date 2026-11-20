#include "common.h"
#include "shared.h"

/* Row-major 4x4 float matrix. */

#include "main/TiWatSim.h"
#include "main/xgl_2.h"


unsigned short xglSRand(void);

void xglVectorMulMat(Vector4 *destination, Matrix4 matrix, const Vector4 *vector);



void tiWatSimStepRain(TiWatSimItem *item);
void tiWatSimRandomizeItem(TiWatSimItem *item, int range);
void tiWatSimDisturbAll(TiWaterSub *table, Vector4 *pPos);
void tiWatSimStoreSourcePos(const Vector4 *position);

#include "quadword.h"

extern int tiWatSimRainCountdown;

/* Rain countdown: a new drop lands 10 to 29 updates after the previous one. */
#define TIWATSIM_RAIN_MIN_WAIT 10

#define TIWATSIM_RAIN_WAIT_SPREAD 20

void assertFailed(const char *message, const char *file, int line);

#define NULL ((void *) 0)

extern const char tiWatSimWaterAssertText[];
extern const char tiWatSimPosAssertText[];
extern const char tiWatSimSourceFile[];
extern Vector4 tiWatSimSourcePos;
extern Vector4 tiWatSimSecondSourcePos;
extern const char tiWatSimWaterAssertText[8];
extern const char tiWatSimSourceFile[24];
extern const char tiWatSimPosAssertText[8];

INCLUDE_ASM("asm/main/nonmatchings/TiWatSim", func_0024CA00);

/* Normal of each interior cell from the height differences of buffer B, scaled by the cell spacing. */
void tiWatSimComputeNormals(TiWatSimItem *item)
{
    DoublewordVector4 *normals; /* stores through the union carry alias set 0, like the original */
    u32 x;
    u32 y;
    u32 index;
    u32 row;
    float dx;
    float dy;
    float dz;
    float length_squared;
    float inverse;

    normals = (DoublewordVector4 *) (item->block + item->normals_offset);
    for (y = 1; y < item->rows - 1; y++) {
        row = y * item->cols;
        for (x = 1; x < item->cols - 1; x++) {
            index = row + x;
            dx = item->height_buffer_b[index - 1] - item->height_buffer_b[index + 1];
            normals[index].vector.x = dx;
            dz = item->height_buffer_b[row - item->cols + x] - item->height_buffer_b[row + item->cols + x];
            normals[index].vector.z = dz;
            dy = item->cell_spacing + item->cell_spacing;
            length_squared = dx * dx + dy * dy + dz * dz;
            inverse = 1.0f / length_squared;
            normals[index].vector.x = dx * inverse;
            normals[index].vector.y = dy * inverse;
            normals[index].vector.z = dz * inverse;
        }
    }
}

/* Rain: when the countdown is zero, one interior cell of the item loses drop_amount * 4.0f and
 * the countdown is set again. */
void tiWatSimStepRain(TiWatSimItem *item)
{
    int col;
    int row;

    if (tiWatSimRainCountdown == 0) {
        col = xglSRand() % item->cols;
        row = xglSRand() % item->rows;
        if (col != 0 && row != 0 && col != item->cols - 1 && row != item->rows - 1) {
            item->height_buffer_a[row * item->cols + col] -= item->drop_amount * 4.0f;
        }
        tiWatSimRainCountdown = xglSRand() % TIWATSIM_RAIN_WAIT_SPREAD + TIWATSIM_RAIN_MIN_WAIT;
    }
    tiWatSimRainCountdown--;
}

/* Pushes the height buffer down at the grid cell under pPos: the cell loses drop_amount, its four neighbours half of it. */
void tiWatSimDisturbAt(TiWatSimItem *item, Vector4 *pPos)
{
    Vector4 pos;
    float half;
    float *mask;
    int col;
    int row;
    int index;

    half = item->drop_amount * 0.5f;
    mask = (float *) (item->block + item->mask_offset);
    if (item == NULL) {
        assertFailed(tiWatSimWaterAssertText, tiWatSimSourceFile, 218);
    }
    if (pPos == NULL) {
        assertFailed(tiWatSimPosAssertText, tiWatSimSourceFile, 219);
    }
    xglVectorMulMat(&pos, item->matrix.rows, pPos);
    pos.x = pos.x - item->grid_origin_x;
    pos.z = pos.z - item->grid_origin_z;
    col = (int) (pos.x / item->cell_spacing);
    row = (int) (pos.z / -item->cell_spacing);
    if (col > 0 && col < item->cols - 1 && row > 0 && row < item->rows - 1) {
        index = row * item->cols + col;
        if (mask[index] != 0.0f) {
            item->height_buffer_a[index] = item->height_buffer_a[index] - item->drop_amount;
            if (col - 1 >= 0) {
                item->height_buffer_a[index - 1] = item->height_buffer_a[index - 1] - half;
            }
            if (row - 1 >= 0) {
                item->height_buffer_a[(row - 1) * item->cols + col] = item->height_buffer_a[(row - 1) * item->cols + col] - half;
            }
            if (col + 1 < item->cols) {
                item->height_buffer_a[index + 1] = item->height_buffer_a[index + 1] - half;
            }
            if (row + 1 < item->rows) {
                item->height_buffer_a[(row + 1) * item->cols + col] = item->height_buffer_a[(row + 1) * item->cols + col] - half;
            }
        }
    }
}

/* Disturbs every item of the table at the position pointed to by pPos. */
void tiWatSimDisturbAll(TiWaterSub *table, Vector4 *pPos)
{
    TiWatSimItem *item;
    u32 i;

    item = table->items;
    for (i = 0; i < table->active; i++) {
        tiWatSimDisturbAt(item, pPos);
        item++;
    }
}

/* Gives each masked cell of the item a random height in a band scaled by range and clears buffer B. */
void tiWatSimRandomizeItem(TiWatSimItem *item, int range)
{
    float *mask;
    u32 i;

    mask = (float *) (item->block + item->mask_offset);
    for (i = 0; i < item->cell_count; i++) {
        if (mask[i] != 0.0f) {
            item->height_buffer_a[i] = (float) (xglSRand() % range) * 0.01f - (float) range * 0.005f;
            item->height_buffer_b[i] = 0.0f;
        }
    }
}

/* Randomizes every item of the table with the same range. */
void tiWatSimRandomizeAll(TiWaterSub *table, int range)
{
    TiWatSimItem *item;
    u32 i;

    item = table->items;
    for (i = 0; i < table->active; i++) {
        tiWatSimRandomizeItem(item, range);
        item++;
    }
}

void tiWatSimStoreSourcePos(const Vector4 *position)
{
    QuadwordCopy(&tiWatSimSourcePos, position);
}

void tiWatSimStoreSecondSourcePos(const Vector4 *position)
{
    QuadwordCopy(&tiWatSimSecondSourcePos, position);
}

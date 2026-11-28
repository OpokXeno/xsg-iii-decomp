#include "common.h"
#include "shared.h"
/* Row-major 4x4 float matrix. */
#include "main/TiWatSim.h"
#include "main/xgl_2.h"

unsigned short xglSRand(void);
void xglVectorMulMat(Vector4 *destination, Matrix4 matrix, const Vector4 *vector);
void tiWatSimStepRain(TiWatSimItem *waterGrid);
void tiWatSimRandomizeItem(TiWatSimItem *waterGrid, int heightRange);
void tiWatSimDisturbAll(TiWaterSub *waterSub, Vector4 *pPos);
void tiWatSimStoreSourcePos(const Vector4 *position);

#include "quadword.h"

#include "main/tu007.h"
extern int tiWatSimRainCountdown;

/* Rain countdown: a new drop lands 10 to 29 updates after the previous one. */
#define TIWATSIM_RAIN_MIN_WAIT 10
#define TIWATSIM_RAIN_WAIT_SPREAD 20

void assertFailed(const char *message, const char *file, int line);

#define NULL ((void *) 0)

extern Vector4 tiWatSimSourcePos;
extern Vector4 tiWatSimSecondSourcePos;
extern PadRecord PadData[];
static u32 tiWatSimCursorRow;
static u32 tiWatSimCursorCol;

/* Water cursor: the held L2 button moves the cursor cell (random cell on press, otherwise one cell per d-pad step within
 * the interior) and lowers that cell of height_buffer_a by drop_amount. */
void tiWatSimDropAtCursor(TiWatSimItem *item) {
    u32 cell_index;

    if (!(PadData[0].held & 0x100)) {
        return;
    }
    if (PadData[0].pressed & 0x100) {
        do {
            tiWatSimCursorRow = xglSRand() % (item->rows - 1);
        } while (tiWatSimCursorRow == 0);
        do {
            tiWatSimCursorCol = xglSRand() % (item->cols - 1);
        } while (tiWatSimCursorCol == 0);
    } else {
        /* kept: the pointer locals decide the emitted store order (STRICT). */
        if (PadData[0].held & 0x2000) {
            u32 *colCursor = &tiWatSimCursorCol;

            if (tiWatSimCursorCol < item->cols - 2) {
                *colCursor = ++tiWatSimCursorCol;
            }
        }
        if (PadData[0].held & 0x8000) {
            u32 *colCursor = &tiWatSimCursorCol;

            if (tiWatSimCursorCol >= 2) {
                *colCursor = --tiWatSimCursorCol;
            }
        }
        if (PadData[0].held & 0x4000) {
            u32 *rowCursor = &tiWatSimCursorRow;

            if (tiWatSimCursorRow < item->rows - 2) {
                *rowCursor = ++tiWatSimCursorRow;
            }
        }
        if (PadData[0].held & 0x1000) {
            u32 *rowCursor = &tiWatSimCursorRow;

            if (tiWatSimCursorRow >= 2) {
                *rowCursor = --tiWatSimCursorRow;
            }
        }
    }
    cell_index = tiWatSimCursorRow * item->cols + tiWatSimCursorCol;
    item->height_buffer_a[cell_index] = item->height_buffer_a[cell_index] - item->drop_amount;
}

/* Normal of each interior cell from the height differences of buffer B, scaled by the cell spacing. */
void tiWatSimComputeNormals(TiWatSimItem *waterGrid)
{
    DoublewordVector4 *normals; /* stores through the union carry alias set 0, like the original */
    u32 x;
    u32 y;
    u32 cellIndex;
    u32 row;
    float dx;
    float dy;
    float dz;
    float length_squared;
    float inverse;

    normals = (DoublewordVector4 *) (waterGrid->block + waterGrid->normals_offset);
    for (y = 1; y < waterGrid->rows - 1; y++) {
        row = y * waterGrid->cols;
        for (x = 1; x < waterGrid->cols - 1; x++) {
            cellIndex = row + x;
            dx = waterGrid->height_buffer_b[cellIndex - 1] - waterGrid->height_buffer_b[cellIndex + 1];
            normals[cellIndex].vector.x = dx;
            dz = waterGrid->height_buffer_b[row - waterGrid->cols + x] - waterGrid->height_buffer_b[row + waterGrid->cols + x];
            normals[cellIndex].vector.z = dz;
            dy = waterGrid->cell_spacing + waterGrid->cell_spacing;
            length_squared = dx * dx + dy * dy + dz * dz;
            inverse = 1.0f / length_squared;
            normals[cellIndex].vector.x = dx * inverse;
            normals[cellIndex].vector.y = dy * inverse;
            normals[cellIndex].vector.z = dz * inverse;
        }
    }
}

/* Rain: when the countdown is zero, one interior cell of the waterGrid loses drop_amount * 4.0f and
 * the countdown is set again. */
void tiWatSimStepRain(TiWatSimItem *waterGrid)
{
    int col;
    int row;

    if (tiWatSimRainCountdown == 0) {
        col = xglSRand() % waterGrid->cols;
        row = xglSRand() % waterGrid->rows;
        if (col != 0 && row != 0 && col != waterGrid->cols - 1 && row != waterGrid->rows - 1) {
            waterGrid->height_buffer_a[row * waterGrid->cols + col] -= waterGrid->drop_amount * 4.0f;
        }
        tiWatSimRainCountdown = xglSRand() % TIWATSIM_RAIN_WAIT_SPREAD + TIWATSIM_RAIN_MIN_WAIT;
    }
    tiWatSimRainCountdown--;
}

/* Pushes the height buffer down at the grid cell under pPos: the cell loses drop_amount, its four neighbours half of it. */
void tiWatSimDisturbAt(TiWatSimItem *waterGrid, Vector4 *pPos) {
    Vector4 pos;
    float neighbourDrop;
    float *mask;
    int col;
    int row;
    int cellIndex;

    neighbourDrop = waterGrid->drop_amount * 0.5f;
    mask = (float *) (waterGrid->block + waterGrid->mask_offset);
    if (waterGrid == NULL) {
        assertFailed("pWater", "TiWatSim.euc.c", 218);
    }
    if (pPos == NULL) {
        assertFailed("pPos", "TiWatSim.euc.c", 219);
    }
    xglVectorMulMat(&pos, waterGrid->matrix.rows, pPos);
    pos.x = pos.x - waterGrid->grid_origin_x;
    pos.z = pos.z - waterGrid->grid_origin_z;
    col = (int) (pos.x / waterGrid->cell_spacing);
    row = (int) (pos.z / -waterGrid->cell_spacing);
    if (col > 0 && col < waterGrid->cols - 1 && row > 0 && row < waterGrid->rows - 1) {
        cellIndex = row * waterGrid->cols + col;
        if (mask[cellIndex] != 0.0f) {
            waterGrid->height_buffer_a[cellIndex] =
                waterGrid->height_buffer_a[cellIndex] - waterGrid->drop_amount;
            if (col - 1 >= 0) {
                waterGrid->height_buffer_a[cellIndex - 1] =
                    waterGrid->height_buffer_a[cellIndex - 1] - neighbourDrop;
            }
            if (row - 1 >= 0) {
                waterGrid->height_buffer_a[(row - 1) * waterGrid->cols + col] =
                    waterGrid->height_buffer_a[(row - 1) * waterGrid->cols + col] - neighbourDrop;
            }
            if (col + 1 < waterGrid->cols) {
                waterGrid->height_buffer_a[cellIndex + 1] =
                    waterGrid->height_buffer_a[cellIndex + 1] - neighbourDrop;
            }
            if (row + 1 < waterGrid->rows) {
                waterGrid->height_buffer_a[(row + 1) * waterGrid->cols + col] =
                    waterGrid->height_buffer_a[(row + 1) * waterGrid->cols + col] - neighbourDrop;
            }
        }
    }
}

/* Disturbs every waterGrid of the table at the position pointed to by pPos. */
void tiWatSimDisturbAll(TiWaterSub *waterSub, Vector4 *pPos)
{
    TiWatSimItem *cursorItem;
    u32 i;

    cursorItem = waterSub->items;
    for (i = 0; i < waterSub->active; i++) {
        tiWatSimDisturbAt(cursorItem, pPos);
        cursorItem++;
    }
}

/* Gives each masked cell of the waterGrid a random height in a band scaled by range and clears buffer B. */
void tiWatSimRandomizeItem(TiWatSimItem *waterGrid, int heightRange)
{
    float *mask;
    u32 i;

    mask = (float *) (waterGrid->block + waterGrid->mask_offset);
    for (i = 0; i < waterGrid->cell_count; i++) {
        if (mask[i] != 0.0f) {
            waterGrid->height_buffer_a[i] = (xglSRand() % heightRange) * 0.01f - heightRange * 0.005f;
            waterGrid->height_buffer_b[i] = 0.0f;
        }
    }
}

/* Randomizes every waterGrid of the table with the same range. */
void tiWatSimRandomizeAll(TiWaterSub *waterSub, int heightRange)
{
    TiWatSimItem *cursorItem;
    u32 i;

    cursorItem = waterSub->items;
    for (i = 0; i < waterSub->active; i++) {
        tiWatSimRandomizeItem(cursorItem, heightRange);
        cursorItem++;
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

Vector4 tiWatSimSourcePos = { 0.0f, 0.0f, 0.0f, 1.0f };
Vector4 D_003EC1C0 = { 5.0f, 0.0f, 5.0f, 1.0f };
Vector4 tiWatSimSecondSourcePos = { 0.0f, 0.0f, 0.0f, 0.0f };

int tiWatSimRainCountdown = 0;
int D_00550E40 = 17;

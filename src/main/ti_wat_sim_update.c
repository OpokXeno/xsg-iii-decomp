#include "common.h"
#include "shared.h"

void tiWatSimSwapWords(float **heightBufferA, float **heightBufferB);

/* Row-major 4x4 float matrix. */

#include "main/TiWatSim.h"
#include "main/xgl_2.h"



void tiWatSimUpdate(float timeStep, TiWatSimItem *waterSimItem);

void tiWatSimSwapWords(f32 **heightBufferA, f32 **heightBufferB) {
    f32 *savedHeightBuffer;

    savedHeightBuffer = *heightBufferA;
    *heightBufferA = *heightBufferB;
    *heightBufferB = savedHeightBuffer;
}

/* One water step: buffer A drives buffer B over the interior cells, masked cells are scaled, the
 * buffers are swapped, and the normals of the interior are rebuilt from buffer B. The step
 * (tiWaterStepSub passes 1.0) is not read. */
void tiWatSimAdvance(float timeStep, TiWatSimItem *waterSimItem)
{
    float courant;
    float courant_squared;
    float self_weight;
    unsigned int x;
    unsigned int y;
    unsigned int row;
    float next_height;
    float nx;
    float nz;
    float dy;
    float length_squared;
    float inverse;
    float *mask = (float *) (waterSimItem->block + waterSimItem->mask_offset);
    DoublewordVector4 *normals;

    /* courant is the wave speed scaled to the grid step; the centre cell keeps 2 - 4 * courant^2 of its height. */
    courant = waterSimItem->wave_speed * 0.111f;
    courant_squared = courant * courant;
    self_weight = 2.0f - courant_squared * 4.0f;
    normals = (DoublewordVector4 *) (waterSimItem->block + waterSimItem->normals_offset);
    for (y = 1; y < waterSimItem->rows - 1; y++) {
        row = y * waterSimItem->cols;
        for (x = 1; x < waterSimItem->cols - 1; x++) {
            next_height = courant_squared * (waterSimItem->height_buffer_a[row - waterSimItem->cols + x] + waterSimItem->height_buffer_a[row + waterSimItem->cols + x]
                    + waterSimItem->height_buffer_a[row + x - 1] + waterSimItem->height_buffer_a[row + x + 1])
                + self_weight * waterSimItem->height_buffer_a[row + x] - waterSimItem->height_buffer_b[row + x];
            waterSimItem->height_buffer_b[row + x] = next_height;
            if (mask[row + x] != 0.0f) {
                waterSimItem->height_buffer_b[row + x] = next_height * mask[row + x];
            } else {
                waterSimItem->height_buffer_b[row + x] = 0.0f;
            }
            nx = waterSimItem->height_buffer_b[row + x - 1] - waterSimItem->height_buffer_b[row + x + 1];
            normals[row + x].vector.x = nx;
            nz = waterSimItem->height_buffer_b[row - waterSimItem->cols + x] - waterSimItem->height_buffer_b[row + waterSimItem->cols + x];
            normals[row + x].vector.z = nz;
            dy = waterSimItem->cell_spacing * 2.25f;
            length_squared = nx * nx + dy * dy + nz * nz;
            inverse = 1.0f / length_squared;
            normals[row + x].vector.z = nz * inverse;
            normals[row + x].vector.x = nx * inverse;
            dy = dy * inverse;
            normals[row + x].vector.y = dy;
        }
    }
    tiWatSimSwapWords(&waterSimItem->height_buffer_a, &waterSimItem->height_buffer_b);
}

/* Runs one water simulation step (tiWatSimAdvance). */
void tiWatSimUpdate(float timeStep, TiWatSimItem *waterSimItem)
{
    tiWatSimAdvance(timeStep, waterSimItem);
}

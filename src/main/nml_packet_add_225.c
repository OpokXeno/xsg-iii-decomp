#include "common.h"
#include "shared.h"

#include "main/Packet.h"
#include "main/xgl_2.h"

/* Row-major 4x4 float matrix. */

/* The EE scratchpad (16 KB), where the GS queue and talk packets are built before they are sent. */
#define SCRATCHPAD_BASE 0x70000000

void xglVectorNormal(Vector4 *destination, const Vector4 *source);
void xglVectorMulMat(Vector4 *destination, Matrix4 matrix, const Vector4 *vector);
void xglMatrixStackMul(Matrix4 matrix);
void xglMatrixStackSave(Matrix4 matrix);
void xglMatrixStackLoad(Matrix4 matrix);
void xglMatrixFromAxes(Matrix4 matrix, const Vector4 *axis0, const Vector4 *axis1, const Vector4 *axis2);







XglPacket *xglPacketGetCurrent(void);
void sceVif1PkCnt(XglPacket *packet, unsigned int count);
void sceVif1PkAddCode(XglPacket *packet, unsigned int tag);
void sceVif1PkRef(XglPacket *packet, const void *data, unsigned int qwc, unsigned int a3, unsigned int a4,
                  unsigned int a5);
void sceVif1PkOpenUpkCode(XglPacket *packet, unsigned int address, unsigned int format, unsigned int a3,
                          unsigned int a4);
void sceVif1PkCloseUpkCode(XglPacket *packet);
void sceVif1PkAddUpkData128N(XglPacket *packet, const void *data, unsigned int count);
void tiWaterSendFrame(XglCamera *camera, Matrix4 data);
void tiWaterSendDirection(XglCamera *camera, Matrix4 transform);
void tiWaterSendItemPair(float x, float y);

/* VIF command words: UNPACK (V4-32), MSCNT (0x17) that starts the VU1 micro-program, and FLUSH. */
#define VIF_CODE_UNPACK_V4_32 0x6C
#define VIF_CODE_MSCNT 0x17000000
#define VIF_CODE_FLUSH 0x11000000
/* Scratchpad addresses: the GIF tag, and the quadword rows that follow it. */
#define SCRATCH_GIF_TAG SCRATCHPAD_BASE
#define SCRATCH_QUAD_ROWS (SCRATCHPAD_BASE + 0x10)
/* GIF tag EOP bit (bit 15 of the tag's low halfword), kept when the tag's loop count is set. */
#define GIF_TAG_EOP 0x8000
/* VU1 addresses of the water sends (the vuAddress argument of xglPacketBeginUnpack). */
#define TIWATER_VU_ROW80 0x3C0
#define TIWATER_VU_ROW70 0x3C1
#define TIWATER_VU_TRANSFORMED 0x3C2
#define TIWATER_VU_MATRIX 0x3C6
#define TIWATER_VU_ORIENTED 0x3CA
#define TIWATER_VU_FIXED 0x3CE
#define TIWATER_VU_POSITION 0x3D2
#define TIWATER_VU_DIRECTION 0x3D6
#define TIWATER_VU_ITEM_PAIR 0x3DA
#define TIWATER_VU_FRAME_PAIR 0x3DE

/* Caches the packet every send in this TU writes to (xglPacketCaptureCurrent stores it). */
extern XglPacket *xglCurrentPacket;
/* Count and entries of the reference table added by xglPacketRefTable. */
extern int xglPacketRefCount;
extern u8 xglPacketRefEntries[];
/* Adds a reference to the table xglPacketRefEntries (xglPacketRefCount entries). */
void xglPacketRefTable(void);

#include "quadword.h"

/* The three axis rows that tiWaterSendOrientedMatrix passes to xglMatrixFromAxes. */
extern Vector4 tiWaterAxes[3];
/* The four rows of the fixed matrix sent by tiWaterSendFixedMatrix. */
extern Matrix4 tiWaterFixedMatrix;

typedef struct WaterStripGrid {
    u8 unmodeled_00[0x08];
    int rowStride;
    int rowCount;
    u8 unmodeled_10[0x30];
    Vector4 origin;
    float cell;
    u8 unmodeled_54[0x28];
    u32 vertexOffset;
    u8 unmodeled_80[0x04];
    u32 vertexBase;
    u8 unmodeled_88[0x48];
    Matrix4 transform;
} WaterStripGrid;

/* Strip count and column step of the grid (the strip width in columns). */
extern int tiWaterStripColumns;
/* Same objects as tiWatSimSourcePos and tiWatSimSourcePosTransformed (TiWatSim.h): the original addresses the pair off one
 * symbol (the transformed one at +0x10), and two separate symbols change the relocations (.text DIFF). */
extern Vector4 tiWatSimSourcePos[2];

/* Fills the four rows of matrix from firstSourceRow..fourthSourceRow (matrix[r][i] = row r [i]). */
void xglMatrixFromRows(float matrix[4][4], const float *firstSourceRow, const float *secondSourceRow, const float *thirdSourceRow,
                       const float *fourthSourceRow) {
    unsigned int i;

    for (i = 0; i < 4; i++) {
        float *matrixElement = &matrix[0][i];
        const float *rowElement = &firstSourceRow[i];
        *matrixElement = *rowElement;
    }
    for (i = 0; i < 4; i++) {
        float *matrixElement = &matrix[1][i];
        const float *rowElement = &secondSourceRow[i];
        *matrixElement = *rowElement;
    }
    for (i = 0; i < 4; i++) {
        float *matrixElement = &matrix[2][i];
        const float *rowElement = &thirdSourceRow[i];
        *matrixElement = *rowElement;
    }
    for (i = 0; i < 4; i++) {
        float *matrixElement = &matrix[3][i];
        const float *rowElement = &fourthSourceRow[i];
        *matrixElement = *rowElement;
    }
}

/* Caches the current packet for the sends below. */
void nmlPacketSetCurrent(void) {
    xglCurrentPacket = xglPacketGetCurrent();
}

/* Opens a VIF UNPACK (V4-32, code 0x6C) of the next data to VU1 address vuAddress. */
void xglPacketBeginUnpack(int vuAddress) {
    sceVif1PkCnt(xglCurrentPacket, 0);
    sceVif1PkOpenUpkCode(xglCurrentPacket, vuAddress & 0xFFFF, VIF_CODE_UNPACK_V4_32, 1, 1);
}

/* Ends the unpack opened by xglPacketBeginUnpack. */
void xglPacketEndUnpack(void) {
    sceVif1PkCloseUpkCode(xglCurrentPacket);
}

/* Table data read by xglPacketRefTable: the entry count at D_00515B00[0] and the entries at D_00515B10. */
extern int D_00515B00[];
extern u8 D_00515B10[];

/* Adds a reference to the table xglPacketRefEntries (xglPacketRefCount entries). */
void xglPacketRefTable(void) {
    sceVif1PkRef(xglCurrentPacket, D_00515B10, D_00515B00[0], 0, 0, 0);
}

/* Resets the count and emits one VIF FLUSH (0x11000000). */
void xglPacketFlush(void) {
    sceVif1PkCnt(xglCurrentPacket, 0);
    sceVif1PkAddCode(xglCurrentPacket, VIF_CODE_FLUSH);
}

/* Sends the view matrix of the camera (the camera tiWaterDrawItem passes) multiplied by the
 * object matrix at VU address 0x3C2; tiWaterSendMatrix sends the object matrix alone at 0x3C6. */
void tiWaterSendTransformed(XglCamera *camera, Matrix4 matrix) {
    Matrix4 savedMatrix;

    xglMatrixStackLoad(camera->volume[0].viewMatrix);
    xglMatrixStackMul(matrix);
    xglMatrixStackSave(savedMatrix);
    xglPacketBeginUnpack(TIWATER_VU_TRANSFORMED);
    sceVif1PkAddUpkData128N(xglCurrentPacket, savedMatrix, 4);
    xglPacketEndUnpack();
}

/* Sends the four quads of data at VU address 0x3C6. */
void tiWaterSendMatrix(Matrix4 objectMatrix) {
    xglPacketBeginUnpack(TIWATER_VU_MATRIX);
    sceVif1PkAddUpkData128N(xglCurrentPacket, objectMatrix, 4);
    xglPacketEndUnpack();
}

/* Builds the matrix from the axes, copies the object matrix into rotationScale with its row 3 xyz cleared, and sends the stack product at VU address 0x3CA. */
void tiWaterSendOrientedMatrix(Matrix4 objectMatrix) {
    Matrix4 rotationScale;
    Matrix4 stack;

    xglMatrixFromAxes(stack, &tiWaterAxes[0], &tiWaterAxes[1], &tiWaterAxes[2]);
    QuadwordCopyMatrix(rotationScale, objectMatrix);
    rotationScale[3][0] = rotationScale[3][1] = rotationScale[3][2] = 0.0f;
    xglMatrixStackLoad(stack);
    xglMatrixStackMul(rotationScale);
    xglMatrixStackSave(stack);
    xglPacketBeginUnpack(TIWATER_VU_ORIENTED);
    sceVif1PkAddUpkData128N(xglCurrentPacket, stack, 4);
    xglPacketEndUnpack();
}

/* Sends the matrix built from tiWaterFixedMatrix at VU address 0x3CE. */
void tiWaterSendFixedMatrix(void) {
    Matrix4 matrix;

    xglMatrixFromRows(matrix, tiWaterFixedMatrix[0], tiWaterFixedMatrix[1], tiWaterFixedMatrix[2], tiWaterFixedMatrix[3]);
    xglPacketBeginUnpack(TIWATER_VU_FIXED);
    sceVif1PkAddUpkData128N(xglCurrentPacket, matrix, 4);
    xglPacketEndUnpack();
}

/* Sets the constants 1/512 and 7/16 in the block, then sends row80 at 0x3C0 and row70 at 0x3C1. */
void tiWaterSendParamRows(XglCamera *camera) {
    camera->screen.screenScale.w = 0.001953125f;
    camera->screen.screenOffset.w = 0.4375f;
    xglPacketBeginUnpack(TIWATER_VU_ROW80);
    sceVif1PkAddUpkData128N(xglCurrentPacket, &camera->screen.screenScale, 1);
    xglPacketEndUnpack();
    xglPacketBeginUnpack(TIWATER_VU_ROW70);
    sceVif1PkAddUpkData128N(xglCurrentPacket, &camera->screen.screenOffset, 1);
    xglPacketEndUnpack();
}

void tiWaterSendDirection(XglCamera *camera, Matrix4 transform) {
    Vector4 dir;

    xglPacketBeginUnpack(TIWATER_VU_POSITION);
    sceVif1PkAddUpkData128N(xglCurrentPacket, &camera->travel.position, 1);
    xglPacketEndUnpack();
    dir.x = transform[3][0] - camera->travel.position.x;
    dir.y = transform[3][1] - camera->travel.position.y;
    dir.z = transform[3][2] - camera->travel.position.z;
    dir.w = 1.0f;
    xglVectorNormal(&dir, &dir);
    xglPacketBeginUnpack(TIWATER_VU_DIRECTION);
    sceVif1PkAddUpkData128N(xglCurrentPacket, &dir, 1);
    xglPacketEndUnpack();
}

/* Sends the pair (x, y) at VU address TIWATER_VU_ITEM_PAIR (0x3DA). */
void tiWaterSendItemPair(f32 x, f32 y) {
    f32 xy[2];

    xy[0] = x;
    xy[1] = y;
    xglPacketBeginUnpack(TIWATER_VU_ITEM_PAIR);
    sceVif1PkAddUpkData128N(xglCurrentPacket, xy, 1);
    xglPacketEndUnpack();
}

/* Sends the pair (x, y) at VU address TIWATER_VU_FRAME_PAIR (0x3DE). */
void tiWaterSendFramePair(f32 x, f32 y) {
    f32 xy[2];

    xy[0] = x;
    xy[1] = y;
    xglPacketBeginUnpack(TIWATER_VU_FRAME_PAIR);
    sceVif1PkAddUpkData128N(xglCurrentPacket, xy, 1);
    xglPacketEndUnpack();
}

/* Sends one water frame: packet cache, reference table, transformed matrix, matrices, parameter rows, then the pair (0.7, 0.05). */
void tiWaterSendFrame(XglCamera *camera, Matrix4 objectMatrix) {
    nmlPacketSetCurrent();
    xglPacketRefTable();
    tiWaterSendTransformed(camera, objectMatrix);
    tiWaterSendMatrix(objectMatrix);
    tiWaterSendOrientedMatrix(objectMatrix);
    tiWaterSendFixedMatrix();
    tiWaterSendParamRows(camera);
    tiWaterSendFramePair(0.7f, 0.05f);
}

/* Writes the strip of columns firstColumn..lastColumn between grid rows row and row + 1 into the scratchpad and sends it. */
void tiWaterSendStrip(int firstColumn, unsigned int lastColumn, int row, WaterStripGrid *grid) {
    u32 *gifTag = (u32 *)SCRATCH_GIF_TAG;
    DoublewordVector4 *quadCursor = (DoublewordVector4 *)SCRATCH_QUAD_ROWS;
    unsigned int stripQuadCount;
    DoublewordVector4 *vertexCopy;
    Vector4 *gridVertices;
    unsigned int rowStart;
    unsigned int gridColumn;
    float cellX;
    float cellZ;
    XglPacket *packet;

    stripQuadCount = 2 * (lastColumn - firstColumn) + 2;
    gridVertices = (Vector4 *)(grid->vertexBase + grid->vertexOffset);
    *gifTag = (*gifTag & GIF_TAG_EOP) | (2 * (lastColumn - firstColumn) + 2);
    cellX = firstColumn * grid->cell + grid->origin.x;
    cellZ = row * -grid->cell + grid->origin.z;
    rowStart = row * grid->rowStride;
    packet = xglCurrentPacket;
    vertexCopy = quadCursor + stripQuadCount;
    for (gridColumn = firstColumn; gridColumn <= lastColumn; gridColumn++) {
        quadCursor->vector.x = cellX;
        quadCursor->vector.y = 0.0f;
        quadCursor->vector.z = cellZ;
        quadCursor->vector.w = 1.0f;
        quadCursor++;
        vertexCopy->vector.x = gridVertices[rowStart + gridColumn].x;
        vertexCopy->vector.y = gridVertices[rowStart + gridColumn].y;
        vertexCopy->vector.z = gridVertices[rowStart + gridColumn].z;
        vertexCopy++;
        quadCursor->vector.x = cellX;
        quadCursor->vector.y = 0.0f;
        quadCursor->vector.z = cellZ - grid->cell;
        quadCursor->vector.w = 1.0f;
        quadCursor++;
        vertexCopy->vector.x = gridVertices[rowStart + grid->rowStride + gridColumn].x;
        vertexCopy->vector.y = gridVertices[rowStart + grid->rowStride + gridColumn].y;
        vertexCopy->vector.z = gridVertices[rowStart + grid->rowStride + gridColumn].z;
        vertexCopy++;
        cellX += grid->cell;
    }
    sceVif1PkOpenUpkCode(packet, 0x8000, VIF_CODE_UNPACK_V4_32, 1, 1);
    sceVif1PkAddUpkData128N(packet, gifTag, 2 * stripQuadCount + 1);
    sceVif1PkCloseUpkCode(packet);
}

/* Sends the grid: copies the block into the GIF tag, sends the transformed origin, then one strip per row pair and column group. */
void tiWaterSendGrid(WaterStripGrid *grid, const void *gifTagBlock) {
    unsigned int strips;
    unsigned int row;
    unsigned int strip;
    int firstColumn;
    int column;
    XglPacket *packet;

    strips = (grid->rowStride - 1) / tiWaterStripColumns;
    packet = xglCurrentPacket;
    if (grid->rowStride >= 2 && grid->rowCount >= 2) {
        __builtin_memcpy((void *)SCRATCH_GIF_TAG, gifTagBlock, 16);
        xglVectorMulMat(&tiWatSimSourcePos[1], grid->transform, &tiWatSimSourcePos[0]);
        for (row = 0; row < grid->rowCount - 1; row++) {
            sceVif1PkCnt(packet, 0);
            firstColumn = 0;
            column = 0;
            for (strip = 0; strip < strips; strip++) {
                column += tiWaterStripColumns;
                tiWaterSendStrip(firstColumn, column, row, grid);
                firstColumn = column;
                sceVif1PkAddCode(packet, VIF_CODE_MSCNT);
            }
            column = column + (grid->rowStride - tiWaterStripColumns * strips) - 1;
            if (firstColumn < column) {
                tiWaterSendStrip(firstColumn, column, row, grid);
                sceVif1PkAddCode(packet, VIF_CODE_MSCNT);
                xglPacketFlush();
            }
        }
    }
}

/* The three axis rows of tiWaterSendOrientedMatrix and the fixed matrix of tiWaterSendFixedMatrix. */
Vector4 tiWaterAxes[3] = {
    { -0.5f, -0.100000001f, 0.0f, 1.0f },
    { 0.5f, 0.100000001f, 0.5f, 1.0f },
    { 0.0f, 1.0f, 0.0f, 1.0f },
};
Matrix4 tiWaterFixedMatrix = {
    { 0.0f, 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 0.0f, 1.0f },
    { 128.0f, 128.0f, 128.0f, 1.0f },
};

int tiWaterStripColumns = 29;
XglPacket *xglCurrentPacket = 0;

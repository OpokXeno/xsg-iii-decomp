#include "common.h"

#include "shared.h"
extern f32 gTiVu0Vec3Matrix[];
void xglCurMatrixMul(f32 *matrix);

/* Copies the vector (x, y, z) into the current matrix's translation column and the vector-rows
   entries at 0x20 and 0x24, then multiplies the current matrix. */
void tiVu0CurMatrixLoadVec3Mul(f32 *vector)
{
    gTiVu0Vec3Matrix[4] = vector[0];
    gTiVu0Vec3Matrix[8] = vector[1];
    gTiVu0Vec3Matrix[9] = vector[2];
    xglCurMatrixMul(gTiVu0Vec3Matrix);
}

#include "shared.h"

void assertFailed(const char *message, const char *file, int line);
extern const char tiVu0UnreachableAssertText[];
extern const char tiVu0AssertFile[];

/* Unconditional assert(0) at TiVu0.euc.c line 802: the expression text is "0" and the file name is "TiVu0.euc.c". */
void tiVu0AssertLine802(void)
{
    assertFailed(tiVu0UnreachableAssertText, tiVu0AssertFile, 802);
}

/* Unconditional assert(0) at TiVu0.euc.c line 821: the expression text is "0" and the file name is "TiVu0.euc.c". */
void tiVu0AssertLine821(void)
{
    assertFailed(tiVu0UnreachableAssertText, tiVu0AssertFile, 821);
}

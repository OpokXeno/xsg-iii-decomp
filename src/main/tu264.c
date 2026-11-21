#include "common.h"

typedef float f32;
typedef unsigned char u8;
typedef int s32;
typedef long long s64;
extern f32 gXglScaleX;
extern f32 gXglScaleY;

void xglScaleSetXY(f32 scaleX, f32 scaleY) {
    gXglScaleX = scaleX;
    gXglScaleY = scaleY;
}

INCLUDE_ASM("asm/main/nonmatchings/tu264", func_00266B40);

INCLUDE_ASM("asm/main/nonmatchings/tu264", func_00266CC0);

extern unsigned char gXglTransformedVector[];
void xglVectorScaleAdd(s32 destination, u8 *first, s32 addend, f32 scale);

void xglScaleVectorTable(s32 destinationVector, f32 scale) {
    xglVectorScaleAdd(destinationVector, gXglTransformedVector, destinationVector, scale);
}

INCLUDE_ASM("asm/main/nonmatchings/tu264", func_00266DE8);

s32 func_0025C408();

void xglCommitDefaultState(void) {
    func_0025C408();
}

INCLUDE_ASM("asm/main/nonmatchings/tu264", func_00266F00);

extern unsigned char gXglMatrixStackSeedVector[];
void xglMatrixStackMulVector();
void xglMatrixStackRotX(f32);
void xglMatrixStackRotY(f32);
void xglMatrixStackUnit();

/* A 16-byte vector, copied as two doublewords. */
typedef struct DoublewordPair {
    s64 xy;
    s64 zw;
} DoublewordPair;

/* gXglMatrixStackSeedVector is the 16-byte seed vector of the matrix stack, read as one DoublewordPair. */
#define gMatrixStackSeedVector (*(DoublewordPair *) gXglMatrixStackSeedVector)

/* The two angles at the start of the caller's record: X at +0, Y at +4. */
typedef struct AngleXY {
    f32 x;
    f32 y;
} AngleXY;

void xglMatrixStackRotateVector(AngleXY *angles) {
    DoublewordPair vector;

    vector.xy = gMatrixStackSeedVector.xy;
    vector.zw = gMatrixStackSeedVector.zw;
    xglMatrixStackUnit();
    xglMatrixStackRotY(angles->y);
    xglMatrixStackRotX(angles->x);
    xglMatrixStackMulVector(gXglTransformedVector, &vector.xy);
}

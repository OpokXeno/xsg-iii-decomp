#include "common.h"
#include "shared.h"

/* Row-major 4x4 float matrix. */

void tiMtnBlendLerpMatrix(Matrix4 dst, Matrix4 a, Matrix4 b, float t);

/* TiMRState.flags of a state block: 0x4 selects the local slot (clear: global). */
#define TI_MTN_BLEND_RUNNING 0x2
#define TI_MTN_BLEND_LOCAL 0x4
#define TI_MTN_BLEND_REQUEST 0x40
/* Set in the state flags of state A: the first joint table skips its second matrix (the loop index 1). */
#define TI_MTN_BLEND_SKIP_MATRIX1 0x800

#include "main/TiMRPackage.h"



extern const char D_00542AC8[];
extern const char D_00542AD0[];
extern const char D_00542AE8[];
extern const char D_00542B00[];
extern const char D_00542B18[];
extern const char D_00542B30[];
extern const char D_00542B38[];
extern const char D_00542B40[];
void assertFailed(const char *message, const char *file, int line);
void tiWordClearBits(int *word, int bits);
void XrgMatrixToQuaternion(float quat[4], Matrix4 matrix);
void XrgQuaternionToMatrix(Matrix4 matrix, const Vector4 *quaternion);
void XrgQuaternionSlerp(Vector4 *result, Vector4 *from, Vector4 *to, float t);

/* Swaps the global and blend matrix arrays when flag 0x4 is clear. */
void tiMtnBlendSwapGlobal(TiMRPackage *ctx, int *flags) {
    Matrix4 *swap;

    if (!(*flags & TI_MTN_BLEND_LOCAL)) {
        if (ctx == 0) {
            assertFailed(D_00542AC8, D_00542AD0, 14);
        }
        if (ctx->globalMat == 0) {
            assertFailed(D_00542AE8, D_00542AD0, 15);
        }
        if (ctx->blendMat == 0) {
            assertFailed(D_00542B00, D_00542AD0, 16);
        }
        swap = ctx->globalMat;
        ctx->globalMat = ctx->blendMat;
        ctx->blendMat = swap;
        ctx->copyDst110 = ctx->copySrc114;
        ctx->copyDst120 = ctx->copySrc124;
    }
}

/* Swaps the local and blend matrix arrays when flag 0x4 is set. */
void tiMtnBlendSwapLocal(TiMRPackage *ctx, int *flags) {
    Matrix4 *swap;

    if (*flags & TI_MTN_BLEND_LOCAL) {
        if (ctx == 0) {
            assertFailed(D_00542AC8, D_00542AD0, 38);
        }
        if (ctx->localMat == 0) {
            assertFailed(D_00542B18, D_00542AD0, 39);
        }
        if (ctx->blendMat == 0) {
            assertFailed(D_00542B00, D_00542AD0, 40);
        }
        swap = ctx->localMat;
        ctx->localMat = ctx->blendMat;
        ctx->blendMat = swap;
        ctx->copyDst110 = ctx->copySrc114;
        ctx->copyDst120 = ctx->copySrc124;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/TiMtnBlend", func_0029E870);

/* Blends two 4x4 matrices: dst = a * (1 - t) + b * t. The elements are addressed by byte offset: the typed [row][col] form changed the code. */
void tiMtnBlendLerpMatrix(Matrix4 dst, Matrix4 a, Matrix4 b, float t) {
    int byteOffset;
    int row;
    int col;
    float *dstElem;
    float *aElem;
    float *bElem;

    for (row = 0; row < 4; row++) {
        for (col = 0; col < 4; col++) {
            byteOffset = row * 16 + col * 4;
            dstElem = (float *)((char *)dst + byteOffset);
            aElem = (float *)((char *)a + byteOffset);
            bElem = (float *)((char *)b + byteOffset);
            *dstElem = *aElem * (1.0f - t) + *bElem * t;
        }
    }
}

/* Blends the rotation of two matrices by quaternion slerp and their translation by lerp. */
void tiMtnBlendMatrix(Matrix4 *dst, Matrix4 *matA, Matrix4 *matB, float t) {
    float oneMinusT = 1.0f - t;
    Vector4 startQuat;
    Vector4 endQuat;
    Vector4 blendQuat;
    float translation[3];
    int row;
    int col;
    float sum;

    if (dst == 0) {
        assertFailed(D_00542B30, D_00542AD0, 99);
    }
    if (matA == 0) {
        assertFailed(D_00542B38, D_00542AD0, 100);
    }
    if (matB == 0) {
        assertFailed(D_00542B40, D_00542AD0, 101);
    }

    translation[0] = (*matA)[3][0] * oneMinusT + (*matB)[3][0] * t;
    translation[1] = (*matA)[3][1] * oneMinusT + (*matB)[3][1] * t;
    translation[2] = (*matA)[3][2] * oneMinusT + (*matB)[3][2] * t;

    for (row = 0; row < 3; row++) {
        sum = 0.0f;
        for (col = 0; col < 3; col++) {
            sum += __builtin_fabsf((*matA)[row][col]);
        }
        if (sum < 1.0e-5f) {
            return;
        }
    }

    XrgMatrixToQuaternion(&startQuat.x, *matA);
    XrgMatrixToQuaternion(&endQuat.x, *matB);
    XrgQuaternionSlerp(&blendQuat, &startQuat, &endQuat, t);
    XrgQuaternionToMatrix(*dst, &blendQuat);
    (*dst)[3][0] = translation[0];
    (*dst)[3][1] = translation[1];
    (*dst)[3][2] = translation[2];
}

/* Advances the global blend windows of the two state blocks: the matrices move from the global set toward the blend set. */
void tiMtnBlendUpdateGlobal(TiMRPackage *ctx) {
    float progress;
    float remaining;

    if (ctx == 0) {
        assertFailed(D_00542AC8, D_00542AD0, 155);
    }
    if (ctx->entryA != 0) {
        unsigned int item;
        int matrixIndex;

        if ((ctx->state_a.flags & TI_MTN_BLEND_REQUEST) == 0) {
            tiWordClearBits(&ctx->state_a.flags, TI_MTN_BLEND_RUNNING);
        } else if ((*(long long *)&ctx->state_a.flags & (TI_MTN_BLEND_RUNNING | TI_MTN_BLEND_LOCAL)) == TI_MTN_BLEND_RUNNING) {
            remaining = ctx->state_a.position - (float)ctx->state_a.rangeMin;
            if (ctx->state_a.value == 0.0f || ctx->state_a.value <= remaining || remaining < 0.0f) {
                tiWordClearBits(&ctx->state_a.flags, TI_MTN_BLEND_RUNNING);
                tiWordClearBits(&ctx->state_a.flags, TI_MTN_BLEND_REQUEST);
                return;
            }
            if (ctx->blendMat == 0) {
                assertFailed(D_00542B00, D_00542AD0, 179);
            }
            if (ctx->globalMat == 0) {
                assertFailed(D_00542AE8, D_00542AD0, 180);
            }
            progress = remaining / ctx->state_a.value;
            matrixIndex = 0;
            if (progress < 0.0f) {
                progress = 0.0f;
            } else if (1.0f < progress) {
                progress = 1.0f;
            }
            for (item = 0; item < ctx->hrcData->jointCount; item++) {
                if (!((ctx->state_a.flags & TI_MTN_BLEND_SKIP_MATRIX1) && matrixIndex == 1)) {
                    tiMtnBlendMatrix(ctx->globalMat + matrixIndex, ctx->blendMat + matrixIndex, ctx->globalMat + matrixIndex, progress);
                }
                matrixIndex++;
            }
            for (item = 0; item < ctx->hrcData->joint2Count; item++) {
                tiMtnBlendMatrix(ctx->globalMat + matrixIndex, ctx->blendMat + matrixIndex, ctx->globalMat + matrixIndex, progress);
                matrixIndex++;
            }
            for (item = 0; item < ctx->hrcData->joint3Count; item++) {
                tiMtnBlendMatrix(ctx->globalMat + matrixIndex, ctx->blendMat + matrixIndex, ctx->globalMat + matrixIndex, progress);
                matrixIndex++;
            }
            for (item = 0; item < ctx->hrcData->dynamicsCount; item++) {
                tiMtnBlendMatrix(ctx->globalMat + matrixIndex, ctx->blendMat + matrixIndex, ctx->globalMat + matrixIndex, progress);
                matrixIndex++;
            }
        }
    }
    if (ctx->entryB != 0) {
        unsigned int item;
        int matrixIndex;

        if ((ctx->state_b.flags & TI_MTN_BLEND_REQUEST) == 0) {
            tiWordClearBits(&ctx->state_b.flags, TI_MTN_BLEND_RUNNING);
        } else if ((*(long long *)&ctx->state_b.flags & (TI_MTN_BLEND_RUNNING | TI_MTN_BLEND_LOCAL)) == TI_MTN_BLEND_RUNNING) {
            remaining = ctx->state_b.position - (float)ctx->state_b.rangeMin;
            if (ctx->state_b.value == 0.0f || ctx->state_b.value <= remaining || remaining < 0.0f) {
                tiWordClearBits(&ctx->state_b.flags, TI_MTN_BLEND_RUNNING);
                tiWordClearBits(&ctx->state_b.flags, TI_MTN_BLEND_REQUEST);
                return;
            }
            if (ctx->blendMat == 0) {
                assertFailed(D_00542B00, D_00542AD0, 235);
            }
            if (ctx->globalMat == 0) {
                assertFailed(D_00542AE8, D_00542AD0, 236);
            }
            progress = remaining / ctx->state_b.value;
            matrixIndex = ctx->hrcData->jointCount + ctx->hrcData->joint2Count + ctx->hrcData->joint3Count;
            if (progress < 0.0f) {
                progress = 0.0f;
            } else if (1.0f < progress) {
                progress = 1.0f;
            }
            for (item = 0; item < ctx->hrcData->skipCount; item++) {
                tiMtnBlendMatrix(ctx->globalMat + matrixIndex, ctx->blendMat + matrixIndex, ctx->globalMat + matrixIndex, progress);
                matrixIndex++;
            }
        }
    }
}

void func_0029E870(Matrix4 *dst, Matrix4 *src);
float sinf(float angle);

/* Advances the local blend windows of the two state blocks: the local matrices move from the local set toward the blend set. Block A eases its progress through sinf. */
void tiMtnBlendUpdateLocal(TiMRPackage *ctx) {
    float progress;
    float remaining;

    if (ctx == 0) {
        assertFailed(D_00542AC8, D_00542AD0, 261);
    }
    if (ctx->entryA != 0) {
        unsigned int item;
        int matrixIndex;

        if ((ctx->state_a.flags & TI_MTN_BLEND_REQUEST) == 0) {
            tiWordClearBits(&ctx->state_a.flags, TI_MTN_BLEND_RUNNING);
        } else if ((*(long long *)&ctx->state_a.flags & (TI_MTN_BLEND_RUNNING | TI_MTN_BLEND_LOCAL)) == (TI_MTN_BLEND_RUNNING | TI_MTN_BLEND_LOCAL)) {
            remaining = ctx->state_a.position - (float)ctx->state_a.rangeMin;
            if (ctx->state_a.value == 0.0f || ctx->state_a.value <= remaining || remaining < 0.0f) {
                tiWordClearBits(&ctx->state_a.flags, TI_MTN_BLEND_RUNNING);
                tiWordClearBits(&ctx->state_a.flags, TI_MTN_BLEND_REQUEST);
                return;
            }
            if (ctx->blendMat == 0) {
                assertFailed(D_00542B00, D_00542AD0, 285);
            }
            if (ctx->localMat == 0) {
                assertFailed(D_00542B18, D_00542AD0, 286);
            }
            progress = remaining / ctx->state_a.value;
            matrixIndex = 0;
            if (progress < 0.0f) {
                progress = 0.0f;
            } else if (1.0f < progress) {
                progress = 1.0f;
            }
            progress = sinf(progress * 3.1415927f * 0.5f);
            for (item = 0; item < ctx->hrcData->jointCount; item++) {
                if (!((ctx->state_a.flags & TI_MTN_BLEND_SKIP_MATRIX1) && matrixIndex == 1)) {
                    if (matrixIndex == 16 || matrixIndex == 23 || matrixIndex == 3 || matrixIndex == 7) {
                        tiMtnBlendLerpMatrix(*(ctx->localMat + matrixIndex), *(ctx->blendMat + matrixIndex), *(ctx->localMat + matrixIndex), progress);
                        func_0029E870(ctx->localMat + matrixIndex, ctx->localMat + matrixIndex);
                    } else {
                        tiMtnBlendMatrix(ctx->localMat + matrixIndex, ctx->blendMat + matrixIndex, ctx->localMat + matrixIndex, progress);
                    }
                }
                matrixIndex++;
            }
            for (item = 0; item < ctx->hrcData->joint2Count; item++) {
                tiMtnBlendMatrix(ctx->localMat + matrixIndex, ctx->blendMat + matrixIndex, ctx->localMat + matrixIndex, progress);
                matrixIndex++;
            }
            for (item = 0; item < ctx->hrcData->joint3Count; item++) {
                tiMtnBlendMatrix(ctx->localMat + matrixIndex, ctx->blendMat + matrixIndex, ctx->localMat + matrixIndex, progress);
                matrixIndex++;
            }
            for (item = 0; item < ctx->hrcData->dynamicsCount; item++) {
                tiMtnBlendMatrix(ctx->localMat + matrixIndex, ctx->blendMat + matrixIndex, ctx->localMat + matrixIndex, progress);
                matrixIndex++;
            }
        }
    }
    if (ctx->entryB != 0) {
        unsigned int item;
        int matrixIndex;

        if ((ctx->state_b.flags & TI_MTN_BLEND_REQUEST) == 0) {
            tiWordClearBits(&ctx->state_b.flags, TI_MTN_BLEND_RUNNING);
        } else if ((*(long long *)&ctx->state_b.flags & (TI_MTN_BLEND_RUNNING | TI_MTN_BLEND_LOCAL)) == (TI_MTN_BLEND_RUNNING | TI_MTN_BLEND_LOCAL)) {
            remaining = ctx->state_b.position - (float)ctx->state_b.rangeMin;
            if (ctx->state_b.value == 0.0f || ctx->state_b.value <= remaining || remaining < 0.0f) {
                tiWordClearBits(&ctx->state_b.flags, TI_MTN_BLEND_RUNNING);
                tiWordClearBits(&ctx->state_b.flags, TI_MTN_BLEND_REQUEST);
                return;
            }
            if (ctx->blendMat == 0) {
                assertFailed(D_00542B00, D_00542AD0, 380);
            }
            if (ctx->localMat == 0) {
                assertFailed(D_00542B18, D_00542AD0, 381);
            }
            progress = remaining / ctx->state_b.value;
            matrixIndex = ctx->hrcData->jointCount + ctx->hrcData->joint2Count + ctx->hrcData->joint3Count;
            if (progress < 0.0f) {
                progress = 0.0f;
            } else if (1.0f < progress) {
                progress = 1.0f;
            }
            for (item = 0; item < ctx->hrcData->skipCount; item++) {
                tiMtnBlendMatrix(ctx->localMat + matrixIndex, ctx->blendMat + matrixIndex, ctx->localMat + matrixIndex, progress);
                matrixIndex++;
            }
        }
    }
}

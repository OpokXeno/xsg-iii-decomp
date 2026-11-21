#include "common.h"
#include "shared.h"

/* Row-major 4x4 float matrix. */

void tiMtnBlendLerpMatrix(Matrix4 outMatrix, Matrix4 startMatrix, Matrix4 endMatrix, float t);

/* TiMRState.motionFlags of a state block: 0x4 selects the local slot (clear: global). */
#define TI_MTN_BLEND_RUNNING 0x2
#define TI_MTN_BLEND_LOCAL 0x4
#define TI_MTN_BLEND_REQUEST 0x40
/* Set in the state flags of state A: the first joint table skips its second matrix (the loop index 1). */
#define TI_MTN_BLEND_SKIP_MATRIX1 0x800

#include "main/TiMRPackage.h"



extern const char tiMtnBlendPackAssertExpr[];
extern const char tiMtnBlendSourceFile[];
extern const char tiMtnBlendPackGlobalMatAssertExpr[];
extern const char tiMtnBlendPackBlendMatAssertExpr[];
extern const char tiMtnBlendPackLocalMatAssertExpr[];
extern const char tiMtnBlendDestAssertExpr[];
extern const char tiMtnBlendStartMatAssertExpr[];
extern const char tiMtnBlendEndMatAssertExpr[];
void assertFailed(const char *message, const char *file, int line);
void tiWordClearBits(int *word, int bits);
void XrgMatrixToQuaternion(float quat[4], Matrix4 matrix);
void XrgQuaternionToMatrix(Matrix4 matrix, const Vector4 *quaternion);
void XrgQuaternionSlerp(Vector4 *result, Vector4 *from, Vector4 *to, float t);

/* Swaps the global and blend matrix arrays when flag 0x4 is clear. */
void tiMtnBlendSwapGlobal(TiMRPackage *package, int *blendStateFlags) {
    Matrix4 *oldGlobalMat;

    if (!(*blendStateFlags & TI_MTN_BLEND_LOCAL)) {
        if (package == 0) {
            assertFailed(tiMtnBlendPackAssertExpr, tiMtnBlendSourceFile, 14);
        }
        if (package->globalMat == 0) {
            assertFailed(tiMtnBlendPackGlobalMatAssertExpr, tiMtnBlendSourceFile, 15);
        }
        if (package->blendMat == 0) {
            assertFailed(tiMtnBlendPackBlendMatAssertExpr, tiMtnBlendSourceFile, 16);
        }
        oldGlobalMat = package->globalMat;
        package->globalMat = package->blendMat;
        package->blendMat = oldGlobalMat;
        package->copyDst110 = package->copySrc114;
        package->copyDst120 = package->copySrc124;
    }
}

/* Swaps the local and blend matrix arrays when flag 0x4 is set. */
void tiMtnBlendSwapLocal(TiMRPackage *package, int *blendStateFlags) {
    Matrix4 *oldLocalMat;

    if (*blendStateFlags & TI_MTN_BLEND_LOCAL) {
        if (package == 0) {
            assertFailed(tiMtnBlendPackAssertExpr, tiMtnBlendSourceFile, 38);
        }
        if (package->localMat == 0) {
            assertFailed(tiMtnBlendPackLocalMatAssertExpr, tiMtnBlendSourceFile, 39);
        }
        if (package->blendMat == 0) {
            assertFailed(tiMtnBlendPackBlendMatAssertExpr, tiMtnBlendSourceFile, 40);
        }
        oldLocalMat = package->localMat;
        package->localMat = package->blendMat;
        package->blendMat = oldLocalMat;
        package->copyDst110 = package->copySrc114;
        package->copyDst120 = package->copySrc124;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/TiMtnBlend", func_0029E870);

/* Blends two 4x4 matrices: dst = a * (1 - t) + b * t. */
void tiMtnBlendLerpMatrix(Matrix4 outMatrix, Matrix4 startMatrix, Matrix4 endMatrix, float t) {
    int row;
    int col;
    int matrixCellIndex;
    float *outCell;
    float *startCell;
    float *endCell;

    for (row = 0; row < 4; row++) {
        for (col = 0; col < 4; col++) {
            matrixCellIndex = row * 4 + col;
            outCell = outMatrix[0] + matrixCellIndex;
            startCell = startMatrix[0] + matrixCellIndex;
            endCell = endMatrix[0] + matrixCellIndex;
            *outCell = *startCell * (1.0f - t) + *endCell * t;
        }
    }
}

/* Blends the rotation of two matrices by quaternion slerp and their translation by lerp. */
void tiMtnBlendMatrix(Matrix4 *outMatrix, Matrix4 *startMatrix, Matrix4 *endMatrix, float t) {
    float oneMinusT = 1.0f - t;
    Vector4 startQuat;
    Vector4 endQuat;
    Vector4 blendQuat;
    float translation[3];
    int row;
    int col;
    float rowAbsSum;

    if (outMatrix == 0) {
        assertFailed(tiMtnBlendDestAssertExpr, tiMtnBlendSourceFile, 99);
    }
    if (startMatrix == 0) {
        assertFailed(tiMtnBlendStartMatAssertExpr, tiMtnBlendSourceFile, 100);
    }
    if (endMatrix == 0) {
        assertFailed(tiMtnBlendEndMatAssertExpr, tiMtnBlendSourceFile, 101);
    }

    translation[0] = (*startMatrix)[3][0] * oneMinusT + (*endMatrix)[3][0] * t;
    translation[1] = (*startMatrix)[3][1] * oneMinusT + (*endMatrix)[3][1] * t;
    translation[2] = (*startMatrix)[3][2] * oneMinusT + (*endMatrix)[3][2] * t;

    for (row = 0; row < 3; row++) {
        rowAbsSum = 0.0f;
        for (col = 0; col < 3; col++) {
            rowAbsSum += __builtin_fabsf((*startMatrix)[row][col]);
        }
        if (rowAbsSum < 1.0e-5f) {
            return;
        }
    }

    XrgMatrixToQuaternion(&startQuat.x, *startMatrix);
    XrgMatrixToQuaternion(&endQuat.x, *endMatrix);
    XrgQuaternionSlerp(&blendQuat, &startQuat, &endQuat, t);
    XrgQuaternionToMatrix(*outMatrix, &blendQuat);
    (*outMatrix)[3][0] = translation[0];
    (*outMatrix)[3][1] = translation[1];
    (*outMatrix)[3][2] = translation[2];
}

/* Advances the global blend windows of the two state blocks: the matrices move from the global set toward the blend set. */
void tiMtnBlendUpdateGlobal(TiMRPackage *package) {
    float progress;
    float remaining;

    if (package == 0) {
        assertFailed(tiMtnBlendPackAssertExpr, tiMtnBlendSourceFile, 155);
    }
    if (package->bodyAnimEntry != 0) {
        unsigned int hrcEntryIndex;
        int matrixIndex;

        if ((package->bodyAnimBlend.motionFlags & TI_MTN_BLEND_REQUEST) == 0) {
            tiWordClearBits(&package->bodyAnimBlend.motionFlags, TI_MTN_BLEND_RUNNING);
        } else if ((*(long long *)&package->bodyAnimBlend.motionFlags & (TI_MTN_BLEND_RUNNING | TI_MTN_BLEND_LOCAL)) == TI_MTN_BLEND_RUNNING) {
            remaining = package->bodyAnimBlend.position - package->bodyAnimBlend.rangeMin;
            if (package->bodyAnimBlend.channelValue == 0.0f || package->bodyAnimBlend.channelValue <= remaining || remaining < 0.0f) {
                tiWordClearBits(&package->bodyAnimBlend.motionFlags, TI_MTN_BLEND_RUNNING);
                tiWordClearBits(&package->bodyAnimBlend.motionFlags, TI_MTN_BLEND_REQUEST);
                return;
            }
            if (package->blendMat == 0) {
                assertFailed(tiMtnBlendPackBlendMatAssertExpr, tiMtnBlendSourceFile, 179);
            }
            if (package->globalMat == 0) {
                assertFailed(tiMtnBlendPackGlobalMatAssertExpr, tiMtnBlendSourceFile, 180);
            }
            progress = remaining / package->bodyAnimBlend.channelValue;
            matrixIndex = 0;
            if (progress < 0.0f) {
                progress = 0.0f;
            } else if (1.0f < progress) {
                progress = 1.0f;
            }
            for (hrcEntryIndex = 0; hrcEntryIndex < package->hrcData->jointCount; hrcEntryIndex++) {
                if (!((package->bodyAnimBlend.motionFlags & TI_MTN_BLEND_SKIP_MATRIX1) && matrixIndex == 1)) {
                    tiMtnBlendMatrix(package->globalMat + matrixIndex, package->blendMat + matrixIndex, package->globalMat + matrixIndex, progress);
                }
                matrixIndex++;
            }
            for (hrcEntryIndex = 0; hrcEntryIndex < package->hrcData->joint2Count; hrcEntryIndex++) {
                tiMtnBlendMatrix(package->globalMat + matrixIndex, package->blendMat + matrixIndex, package->globalMat + matrixIndex, progress);
                matrixIndex++;
            }
            for (hrcEntryIndex = 0; hrcEntryIndex < package->hrcData->joint3Count; hrcEntryIndex++) {
                tiMtnBlendMatrix(package->globalMat + matrixIndex, package->blendMat + matrixIndex, package->globalMat + matrixIndex, progress);
                matrixIndex++;
            }
            for (hrcEntryIndex = 0; hrcEntryIndex < package->hrcData->dynamicsCount; hrcEntryIndex++) {
                tiMtnBlendMatrix(package->globalMat + matrixIndex, package->blendMat + matrixIndex, package->globalMat + matrixIndex, progress);
                matrixIndex++;
            }
        }
    }
    if (package->faceAnimEntry != 0) {
        unsigned int hrcEntryIndex;
        int matrixIndex;

        if ((package->faceAnimBlend.motionFlags & TI_MTN_BLEND_REQUEST) == 0) {
            tiWordClearBits(&package->faceAnimBlend.motionFlags, TI_MTN_BLEND_RUNNING);
        } else if ((*(long long *)&package->faceAnimBlend.motionFlags & (TI_MTN_BLEND_RUNNING | TI_MTN_BLEND_LOCAL)) == TI_MTN_BLEND_RUNNING) {
            remaining = package->faceAnimBlend.position - package->faceAnimBlend.rangeMin;
            if (package->faceAnimBlend.channelValue == 0.0f || package->faceAnimBlend.channelValue <= remaining || remaining < 0.0f) {
                tiWordClearBits(&package->faceAnimBlend.motionFlags, TI_MTN_BLEND_RUNNING);
                tiWordClearBits(&package->faceAnimBlend.motionFlags, TI_MTN_BLEND_REQUEST);
                return;
            }
            if (package->blendMat == 0) {
                assertFailed(tiMtnBlendPackBlendMatAssertExpr, tiMtnBlendSourceFile, 235);
            }
            if (package->globalMat == 0) {
                assertFailed(tiMtnBlendPackGlobalMatAssertExpr, tiMtnBlendSourceFile, 236);
            }
            progress = remaining / package->faceAnimBlend.channelValue;
            matrixIndex = package->hrcData->jointCount + package->hrcData->joint2Count + package->hrcData->joint3Count;
            if (progress < 0.0f) {
                progress = 0.0f;
            } else if (1.0f < progress) {
                progress = 1.0f;
            }
            for (hrcEntryIndex = 0; hrcEntryIndex < package->hrcData->skipCount; hrcEntryIndex++) {
                tiMtnBlendMatrix(package->globalMat + matrixIndex, package->blendMat + matrixIndex, package->globalMat + matrixIndex, progress);
                matrixIndex++;
            }
        }
    }
}

void func_0029E870(Matrix4 *dst, Matrix4 *src);
float sinf(float angle);

/* Advances the local blend windows of the two state blocks: the local matrices move from the local set toward the blend set. Block A eases its progress through sinf. */
void tiMtnBlendUpdateLocal(TiMRPackage *package) {
    float progress;
    float remaining;

    if (package == 0) {
        assertFailed(tiMtnBlendPackAssertExpr, tiMtnBlendSourceFile, 261);
    }
    if (package->bodyAnimEntry != 0) {
        unsigned int hrcEntryIndex;
        int matrixIndex;

        if ((package->bodyAnimBlend.motionFlags & TI_MTN_BLEND_REQUEST) == 0) {
            tiWordClearBits(&package->bodyAnimBlend.motionFlags, TI_MTN_BLEND_RUNNING);
        } else if ((*(long long *)&package->bodyAnimBlend.motionFlags & (TI_MTN_BLEND_RUNNING | TI_MTN_BLEND_LOCAL)) == (TI_MTN_BLEND_RUNNING | TI_MTN_BLEND_LOCAL)) {
            remaining = package->bodyAnimBlend.position - package->bodyAnimBlend.rangeMin;
            if (package->bodyAnimBlend.channelValue == 0.0f || package->bodyAnimBlend.channelValue <= remaining || remaining < 0.0f) {
                tiWordClearBits(&package->bodyAnimBlend.motionFlags, TI_MTN_BLEND_RUNNING);
                tiWordClearBits(&package->bodyAnimBlend.motionFlags, TI_MTN_BLEND_REQUEST);
                return;
            }
            if (package->blendMat == 0) {
                assertFailed(tiMtnBlendPackBlendMatAssertExpr, tiMtnBlendSourceFile, 285);
            }
            if (package->localMat == 0) {
                assertFailed(tiMtnBlendPackLocalMatAssertExpr, tiMtnBlendSourceFile, 286);
            }
            progress = remaining / package->bodyAnimBlend.channelValue;
            matrixIndex = 0;
            if (progress < 0.0f) {
                progress = 0.0f;
            } else if (1.0f < progress) {
                progress = 1.0f;
            }
            progress = sinf(progress * 3.1415927f * 0.5f);
            for (hrcEntryIndex = 0; hrcEntryIndex < package->hrcData->jointCount; hrcEntryIndex++) {
                if (!((package->bodyAnimBlend.motionFlags & TI_MTN_BLEND_SKIP_MATRIX1) && matrixIndex == 1)) {
                    if (matrixIndex == 16 || matrixIndex == 23 || matrixIndex == 3 || matrixIndex == 7) {
                        tiMtnBlendLerpMatrix(*(package->localMat + matrixIndex), *(package->blendMat + matrixIndex), *(package->localMat + matrixIndex), progress);
                        func_0029E870(package->localMat + matrixIndex, package->localMat + matrixIndex);
                    } else {
                        tiMtnBlendMatrix(package->localMat + matrixIndex, package->blendMat + matrixIndex, package->localMat + matrixIndex, progress);
                    }
                }
                matrixIndex++;
            }
            for (hrcEntryIndex = 0; hrcEntryIndex < package->hrcData->joint2Count; hrcEntryIndex++) {
                tiMtnBlendMatrix(package->localMat + matrixIndex, package->blendMat + matrixIndex, package->localMat + matrixIndex, progress);
                matrixIndex++;
            }
            for (hrcEntryIndex = 0; hrcEntryIndex < package->hrcData->joint3Count; hrcEntryIndex++) {
                tiMtnBlendMatrix(package->localMat + matrixIndex, package->blendMat + matrixIndex, package->localMat + matrixIndex, progress);
                matrixIndex++;
            }
            for (hrcEntryIndex = 0; hrcEntryIndex < package->hrcData->dynamicsCount; hrcEntryIndex++) {
                tiMtnBlendMatrix(package->localMat + matrixIndex, package->blendMat + matrixIndex, package->localMat + matrixIndex, progress);
                matrixIndex++;
            }
        }
    }
    if (package->faceAnimEntry != 0) {
        unsigned int hrcEntryIndex;
        int matrixIndex;

        if ((package->faceAnimBlend.motionFlags & TI_MTN_BLEND_REQUEST) == 0) {
            tiWordClearBits(&package->faceAnimBlend.motionFlags, TI_MTN_BLEND_RUNNING);
        } else if ((*(long long *)&package->faceAnimBlend.motionFlags & (TI_MTN_BLEND_RUNNING | TI_MTN_BLEND_LOCAL)) == (TI_MTN_BLEND_RUNNING | TI_MTN_BLEND_LOCAL)) {
            remaining = package->faceAnimBlend.position - package->faceAnimBlend.rangeMin;
            if (package->faceAnimBlend.channelValue == 0.0f || package->faceAnimBlend.channelValue <= remaining || remaining < 0.0f) {
                tiWordClearBits(&package->faceAnimBlend.motionFlags, TI_MTN_BLEND_RUNNING);
                tiWordClearBits(&package->faceAnimBlend.motionFlags, TI_MTN_BLEND_REQUEST);
                return;
            }
            if (package->blendMat == 0) {
                assertFailed(tiMtnBlendPackBlendMatAssertExpr, tiMtnBlendSourceFile, 380);
            }
            if (package->localMat == 0) {
                assertFailed(tiMtnBlendPackLocalMatAssertExpr, tiMtnBlendSourceFile, 381);
            }
            progress = remaining / package->faceAnimBlend.channelValue;
            matrixIndex = package->hrcData->jointCount + package->hrcData->joint2Count + package->hrcData->joint3Count;
            if (progress < 0.0f) {
                progress = 0.0f;
            } else if (1.0f < progress) {
                progress = 1.0f;
            }
            for (hrcEntryIndex = 0; hrcEntryIndex < package->hrcData->skipCount; hrcEntryIndex++) {
                tiMtnBlendMatrix(package->localMat + matrixIndex, package->blendMat + matrixIndex, package->localMat + matrixIndex, progress);
                matrixIndex++;
            }
        }
    }
}

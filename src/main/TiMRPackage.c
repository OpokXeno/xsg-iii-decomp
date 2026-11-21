#include "common.h"
#include "shared.h"
#include "main/TiDynamicsColl.h"
#include "main/TiMRPackage.h"
#include "main/xenoActor.h"

/* A byte offset read from a header (data-driven, not a field): the record starts at base + off bytes. */
#define BYTE_OFFSET(base, off) ((void *) (base) + (off))

typedef struct TiSolverState TiSolverBody;

void tiWordClearBits(int *word, int bits);
void tiWordSetBits(int *word, int bits);
void tiMRStateFlagsZero(int *word);
void tiMRStateSetRange(TiMRState *motionState, int rangeMin, int rangeMax);
void tiMRPackageSetAnimData(TiMRPackage *pMRPackage, TiOffsetEntry *entry);
void tiMRPackageSetFaceAnimData(TiMRPackage *pMRPackage, TiOffsetEntry *entry);
int tiMRPackageIsAnimEntryValid(TiMRPackage *pMRPackage, int unused, unsigned int index);
void tiMRPackageSetHrcData(TiMRPackage *pMRPackage, TiHrcData *value);
void tiMRPackageSetClampRange(TiMRPackage *pMRPackage, TiPamData *pam);
void tiMRPackageSetEntryTableC(TiMRPackage *pMRPackage, TiNodeTable *value);
void tiMRStateSetSpeed(TiMRState *motionState, float value);
void tiMRStateSetPosition(TiMRState *motionState, float value);
void tiMRStateSetValue(TiMRState *motionState, float value);
void tiMRPackageSetFirstWord(TiMRPackage *pMRPackage, int value);
void tiMRPackageSetLocalMat(TiMRPackage *pMRPackage, Matrix4 *value);
void tiMRPackageSetGlobalMat(TiMRPackage *pMRPackage, Matrix4 *value);
void tiMRPackageSetBlendMat(TiMRPackage *pMRPackage, Matrix4 *value);
void tiMRPackageSetPartsMat(TiMRPackage *pMRPackage, void *value);
/* Binds each dynamics node of the hierarchy to its solver body: the body's position takes the node's vector (+0x90), the body gets the node pointer and gust 1.0, and the walk steps over the node's collision entries. */
void tiMRPackageBindDynamics(TiMRPackage *pMRPackage, TiSolverBody *nodes);
struct TiXstRange;
void tiMRPackageSetXstHeader(TiMRPackage *pMRPackage, struct TiXstRange *xstHeader);
void *memset(void *destination, int value, unsigned int count);
void tiMRPackageSetVectorUniform(TiMRPackage *pMRPackage, float value);
void tiMRPackageSetVector(TiMRPackage *pMRPackage, float x, float y, float z);
void tiMRPackageSelectEntryA(TiMRPackage *pMRPackage, unsigned int index);
void tiMRPackageSelectEntryB(TiMRPackage *pMRPackage, unsigned int index);
void assertFailed(const char *message, const char *file, int line);

typedef struct TiXglCameraView {
    unsigned char unmodeled_000[0x5F4];
    float advanceScale;             /* +0x5F4: multiplies the state speed each frame */
} TiXglCameraView;

TiXglCameraView *xglStudioGetCamera2(int cameraIndex);

#define TI_MR_STATE_FLAG_ENTRY_BOUND 0x40  /* set when an entry is bound to the state */

/* Scaffold strings of the TU's .rodata: "pMRPackage" and "TiMRPackage.euc.c" (splat names). */
extern const char tiPackageArgName[];
extern const char tiSourceFileName[];
extern const char tiStateArgName[];
extern const char tiFirstWordAssertText[];
extern const char D_00542850[];
extern const char D_00542868[];
extern const char D_00542880[];
extern const char D_005428D8[];
extern const char D_00542930[];
void XenoActorConvertSourceRecord(XenoMotionState *record);

/* Sets the bits of a flag word. */
void tiWordSetBits(int *motionFlags, int bits) {
    *motionFlags |= bits;
}

/* Clears the bits of a flag word. */
void tiWordClearBits(int *motionFlags, int bits) {
    *motionFlags &= ~bits;
}

/* Zeroes one word; the init code passes the flag word of each state. */
void tiMRStateFlagsZero(int *motionFlags) {
    *motionFlags = 0;
}

/*
 * Package initializer: clears the package, sets speed 1.0 and position 0.0 on both states and the
 * scale vector (1, 1, 1), and binds the default targets and the node table.
 */
void tiMRPackageInit(TiMRPackage *pMRPackage) {
    TiMRState *motionStateA;
    TiMRState *motionStateB;

    motionStateA = &pMRPackage->bodyAnimBlend;
    motionStateB = &pMRPackage->faceAnimBlend;
    if (pMRPackage == 0) {
        assertFailed(tiPackageArgName, tiSourceFileName, 46);
    }
    memset(pMRPackage, 0, 0x140);
    tiMRStateFlagsZero(&motionStateA->motionFlags);
    tiMRStateFlagsZero(&motionStateB->motionFlags);
    tiMRStateSetPosition(motionStateA, 0.0f);
    tiMRStateSetPosition(motionStateB, 0.0f);
    tiMRStateSetValue(motionStateA, 0.0f);
    tiMRStateSetValue(motionStateB, 0.0f);
    tiMRStateSetSpeed(motionStateA, 1.0f);
    tiMRStateSetSpeed(motionStateB, 1.0f);
    tiMRPackageSetFirstWord(pMRPackage, 1);
    tiMRPackageSetVector(pMRPackage, 1.0f, 1.0f, 1.0f);
    tiMRPackageSetHrcData(pMRPackage, 0);
    tiMRPackageSetAnimData(pMRPackage, 0);
    tiMRPackageSetLocalMat(pMRPackage, 0);
    tiMRPackageSetGlobalMat(pMRPackage, 0);
    tiMRPackageSetBlendMat(pMRPackage, 0);
    tiMRPackageSetPartsMat(pMRPackage, 0);
    tiMRPackageSetClampRange(pMRPackage, 0);
    tiMRPackageSetEntryTableC(pMRPackage, 0);
    tiMRPackageBindDynamics(pMRPackage, 0);
    pMRPackage->targetNode = 0x1D;
    pMRPackage->xstRecord = 0;
    tiMRPackageSetXstHeader(pMRPackage, 0);
}

/* Stores the dynamics table pointer of the package (+0xEC). */
void tiMRPackageSetLocalMat(TiMRPackage *pMRPackage, Matrix4 *localMatrix) {
    if (pMRPackage == 0) {
        assertFailed(tiPackageArgName, tiSourceFileName, 82);
    }
    pMRPackage->localMat = localMatrix;
}

/* Stores the place matrix pointer of the package (+0xF0). */
void tiMRPackageSetGlobalMat(TiMRPackage *pMRPackage, Matrix4 *globalMatrix) {
    if (pMRPackage == 0) {
        assertFailed(tiPackageArgName, tiSourceFileName, 94);
    }
    pMRPackage->globalMat = globalMatrix;
}

void tiMRPackageSetBlendMat(TiMRPackage *pMRPackage, Matrix4 *blendMatrix) {
    if (pMRPackage == 0) {
        assertFailed(tiPackageArgName, tiSourceFileName, 106);
    }
    pMRPackage->blendMat = blendMatrix;
}

void tiMRPackageSetPartsMat(TiMRPackage *pMRPackage, void *partsMatrix) {
    if (pMRPackage == 0) {
        assertFailed(tiPackageArgName, tiSourceFileName, 118);
    }
    pMRPackage->partsMatrix = partsMatrix;
}

/* Stores the integer range of a state (min at +0x10, max at +0x14); the advance functions loop over it. */
void tiMRStateSetRange(TiMRState *motionState, int positionMin, int positionMax) {
    motionState->rangeMin = positionMin;
    motionState->rangeMax = positionMax;
}

/*
 * Binds entry A: a null entry clears +0xC0; an entry whose float range at +8
 * and +C is valid (min <= max) is stored at +0xC0 and its range copied into
 * state A (tiMRStateSetRange).
 */
void tiMRPackageSetAnimData(TiMRPackage *pMRPackage, TiOffsetEntry *animEntry) {
    if (pMRPackage == 0) {
        assertFailed(tiPackageArgName, tiSourceFileName, 142);
    }
    if (animEntry != 0) {
        if (animEntry->rangeMin <= animEntry->rangeMax) {
            pMRPackage->bodyAnimEntry = animEntry;
            tiMRStateSetRange(&pMRPackage->bodyAnimBlend, animEntry->rangeMin, animEntry->rangeMax);
        }
    } else {
        pMRPackage->bodyAnimEntry = 0;
    }
}

/* Stores the offset table A pointer of the package (+0xC4). */
void tiMRPackageSetAnimTable(TiMRPackage *pMRPackage, OffsetTable *animTable) {
    if (pMRPackage == 0) {
        assertFailed(tiPackageArgName, tiSourceFileName, 165);
    }
    pMRPackage->entryTableA = animTable;
}

/* Binds entry B the same way as tiMRPackageSetAnimData, into +0xC8 and state B at +0x60. */
void tiMRPackageSetFaceAnimData(TiMRPackage *pMRPackage, TiOffsetEntry *faceEntry) {
    if (pMRPackage == 0) {
        assertFailed(tiPackageArgName, tiSourceFileName, 177);
    }
    if (faceEntry != 0) {
        if (faceEntry->rangeMin <= faceEntry->rangeMax) {
            pMRPackage->faceAnimEntry = faceEntry;
            tiMRStateSetRange(&pMRPackage->faceAnimBlend, faceEntry->rangeMin, faceEntry->rangeMax);
        }
    } else {
        pMRPackage->faceAnimEntry = 0;
    }
}

/*
 * Returns 1 when entry `index` of offset table A exists and its range is valid
 * (float at +8 <= float at +C); 0 otherwise. The second argument is not read.
 */
int tiMRPackageIsAnimEntryValid(TiMRPackage *pMRPackage, int unused, unsigned int animEntryIndex) {
    OffsetTable *animTable;
    TiOffsetEntry *animEntry;

    animTable = pMRPackage->entryTableA;
    if (animTable == 0) {
        return 0;
    }
    if (animEntryIndex >= animTable->count) {
        return 0;
    }
    /* entryOffsets holds byte offsets from the table. The pointer form is kept: table->entryOffsets[index] changes the code. */
    animEntry = BYTE_OFFSET(animTable, *(animTable->entryOffsets + animEntryIndex));
    if (animEntry->rangeMin <= animEntry->rangeMax) {
        return 1;
    }
    return 0;
}

/* Stores the face offset table pointer of the package (+0xCC). */
void tiMRPackageSetFaceAnimTable(TiMRPackage *pMRPackage, OffsetTable *faceTable) {
    if (pMRPackage == 0) {
        assertFailed(tiPackageArgName, tiSourceFileName, 228);
    }
    pMRPackage->entryTableB = faceTable;
}

/* Stores the hierarchy data pointer of the package (+0xD0). */
void tiMRPackageSetHrcData(TiMRPackage *pMRPackage, TiHrcData *hrcData) {
    if (pMRPackage == 0) {
        assertFailed(tiPackageArgName, tiSourceFileName, 240);
    }
    pMRPackage->hrcData = hrcData;
}

/* Stores the parts animation data pointer of the package (+0xD4). */
void tiMRPackageSetClampRange(TiMRPackage *pMRPackage, TiPamData *pam) {
    if (pMRPackage == 0) {
        assertFailed(tiPackageArgName, tiSourceFileName, 252);
    }
    pMRPackage->pamData = pam;
}

/* Stores the node table pointer of the package (+0xD8). */
void tiMRPackageSetEntryTableC(TiMRPackage *pMRPackage, TiNodeTable *nodePointTable) {
    if (pMRPackage == 0) {
        assertFailed(tiPackageArgName, tiSourceFileName, 264);
    }
    pMRPackage->nodeTable = nodePointTable;
}

/* Stores the rate of a state (speed, +4). */
void tiMRStateSetSpeed(TiMRState *motionState, float speed) {
    if (motionState == 0) {
        assertFailed(tiStateArgName, tiSourceFileName, 276);
    }
    motionState->speed = speed;
}

/* Stores the position of a state (+8), the value the advance functions accumulate. */
void tiMRStateSetPosition(TiMRState *motionState, float position) {
    if (motionState == 0) {
        assertFailed(tiStateArgName, tiSourceFileName, 288);
    }
    motionState->position = position;
}

/* Stores the value of a state (+0xC). */
void tiMRStateSetValue(TiMRState *motionState, float channelValue) {
    if (motionState == 0) {
        assertFailed(tiStateArgName, tiSourceFileName, 300);
    }
    motionState->channelValue = channelValue;
}

/* Stores the first word of the package (set to 1 by the initializer and by the per-frame update). */
void tiMRPackageSetFirstWord(TiMRPackage *pMRPackage, int firstWord) {
    if (pMRPackage == 0) {
        assertFailed(tiFirstWordAssertText, tiSourceFileName, 312);
    }
    pMRPackage->first_word = firstWord;
}

/* Sets the package scale vector to one value. */
void tiMRPackageSetVectorUniform(TiMRPackage *pMRPackage, float scaleValue) {
    pMRPackage->scale[2] = scaleValue;
    pMRPackage->scale[1] = scaleValue;
    pMRPackage->scale[0] = scaleValue;
}

/* Sets the package scale vector. */
void tiMRPackageSetVector(TiMRPackage *pMRPackage, float x, float y, float z) {
    pMRPackage->scale[0] = x;
    pMRPackage->scale[1] = y;
    pMRPackage->scale[2] = z;
}

void tiMRPackageAdvanceAnim(TiMRPackage *pMRPackage) {
    TiXglCameraView *camera;
    TiPamData *pam;

    if (pMRPackage == 0) {
        assertFailed(tiPackageArgName, tiSourceFileName, 350);
    }
    camera = xglStudioGetCamera2(0);
    pMRPackage->bodyAnimBlend.position = pMRPackage->bodyAnimBlend.position + pMRPackage->bodyAnimBlend.speed * camera->advanceScale;
    if (pMRPackage->xstData != 0) {
        if (pMRPackage->bodyAnimBlend.rangeMax <= pMRPackage->bodyAnimBlend.position) {
            pMRPackage->bodyAnimBlend.position = pMRPackage->bodyAnimBlend.rangeMin;
            tiWordSetBits(&pMRPackage->bodyAnimBlend.motionFlags, 8);
            tiWordClearBits(&pMRPackage->bodyAnimBlend.motionFlags, 2);
        } else if (pMRPackage->bodyAnimBlend.position < pMRPackage->bodyAnimBlend.rangeMin) {
            pMRPackage->bodyAnimBlend.position = pMRPackage->bodyAnimBlend.rangeMax;
            tiWordSetBits(&pMRPackage->bodyAnimBlend.motionFlags, 8);
            tiWordClearBits(&pMRPackage->bodyAnimBlend.motionFlags, 2);
        } else {
            tiWordClearBits(&pMRPackage->bodyAnimBlend.motionFlags, 8);
        }
    }
    if (pMRPackage->bodyAnimEntry != 0) {
        if (pMRPackage->bodyAnimBlend.rangeMax <= pMRPackage->bodyAnimBlend.position) {
            pMRPackage->bodyAnimBlend.position = pMRPackage->bodyAnimBlend.rangeMin;
            tiWordSetBits(&pMRPackage->bodyAnimBlend.motionFlags, 8);
            tiWordClearBits(&pMRPackage->bodyAnimBlend.motionFlags, 2);
        } else if (pMRPackage->bodyAnimBlend.position < pMRPackage->bodyAnimBlend.rangeMin) {
            pMRPackage->bodyAnimBlend.position = pMRPackage->bodyAnimBlend.rangeMax;
            tiWordSetBits(&pMRPackage->bodyAnimBlend.motionFlags, 8);
            tiWordClearBits(&pMRPackage->bodyAnimBlend.motionFlags, 2);
        } else {
            tiWordClearBits(&pMRPackage->bodyAnimBlend.motionFlags, 8);
        }
    }
    pam = pMRPackage->pamData;
    if (pam != 0) {
        if (pam->upperLimit <= pMRPackage->bodyAnimBlend.position) {
            pMRPackage->bodyAnimBlend.position = pam->lowerLimit;
            tiWordSetBits(&pMRPackage->bodyAnimBlend.motionFlags, 8);
        } else if (pMRPackage->bodyAnimBlend.position < pam->lowerLimit) {
            pMRPackage->bodyAnimBlend.position = pam->upperLimit;
            tiWordSetBits(&pMRPackage->bodyAnimBlend.motionFlags, 8);
        } else {
            tiWordClearBits(&pMRPackage->bodyAnimBlend.motionFlags, 8);
        }
    }
}

void tiMRPackageAdvanceAnimUnwrapped(TiMRPackage *pMRPackage) {
    TiXglCameraView *camera;
    TiPamData *pam;

    if (pMRPackage == 0) {
        assertFailed(tiPackageArgName, tiSourceFileName, 434);
    }
    camera = xglStudioGetCamera2(0);
    pMRPackage->bodyAnimBlend.position = pMRPackage->bodyAnimBlend.position + pMRPackage->bodyAnimBlend.speed * camera->advanceScale;
    if (pMRPackage->bodyAnimEntry != 0) {
        if (pMRPackage->bodyAnimBlend.rangeMax <= pMRPackage->bodyAnimBlend.position) {
            tiWordSetBits(&pMRPackage->bodyAnimBlend.motionFlags, 8);
            if (pMRPackage->bodyAnimBlend.motionFlags & 2) {
                tiWordClearBits(&pMRPackage->bodyAnimBlend.motionFlags, 2);
            }
        } else if (pMRPackage->bodyAnimBlend.position < pMRPackage->bodyAnimBlend.rangeMin) {
            tiWordSetBits(&pMRPackage->bodyAnimBlend.motionFlags, 8);
            if (pMRPackage->bodyAnimBlend.motionFlags & 2) {
                tiWordClearBits(&pMRPackage->bodyAnimBlend.motionFlags, 2);
            }
        } else {
            tiWordClearBits(&pMRPackage->bodyAnimBlend.motionFlags, 8);
        }
    }
    pam = pMRPackage->pamData;
    if (pam != 0) {
        if (pam->upperLimit <= pMRPackage->bodyAnimBlend.position) {
            tiWordSetBits(&pMRPackage->bodyAnimBlend.motionFlags, 8);
        } else if (pMRPackage->bodyAnimBlend.position < pam->lowerLimit) {
            tiWordSetBits(&pMRPackage->bodyAnimBlend.motionFlags, 8);
        } else {
            tiWordClearBits(&pMRPackage->bodyAnimBlend.motionFlags, 8);
        }
    }
}

void tiMRPackageAdvanceFaceAnim(TiMRPackage *pMRPackage) {
    TiXglCameraView *camera;

    if (pMRPackage == 0) {
        assertFailed(tiPackageArgName, tiSourceFileName, 490);
    }
    camera = xglStudioGetCamera2(0);
    pMRPackage->faceAnimBlend.position = pMRPackage->faceAnimBlend.position + pMRPackage->faceAnimBlend.speed * camera->advanceScale;
    if (pMRPackage->faceAnimEntry != 0) {
        if (pMRPackage->faceAnimBlend.rangeMax <= pMRPackage->faceAnimBlend.position) {
            pMRPackage->faceAnimBlend.position = pMRPackage->faceAnimBlend.rangeMin;
            tiWordSetBits(&pMRPackage->faceAnimBlend.motionFlags, 8);
            tiWordClearBits(&pMRPackage->faceAnimBlend.motionFlags, 2);
        } else if (pMRPackage->faceAnimBlend.position < pMRPackage->faceAnimBlend.rangeMin) {
            pMRPackage->faceAnimBlend.position = pMRPackage->faceAnimBlend.rangeMax;
            tiWordSetBits(&pMRPackage->faceAnimBlend.motionFlags, 8);
            tiWordClearBits(&pMRPackage->faceAnimBlend.motionFlags, 2);
        } else {
            tiWordClearBits(&pMRPackage->faceAnimBlend.motionFlags, 8);
        }
    }
}

/* The part of the camera that the advance functions read: the step at +0x5F4 (set to 1.0f when the camera is initialized). */
/* Advances state B: position += speed * advanceScale, then sets flag 0x8 (and clears 0x2) past rangeMax or below rangeMin, else clears 0x8. */
void tiMRPackageAdvanceFaceAnimUnwrapped(TiMRPackage *pMRPackage) {
    TiXglCameraView *camera;

    if (pMRPackage == 0) {
        assertFailed(tiPackageArgName, tiSourceFileName, 528);
    }
    camera = xglStudioGetCamera2(0);
    pMRPackage->faceAnimBlend.position = pMRPackage->faceAnimBlend.position + pMRPackage->faceAnimBlend.speed * camera->advanceScale;
    if (pMRPackage->faceAnimEntry != 0) {
        if (pMRPackage->faceAnimBlend.rangeMax <= pMRPackage->faceAnimBlend.position) {
            tiWordSetBits(&pMRPackage->faceAnimBlend.motionFlags, 8);
            if (pMRPackage->faceAnimBlend.motionFlags & 2) {
                tiWordClearBits(&pMRPackage->faceAnimBlend.motionFlags, 2);
            }
        } else if (pMRPackage->faceAnimBlend.position < pMRPackage->faceAnimBlend.rangeMin) {
            tiWordSetBits(&pMRPackage->faceAnimBlend.motionFlags, 8);
            if (pMRPackage->faceAnimBlend.motionFlags & 2) {
                tiWordClearBits(&pMRPackage->faceAnimBlend.motionFlags, 2);
            }
        } else {
            tiWordClearBits(&pMRPackage->faceAnimBlend.motionFlags, 8);
        }
    }
}

extern float D_005500B4;
extern float D_005500B8;
extern float D_005500BC;
extern float D_005500C0;
extern float D_005500C4;
void func_00299E98(TiMRPackage *pMRPackage, Matrix4 *globalMat);
void func_0029AD70(TiMRPackage *pMRPackage);
void func_0029C9A8(TiMRPackage *pMRPackage, Matrix4 *globalMat);
void func_0029B8C0(TiMRPackage *pMRPackage);
void tiMtnBlendUpdateLocal(TiMRPackage *pMRPackage);

INCLUDE_ASM("asm/main/nonmatchings/TiMRPackage", func_00299350);

/* Binds entry `index` of table A to state A and sets flag 0x40 of state A. */
void tiMRPackageSelectEntryA(TiMRPackage *pMRPackage, unsigned int animEntryIndex) {
    OffsetTable *animTable;
    int animEntryOffset;

    animTable = pMRPackage->entryTableA;
    if (animTable == 0) {
        return;
    }
    if (animEntryIndex < animTable->count) {
        animEntryOffset = *(animTable->entryOffsets + animEntryIndex);
        tiMRPackageSetAnimData(pMRPackage, BYTE_OFFSET(animTable, animEntryOffset));
        tiWordSetBits(&pMRPackage->bodyAnimBlend.motionFlags, TI_MR_STATE_FLAG_ENTRY_BOUND);
    }
}

/* Stores table A (the argument), then binds entry `index` of it to state A as tiMRPackageSelectEntryA. */
void tiMRPackageSetAnimTableAndEntry(TiMRPackage *pMRPackage, OffsetTable *animTable, unsigned int animEntryIndex) {
    int animEntryOffset;

    if (animTable == 0) {
        return;
    }
    if (animEntryIndex < animTable->count) {
        animEntryOffset = *(animTable->entryOffsets + animEntryIndex);
        tiMRPackageSetAnimTable(pMRPackage, animTable);
        tiMRPackageSetAnimData(pMRPackage, BYTE_OFFSET(animTable, animEntryOffset));
        tiWordSetBits(&pMRPackage->bodyAnimBlend.motionFlags, TI_MR_STATE_FLAG_ENTRY_BOUND);
    }
}

/* Binds entry `index` of table B to state B and sets flag 0x40 of state B. */
void tiMRPackageSelectEntryB(TiMRPackage *pMRPackage, unsigned int faceEntryIndex) {
    OffsetTable *faceTable;
    int faceEntryOffset;

    faceTable = pMRPackage->entryTableB;
    if (faceTable == 0) {
        return;
    }
    if (faceEntryIndex < faceTable->count) {
        faceEntryOffset = *(faceTable->entryOffsets + faceEntryIndex);
        tiMRPackageSetFaceAnimData(pMRPackage, BYTE_OFFSET(faceTable, faceEntryOffset));
        tiWordSetBits(&pMRPackage->faceAnimBlend.motionFlags, TI_MR_STATE_FLAG_ENTRY_BOUND);
    }
}

#include "quadword.h"

/*
 * Binds each dynamics node of the hierarchy to its solver body: the body's position takes the node's vector
 * (+0x90), the body gets the node pointer and gust 1.0, and the walk steps over the node's collision entries.
 */
void tiMRPackageBindDynamics(TiMRPackage *pMRPackage, TiSolverBody *solverBodies) {
    TiHrcData *hrc;
    TiHrcDynamics *dynamics;
    TiSolverBody *solverBody;
    TiCollEntry *collEntries;
    unsigned int dynamicsIndex;

    pMRPackage->dynMat = solverBodies;
    if (solverBodies != 0) {
        hrc = pMRPackage->hrcData;
        dynamicsIndex = 0;
        dynamics = BYTE_OFFSET(hrc, hrc->dynamicsOffset);
        if (hrc->dynamicsCount != 0) {
            solverBody = solverBodies;
            do {
                QuadwordCopy(&solverBody->position, &dynamics->bodyPosition);
                dynamicsIndex++;
                solverBody->node = dynamics;
                solverBody->gust = 1.0f;
                solverBody++;
                collEntries = dynamics->entries;
                dynamics = BYTE_OFFSET(collEntries,
                    (dynamics->sphereCount + dynamics->matrixCount + dynamics->capsuleCount) * sizeof(TiCollEntry));
            } while (dynamicsIndex < pMRPackage->hrcData->dynamicsCount);
        }
    }
}

/* Converts the source value (range[0]) of a motion state to the float at +8 of the state. */
void XenoActorConvertSourceRecord(XenoMotionState *motionState) {
    motionState->frame = motionState->range[0];
}

/* The range words of an Xst header: min at +0xC and max at +0x10 (the state A range). */
struct TiXstRange {
    unsigned char unmodeled_00[0xC];
    int xstRangeMin;                /* +0xC */
    int xstRangeMax;                /* +0x10 */
};

/* Binds the Xst header to the package and takes its range as state A's range; 0 unbinds it. */
void tiMRPackageSetXstHeader(TiMRPackage *pMRPackage, struct TiXstRange *xstHeader) {
    if (xstHeader != 0) {
        pMRPackage->xstData = xstHeader;
        tiMRStateSetRange(&pMRPackage->bodyAnimBlend, xstHeader->xstRangeMin, xstHeader->xstRangeMax);
        return;
    }
    pMRPackage->xstData = 0;
}

/* Four-character tag of an Xst file record, read as one word and as its bytes. */
typedef union TiFourCC {
    unsigned int fourCC;
    unsigned char fourCCChars[4];
} TiFourCC;

/* The Xst file header that localMat points at: tag "XST ", the cluster count at +8, the first cluster at +0x40. */
typedef struct TiXstHeader {
    TiFourCC tag;
    unsigned char unmodeled_04[4];
    unsigned int clusterCount;      /* +8 */
    unsigned char unmodeled_0C[0x34];
    struct TiXstCluster *firstCluster; /* +0x40 */
} TiXstHeader;

/* One cluster: tag "CLST", the float range (int fields) at +4 and +8, its size at +0xC, the flag at +0x18 and the element offset at +0x1C. */
typedef struct TiXstCluster {
    TiFourCC tag;
    int clusterRangeMin;            /* +4 */
    int clusterRangeMax;            /* +8 */
    int clusterSize;                /* +0xC: the next cluster is size + 0xB0 bytes on */
    unsigned char unmodeled_10[8];
    int validFlag;                  /* +0x18: must be 1 */
    int echrOffset;              /* +0x1C: offset from the header to the element, 0 when none */
} TiXstCluster;

/* The element a cluster points at: tag "ECHR". */
typedef struct TiXstElement {
    TiFourCC tag;
} TiXstElement;

extern unsigned char D_00542988[];
extern unsigned char D_005429A0[];
extern unsigned char D_005429C0[];
extern unsigned char D_005429E8[];
int printf(const char *format, ...);

/*
 * Finds the cluster whose range holds the state A position, binds its element to globalMat and stores
 * the offset of the position in it at +0xE8. Each failed tag check prints its message and returns.
 */
INCLUDE_ASM("asm/main/nonmatchings/TiMRPackage", func_00299770);

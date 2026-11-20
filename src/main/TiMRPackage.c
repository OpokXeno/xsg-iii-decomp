#include "common.h"
#include "shared.h"
#include "main/TiDynamicsColl.h"
#include "main/TiMRPackage.h"
#include "main/xenoActor.h"

typedef struct TiSolverState TiDynamicsOut;

void tiWordClearBits(int *word, int bits);
void tiWordSetBits(int *word, int bits);
void tiWordZero(int *word);
void tiMRStateSetRange(TiMRState *pState, int rangeMin, int rangeMax);
void tiMRPackageSetAnimData(TiMRPackage *pMRPackage, TiOffsetEntry *entry);
void tiMRPackageSetFaceAnimData(TiMRPackage *pMRPackage, TiOffsetEntry *entry);
int tiMRPackageIsEntryValidA(TiMRPackage *pMRPackage, int unused, unsigned int index);
void tiMRPackageSetHrcData(TiMRPackage *pMRPackage, TiHrcData *value);
void tiMRPackageSetClampRange(TiMRPackage *pMRPackage, TiPamData *pam);
void tiMRPackageSetEntryTableC(TiMRPackage *pMRPackage, TiNodeTable *value);
void tiMRStateSetSpeed(TiMRState *pState, float value);
void tiMRStateSetPosition(TiMRState *pState, float value);
void tiMRStateSetValue(TiMRState *pState, float value);
void tiMRPackageSetFirstWord(TiMRPackage *pMRPackage, int value);
void tiMRPackageSetLocalMat(TiMRPackage *pMRPackage, TiDynamicsOut *value);
void tiMRPackageSetGlobalMat(TiMRPackage *pMRPackage, Matrix4 *value);
void tiMRPackageSetBlendMat(TiMRPackage *pMRPackage, void *value);
void tiMRPackageSetPartsMat(TiMRPackage *pMRPackage, void *value);
/* * Binds each dynamics node of the hierarchy to its solver body: the body's position takes the node's vector * (+0x90), the body gets the node pointer and gust 1.0, and the walk steps over the node's collision entries. */ void func_00299698(TiMRPackage *pMRPackage, TiDynamicsOut *nodes);
void func_00299730(TiMRPackage *pMRPackage, int value);
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
extern const char D_00542818[];
extern const char D_00542828[];
extern const char D_00542840[];
extern const char D_00542848[];
extern const char D_00542850[];
extern const char D_00542868[];
extern const char D_00542880[];
extern const char D_005428D8[];
extern const char D_00542930[];
void XenoActorConvertSourceRecord(XenoMotionState *record);

/* Sets the bits of a flag word. */
void tiWordSetBits(int *word, int bits) {
    *word |= bits;
}

/* Clears the bits of a flag word. */
void tiWordClearBits(int *word, int bits) {
    *word &= ~bits;
}

/* Zeroes one word; the init code passes the flag word of each state. */
void tiWordZero(int *word) {
    *word = 0;
}

/*
 * Package initializer: clears the package, sets speed 1.0 and position 0.0 on both states and the
 * scale vector (1, 1, 1), and binds the default targets and the node table.
 */
void tiMRPackageInit(TiMRPackage *pMRPackage) {
    TiMRState *pStateA;
    TiMRState *pStateB;

    pStateA = &pMRPackage->state_a;
    pStateB = &pMRPackage->state_b;
    if (pMRPackage == 0) {
        assertFailed(D_00542818, D_00542828, 46);
    }
    memset(pMRPackage, 0, 0x140);
    tiWordZero(&pStateA->flags);
    tiWordZero(&pStateB->flags);
    tiMRStateSetPosition(pStateA, 0.0f);
    tiMRStateSetPosition(pStateB, 0.0f);
    tiMRStateSetValue(pStateA, 0.0f);
    tiMRStateSetValue(pStateB, 0.0f);
    tiMRStateSetSpeed(pStateA, 1.0f);
    tiMRStateSetSpeed(pStateB, 1.0f);
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
    func_00299698(pMRPackage, 0);
    pMRPackage->targetNode = 0x1D;
    pMRPackage->xstRecord = 0;
    func_00299730(pMRPackage, 0);
}

/* Stores the dynamics table pointer of the package (+0xEC). */
void tiMRPackageSetLocalMat(TiMRPackage *pMRPackage, TiDynamicsOut *value) {
    if (pMRPackage == 0) {
        assertFailed(D_00542818, D_00542828, 82);
    }
    pMRPackage->localMat = value;
}

/* Stores the place matrix pointer of the package (+0xF0). */
void tiMRPackageSetGlobalMat(TiMRPackage *pMRPackage, Matrix4 *value) {
    if (pMRPackage == 0) {
        assertFailed(D_00542818, D_00542828, 94);
    }
    pMRPackage->globalMat = value;
}

void tiMRPackageSetBlendMat(TiMRPackage *pMRPackage, void *value) {
    if (pMRPackage == 0) {
        assertFailed(D_00542818, D_00542828, 106);
    }
    pMRPackage->blendMat = value;
}

void tiMRPackageSetPartsMat(TiMRPackage *pMRPackage, void *value) {
    if (pMRPackage == 0) {
        assertFailed(D_00542818, D_00542828, 118);
    }
    pMRPackage->valueF8 = value;
}

/* Stores the integer range of a state (min at +0x10, max at +0x14); the advance functions loop over it. */
void tiMRStateSetRange(TiMRState *pState, int rangeMin, int rangeMax) {
    pState->rangeMin = rangeMin;
    pState->rangeMax = rangeMax;
}

/*
 * Binds entry A: a null entry clears +0xC0; an entry whose float range at +8
 * and +C is valid (min <= max) is stored at +0xC0 and its range copied into
 * state A (tiMRStateSetRange).
 */
void tiMRPackageSetAnimData(TiMRPackage *pMRPackage, TiOffsetEntry *entry) {
    if (pMRPackage == 0) {
        assertFailed(D_00542818, D_00542828, 142);
    }
    if (entry != 0) {
        if (entry->rangeMin <= entry->rangeMax) {
            pMRPackage->entryA = entry;
            tiMRStateSetRange(&pMRPackage->state_a, (int) entry->rangeMin, (int) entry->rangeMax);
        }
    } else {
        pMRPackage->entryA = 0;
    }
}

/* Stores the offset table A pointer of the package (+0xC4). */
void tiMRPackageSetAnimTable(TiMRPackage *pMRPackage, OffsetTable *table) {
    if (pMRPackage == 0) {
        assertFailed(D_00542818, D_00542828, 165);
    }
    pMRPackage->entryTableA = table;
}

/* Binds entry B the same way as tiMRPackageSetAnimData, into +0xC8 and state B at +0x60. */
void tiMRPackageSetFaceAnimData(TiMRPackage *pMRPackage, TiOffsetEntry *entry) {
    if (pMRPackage == 0) {
        assertFailed(D_00542818, D_00542828, 177);
    }
    if (entry != 0) {
        if (entry->rangeMin <= entry->rangeMax) {
            pMRPackage->entryB = entry;
            tiMRStateSetRange(&pMRPackage->state_b, (int) entry->rangeMin, (int) entry->rangeMax);
        }
    } else {
        pMRPackage->entryB = 0;
    }
}

/*
 * Returns 1 when entry `index` of offset table A exists and its range is valid
 * (float at +8 <= float at +C); 0 otherwise. The second argument is not read.
 */
int tiMRPackageIsEntryValidA(TiMRPackage *pMRPackage, int unused, unsigned int index) {
    OffsetTable *table;
    TiOffsetEntry *entry;

    table = pMRPackage->entryTableA;
    if (table == 0) {
        return 0;
    }
    if (index >= table->count) {
        return 0;
    }
    /* entryOffsets is a byte offset from the table (the form index[table->entryOffsets] is kept: table->entryOffsets[index] changed the code). */
    entry = (TiOffsetEntry *) ((unsigned char *) table + index[table->entryOffsets]);
    if (entry->rangeMin <= entry->rangeMax) {
        return 1;
    }
    return 0;
}

/* Stores the face offset table pointer of the package (+0xCC). */
void tiMRPackageSetFaceAnimTable(TiMRPackage *pMRPackage, OffsetTable *table) {
    if (pMRPackage == 0) {
        assertFailed(D_00542818, D_00542828, 228);
    }
    pMRPackage->entryTableB = table;
}

/* Stores the hierarchy data pointer of the package (+0xD0). */
void tiMRPackageSetHrcData(TiMRPackage *pMRPackage, TiHrcData *value) {
    if (pMRPackage == 0) {
        assertFailed(D_00542818, D_00542828, 240);
    }
    pMRPackage->hrcData = value;
}

/* Stores the parts animation data pointer of the package (+0xD4). */
void tiMRPackageSetClampRange(TiMRPackage *pMRPackage, TiPamData *pam) {
    if (pMRPackage == 0) {
        assertFailed(D_00542818, D_00542828, 252);
    }
    pMRPackage->pamData = pam;
}

/* Stores the node table pointer of the package (+0xD8). */
void tiMRPackageSetEntryTableC(TiMRPackage *pMRPackage, TiNodeTable *value) {
    if (pMRPackage == 0) {
        assertFailed(D_00542818, D_00542828, 264);
    }
    pMRPackage->nodeTable = value;
}

/* Stores the rate of a state (speed, +4). */
void tiMRStateSetSpeed(TiMRState *pState, float value) {
    if (pState == 0) {
        assertFailed(D_00542840, D_00542828, 276);
    }
    pState->speed = value;
}

/* Stores the position of a state (+8), the value the advance functions accumulate. */
void tiMRStateSetPosition(TiMRState *pState, float value) {
    if (pState == 0) {
        assertFailed(D_00542840, D_00542828, 288);
    }
    pState->position = value;
}

/* Stores the value of a state (+0xC). */
void tiMRStateSetValue(TiMRState *pState, float value) {
    if (pState == 0) {
        assertFailed(D_00542840, D_00542828, 300);
    }
    pState->value = value;
}

/* Stores the first word of the package (set to 1 by the initializer and by the per-frame update). */
void tiMRPackageSetFirstWord(TiMRPackage *pMRPackage, int value) {
    if (pMRPackage == 0) {
        assertFailed(D_00542848, D_00542828, 312);
    }
    pMRPackage->first_word = value;
}

/* Sets the package scale vector to one value. */
void tiMRPackageSetVectorUniform(TiMRPackage *pMRPackage, float value) {
    pMRPackage->scale[2] = value;
    pMRPackage->scale[1] = value;
    pMRPackage->scale[0] = value;
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
        assertFailed(D_00542818, D_00542828, 350);
    }
    camera = xglStudioGetCamera2(0);
    pMRPackage->state_a.position = pMRPackage->state_a.position + pMRPackage->state_a.speed * camera->advanceScale;
    if (pMRPackage->xstData != 0) {
        if ((float) pMRPackage->state_a.rangeMax <= pMRPackage->state_a.position) {
            pMRPackage->state_a.position = (float) pMRPackage->state_a.rangeMin;
            tiWordSetBits(&pMRPackage->state_a.flags, 8);
            tiWordClearBits(&pMRPackage->state_a.flags, 2);
        } else if (pMRPackage->state_a.position < (float) pMRPackage->state_a.rangeMin) {
            pMRPackage->state_a.position = (float) pMRPackage->state_a.rangeMax;
            tiWordSetBits(&pMRPackage->state_a.flags, 8);
            tiWordClearBits(&pMRPackage->state_a.flags, 2);
        } else {
            tiWordClearBits(&pMRPackage->state_a.flags, 8);
        }
    }
    if (pMRPackage->entryA != 0) {
        if ((float) pMRPackage->state_a.rangeMax <= pMRPackage->state_a.position) {
            pMRPackage->state_a.position = (float) pMRPackage->state_a.rangeMin;
            tiWordSetBits(&pMRPackage->state_a.flags, 8);
            tiWordClearBits(&pMRPackage->state_a.flags, 2);
        } else if (pMRPackage->state_a.position < (float) pMRPackage->state_a.rangeMin) {
            pMRPackage->state_a.position = (float) pMRPackage->state_a.rangeMax;
            tiWordSetBits(&pMRPackage->state_a.flags, 8);
            tiWordClearBits(&pMRPackage->state_a.flags, 2);
        } else {
            tiWordClearBits(&pMRPackage->state_a.flags, 8);
        }
    }
    pam = pMRPackage->pamData;
    if (pam != 0) {
        if ((float) pam->upperLimit <= pMRPackage->state_a.position) {
            pMRPackage->state_a.position = (float) pam->lowerLimit;
            tiWordSetBits(&pMRPackage->state_a.flags, 8);
        } else if (pMRPackage->state_a.position < (float) pam->lowerLimit) {
            pMRPackage->state_a.position = (float) pam->upperLimit;
            tiWordSetBits(&pMRPackage->state_a.flags, 8);
        } else {
            tiWordClearBits(&pMRPackage->state_a.flags, 8);
        }
    }
}

void tiMRPackageAdvanceAnimUnwrapped(TiMRPackage *pMRPackage) {
    TiXglCameraView *camera;
    TiPamData *pam;

    if (pMRPackage == 0) {
        assertFailed(D_00542818, D_00542828, 434);
    }
    camera = xglStudioGetCamera2(0);
    pMRPackage->state_a.position = pMRPackage->state_a.position + pMRPackage->state_a.speed * camera->advanceScale;
    if (pMRPackage->entryA != 0) {
        if ((float) pMRPackage->state_a.rangeMax <= pMRPackage->state_a.position) {
            tiWordSetBits(&pMRPackage->state_a.flags, 8);
            if (pMRPackage->state_a.flags & 2) {
                tiWordClearBits(&pMRPackage->state_a.flags, 2);
            }
        } else if (pMRPackage->state_a.position < (float) pMRPackage->state_a.rangeMin) {
            tiWordSetBits(&pMRPackage->state_a.flags, 8);
            if (pMRPackage->state_a.flags & 2) {
                tiWordClearBits(&pMRPackage->state_a.flags, 2);
            }
        } else {
            tiWordClearBits(&pMRPackage->state_a.flags, 8);
        }
    }
    pam = pMRPackage->pamData;
    if (pam != 0) {
        if ((float) pam->upperLimit <= pMRPackage->state_a.position) {
            tiWordSetBits(&pMRPackage->state_a.flags, 8);
        } else if (pMRPackage->state_a.position < (float) pam->lowerLimit) {
            tiWordSetBits(&pMRPackage->state_a.flags, 8);
        } else {
            tiWordClearBits(&pMRPackage->state_a.flags, 8);
        }
    }
}

void tiMRPackageAdvanceFaceAnim(TiMRPackage *pMRPackage) {
    TiXglCameraView *camera;

    if (pMRPackage == 0) {
        assertFailed(D_00542818, D_00542828, 490);
    }
    camera = xglStudioGetCamera2(0);
    pMRPackage->state_b.position = pMRPackage->state_b.position + pMRPackage->state_b.speed * camera->advanceScale;
    if (pMRPackage->entryB != 0) {
        if ((float) pMRPackage->state_b.rangeMax <= pMRPackage->state_b.position) {
            pMRPackage->state_b.position = (float) pMRPackage->state_b.rangeMin;
            tiWordSetBits(&pMRPackage->state_b.flags, 8);
            tiWordClearBits(&pMRPackage->state_b.flags, 2);
        } else if (pMRPackage->state_b.position < (float) pMRPackage->state_b.rangeMin) {
            pMRPackage->state_b.position = (float) pMRPackage->state_b.rangeMax;
            tiWordSetBits(&pMRPackage->state_b.flags, 8);
            tiWordClearBits(&pMRPackage->state_b.flags, 2);
        } else {
            tiWordClearBits(&pMRPackage->state_b.flags, 8);
        }
    }
}

/* The part of the camera that the advance functions read: the step at +0x5F4 (set to 1.0f when the camera is initialized). */
/* Advances state B: position += speed * advanceScale, then sets flag 0x8 (and clears 0x2) past rangeMax or below rangeMin, else clears 0x8. */
void tiMRPackageAdvanceFaceAnimUnwrapped(TiMRPackage *pMRPackage) {
    TiXglCameraView *camera;

    if (pMRPackage == 0) {
        assertFailed(D_00542818, D_00542828, 528);
    }
    camera = xglStudioGetCamera2(0);
    pMRPackage->state_b.position = pMRPackage->state_b.position + pMRPackage->state_b.speed * camera->advanceScale;
    if (pMRPackage->entryB != 0) {
        if ((float) pMRPackage->state_b.rangeMax <= pMRPackage->state_b.position) {
            tiWordSetBits(&pMRPackage->state_b.flags, 8);
            if (pMRPackage->state_b.flags & 2) {
                tiWordClearBits(&pMRPackage->state_b.flags, 2);
            }
        } else if (pMRPackage->state_b.position < (float) pMRPackage->state_b.rangeMin) {
            tiWordSetBits(&pMRPackage->state_b.flags, 8);
            if (pMRPackage->state_b.flags & 2) {
                tiWordClearBits(&pMRPackage->state_b.flags, 2);
            }
        } else {
            tiWordClearBits(&pMRPackage->state_b.flags, 8);
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
void tiMRPackageSelectEntryA(TiMRPackage *pMRPackage, unsigned int index) {
    OffsetTable *table;
    int entryOffset;

    table = pMRPackage->entryTableA;
    if (table == 0) {
        return;
    }
    if (index < table->count) {
        entryOffset = index[table->entryOffsets];
        tiMRPackageSetAnimData(pMRPackage, (TiOffsetEntry *) ((unsigned char *) table + entryOffset));
        tiWordSetBits(&pMRPackage->state_a.flags, TI_MR_STATE_FLAG_ENTRY_BOUND);
    }
}

/* Stores table A (the argument), then binds entry `index` of it to state A as tiMRPackageSelectEntryA. */
void tiMRPackageSetTableAndEntryA(TiMRPackage *pMRPackage, OffsetTable *table, unsigned int index) {
    int entryOffset;

    if (table == 0) {
        return;
    }
    if (index < table->count) {
        entryOffset = index[table->entryOffsets];
        tiMRPackageSetAnimTable(pMRPackage, table);
        tiMRPackageSetAnimData(pMRPackage, (TiOffsetEntry *) ((unsigned char *) table + entryOffset));
        tiWordSetBits(&pMRPackage->state_a.flags, TI_MR_STATE_FLAG_ENTRY_BOUND);
    }
}

/* Binds entry `index` of table B to state B and sets flag 0x40 of state B. */
void tiMRPackageSelectEntryB(TiMRPackage *pMRPackage, unsigned int index) {
    OffsetTable *table;
    int entryOffset;

    table = pMRPackage->entryTableB;
    if (table == 0) {
        return;
    }
    if (index < table->count) {
        entryOffset = index[table->entryOffsets];
        tiMRPackageSetFaceAnimData(pMRPackage, (TiOffsetEntry *) ((unsigned char *) table + entryOffset));
        tiWordSetBits(&pMRPackage->state_b.flags, TI_MR_STATE_FLAG_ENTRY_BOUND);
    }
}

#include "quadword.h"

/*
 * Binds each dynamics node of the hierarchy to its solver body: the body's position takes the node's vector
 * (+0x90), the body gets the node pointer and gust 1.0, and the walk steps over the node's collision entries.
 */
void func_00299698(TiMRPackage *pMRPackage, TiDynamicsOut *nodes) {
    TiHrcData *hrc;
    TiHrcDynamics *dynamics;
    TiDynamicsOut *node;
    Vector4 *vectorBlock;
    unsigned int nodeIndex;

    pMRPackage->dynMat = nodes;
    if (nodes != 0) {
        hrc = pMRPackage->hrcData;
        nodeIndex = 0;
        dynamics = (TiHrcDynamics *) ((unsigned char *) hrc + hrc->dynamicsOffset);
        if (hrc->dynamicsCount != 0) {
            node = nodes;
            do {
                QuadwordCopy(&node->position, &dynamics->unmodeled_90);
                nodeIndex++;
                node->node = dynamics;
                node->gust = 1.0f;
                node++;
                vectorBlock = (Vector4 *) (dynamics + 1);
                dynamics = (TiHrcDynamics *) (vectorBlock
                    + (dynamics->sphereCount + dynamics->matrixCount + dynamics->capsuleCount));
            } while (nodeIndex < pMRPackage->hrcData->dynamicsCount);
        }
    }
}

/* Converts the source value (range[0]) of a motion state to the float at +8 of the state. */
void XenoActorConvertSourceRecord(XenoMotionState *record) {
    record->frame = (f32) record->range[0];
}

INCLUDE_ASM("asm/main/nonmatchings/TiMRPackage", func_00299730);

/* Four-character tag of an Xst file record, read as one word and as its bytes. */
typedef union TiFourCC {
    unsigned int word;
    unsigned char bytes[4];
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
    int rangeMin;                   /* +4 */
    int rangeMax;                   /* +8 */
    int size;                       /* +0xC: the next cluster is size + 0xB0 bytes on */
    unsigned char unmodeled_10[8];
    int flag;                       /* +0x18: must be 1 */
    int elementOffset;              /* +0x1C: offset from the header to the element, 0 when none */
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

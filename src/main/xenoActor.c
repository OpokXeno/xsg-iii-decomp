#include "common.h"
#include "shared.h"
#include "quadword.h"

/* Row-major 4x4 float matrix. */
struct TiDrawJointState;
extern void tiDrawJointDrawLists(struct TiDrawJointState *state, float (*pGlobalMat)[4][4], float (*current)[4][4]);

#include "main/TiDynamicsColl.h"
#include "main/TiMRPackage.h"
#include "main/tu141.h"
#include "main/tu352.h"
#include "main/xenoActor.h"
#include "main/xgl_2.h"
#include "main/tu045.h"
#include "vu0_macro.h"
XglCamera *xglStudioSelectGetActiveCamera(int index);
float xglPointLength(Vector4 *a, Vector4 *b);
extern float XenoActorFadeDistanceScale;

typedef struct TiSolverState TiDynamicsOut;

void tiMRPackageInit(TiMRPackage *pMRPackage);
void tiMRPackageSetAnimData(TiMRPackage *pMRPackage, TiOffsetEntry *entry);
/* A vector kept as a plain float[4] (like sceVu0FVECTOR); its w is element 3. Vector4 here changes the
 * address arithmetic of the w stores, so the casts to Vector4 * at the calls stay. */
void xglLightSetDefault(XglLightSet *lightSet);
struct RgHeap;
struct RgHeap *InstanceOfRgHeapById(unsigned int heapId);
void InitRgHeap(struct RgHeap *pHeap, void *pMemory, int size);
void RgHeapFree(void *pPtr, const char *pSourceFile, int line);
void assertFailed(const char *message, const char *file, int line);
void UnduParamInit(MapCollisionObject *object);
struct XenoActor;

/* Indices of XenoActor.rsrc: the model pack (.pxy, xenoActor.c reads it as PxyModelHeader *), the texture (.txy), the
 * hierarchy (.xhr, XenoActorAllocMatrices builds the matrices from it), the motion tables (.xap: slot n is
 * XAct_Rsrc_XAP + n, slots 0..5 = indices 4..9) and a few entries xenoActor.c reads as pointers. The names of the
 * first seven are those of the original's asserts; no assert names the rest. */
enum XActRsrc {
    XAct_Rsrc_MDL,
    XAct_Rsrc_TEX,
    XAct_Rsrc_XHR,
    XAct_Rsrc_3,                            /* entry passed to tiMRPackageSetAnimData */
    XAct_Rsrc_XAP,                          /* OffsetTable * of slot 0 */
    XAct_Rsrc_5,
    XAct_Rsrc_XAP_Ext2,
    XAct_Rsrc_7,
    XAct_Rsrc_SlotEntry,                    /* slot 4, read as an int entry (the motion set's second entry) */
    XAct_Rsrc_SlotTableB,                   /* slot 5, read as another table */
    XAct_Rsrc_PAM,                          /* non-zero when a PAM pose is active */
    XAct_Rsrc_EntryTableC,                  /* tiMRPackage range entry C */
    XAct_Rsrc_Count
};

void XenoActorInit(struct XenoActor *actor);
void XenoActorInitHeap(void *memory, int size);
int XenoActorGetPoolCount(void);
void XenoActorClearHoldFlag(void);
void XenoActorRelease(XenoActor *actor);
void XenoActorReleaseAll(void);
void XenoActorSetHoldFlag(void);

/* Package B of an actor (shadowB, +0x560) as the TiMRPackage of main/TiMRPackage.h. Its state A (+0x570, shadowB2) and
 * state B (+0x5C0) are the TiMRState blocks at +0x10 and +0x60 of the package. */
#define XenoActorPackageB(a) ((TiMRPackage *)&(a)->shadowB)
#define XenoActorStateA(a) (&XenoActorPackageB(a)->bodyAnimBlend)
#define XenoActorStateB(a) (&XenoActorPackageB(a)->faceAnimBlend)
/* Slot index halfword at +0x4C8 (unmodeled_4c8): stored by the attach calls, read by XenoActorPushMatrix. */
#define XenoActorSlotIndex(a) (*(short *)(a)->attachSlotIndex)
/* The model pack of an actor (rsrc slot XAct_Rsrc_MDL). */
#define XenoActorModelPack(a) ((PxyModelHeader *)(a)->rsrc[XAct_Rsrc_MDL])
/* The offset table of rsrc slot `slot` (slots 4 and up hold the motion tables). */
#define XenoActorSlotTable(a, slot) (((OffsetTable **)&(a)->rsrc[4])[slot])
/* The local matrix at +0xB0 as an XenoMatrix. */
#define XenoActorLocalMatrix(a) ((XenoMatrix *)&(a)->localMatrix)
/* The focus vector at +0x590 (colorIndex block) read as a Vector4. */
#define XenoActorFocusVector(a) ((Vector4 *)(a)->colorIndex)
/* colorIndex value meaning "no color"; XenoActorInit fills the 0x10-byte block at colorIndex with it. */
#define XENO_COLOR_INDEX_NONE 0xFF
/* The "pxy" tag of a model pack (PxyModelHeader.magic), read as a little-endian 24-bit word. */
#define XENO_PXY_TAG 0x797870

extern int XenoActorPoolCount;
extern XenoActor **XenoActorActiveTable;
extern const char xenoActorAssertFile[16];
extern const char D_0053F9A8[];
extern const char D_0053F910[];
extern const char xenoActorHrcItemLabel[];
extern unsigned char xenoActorAssertExpr[];
extern int XenoActorHoldFlag;
extern unsigned char xenoActorAssertExpr[8];
void *memset(void *dst, int value, unsigned int size);
void XenoActorInit(XenoActor *actor);
extern const char xenoActorAssertExprRelease[16];

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028D350);

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028D378);

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028D3B0);

/* Pool record stride of X3 (0x6D0 bytes; the record size is the X3 layout, not the X2 pad above). */
/* Header of resource slot 0: only the offset of its extra block is read here. */
typedef struct XenoRsrcHeader {
    unsigned char unk_00[0xD0];
    int subPackOffset;
} XenoRsrcHeader;
#define XENO_ACTOR_POOL_STRIDE 0x6D0
/* Data-driven byte offset: the offset is read from a resource header at run time. */
#define BYTE_OFFSET(base, off) ((unsigned char *) (base) + (off))

extern XenoActor *XenoActorPool;
extern unsigned char XenoActorHeapMemory[];
extern unsigned char XenoActorDefaultPool[];
void tiGlobalCollClearCounters(void);
void func_0028F2C8(float value);
void tiMRStateSetValue(unsigned char *state, float value);
void tiMRPackageSetXstHeader(TiMRPackage *pMRPackage, int value);
/* Builds the actor transform on the matrix stack from its parent (or from itself), saves it in localMatrix and concatenates * the override matrix; a slot of 2 or more also multiplies by the parent's joint matrix and the pack's joint matrix. */
void XenoActorPushMatrix(XenoActor *actor);
void xglCurMatrixSet(void *matrix);
void xglCurApplyVector(Vector4 *in, Vector4 *out);
void xglCurSetTranslate(Vector4 *point);
void xglCurMatrixGet(void *matrix);
void tiDrawJointDrawAxes(void *matrix, float scale);
int func_0030E68C(unsigned char *entry, const char *name);
void func_001975C8(void *block);
void func_002A70E8(void *block);
void tiWordClearBits(void *block, int mask);
void tiWordSetBits(void *block, int mask);
void XenoActorInit(XenoActor *actor);

/* Pool init (X2 homolog name XenoActorPoolInit, lead only): sets the pool base and count, builds the active table after the pool and inits every record. */
unsigned char *XenoActorPoolInit(void *actorPoolMemory, int actorPoolCount) {
    void *actorPoolEnd;
    int i;
    XenoActor *pooledActor;
    XenoActor **activeSlot;

    actorPoolEnd = actorPoolMemory;
    XenoActorPoolCount = actorPoolCount;
    XenoActorPool = actorPoolEnd;
    actorPoolEnd += actorPoolCount * XENO_ACTOR_POOL_STRIDE;
    XenoActorActiveTable = actorPoolEnd;
    actorPoolEnd += actorPoolCount * 4;
    pooledActor = actorPoolMemory;
    for (i = 0; i < XenoActorPoolCount; i++) {
        XenoActorInit(pooledActor);
        pooledActor++;
    }
    if (XenoActorPoolCount > 0) {
        activeSlot = XenoActorActiveTable;
        i = XenoActorPoolCount;
        do {
            *activeSlot++ = 0;
        } while (--i);
    }
    XenoActorInitHeap(XenoActorHeapMemory, 0x40000);
    tiGlobalCollClearCounters();
    XenoActorClearHoldFlag();
    return actorPoolEnd;
}

/* Default pool init: runs the setup, sets the scale to 1.0 and initializes the pool at XenoActorDefaultPool with 128 records. */
unsigned char *XenoActorPoolInitDefault(void) {
    tiGlobalCollClearCounters();
    func_0028F2C8(1.0f);
    return XenoActorPoolInit(XenoActorDefaultPool, 128);
}

/* XenoActorInitHeap: initializes RG heap 5, the heap the actor matrices are allocated from, over the given memory block. */
void XenoActorInitHeap(void *memory, int heapSize) {
    InitRgHeap(InstanceOfRgHeapById(5), memory, heapSize);
}

/* Resets an actor record to its defaults: zeroes it, sets the unit scale, alpha and fade factors, the 0xFF block fills, the motion package, the light set and the collision block. */
void XenoActorInit(XenoActor *actor) {
    TiMRPackage *motionPackage = XenoActorPackageB(actor);

    if (actor == 0) {
        return;
    }
    memset(actor, 0, sizeof(XenoActor));
    actor->position.w = 1.0f;
    actor->unmodeled_0a0 = 0.2f;
    actor->vector60.w = 1.0f;
    actor->alpha = 1.0f;
    actor->unmodeled_098 = 1.0f;
    actor->scale.w = 1.0f;
    actor->scale.z = 1.0f;
    actor->scale.y = 1.0f;
    actor->scale.x = 1.0f;
    actor->unmodeled_0ac = 1.0f;
    actor->alphaScale = 1.0f;
    actor->unmodeled_13c = 1.0f;
    actor->sweepSpeed = 1.0f;
    actor->clipPlaneScale = 1.0f;
    actor->shadowValue3 = 1.0f;
    actor->unmodeled_12c = 1.0f;
    actor->unmodeled_124 = 0.5f;
    actor->unmodeled_128 = -1.0f;
    actor->unmodeled_09c = 0;
    actor->requestedMotionSlot = -1;
    XenoActorSlotIndex(actor) = -1;
    actor->unmodeled_0fc = 0;
    actor->shadowValue = 0;
    actor->sweepPhase = 0;
    actor->shadowCount = 0;
    actor->unmodeled_11c = 0;
    actor->unmodeled_120 = 0;
    actor->unmodeled_0f8 = 0;
    actor->unmodeled_130 = 0;
    actor->fadeRadius = 0;
    memset(&actor->sourceBlock, 255, 16);
    tiMRPackageInit(motionPackage);
    tiMRPackageSetAnimData(motionPackage, 0);
    tiMRPackageSetXstHeader(motionPackage, 0);
    memset(actor->unmodeled_330, 255, 32);
    memset(actor->unmodeled_350, 255, 256);
    actor->unmodeled_450 = 0;
    actor->unmodeled_454 = 0;
    func_001975C8(&actor->light);
    func_002A70E8(actor->collision);
    actor->unmodeled_004 = 0;
    actor->unmodeled_010 = 0.3f;
}

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028D670);

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028D760);

/* XenoActorAttachChild (X2 homolog name, lead only): links the child into the first free parent slot and sets child->parent.
   The X3 record keeps its child slots at 0x4CC and its parent at 0x4C4; its live test is attr & 1. */
int XenoActorAttachChild(XenoActor *parent, XenoActor *child) {
    int i;

    if (parent != 0 && child != 0 && (parent->attr & 1) && (child->attr & 1) && child->parent == 0) {
        for (i = 0; i < 8; i++) {
            if (parent->children[i] == 0) {
                parent->children[i] = child;
                child->parent = parent;
                return 1;
            }
        }
    }
    return 0;
}

/* XenoActorAppendChildOnJoint (X2 homolog name, lead only): as XenoActorAttachChild, and the child's overrideMatrix is the parent's
   joint matrix (joint * 64 past jointMatrices), or 0 for a negative joint. */
int XenoActorAppendChildOnJoint(XenoActor *parent, XenoActor *child, int joint) {
    int i;

    if (parent != 0 && child != 0 && (parent->attr & 1) && (child->attr & 1) && child->parent == 0) {
        for (i = 0; i < 8; i++) {
            if (parent->children[i] == 0) {
                if (joint >= 0) {
                    child->overrideMatrix = parent->jointMatrices + joint;
                } else {
                    child->overrideMatrix = 0;
                }
                parent->children[i] = child;
                child->parent = parent;
                return 1;
            }
        }
    }
    return 0;
}

#include "main/tu045.h"

/* One entry of the static joint table of a hierarchy (0x40 bytes); only the joint id at +0x38 is read here. */
typedef struct TiHrcStatic {
    unsigned char unmodeled_00[0x38];
    int id;
    unsigned char unmodeled_3C[0x40 - 0x3C];
} TiHrcStatic;

/* Attaches the child to the joint with the given id: the joint is looked up in the static table of the hierarchy (an
 * index into the joint records), or by number when the hierarchy version is below 2.4. 0 when the joint is not found. */
int XenoActorAttachChildToJoint(XenoActor *actor, XenoActor *child, int jointId) {
    TiHrcData *resource;
    TiHrcStatic *joints;
    int found;
    u32 i;

    resource = (TiHrcData *)actor->rsrc[XAct_Rsrc_XHR];
    if (resource == 0) {
        return 0;
    }
    if (resource->version < 2.4f) {
        XenoHrc *hrc = actor->pHrcData;

        if (jointId >= hrc->jointCount) {
            return 0;
        }
        return XenoActorAppendChildOnJoint(actor, child, hrc->leadCountA + hrc->leadCountB + hrc->focus + hrc->jointRecordCount + jointId);
    }
    joints = (TiHrcStatic *)BYTE_OFFSET(resource, resource->staticOffset);
    found = -1;
    for (i = 0; i < resource->staticCount; i++) {
        if (joints[i].id == jointId) {
            found = i;
            break;
        }
    }
    if (found < 0) {
        return 0;
    }
    return XenoActorAppendChildOnJoint(actor, child, actor->pHrcData->leadCountA + actor->pHrcData->leadCountB + actor->pHrcData->focus + actor->pHrcData->jointRecordCount + found);
}

/* Attaches the child to the parent at a slot index: stores the index in the child, attaches it and returns 1; 0 on bad arguments. */
int XenoActorAttachAtSlot(XenoActor *parent, XenoActor *child, int attachSlot) {
    if (parent != 0 && child != 0 && attachSlot >= 0) {
        XenoActorSlotIndex(child) = attachSlot;
        XenoActorAttachChild(parent, child);
        return 1;
    }
    return 0;
}

int XenoActorReleaseChild(XenoActor *parent, XenoActor *child) {
    XenoActor **childSlot;
    int n;

    if (parent != 0 && child != 0 && (parent->attr & 1) && (child->attr & 1) && child->parent != 0) {
        childSlot = parent->children;
        for (n = 0; n < 8; n++) {
            if (*childSlot == child) {
                *childSlot = 0;
                child->parent = 0;
                XenoActorSlotIndex(child) = -1;
                child->overrideMatrix = 0;
                return 1;
            }
            childSlot++;
        }
    }
    return 0;
}

/* XenoActorSetFlagAll (X2 homolog name, lead only): ORs a flag mask into the flag word of every live record. */
void XenoActorSetFlagAll(int drawFlagMask) {
    unsigned int i;
    unsigned int loopLimit;
    unsigned int actorPoolCount;
    XenoActor *pooledActor;

    loopLimit = XenoActorPoolCount;
    i = 0;
    if (loopLimit != 0) {
        actorPoolCount = loopLimit;
        pooledActor = XenoActorPool;
        do {
            i++;
            if (pooledActor->attr & 1) {
                pooledActor->drawFlags |= drawFlagMask;
                loopLimit = actorPoolCount;
            }
            pooledActor++;
        } while (i < loopLimit);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028DE30);

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028DE40);

/* Sets the value of the state at +0x570 (shadowB2, X3 offset 1392) through tiMRStateSetValue. */
void XenoActorSetChannelValue(XenoActor *actor, float channelValue) {
    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 501);
    }
    tiMRStateSetValue(actor->shadowB2, channelValue);
}

/* Sets the value of the state at +0x5C0 (X3 offset 1472) through tiMRStateSetValue; asserts the actor is set. */
void XenoActorSetChannelValueB(XenoActor *actor, float channelValue) {
    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 511);
    }
    tiMRStateSetValue(actor->unmodeled_5ac + 0x14, channelValue);
}

/* Sets flag bits 4 and 2 of the state at +0x570. */
void XenoActorEnableChannel(XenoActor *actor) {
    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 523);
    }
    tiWordSetBits(actor->shadowB2, 4);
    tiWordSetBits(actor->shadowB2, 2);
}

/* Clears flag bit 4 and sets bit 2 of the state at +0x570. */
void XenoActorDisableChannel(XenoActor *actor) {
    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 536);
    }
    tiWordClearBits(actor->shadowB2, 4);
    tiWordSetBits(actor->shadowB2, 2);
}

/* Sets flag bits 4 and 2 of the state at +0x5C0. */
void XenoActorEnableChannelB(XenoActor *actor) {
    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 547);
    }
    tiWordSetBits(actor->unmodeled_5ac + 0x14, 4);
    tiWordSetBits(actor->unmodeled_5ac + 0x14, 2);
}

/* Clears flag bit 4 and sets bit 2 of the state at +0x5C0. */
void XenoActorDisableChannelB(XenoActor *actor) {
    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 558);
    }
    tiWordClearBits(actor->unmodeled_5ac + 0x14, 4);
    tiWordSetBits(actor->unmodeled_5ac + 0x14, 2);
}

void tiMRStateSetRange(void *state, int start, int end);
void tiMRPackageSelectEntryB(TiMRPackage *pMRPackage, int index);
void tiDrawCollisionDrawObjectRing(float weight, void *payload, const char *text);
void func_0028F8C8(XenoActor *actor);
void func_0028FBA0(XenoActor *actor);
void tiMRStateSetPosition(unsigned char *state, float value);
void tiMtnBlendSwapLocal(TiMRPackage *pMRPackage, unsigned char *state);
void tiMtnBlendSwapGlobal(TiMRPackage *pMRPackage, unsigned char *state);

void XenoActorResetChannelFlags(XenoActor *actor) {
    tiWordClearBits(actor->shadowB2, 2);
    tiWordClearBits(actor->shadowB2, 64);
    tiWordClearBits(actor->unmodeled_5ac + 0x14, 2);
    tiWordClearBits(actor->unmodeled_5ac + 0x14, 64);
}

/* Clears the package at +0x560 (X3 offset 1376): its anim table and anim data, then tiMRPackageSetXstHeader. */
void XenoActorClearChannel(XenoActor *actor) {
    tiMRPackageSetAnimTable(XenoActorPackageB(actor), 0);
    tiMRPackageSetAnimData(XenoActorPackageB(actor), 0);
    tiMRPackageSetXstHeader(XenoActorPackageB(actor), 0);
}

/* Copies the position and rotation vectors and wraps the three rotation angles into [0, 2*pi) (X2 name XenoActorWrapRotation, lead only). */
void XenoActorWrapRotation(XenoActor *actor);
void tiMRPackageSetAnimTableAndEntry(TiMRPackage *pMRPackage, int table, short slot);

void XenoActorSetMode(XenoActor *actor, int motionSlot) {
    TiOffsetEntry *motionKey;

    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 589);
    }
    actor->requestedMotionSlot = -1;
    actor->currentMotionSlot = motionSlot;
    actor->currentMotionTable = 0;
    tiMRPackageSetAnimTableAndEntry(XenoActorPackageB(actor), actor->rsrc[4], motionSlot);
    actor->resetFlags &= ~0x40;
    tiWordClearBits(actor->shadowB2, 8);
    tiWordClearBits(actor->unmodeled_5ac + 0x14, 8);
    motionKey = actor->motionKeys[0];
    if (motionKey != 0) {
        if (XenoActorStateA(actor)->speed < 0.0f) {
            actor->frame = motionKey->rangeMax;
        } else {
            actor->frame = motionKey->rangeMin;
        }
        tiMRStateSetPosition(actor->shadowB2, actor->frame);
        if (!(actor->resetFlags & 0x10)) {
            if (XenoActorStateA(actor)->motionFlags & 2) {
                tiMtnBlendSwapLocal(XenoActorPackageB(actor), actor->shadowB2);
                tiMtnBlendSwapGlobal(XenoActorPackageB(actor), actor->shadowB2);
                actor->resetFlags |= 0x10;
            }
        }
    }
}

void XenoActorCheckMode(XenoActor *actor, int motionSlot) {
    if (actor->currentMotionSlot != motionSlot) {
        XenoActorSetMode(actor, motionSlot);
    }
}

/* Sets the play range of state A (shadowB2) and clamps the play position into [start, end]: outside it, a backward motion (speed < 0) restarts from end and a forward one from start. */
void XenoActorSetPlayRange(XenoActor *actor, int playStart, int playEnd) {
    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 647);
    }
    actor->resetFlags |= 0x40;
    if (actor->frame < playStart || playEnd < actor->frame) {
        if (XenoActorStateA(actor)->speed < 0.0f) {
            actor->frame = playEnd;
        } else {
            actor->frame = playStart;
        }
    }
    tiMRStateSetRange(actor->shadowB2, playStart, playEnd);
}

void XenoActorConvertSourceRecord(XenoMotionState *record);

/* Sets the frame to the first range bound of state A and converts state A's source record. */
void XenoActorApplySourceValue(XenoActor *actor) {
    actor->frame = XenoActorStateA(actor)->rangeMin;
    XenoActorConvertSourceRecord((XenoMotionState *)XenoActorStateA(actor));
}

void XenoActorSetModeInSlot(XenoActor *actor, u32 motionTableIndex, int motionSlot) {
    TiOffsetEntry *motionKey;

    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 677);
    }
    if (motionTableIndex >= 4) {
        return;
    }
    actor->requestedMotionSlot = -1;
    actor->currentMotionSlot = motionSlot;
    actor->currentMotionTable = motionTableIndex;
    tiMRPackageSetAnimTableAndEntry(XenoActorPackageB(actor), actor->rsrc[motionTableIndex + 4], motionSlot);
    actor->resetFlags &= ~0x40;
    tiWordClearBits(actor->shadowB2, 8);
    tiWordClearBits(actor->unmodeled_5ac + 0x14, 8);
    motionKey = actor->motionKeys[0];
    if (motionKey != 0) {
        if (XenoActorStateA(actor)->speed < 0.0f) {
            actor->frame = motionKey->rangeMax;
        } else {
            actor->frame = motionKey->rangeMin;
        }
        tiMRStateSetPosition(actor->shadowB2, actor->frame);
        if (!(actor->resetFlags & 0x10)) {
            if (XenoActorStateA(actor)->motionFlags & 2) {
                tiMtnBlendSwapLocal(XenoActorPackageB(actor), actor->shadowB2);
                tiMtnBlendSwapGlobal(XenoActorPackageB(actor), actor->shadowB2);
                actor->resetFlags |= 0x10;
            }
        }
    }
}

/* Selects entry `selector` of package B (shadowB) and stores it as a halfword at +0x500, then copies the entry's range minimum into the cached float at +0x4FC. When state B (+0x5C0) has bit 2 and the blend is not swapped yet, the blend is swapped in and 0x10 is set. */
void XenoActorSetSelector(XenoActor *actor, int selector) {
    TiOffsetEntry *selectorEntry;
    float selectorRangeMin;

    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 725);
    }
    actor->selectedEntry = selector;
    tiMRPackageSelectEntryB(XenoActorPackageB(actor), (short)selector);
    selectorEntry = actor->motionKeys[2];
    if (selectorEntry != 0) {
        selectorRangeMin = selectorEntry->rangeMin;
        actor->selectorRangeMin = selectorRangeMin;
        tiMRStateSetPosition(actor->unmodeled_5ac + 0x14, selectorRangeMin);
        if (!(actor->resetFlags & 0x10) && (XenoActorStateB(actor)->motionFlags & 2)) {
            tiMtnBlendSwapLocal(XenoActorPackageB(actor), actor->unmodeled_5ac + 0x14);
            tiMtnBlendSwapGlobal(XenoActorPackageB(actor), actor->unmodeled_5ac + 0x14);
            actor->resetFlags |= 0x10;
        }
    }
}

/* Requests a mode for the actor: stores the mode as a halfword at +0x4F8 (X3 offset 1272). */
void XenoActorRequestMode(XenoActor *actor, int requestedMode) {
    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 754);
    }
    actor->requestedMotionSlot = requestedMode;
}

/* Returns 1 when state A (shadowB2) has bit 8 set, else 0. */
int XenoActorIsDone(XenoActor *actor) {
    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 765);
    }
    if ((XenoActorStateA(actor)->motionFlags & 8) != 0) {
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028E5E8);

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028E788);

void tiMRPackageInit(TiMRPackage *pMRPackage);
extern const float D_0054FD90;
extern const float D_0054FD94;
void func_00299350(TiMRPackage *pMRPackage, float (*matrix)[4]);
void func_0029F548(int value, TiMRPackage *pMRPackage, int other);

void XenoActorApplyRequestedMode(XenoActor *actor) {
    XenoRsrcHeader *resourceHeader;

    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 910);
    }
    if (actor->requestedMotionSlot >= 0) {
        if (XenoActorIsDone(actor)) {
            XenoActorEnableChannel(actor);
            XenoActorSetMode(actor, actor->requestedMotionSlot);
            actor->requestedMotionSlot = -1;
        }
    }
    if (actor->resetFlags & 0x4000) {
        tiWordSetBits(actor->shadowB2, 1024);
    } else {
        tiWordClearBits(actor->shadowB2, 1024);
    }
    if (actor->pHrcData != 0) {
        actor->resetFlags &= ~0x80;
        actor->currentMatrix = actor->localMatrix;
        func_00299350(XenoActorPackageB(actor), actor->localMatrix);
        if ((XenoActorStateA(actor)->motionFlags & 0x20) || (XenoActorStateB(actor)->motionFlags & 0x20)) {
            actor->resetFlags |= 0x80;
        }
    }
    if (actor->unmodeled_658 != 0) {
        resourceHeader = (XenoRsrcHeader *)actor->rsrc[0];
        if (resourceHeader != 0) {
            func_0029F548(actor->unmodeled_54c, XenoActorPackageB(actor),
                          *(int *)BYTE_OFFSET(resourceHeader, resourceHeader->subPackOffset + 96));
        }
    }
    actor->resetFlags &= ~0x10;
    tiWordClearBits(actor->shadowB2, 16);
}

int XenoActorSlotRangeValid(XenoActor *actor, int motionSlot, unsigned int motionRangeIndex) {
    OffsetTable *motionTable;
    TiOffsetEntry *motionRangeEntry;

    motionTable = XenoActorSlotTable(actor, motionSlot);
    if (motionTable == 0) {
        return 0;
    }
    if (motionRangeIndex >= motionTable->count) {
        return 0;
    }
    motionRangeEntry = (TiOffsetEntry *)BYTE_OFFSET(motionTable, *(motionTable->entryOffsets + motionRangeIndex));
    if (motionRangeEntry->rangeMin <= motionRangeEntry->rangeMax) {
        return 1;
    }
    return 0;
}

/* XenoActorSetHoldFlag: sets the global update hold XenoActorHoldFlag; XenoActorDoit skips every tree update while it is set. */
void XenoActorSetHoldFlag(void) {
    XenoActorHoldFlag = 1;
}

/* XenoActorClearHoldFlag: clears the global update hold XenoActorHoldFlag; XenoActorPoolInit clears it when the pool is built. */
void XenoActorClearHoldFlag(void) {
    XenoActorHoldFlag = 0;
}

typedef float XglFVector[4];

typedef MapFixupEntry MapRecordGroup;

/* Flag word bit of XenoActor.motionFlags (X3 offset 0x4EC): set by XenoActorWrapRotation, the next update only snapshots the joint rows. */
#define XENO_FLAG_SKIP_DELTA 0x800

void RgHeapFree(void *pPtr, const char *pSourceFile, int line);

void XenoActorRelease(XenoActor *actor) {
    int i;
    int j;
    int found;
    XenoActor *parent;

    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 1016);
    }
    if (actor->pLocalMat != 0) {
        RgHeapFree(actor->pLocalMat, D_0053F910, 1018);
        actor->pLocalMat = 0;
    }
    if (actor->pGlobalMat != 0) {
        RgHeapFree(actor->pGlobalMat, D_0053F910, 1019);
        actor->pGlobalMat = 0;
    }
    if (actor->pBlendMat != 0) {
        RgHeapFree(actor->pBlendMat, D_0053F910, 1020);
        actor->pBlendMat = 0;
    }
    if (actor->pPartsMat != 0) {
        RgHeapFree(actor->pPartsMat, D_0053F910, 1021);
        actor->pPartsMat = 0;
    }
    if (actor->pDynamics != 0) {
        RgHeapFree(actor->pDynamics, D_0053F910, 1022);
        actor->pDynamics = 0;
    }
    found = 0;
    for (i = 0; i < XenoActorPoolCount; i++) {
        if (XenoActorActiveTable[i] == actor) {
            XenoActorActiveTable[i] = 0;
            found = 1;
            break;
        }
    }
    if (!found) {
        assertFailed(&D_0053F9A8[0x30], D_0053F910, 1050);
    }
    parent = actor->parent;
    if (parent != 0) {
        for (j = 0; j < 8; j++) {
            if (parent->children[j] == actor) {
                parent->children[j] = 0;
                break;
            }
        }
    }
    for (j = 0; j < 8; j++) {
        if (actor->children[j] != 0) {
            actor->children[j]->parent = 0;
        }
    }
    XenoActorInit(actor);
}

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028ED20);

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028F1A8);

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028F2C8);

/* Fades the actor with its distance to the active camera: alphaScale ramps from 0 at the fade radius to 1 at 1.6 times it
 * (drawFlags bit 0x8000000), or from 1 down to 0 past 25 * XenoActorFadeDistanceScale (bit 0x8000). */
void XenoActorUpdateDistanceFade(XenoActor *actor)
{
    XglCamera *camera;
    PxyModelHeader *pack;
    DoublewordAlignedVector4 fadeTarget;
    DoublewordVector4 cameraPosition;
    float radius;
    float distance;
    float fadeEnd;
    float alpha;

    camera = xglStudioSelectGetActiveCamera(0);
    if (camera == 0 || actor->parent != 0) {
        return;
    }
    if (actor->drawFlags & 0x8000000) {
        actor->alphaScale = 1.0f;
        pack = XenoActorModelPack(actor);
        if (pack != 0 && pack->magic[0] == 'p' && pack->magic[1] == 'x' && pack->magic[2] == 'y' && (pack->flags & 0x81) == 0) {
            radius = pack->sphere.w;
            fadeTarget = actor->position;
            cameraPosition = *(DoublewordVector4 *)&camera->travel.position;
            if (actor->fadeRadius != 0.0f) {
                radius = actor->fadeRadius;
            }
            Vu0AddVectorXYZ(&fadeTarget, &fadeTarget, &actor->vector70);
            fadeTarget.w = 1.0f;
            distance = xglPointLength(&cameraPosition.vector, &fadeTarget);
            if (distance < radius) {
                actor->alphaScale = 0.0f;
            } else {
                fadeEnd = radius * 1.6f;
                if (distance < fadeEnd) {
                    actor->alphaScale = 1.0f - (fadeEnd - distance) / (fadeEnd - radius);
                }
            }
            if (actor->alphaScale < 0.0f) {
                actor->alphaScale = 0.0f;
            }
        }
    }
    if (actor->drawFlags & 0x8000) {
        float alphaDistance;

        alpha = 1.0f;
        pack = XenoActorModelPack(actor);
        if (pack == 0 || pack->magic[0] != 'p' || pack->magic[1] != 'x' || pack->magic[2] != 'y' || (pack->flags & 0x81) != 0) {
            actor->alphaScale = alpha;
            return;
        }
        radius = XenoActorFadeDistanceScale * 25.0f;
        cameraPosition.vector = actor->position;
        cameraPosition.vector.w = 1.0f;
        alphaDistance = xglPointLength(&camera->travel.position, &cameraPosition.vector);
        if (radius < alphaDistance) {
            alpha = 1.0f - (alphaDistance - radius) / radius;
            if (alpha < 0.0f) {
                alpha = 0.0f;
            }
        }
        actor->alphaScale = alpha;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028F510);

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028F8B8);

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028F8C0);

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028F8C8);

void XenoActorClearResetBits(XenoActor *actor) {
    actor->resetFlags &= 0xFFFDFFFF;
    actor->resetFlags &= 0xFFFBFFFF;
}

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028FA28);

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028FBA0);

/* One 0x30-byte item of the HRC table: a radius word and the ring data drawn with it. */
typedef struct XenoHrcItem {
    unsigned char unk_00[4];
    float radius;
    unsigned char unk_08[0x18];
    unsigned char ring[0x10];
} XenoHrcItem;

/* Draws each item of the HRC table at actor+0x630 (0x30 bytes each, from its dataOffset) at the item's weight through tiDrawCollisionDrawObjectRing. */
void XenoActorRefreshItems(XenoActor *actor) {
    XenoHrc *hrcData;
    unsigned int hrcItemIndex;
    XenoHrcItem *hrcItem;

    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 0x77D);
    }
    hrcData = actor->pHrcData;
    hrcItemIndex = 0;
    if (hrcData != 0) {
        hrcItem = (XenoHrcItem *)BYTE_OFFSET(hrcData, hrcData->dataOffset);
        if (hrcData->count != 0) {
            do {
                tiDrawCollisionDrawObjectRing(hrcItem->radius, hrcItem->ring, xenoActorHrcItemLabel);
                hrcItemIndex++;
                hrcItem++;
            } while (hrcItemIndex < actor->pHrcData->count);
        }
    }
}

int tiMRPackageGetNodeMatrix(Vector4 *block, void *arg);
void xglMatrixMul(XenoMatrix *dst, XenoMatrix *a, XenoMatrix *b);

int XenoActorMulMatrix(XenoActor *actor, void *nodeKey, XenoMatrix *matrix) {
    if (tiMRPackageGetNodeMatrix(&actor->shadowB, nodeKey) == 0) {
        return 0;
    }
    if (actor->overrideMatrix != 0) {
        xglMatrixMul(matrix, matrix, actor->overrideMatrix);
    } else {
        xglMatrixMul(matrix, matrix, XenoActorLocalMatrix(actor));
    }
    return 1;
}

int tiMRPackageGetNodePoint(Vector4 *block, void *arg);
void xglVectorMulMat(Vector4 *dst, XenoMatrix *matrix, Vector4 *src);

int XenoActorMulVector(XenoActor *actor, void *nodeKey, Vector4 *vector) {
    if (tiMRPackageGetNodePoint(&actor->shadowB, nodeKey) == 0) {
        return 0;
    }
    if (actor->overrideMatrix != 0) {
        xglVectorMulMat(vector, actor->overrideMatrix, vector);
    } else {
        xglVectorMulMat(vector, XenoActorLocalMatrix(actor), vector);
    }
    return 1;
}

void xglMatrixStackUnit(void);
void xglMatrixStackTrans(const Vector4 *vector);
void xglMatrixStackRotX(float angle);
void xglMatrixStackRotY(float angle);
void xglMatrixStackRotZ(float angle);
void xglMatrixStackScale(const Vector4 *vector);
void xglMatrixStackSave(Matrix4 matrix);
void xglMatrixStackLoad(Matrix4 matrix);
void xglMatrixStackMul(Matrix4 matrix);
/* Builds the actor transform on the matrix stack from its parent (or from itself), saves it in localMatrix and concatenates
 * the override matrix; a slot of 2 or more also multiplies by the parent's joint matrix and the pack's joint matrix. */
void XenoActorPushMatrix(XenoActor *actor) {
    XenoActor *transformOrigin;
    PxyModelHeader *pack;

    transformOrigin = actor->parent;
    xglMatrixStackUnit();
    if (transformOrigin == 0) {
        transformOrigin = actor;
    }
    xglMatrixStackTrans(&transformOrigin->position);
    xglMatrixStackRotZ(transformOrigin->rotation.z);
    xglMatrixStackRotY(transformOrigin->rotation.y);
    xglMatrixStackRotX(transformOrigin->rotation.x);
    xglMatrixStackScale(&transformOrigin->scale);
    xglMatrixStackTrans(&transformOrigin->vector60);
    xglMatrixStackSave(actor->localMatrix);
    if (actor->overrideMatrix != 0) {
        xglMatrixMul(XenoActorLocalMatrix(actor), XenoActorLocalMatrix(actor), actor->overrideMatrix);
    }
    if (XenoActorSlotIndex(actor) >= 2 && actor->parent != 0 && actor->parent->jointMatrices != 0) {
        pack = XenoActorModelPack(actor->parent);
        xglMatrixStackLoad(actor->localMatrix);
        xglMatrixStackMul(actor->parent->jointMatrices[XenoActorSlotIndex(actor)].m);
        xglMatrixStackMul(((XenoMatrix *)(BYTE_OFFSET(pack, pack->jointMatrixOffset)))[XenoActorSlotIndex(actor) - 2].m);
        xglMatrixStackSave(actor->localMatrix);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_00290548);

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_00290560);

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_00290578);

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_00290590);

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_00291118);

/* XenoActorReleaseAll: releases every record in the active table. */
void XenoActorReleaseAll(void) {
    int activeIndex;
    XenoActor *activeRecord;

    activeIndex = 0;
    if (XenoActorPoolCount > 0) {
        do {
            activeRecord = XenoActorActiveTable[activeIndex];
            activeIndex += 1;
            if (activeRecord != 0) {
                XenoActorRelease(activeRecord);
            }
        } while (activeIndex < XenoActorPoolCount);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_002911C0);

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_002912B0);

/* Returns the index of the first entry (112 bytes each, from the link record at actor+0x504) for which func_0030E68C returns 0; -1 when there is no link, the link's tag is not 0x797870 or none matches. */
int XenoActorFindPart(XenoActor *actor, const char *partName) {
    PxyModelHeader *partPack;
    unsigned char *partGeometryEntry;
    int i;

    if (actor != 0 && partName != 0) {
        partPack = XenoActorModelPack(actor);
        if (partPack != 0 && (*(u64 *)partPack & 0xFFFFFF) == XENO_PXY_TAG) {
            partGeometryEntry = BYTE_OFFSET(partPack, partPack->geometryOffset);
            i = 0;
            if (partPack->geometryCount > 0) {
                for (; i < partPack->geometryCount; i++) {
                    if (func_0030E68C(partGeometryEntry, partName) == 0) {
                        return i;
                    }
                    partGeometryEntry += 112;
                }
            }
        }
    }
    return -1;
}

int XenoActorHidePart(XenoActor *actor, const char *partName) {
    PxyModelHeader *pack;
    PxyModelHeader *subPack;
    unsigned char *geom;
    unsigned char *subGeom;
    int i;

    if (actor != 0 && partName != 0 && (pack = XenoActorModelPack(actor)) != 0 && (*(u64 *)pack & 0xFFFFFF) == XENO_PXY_TAG) {
        geom = BYTE_OFFSET(pack, pack->geometryOffset);
        if (pack->subPackOffset != 0) {
            subPack = (PxyModelHeader *)BYTE_OFFSET(pack, pack->subPackOffset);
            subGeom = BYTE_OFFSET(subPack, subPack->geometryOffset);
            for (i = 0; i < subPack->geometryCount; i++) {
                if (func_0030E68C(subGeom, partName) == 0) {
                    actor->subVisibility[i >> 5] |= 1 << (i & 31);
                    return 1;
                }
                subGeom += 112;
            }
        }
        for (i = 0; i < pack->geometryCount; i++) {
            if (func_0030E68C(geom, partName) == 0) {
                actor->visibility[i >> 5] |= 1 << (i & 31);
                return 1;
            }
            geom += 112;
        }
    }
    return 0;
}

int XenoActorShowPart(XenoActor *actor, const char *partName) {
    PxyModelHeader *pack;
    PxyModelHeader *subPack;
    unsigned char *geom;
    unsigned char *subGeom;
    int i;

    if (actor != 0 && partName != 0 && (pack = XenoActorModelPack(actor)) != 0 && (*(u64 *)pack & 0xFFFFFF) == XENO_PXY_TAG) {
        geom = BYTE_OFFSET(pack, pack->geometryOffset);
        if (pack->subPackOffset != 0) {
            subPack = (PxyModelHeader *)BYTE_OFFSET(pack, pack->subPackOffset);
            subGeom = BYTE_OFFSET(subPack, subPack->geometryOffset);
            for (i = 0; i < subPack->geometryCount; i++) {
                if (func_0030E68C(subGeom, partName) == 0) {
                    actor->subVisibility[i >> 5] &= ~(1 << (i & 31));
                    return 1;
                }
                subGeom += 112;
            }
        }
        for (i = 0; i < pack->geometryCount; i++) {
            if (func_0030E68C(geom, partName) == 0) {
                actor->visibility[i >> 5] &= ~(1 << (i & 31));
                return 1;
            }
            geom += 112;
        }
    }
    return 0;
}

void XenoActorRequestJointShift(XenoActor *actor) {
    actor->resetFlags |= 0x400;
}

void XenoActorShiftJoints(XenoActor *actor, int shiftDirection, int moveX, int moveY, int moveZ) {
    XenoHrc *hrcData;
    XenoJointRecord *jointRecordBase;
    u32 i;

    hrcData = actor->pHrcData;
    if (hrcData == 0) {
        return;
    }
    if (shiftDirection > 0) {
        for (i = 0; i < hrcData->jointRecordCount; i++) {
            if (moveX) {
                jointRecordBase = actor->jointRecords;
                jointRecordBase[i].translation.vector.x += actor->vector70.x;
            }
            if (moveY) {
                jointRecordBase = actor->jointRecords;
                jointRecordBase[i].translation.vector.y += actor->vector70.y;
            }
            if (moveZ) {
                jointRecordBase = actor->jointRecords;
                jointRecordBase[i].translation.vector.z += actor->vector70.z;
            }
        }
    } else {
        for (i = 0; i < hrcData->jointRecordCount; i++) {
            if (moveX) {
                jointRecordBase = actor->jointRecords;
                jointRecordBase[i].translation.vector.x -= actor->vector70.x;
            }
            if (moveY) {
                jointRecordBase = actor->jointRecords;
                jointRecordBase[i].translation.vector.y -= actor->vector70.y;
            }
            if (moveZ) {
                jointRecordBase = actor->jointRecords;
                jointRecordBase[i].translation.vector.z -= actor->vector70.z;
            }
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_002918C8);

/* Copies the position and rotation vectors and wraps the three rotation angles into [0, 2*pi) (X2 name XenoActorWrapRotation, lead only). */
void XenoActorWrapRotation(XenoActor *actor) {
    QuadwordCopy(&actor->previousPosition, &actor->position);
    QuadwordCopy(actor->angle, &actor->rotation);
    while (actor->angle[0] < 0.0f) {
        actor->angle[0] += 6.2831855f;
    }
    while (actor->angle[1] < 0.0f) {
        actor->angle[1] += 6.2831855f;
    }
    while (actor->angle[2] < 0.0f) {
        actor->angle[2] += 6.2831855f;
    }
    actor->resetFlags |= XENO_FLAG_SKIP_DELTA;
}

/* Moves the actor to its transformed position on the selected axes (moveX, moveY, moveZ) and shifts every joint record
 * back by the actor's offset on those axes. */
void XenoActorApplyRootOffset(XenoActor *actor, int moveX, int moveY, int moveZ) {
    Vector4 point;
    XenoHrc *hrcData;
    XenoJointRecord *jointRecordBase;
    u32 i;

    hrcData = actor->pHrcData;
    XenoActorPushMatrix(actor);
    xglCurMatrixSet(actor->localMatrix);
    xglCurApplyVector(&point, &actor->vector70);
    if (moveX) {
        actor->position.x = point.x;
    }
    if (moveY) {
        actor->position.y = point.y;
    }
    if (moveZ) {
        actor->position.z = point.z;
    }
    for (i = 0; i < hrcData->jointRecordCount; i++) {
        if (moveX) {
            jointRecordBase = actor->jointRecords;
            jointRecordBase[i].translation.vector.x -= actor->vector70.x;
        }
        if (moveY) {
            jointRecordBase = actor->jointRecords;
            jointRecordBase[i].translation.vector.y -= actor->vector70.y;
        }
        if (moveZ) {
            jointRecordBase = actor->jointRecords;
            jointRecordBase[i].translation.vector.z -= actor->vector70.z;
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_00291BC0);

/* Sets flag 0x10 on the first state, copies the target into the state at +0x590 and shifts the point at +0x5B0 into +0x5A0;
 * with resetFlags bit 0x100, draws the point's axes at 1.5. */
void XenoActorSetFocusTarget(XenoActor *actor, Vector4 *focusPoint) {
    f32 matrix[4][4];

    tiWordSetBits(actor->shadowB2, 16);
    QuadwordCopy(XenoActorFocusVector(actor), focusPoint);
    QuadwordCopy((Vector4 *)actor->unmodeled_5a0, (Vector4 *)(actor->unmodeled_5ac + 4));
    if (actor->resetFlags & 0x100) {
        xglCurSetTranslate(XenoActorFocusVector(actor));
        xglCurMatrixGet(matrix);
        tiDrawJointDrawAxes(matrix, 1.5f);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_00291C68);

/* Reads the two bounds of entry idx of the slot's range table as ints; 0 when the table or entry is missing or the bounds are inverted. */
int XenoActorGetSlotRange(XenoActor *actor, int motionSlot, unsigned int motionRangeIndex, int *motionRangeStart, int *motionRangeEnd) {
    OffsetTable *motionTable;
    TiOffsetEntry *motionRangeEntry;

    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 2988);
    }
    motionTable = XenoActorSlotTable(actor, motionSlot);
    if (motionTable == 0) {
        return 0;
    }
    if (motionRangeIndex < motionTable->count) {
        motionRangeEntry = (TiOffsetEntry *)BYTE_OFFSET(motionTable, *(motionTable->entryOffsets + motionRangeIndex));
        if (!(motionRangeEntry->rangeMin > motionRangeEntry->rangeMax)) {
            *motionRangeStart = motionRangeEntry->rangeMin;
            *motionRangeEnd = motionRangeEntry->rangeMax;
            return 1;
        }
    }
    return 0;
}

/* XenoActorGetPoolCount: returns the pool count XenoActorPoolCount. */
int XenoActorGetPoolCount(void) {
    return XenoActorPoolCount;
}

/* Calls XenoActorPushMatrix with the actor and vector, sets the matrix at +0xB0 and applies it to the vector through +0x70. */
void XenoActorTransformOffset(XenoActor *actor, Vector4 *vector) {
    XenoActorPushMatrix(actor);
    xglCurMatrixSet(&actor->localMatrix);
    xglCurApplyVector(vector, &actor->vector70);
}

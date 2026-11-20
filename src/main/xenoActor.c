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
extern float D_0055113C;

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

/* Indices of XenoActor.rsrc: the model pack (.pxy, xenoActor.c reads it as PxyHeader *), the texture (.txy), the
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
void XenoActorInitHeap(int memory, int size);
int XenoActorGetPoolCount(void);
void XenoActorClearHoldFlag(void);
void XenoActorRelease(XenoActor *actor);
void XenoActorReleaseAll(void);
void XenoActorSetHoldFlag(void);

/* The package block at 0x640 is typed int motionPack[4] and XenoMotionState motion[2] above; these are its TiMRPackage
 * and TiMRState views (main/TiMRPackage.h), for the calls into TiMRPackage.c. */
#define XenoActorMotionPack(a) ((TiMRPackage *)(a)->motionPack)
/* colorIndex value meaning "no color"; XenoActorInit fills the 0x10-byte block at colorIndex with it. */
#define XENO_COLOR_INDEX_NONE 0xFF

extern int XenoActorPoolCount;
extern int *XenoActorActiveTable;
extern const char xenoActorAssertFile[16];
extern const char D_0053F9A8[];
extern const char D_0053F910[];
extern const char D_004ADF70[];
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
#define XENO_ACTOR_POOL_STRIDE 0x6D0

extern XenoActor *D_0055112C;
extern unsigned char D_007A7C80[];
extern unsigned char D_00771280[];
void func_002969A8(void);
void func_0028F2C8(float value);
void tiMRStateSetValue(unsigned char *state, float value);
void func_00299730(TiMRPackage *pMRPackage, int value);
/* Builds the actor transform on the matrix stack from its parent (or from itself), saves it in localMatrix and concatenates * the override matrix; a slot of 2 or more also multiplies by the parent's joint matrix and the pack's joint matrix. */
void XenoActorPushMatrix();
void xglCurMatrixSet(void *matrix);
void xglCurApplyVector(Vector4 *in, Vector4 *out);
void xglCurSetTranslate(Vector4 *point);
void xglCurMatrixGet(void *matrix);
void tiDrawJointDrawAxes(void *matrix, float scale);
int func_0030E68C(unsigned char *entry, int value);
void XenoActorInit(XenoActor *actor);

/* Pool init (X2 homolog name XenoActorPoolInit, lead only): sets the pool base and count, builds the active table after the pool and inits every record. */
unsigned char *XenoActorPoolInit(void *data, int size) {
    unsigned char *end;
    int i;
    unsigned char *record;
    int *slot;

    end = data;
    XenoActorPoolCount = size;
    D_0055112C = (XenoActor *)end;
    end += size * XENO_ACTOR_POOL_STRIDE;
    XenoActorActiveTable = (int *)end;
    end += size * 4;
    record = data;
    for (i = 0; i < XenoActorPoolCount; i++) {
        XenoActorInit((XenoActor *)record);
        record += XENO_ACTOR_POOL_STRIDE;
    }
    if (XenoActorPoolCount > 0) {
        slot = XenoActorActiveTable;
        i = XenoActorPoolCount;
        do {
            *slot++ = 0;
        } while (--i);
    }
    XenoActorInitHeap((int)D_007A7C80, 0x40000);
    func_002969A8();
    XenoActorClearHoldFlag();
    return end;
}

/* Default pool init: runs the setup, sets the scale to 1.0 and initializes the pool at D_00771280 with 128 records. */
unsigned char *func_0028D4B0(void) {
    func_002969A8();
    func_0028F2C8(1.0f);
    return XenoActorPoolInit(D_00771280, 128);
}

/* XenoActorInitHeap: initializes RG heap 5, the heap the actor matrices are allocated from, over the given memory block. */
void XenoActorInitHeap(int memory, int size) {
    InitRgHeap(InstanceOfRgHeapById(5), (void *)memory, size); /* memory is an int address in the original's prototype */
}

/* Resets an actor record to its defaults: zeroes it, sets the unit scale, alpha and fade factors, the 0xFF block fills, the motion package, the light set and the collision block. */
void XenoActorInit(XenoActor *actor) {
    TiMRPackage *handle = (TiMRPackage *)&actor->shadowB;

    if (actor == 0) {
        return;
    }
    memset(actor, 0, sizeof(XenoActor));
    actor->position.w = 1.0f;
    {
        float gpDefaultA = 0.2f;
        actor->unmodeled_0a0 = gpDefaultA;
    }
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
    {
        short minusOneShort = -1;
        {
            float half = 0.5f;
            actor->unmodeled_124 = half;
        }
        {
            float minusOne = -1.0f;
            actor->unmodeled_128 = minusOne;
        }
        actor->unmodeled_09c = 0;
        actor->unmodeled_4f8 = minusOneShort;
        *(short *)actor->unmodeled_4c8 = minusOneShort;
    }
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
    tiMRPackageInit(handle);
    tiMRPackageSetAnimData(handle, 0);
    func_00299730(handle, 0);
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
    joints = (TiHrcStatic *)((unsigned char *)resource + resource->staticOffset);
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
int XenoActorAttachAtSlot(XenoActor *parent, XenoActor *child, int slotIndex) {
    if (parent != 0 && child != 0 && slotIndex >= 0) {
        *(short *)child->unmodeled_4c8 = slotIndex;
        XenoActorAttachChild(parent, child);
        return 1;
    }
    return 0;
}

int XenoActorReleaseChild(XenoActor *parent, XenoActor *child) {
    XenoActor **slot;
    int n;

    if (parent != 0 && child != 0 && (parent->attr & 1) && (child->attr & 1) && child->parent != 0) {
        slot = parent->children;
        for (n = 0; n < 8; n++) {
            if (*slot == child) {
                *slot = 0;
                child->parent = 0;
                *(short *)child->unmodeled_4c8 = -1;
                child->overrideMatrix = 0;
                return 1;
            }
            slot++;
        }
    }
    return 0;
}

/* XenoActorSetFlagAll (X2 homolog name, lead only): ORs a flag mask into the flag word of every live record. */
void XenoActorSetFlagAll(int flag) {
    unsigned int i;
    unsigned int count;
    unsigned int poolCount;
    XenoActor *record;

    count = XenoActorPoolCount;
    i = 0;
    if (count != 0) {
        poolCount = count;
        record = D_0055112C;
        do {
            i++;
            if (record->attr & 1) {
                record->drawFlags |= flag;
                count = poolCount;
            }
            record = (XenoActor *)((unsigned char *)record + XENO_ACTOR_POOL_STRIDE);
        } while (i < count);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028DE30);

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028DE40);

/* Sets the value of the state at +0x570 (shadowB2, X3 offset 1392) through tiMRStateSetValue. */
void XenoActorSetChannelValue(XenoActor *actor, float value) {
    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 501);
    }
    tiMRStateSetValue(actor->shadowB2, value);
}

/* Sets the value of the state at +0x5C0 (X3 offset 1472) through tiMRStateSetValue; asserts the actor is set. */
void XenoActorSetChannelValueB(XenoActor *actor, float value) {
    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 511);
    }
    tiMRStateSetValue(actor->unmodeled_5ac + 0x14, value);
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

void tiWordClearBits(void *block, int mask);
void tiWordSetBits(void *block, int mask);
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

/* Clears the package at +0x560 (X3 offset 1376): its anim table and anim data, then func_00299730. */
void XenoActorClearChannel(XenoActor *actor) {
    tiMRPackageSetAnimTable((TiMRPackage *)&actor->shadowB, 0);
    tiMRPackageSetAnimData((TiMRPackage *)&actor->shadowB, 0);
    func_00299730((TiMRPackage *)&actor->shadowB, 0);
}

/* Copies the position and rotation vectors and wraps the three rotation angles into [0, 2*pi) (X2 name XenoActorWrapRotation, lead only). */
void XenoActorWrapRotation(XenoActor *actor);
void tiMRPackageSetTableAndEntryA(TiMRPackage *pMRPackage, int table, short slot);

void XenoActorSetMode(XenoActor *actor, int slot) {
    TiOffsetEntry *motionKey;

    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 589);
    }
    actor->unmodeled_4f8 = -1;
    actor->unmodeled_4f4 = (short)slot;
    actor->unmodeled_4f6 = 0;
    tiMRPackageSetTableAndEntryA((TiMRPackage *)&actor->shadowB, actor->rsrc[4], (short)slot);
    actor->resetFlags &= ~0x40;
    tiWordClearBits(actor->shadowB2, 8);
    tiWordClearBits(actor->unmodeled_5ac + 0x14, 8);
    motionKey = actor->motionKeys[0];
    if (motionKey != 0) {
        if (*(float *)(actor->shadowB2 + 4) < 0.0f) {
            actor->frame = motionKey->rangeMax;
        } else {
            actor->frame = motionKey->rangeMin;
        }
        tiMRStateSetPosition(actor->shadowB2, actor->frame);
        if (!(actor->resetFlags & 0x10)) {
            if (*(unsigned int *)actor->shadowB2 & 2) {
                tiMtnBlendSwapLocal((TiMRPackage *)&actor->shadowB, actor->shadowB2);
                tiMtnBlendSwapGlobal((TiMRPackage *)&actor->shadowB, actor->shadowB2);
                actor->resetFlags |= 0x10;
            }
        }
    }
}

void XenoActorCheckMode(XenoActor *actor, int mode) {
    if (actor->unmodeled_4f4 != mode) {
        XenoActorSetMode(actor, mode);
    }
}

/* Sets the play range of state A (shadowB2) and clamps the play position into [start, end]: outside it, a backward motion (speed < 0) restarts from end and a forward one from start. */
void XenoActorSetPlayRange(XenoActor *actor, int start, int end) {
    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 647);
    }
    actor->resetFlags |= 0x40;
    if (actor->frame < (float)start || (float)end < actor->frame) {
        if (*(float *)(actor->shadowB2 + 4) < 0.0f) {
            actor->frame = (float)end;
        } else {
            actor->frame = (float)start;
        }
    }
    tiMRStateSetRange(actor->shadowB2, start, end);
}

void XenoActorConvertSourceRecord(XenoMotionState *record);

/* Sets the frame to the first range bound of state A and converts state A's source record. */
void XenoActorApplySourceValue(XenoActor *actor) {
    actor->frame = (float)((XenoMotionState *)actor->shadowB2)->range[0];
    XenoActorConvertSourceRecord((XenoMotionState *)actor->shadowB2);
}

void XenoActorSetModeInSlot(XenoActor *actor, u32 index, int slot) {
    TiOffsetEntry *motionKey;

    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 677);
    }
    if (index >= 4) {
        return;
    }
    actor->unmodeled_4f8 = -1;
    actor->unmodeled_4f4 = (short)slot;
    actor->unmodeled_4f6 = (short)index;
    tiMRPackageSetTableAndEntryA((TiMRPackage *)&actor->shadowB, actor->rsrc[index + 4], (short)slot);
    actor->resetFlags &= ~0x40;
    tiWordClearBits(actor->shadowB2, 8);
    tiWordClearBits(actor->unmodeled_5ac + 0x14, 8);
    motionKey = actor->motionKeys[0];
    if (motionKey != 0) {
        if (*(float *)(actor->shadowB2 + 4) < 0.0f) {
            actor->frame = motionKey->rangeMax;
        } else {
            actor->frame = motionKey->rangeMin;
        }
        tiMRStateSetPosition(actor->shadowB2, actor->frame);
        if (!(actor->resetFlags & 0x10)) {
            if (*(unsigned int *)actor->shadowB2 & 2) {
                tiMtnBlendSwapLocal((TiMRPackage *)&actor->shadowB, actor->shadowB2);
                tiMtnBlendSwapGlobal((TiMRPackage *)&actor->shadowB, actor->shadowB2);
                actor->resetFlags |= 0x10;
            }
        }
    }
}

/* Selects entry `selector` of package B (shadowB) and stores it as a halfword at +0x500, then copies the entry's range minimum into the cached float at +0x4FC. When state B (+0x5C0) has bit 2 and the blend is not swapped yet, the blend is swapped in and 0x10 is set. */
void XenoActorSetSelector(XenoActor *actor, int selector) {
    TiOffsetEntry *entry;
    float entryValue;

    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 725);
    }
    actor->unmodeled_500 = selector;
    tiMRPackageSelectEntryB((TiMRPackage *)&actor->shadowB, (short)selector);
    entry = (TiOffsetEntry *)actor->motionKeys[2];
    if (entry != 0) {
        entryValue = entry->rangeMin;
        actor->unmodeled_4fc = entryValue;
        tiMRStateSetPosition(actor->unmodeled_5ac + 0x14, entryValue);
        if (!(actor->resetFlags & 0x10) && (*(int *)(actor->unmodeled_5ac + 0x14) & 2)) {
            tiMtnBlendSwapLocal((TiMRPackage *)&actor->shadowB, actor->unmodeled_5ac + 0x14);
            tiMtnBlendSwapGlobal((TiMRPackage *)&actor->shadowB, actor->unmodeled_5ac + 0x14);
            actor->resetFlags |= 0x10;
        }
    }
}

/* Requests a mode for the actor: stores the mode as a halfword at +0x4F8 (X3 offset 1272). */
void XenoActorRequestMode(XenoActor *actor, int mode) {
    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 754);
    }
    actor->unmodeled_4f8 = mode;
}

/* Returns 1 when state A (shadowB2) has bit 8 set, else 0. */
int XenoActorIsDone(XenoActor *actor) {
    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 765);
    }
    if ((*(u32 *)actor->shadowB2 & 8) != 0) {
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028E5E8);

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028E788);

void tiMRPackageInit(TiMRPackage *pMRPackage);
void func_001975C8(void *block);
void func_002A70E8(void *block);
extern const float D_0054FD90;
extern const float D_0054FD94;
void func_00299350(TiMRPackage *pMRPackage, float (*matrix)[4]);
void func_0029F548(int value, TiMRPackage *pMRPackage, int other);

void XenoActorApplyRequestedMode(XenoActor *actor) {
    unsigned char *base;
    int rsrc0;

    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 910);
    }
    base = (unsigned char *)actor;
    if (actor->unmodeled_4f8 >= 0) {
        if (XenoActorIsDone(actor)) {
            XenoActorEnableChannel(actor);
            XenoActorSetMode(actor, actor->unmodeled_4f8);
            actor->unmodeled_4f8 = -1;
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
        func_00299350((TiMRPackage *)&actor->shadowB, actor->localMatrix);
        if ((*(int *)actor->shadowB2 & 0x20) || (*(int *)(actor->unmodeled_5ac + 0x14) & 0x20)) {
            actor->resetFlags |= 0x80;
        }
    }
    if (actor->unmodeled_658 != 0) {
        rsrc0 = actor->rsrc[0];
        if (rsrc0 != 0) {
            func_0029F548(actor->unmodeled_54c, (TiMRPackage *)&actor->shadowB,
                          *(int *)(rsrc0 + *(int *)(rsrc0 + 208) + 96));
        }
    }
    actor->resetFlags &= ~0x10;
    tiWordClearBits(actor->shadowB2, 16);
}

int XenoActorSlotRangeValid(XenoActor *actor, int slot, unsigned int index) {
    OffsetTable *table;
    TiOffsetEntry *entry;

    table = ((OffsetTable **)&actor->rsrc[4])[slot];
    if (table == 0) {
        return 0;
    }
    if (index >= table->count) {
        return 0;
    }
    entry = (TiOffsetEntry *)((unsigned char *)table + *(table->entryOffsets + index));
    if (entry->rangeMin <= entry->rangeMax) {
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

/* Flag word bit of XenoActor.flags (X3 offset 0x4EC): set by XenoActorWrapRotation, the next update only snapshots the joint rows. */
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
        if ((XenoActor *)XenoActorActiveTable[i] == actor) {
            XenoActorActiveTable[i] = 0;
            found = 1;
            break;
        }
    }
    if (!found) {
        assertFailed(D_0053F9A8 + 0x30, D_0053F910, 1050);
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
 * (drawFlags bit 0x8000000), or from 1 down to 0 past 25 * D_0055113C (bit 0x8000). */
void func_0028F2D0(XenoActor *actor)
{
    XglCamera *camera;
    PxyHeader *pack;
    DoublewordAlignedVector4 target;
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
        pack = (PxyHeader *)actor->rsrc[XAct_Rsrc_MDL];
        if (pack != 0 && pack->magic[0] == 'p' && pack->magic[1] == 'x' && pack->magic[2] == 'y' && (pack->flags & 0x81) == 0) {
            radius = pack->sphere.w;
            target = actor->position;
            cameraPosition = *(DoublewordVector4 *)&camera->travel.position;
            if (actor->fadeRadius != 0.0f) {
                radius = actor->fadeRadius;
            }
            Vu0AddVectorXYZ(&target, &target, &actor->vector70);
            target.w = 1.0f;
            distance = xglPointLength(&cameraPosition.vector, &target);
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
        pack = (PxyHeader *)actor->rsrc[XAct_Rsrc_MDL];
        if (pack == 0 || pack->magic[0] != 'p' || pack->magic[1] != 'x' || pack->magic[2] != 'y' || (pack->flags & 0x81) != 0) {
            actor->alphaScale = alpha;
            return;
        }
        radius = D_0055113C * 25.0f;
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

void func_0028FA00(XenoActor *actor) {
    actor->resetFlags &= 0xFFFDFFFF;
    actor->resetFlags &= 0xFFFBFFFF;
}

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028FA28);

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_0028FBA0);

/* Draws each item of the HRC table at actor+0x630 (0x30 bytes each, from its dataOffset) at the item's weight through tiDrawCollisionDrawObjectRing. */
void XenoActorRefreshItems(XenoActor *actor) {
    XenoHrc *table;
    unsigned int itemIndex;
    unsigned char *item;

    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 0x77D);
    }
    table = (XenoHrc *)actor->pHrcData;
    itemIndex = 0;
    if (table != 0) {
        item = (unsigned char *)table + table->dataOffset;
        if (table->count != 0) {
            do {
                tiDrawCollisionDrawObjectRing(*(float *)(item + 4), item + 0x20, D_004ADF70);
                itemIndex++;
                item += 0x30;
            } while (itemIndex < ((XenoHrc *)actor->pHrcData)->count);
        }
    }
}

int tiMRPackageGetNodeMatrix(Vector4 *block, void *arg);
void xglMatrixMul(XenoMatrix *dst, XenoMatrix *a, XenoMatrix *b);

int XenoActorMulMatrix(XenoActor *actor, void *arg, XenoMatrix *matrix) {
    if (tiMRPackageGetNodeMatrix(&actor->shadowB, arg) == 0) {
        return 0;
    }
    if (actor->overrideMatrix != 0) {
        xglMatrixMul(matrix, matrix, actor->overrideMatrix);
    } else {
        xglMatrixMul(matrix, matrix, (XenoMatrix *)&actor->localMatrix);
    }
    return 1;
}

int tiMRPackageGetNodePoint(Vector4 *block, void *arg);
void xglVectorMulMat(Vector4 *dst, XenoMatrix *matrix, Vector4 *src);

int XenoActorMulVector(XenoActor *actor, void *arg, Vector4 *vector) {
    if (tiMRPackageGetNodePoint(&actor->shadowB, arg) == 0) {
        return 0;
    }
    if (actor->overrideMatrix != 0) {
        xglVectorMulMat(vector, actor->overrideMatrix, vector);
    } else {
        xglVectorMulMat(vector, (XenoMatrix *)&actor->localMatrix, vector);
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
void XenoActorPushMatrix(actor) XenoActor *actor;

{
    XenoActor *source;
    PxyHeader *pack;

    source = actor->parent;
    xglMatrixStackUnit();
    if (source == 0) {
        source = actor;
    }
    xglMatrixStackTrans(&source->position);
    xglMatrixStackRotZ(source->rotation.z);
    xglMatrixStackRotY(source->rotation.y);
    xglMatrixStackRotX(source->rotation.x);
    xglMatrixStackScale(&source->scale);
    xglMatrixStackTrans(&source->vector60);
    xglMatrixStackSave(actor->localMatrix);
    if (actor->overrideMatrix != 0) {
        xglMatrixMul((XenoMatrix *)&actor->localMatrix, (XenoMatrix *)&actor->localMatrix, actor->overrideMatrix);
    }
    if (*(short *)actor->unmodeled_4c8 >= 2 && actor->parent != 0 && actor->parent->jointMatrices != 0) {
        pack = (PxyHeader *)actor->parent->rsrc[XAct_Rsrc_MDL];
        xglMatrixStackLoad(actor->localMatrix);
        xglMatrixStackMul(actor->parent->jointMatrices[*(short *)actor->unmodeled_4c8].m);
        xglMatrixStackMul(((XenoMatrix *)((unsigned char *)pack + pack->jointMatrixOffset))[*(short *)actor->unmodeled_4c8 - 2].m);
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
    int index;
    int activeRecord;

    index = 0;
    if (XenoActorPoolCount > 0) {
        do {
            activeRecord = XenoActorActiveTable[index];
            index += 1;
            if (activeRecord != 0) {
                XenoActorRelease((XenoActor *)activeRecord);
            }
        } while (index < XenoActorPoolCount);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_002911C0);

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_002912B0);

/* Returns the index of the first entry (112 bytes each, from the link record at actor+0x504) for which func_0030E68C returns 0; -1 when there is no link, the link's tag is not 0x797870 or none matches. */
int XenoActorFindPart(XenoActor *actor, int value) {
    unsigned char *link;
    unsigned char *entry;
    int i;

    if (actor != 0 && value != 0) {
        link = (unsigned char *)actor->rsrc[0];
        if (link != 0 && (*(u64 *)link & 0xFFFFFF) == 0x797870) {
            entry = link + *(int *)(link + 108);
            i = 0;
            if (*(int *)(link + 96) > 0) {
                for (; i < *(int *)(link + 96); i++) {
                    if (func_0030E68C(entry, value) == 0) {
                        return i;
                    }
                    entry += 112;
                }
            }
        }
    }
    return -1;
}

int XenoActorHidePart(XenoActor *actor, const char *partName) {
    PxyHeader *pack;
    PxyHeader *subPack;
    unsigned char *geom;
    unsigned char *subGeom;
    int i;

    if (actor != 0 && partName != 0 && (pack = (PxyHeader *)actor->rsrc[XAct_Rsrc_MDL]) != 0 && (*(u64 *)pack & 0xFFFFFF) == 0x797870) {
        geom = (unsigned char *)pack + pack->geometryOffset;
        if (pack->subPackOffset != 0) {
            subPack = (PxyHeader *)((unsigned char *)pack + pack->subPackOffset);
            subGeom = (unsigned char *)subPack + subPack->geometryOffset;
            for (i = 0; i < subPack->geometryCount; i++) {
                if (func_0030E68C(subGeom, (int)partName) == 0) {
                    actor->subVisibility[i >> 5] |= 1 << (i & 31);
                    return 1;
                }
                subGeom += 112;
            }
        }
        for (i = 0; i < pack->geometryCount; i++) {
            if (func_0030E68C(geom, (int)partName) == 0) {
                actor->visibility[i >> 5] |= 1 << (i & 31);
                return 1;
            }
            geom += 112;
        }
    }
    return 0;
}

int XenoActorShowPart(XenoActor *actor, const char *partName) {
    PxyHeader *pack;
    PxyHeader *subPack;
    unsigned char *geom;
    unsigned char *subGeom;
    int i;

    if (actor != 0 && partName != 0 && (pack = (PxyHeader *)actor->rsrc[XAct_Rsrc_MDL]) != 0 && (*(u64 *)pack & 0xFFFFFF) == 0x797870) {
        geom = (unsigned char *)pack + pack->geometryOffset;
        if (pack->subPackOffset != 0) {
            subPack = (PxyHeader *)((unsigned char *)pack + pack->subPackOffset);
            subGeom = (unsigned char *)subPack + subPack->geometryOffset;
            for (i = 0; i < subPack->geometryCount; i++) {
                if (func_0030E68C(subGeom, (int)partName) == 0) {
                    actor->subVisibility[i >> 5] &= ~(1 << (i & 31));
                    return 1;
                }
                subGeom += 112;
            }
        }
        for (i = 0; i < pack->geometryCount; i++) {
            if (func_0030E68C(geom, (int)partName) == 0) {
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

void XenoActorShiftJoints(XenoActor *actor, int add, int moveX, int moveY, int moveZ) {
    XenoHrc *table;
    XenoJointRecord *records;
    u32 i;

    table = actor->pHrcData;
    if (table == 0) {
        return;
    }
    if (add > 0) {
        for (i = 0; i < table->jointRecordCount; i++) {
            if (moveX) {
                records = actor->jointRecords;
                records[i].translation.vector.x += actor->vector70.x;
            }
            if (moveY) {
                records = actor->jointRecords;
                records[i].translation.vector.y += actor->vector70.y;
            }
            if (moveZ) {
                records = actor->jointRecords;
                records[i].translation.vector.z += actor->vector70.z;
            }
        }
    } else {
        for (i = 0; i < table->jointRecordCount; i++) {
            if (moveX) {
                records = actor->jointRecords;
                records[i].translation.vector.x -= actor->vector70.x;
            }
            if (moveY) {
                records = actor->jointRecords;
                records[i].translation.vector.y -= actor->vector70.y;
            }
            if (moveZ) {
                records = actor->jointRecords;
                records[i].translation.vector.z -= actor->vector70.z;
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
    XenoHrc *table;
    XenoJointRecord *records;
    u32 i;

    table = actor->pHrcData;
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
    for (i = 0; i < table->jointRecordCount; i++) {
        if (moveX) {
            records = actor->jointRecords;
            records[i].translation.vector.x -= actor->vector70.x;
        }
        if (moveY) {
            records = actor->jointRecords;
            records[i].translation.vector.y -= actor->vector70.y;
        }
        if (moveZ) {
            records = actor->jointRecords;
            records[i].translation.vector.z -= actor->vector70.z;
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_00291BC0);

/* Sets flag 0x10 on the first state, copies the target into the state at +0x590 and shifts the point at +0x5B0 into +0x5A0;
 * with resetFlags bit 0x100, draws the point's axes at 1.5. */
void XenoActorSetFocusTarget(XenoActor *actor, Vector4 *target) {
    f32 matrix[4][4];

    tiWordSetBits(actor->shadowB2, 16);
    QuadwordCopy((Vector4 *)actor->colorIndex, target);
    QuadwordCopy((Vector4 *)actor->unmodeled_5a0, (Vector4 *)(actor->unmodeled_5ac + 4));
    if (actor->resetFlags & 0x100) {
        xglCurSetTranslate((Vector4 *)actor->colorIndex);
        xglCurMatrixGet(matrix);
        tiDrawJointDrawAxes(matrix, 1.5f);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/xenoActor", func_00291C68);

/* Reads the two bounds of entry idx of the slot's range table as ints; 0 when the table or entry is missing or the bounds are inverted. */
int XenoActorGetSlotRange(XenoActor *actor, int slot, unsigned int idx, int *start, int *end) {
    OffsetTable *table;
    TiOffsetEntry *range;

    if (actor == 0) {
        assertFailed(D_0053F9A8, D_0053F910, 2988);
    }
    table = ((OffsetTable **)&actor->rsrc[4])[slot];
    if (table == 0) {
        return 0;
    }
    if (idx < table->count) {
        range = (TiOffsetEntry *)((unsigned char *)table + *(table->entryOffsets + idx));
        if (!(range->rangeMin > range->rangeMax)) {
            *start = (int)range->rangeMin;
            *end = (int)range->rangeMax;
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
    XenoActorPushMatrix(actor, vector);
    xglCurMatrixSet(&actor->localMatrix);
    xglCurApplyVector(vector, &actor->vector70);
}

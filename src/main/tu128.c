#include "common.h"
#include "shared.h"

#include "quadword.h"
#include "gs.h"
#include "vu0_macro.h"
/* Five signed halfwords in the TU's .data (scaffold label gSefDraw2DOverrides, 10 bytes). */
extern s16 gSefDraw2DOverrides[5];

/* Returns the first of the five 2D draw overrides (gSefDraw2DOverrides). */
s16 sefGetDraw2DOverride(void) {
    return gSefDraw2DOverrides[0];
}

typedef signed char s8;

typedef int s32;

extern s16 gSefDraw3DOverrides[];

/* Returns the first 3D draw override (gSefDraw3DOverrides[0]). */
s16 sefGetDraw3DOverride(void) {
    return gSefDraw3DOverrides[0];
}

/* One 0x40-byte entry of the record table at gSefEffectTransforms (0x28 entries, indexed by sefEffectTransformClear). */
typedef struct EffectRecordCell {
    unsigned char unmodeled_00[0x40];
} EffectRecordCell;

extern EffectRecordCell gSefEffectTransforms[];
extern unsigned char gSefEffectTable[];
void *memset(void *destination, s32 value, u32 size);

/* Clears the effect table (0x494C0 bytes) and the record table (0xA00 bytes). */
void sefEffectTablesClear(void) {
    memset(gSefEffectTable, 0, 0x494C0);
    memset(gSefEffectTransforms, 0, 0xA00);
}

typedef union SefFlagWord {
    s32 flags;
    struct {
        unsigned int lo : 8;
        unsigned int b8 : 1;
        unsigned int rest : 23;
    } bits;
} SefFlagWord;

typedef struct SefSlot {
    s32 active;
    s32 value;
    unsigned char unmodeled_08[8];
    f32 weight;
    s32 mapValue;
    s16 poolSlots[32];
    s16 key;
    s16 index;
    s16 mapIndex;
    s8 flag;
    s8 result;
    SefFlagWord u;
} SefSlot;

typedef struct SefEffectNode SefEffectNode;

typedef struct SefEffectNodeLink {
    u16 index;
    u16 unmodeled_52;
    SefEffectNode *next;
    s32 used;
    s32 unmodeled_5C;
} SefEffectNodeLink;

struct SefEffectNode {
    AlignedVector4 vectors[3];
    unsigned char unmodeled_30[0x1C];
    s16 pairX;
    s16 pairY;
    SefEffectNodeLink key;
};

typedef struct SefPoolEntry {
    s16 inUse;
    unsigned char unmodeled_02[0x6E];
} SefPoolEntry;

typedef struct SefKey {
    f32 pos[3];
    f32 rot;
    f32 col[3];
    f32 sc;
} SefKey;

/* The effect table at gSefEffectTable: keys, pool, nodes, slots, key-use flags and the ring/cursor counters. */
typedef struct SefEffectTable {
    SefKey keys[5000];
    SefPoolEntry effectPool[400];
    SefEffectNode effectNodes[900];
    SefSlot effectSlots[40];
    s8 keyUsed[5000];
    s16 ringStart;
    s16 ringCount;
    s16 poolCursor;
    s16 nodeCursor;
    s16 nodesInUse;
    s16 claimedSlots;
    s16 unk_494B4;
    s16 unk_494B6;
} SefEffectTable;

/* The effect table at gSefEffectTable seen with the complete layout of SefEffectTable. */
#define SEF_SLOTS (*(SefEffectTable *)&gSefEffectTable)
/* Bit 8 of SefSlot.u (the b8 bit of SefFlagWord); its meaning is not proven, only that the slot is claimed. */
#define SEF_SLOT_FLAG_B8 0x100

s32 svFindEffectMapperIndex(s32 value);
s32 sefEffectTransformClaim(void);

/* Claims the first free effect slot (flag 0): marks it active with the B8 flag, stores key and value, returns its index or -1. */
s32 sefClaimFirstSlot(s32 key, s32 mapValue) {
    s32 index;

    for (index = 0; index < 40; index++) {
        if (SEF_SLOTS.effectSlots[index].flag == 0) {
            SEF_SLOTS.effectSlots[index].active = 1;
            SEF_SLOTS.effectSlots[index].weight = 1.0f;
            SEF_SLOTS.effectSlots[index].u.flags |= SEF_SLOT_FLAG_B8;
            SEF_SLOTS.effectSlots[index].key = key;
            SEF_SLOTS.effectSlots[index].mapValue = mapValue;
            SEF_SLOTS.effectSlots[index].index = index;
            SEF_SLOTS.effectSlots[index].mapIndex = sefEffectTransformClaim();
            SEF_SLOTS.effectSlots[index].result = svFindEffectMapperIndex(mapValue);
            SEF_SLOTS.claimedSlots++;
            memset(SEF_SLOTS.effectSlots[index].poolSlots, -1, sizeof(SEF_SLOTS.effectSlots[index].poolSlots));
            return index;
        }
    }
    return -1;
}


/* Claims slot slotIndex for a resource (when it is in range and free): marks it active with the B8 flag and returns it, or -1. */
s32 sefClaimSlotAt(s32 claimSlot, s32 resourceId, s32 mapValue) {
    if (claimSlot >= 0 && SEF_SLOTS.effectSlots[claimSlot].flag == 0) {
        SEF_SLOTS.effectSlots[claimSlot].active = 1;
        SEF_SLOTS.effectSlots[claimSlot].weight = 1.0f;
        SEF_SLOTS.effectSlots[claimSlot].u.bits.b8 = 1;
        SEF_SLOTS.effectSlots[claimSlot].mapValue = mapValue;
        SEF_SLOTS.effectSlots[claimSlot].key = resourceId;
        SEF_SLOTS.effectSlots[claimSlot].index = claimSlot;
        SEF_SLOTS.effectSlots[claimSlot].mapIndex = sefEffectTransformClaim();
        SEF_SLOTS.effectSlots[claimSlot].result = svFindEffectMapperIndex(mapValue);
        SEF_SLOTS.claimedSlots++;
        memset(SEF_SLOTS.effectSlots[claimSlot].poolSlots, -1, sizeof(SEF_SLOTS.effectSlots[claimSlot].poolSlots));
        return claimSlot;
    }
    return -1;
}

/* The object record read by objectHasFlag5E: only the byte at +0x5E is claimed here. */
typedef struct EffectTaggedRecord {
    unsigned char unmodeled_00[0x5E];
    signed char unk_5E;
} EffectTaggedRecord;

/* Returns 1 when the record is present and its byte at +0x5E is nonzero, else 0. */
s32 objectHasFlag5E(EffectTaggedRecord *taggedRecord) {
    if (!(taggedRecord != 0)) {
        return 0;
    }
    return taggedRecord->unk_5E != 0;
}

typedef struct TypeList {
    s32 used;
    s32 unmodeled_04[7];
    struct {
        s32 h;
        s32 b;
        s16 refs;
        s8 on;
    } s;
    u8 unmodeled_2C[0x13DC - 0x2C];
} TypeList;

extern TypeList svResourceTypeLists[];
void srsReleaseEffectRecordsByResource(s32 used, s32 flag);
/* Zeroes record number recordNumber (0x40 bytes); numbers past the 40 records are ignored. */
void sefEffectTransformClear(u32 index);
void sefEffectPoolRelease(u32 i);

/* Releases one effect slot: frees its pool entries, drops its resource type reference and clears the slot record. */
void sefEffectInstanceRelease(SefSlot *effectSlot) {
    s32 poolIndex;

    if (effectSlot != 0) {
        for (poolIndex = 0; poolIndex < 32; poolIndex++) {
            if (effectSlot->poolSlots[poolIndex] >= 0) {
                sefEffectPoolRelease(effectSlot->poolSlots[poolIndex]);
            }
        }
        sefEffectTransformClear(effectSlot->mapIndex);
        svResourceTypeLists[effectSlot->result].s.refs--;
        if (svResourceTypeLists[effectSlot->result].s.refs <= 0) {
            svResourceTypeLists[effectSlot->result].s.refs = 0;
            if (svResourceTypeLists[effectSlot->result].s.on != 0) {
                srsReleaseEffectRecordsByResource(svResourceTypeLists[effectSlot->result].used, 1);
            }
        }
        memset(effectSlot, 0, sizeof(SefSlot));
        SEF_SLOTS.claimedSlots--;
        if (SEF_SLOTS.claimedSlots < 0) {
            SEF_SLOTS.claimedSlots = 0;
        }
    }
}

typedef AlignedVector4 V4;

typedef struct KeyR {
    V4 a;
    V4 b;
} KeyR;

typedef struct Node {
    V4 a;
    V4 b;
    f32 f20[8];
    union {
        f32 f;
        s32 w;
    } f40;
    union {
        s16 h[4];
        s32 q[2];
    } nu;
    s16 firstKey;
    s16 keysTaken;
    union {
        s16 v;
        s16 w;
    } idx;
    s16 q;
    union {
        struct Node *p;
        s32 w;
    } next;
    struct Pool *used;
    s32 tail;
} Node;

typedef struct Pool {
    V4 v[4];
    s16 *ptr;
    s32 r44[3];
    s16 used;
    s16 b52;
    s32 b54;
    u64 fl;
    union {
        struct Node *p;
        s32 w;
    } head;
    s32 r[3];
} Pool;

typedef struct Ent {
    s32 f0;
    s32 f4;
    s32 f8;
    s32 fC;
    f32 f10;
    s32 res;
    s16 slots[32];
    s16 id;
    s16 idx;
    s16 slot;
    s8 active;
    s8 mapper;
    s32 flags;
} Ent;

typedef struct Tab {
    KeyR keys[5000];
    Pool pool[400];
    Node node[900];
    Ent e[40];
    s8 keyUsed[5000];
    s16 keyCur;
    s16 keyCount;
    s16 cursor;
    s16 cur2;
    s16 count;
    s16 gate;
    s16 cb4;
    s16 cb6;
} Tab;

#define T (*(Tab *)&gSefEffectTable)

/* Claims the first free pool entry from the pool cursor (used == 0), moves the cursor past it and returns it, or -1. */
s32 sefEffectPoolClaim(void) {
    s32 effectPoolIndex = T.cursor;
    s32 scanEnd = (effectPoolIndex - 1) % 400;

    while (effectPoolIndex != scanEnd) {
        if (T.pool[effectPoolIndex].used == 0) {
            T.cursor = (effectPoolIndex + 1) % 400;
            return effectPoolIndex;
        }
        effectPoolIndex = (effectPoolIndex + 1) % 400;
        if (scanEnd != T.cursor) {
            scanEnd = T.cursor;
        }
    }
    return -1;
}

typedef struct SefSlotView {
    s32 active;
    s32 value;
    unsigned char unmodeled_08[8];
    f32 weight;
    s32 mapValue;
    unsigned char block18[0x40];
    s16 key;
    s16 index;
    s16 mapIndex;
    s8 flag;
    s8 result;
    s32 flags;
} SefSlotView;

/* Claims one node from the node cursor (key.used == 0), counts it as used and returns its index, or -1 when the table is full. */
s32 sefEffectNodeClaim(void) {
    s32 candidateNode;
    s32 scanEnd;
    s32 nextCandidate;

    if (SEF_SLOTS.nodesInUse < 900) {
        candidateNode = SEF_SLOTS.nodeCursor;
        nextCandidate = (candidateNode - 1) % 900;
        scanEnd = nextCandidate;
        while (candidateNode != scanEnd) {
            if (SEF_SLOTS.effectNodes[candidateNode].key.used == 0) {
                SEF_SLOTS.nodeCursor = (candidateNode + 1) % 900;
                SEF_SLOTS.nodesInUse++;
                return candidateNode;
            }
            nextCandidate = (candidateNode + 1) % 900;
            scanEnd = SEF_SLOTS.nodeCursor;
            if (nextCandidate == scanEnd) {
                break;
            }
            candidateNode = nextCandidate;
        }
    }
    return -1;
}

void sefEffectNodeRelease(SefEffectNode *node);

/* Frees pool entry poolIndex: releases its nodes through sefEffectNodeRelease, clears the entry and moves the pool cursor back to it. */
void sefEffectPoolRelease(u32 effectPoolIndex) {
    if (effectPoolIndex < 400) {
        Node *chainNode = T.pool[effectPoolIndex].head.p;
        while (chainNode != 0) {
            Node *freedNode = chainNode;
            chainNode = chainNode->next.p;
            sefEffectNodeRelease(freedNode);
        }
        memset(&T.pool[effectPoolIndex], 0, sizeof(Pool));
        T.cursor = effectPoolIndex;
    }
}

void sefReleaseKeys(s32 startFrame, s32 count);

/* Frees one node: releases its key range through sefReleaseKeys, makes it the next free node, clears it and counts one less. */
void sefEffectNodeRelease(SefEffectNode *node) {
    sefReleaseKeys(node->pairX, node->pairY);
    SEF_SLOTS.nodeCursor = node->key.index;
    memset(node, 0, sizeof(SefEffectNode));
    SEF_SLOTS.nodesInUse--;
    if (SEF_SLOTS.nodesInUse < 0) {
        SEF_SLOTS.nodesInUse = 0;
    }
}

typedef struct SefSlotAlt {
    s32 active;
    s32 value;
    s32 f8;
    s32 fC;
    f32 weight;
    s32 mapValue;
    s16 poolSlots[32];
    s16 key;
    s16 index;
    s16 mapIndex;
    s8 flag;
    s8 result;
    SefFlagWord u;
} SefSlotAlt;

typedef struct SefPool {
    AlignedVector4 v[5];
    s16 used;
    unsigned char unmodeled_52[0x1E];
} SefPool;

typedef struct SefCurve {
    void *handle;
    s32 x[2];
    s32 value;
} SefCurve;

typedef struct SefSpawnObject {
    unsigned char a[0x38];
    SefCurve *positionCurve;
    SefCurve *colorCurve;
    s32 x40;
    f32 *f44;
    s32 x48;
    f32 *f4c;
    s16 n50;
} SefSpawnObject;

typedef struct SefSpawn {
    unsigned char unmodeled_00[0x4C];
    s16 firstKey;
    s16 keysTaken;
    s16 unk_50;
    s16 unk_52;
    s32 unk_54;
    SefSpawnObject *ob;
} SefSpawn;

f32 sefNoiseNext(void);
void func_00206058(void *position, SefCurve *curve, void *owner, s32 v);

/* Takes up to object->n50 free key records from the key ring, fills each one from its object curves and noise, and stores the
   first key and the count in the spawn. Returns the count taken, or 0 when the ring is full. */
s32 sefTakeKeys(SefSpawn *spawn) {
    s32 keyIndex;
    s32 scanEnd;
    s32 taken = 0;
    s32 firstKey = -1;
    SefSpawnObject *spawnObject = spawn->ob;

    keyIndex = SEF_SLOTS.ringStart;
    scanEnd = (keyIndex - 1) % 5000;
    if (SEF_SLOTS.ringCount >= 5000) {
        return 0;
    }
    while (keyIndex != scanEnd) {
        if (SEF_SLOTS.keyUsed[keyIndex] == 0) {
            SefKey *key = &SEF_SLOTS.keys[keyIndex];

            if (spawnObject->positionCurve != 0) {
                func_00206058(key->pos, spawnObject->positionCurve, 0, spawnObject->positionCurve->value);
            }
            if (spawnObject->colorCurve != 0) {
                func_00206058(key->col, spawnObject->colorCurve, key, spawnObject->colorCurve->value);
            }
            if (spawnObject->f44[-1] != 0.0f) {
                key->rot = sefNoiseNext() * (spawnObject->f44[-1] / 180.0f * 3.14159265f);
            }
            key->sc = spawnObject->f4c[-2] * (1.0f - sefNoiseNext() * spawnObject->f4c[-1]);
            if (taken == 0) {
                firstKey = keyIndex;
            }
            SEF_SLOTS.keyUsed[keyIndex] = 1;
            taken++;
        } else {
            if (firstKey != -1) {
                break;
            }
        }
        if (taken == spawnObject->n50) {
            break;
        }
        keyIndex = (keyIndex + 1) % 5000;
        scanEnd = SEF_SLOTS.ringStart;
    }
    if (taken != 0) {
        spawn->firstKey = firstKey;
        spawn->keysTaken = taken;
        SEF_SLOTS.ringStart = (keyIndex + 1) % 5000;
        SEF_SLOTS.ringCount += taken;
        return taken;
    }
    return 0;
}

extern unsigned char gSefKeyUsed[5000];

/* Releases count key records starting at ring index start (wrapping at 5000) and moves the ring start back to start. */
void sefReleaseKeys(s32 keyStart, s32 keyCount) {
    s32 keyEnd = keyStart + keyCount;

    if (keyCount == 0) {
        return;
    }
    if (keyCount >= 2 && keyEnd > 5000) {
        keyEnd -= 5000;
        memset(&gSefKeyUsed[keyStart], 0, 5000 - keyStart);
        memset(gSefKeyUsed, 0, keyEnd + 1);
    } else {
        memset(&gSefKeyUsed[keyStart], 0, keyCount);
    }
    SEF_SLOTS.ringStart = keyStart;
    SEF_SLOTS.ringCount -= keyCount;
}

typedef struct RecordEntryFloats {
    f32 pos[4];
    f32 unk_10[4];
    f32 unk_20[4];
    f32 unk_30[4];
} RecordEntryFloats;

extern RecordEntryFloats D_004516B0_1F5CA8[];

/* Claims the first record of the 40-entry table at D_004516B0_1F5CA8 whose pos[3] is 0 (sets it to 1); returns its index or -1. */
s32 sefEffectTransformClaim(void) {
    s32 recordNumber;

    for (recordNumber = 0; recordNumber < 40; recordNumber++) {
        if (D_004516B0_1F5CA8[recordNumber].pos[3] == 0.0f) {
            D_004516B0_1F5CA8[recordNumber].pos[3] = 1.0f;
            return recordNumber;
        }
    }
    return -1;
}

void sefEffectTransformClear(u32 recordNumber) {
    if (recordNumber < 0x28U) {
        memset(&gSefEffectTransforms[recordNumber], 0, 0x40);
    }
}

typedef struct SvTypeListEntry {
    void *data;
    u8 unmodeled_04[0x0C - 0x04];
    u16 resourceKind;
    u8 unmodeled_0E[0x12 - 0x0E];
    u16 basePointer;
    u16 bitsPerPixel;
    u16 bufferWidthPixels;
    u16 width;
    u16 height;
    u16 textureWidthExponent;
    u16 textureHeightExponent;
} SvTypeListEntry;

typedef struct EffectNodeView {
    V4 a;
    V4 b;
    f32 f20[8];
    union {
        f32 f;
        s32 w;
    } f40;
    union {
        s16 h[4];
        s32 q[2];
    } nu;
    s16 firstKey;
    s16 keysTaken;
    union {
        s16 v;
        s16 w;
    } idx;
    s16 q;
    union {
        struct EffectNodeView *p;
        s32 w;
    } next;
    struct EffectPoolRecord *used;
    s32 tail;
} EffectNodeView;

/* One 0x70-byte pool entry of the effect table as sefBuildSpecEntries fills it. */
typedef struct EffectPoolRecord {
    V4 vector0;
    V4 translation;
    u64 tex0;
    f32 unk_28;
    f32 unk_2C;
    f32 unk_30;
    union {
        f32 asFloat;
        s32 asInt;
    } unk_34;
    void *unk_38;
    void *unk_3C;
    s16 *keys;
    s16 *unk_44;
    s16 *unk_48;
    s16 *unk_4C;
    s16 used;
    s16 frameStart;
    s16 frameEnd;
    s16 frameLength;
    union {
        u64 packed;
        struct {
            s16 effectFlags;
            u8 track;
            u8 texWidth;
            u8 texHeight;
            u8 unk_5D;
            u8 kind;
            s8 unk_5F;
        } bits;
    } flags;
    union {
        struct EffectNodeView *p;
        s32 w;
    } head;
    s32 reserved[3];
} EffectPoolRecord;

typedef struct EffectTableView {
    KeyR keys[5000];
    EffectPoolRecord pool[400];
    EffectNodeView node[900];
    Ent e[40];
    s8 keyUsed[5000];
    s16 keyCur;
    s16 keyCount;
    s16 cursor;
    s16 cur2;
    s16 count;
    s16 gate;
    s16 cb4;
    s16 cb6;
} EffectTableView;

/* The same table seen with the pool entries of sefBuildSpecEntries (kept apart: its field types differ). */
#define SEF_POOL_VIEW (*(EffectTableView *)&gSefEffectTable)

/* One 16-byte copy through $8 (lq/sq); the asm text and operands stay as written. */
static inline void CopyVectorAsm(V4 *destination, V4 *source) {
    __asm__ __volatile__("lq $8,0(%5)\n\tsq $8,0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination), "r"(source)
                         : "$8");
}

#define CopyVector(dst, src) CopyVectorAsm(dst, src)
#define CpM(d, s) __asm__ __volatile__("lq $8,0(%1)\n\tlq $9,0x10(%1)\n\tlq $10,0x20(%1)\n\tlq $11,0x30(%1)\n\tsq $8,0(%0)\n\tsq $9,0x10(%0)\n\tsq $10,0x20(%0)\n\tsq $11,0x30(%0)" : : "r"(d), "r"(s) : "$8", "$9", "$10", "$11", "memory")
#define StoreZeroVector(dst) __asm__ __volatile__("sqc2 vf0,0(%0)" : : "r"(dst) : "memory")

typedef struct M4 {
    V4 r[4];
} M4;

typedef struct Cam {
    u8 unmodeled_00[0x170];
    M4 m;
} Cam;

typedef struct Ob2 {
    u8 unmodeled_00[0xB0];
    M4 m;
} Ob2;

extern M4 sefCameraMatrixBlock;
Cam *xglStudioGetActiveCamera(void);
EffectPoolRecord *rotationMatrixToQuaternion(EffectPoolRecord *, M4 *);
void sefGetBattlePoint(V4 *, s32, s32);
void *scGetImmAdrImmIdx2(u8 *tag, void *table, s32 index);
SvTypeListEntry *svGetImageListItem(int type, unsigned int flags);

typedef struct SpecEntry {
    u16 startFrame;
    u16 endFrame;
    u16 length;
    u8 unmodeled_06[2];
    u16 kind : 8;
    u8 unmodeled_09[1];
} SpecEntry;

/* Loads matrix m into pool entry n through rotationMatrixToQuaternion, then copies its row 3 into translation. */
static inline void setMatrix(EffectPoolRecord *entry, M4 *matrix) {
    rotationMatrixToQuaternion(entry, matrix);
    CopyVector(&entry->translation, &matrix->r[3]);
}

/* Pool entry i of the effect table, or 0 when i is out of range. */
static inline EffectPoolRecord *effectPoolEntry(u32 i) {
    if (i < 400) {
        return &SEF_POOL_VIEW.pool[i];
    }
    return 0;
}

typedef struct THdr {
    u32 b5d : 8;
    u32 pad : 24;
    s32 w[4];
} THdr;

/* Builds the pool entries of one effect spec list from the tag image (spec = its SpecEntry table): each entry is claimed
   from the pool, takes its key tables, texture and matrix from the tag, and the slot records its flags. Returns e, or 0
   when no entry was claimed. */
Ent *sefBuildSpecEntries(Ent *effect, s32 effectFlagBits, s32 specWord, u8 *tag, s32 unused, s32 frameBase, f32 scale) {
    SpecEntry *spec = (SpecEntry *) specWord;
    s32 i;
    s32 specKindSum = 0;
    s32 specIndex = 0;

    for (i = 0; spec[specIndex].startFrame < 0x1000 && i < 32; i++) {
        M4 *cameraMatrix = &sefCameraMatrixBlock;
        V4 *cameraPosition = &cameraMatrix->r[3];
        s32 claimedPool = sefEffectPoolClaim();
        s16 claimedPoolSlot = claimedPool;
        EffectPoolRecord *claimedPoolEntry;
        V4 *entryVector;
        u16 *specTable;
        s16 *texInfo;
        s32 mapper;
        s32 psm;
        u64 texRegister;
        SvTypeListEntry *texture;
        SvTypeListEntry *clut;
        s32 usedValue;
        s16 keyCount;
        s16 *keyTable;

        effect->slots[i] = claimedPool;
        if (claimedPoolSlot < 0) {
            return 0;
        }
        claimedPoolEntry = effectPoolEntry(claimedPoolSlot);
        entryVector = &claimedPoolEntry->vector0;
        specTable = scGetImmAdrImmIdx2(tag, &spec[specIndex], 3);
        texInfo = scGetImmAdrImmIdx2(tag, specTable, 4);
        claimedPoolEntry->keys = scGetImmAdrImmIdx2(tag, specTable, 0);
        claimedPoolEntry->unk_38 = scGetImmAdrImmIdx2(tag, specTable, 1);
        claimedPoolEntry->unk_3C = scGetImmAdrImmIdx2(tag, specTable, 2);
        claimedPoolEntry->unk_44 = scGetImmAdrImmIdx2(tag, specTable, 3);
        claimedPoolEntry->unk_48 = scGetImmAdrImmIdx2(tag, specTable, 5);
        claimedPoolEntry->unk_4C = scGetImmAdrImmIdx2(tag, specTable, 6);
        claimedPoolEntry->frameStart = spec[specIndex].startFrame + frameBase;
        claimedPoolEntry->frameEnd = spec[specIndex].endFrame == 0x2000 ? spec[specIndex].endFrame : (s16) spec[specIndex].endFrame + frameBase;
        claimedPoolEntry->frameLength = spec[specIndex].length;
        keyTable = claimedPoolEntry->keys;
        claimedPoolEntry->flags.bits.kind = spec[specIndex].kind;
        claimedPoolEntry->flags.bits.effectFlags = effectFlagBits;
        specKindSum += claimedPoolEntry->flags.bits.kind;
        claimedPoolEntry->flags.bits.track = 0;
        keyCount = keyTable[-1];
        {
            u16 rawKeyCount = ((u16 *) claimedPoolEntry->keys)[-1];

            if (keyCount == 1) {
                usedValue = rawKeyCount;
            } else {
                usedValue = (f32) keyCount * scale;
            }
        }
        claimedPoolEntry->used = usedValue;
        if ((s16) usedValue == 0) {
            claimedPoolEntry->used = 1;
        }
        if (texInfo != 0) {
            mapper = effect->mapper;
            texture = svGetImageListItem(mapper, texInfo[2]);
            clut = svGetImageListItem(mapper, texInfo[3]);
            if (texture != 0 && clut != 0) {
                psm = texture->bitsPerPixel == 8 ? GS_PSMT8 : GS_PSMT4;
                texRegister = GS_SET_TEX0(texture->basePointer, texture->bufferWidthPixels >> 6, psm,
                                   texture->textureWidthExponent, texture->textureHeightExponent, 1, 0,
                                   clut->basePointer, 0, 0, 0, 1);
                claimedPoolEntry->flags.bits.texWidth = texture->width;
                claimedPoolEntry->flags.bits.texHeight = texture->height;
                {
                    claimedPoolEntry->unk_28 = ((f32 *) texInfo)[-4];
                    claimedPoolEntry->unk_2C = ((f32 *) texInfo)[-3];
                    claimedPoolEntry->flags.bits.unk_5D = ((THdr *) texInfo)[-1].b5d;
                    claimedPoolEntry->tex0 = texRegister;
                }
                claimedPoolEntry->flags.packed = (claimedPoolEntry->flags.packed & ~0x10000) | (u64) ((((u8 *) texInfo)[3] >> 5) & 1) << 16;
            }
        }
        claimedPoolEntry->unk_30 = ((f32 *) claimedPoolEntry->keys)[-4];
        claimedPoolEntry->unk_34.asFloat = -claimedPoolEntry->frameLength;
        if (claimedPoolEntry->keys[14] == 0x1000) {
            claimedPoolEntry->flags.packed |= 0x20000;
        }
        if (claimedPoolEntry->unk_44[6] == 0x1000) {
            claimedPoolEntry->flags.packed |= 0x40000;
        }
        if (claimedPoolEntry->unk_48[24] == 0x1000) {
            claimedPoolEntry->flags.packed |= 0x80000;
        }
        if (claimedPoolEntry->unk_4C[6] == 0x1000) {
            claimedPoolEntry->flags.packed |= 0x100000;
        }
        /* keys[-2] is the matrix source: 4 and 5 copy the camera matrix, 6 the active camera, the object list the Ob2 matrix. */
        if (claimedPoolEntry->keys[-2] == 4 || claimedPoolEntry->keys[-2] == 5) {
            rotationMatrixToQuaternion(claimedPoolEntry, cameraMatrix);
            CopyVector(&claimedPoolEntry->translation, cameraPosition);
        } else if (claimedPoolEntry->keys[-2] == 6) {
            setMatrix(claimedPoolEntry, &xglStudioGetActiveCamera()->m);
        } else if (claimedPoolEntry->keys[-2] == 59 || claimedPoolEntry->keys[-2] == 60 || claimedPoolEntry->keys[-2] == 57 || claimedPoolEntry->keys[-2] == 58 ||
                   claimedPoolEntry->keys[-2] == 324) {
            Ob2 *objectMatrix = (Ob2 *) effect->f4;
            if (objectMatrix != 0) {
                M4 objectMatrixCopy;

                CpM(&objectMatrixCopy, &objectMatrix->m);
                rotationMatrixToQuaternion(claimedPoolEntry, &objectMatrixCopy);
                if (claimedPoolEntry->keys[-2] == 324) {
                    sefGetBattlePoint(&claimedPoolEntry->translation, 3, 1);
                    claimedPoolEntry->translation.y = 0.0f;
                } else {
                    CopyVector(&claimedPoolEntry->translation, &objectMatrixCopy.r[3]);
                }
            } else {
                StoreZeroVector(entryVector);
                StoreZeroVector(&claimedPoolEntry->translation);
            }
        }
        effect->active++;
        specIndex++;
        do {
        } while (0);
    }
    /* Bits 10-11 of flags: a spec list whose header word is set and whose id is live gets 0x800; otherwise bit 10 says
       whether any spec kind is set. */
    if (((s16 *) specWord)[-1] != 0 && effect->id >= 0) {
        effect->flags = (effect->flags & ~0xC00) | 0x800;
    } else {
        s32 kindPresent = specKindSum != 0;

        effect->flags = (effect->flags & ~0xC00) | (kindPresent << 10);
    }
    if (effect->active != 0) {
        return effect;
    }
    return 0;
}

/* Copies the vector at s to d through $8 (lq/sq), with the source as a memory input. */
#define CopyQuadMemoryOperand(d, s) __asm__ __volatile__("lq $8,0(%1)\n\tsq $8,0(%0)" : : "r"(d), "r"(s), "m"(*(s)) : "$8", "memory")

/* Stores the zero vector vf0 to d (sqc2). */
#define SefStoreZeroVector(d) __asm__ __volatile__("sqc2 vf0,0(%0)" : : "r"(d) : "memory")

extern Node D_0043A1F0[];

/* Active effect slot with the given index (0..39), or 0 when the slot is free. */
static inline Ent *activeEffectSlot(u32 effectSlotIndex) {
    if (effectSlotIndex < 40 && T.e[effectSlotIndex].active != 0) {
        return &T.e[effectSlotIndex];
    }
    return 0;
}

/* Node with the given index in the node table D_0043A1F0 (900 nodes), or 0 when the index is out of range. */
static inline Node *sefEffectNodeAt(u32 effectNodeIndex) {
    if (effectNodeIndex < 900) {
        return &D_0043A1F0[effectNodeIndex];
    }
    return 0;
}

/* Signature of rotationMatrixToQuaternion: called with a node and a matrix, returns the node. */
typedef Node *(*SefNodeTransformFn)(Node *, M4 *);

/* Sets node n from matrix m: calls rotationMatrixToQuaternion, then copies the matrix translation row (r[3]) into n->b. */
static inline void sefNodeSetMatrix(Node *normal, M4 *matrix) {
    ((SefNodeTransformFn) rotationMatrixToQuaternion)(normal, matrix);
    QuadwordCopyT0(&normal->b, &matrix->r[3]);
}

/* Adds a node from the node table to pool p (pushed on the pool's head). The node takes its transform
   from the pool's kind: the camera matrix, the studio camera, an object (slot index in the low halfword of
   p->fl) or the pool's own two vectors. The flag bits 0x20000..0x100000 clear one halfword each. */
void func_001F62F8(Pool *p) {
    s32 allocatedNodeIndex = sefEffectNodeClaim();

    if (allocatedNodeIndex >= 0) {
        Node *attachedNode = sefEffectNodeAt(allocatedNodeIndex);
        V4 *nodeFirstVector = &attachedNode->a;
        Node *oldHead;
        f32 scale;

        memset(attachedNode, 0, 96);
        oldHead = p->head.p;
        scale = p->v[3].x;
        attachedNode->next.p = oldHead;
        attachedNode->idx.v = allocatedNodeIndex;
        attachedNode->f40.f = scale;
        p->head.p = attachedNode;
        attachedNode->used = p;
        if (p->fl & 0x20000) {
            attachedNode->nu.h[0] = -1;
        }
        if (p->fl & 0x40000) {
            attachedNode->nu.h[1] = -1;
        }
        if (p->fl & 0x80000) {
            attachedNode->nu.h[2] = -1;
        }
        if (p->fl & 0x100000) {
            attachedNode->nu.h[3] = -1;
        }
        if (p->ptr[-2] == 5) {
            sefNodeSetMatrix(attachedNode, &sefCameraMatrixBlock);
        } else if (p->ptr[-2] == 6) {
            sefNodeSetMatrix(attachedNode, &xglStudioGetActiveCamera()->m);
        } else if (p->ptr[-2] == 0x3C || p->ptr[-2] == 0x3A || p->ptr[-2] == 0x144) {
            Ob2 *effectObjectRecord = (Ob2 *) activeEffectSlot(*(s16 *) &p->fl)->f4;

            if (effectObjectRecord != 0) {
                M4 matrix;

                CpM(&matrix, &effectObjectRecord->m);
                ((SefNodeTransformFn) rotationMatrixToQuaternion)(attachedNode, &matrix);
                if (p->ptr[-2] == 0x144) {
                    sefGetBattlePoint(&attachedNode->b, 3, 1);
                    attachedNode->b.y = 0.0f;
                } else {
                    QuadwordCopyT0(&attachedNode->b, &matrix.r[3]);
                }
            } else {
                SefStoreZeroVector(nodeFirstVector);
                SefStoreZeroVector(&attachedNode->b);
            }
        } else {
            CopyQuadMemoryOperand(&attachedNode->a, &p->v[0]);
            QuadwordCopyT0(&attachedNode->b, &p->v[1]);
        }
        sefTakeKeys(attachedNode);
    }
}

/* Effect slot as func_001F6590 uses it: SefSlot (region 1) leaves the word at +0x08 as bytes; the
   s32 field pairExtra is kept apart because the field type decides the emitted order. */
typedef struct SefEffectSlot {
    s32 active;
    s32 slotValue;
    s32 pairExtra;
    s32 unmodeled_0C;
    f32 weight;
    s32 mapValue;
    s16 poolSlots[32];
    s16 key;
    s16 index;
    s16 mapIndex;
    s8 flag;
    s8 result;
    SefFlagWord u;
} SefEffectSlot;

/* The effect slot table at gSefEffectTable as the burst and slot code sees it (slots only). */
typedef struct SefEffectSlotTable {
    unsigned char unmodeled_00[0x47180];
    SefEffectSlot slots[40];
} SefEffectSlotTable;

#define gEffectSlots (*(SefEffectSlotTable *) gSefEffectTable)

/* The two words a caller passes with a slot: slotValue goes to the slot, slotExtra to the word at +0x08. */
typedef struct SefEffectSlotWords {
    s32 slotValue;
    s32 slotExtra;
} SefEffectSlotWords;

/* Object record view: the kind byte sits at +0x16. */
typedef struct SefEffectObjectKind {
    unsigned char unmodeled_00[0x16];
    unsigned char objectKind;
} SefEffectObjectKind;

/* Claims an effect slot for key and resourceId, fills it from the object record and the two words, and
   releases the slot again when sefBuildSpecEntries cannot set it up. Returns the slot index or -1. */
s32 func_001F6590(s32 specWord, unsigned char *effectObjectRecord, s32 key, s32 resourceId,
                  SefEffectSlotWords *effectSlotWordPair) {
    s32 effectSlotIndex = sefClaimFirstSlot(key, resourceId);

    if (effectSlotIndex >= 0) {
        gEffectSlots.slots[effectSlotIndex].u.bits.lo = ((SefEffectObjectKind *) effectObjectRecord)->objectKind;
        gEffectSlots.slots[effectSlotIndex].slotValue = effectSlotWordPair->slotValue;
        gEffectSlots.slots[effectSlotIndex].pairExtra = effectSlotWordPair->slotExtra;
        if (sefBuildSpecEntries(&gEffectSlots.slots[effectSlotIndex], effectSlotIndex, specWord, effectObjectRecord,
                          resourceId, 0, 1.0f) != 0) {
            return effectSlotIndex;
        }
        sefEffectInstanceRelease(&gEffectSlots.slots[effectSlotIndex]);
    }
    return -1;
}

/* One 0x40-byte entry of the effect record table gSefEffectTransforms: three matrix rows, then the unit value,
   the group index and the resource id (unk_30 and unmodeled_3C are not read here). */
typedef struct SefEffectRecordEntry {
    AlignedVector4 r[3];
    f32 unk_30;
    s32 groupIndex;
    s32 resourceId;
    s32 unmodeled_3C;
} SefEffectRecordEntry;

extern SefEffectRecordEntry D_004516B0_1F6698[];

/* A 4x4 matrix as four AlignedVector4 rows. */
typedef struct SefMatrix {
    AlignedVector4 r[4];
} SefMatrix;


/* Request for a burst of effect spawns: resource, spawn and group counts, yaw and the burst volume. */
typedef struct SefBurstRequest {
    s32 resourceId;           /* +0x00 */
    s32 spawnCount;           /* +0x04 */
    s32 groupCount;           /* +0x08 */
    f32 yaw;                  /* +0x0C */
    AlignedVector4 position;  /* +0x10 */
    AlignedVector4 direction; /* +0x20 */
    AlignedVector4 volumeSize; /* +0x30 */
} SefBurstRequest;

typedef struct SefBurstKey {
    unsigned char unmodeled_00[0x18];
    s32 resourceId; /* +0x18 */
    f32 scale;      /* +0x1C */
    unsigned char unmodeled_20[0x10];
} SefBurstKey;

typedef struct SefBurstScheduler {
    unsigned char unmodeled_00[8];
    s16 referenceIndex; /* +0x08 */
    unsigned char unmodeled_0A[6];
    AlignedVector4 position; /* +0x10 */
    AlignedVector4 unk_20;   /* +0x20 */
    AlignedVector4 direction; /* +0x30 */
    unsigned char unmodeled_40[0x150 - 0x40];
} SefBurstScheduler;

typedef struct SefBurstRefCount {
    unsigned char unmodeled_00[8];
    u16 referenceCount; /* +0x08 */
    unsigned char unmodeled_0A[0x13DC - 10];
} SefBurstRefCount;

extern SefBurstRefCount sefReferenceCounts[];
extern s32 sefOutputLinkEnabled;
extern const char D_0053EEC8[];
s16 *scScriptLookup(s32, s32 *, s32, s32 *);
s32 sefCreateScheduler(s16 *, SefBurstKey *, s32, s16 *, s32);
SefBurstScheduler *sefGetScheduler(void);
void LOG(const void *loggedVector, const char *format, ...);
void vu0SetCurrentRotationY(f32 angle);
u32 MMathRandomNext(void);
f32 sefRandf(void);
void sefNoiseNextVector(AlignedVector4 *vector);
void *MMathApplyMatrix(void *destination, void *matrix, void *vector);

/* Copies the current rotation matrix (vf26..vf29) into matrix. */
static inline void sefGetCurrentRotation(SefMatrix *matrix) {
    __asm__ __volatile__("sqc2 vf26, 0(%0)\n"
                         "sqc2 vf27, 16(%0)\n"
                         "sqc2 vf28, 32(%0)\n"
                         "sqc2 vf29, 48(%0)\n"
                         :
                         : "r"(matrix)
                         : "memory");
}

static inline void sefIncrementRef(s32 referenceIndex) {
    SefBurstRefCount *referenceSlot = &sefReferenceCounts[referenceIndex];
    referenceSlot->referenceCount++;
}

static inline void sefMulXYZ(AlignedVector4 *product, AlignedVector4 *factorA, AlignedVector4 *factorB) {
    __asm__ __volatile__("lqc2 vf1, 0(%5)\n"
                         "lqc2 vf2, 0(%6)\n"
                         "vmul.xyz vf1, vf1, vf2\n"
                         "sqc2 vf1, 0(%4)\n"
                         : "=m"(product->x), "=m"(product->y), "=m"(product->z),
                           "=m"(product->w)
                         : "r"(product), "r"(factorA), "r"(factorB)
                         : "memory");
}

static inline void sefAddXYZ(AlignedVector4 *vectorSum, AlignedVector4 *addendA, AlignedVector4 *addendB) {
    __asm__ __volatile__("lqc2 vf1, 0(%4)\n"
                         "lqc2 vf2, 0(%5)\n"
                         "vadd.xyz vf1, vf1, vf2\n"
                         "sqc2 vf1, 0(%3)\n"
                         : "=m"(vectorSum->x), "=m"(vectorSum->y), "=m"(vectorSum->z)
                         : "r"(vectorSum), "r"(addendA), "r"(addendB)
                         : "memory");
}

static inline void sefVectorSetOne(AlignedVector4 *vector) {
    __asm__ __volatile__("vmaxw.xyzw vf1, vf0, vf0w\n\tsqc2 vf1, 0(%0)" : : "r"(vector) : "memory");
}

/* Float at byte bytesIntoRecord of record recordIndex; the byte form decides the emitted address (kept). */
static inline f32 *sefEffectRecordFloat(s32 effectRecordIndex, s32 bytesIntoRecord) {
    s32 effectRecordByteOffset = effectRecordIndex * sizeof(SefEffectRecordEntry) + bytesIntoRecord;
    return (f32 *) ((char *) D_004516B0_1F6698 + effectRecordByteOffset);
}

static inline void sefCopyVector(AlignedVector4 *copyVectorDest, AlignedVector4 *copyVectorSource) {
    __asm__ __volatile__("lq $8, 0(%2)\n"
                         "sq $8, 0(%1)\n"
                         : "=m"(*copyVectorDest)
                         : "r"(copyVectorDest), "r"(copyVectorSource)
                         : "$8", "memory");
}

/* Setup routine of an effect slot (sefBuildSpecEntries) with its resource table as the tag argument. */
typedef SefEffectSlot *(*SefSlotSetup)(SefEffectSlot *, s32, s32, s16 *, s32, s32, f32);

s32 func_001F6698(SefBurstRequest *burstRequest) {
    SefBurstKey key;
    SefMatrix rotation;
    s8 cellUse[27];
    s8 groupSpawns[40];
    AlignedVector4 scatterOffset;
    s32 resourceTableLength;
    s32 burstGroupIndex;
    s32 resourceId;
    s16 *resourceTable;
    s32 burstGroupCount;
    s32 emittedCount;
    AlignedVector4 *volume;
    s8 *groupCursor;
    s32 spawnLimit;
    s32 schedIndex;
    f32 scale;
    s32 countDelta;
    s32 spawnIndex;
    s32 slotId;
    s32 groupPick;
    s32 cellIndex;
    s32 tryIndex;
    s32 effectRecordIndex;
    SefEffectSlot *burstSetupSlot;
    SefBurstScheduler *scheduler;
    f32 meanExtent;
    f32 minExtent;

    scale = 1.0f;
    resourceId = burstRequest->resourceId;
    resourceTable = scScriptLookup(resourceId, &resourceTableLength, 0, 0);
    if (resourceTable != 0) {
        spawnLimit = burstRequest->spawnCount;
        if (spawnLimit > 40) {
            spawnLimit = 40;
        }
        if (resourceTableLength != 0) {
            memset(&key, 0, sizeof(key));
            key.resourceId = resourceId;
            key.scale = scale;
            /* Start the burst's scheduler and give it the request's position and direction. */
            schedIndex = sefCreateScheduler(resourceTable + resourceTableLength, &key, 0, resourceTable, -1);
            if (schedIndex >= 0) {
                scheduler = &sefGetScheduler()[schedIndex];
                sefCopyVector(&scheduler->position, &burstRequest->position);
                sefCopyVector(&scheduler->direction, &burstRequest->direction);
                sefIncrementRef(scheduler->referenceIndex);
            }
            if (sefOutputLinkEnabled == 0) {
                return 0;
            }
        } else if (sefOutputLinkEnabled == 0) {
            return -1;
        }
        volume = &burstRequest->volumeSize;
        /* Burst scale comes from the volume: its mean extent when moderate, else its smallest side or half the spawn limit, clamped to [0.9, 8]. */
        meanExtent = (volume->x + volume->y + volume->z) / 3.0f;
        minExtent = volume->x < volume->y ? volume->x : volume->y;
        minExtent = minExtent < volume->z ? minExtent : volume->z;
        if (1.0f < meanExtent && meanExtent < 6.0f) {
            scale = meanExtent / 2.5f;
        } else if (minExtent < (f32) spawnLimit) {
            scale = minExtent;
        } else {
            scale = (f32) (spawnLimit / 2);
        }
        if (8.0f < scale) {
            scale = 8.0f;
        }
        if (scale < 0.9f) {
            scale = 0.9f;
        }
        memset(cellUse, 0, sizeof(cellUse));
        vu0SetCurrentRotationY(burstRequest->yaw);
        emittedCount = 0;
        sefGetCurrentRotation(&rotation);
        /* Resource kind 100 spawns into several groups; every other kind is one groupPick. */
        if (((burstRequest->resourceId >> 12) & 0x3FF) == 100) {
            LOG(volume, D_0053EEC8, scale);
            for (spawnIndex = 0; spawnIndex < burstRequest->groupCount; spawnIndex++) {
                groupSpawns[spawnIndex] = 1;
            }
            if (burstRequest->spawnCount >= burstRequest->groupCount) {
                countDelta = burstRequest->spawnCount - burstRequest->groupCount;
                for (spawnIndex = 0; spawnIndex < countDelta; spawnIndex++) {
                    groupSpawns[MMathRandomNext() % burstRequest->groupCount] += 1;
                }
            } else {
                countDelta = burstRequest->groupCount - burstRequest->spawnCount;
                while (countDelta > 0) {
                    groupPick = MMathRandomNext() % (burstRequest->groupCount - 1) + 1;
                    if (groupSpawns[groupPick] > 0) {
                        groupSpawns[groupPick] -= 1;
                        countDelta--;
                    }
                }
            }
            burstGroupCount = burstRequest->groupCount;
        } else {
            burstGroupCount = 1;
            groupSpawns[0] = 1;
        }
        burstGroupIndex = 0;
        if (burstGroupCount > 0) {
            groupCursor = groupSpawns;
            do {
                spawnIndex = 0;
                if (*groupCursor > 0) {
                    do {
                        slotId = sefClaimFirstSlot(-1, resourceId);
                        emittedCount++;
                        if (slotId < 0) {
                            break;
                        }
                        burstSetupSlot = ((SefSlotSetup) sefBuildSpecEntries)(
                            &gEffectSlots.slots[slotId], slotId,
                            sefOutputLinkEnabled, resourceTable, resourceId, burstGroupIndex, 1.0f);
                        if (burstSetupSlot != 0) {
                            effectRecordIndex = gEffectSlots.slots[slotId].mapIndex;
                            burstSetupSlot->weight = sefRandf() * 0.1f + 0.9f;
                            /* The first record sits at the burst position (raised by half the volume height); later ones are scattered. */
                            if (emittedCount == 1) {
                                *sefEffectRecordFloat(effectRecordIndex, 0) = burstRequest->position.x;
                                *sefEffectRecordFloat(effectRecordIndex, 4) =
                                    burstRequest->position.y + burstRequest->volumeSize.y * 0.5f;
                                *sefEffectRecordFloat(effectRecordIndex, 8) = burstRequest->position.z;
                                *sefEffectRecordFloat(effectRecordIndex, 12) = 1.0f;
                                cellUse[13] = 1;
                            } else {
                                /* Draw up to five random offsets in the 3x3x3 cell grid and stop at the first free cell. */
                                for (tryIndex = 0; tryIndex < 5; tryIndex++) {
                                    sefNoiseNextVector(&scatterOffset);
                                    scatterOffset.y = scatterOffset.y + 0.5f;
                                    cellIndex = (s32) (scatterOffset.y * 3.0f) * 3 +
                                                (s32) ((scatterOffset.x + 0.5f) * 3.0f) * 9 +
                                                (s32) ((scatterOffset.z + 0.5f) * 3.0f);
                                    if (cellUse[cellIndex] == 0) {
                                        break;
                                    }
                                }
                                cellUse[cellIndex] += 1;
                                sefMulXYZ(&scatterOffset, &scatterOffset, volume);
                                MMathApplyMatrix(&scatterOffset, &rotation, &scatterOffset);
                                sefAddXYZ(&D_004516B0_1F6698[effectRecordIndex].r[0], &burstRequest->position, &scatterOffset);
                            }
                            QuadwordCopyT0(&D_004516B0_1F6698[effectRecordIndex].r[1], &burstRequest->direction);
                            Vu0SefScaleVectorXYZ(&D_004516B0_1F6698[effectRecordIndex].r[1],
                                                 &D_004516B0_1F6698[effectRecordIndex].r[1],
                                                 scale * (sefRandf() * 0.4f + 0.6f));
                            sefVectorSetOne(&D_004516B0_1F6698[effectRecordIndex].r[2]);
                            D_004516B0_1F6698[effectRecordIndex].unk_30 = 1.0f;
                            D_004516B0_1F6698[effectRecordIndex].groupIndex = burstGroupIndex;
                            D_004516B0_1F6698[effectRecordIndex].resourceId = burstRequest->resourceId;
                            svResourceTypeLists[burstSetupSlot->result].s.refs++;
                        } else {
                            /* No burst entry could be set up: release the slot just taken. */
                            sefEffectInstanceRelease(&gEffectSlots.slots[slotId]);
                            break;
                        }
                        spawnIndex++;
                    } while (spawnIndex < *groupCursor);
                }
                burstGroupIndex++;
                groupCursor++;
            } while (burstGroupIndex < burstGroupCount);
        }
        return 0;
    }
    return -1;
}

extern unsigned char gEffectTable[];

/* Zeroes the first 0xC80 bytes of the effect table gEffectTable. */
void clearEffectTable(void) {
    memset(gEffectTable, 0, 0xC80);
}

typedef struct SefRecordEntry {
    V4 vectors[3];
    f32 x;
    s32 y;
    s32 z;
    s32 w;
} SefRecordEntry;

extern SefRecordEntry D_004516B0_1F6DE0[];

typedef struct EffectRecordHeader {
    f32 x;
    s32 y;
    s32 z;
    s32 entryIndex;
    s32 timeMs;
    s32 pad[3];
} EffectRecordHeader;

typedef struct EffectTableEntry {
    V4 vectors[3];
    EffectRecordHeader header;
} EffectTableEntry;

extern EffectTableEntry gEffectTable_1F6DE0[];

#define copyQuadwordAsm(d, s) __asm__ __volatile__("lq $8,0(%1)\n\tsq $8,0(%0)" : : "r"(d), "r"(s) : "$8", "memory")

/* Compacts the records of the active effect slots, in slot order, into gEffectTable_1F6DE0.
   Each compacted entry keeps the slot's three vectors, its record position and its index and frame time. */
void sefCompactActiveEffectRecords(void) {
    s32 i;
    s32 compactIndex = 0;
    s32 activeCount = T.gate;

    if (activeCount == 0) {
        return;
    }
    for (i = 0; i < 40 && activeCount > 0; i++) {
        if (T.e[i].active != 0) {
            s32 recordIndex = T.e[i].slot;

            if (recordIndex >= 0) {
                copyQuadwordAsm(&gEffectTable_1F6DE0[compactIndex].vectors[0], &D_004516B0_1F6DE0[recordIndex].vectors[0]);
                copyQuadwordAsm(&gEffectTable_1F6DE0[compactIndex].vectors[1], &D_004516B0_1F6DE0[recordIndex].vectors[1]);
                copyQuadwordAsm(&gEffectTable_1F6DE0[compactIndex].vectors[2], &D_004516B0_1F6DE0[recordIndex].vectors[2]);
                gEffectTable_1F6DE0[compactIndex].header.x = D_004516B0_1F6DE0[recordIndex].x;
                gEffectTable_1F6DE0[compactIndex].header.y = D_004516B0_1F6DE0[recordIndex].y;
                gEffectTable_1F6DE0[compactIndex].header.z = D_004516B0_1F6DE0[recordIndex].z;
                gEffectTable_1F6DE0[compactIndex].header.entryIndex = T.e[i].idx;
                gEffectTable_1F6DE0[compactIndex].header.timeMs = T.e[i].fC;
                compactIndex++;
            }
        }
    }
}

extern SefRecordEntry D_004516B0_1F6EE8[];


extern TypeList svResourceTypeLists_1F6EE8[];
void sefRunEffectFrames(Ent *, f32, f32);

/* Returns entry i of the effect entry table when it is in range and active, else 0. */
static inline Ent *getActiveEntry(u32 i) {
    if (i < 40 && T.e[i].active != 0) {
        return &T.e[i];
    }
    return 0;
}

/* Runs each compacted effect entry: spawns it through sefBuildSpecEntries, copies its vectors and position back into
   the record table, takes a reference on its mapper and starts its frames. */
void sefRunCompactedEffects(void) {
    s32 entryIndex;
    EffectTableEntry *effectTable = (*(EffectTableEntry(*)[]) & gEffectTable);
    s32 tableLength;
    u8 *tag = 0;
    s32 prevKey = 0;

    for (entryIndex = 0; entryIndex < 40; entryIndex++) {
        EffectRecordHeader *header = &effectTable[entryIndex].header;
        s32 key;

        if (((f32 *) header)[-9] == 0.0f) {
            return;
        }
        key = header->z;
        if (key != prevKey) {
            tag = ((u8 * (*) (s32, s32 *, s32, s32)) scScriptLookup)(key, &tableLength, 0, 0);
        }
        if (tag != 0) {
            s32 slotIndex;
            Ent *spawned;

            if (sefOutputLinkEnabled == 0) {
                return;
            }
            slotIndex = sefClaimSlotAt(header->entryIndex, -1, key);
            if (slotIndex < 0) {
                return;
            }
            spawned = sefBuildSpecEntries(&T.e[slotIndex], slotIndex, sefOutputLinkEnabled, tag, key, header->y, header->x);
            if (spawned != 0) {
                s32 recordIndex = T.e[slotIndex].slot;

                copyQuadwordAsm(&D_004516B0_1F6EE8[recordIndex].vectors[0], &(*(EffectTableEntry(*)[]) & gEffectTable)[entryIndex].vectors[0]);
                copyQuadwordAsm(&D_004516B0_1F6EE8[recordIndex].vectors[1], &(*(EffectTableEntry(*)[]) & gEffectTable)[entryIndex].vectors[1]);
                copyQuadwordAsm(&D_004516B0_1F6EE8[recordIndex].vectors[2], &(*(EffectTableEntry(*)[]) & gEffectTable)[entryIndex].vectors[2]);
                D_004516B0_1F6EE8[recordIndex].x = header->x;
                D_004516B0_1F6EE8[recordIndex].y = header->y;
                D_004516B0_1F6EE8[recordIndex].z = header->z;
                svResourceTypeLists_1F6EE8[spawned->mapper].s.refs++;
                sefRunEffectFrames(getActiveEntry(slotIndex), (f32) header->timeMs / 1000.0f, 1.0f);
            } else {
                sefEffectInstanceRelease(&T.e[slotIndex]);
                return;
            }
        }
        prevKey = key;
    }
}

/* The effect slot table at gSefEffectTable viewed through its 0x47180 slot array and gate. */
#define gSlotTableFull (*(SefSlotTableFull *) &gSefEffectTable)
/* The same object viewed through its keyed node, slot and ring fields. */
#define gSlotTableKeyed (*(SefSlotTableKeyed(*)[]) & gSefEffectTable)

typedef union SefFlagWordBits {
    s32 flags;
    struct {
        signed char lowByte : 8;
        unsigned int bit8 : 1;
        unsigned int bit9 : 1;
        unsigned int bit10 : 2;
        unsigned int highBits : 20;
    } bits;
} SefFlagWordBits;

typedef struct SefSlotFull {
    s32 active;
    s32 value;
    s32 unk_08;
    s32 unk_0C;
    f32 weight;
    s32 mapValue;
    s16 poolSlots[32];
    s16 key;
    s16 index;
    s16 mapIndex;
    s8 flag;
    s8 result;
    s32 flags;
} SefSlotFull;

typedef struct SefSlotTableFull {
    unsigned char unmodeled_00[0x27150];
    SefPoolEntry pool[400];
    unsigned char unmodeled_32050[0x15130];
    SefSlotFull slots[40];
    unsigned char unmodeled_48120[0x138C];
    s16 cursor;
    s16 cursor2;
    s16 count;
    s16 gate;
} SefSlotTableFull;

typedef struct SefSlotBits {
    s32 active;
    s32 value;
    s32 unk_08;
    s32 unk_0C;
    f32 weight;
    s32 mapValue;
    s16 poolSlots[32];
    s16 key;
    s16 index;
    s16 mapIndex;
    s8 flag;
    s8 result;
    SefFlagWordBits flagWord;
} SefSlotBits;

typedef struct SefSlotTableKeyed {
    SefKey keys[5000];
    SefPool pool[400];
    SefEffectNode node[900];
    SefSlotBits slots[40];
    s8 keyUsed[5000];
    s16 ringStart;
    s16 ringCount;
    s16 cursor;
    s16 cursor2;
    s16 count;
    s16 gate;
    s16 cb4;
    s16 cb6;
} SefSlotTableKeyed;

/* Returns slot i when it is in range and its flag byte is set, else 0. */
static inline SefSlotBits *getActiveSlot(u32 i) {
    if (i < 40 && gSlotTableKeyed->slots[i].flag != 0) {
        return &gSlotTableKeyed->slots[i];
    }
    return 0;
}

/* Releases every active slot whose key is id. A slot whose low flag byte is not 56 is only marked
   (bit 10) unless force is set. The value 56 is the original test; its meaning is unproven. */
void sefReleaseSlotsByKey(s32 id, s32 force) {
    s32 i;

    if (gSlotTableKeyed->gate != 0) {
        for (i = 0; i < 40; i++) {
            SefSlotBits *slot = getActiveSlot(i);

            if (slot != 0 && slot->key == id) {
                if (slot->flagWord.bits.lowByte == 56 || force != 0) {
                    sefEffectInstanceRelease(slot);
                } else {
                    slot->flagWord.bits.bit10 = 1;
                }
            }
        }
    }
}

static inline SefSlotFull *findSlotByKey(s32 key) {
    s32 i;

    if (gSlotTableFull.gate != 0) {
        for (i = 0; i < 40; i++) {
            if (gSlotTableFull.slots[i].key == key) {
                if (gSlotTableFull.slots[i].flag != 0) {
                    return &gSlotTableFull.slots[i];
                }
            }
        }
    }
    return 0;
}

/* Stores value in the first active slot whose key is key. */
void sefSetSlotValueByKey(s32 key, s32 value) {
    SefSlotFull *slot = findSlotByKey(key);

    if (slot != 0) {
        slot->value = value;
    }
}

/* Returns the first active slot of the table whose key is id, else 0. */
static inline SefSlotFull *findActiveSlotById(s32 id) {
    s32 i;

    if (gSlotTableFull.gate != 0) {
        for (i = 0; i < 40; i++) {
            if (gSlotTableFull.slots[i].key == id &&
                gSlotTableFull.slots[i].flag != 0) {
                return &gSlotTableFull.slots[i];
            }
        }
    }
    return 0;
}

/* Stores value in the active entry whose id is id (the field at +0x08). */
void sefSetEntryValueById(s32 id, s32 value) {
    SefSlotFull *slot = findActiveSlotById(id);

    if (slot != 0) {
        slot->unk_08 = value;
    }
}

/* Stores value as the weight (+0x10) of the active entry whose id is id. */
void sefSetEntryWeightById(s32 id, f32 value) {
    SefSlotFull *slot = findActiveSlotById(id);

    if (slot != 0) {
        slot->weight = value;
    }
}

typedef struct SefRecordQuad {
    f32 pos[4];
    f32 vectorQ[4];
    f32 vectorR[4];
    f32 vectorS[4];
} SefRecordQuad;

extern SefRecordQuad D_004516B0_1F73C8[];

static inline SefSlotFull *findSlotByKeyView(s32 key) {
    s32 i;

    if (gSlotTableFull.gate != 0) {
        for (i = 0; i < 40; i++) {
            if (gSlotTableFull.slots[i].key == key) {
                if (gSlotTableFull.slots[i].flag != 0) {
                    return &gSlotTableFull.slots[i];
                }
            }
        }
    }
    return 0;
}

/* Copies one quadword (16 bytes) with lq/sq through $8; same operation as copyQuadwordAsm. */
static inline void copyQuadword(void *destination, const void *source) {
    __asm__ __volatile__("lq $8, 0(%1)\n\tsq $8, 0(%0)" : : "r"(destination), "r"(source) : "$8", "memory");
}

static inline SefRecordQuad *recordAt(u32 index) {
    if (index < 40 && D_004516B0_1F73C8[index].pos[3] != 0.0f) {
        return &D_004516B0_1F73C8[index];
    }
    return 0;
}

/* Copies the 16-byte vector source into the pos quadword of the record the slot with key maps to. */
void sefSetSlotRecordPos(s32 key, f32 *source) {
    SefRecordQuad *record;
    SefSlotFull *slot = findSlotByKeyView(key);

    if (slot != 0) {
        record = recordAt(slot->mapIndex);
        if (record != 0) {
            copyQuadword(record->pos, source);
        }
    }
}

extern SefRecordQuad D_004516B0_1F74A8[];

static inline SefSlotFull *findSlotByKeyAlt(s32 key) {
    s32 i;

    if (gSlotTableFull.gate != 0) {
        for (i = 0; i < 40; i++) {
            if (gSlotTableFull.slots[i].key == key) {
                if (gSlotTableFull.slots[i].flag != 0) {
                    return &gSlotTableFull.slots[i];
                }
            }
        }
    }
    return 0;
}

static inline SefRecordQuad *recordAtView(u32 index) {
    if (index < 40 && D_004516B0_1F74A8[index].pos[3] != 0.0f) {
        return &D_004516B0_1F74A8[index];
    }
    return 0;
}

/* Copies the 16-byte vector source into the vectorQ quadword of the record the slot with key maps to. */
void sefSetSlotRecordVectorQ(s32 key, f32 *source) {
    SefRecordQuad *record;
    SefSlotFull *slot = findSlotByKeyAlt(key);

    if (slot != 0) {
        record = recordAtView(slot->mapIndex);
        if (record != 0) {
            copyQuadword(record->vectorQ, source);
        }
    }
}

extern SefRecordQuad D_004516B0_1F7588[];

static inline SefSlotFull *findSlotByKeyLocal(s32 key) {
    s32 i;

    if (gSlotTableFull.gate != 0) {
        for (i = 0; i < 40; i++) {
            if (gSlotTableFull.slots[i].key == key) {
                if (gSlotTableFull.slots[i].flag != 0) {
                    return &gSlotTableFull.slots[i];
                }
            }
        }
    }
    return 0;
}

static inline SefRecordQuad *recordAtAlt(u32 index) {
    if (index < 40 && D_004516B0_1F7588[index].pos[3] != 0.0f) {
        return &D_004516B0_1F7588[index];
    }
    return 0;
}

/* Copies the 16-byte vector source into the vectorR quadword of the record the slot with key maps to. */
void sefSetSlotRecordVectorR(s32 key, f32 *source) {
    SefRecordQuad *record;
    SefSlotFull *slot = findSlotByKeyLocal(key);

    if (slot != 0) {
        record = recordAtAlt(slot->mapIndex);
        if (record != 0) {
            copyQuadword(record->vectorR, source);
        }
    }
}

extern SefRecordQuad D_004516B0_1F7668[];

static inline SefSlotFull *findSlotByKeyOther(s32 key) {
    s32 i;

    if (gSlotTableFull.gate != 0) {
        for (i = 0; i < 40; i++) {
            if (gSlotTableFull.slots[i].key == key) {
                if (gSlotTableFull.slots[i].flag != 0) {
                    return &gSlotTableFull.slots[i];
                }
            }
        }
    }
    return 0;
}

static inline SefRecordQuad *recordAtLocal(u32 index) {
    if (index < 40 && D_004516B0_1F7668[index].pos[3] != 0.0f) {
        return &D_004516B0_1F7668[index];
    }
    return 0;
}

/* Stores value in element 3 of vectorR, the record the slot with key maps to. */
void sefSetSlotRecordVectorRW(s32 key, f32 value) {
    SefRecordQuad *record;
    SefSlotFull *slot = findSlotByKeyOther(key);

    if (slot != 0) {
        record = recordAtLocal(slot->mapIndex);
        if (record != 0) {
            record->vectorR[3] = value;
        }
    }
}

static inline SefSlotFull *findSlotByKeySecond(s32 key) {
    s32 i;

    if (gSlotTableFull.gate != 0) {
        for (i = 0; i < 40; i++) {
            if (gSlotTableFull.slots[i].key == key) {
                if (gSlotTableFull.slots[i].flag != 0) {
                    return &gSlotTableFull.slots[i];
                }
            }
        }
    }
    return 0;
}

/* Sets bit 8 of the flags word of the slot with key to the low bit of value. */
void func_001F7740(s32 key, s32 value) {
    SefSlotFull *slot = findSlotByKeySecond(key);

    if (slot != 0) {
        slot->flags = (slot->flags & ~0x100) | ((value & 1) << 8);
    }
}

static inline SefSlotFull *findSlotByKeyThird(s32 key) {
    s32 i;

    if (gSlotTableFull.gate != 0) {
        for (i = 0; i < 40; i++) {
            if (gSlotTableFull.slots[i].key == key) {
                if (gSlotTableFull.slots[i].flag != 0) {
                    return &gSlotTableFull.slots[i];
                }
            }
        }
    }
    return 0;
}

/* Sets bit 9 of the flags word of the slot with key to the low bit of value. */
void func_001F77D8(s32 key, s32 value) {
    SefSlotFull *slot = findSlotByKeyThird(key);

    if (slot != 0) {
        slot->flags = (slot->flags & ~0x200) | ((value & 1) << 9);
    }
}

static inline SefSlotFull *findSlotByKeyFourth(s32 key) {
    s32 i;

    if (gSlotTableFull.gate != 0) {
        for (i = 0; i < 40; i++) {
            if (gSlotTableFull.slots[i].key == key) {
                if (gSlotTableFull.slots[i].flag != 0) {
                    return &gSlotTableFull.slots[i];
                }
            }
        }
    }
    return 0;
}

/* Returns objectHasFlag5E of the slot that key selects. */
s32 func_001F7870(s32 key) {
    return objectHasFlag5E(findSlotByKeyFourth(key));
}

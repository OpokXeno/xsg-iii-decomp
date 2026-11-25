#include "common.h"
#include "shared.h"
#include "main/tu141.h"
#include "main/Packet.h"
#include "main/sv_get_image_item_142.h"
#include "main/sef_135.h"
#include "main/tu045.h"
#include "main/tu122.h"
#include "main/tu124.h"
#include "main/tu132.h"
#include "main/xgl_2.h"
#include "vu0_macro.h"
#include "quadword.h"

/* One value of the scratchpad block at 0x70000000 (the effect work area). */
#define SV_SCRATCHPAD_SORT_KEYS ((SvLineSortKey *) 0x70000000)

/* >>> CHUNK sef_light_set */
struct XglLightSet;
void *sefLightSetActiveGet(void);
/* A vector kept as a plain float[4] (like sceVu0FVECTOR); its w is element 3. Vector4 here changes the
 * address arithmetic of the w stores, so the casts to Vector4 * at the calls stay. */
/* 16-byte aligned in the original (its copies use ld/sd); the getters below only take its address. */
extern XglLightSet sefActiveLightSet;

/*
 * Three 0x180-byte blocks (sefActiveLightSet, sefDefaultLightSet and sefAppliedLightSet) are moved
 * by sefLightSetsResetDefault and sefLightSetsApply. The getters below return two of them;
 * sefSetGateBlocks (sef.c) calls both, sefRefreshEntryRows (sef.c) the first.
 */
void *sefLightSetActiveGet(void)
{
    return &sefActiveLightSet;
}

/* Default-reset routine of tu030 (asm); takes the light set to reset. */
void func_001975C8(XglLightSet *lightSet);
extern XglLightSet D_0076D4E0;

void sefLightSetsResetDefault(void)
{
    func_001975C8(&sefActiveLightSet);
    D_0076D4E0 = sefActiveLightSet;
}

/* Block copy of one light set through a view of align 4: the original's source pointer has no 16-byte alignment. */
typedef struct LightSetWords {
    float words[96];
} LightSetWords;

void sefLightSetsApply(XglLightSet *lights)
{
    *(LightSetWords *)&D_0076D4E0 = *(LightSetWords *)lights;
    *(LightSetWords *)&sefActiveLightSet = *(LightSetWords *)lights;
}

extern int svDepthNodeCount;
typedef struct SvDepthNode {
    s16 next;
    s16 itemId;
    s16 itemFlags;
    s16 tail;
    f32 depth;
} SvDepthNode;
extern SvDepthNode svDepthBuckets[40];
extern SvDepthNode svDepthNodes[1100];
extern void *memset(void *destination, int value, unsigned int count);

void svSortReset(void)
{
    svDepthNodeCount = 0;
    memset(svDepthBuckets, -1, 480);
    memset(svDepthNodes, -1, 13200);
}

/* The camera select word: bits 2-3 are the camera mode (the same layout sef_132.c reads). */
typedef struct CameraSelectFlags {
    u64 unmodeled_lo : 2;
    u64 mode : 2;
    u64 unmodeled_hi : 60;
} CameraSelectFlags;

extern u64 gCameraSelectFlags;

/*
 * Inserts one node into depth bucket bucketIndex (40 buckets, depth * 20 clamped to 39)
 * of svDepthBuckets, keeping each bucket sorted by depth; nodes come from svDepthNodes and
 * svDepthNodeCount counts them. Depths below zero are kept only in camera mode 2 and clamp to 0.
 */
void svDepthListInsert(int itemId, int itemFlags, float depth) {
    int nodeIndex = svDepthNodeCount;
    int bucketIndex;
    SvDepthNode *bucketHead;
    SvDepthNode *node;
    SvDepthNode *nextNode;
    SvDepthNode *prevNode;
    int nextNodeIndex;

    if (depth < 0.0f) {
        if (((CameraSelectFlags *) &gCameraSelectFlags)->mode != 2) {
            return;
        }
        depth = 0.0f;
    }
    bucketIndex = (int) (depth * 20.0f);
    if (bucketIndex >= 40) {
        bucketIndex = 39;
    }
    if (nodeIndex >= 1100) {
        return;
    }
    bucketHead = &svDepthBuckets[bucketIndex];
    node = &svDepthNodes[nodeIndex];
    /* The bucket head sits where an item keeps its next index, so the walk starts on the bucket. */
    prevNode = bucketHead;
    if (bucketHead->next < 0) {
        bucketHead->next = nodeIndex;
        bucketHead->tail = nodeIndex;
        node->depth = depth;
        node->itemFlags = itemFlags;
        node->itemId = itemId;
    } else {
        node->itemId = itemId;
        node->itemFlags = itemFlags;
        node->depth = depth;
        while (prevNode->next >= 0) {
            nextNodeIndex = prevNode->next;
            nextNode = &svDepthNodes[nextNodeIndex];
            if (nextNode->depth < depth) {
                prevNode->next = nodeIndex;
                node->next = nextNodeIndex;
                break;
            }
            prevNode = nextNode;
        }
        if (prevNode->next < 0) {
            prevNode->next = nodeIndex;
        }
    }
    svDepthNodeCount++;
}

/* <<< CHUNK sef_light_set */
/* >>> CHUNK sv_get_image_item */
/* Row-major 4x4 float matrix. */
#define SV_ENTRY_COUNT 140

/* SvTypeListEntry.resourceKind, equal to the chunk kind it was built from. */
enum {
    SV_RES_CLUT = 3,
    SV_RES_IMAGE = 4,
    SV_RES_SCRIPT = 5,
    SV_RES_MODEL = 6
};

typedef struct SvResourceBlob {
    unsigned int byteSize;
    unsigned char unmodeled_04[4];
    short kind;
    unsigned short typeFlags;
    char payload[0x34];
} SvResourceBlob;

/* Byte offset from a base pointer by a runtime amount read from the data (no field expresses it). */
#define BYTE_OFFSET(base, off) ((u8 *) (base) + (off))
/* The chunk after this one: chunks are laid out back to back, each byteSize bytes long. */
#define SV_CHUNK_NEXT(chunk) ((SvResourceBlob *) BYTE_OFFSET(chunk, (chunk)->byteSize))

typedef struct SvImageBlob {
    unsigned int byteSize;
    unsigned int pixelDataSize;          /* bytes of pixel data after the header */
    unsigned char unmodeled_08[2];
    unsigned short typeFlags;
    unsigned short width;
    unsigned short height;
    short bitsPerPixel;
    unsigned char unmodeled_12[2];
    char name[0x2C];
} SvImageBlob;

/* A CLUT chunk: the payload's leading halfwords are the size and depth, the name follows at +0x14. */
typedef struct SvClutBlob {
    unsigned char unmodeled_00[10];
    unsigned short typeFlags;
    unsigned short width;
    unsigned short height;
    unsigned short bitsPerPixel;
    unsigned char unmodeled_12[2];
    char name[0x2C];
    unsigned char data[1];
} SvClutBlob;

extern int _draw3D;

/* The _draw3D modes: 2D items, 2D kind-8 sprites and 3D models. */
enum SvRenderMode {
    SV_RENDER_2D = 0,
    SV_RENDER_2D_SPRITE = 1,
    SV_RENDER_3D_MODEL = 2
};

int svImageListAlloc(SvTypeList *resourceTypeList);
SvTypeListEntry *svGetImageListItem(int type, unsigned int flags);

#define GS_PSMT8 0x13
#define GS_PSMT4 0x14

XglPacket *xglPacketGetCurrent(void);
void sceVif1PkCnt(XglPacket *packet, unsigned int count);
void sceVif1PkAddCode(XglPacket *packet, unsigned int tag);
void sceVif1PkRef(XglPacket *packet, const void *data, unsigned int qwc, unsigned int a3, unsigned int a4,
                  unsigned int a5);
void sceVif1PkOpenUpkCode(XglPacket *packet, unsigned int address, unsigned int format, unsigned int a3,
                          unsigned int a4);
void sceVif1PkCloseUpkCode(XglPacket *packet);
void sceVif1PkAddUpkData128N(XglPacket *packet, const void *data, unsigned int count);
static int svDrawMode;
void svDrawListAppend(int code, int operandA, int operandB);
void svDrawListEmit(XglPacket *packet, DrawListEntry *entry, int pendingCount);

/* Queued records that trigger svDrawListFlush from svDrawListTryFlush. */
#define SV_DRAW_LIST_FLUSH_THRESHOLD 0x1AE

extern DrawList svDrawList;

/* The record at the end cursor: svDrawList.entries is the base of the 16-byte slots, begin_index a slot count. */
#define SV_DRAW_LIST_BEGIN_RECORD ((DrawListEntry *) (svDrawList.entries + svDrawList.begin_index))

/* The VU1 program address and the quadwords per record of each draw mode. */
typedef struct SvDrawMode {
    u32 programAddress;
    u32 recordQuadwords;
} SvDrawMode;

extern SvDrawMode svDrawModeTagTable[16];
extern int svStaticRefQuadwordCount[];
extern unsigned char svStaticRefQuadwords[];
extern DrawStatePacket *svDrawStatePacket;
void sceVif1PkAddUpkData128(XglPacket *packet, Quadword word);
extern u8 D_00000080[];
extern u8 D_00000110[];

/* VRAM size svAddImage reserves for a square 4-bit image of at most 16 pixels. */
#define SV_IMAGE_SQUARE_4BIT_VRAM 0x100
/* Bytes svAddClut advances the list by: a 256-colour or a 16-colour CLUT block. */
#define SV_CLUT_8BIT_BYTES 256
#define SV_CLUT_4BIT_BYTES 128

/* The type lists sit back to back at 0x475600 (scaffold data, no name in config/symbols): svGetTypeList
 * indexes them by 5084 bytes (one SvTypeList). svImageListCreate clears one 5084-byte list at 0x4AC2C0. */
extern SvTypeList svResourceTypeLists[];
extern SvTypeList svResourceTypeListScratch;
const u64 svAlphaTestTable[4] = { 0x0000000000000044ULL, 0x0000000000000048ULL, 0x0000008000000042ULL, 0x000000000000006AULL };
const u64 svPairAlphaTable[5] = { 0x0000000000000054ULL, 0x0000000000000058ULL, 0x0000008000000052ULL, 0x000000000000006AULL, 0x0000000000000000ULL };
const Matrix4 svUnitQuadCornerMatrix = {
    { 0.0f, 0.0f, 1.0f, 0.0f },
    { 1.0f, 0.0f, 1.0f, 0.0f },
    { 0.0f, 1.0f, 1.0f, 0.0f },
    { 1.0f, 1.0f, 1.0f, 0.0f }
};
SvTypeList *svGetTypeList(int type);
SvTypeListEntry *svGetImageListItemSub(int type, unsigned int flags);
int sceVif1PkRefLoadImage(void *packet, u16 basePointer, int psm, int bufferWidth, const void *data, int quadwordCount, int, int, int, int);

int svAddModel(SvTypeList *resourceTypeList, SvResourceBlob *chunk);
int svAddScript(SvTypeList *resourceTypeList, SvResourceBlob *chunk);
/* * Adds an image chunk to a type list: the entry takes the image's size, depth, * name and data, and the VRAM cursor advances by the storage the image takes. */
int svAddImage(SvTypeList *list, SvImageBlob *image);
/* Adds a CLUT chunk as a list entry and advances savedSize by 256 (8 bits per pixel) or 128. Returns the entry * index, or -1 when the list is full. */ int svAddClut(SvTypeList *resourceTypeList, SvClutBlob *chunk);
int svAddScript2(SvTypeList *resourceTypeList, SvResourceBlob *script);

/* The two buffer addresses sefEffectTallyAt hands back: the first one is used when the flag is set, the second one otherwise. */
typedef struct DrawBufferPair {
    u32 first;
    u32 second;
} DrawBufferPair;

/* One 20-byte entry of the table sefEffectTallyAt indexes: the first two words are the buffer pair, +8 is the count of draw entries of that kind. */
typedef struct DrawKindEntry {
    DrawBufferPair buffers;
    int count;
    unsigned char unmodeled_0c[8];
} DrawKindEntry;

/* One 0x18-byte record of the entry count table: four counters (the last one at +0xC) and a total. */
typedef struct SefDrawKindCounts {
    int counts[4];                  /* +0x00 */
    unsigned int total;             /* +0x10 */
    unsigned char unmodeled_14[4];  /* +0x14 */
} SefDrawKindCounts;

#define SV_DRAW_LIST_FLUSH_CODE 0x28

/*
 * Opens a draw list buffer: picks the buffer by useFirst and reserves a packet of the chosen size.
 * Returns the reserved size (callers test it against zero); 0 when nothing was reserved.
 */
typedef struct DrawListWithCapacity {
    DrawList list;
    int capacity;
} DrawListWithCapacity;

SefDrawKindCounts *sefEffectTallyAt(int entryIndex);
extern void *xglPacketReserve(int);

u8 *svDrawListBegin(int useFirst) {
    DrawKindEntry *drawKind;
    DrawListWithCapacity *list;
    int cameraMode;
    int bufferBytes;
    int baseOffset;
    int packetSize;
    void *packet;

    drawKind = (DrawKindEntry *) sefEffectTallyAt(0x2C);
    cameraMode = ((CameraSelectFlags *) &gCameraSelectFlags)->mode;
    if (cameraMode == 2) {
        baseOffset = 0x1000;
        bufferBytes = 0x400;
    } else {
        baseOffset = 0x800;
        bufferBytes = 0x200;
    }
    if (useFirst != 0) {
        if (useFirst == 2) {
            int countBase = drawKind->count;
            packetSize = countBase != 0 ? countBase + baseOffset : bufferBytes;
        } else {
            int firstBase = drawKind->buffers.first;
            packetSize = firstBase != 0 ? firstBase + baseOffset : bufferBytes;
        }
    } else {
        int secondBase = drawKind->buffers.second;
        packetSize = secondBase != 0 ? secondBase + 0x400 : 0;
    }
    list = (DrawListWithCapacity *) &svDrawList;
    list->list.entries = 0;
    list->capacity = 0;
    if (packetSize == 0) {
        return 0;
    }
    packet = xglPacketReserve(packetSize);
    if (packet == 0) {
        return 0;
    }
    list->list.entries = packet;
    list->list.flush_code = SV_DRAW_LIST_FLUSH_CODE;
    list->list.end_index = 0;
    list->list.begin_index = 0;
    list->capacity = packetSize;
    return (u8 *) packetSize;
}

/*
 * Appends one draw record at the end cursor: a VIF FLUSH, STCYCL 4,4, an
 * UNPACK to VU address `vuAddress` and the operands. The record starts at the
 * old end, which becomes begin_index, and end_index moves on by 2.
 */
void svDrawListAppend(int vuAddress, int operandA, int operandB)
{
    DrawListEntry *flushRecord;

    svDrawList.begin_index = svDrawList.end_index;
    flushRecord = SV_DRAW_LIST_BEGIN_RECORD;
    flushRecord->payload = 0;
    flushRecord->flush_word = 0x11000000;
    flushRecord->nop_word = 0;
    flushRecord->stcycl_word = 0x01000404;
    flushRecord->unpack_word = vuAddress | 0x6C010000;
    flushRecord->count_word = 0;
    flushRecord->operand_a = operandA;
    flushRecord->operand_b = operandB;
    svDrawList.end_index += 2;
}

/* A 16-byte slot of the draw list: one quadword, or its four words (the UNPACK header is word 3). */
typedef union DrawListSlot {
    Quadword quad;
    struct {
        unsigned int word0;
        unsigned int word1;
        unsigned int word2;
        unsigned int unpack_word;
    };
} DrawListSlot;

/* Emits the VIF packet for `count` records at `entry`: sceVif1PkRef calls and UNPACK headers, split at 0x100 records. */
void svDrawListEmit(XglPacket *packet, DrawListEntry *pendingRecords, int pendingCount)
{
    DrawListEntry *batchUnpackRecord;
    DrawListSlot *drawSlots;
    int drawMode;
    int divisor;
    int countMinusOne;
    const void *recordsAddress;
    Quadword carriedQuad;

    drawMode = svDrawMode;
    recordsAddress = (const void *) ((u32) pendingRecords & 0xfffffff);
    divisor = svDrawModeTagTable[drawMode].recordQuadwords;
    batchUnpackRecord = SV_DRAW_LIST_BEGIN_RECORD;
    drawSlots = (DrawListSlot *) batchUnpackRecord;
    countMinusOne = pendingCount - 1;
    if (drawMode == 13) {
        batchUnpackRecord->count_word = (pendingCount - 8) / divisor;
        batchUnpackRecord->operand_a = divisor;
    } else if (drawMode == 14) {
        batchUnpackRecord->count_word = ((pendingCount - 14) / divisor) >> 1;
        batchUnpackRecord->operand_a = divisor;
    } else {
        batchUnpackRecord->count_word = countMinusOne / divisor;
        batchUnpackRecord->operand_a = divisor;
    }
    if (countMinusOne < 256) {
        batchUnpackRecord->unpack_word = (countMinusOne << 16) | 0x6C000028;
        sceVif1PkRef(packet, recordsAddress, pendingCount, 0, 0, 0);
        return;
    }
    if (countMinusOne == 256) {
        batchUnpackRecord->unpack_word = 0x6C000028;
        sceVif1PkRef(packet, recordsAddress, pendingCount, 0, 0, 0);
        return;
    }
    batchUnpackRecord->unpack_word = 0x6C000028;
    carriedQuad = drawSlots[257].quad;
    sceVif1PkRef(packet, recordsAddress, 257, 0, 0, 0);
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenUpkCode(packet, 296, 108, 4, 4);
    sceVif1PkAddUpkData128(packet, carriedQuad);
    sceVif1PkCloseUpkCode(packet);
    if (countMinusOne == 257) {
        return;
    }
    drawSlots[257].quad = 0;
    drawSlots[257].unpack_word = ((countMinusOne - 257) << 16) | 0x6C000129;
    sceVif1PkRef(packet, &drawSlots[257], countMinusOne - 256, 0, 0, 0);
}

/*
 * Flushes the queued draw records. Runs only with at least three queued,
 * _draw3D != 2 and svDrawMode > 0: it emits the records, FLUSHes, calls the
 * draw microprogram of the current mode (MSCAL), then appends record 0x28.
 */
void svDrawListFlush(int pendingCount)
{
    XglPacket *packet;
    u32 tag;
    DrawListEntry *flushRecord;

    if (pendingCount >= 3 && _draw3D != SV_RENDER_3D_MODEL && svDrawMode > 0) {
        packet = xglPacketGetCurrent();
        svDrawListEmit(packet, SV_DRAW_LIST_BEGIN_RECORD, pendingCount);
        sceVif1PkCnt(packet, 0);
        sceVif1PkAddCode(packet, 0x11000000);
        tag = svDrawModeTagTable[svDrawMode].programAddress;
        sceVif1PkAddCode(packet, (tag >> 3) | 0x14000000);
        sceVif1PkAddCode(packet, 0x11000000);
        flushRecord = SV_DRAW_LIST_BEGIN_RECORD;
        svDrawListAppend(0x28, flushRecord->operand_a, flushRecord->operand_b);
    }
}

void svDrawListTryFlush(void)
{
    int pendingCount;

    pendingCount = svDrawList.end_index - svDrawList.begin_index;
    if (pendingCount >= SV_DRAW_LIST_FLUSH_THRESHOLD) {
        svDrawListFlush(pendingCount);
    }
}

/* Sends the static reference quadwords (svStaticRefQuadwords, count svStaticRefQuadwordCount[0]) to VIF. */
void svDrawSendStaticRef(void)
{
    XglPacket *packet;

    packet = xglPacketGetCurrent();
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddCode(packet, 0x11000000);
    sceVif1PkRef(packet, svStaticRefQuadwords, svStaticRefQuadwordCount[0], 0, 0, 0);
}

extern s16 D_004C8390[];
/* One GS register setting of a packed A+D packet. */
typedef struct SvGsRegister {
    u64 value;
    u64 address;
} SvGsRegister;

/* The GIF tag of the view frame packet, its five constant registers, FRAME_1 and SCISSOR_1 (filled for each frame). */
extern SvGsRegister D_004754E0[8];
/* The 2D view offset and view scale. */
extern Vector4 D_00475440[2];
extern int svEffectSceneFlag;
extern const Matrix4 MMathUnitMatrix;
extern const Matrix4 svUnitQuadCornerMatrix;
extern char D_50[];
struct XglCamera *xglStudioGetActiveCamera(void);

typedef struct SvViewUpload {
    Matrix4 unitRotation;
    Vector4 scale;
    Vector4 offset;
    Matrix4 viewMatrix;
} SvViewUpload;

extern SvViewUpload svViewUpload;
DrawList svDrawList;

typedef union {
    float f;
    u32 w;
} CameraFloatCell;

/* One quadword through t0; an inline helper, so the copies the two arms below end with stay one asm statement. */
static inline void svViewCopyQuadword(Vector4 *copyDestination, Vector4 *copySource) {
    __asm__ __volatile__("lq $8, 0(%3)\nsq $8, 0(%2)\n"
                         : "=m"(*copyDestination), "+m"(*copySource)
                         : "r"(copyDestination), "r"(copySource)
                         : "$8", "memory");
}

/*
 * Uploads the per-frame view block to VU0: a 4-halfword camera block from D_004C8390 into
 * D_00475540 and D_00475550, the unit and view matrices, the scale and offset vectors
 * (from the 3D list or the camera, by _draw3D) and the 0xA-quadword view block.
 */
/* The camera fields svUploadViewFrame reads: four float cells at +0x60..+0x6C (the unmodeled ones keep unk_XX), then the view offset and scale vectors. */
typedef struct SvViewCamera {
    unsigned char unmodeled_00[0x60];
    CameraFloatCell unk_60;
    CameraFloatCell unk_64;
    CameraFloatCell unk_68;
    CameraFloatCell unk_6C;
    Vector4 viewOffset;             /* +0x70 */
    Vector4 viewScale;              /* +0x80 */
} SvViewCamera;

void svUploadViewFrame(void) {
    XglPacket *packet;
    SvViewCamera *camera;
    u16 lowHalf;
    s16 scaled;
    int highByte;
    s16 *scissor;

    camera = (SvViewCamera *) xglStudioGetActiveCamera();
    packet = xglPacketGetCurrent();
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddCode(packet, 0x11000000);

    if (svEffectSceneFlag != 0) {
        scaled = D_004C8390[2] / 64;
        lowHalf = ((u16 *) D_004C8390)[7];
        highByte = D_004C8390[1];
        D_004754E0[6].value = lowHalf | ((u64) scaled << 16) | ((u64) highByte << 24);
    } else {
        scaled = D_004C8390[2] / 64;
        lowHalf = ((u16 *) D_004C8390)[11];
        highByte = D_004C8390[1];
        D_004754E0[6].value = lowHalf | ((u64) scaled << 16) | ((u64) highByte << 24);
    }

    scissor = (s16 *) &D_004754E0[7].value;
    scissor[0] = (s16) camera->unk_60.f;
    scissor[1] = (s16) ((int) camera->unk_68.f - 1);
    scissor[2] = (s16) camera->unk_64.f;
    scissor[3] = (s16) ((int) camera->unk_6C.f - 1);

    sceVif1PkOpenUpkCode(packet, 0, 0x6C, 4, 4);
    sceVif1PkAddUpkData128N(packet, D_004754E0, 8);
    sceVif1PkCloseUpkCode(packet);

    __asm__ __volatile__("lqc2 vf1, 32(%1)\n"
                         "lqc2 vf2, 16(%1)\n"
                         "lqc2 vf3, 0(%1)\n"
                         "sqc2 vf0, 48(%0)\n"
                         "sqc2 vf1, 32(%0)\n"
                         "sqc2 vf2, 16(%0)\n"
                         "sqc2 vf3, 0(%0)\n"
                         :
                         : "r"(svViewUpload.unitRotation), "r"(MMathUnitMatrix)
                         : "memory");
    QuadwordCopyMatrixT0(svViewUpload.viewMatrix, svUnitQuadCornerMatrix);
    if (_draw3D == 1) {
        svViewCopyQuadword(&svViewUpload.scale, &D_00475440[1]);
        svViewCopyQuadword(&svViewUpload.offset, &D_00475440[0]);
    } else {
        svViewCopyQuadword(&svViewUpload.scale, &camera->viewScale);
        svViewCopyQuadword(&svViewUpload.offset, &camera->viewOffset);
    }
    svViewUpload.scale.z = svViewUpload.scale.z * 0.0625f;
    svViewUpload.scale.w = 0;
    svViewUpload.offset.z = svViewUpload.offset.z * 0.0625f;
    svViewUpload.offset.w = 0;

    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenUpkCode(packet, 0xA, 0x6C, 4, 4);
    sceVif1PkAddUpkData128N(packet, &svViewUpload, 0xA);
    sceVif1PkCloseUpkCode(packet);
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddCode(packet, 0x11000000);
    sceVif1PkAddCode(packet, ((unsigned int) D_50 >> 3) | 0x14000000);
}

/* Fills the draw state packet: a GIF tag with the 0xE register list, CLAMP, ZBUF and the ALPHA entry alphaTableIndex of svAlphaTestTable. */
void svDrawSetState(int clampEnable, int alphaTableIndex, int clampRepeat)
{
    DrawStatePacket *statePacket;
    DrawStatePacket *statePacketAlias;
    u64 alphaData;

    statePacket = svDrawStatePacket;
    statePacket->gif_tag = 0x1000000000008003ULL;
    statePacket->gif_registers = 0xE;
    if (clampEnable != 0 && clampRepeat == 0) {
        statePacket->clamp.data = 0;
    } else {
        statePacket->clamp.data = 5;
    }
    statePacketAlias = svDrawStatePacket;
    statePacketAlias->clamp.address = 8;
    statePacketAlias->zbuf.data = 0x9880ULL << 17;
    statePacketAlias->zbuf.address = 78;
    alphaData = svAlphaTestTable[alphaTableIndex];
    statePacketAlias->alpha.address = 66;
    statePacketAlias->alpha.data = alphaData;
}

/* Sends the draw state packet to VIF, then the tag address of the current mode (D_00000080 or D_00000110). */
void svDrawSubmitMode(void)
{
    XglPacket *packet;

    packet = xglPacketGetCurrent();
    if (_draw3D == SV_RENDER_3D_MODEL) {
        return;
    }
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddCode(packet, 0x11000000);
    sceVif1PkOpenUpkCode(packet, 20, 108, 4, 4);
    sceVif1PkAddUpkData128N(packet, svDrawStatePacket, 16);
    sceVif1PkCloseUpkCode(packet);
    sceVif1PkAddCode(packet, 0x11000000);
    if (svDrawMode < 5 || svDrawMode == 11 || svDrawMode == 12) {
        sceVif1PkAddCode(packet, ((u32) D_00000080 >> 3) | 0x14000000);
    } else {
        sceVif1PkAddCode(packet, ((u32) D_00000110 >> 3) | 0x14000000);
    }
}

void svImageListCreate(void)
{
    memset(&svResourceTypeListScratch, 0, sizeof(SvTypeList));
}

void svImageListDestroy(SvTypeList *resourceTypeList)
{
    int resourceEntryIndex;

    for (resourceEntryIndex = 0; resourceEntryIndex < SV_ENTRY_COUNT; resourceEntryIndex++) {
        if (resourceTypeList->entries[resourceEntryIndex].used != 0) {
            resourceTypeList->entries[resourceEntryIndex].used = 0;
        }
    }
    resourceTypeList->typeKind = 0;
    resourceTypeList->savedSize = resourceTypeList->size;
    do { } while (0);
    resourceTypeList->entryCount = 0;
    resourceTypeList->used = 0;
    do { } while (0);
    resourceTypeList->dimensions = 0;
    resourceTypeList->pendingCount = 0;
}

int svImageListAlloc(SvTypeList *resourceTypeList)
{
    int resourceEntryIndex;

    for (resourceEntryIndex = 0; resourceEntryIndex < SV_ENTRY_COUNT; resourceEntryIndex++) {
        if (resourceTypeList->entries[resourceEntryIndex].used == 0) {
            return resourceEntryIndex;
        }
    }
    return -1;
}

SvTypeList *svGetTypeList(int resourceTypeId)
{
    return &svResourceTypeLists[resourceTypeId];
}

SvTypeListEntry *svGetImageListItemSub(int resourceTypeId, unsigned int typeKey)
{
    SvTypeList *resourceTypeList;
    SvTypeListEntry *entryCursor;
    int resourceEntryIndex;

    resourceTypeList = svGetTypeList(resourceTypeId);
    entryCursor = resourceTypeList->entries;
    resourceEntryIndex = 0;
    while (resourceEntryIndex < SV_ENTRY_COUNT) {
        if (entryCursor->used != 0 && entryCursor->type == (unsigned char) typeKey) {
            return entryCursor;
        }
        entryCursor++;
        resourceEntryIndex++;
    }
    return 0;
}

/* Looks the list up with the type 0 for flag 0x100, else with the given type; a negative type returns 0. */
SvTypeListEntry *svGetImageListItem(int resourceTypeId, unsigned int typeKey) {
    if ((typeKey & 0x100) != 0) {
        resourceTypeId = 0;
    }
    if (resourceTypeId < 0) {
        return 0;
    }
    return svGetImageListItemSub(resourceTypeId, typeKey);
}

/* Queues the image load of one texture entry: its GS format (4 or 8 bits per pixel), base, width and size. */
void svLoadTexture(SvTypeListEntry *textureEntry) {
    int psm;
    int quadwordCount;
    u16 gsTextureBase;
    u16 width;
    u16 height;
    void *packet;

    height = textureEntry->height;
    gsTextureBase = textureEntry->basePointer;
    width = textureEntry->width;
    psm = GS_PSMT8;
    if (textureEntry->bitsPerPixel != 8) {
        psm = GS_PSMT4;
    }
    quadwordCount = (textureEntry->bufferWidthPixels * height) / 16;
    packet = xglPacketGetCurrent();
    sceVif1PkRefLoadImage(packet, gsTextureBase, psm, (textureEntry->bufferWidthPixels >> 6) + 1, textureEntry->data, quadwordCount, 0, 0, width, height);
}

/* Uploads a palette: 16x16 for 8-bit entries, 8x2 for 4-bit, to the current packet. */
void svLoadClut(SvTypeListEntry *clutEntry) {
    u16 gsClutBase;
    void *packet;

    gsClutBase = clutEntry->basePointer;
    packet = xglPacketGetCurrent();
    if (clutEntry->bitsPerPixel == 8) {
        sceVif1PkRefLoadImage(packet, gsClutBase, 0, 1, clutEntry->data, 0x40, 0, 0, 0x10, 0x10);
        return;
    }
    sceVif1PkRefLoadImage(packet, gsClutBase, 0, 1, clutEntry->data, 4, 0, 0, 8, 2);
}

/* Queues every image and CLUT entry of a list into the current packet and returns the image count. */
int svLoadImageList(SvTypeList *resourceTypeList)
{
    int resourceEntryIndex;
    unsigned int imageCount;

    imageCount = 0;
    for (resourceEntryIndex = 0; resourceEntryIndex < SV_ENTRY_COUNT; resourceEntryIndex++) {
        if (resourceTypeList->entries[resourceEntryIndex].used) {
            if (resourceTypeList->entries[resourceEntryIndex].resourceKind == SV_RES_IMAGE) {
                if (imageCount == 0) {
                    void *packet;

                    packet = xglPacketGetCurrent();
                    sceVif1PkCnt(packet, 0);
                    sceVif1PkAddCode(packet, 0x11000000);
                }
                imageCount++;
                svLoadTexture(&resourceTypeList->entries[resourceEntryIndex]);
            } else if (resourceTypeList->entries[resourceEntryIndex].resourceKind == SV_RES_CLUT) {
                svLoadClut(&resourceTypeList->entries[resourceEntryIndex]);
            }
        }
    }
    return imageCount;
}

/* Returns the index of the lowest set bit of dimension from bit 1 up (its texture size exponent), or 0. */
static int svGetSizeBit(unsigned int dimension) {
    unsigned int mask;
    int bitIndex;

    mask = 2;
    bitIndex = 1;
    while (bitIndex < 32) {
        if ((dimension & mask) != 0) {
            return bitIndex;
        }
        mask <<= 1;
        bitIndex++;
    }
    return 0;
}

/*
 * Adds an image chunk to a type list: the entry takes the image's size, depth,
 * name and data, and the VRAM cursor advances by the storage the image takes.
 */
int svAddImage(SvTypeList *resourceTypeList, SvImageBlob *image)
{
    SvTypeListEntry *imageEntry;
    unsigned int width;
    unsigned int height;
    unsigned int storageSize;
    int resourceEntryIndex;

    resourceEntryIndex = svImageListAlloc(resourceTypeList);
    if (resourceEntryIndex < 0) {
        return -1;
    }
    imageEntry = &resourceTypeList->entries[resourceEntryIndex];
    imageEntry->resourceKind = SV_RES_IMAGE;
    imageEntry->data = image + 1;
    imageEntry->typeFlags = image->typeFlags;
    imageEntry->used = 1;
    imageEntry->width = image->width;
    width = imageEntry->width;
    imageEntry->height = image->height;
    imageEntry->bitsPerPixel = image->bitsPerPixel;
    imageEntry->textureWidthExponent = svGetSizeBit(width);
    height = imageEntry->height;
    imageEntry->textureHeightExponent = svGetSizeBit(height);
    imageEntry->name = image->name;
    storageSize = image->pixelDataSize;
    if (image->bitsPerPixel == 4) {
        unsigned long comparedHeight; /* height widened for the side comparisons */

        imageEntry->bufferWidthPixels = (short)image->width >> 1;
        comparedHeight = height;
        if (comparedHeight != width) {
            if (comparedHeight < width) {
                storageSize *= (width / height) >> 1;
            } else {
                storageSize *= (height / width) >> 1;
            }
        } else if (comparedHeight <= 16) {
            storageSize = SV_IMAGE_SQUARE_4BIT_VRAM;
        }
    } else {
        unsigned long comparedHeight; /* height widened for the side comparisons */

        comparedHeight = height;
        if (comparedHeight != width) {
            if (comparedHeight < width) {
                storageSize *= (width / height) >> 1;
            } else {
                storageSize *= (height / width) >> 1;
            }
        }
        if (imageEntry->height > imageEntry->width) {
            imageEntry->bufferWidthPixels = (short)image->width * (int)height / (int)width;
        } else {
            imageEntry->bufferWidthPixels = image->width;
        }
    }
    imageEntry->basePointer = resourceTypeList->savedSize >> 6;
    resourceTypeList->savedSize += storageSize >> 2;
    return resourceEntryIndex;
}

/* Adds a CLUT chunk as a list entry and advances savedSize by 256 (8 bits per pixel) or 128. Returns the entry
 * index, or -1 when the list is full. */
int svAddClut(SvTypeList *resourceTypeList, SvClutBlob *chunk) {
    SvTypeListEntry *clutEntry;
    int resourceEntryIndex;

    resourceEntryIndex = svImageListAlloc(resourceTypeList);
    if (resourceEntryIndex < 0) {
        return -1;
    }
    clutEntry = &resourceTypeList->entries[resourceEntryIndex];
    clutEntry->data = chunk->data;
    clutEntry->used = 1;
    clutEntry->resourceKind = SV_RES_CLUT;
    clutEntry->typeFlags = chunk->typeFlags;
    clutEntry->width = chunk->width;
    clutEntry->height = chunk->height;
    clutEntry->bitsPerPixel = chunk->bitsPerPixel;
    clutEntry->textureWidthExponent = svGetSizeBit(clutEntry->width);
    clutEntry->textureHeightExponent = svGetSizeBit(clutEntry->height);
    clutEntry->bufferWidthPixels = (short) chunk->width * 4;
    clutEntry->name = chunk->name;
    clutEntry->basePointer = resourceTypeList->savedSize >> 6;
    resourceTypeList->savedSize += clutEntry->bitsPerPixel == 8 ? SV_CLUT_8BIT_BYTES : SV_CLUT_4BIT_BYTES;
    return resourceEntryIndex;
}

/* Adds a model resource as a list entry; returns its index, or -1 when the list is full. */
int svAddModel(SvTypeList *resourceTypeList, SvResourceBlob *modelResource)
{
    int resourceEntryIndex;
    SvTypeListEntry *modelEntry;

    resourceEntryIndex = svImageListAlloc(resourceTypeList);
    if (resourceEntryIndex < 0) {
        return -1;
    }
    modelEntry = &resourceTypeList->entries[resourceEntryIndex];
    modelEntry->resourceKind = SV_RES_MODEL;
    modelEntry->name = modelResource->payload + 8;
    modelEntry->typeFlags = modelResource->typeFlags | 0x2000;
    modelEntry->data = modelResource + 1;
    modelEntry->used = 1;
    modelEntry->height = 0;
    modelEntry->width = 0;
    return resourceEntryIndex;
}

/* Adds a script resource as a list entry; returns its index, or -1 when the list is full. */
int svAddScript(SvTypeList *resourceTypeList, SvResourceBlob *scriptResource)
{
    int resourceEntryIndex;
    SvTypeListEntry *scriptEntry;

    resourceEntryIndex = svImageListAlloc(resourceTypeList);
    if (resourceEntryIndex < 0) {
        return -1;
    }
    scriptEntry = &resourceTypeList->entries[resourceEntryIndex];
    scriptEntry->resourceKind = SV_RES_SCRIPT;
    scriptEntry->name = scriptResource->payload + 8;
    scriptEntry->typeFlags = scriptResource->typeFlags;
    scriptEntry->data = scriptResource + 1;
    scriptEntry->used = 1;
    scriptEntry->height = 0;
    scriptEntry->width = 0;
    return resourceEntryIndex;
}

/* A script resource: the header (chunk and script header) fills the first 0xC0 bytes, the script data follows. */
typedef struct SvScriptBlob {
    unsigned char unmodeled_00[0xC0];
    unsigned char data[0];
} SvScriptBlob;

int svAddScript2(SvTypeList *resourceTypeList, SvResourceBlob *scriptResource)
{
    int resourceEntryIndex;
    SvTypeListEntry *scriptEntry;

    resourceEntryIndex = svImageListAlloc(resourceTypeList);
    if (resourceEntryIndex < 0) {
        return -1;
    }
    scriptEntry = &resourceTypeList->entries[resourceEntryIndex];
    scriptEntry->resourceKind = SV_RES_SCRIPT;
    scriptEntry->name = scriptResource->payload + 8;
    scriptEntry->typeFlags = scriptResource->typeFlags;
    scriptEntry->data = ((SvScriptBlob *) scriptResource)->data;
    scriptEntry->used = 1;
    scriptEntry->height = 0;
    scriptEntry->width = 0;
    return resourceEntryIndex;
}

const DrawStateWords D_0053F2E0 = { { 0x3F800000BF800000ULL, 0x3F80000000000000ULL, 0x3F8000003F800000ULL, 0x3F80000000000000ULL, 0xBF800000BF800000ULL, 0x3F80000000000000ULL, 0xBF8000003F800000ULL, 0x3F80000000000000ULL } };
const DrawStateWords D_0053F320 = { { 0x00000000BF800000ULL, 0x3F8000003F800000ULL, 0x000000003F800000ULL, 0x3F8000003F800000ULL, 0x00000000BF800000ULL, 0x3F800000BF800000ULL, 0x000000003F800000ULL, 0x3F800000BF800000ULL } };
const DrawStateWords D_0053F360 = { { 0x3F80000000000000ULL, 0x3F800000BF800000ULL, 0x3F80000000000000ULL, 0x3F8000003F800000ULL, 0xBF80000000000000ULL, 0x3F800000BF800000ULL, 0xBF80000000000000ULL, 0x3F8000003F800000ULL } };
const DrawStateWords D_0053F3A0 = { { 0x42C80000C2C80000ULL, 0x3F80000000000000ULL, 0x42C8000042C80000ULL, 0x3F80000000000000ULL, 0xC2C80000C2C80000ULL, 0x3F80000000000000ULL, 0xC2C8000042C80000ULL, 0x3F80000000000000ULL } };
const Vector4 D_0053F3E0 = { 0.0f, 0.0f, 0.0f, 1.0f };
const u64 D_0053F3F0[2] = { 0xC02A400000008000ULL, 0x0000525252521EEEULL };

/* Adds each chunk of a resource file to the list from firstResource on, until a kind 0 or an unknown kind. */
void svAnalyzeChunk(SvTypeList *resourceTypeList, SvResourceBlob **firstResource)
{
    SvResourceBlob *currentResource;

    currentResource = *firstResource;
    while (currentResource->kind != 0) {
        switch (currentResource->kind) {
        case 1:
        case 2:
            currentResource = currentResource + 1;
            continue;
        case SV_RES_IMAGE:
            svAddImage(resourceTypeList, (SvImageBlob *) currentResource);
            currentResource = SV_CHUNK_NEXT(currentResource);
            continue;
        case SV_RES_CLUT:
            svAddClut(resourceTypeList, (SvClutBlob *) currentResource);
            currentResource = SV_CHUNK_NEXT(currentResource);
            continue;
        case 7:
            svAddImage(resourceTypeList, (SvImageBlob *) currentResource);
            currentResource = SV_CHUNK_NEXT(currentResource);
            continue;
        case SV_RES_MODEL:
        case 9:
            svAddModel(resourceTypeList, currentResource);
            currentResource = SV_CHUNK_NEXT(currentResource);
            continue;
        case SV_RES_SCRIPT:
            svAddScript(resourceTypeList, currentResource);
            currentResource = SV_CHUNK_NEXT(currentResource);
            continue;
        case 10:
            svAddScript2(resourceTypeList, currentResource);
            currentResource = SV_CHUNK_NEXT(currentResource);
            continue;
        default:
            return;
        }
    }
}


/* Clears the 44 type lists of svResourceTypeLists and sets each one's storage size and capacity. */
void svInitImageMapper(void)
{
    int resourceTypeId;
    unsigned int typeListStorageBytes;
    int listCapacity;

    memset(svResourceTypeLists, 0, 44 * sizeof(SvTypeList));
    for (resourceTypeId = 0; resourceTypeId < 44; resourceTypeId++) {
        if (resourceTypeId == 0) {
            typeListStorageBytes = 0xE0000;
            listCapacity = 0x10000;
        } else {
            typeListStorageBytes = 0xF0000;
            listCapacity = 0x10000;
        }
        svResourceTypeLists[resourceTypeId].size = typeListStorageBytes;
        svResourceTypeLists[resourceTypeId].savedSize = typeListStorageBytes;
        svResourceTypeLists[resourceTypeId].capacity = listCapacity;
        svResourceTypeLists[resourceTypeId].entryCount = 0;
    }
}

/* The list kind a load request carries; its low byte becomes SvTypeList.typeKind. */
typedef enum SvListKind {
    SV_LIST_KIND_0,
    SV_LIST_KIND_1
} SvListKind;

/* A resource file to register in a type list. */
typedef struct SvFileRequest {
    SvResourceBlob *data;
    const char *name;
    int size; /* bits 28..29: 3 for a file that holds nested lists */
    SvListKind kind;
} SvFileRequest;

#define SV_FIRST_PART_LIST 4
#define SV_PART_LIST_COUNT 40



/*
 * Registers a resource file. A plain file becomes the type list `type`; in a file of nested lists every part chunk
 * (kinds 5 and 10) becomes the first free list from SV_FIRST_PART_LIST on.
 */
void svRegisterResourceFile(int type, SvFileRequest *request) {
    SvResourceBlob *chunk;
    int partIndex;

    if (((request->size >> 28) & 3) != 3) {
        SvTypeList *list;

        list = svGetTypeList(type);
        if (request->name != 0) {
            strcpy((char *) &list->dimensions, request->name);
        } else {
            list->dimensions = 0;
        }
        list->pendingCount = request->size;
        list->used = (int) request->data;
        list->typeKind = request->kind;
        svAnalyzeChunk(list, &request->data);
        return;
    }
    chunk = request->data;
    while (chunk->kind != 0) {
        switch (chunk->kind) {
        case 1:
        case 2:
            chunk++;
            continue;
        case 5:
        case 10:
            chunk++;
            for (partIndex = 0; partIndex < SV_PART_LIST_COUNT; partIndex++) {
                if (svResourceTypeLists[partIndex + SV_FIRST_PART_LIST].used == 0) {
                    break;
                }
            }
            if (partIndex != SV_PART_LIST_COUNT) {
                SvTypeList *list;
                SvFileRequest part;

                list = svGetTypeList(partIndex + SV_FIRST_PART_LIST);
                part.size = request->size;
                part.name = request->name;
                part.data = chunk;
                part.kind = request->kind;
                if (part.name != 0) {
                    strcpy((char *) &list->dimensions, part.name);
                } else {
                    list->dimensions = 0;
                }
                list->pendingCount = request->size;
                list->used = (int) chunk;
                svAnalyzeChunk(list, &part.data);
            }
            chunk = SV_CHUNK_NEXT(chunk);
            continue;
        case 3:
        case 4:
        case 6:
        case 7:
        case 9:
            chunk = SV_CHUNK_NEXT(chunk);
            continue;
        case 8:
        default:
            return;
        }
    }
}

/* One entry of the visible list svVisibleSlots: disabled is set for a resource slot, whose recordIndex is then
 * a sefResourceTable index, so svDrawEntriesByKind skips it. */
typedef struct SvDrawSlot {
    short recordIndex;
    short disabled;
    float depth;       /* +0x4: the sort key, the record's clip depth */
} SvDrawSlot;

#undef SV_ENTRY_COUNT
#define SV_ENTRY_COUNT 0x100

/* A part's transform block, filled by sefInitPartTransform: the 4x4 matrix (rows 0..2 the scaled identity, row 3 the
 * position with w = 1), the position vector, the scale, whether the scale applies, and the kind. The first 0x58 bytes of
 * an SvScenePart, and the object sefGlobalTransform. */
typedef struct SefPartTransform {
    Matrix4 matrix;   /* +0x00 */
    AlignedVector4 position; /* +0x40 */
    float scale;      /* +0x50 */
    short scaled;     /* +0x54: nonzero when scale applies */
    short kind;       /* +0x56 */
} SefPartTransform;

void svLoadDrawViewMatrix(void);
void svBuildDrawStatePacket(void);
void sefTransformClipPoint(Vector4 *out, const Vector4 *in);
float sefGetEntryScale(int id);
void sefGetPosition(Vector4 *dest, const Vector4 *source, int kind);
int sefInitPartTransform(SefPartTransform *part, int kind, int scaleKey100, int scaleKey200);
typedef struct SvKeyHeader {
    short kind;
    unsigned short parameter;
} SvKeyHeader;

typedef struct SvDrawItem {
    unsigned char unmodeled_00[0x160];
    short *entries; /* +0x160: entries[-2] is the mode key read by svSortLineSlots (X3 asm lw 352) */
    unsigned char unmodeled_164[0x1BC - 0x164];
    unsigned short imageFlags; /* +0x1BC: lhu 444, the clip bit 0x4000 and the draw flags */
    unsigned char unmodeled_1BE[0x664 - 0x1BE];
    short *s19Keys; /* +0x664: s19Keys[-1] is the s19 key (X3 asm lw 1636) */
    unsigned char unmodeled_668[0x66C - 0x668];
    SvKeyHeader *keyHeaders; /* +0x66C: keyHeaders[-1].kind (X3 asm lw 1644) */
    unsigned char unmodeled_670[0x674 - 0x670];
    short *fogTable; /* +0x674: fogTable[-1] is the fog value (X3 asm lw 1652) */
    unsigned char unmodeled_678[0x690 - 0x678];
    union {
        u64 modeWide; /* +0x690: the doubleword that the wide-mode test loads (ld 1680); modeFlags is its high word */
        struct {
            int modeWideLow;
            int modeFlags;
        } modeWords; /* +0x694: lw 1684 */
    };
    unsigned char unmodeled_698[0x69C - 0x698];
    short pointHandles[0x100]; /* +0x69C: one slot handle per row, -1 when unused (X3 asm addiu 1692) */
    unsigned char unmodeled_89C[0x89E - 0x89C];
    unsigned short rowCount; /* +0x89E: the row count of the loop (X3 asm lhu 2206) */
    unsigned short count;    /* +0x8A0: the item's count (X3 asm lhu 2208) */
    unsigned char unmodeled_8A2[0x8A4 - 0x8A2];
    short kindOverride; /* +0x8A4: read in place of the mode key when that key is 16 or 17 (X3 asm lh 2212) */
    unsigned char unmodeled_8A6[0x8A9 - 0x8A6];
    signed char clampRepeat; /* +0x8A9: lb 2217, the third svDrawSetState argument */
} SvDrawItem;
void svDrawScene2D(void);
void svDrawScene3D(void);
void sdvDrawAlters(int layer);

short MMathIsVectorEqual(const Vector4 *first, const Vector4 *second);
float MMathVectorDotProduct(const Vector4 *first, const Vector4 *second);
Vector4 *MMathApplyMatrix(Vector4 *out, Matrix4 *matrix, const Vector4 *vector);
Matrix4 *MMathMulMatrix(Matrix4 *out, Matrix4 *rhs, Matrix4 *lhs);
Matrix4 *MMathScaleMatrix(Matrix4 *out, Matrix4 *matrix, const Vector4 *scale);
Matrix4 *MMathScaleMatrixRotS(Matrix4 *out, Matrix4 *matrix, float scale);
struct XglCamera;

SefEffectRecord *sefGetScheduler(void);
int sefEffectFlagTest(int mask);

struct SefRecordOwner;

/* The fog override block sefSetFogOverride fills (the statics sefOverrideFlag and the two override vectors of sef.c,
 * one object in the original): the enabled word, four fog distances at +0x10 and the fog colour at +0x20. */
typedef struct SefFogOverride {
    int enabled;
    unsigned char unmodeled_04[0x10 - 0x04];
    float dist[4];
    Vector4 fogColor;
} SefFogOverride;

SefFogOverride *sefGetFogOverrideBlock(void);
struct SvDrawItem *sefGetLineAdr(int entryIndex);
unsigned char *sevGetPtAllocator2(int slotIndex);

#define GS_TEX0_1 0x06
#define GS_ALPHA_1 0x42
#define GS_TEST_1 0x47
/* GIFtag register descriptor of an A+D (address plus data) register. */
#define GIF_AD 0x0E

void nmlModelSetStencilAlpha(long long value);
void nmlModelSetDrawMatrix(Matrix4 matrix);
void nmlModelSetStencil(int enabled);
void nmlModelSetFogCol(Vector4 *position);
void nmlModelSetFogDist(float fogNear, float fogFar, float fogMin, float fogMax);

void nmlModelSetScale(const Vector4 *src);
void nmlModelSetSortOffset(float value);
void nmlModelSetTexture(TxyBlock *block);
void nmlModelSetTransparency(float value);
void nmlModelSetZwrite(int enabled);
void pxySetAnimTime(float value);
void pxySetLightCopy(XglLightSet *light);
int pxySubmitModel(PxyModelHeader *header);
/* The identity matrix the rotation functions start from (test_128.c defines it). */
extern const Vector4 vu0IdentityMatrix[4];
void vu0CurMatrixMulRight(Vector4 *rows);

void vu0RotateCurrentZ(float angle);
void vu0TransformPoint(Vector4 *point);
void vu0CurMatrixMul(Matrix4 matrix);

static u64 svZTestWord;
/* Scaffold .sbss int svZTestEnable: the Z-test enable flag written by svSetZTestEnv. */
static int svZTestEnable;
/* Scaffold .sbss pointer svVertexScale (4 bytes): the optional scale vector of the draw records, null when unset. */
static const Vector4 *svVertexScale;

/* One depth-sort item (svSortItems, 12 bytes): the next item index in its bucket, -1 at the end, then the id, flags and depth. */
typedef struct SortItem {
    short next;
    short id;
    short flags;
    float depth;
} SortItem;

void svSortReset(void);

u8 *svDrawListBegin(int useFirst);
void svDrawListAppend(int vuAddress, int operand_a, int operand_b);

void svDrawListEmit(XglPacket *packet, DrawListEntry *records, int pending_count);
void svDrawListFlush(int pending_count);
void svDrawListTryFlush(void);
void svDrawSendStaticRef(void);

/* The static VU1 view setup packet, nine quadwords. */
extern unsigned int svViewSetupPacket[];

void svDrawSubmitMode(void);

void svLoadTexture(SvTypeListEntry *entry);
void svLoadClut(SvTypeListEntry *entry);
int svLoadImageList(SvTypeList *resourceTypeList);

/* The TEST_1 words svSetZTestEnv stores: Z test on (GEQUAL) and always pass. */
#define SV_ZTEST_ON 0x5000D
#define SV_ZTEST_OFF 0x3000D

void sefInitClipViewVolume(void *clipVolume);

/* * Draws the scene. List 0 is loaded first. In 2D with flag 1 of svSceneFlags every visible effect record is drawn in * depth order, switching type lists as the records need them; otherwise each list 2..43 that has images is loaded and * its entries and alters are drawn. The records' shadow blocks come last, outside the effect scene. */ void svDrawScene(void);
extern u64 svMapperPacketEnv[6];

typedef struct SvDrawTagArea {
    u64 tags[4]; /* +0x00: the VIF and GIF tags */
    u64 tag;     /* +0x20: the TEST_1 value */
    u64 tagAddress;
    u64 zbuf[2];
} SvDrawTagArea;

/* Scaffold .data object svDrawStateTagArea (0x40 bytes): the draw-state tag area; its tag at +0x20 starts as the ON word. */
extern SvDrawTagArea svDrawStateTagArea;
void svSetZTestEnv(int enable);

/* Scaffold .data object svScaledVertexClampMax (16 bytes): the upper per-lane clamp of the scaled vertex (vmini in the scaled branch). */
extern Vector4 svScaledVertexClampMax;

/* The texture object at SvDrawState+0x1EC: its tex0 word is at +0x10. */
typedef struct SvDrawTexture {
    unsigned char unmodeled_00[0x10];
    u64 tex0;
} SvDrawTexture;

typedef struct SvDrawUv {
    float u;
    float v;
    unsigned char unmodeled_08[0x8];
    u64 tex0;
    unsigned char unmodeled_18[0x4];
    unsigned short flags;
    short kind;                     /* +0x1E: 4 for the image kinds drawn by svDrawSortedImages */
} SvDrawUv;

typedef struct SvDrawState {
    Matrix4 matrix;
    /* +0x00 */ unsigned char unmodeled_040[0x60];
    Vector4 pointA;
    /* +0xA0 */ unsigned char unmodeled_0B0[0x10];
    Vector4 pointB;
    /* +0xC0 */ unsigned char unmodeled_0D0[0x60];
    int vertex[4];
    /* +0x130 */ unsigned char unmodeled_140[0x18];
    SvDrawUv *uv;
    /* +0x158 */ SvDrawTexture *texture;
    /* +0x15C */ u64 primaryPrim;
    /* +0x160 */ u64 secondaryPrim;
    /* +0x168 */ unsigned char unmodeled_170[0x28];
    float angleZ;
    /* +0x198 */ unsigned char unmodeled_19C[0xC];
    float angleZOffset;
    /* +0x1A8 */ unsigned char unmodeled_1AC[0x50];
    int tableIndex;
    /* +0x1FC: the svPairAlphaTable index (X3 asm lw 508) */ Vector4 extraState;
/* +0x200 */ } SvDrawState;

typedef struct SvDrawRecordC {
    DoublewordVector4 gif;          /* +0x00: GIF tag and register list */
    DoublewordVector4 texture;      /* +0x10: the UV block's tex0, register 6 */
    DoublewordVector4 test;         /* +0x20: svDrawTestWord, register 0x47 */
    DoublewordVector4 alpha;        /* +0x30: the alpha argument, register 0x42 */
    Vector4 pointA;
    union {
        Vector4 vector;             /* +0x50: the vertex the sqc2 writes */
        int words[4];               /* the same as ints */
        Quadword quad;              /* the same, moved as one quadword */
    } vertex;
    DrawStateRegister prim;
    Vector4 extraState;
    DoublewordVector4 coords;       /* +0x80: u and v of the UV block, z 0 and w 1.0f */
    Vector4 pointB;
    Matrix4 matrix;
} SvDrawRecordC;

void svDrawListAppendSpriteRecord(SvDrawState *state, u64 alpha);
/* * Appends a pair of kind C draw records (28 quadwords): the first carries the GIF tag, the texture block's * tex0, the fixed test word 0x31003 and alpha 0x4A; the second the UV block's tex0, svZTestWord and the alpha * argument. The vertex (scaled by svVertexScale when set) goes to both, the first point to the first, the * secondary and primary prims to the first and second, then the extra state, coordinates (u, v, 0, 1), * second point and matrix like svDrawListAppendSpriteRecord. */ void svDrawListAppendSpriteRecordPair(SvDrawState *state, u64 alpha);

typedef struct SvRenderItem { unsigned char unmodeled_00[0xC0]; Vector4 scale; /* +0xC0 */ unsigned char unmodeled_0D0[0x130 - 0xD0]; int color[3]; /* +0x130 */ unsigned int transparency; /* +0x13C */ unsigned char unmodeled_140[0x158 - 0x140]; struct SvRenderPart *part; /* +0x158 */ unsigned char unmodeled_15C[0x1B8 - 0x15C]; float animTime; /* +0x1B8 */ unsigned char unmodeled_1BC[0x1FC - 0x1BC]; int stencilIndex; /* +0x1FC */ } SvRenderItem;

/* The part of the render item: the block pxySubmitModel takes at +0x8 and the block nmlModelSetTexture takes at +0xC. */
typedef struct SvRenderPart {
    unsigned char unmodeled_00[0x8];
    PxyModelHeader *submitBlock;
    TxyBlock *txyBlock;
} SvRenderPart;

/* The view block svSetModelRenderState writes: the scaled vertex position at +0xD0 and its copy at +0xE0. */
typedef struct SvVertexViewBlock {
    unsigned char unmodeled_00[0xD0];
    AlignedVector4 scaled;
    AlignedVector4 scaledCopy;
} SvVertexViewBlock;

extern const u64 svStencilTable[];
extern const f32 svSortOffsetScale;
void svSetModelRenderState(SvRenderItem *item, Matrix4 *matrix, int scale);

/* Screen vertex written by svProjectVertex: x and y in 12.4 fixed point, then z and the clip flags. */
typedef struct SvScreenVertex {
    int x;
    int y;
    int z;
    int clipFlags;
} SvScreenVertex;

/* One unit of screen space is 16 subpixel steps (the GS 12.4 coordinate format). */
#define SV_SUBPIXEL_STEPS 16.0f

/* A vertex of the two clip buffers: the vector, its components (element[3] holds the signed plane distance
 * while a plane is being clipped) and its raw words. */
typedef union SefClipVertex {
    Vector4 vector;
    float element[4];
    int word[4];
} SefClipVertex;

/* One vertex sefClipPolygon emits: the clipped position, then its texture coordinate. */
typedef struct SvClipVertex {
    Vector4 position;
    Vector4 texCoord;
} SvClipVertex;

/* Clips the draw state quad, transformed by matrix, against the first five planes of the active camera's view * volume (Sutherland-Hodgman, two vertex buffers). Each surviving vertex goes to out as a pair: the position * with w = 1, then svUnitQuadCornerMatrix with x and y replaced by the position's projections on the quad's two edges. * Returns the vertex count, 0 when the quad is degenerate or clipped away. */
int sefClipPolygon(void *slot, Matrix4 *matrix, SvClipVertex *out);

/* A vector moved as one quadword. */
typedef union SvQuadVector {
    Quadword quad;
    Vector4 vector;
} SvQuadVector;

/* A texture part: the two scale factors the polygon axes take and the TEX0 register word at +0x10. */
typedef struct SvPolygonPart {
    float scaleX;
    float scaleY;
    unsigned char unmodeled_08[0x10 - 0x08];
    u64 tex0;
} SvPolygonPart;

/* The argument block the item draw (svDrawItemSlots) and the sorted-image draw (svDrawSortedImages) hand to the slot
 * callees, which read only quad: the vector at +0x4 (the entry's scale vector, or the slot state's second point),
 * then the two reference blocks at +0x8 and +0xC and the draw-mode bits (-1 before the first line). */
typedef struct SvDrawCall {
    unsigned char unmodeled_00[4];
    Vector4 *quad;
    const void *texture;            /* +0x08 */
    const void *uv;                 /* +0x0C */
    int mode;                       /* +0x10 */
} SvDrawCall;

/* The packet head: the GIF A+D tag, TEX0, TEST, ALPHA and the CLAMP entry of the draw state, then the vertex tag. */
typedef struct SvPolygonPacket {
    DoublewordVector4 gif;
    DoublewordVector4 tex;
    DoublewordVector4 test;
    DoublewordVector4 alpha;
    Quadword clamp;
    DoublewordVector4 vertexTag;
} SvPolygonPacket;

/* One packed vertex: ST, RGBAQ and XYZ2. */
typedef struct SvPolygonVertex {
    SvQuadVector texCoord;
    Quadword color;
    Quadword position;
} SvPolygonVertex;

/* Part record of a slot: the two floats the draw packet reads, the 64-bit word at +0x10, the flags halfword (bit 0x10 hides the slot unless the flag allows it) and the kind halfword (4 is drawn here). */
typedef struct SefSlotPartDefinition {
    float coordinateX;
    float coordinateY;
    unsigned char unmodeled_08[0x10 - 0x08];
    unsigned long long word10;
    unsigned char unmodeled_18[0x1C - 0x18];
    unsigned short flags;
    short kind;
} SefSlotPartDefinition;

typedef struct SefDepthSlot {
    Matrix4 matrix;
    unsigned char unmodeled_40[0xA0 - 0x40];
    Vector4 position; /* +0xA0 */
    unsigned char unmodeled_B0[0xC0 - 0xB0];
    Vector4 scale; /* +0xC0 */
    unsigned char unmodeled_D0[0x120 - 0xD0];
    Vector4 pivot; /* +0x120 */
    unsigned char scaleBlock[0x13C - 0x130];
    void *drawData; /* +0x13C */
    unsigned char unmodeled_140[0x158 - 0x140];
    SefSlotPartDefinition *part; /* +0x158 */
    unsigned char unmodeled_15C[0x160 - 0x15C];
    Vector4 partBlock; /* +0x160 */
    unsigned char unmodeled_170[0x1FC - 0x170];
    int drawIndex;     /* +0x1FC */
    Vector4 slotBlock; /* +0x200 */
    unsigned char unmodeled_210[0x270 - 0x210];
} SefDepthSlot;

/* A draw record of 17 quadwords (0x110 bytes): the A+D registers, the position, the prim, the vertex, the extra state, the float coordinates, the quadword of the reference and the matrix. */
typedef struct SvDrawRecordB {
    DoublewordVector4 gif;          /* +0x00: GIF tag and register list */
    DoublewordVector4 texture;      /* +0x10: tex0 word, register 6 */
    DoublewordVector4 test;         /* +0x20: register 0x47 */
    DoublewordVector4 alpha;        /* +0x30: register 0x42 */
    Vector4 position;
    DrawStateRegister prim;
    Vector4 vertex;
    Vector4 extraState;
    DoublewordVector4 coords;       /* +0x80: u and v placed by svDrawMode, the remaining lane 0 and w 1.0f */
    Vector4 quad;
    Matrix4 matrix;
    unsigned char unmodeled_E0[0x110 - 0xE0];
} SvDrawRecordB;

typedef struct SvDrawRecordA {
    DoublewordVector4 gif;          /* +0x00: GIF tag and register list */
    DoublewordVector4 texture;      /* +0x10: tex0 word, register 6 */
    DoublewordVector4 test;         /* +0x20: svDrawTestWord, register 0x47 */
    DoublewordVector4 alpha;        /* +0x30: the svAlphaTestTable entry, register 0x42 */
    Vector4 position;
    union {
        int words[4];               /* +0x50: the int vertex the sqc2 writes */
        Quadword quad;              /* the same, moved as one quadword */
    } vertex;
    DrawStateRegister prim;
    Vector4 extraState;
    DoublewordVector4 coords;       /* +0x80: u and v from the UV block, then (0.0f, 1.0f) written as one doubleword */
    Vector4 quad;
    Matrix4 matrix;
} SvDrawRecordA;

/* The fixed TEST_1 data (0x31003) and alpha data (0x4A) of the first record of a pair. */
#define SV_TEST_FIXED_DATA 0x31003
#define SV_ALPHA_FIXED_DATA 0x4A

/* One quadword as two doublewords. */
typedef struct DrawQuad {
    u64 lowDoubleword;
    u64 highDoubleword;
} DrawQuad;

/* The 14-quadword draw packet appended to svDrawList: a GIF tag, the register list, then the A+D entries and the vectors. */
typedef struct SvPartPacket {
    DoublewordVector4 gif;
    DoublewordVector4 tex;          /* +0x10 */
    DoublewordVector4 test;         /* +0x20 */
    DoublewordVector4 alpha;        /* +0x30 */
    Vector4 vertex;                 /* +0x40 */
    int position[4];                /* +0x50 */
    DrawQuad extentA;               /* +0x60: high doubleword zero */
    Vector4 extentB;                /* +0x70 */
    DoublewordVector4 source;       /* +0x80: the source header, high doubleword 0x3F80000000000000 */
    Vector4 ref;                    /* +0x90 */
    Matrix4 rows;                   /* +0xA0 */
} SvPartPacket;

static inline void svCopyQuadword(Vector4 *dstVector, const Vector4 *srcVector)
{
    QuadwordCopyT0(dstVector, srcVector);
}

typedef struct SefEffectDrawState {
    unsigned char unmodeled_00[0x20];
    Vector4 scale;
    unsigned char unmodeled_30[0x50 - 0x30];
    int scaleKey40;  /* +0x50: the scale key for kind 0x40 */
    int scaleKey100; /* +0x54: the scale key for kind 0x100 */
    int scaleKey80;  /* +0x58: the scale key for kind 0x80 */
    int scaleKey200; /* +0x5C: the second scale key the X3 item draw passes (asm lw 92) */
    unsigned char unmodeled_60[0x74 - 0x60];
    int drawSelector; /* +0x74 */
} SefEffectDrawState;

extern SefEffectDrawState *sefCurrentEffectRecord;
extern int svSceneFlags;

/* sefCurrentEffectRecord seen as a one-member record slot. */
typedef struct SefCurrentEffectSlot {
    SefEffectRecord *record;
} SefCurrentEffectSlot;

void svDrawSortedImages(SefEffectRecord *effectRecord);

/* Scratchpad base address: svCollectSortedLines fills the SvLineSortKey array here, and the cull code reuses it. */
#define SV_SCRATCHPAD 0x70000000

typedef struct SvLineSortKey {
    short lineIndex;
    short sortKey;
    short on; /* +0x04: the on flag of the EffectRequest layout (tu129); zero in the collected pairs */
} SvLineSortKey;

/* Context block passed by svDrawEntry: the current matrix sits at +0x470. */
typedef struct SvDrawContext {
    unsigned char unmodeled_00[0x470];
    Matrix4 matrix;
} SvDrawContext;

void svDrawSortedImages(SefEffectRecord *record);

/* Read-only scaffold items after the C-owned literals: the stencil table svStencilTable and the matrix svEffectProjectionMatrix. */
extern const u64 svStencilTable[3];
/* .data of this TU: definitions in address order (data recovery pass 2). */
extern const float svSortOffsetScale;
extern unsigned int svViewSetupPacket[36];

typedef int s32;

void eBattleWinInit(s32 resourceTypeId) {
    SvTypeList *resourceTypeList;
    resourceTypeList = svGetTypeList(resourceTypeId);
    svImageListDestroy(resourceTypeList);
}

/* Refers the current packet to the three-quadword environment block svMapperPacketEnv; runs after every mapper switch. */
void svRefMapperPacket(void) {
    sceVif1PkRef(xglPacketGetCurrent(), svMapperPacketEnv, 3, 0, 0, 0);
}

/* Selects the Z test: on (GEQUAL, always pass) or off; the flag and the draw-state word follow. */
void svSetZTestEnv(int zTestEnable) {
    if (zTestEnable != 0) {
        svDrawStateTagArea.tag = SV_ZTEST_ON;
    } else {
        svDrawStateTagArea.tag = SV_ZTEST_OFF;
    }
    svZTestEnable = zTestEnable;
    svZTestWord = svDrawStateTagArea.tag;
}

/* Loads the clip matrices of a camera volume into VU0 (Vu0SetClipMatrices). */
void sefInitClipViewVolume(void *clipVolume) {
    Vu0SetClipMatrices(clipVolume);
}

/*
 * sefClipViewVolumeA: the VU0 clip test as one hand-written block: the flag gate on gCameraSelectFlags, six lane
 * compares of the transformed point against limit (two passes of three), the set-clear join. Returns 0 when the
 * gate skips the test or every lane is below limit, 1 otherwise.
 */
int sefClipViewVolumeA(const Vector4 *model, float limit) {
    __asm__ __volatile__(
        ".set noreorder\n\t"
        "lui $2, %%hi(gCameraSelectFlags)\n\t"
        "daddu $5, $4, $0\n\t"
        "ld $3, %%lo(gCameraSelectFlags)($2)\n\t"
        "dsrl $3, $3, 2\n\t"
        "andi $3, $3, 3\n\t"
        "dsll32 $3, $3, 0\n\t"
        "dsra32 $3, $3, 0\n\t"
        "beq $3, $0, 4f\n\t"
        "daddu $2, $0, $0\n\t"
        "lqc2 vf20, 0($5)\n\t"
        "mfc1 $4, $f12\n\t"
        "qmtc2 $4, vf2\n\t"
        "mtc1 $4, $f1\n\t"
        "vaddx.x vf2x, vf2x, vf20x\n\t"
        "vmulax.xyzw ACC, vf16, vf20x\n\t"
        "vmadday.xyzw ACC, vf17, vf20y\n\t"
        "vmaddaz.xyzw ACC, vf18, vf20z\n\t"
        "vmaddw.xyzw vf9, vf19, vf0w\n\t"
        "qmfc2 $4, vf9\n\t"
        "mtc1 $4, $f0\n\t"
        "sll $0, $0, 0\n\t"
        "c.lt.s $f0, $f1\n\t"
        "sll $0, $0, 0\n\t"
        "bc1f 1f\n\t"
        "sll $0, $0, 0\n\t"
        "sll $0, $0, 0\n\t"
        "vmr32.xyzw vf9, vf9\n\t"
        "qmfc2 $4, vf9\n\t"
        "mtc1 $4, $f0\n\t"
        "sll $0, $0, 0\n\t"
        "c.lt.s $f0, $f1\n\t"
        "sll $0, $0, 0\n\t"
        "bc1f 1f\n\t"
        "sll $0, $0, 0\n\t"
        "sll $0, $0, 0\n\t"
        "vmr32.xyzw vf9, vf9\n\t"
        "qmfc2 $4, vf9\n\t"
        "mtc1 $4, $f0\n\t"
        "sll $0, $0, 0\n\t"
        "c.lt.s $f0, $f1\n\t"
        "sll $0, $0, 0\n\t"
        "bc1f 1f\n\t"
        "sll $0, $0, 0\n\t"
        "sll $0, $0, 0\n\t"
        "vmulax.xyzw ACC, vf12, vf20x\n\t"
        "vmadday.xyzw ACC, vf13, vf20y\n\t"
        "vmaddaz.xyzw ACC, vf14, vf20z\n\t"
        "vmaddw.xyzw vf9, vf15, vf0w\n\t"
        "qmfc2 $4, vf9\n\t"
        "mtc1 $4, $f0\n\t"
        "sll $0, $0, 0\n\t"
        "c.lt.s $f0, $f1\n\t"
        "sll $0, $0, 0\n\t"
        "bc1f 1f\n\t"
        "sll $0, $0, 0\n\t"
        "sll $0, $0, 0\n\t"
        "vmr32.xyzw vf9, vf9\n\t"
        "qmfc2 $4, vf9\n\t"
        "mtc1 $4, $f0\n\t"
        "sll $0, $0, 0\n\t"
        "c.lt.s $f0, $f1\n\t"
        "sll $0, $0, 0\n\t"
        "bc1f 1f\n\t"
        "sll $0, $0, 0\n\t"
        "sll $0, $0, 0\n\t"
        "vmr32.xyzw vf9, vf9\n\t"
        "qmfc2 $4, vf9\n\t"
        "mtc1 $4, $f0\n\t"
        "sll $0, $0, 0\n\t"
        "c.lt.s $f0, $f1\n\t"
        "sll $0, $0, 0\n\t"
        "bc1f 1f\n\t"
        "sll $0, $0, 0\n\t"
        "sll $0, $0, 0\n\t"
        "sub $4, $4, $4\n\t"
        "j 2f\n\t"
        "sll $0, $0, 0\n\t"
        "sll $0, $0, 0\n\t"
        "1:\n\t"
        "sub $4, $4, $4\n\t"
        "addi $4, $4, 1\n\t"
        "2:\n\t"
        "daddu $2, $4, $0\n\t"
        "4:\n\t"
        ".set reorder"
        :
        :
        : "$2", "$3", "$4", "$5", "$f0", "$f1", "memory");
}

/*
 * Appends one kind C draw record (14 quadwords) to svDrawList: GIF tag, tex0, test and alpha words, the vertex
 * (scaled by svVertexScale when set), the points and prims, the extra state, coordinates (u, v, 0, 1) and the matrix.
 */
void svDrawListAppendSpriteRecord(SvDrawState *state, u64 alpha) {
    SvDrawRecordC *record;
    Vector4 *vertexDst;
    const Vector4 *vertexScale;
    Vector4 *extraDst;
    Vector4 *extraSrc;
    DoublewordVector4 *coordsDst;

    record = (SvDrawRecordC *) (svDrawList.entries + svDrawList.end_index);
    record->gif.words[0] = 0xD02A400000008001ULL;
    record->gif.words[1] = 0x000525252521EEEEULL;
    record->texture.words[0] = state->uv->tex0;
    record->texture.words[1] = GS_TEX0_1;
    record->test.words[0] = svZTestWord;
    record->test.words[1] = GS_TEST_1;
    record->alpha.words[0] = alpha;
    record->alpha.words[1] = GS_ALPHA_1;
    vertexDst = &record->vertex.vector;
    vertexScale = svVertexScale;
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(state->vertex) : "memory");
    if (vertexScale != 0) {
        __asm__ __volatile__("lqc2 vf2, 0(%0)\n\t"
                             "lqc2 vf3, 0(%1)\n\t"
                             "vitof0.xyzw vf1, vf1\n\t"
                             "vmul.xyzw vf1, vf1, vf2\n\t"
                             "vmini.xyz vf1, vf1, vf3\n\t"
                             "vftoi0.xyzw vf1, vf1"
                             :
                             : "r"(vertexScale), "r"(&svScaledVertexClampMax)
                             : "memory");
    }
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(vertexDst));
    QuadwordCopyT0(&record->pointA, &state->pointA);
    QuadwordCopyT0(&record->prim, &state->primaryPrim);
    record->prim.address = 0;
    extraDst = &record->extraState;
    extraSrc = &state->extraState;
    __asm__ __volatile__("lq $8, 0(%3)\nsq $8, 0(%2)\n"
                         : "=m"(*extraDst), "+m"(*extraSrc)
                         : "r"(extraDst), "r"(extraSrc)
                         : "$8", "memory");
    coordsDst = &record->coords;
    __asm__ __volatile__("lq $8, 0(%3)\nsq $8, 0(%2)\n"
                         : "=m"(*coordsDst), "+m"(*state->uv)
                         : "r"(coordsDst), "r"(state->uv)
                         : "$8", "memory");
    record->coords.words[1] = 0x3F80000000000000ULL;
    __asm__ __volatile__("lq $8, 0(%3)\nsq $8, 0(%2)\n"
                         : "=m"(record->pointB), "+m"(state->pointB)
                         : "r"(&record->pointB), "r"(&state->pointB)
                         : "$8", "memory");
    QuadwordCopyMatrixT0(record->matrix, state->matrix);
    svDrawList.end_index += 14;
}

/*
 * Appends a pair of kind C draw records (28 quadwords) for one sprite: the first record carries the GIF tag, the fixed
 * test and alpha words, the second the UV block's tex0. The vertex goes to both, the prims and coordinates follow.
 */
void svDrawListAppendSpriteRecordPair(SvDrawState *drawState, u64 alpha) {
    Vector4 *extraSrc;
    Vector4 *extraDst;
    DoublewordVector4 *coordsDst;
    SvDrawRecordC *spritePair;
    const Vector4 *vertexScale;
    int *firstVertexDst;

    spritePair = (SvDrawRecordC *) (svDrawList.entries + svDrawList.end_index);
    spritePair[0].gif.words[0] = 0xD02A400000008001ULL;
    spritePair[0].gif.words[1] = 0x000525252521EEEEULL;
    spritePair[0].texture.words[0] = drawState->texture->tex0;
    vertexScale = svVertexScale;
    spritePair[1].texture.words[0] = drawState->uv->tex0;
    spritePair[1].texture.words[1] = GS_TEX0_1;
    spritePair[1].test.words[1] = GS_TEST_1;
    spritePair[0].texture.words[1] = GS_TEX0_1;
    spritePair[0].test.words[0] = SV_TEST_FIXED_DATA;
    spritePair[1].test.words[0] = svZTestWord;
    spritePair[0].test.words[1] = GS_TEST_1;
    spritePair[0].alpha.words[0] = SV_ALPHA_FIXED_DATA;
    firstVertexDst = spritePair[0].vertex.words;
    spritePair[1].alpha.words[0] = alpha;
    spritePair[1].alpha.words[1] = GS_ALPHA_1;
    spritePair[0].alpha.words[1] = GS_ALPHA_1;
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(drawState->vertex) : "memory");
    if (vertexScale != 0) {
        __asm__ __volatile__("lqc2 vf2, 0(%0)\n\t"
                             "lqc2 vf3, 0(%1)\n\t"
                             "vitof0.xyzw vf1, vf1\n\t"
                             "vmul.xyzw vf1, vf1, vf2\n\t"
                             "vmini.xyz vf1, vf1, vf3\n\t"
                             "vftoi0.xyzw vf1, vf1"
                             :
                             : "r"(vertexScale), "r"(&svScaledVertexClampMax)
                             : "memory");
    }
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(firstVertexDst));
    spritePair[1].vertex.quad = spritePair[0].vertex.quad;
    __asm__ __volatile__("lq $8, 0(%3)\nsq $8, 0(%2)\n"
                         : "=m"(spritePair[0].pointA), "+m"(drawState->pointA)
                         : "r"(&spritePair[0].pointA), "r"(&drawState->pointA)
                         : "$8", "memory");
    spritePair[0].prim.data = drawState->secondaryPrim;
    spritePair[0].prim.address = 0;
    spritePair[1].prim.data = drawState->primaryPrim;
    spritePair[1].prim.address = 0;
    extraDst = &spritePair[0].extraState;
    extraSrc = &drawState->extraState;
    __asm__ __volatile__("lq $8, 0(%3)\nsq $8, 0(%2)\n"
                         : "=m"(*extraDst), "+m"(*extraSrc)
                         : "r"(extraDst), "r"(extraSrc)
                         : "$8", "memory");
    coordsDst = &spritePair[0].coords;
    __asm__ __volatile__("lq $8, 0(%3)\nsq $8, 0(%2)\n"
                         : "=m"(*coordsDst), "+m"(*drawState->uv)
                         : "r"(coordsDst), "r"(drawState->uv)
                         : "$8", "memory");
    spritePair[0].coords.words[1] = 0x3F80000000000000ULL;
    __asm__ __volatile__("lq $8, 0(%3)\nsq $8, 0(%2)\n"
                         : "=m"(spritePair[0].pointB), "+m"(drawState->pointB)
                         : "r"(&spritePair[0].pointB), "r"(&drawState->pointB)
                         : "$8", "memory");
    QuadwordCopyMatrixT0(spritePair[0].matrix, drawState->matrix);
    svDrawList.end_index += 28;
}

/* The item transform func_0020A320 builds: the matrix rows at +0x00, the quaternion MMathQuatToMatrix converts at +0x60,
 * the scale vector at +0xC0 and the point at +0x200 (offsets measured from the asm). */
typedef struct SvItemTransform {
    Matrix4 matrix; /* +0x000 */
    unsigned char unmodeled_040[0x60 - 0x40];
    Vector4 rotation; /* +0x060 */
    unsigned char unmodeled_070[0xC0 - 0x70];
    Vector4 scale; /* +0x0C0 */
    unsigned char unmodeled_0D0[0x200 - 0xD0];
    Vector4 point; /* +0x200 */
} SvItemTransform;

Vector4 *MMathQuatToMatrix(Vector4 *outVector, const Vector4 *inputQuat);
void vu0CurMatrixMulRightAffine(Matrix4 rows);
/* Scaffold data of func_0020A320 by its splat names (config/symbols has none): the effect matrix, the effect scale vector,
 * the fog value (gp-relative) and the record matrix pointer (gp-relative). */
extern Matrix4 svEffectMatrix;
extern Vector4 svEffectScaleVector;
extern int svFogValue;
static Matrix4 *svRecordMatrix;

/* Builds the current matrix of one item like svBuildItemMatrix: the fixed matrix svEffectMatrix, the record matrix svRecordMatrix or
 * the identity, times transform->matrix, then the scale rows. useItemRows also applies the rotation rows of transform->rotation. */
void func_0020A320(Vector4 *point, SvItemTransform *transform, int useFixedMatrix, int useItemRows,
                   int ignoreRecordTranslation) {
    Vector4 rotationRows[4];

    if (useFixedMatrix != 0) {
        __asm__ __volatile__("lqc2 vf26, 0(%0)\n\t"
                             "lqc2 vf27, 16(%0)\n\t"
                             "lqc2 vf28, 32(%0)\n\t"
                             "lqc2 vf29, 48(%0)"
                             :
                             : "r"(svEffectMatrix));
        vu0TransformPoint(point);
        vu0CurMatrixMul(transform->matrix);
        __asm__ __volatile__("lqc2 vf1, 0(%0)\n\t"
                             "vmulx.xyzw vf26, vf26, vf1x\n\t"
                             "vmuly.xyzw vf27, vf27, vf1y\n\t"
                             "vmulz.xyzw vf28, vf28, vf1z\n\t"
                             "vmulw.xyzw vf29, vf29, vf1w"
                             :
                             : "r"(&svEffectScaleVector));
    } else {
        if (svRecordMatrix != 0) {
            if (ignoreRecordTranslation != 0) {
                __asm__ __volatile__("lqc2 vf26, 0(%0)\n\t"
                                     "lqc2 vf27, 16(%0)\n\t"
                                     "lqc2 vf28, 32(%0)"
                                     :
                                     : "r"(svRecordMatrix));
                __asm__ __volatile__("vmove.xyzw vf29, vf0");
                vu0TransformPoint(point);
            } else {
                __asm__ __volatile__("lqc2 vf26, 0(%0)\n\t"
                                     "lqc2 vf27, 16(%0)\n\t"
                                     "lqc2 vf28, 32(%0)\n\t"
                                     "lqc2 vf29, 48(%0)"
                                     :
                                     : "r"(svRecordMatrix));
                vu0TransformPoint(point);
            }
        } else {
            __asm__ __volatile__("lqc2 vf26, 0(%0)\n\t"
                                 "lqc2 vf27, 16(%0)\n\t"
                                 "lqc2 vf28, 32(%0)"
                                 :
                                 : "r"(vu0IdentityMatrix));
            __asm__ __volatile__("lqc2 vf29, 0(%0)" : : "r"(point));
        }
        vu0CurMatrixMul(transform->matrix);
    }
    if (svFogValue != 0) {
        __asm__ __volatile__("lqc2 vf1, 0(%0)\n\t"
                             "vmulx.xyzw vf26, vf26, vf1x\n\t"
                             "vmuly.xyzw vf27, vf27, vf1y\n\t"
                             "vmulz.xyzw vf28, vf28, vf1z\n\t"
                             "vmulw.xyzw vf29, vf29, vf1w"
                             :
                             : "r"(&transform->scale));
        if (svFogValue >= 0) {
            vu0TransformPoint(&transform->point);
        }
    } else {
        vu0TransformPoint(&transform->point);
        __asm__ __volatile__("lqc2 vf1, 0(%0)\n\t"
                             "vmulx.xyzw vf26, vf26, vf1x\n\t"
                             "vmuly.xyzw vf27, vf27, vf1y\n\t"
                             "vmulz.xyzw vf28, vf28, vf1z\n\t"
                             "vmulw.xyzw vf29, vf29, vf1w"
                             :
                             : "r"(&transform->scale));
    }
    if (useItemRows != 0) {
        MMathQuatToMatrix(rotationRows, &transform->rotation);
        __asm__ __volatile__("lqc2 vf26, 0(%0)\n\t"
                             "lqc2 vf27, 16(%0)\n\t"
                             "lqc2 vf28, 32(%0)"
                             :
                             : "r"(rotationRows));
        __asm__ __volatile__("lqc2 vf1, 0(%0)\n\t"
                             "vmulx.xyzw vf26, vf26, vf1x\n\t"
                             "vmuly.xyzw vf27, vf27, vf1y\n\t"
                             "vmulz.xyzw vf28, vf28, vf1z\n\t"
                             "vmulw.xyzw vf29, vf29, vf1w"
                             :
                             : "r"(&svEffectScaleVector));
        __asm__ __volatile__("lqc2 vf1, 0(%0)\n\t"
                             "vmulx.xyzw vf26, vf26, vf1x\n\t"
                             "vmuly.xyzw vf27, vf27, vf1y\n\t"
                             "vmulz.xyzw vf28, vf28, vf1z\n\t"
                             "vmulw.xyzw vf29, vf29, vf1w"
                             :
                             : "r"(&transform->scale));
        vu0CurMatrixMulRightAffine(sefCameraMatrix[0]);
    }
}

/* Splat labels of the scaffold objects svSetModelRenderState reads (the X3 .data, .sbss, .sdata and .rodata objects). */
extern void *svCurrentVertexViewBlock;
static int svModelDrawFlags;
/* Callees of the other units, prototyped from their call sites. */
void pxyAddRenderLevel(int level);
void nmlModelSetFogDist(float d0, float d1, float d2, float d3);
void nmlModelSetSortOffset(float offset);
void nmlModelSetTexture(TxyBlock *txy);
void nmlModelSetScale(const Vector4 *scale);
void nmlModelSetZwrite(int zwrite);
void nmlModelSetTransparency(float transparency);
void nmlModelSetStencil(int stencil);
void pxySetAnimTime(float animTime);

/* Sets the render state of one model item and submits it: the stencil word from svAlphaTestTable, the scaled colour into the
 * view block, the transparency, the flag bits, the fog override, the sort offset, the scale and the submit. */
void svSetModelRenderState(SvRenderItem *renderItem, Matrix4 *matrix, int scale) {
    u64 stencil;
    const Vector4 *vertexScale;
    AlignedVector4 scaledVertex;
    AlignedVector4 *scaledVertexOut;
    int *itemColor;
    SvVertexViewBlock *vertexViewBlock;
    SefFogOverride *fogOverride;
    int modelDrawFlags;
    float transparency;

    if (renderItem->stencilIndex == 0 && renderItem->transparency == 0x80) {
        stencil = 0;
    } else {
        stencil = svAlphaTestTable[renderItem->stencilIndex];
    }
    scaledVertexOut = &scaledVertex;
    itemColor = renderItem->color;
    vertexScale = svVertexScale;
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(itemColor) : "memory");
    if (vertexScale != 0) {
        __asm__ __volatile__(
            "lqc2 vf2, 0(%0)\n\t"
            "lqc2 vf3, 0(%1)\n\t"
            "vitof0.xyzw vf1, vf1\n\t"
            "vmul.xyzw vf1, vf1, vf2\n\t"
            "vmini.xyz vf1, vf1, vf3\n\t"
            "vftoi0.xyzw vf1, vf1"
            : : "r"(vertexScale), "r"(&svScaledVertexClampMax) : "memory");
    }
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(scaledVertexOut) : "memory");
    vertexViewBlock = svCurrentVertexViewBlock;
    __asm__ __volatile__("li.s $f0, 0.0078125" : : : "$f0");
    __asm__ __volatile__(
        "mfc1 $8, $f0\n\t"
        "qmtc2 $8, vf1\n\t"
        "lqc2 vf2, 0(%0)\n\t"
        "vitof0.xyzw vf2, vf2\n\t"
        "vmulx.xyz vf2, vf2, vf1x\n\t"
        "sqc2 vf2, 0(%1)"
        : : "r"(&scaledVertex), "r"(&vertexViewBlock->scaled) : "$8", "$f0");
    QuadwordCopyT0(&vertexViewBlock->scaledCopy, &vertexViewBlock->scaled);

    if (stencil != 0) {
        nmlModelSetStencil(stencil);
        nmlModelSetStencilAlpha(stencil);
    }
    if (renderItem->part->txyBlock != 0 && renderItem->part->submitBlock != 0) {
        modelDrawFlags = svModelDrawFlags;
        transparency = renderItem->transparency * 0.0078125f;
        pxySetLightCopy(svCurrentVertexViewBlock);
        pxySetAnimTime(renderItem->animTime);
        nmlModelSetTexture(renderItem->part->txyBlock);
        nmlModelSetDrawMatrix(matrix);
        if (svVertexScale != 0) {
            transparency *= svVertexScale->w;
        }
        nmlModelSetTransparency(transparency);
        if ((modelDrawFlags & 1) != 0) {
            pxyAddRenderLevel(4);
        }
        if ((modelDrawFlags & 8) != 0) {
            nmlModelSetZwrite(1);
        } else {
            nmlModelSetZwrite(0);
        }
        if ((modelDrawFlags & 2) != 0) {
            pxyAddRenderLevel(8);
        }
        if ((modelDrawFlags & 0x80) != 0) {
            pxyAddRenderLevel(0x200);
        }
        if ((modelDrawFlags & 0x40) != 0) {
            pxyAddRenderLevel(0x100);
        } else {
            fogOverride = sefGetFogOverrideBlock();
            if (fogOverride->enabled != 0) {
                nmlModelSetFogCol(&fogOverride->fogColor);
                nmlModelSetFogDist(fogOverride->dist[0], fogOverride->dist[1], fogOverride->dist[2], fogOverride->dist[3]);
            }
        }
        if (scale > 0) {
            nmlModelSetSortOffset(scale * -0.1f);
        }
        nmlModelSetScale(&renderItem->scale);
        pxySubmitModel(renderItem->part->submitBlock);
    }
}

/* Projects a homogeneous vertex to the screen: divides by w, flags 0x8000 when the result lies outside the unit cube, then applies the active camera's screen scale and offset. Returns the flags. */
int svProjectVertex(SvScreenVertex *screenVertex, Matrix4 *matrix, const Vector4 *vector) {
    Vector4 clip;
    float invW;
    int clipFlags;
    XglCamera *camera;

    MMathApplyMatrix(&clip, matrix, vector);
    invW = 1.0f / clip.w;
    __asm__ __volatile__(
        "lqc2 vf1, 0(%0)\n\t"
        "mfc1 $8, %1\n\t"
        "qmtc2 $8, vf2\n\t"
        "vmulx.xyz vf1, vf1, vf2x\n\t"
        "sqc2 vf1, 0(%0)"
        : : "r"(&clip), "f"(invW) : "$8", "memory");
    if (clip.x < -1.0f || 1.0f < clip.x || clip.y < -1.0f || 1.0f < clip.y || clip.z < -1.0f || 1.0f < clip.z) {
        clipFlags = 0x8000;
    } else {
        clipFlags = 0;
    }
    screenVertex->clipFlags = clipFlags;
    camera = xglStudioGetActiveCamera();
    __asm__ __volatile__("lqc2 vf1, 0(%0)\n\tlqc2 vf2, 0(%1)" : : "r"(&clip), "r"(&camera->screen.screenScale), "r"(&camera->screen.screenOffset) : "memory");
    __asm__ __volatile__("lqc2 vf3, 0(%0)" : : "r"(&camera->screen.screenOffset) : "memory");
    __asm__ __volatile__("vmula.xyz ACC, vf1, vf2" : : : "memory");
    __asm__ __volatile__("vmaddw.xyz vf1, vf3, vf0w" : : : "memory");
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(&clip) : "memory");
    screenVertex->z = clip.z;
    screenVertex->x = clip.x * SV_SUBPIXEL_STEPS;
    screenVertex->y = clip.y * SV_SUBPIXEL_STEPS;
    return screenVertex->clipFlags;
}

/* Signed distance of a point (its w) from a clip plane whose offset comes from MMathVectorDotProduct. */
f32 sefClipPlaneDistance(Vector4 *plane, Vector4 *point) {
    return -(MMathVectorDotProduct(plane, point) + plane->w);
}

/* Intersects the segment p1-p2 with a plane (VU0 dot products, one divide); writes the crossing point. */
void sefClipEdgeToPlane(Vector4 *crossingPoint, const Vector4 *plane, const Vector4 *edgeStartPoint, const Vector4 *edgeEndPoint) {
    __asm__ __volatile__("vmove.xyzw vf4, vf0" : : : "memory");
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(edgeStartPoint) : "memory");
    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(edgeEndPoint) : "memory");
    __asm__ __volatile__("lqc2 vf3, 0(%0)" : : "r"(plane) : "memory");
    __asm__ __volatile__("vsub.xyz vf4, vf1, vf2" : : : "memory");
    __asm__ __volatile__("vmul.xyz vf5, vf3, vf1" : : : "memory");
    __asm__ __volatile__("vaddy.x vf5, vf5, vf5y" : : : "memory");
    __asm__ __volatile__("vaddz.x vf5, vf5, vf5z" : : : "memory");
    __asm__ __volatile__("vmul.xyz vf6, vf3, vf4" : : : "memory");
    __asm__ __volatile__("vaddy.x vf6, vf6, vf6y" : : : "memory");
    __asm__ __volatile__("vaddz.x vf6, vf6, vf6z" : : : "memory");
    __asm__ __volatile__("vaddw.x vf5, vf5, vf3w" : : : "memory");
    __asm__ __volatile__("vsub.x vf5, vf0, vf5" : : : "memory");
    __asm__ __volatile__("vdiv Q, vf5x, vf6x" : : : "memory");
    __asm__ __volatile__("vwaitq" : : : "memory");
    __asm__ __volatile__("vmulq.xyz vf4, vf4, Q" : : : "memory");
    __asm__ __volatile__("vadd.xyz vf4, vf4, vf1" : : : "memory");
    __asm__ __volatile__("sqc2 vf4, 0(%0)" : : "r"(crossingPoint) : "memory");
}

/* vf20 = the matrix loaded in vf26..vf29 applied to the point at source. */
static inline void sefTransformToVf20(Vector4 *inVector)
{
    __asm__ __volatile__("lqc2 vf1, 0(%0)\n"
                         "vmulax.xyzw ACC, vf26, vf1x\n"
                         "vmadday.xyzw ACC, vf27, vf1y\n"
                         "vmaddaz.xyzw ACC, vf28, vf1z\n"
                         "vmaddw.xyzw vf20, vf29, vf1w\n"
                         : : "r"(inVector) : "memory");
}

/* Stores vf20 to destination. */
static inline void sefStoreVf20(Vector4 *outVector)
{
    __asm__ __volatile__("sqc2 vf20, 0(%1)\n" : "=m"(outVector->w) : "r"(outVector) : "memory");
}

/* destination.xyz = minuend.xyz - subtrahend.xyz; the outputs are the four components of destination. */
#define SefSubVectorXYZ(destination, minuend, subtrahend) \
    __asm__ __volatile__("lqc2 vf1, 0(%5)\n" \
                         "lqc2 vf2, 0(%6)\n" \
                         "vsub.xyz vf1, vf1, vf2\n" \
                         "sqc2 vf1, 0(%4)\n" \
                         : "=m"((destination)->x), "=m"((destination)->y), "=m"((destination)->z), "=m"((destination)->w) \
                         : "r"(destination), "r"(minuend), "r"(subtrahend) : "memory")
/* *destination = *source through $8; the outputs are the four components of destination. */
#define SefCopyVector(destination, source) \
    __asm__ __volatile__("lq $8, 0(%5)\n" \
                         "sq $8, 0(%4)\n" \
                         : "=m"((destination)->x), "=m"((destination)->y), "=m"((destination)->z), "=m"((destination)->w) \
                         : "r"(destination), "r"(source) : "$8", "memory")

/* Clips the draw state quad, transformed by matrix, against the first five planes of the active camera's view
 * volume (Sutherland-Hodgman, two vertex buffers). Each surviving vertex goes to out as a pair: the position
 * with w = 1, then svUnitQuadCornerMatrix with x and y replaced by the position's projections on the quad's two edges.
 * Returns the vertex count, 0 when the quad is degenerate or clipped away. */
int sefClipPolygon(void *drawSlot, Matrix4 *matrix, SvClipVertex *outVertices)
{
    int vertexCounts[2];
    float edgeLengths[2];
    Vector4 corners[4];
    Vector4 edges[2];
    SefClipVertex clipBuffers[2][9];
    Vector4 cornerOffset;
    int planeIndex;
    XglCamera *camera;
    DrawStatePacket *drawPacket;
    Vector4 *planeVector;
    Vector4 *origin;
    Vector4 *outVertex;
    int clipBufferIndex;
    int nextClipBufferIndex;
    int packetCorner;
    int clipCount;
    int i;
    int nextVertex;

    camera = xglStudioGetActiveCamera();
    clipBufferIndex = 0;
    vertexCounts[1] = 0;
    vertexCounts[0] = 0;
    __asm__ __volatile__("lqc2 vf26, 0(%0)\n"
                         "lqc2 vf27, 16(%0)\n"
                         "lqc2 vf28, 32(%0)\n"
                         "lqc2 vf29, 48(%0)\n"
                         : : "r"(matrix) : "memory");
    drawPacket = svDrawStatePacket;
    /* the loop reads the packet corners in the order 0, 1, 3, 2 */
    for (i = 0; i < 4; i++) {
        if (i == 2) {
            packetCorner = 3;
        } else if (i == 3) {
            packetCorner = 2;
        } else {
            packetCorner = i;
        }
        sefTransformToVf20(&drawPacket->corners[packetCorner]);
        sefStoreVf20(&corners[i]);
        sefStoreVf20(&clipBuffers[clipBufferIndex][i].vector);
    }
    vertexCounts[clipBufferIndex] = 4;
    origin = &corners[0];
    SefSubVectorXYZ(&edges[0], &corners[1], origin);
    SefSubVectorXYZ(&edges[1], &corners[3], origin);
    edgeLengths[0] = MMathVectorDotProduct(&edges[0], &edges[0]);
    edgeLengths[1] = MMathVectorDotProduct(&edges[1], &edges[1]);
    if (edgeLengths[0] == 0.0f || edgeLengths[1] == 0.0f) {
        return 0;
    }
    for (planeIndex = 0; planeIndex < 5; planeIndex++) {
        planeVector = &camera->volume[0].plane[planeIndex].vector;
        nextClipBufferIndex = clipBufferIndex ^ 1;
        for (i = 0; i < vertexCounts[clipBufferIndex]; i++) {
            clipBuffers[clipBufferIndex][i].element[3] = sefClipPlaneDistance(planeVector, &clipBuffers[clipBufferIndex][i].vector);
        }
        clipCount = 0;
        for (i = 0; i < vertexCounts[clipBufferIndex]; i++) {
            nextVertex = (i + 1) % vertexCounts[clipBufferIndex];
            if (clipBuffers[clipBufferIndex][i].element[3] >= 0.0f) {
                clipBuffers[nextClipBufferIndex][clipCount++] = clipBuffers[clipBufferIndex][i];
                if (clipBuffers[clipBufferIndex][nextVertex].element[3] < 0.0f) {
                    sefClipEdgeToPlane(&clipBuffers[nextClipBufferIndex][clipCount++].vector, planeVector, &clipBuffers[clipBufferIndex][i].vector, &clipBuffers[clipBufferIndex][nextVertex].vector);
                }
            } else if (clipBuffers[clipBufferIndex][nextVertex].element[3] >= 0.0f) {
                sefClipEdgeToPlane(&clipBuffers[nextClipBufferIndex][clipCount++].vector, planeVector, &clipBuffers[clipBufferIndex][i].vector, &clipBuffers[clipBufferIndex][nextVertex].vector);
            }
        }
        vertexCounts[nextClipBufferIndex] = clipCount;
        clipBufferIndex = nextClipBufferIndex;
        if (clipCount < 3) {
            return 0;
        }
    }
    for (i = 0; i < vertexCounts[clipBufferIndex]; i++) {
        outVertex = &outVertices[i].position;
        __asm__ __volatile__("lq $8, %5\n"
                             "sq $8, 0(%4)\n"
                             : "=m"(outVertex[0].x), "=m"(outVertex[0].y), "=m"(outVertex[0].z), "=m"(outVertex[0].w)
                             : "r"(&outVertex[0]), "m"(*(&clipBuffers[clipBufferIndex][i])) : "$8", "memory");
        outVertex[0].w = 1.0f;
        SefCopyVector(&outVertex[1], &svUnitQuadCornerMatrix);
        SefSubVectorXYZ(&cornerOffset, &outVertex[0], origin);
        outVertex[1].x = MMathVectorDotProduct(&cornerOffset, &edges[0]) / edgeLengths[0];
        outVertex[1].y = MMathVectorDotProduct(&cornerOffset, &edges[1]) / edgeLengths[1];
    }
    return vertexCounts[clipBufferIndex];
}

/* One polygon item as svDrawPolygonItem reads it: the matrix at +0x00, the scale at +0xC0,
 * the colour at +0x130, the base and overlay parts at +0x158 and +0x15C, the texture offsets at +0x160 and the overlay
 * offsets at +0x168, the alpha table index at +0x1FC and the point at +0x200. */
typedef struct SvPolygonRecord {
    Matrix4 matrix;
    unsigned char unmodeled_040[0xC0 - 0x40];
    Vector4 scale;
    unsigned char unmodeled_0D0[0x130 - 0xD0];
    int color[4];
    unsigned char unmodeled_140[0x158 - 0x140];
    SvPolygonPart *part;
    SvPolygonPart *overlayPart;
    float texOffsetX;
    float texOffsetY;
    float overlayOffsetX;
    float overlayOffsetY;
    unsigned char unmodeled_170[0x1FC - 0x170];
    int drawIndex;
    Vector4 point;
} SvPolygonRecord;

/* Scaffold data of svDrawPolygonItem by splat names (gp-relative words: the clip flag, the record and view matrix pointers, the
 * axes mode, the vertex scale pointer, the fog value above; the two alpha tables are 64-bit words). */
extern int D_005510D4;
static Matrix4 *svBattleLinkedMatrix;
static int svPolygonAxesMode;
extern SefPartTransform gSdvCameraPartTransform;

/* The next free vertex slot of svDrawList, where a polygon packet starts. */
static inline SvPolygonVertex *svPolygonPacketBegin(void) {
    return (SvPolygonVertex *) (svDrawList.entries + svDrawList.end_index);
}

/* Transforms a polygon item's vertices by the item matrix, the axes and the scale, clips the polygon with
 * sefClipPolygon and appends its textured packet to svDrawList: an overlay packet and then the base packet when
 * D_005510D4 is set, one packet otherwise. */
void svDrawPolygonItem(SvPolygonRecord *polygonItem, Vector4 *position, SvDrawCall *drawCall, int scaled) {
    Vector4 *axesPointer;
    Matrix4 matrix;
    int clipCount;
    Vector4 axes;
    Quadword vertexColor;
    SvClipVertex clipVertices[9];
    int *itemColor;
    Quadword *colorStore;
    SvPolygonPacket *packet;
    const Vector4 *vertexScale;
    SvPolygonVertex *vertexCursor;
    SvPolygonPart *polygonPart;
    int i;
    int axesMode;
    float texCoordOffsetX;

    vertexCursor = svPolygonPacketBegin();
    polygonPart = polygonItem->part;
    __asm__ __volatile__("lqc2 vf26, 0(%0)\n\t"
                         "lqc2 vf27, 16(%0)\n\t"
                         "lqc2 vf28, 32(%0)"
                         :
                         : "r"(vu0IdentityMatrix));
    __asm__ __volatile__("lqc2 vf29, 0(%0)" : : "r"(position));
    vu0CurMatrixMul(polygonItem->matrix);
    if (gSdvCameraPartTransform.scaled != 0) {
        __asm__ __volatile__("mfc1 $8, %0\n\t"
                             "qmtc2 $8, vf1\n\t"
                             "vmulx.xyzw vf26, vf26, vf1x\n\t"
                             "vmulx.xyzw vf27, vf27, vf1x\n\t"
                             "vmulx.xyzw vf28, vf28, vf1x"
                             :
                             : "f"(gSdvCameraPartTransform.scale)
                             : "$8", "memory");
    }
    if (svFogValue != 0) {
        __asm__ __volatile__("lqc2 vf1, 0(%0)\n\t"
                             "vmulx.xyzw vf26, vf26, vf1x\n\t"
                             "vmuly.xyzw vf27, vf27, vf1y\n\t"
                             "vmulz.xyzw vf28, vf28, vf1z\n\t"
                             "vmulw.xyzw vf29, vf29, vf1w"
                             :
                             : "r"(&polygonItem->scale));
        if (svFogValue >= 0) {
            vu0TransformPoint(&polygonItem->point);
        }
    } else {
        vu0TransformPoint(&polygonItem->point);
        __asm__ __volatile__("lqc2 vf1, 0(%0)\n\t"
                             "vmulx.xyzw vf26, vf26, vf1x\n\t"
                             "vmuly.xyzw vf27, vf27, vf1y\n\t"
                             "vmulz.xyzw vf28, vf28, vf1z\n\t"
                             "vmulw.xyzw vf29, vf29, vf1w"
                             :
                             : "r"(&polygonItem->scale));
    }
    axesPointer = &axes;
    __asm__ __volatile__("vmaxw.xyzw vf1, vf0, vf0w\n\tsqc2 vf1, 0(%4)"
                         : "=m"(axesPointer->x), "=m"(axesPointer->y), "=m"(axesPointer->z), "=m"(axesPointer->w)
                         : "r"(axesPointer));
    axesMode = svPolygonAxesMode;
    if (axesMode == 5) {
        axes.x = polygonPart->scaleX;
        axes.y = polygonPart->scaleY;
    } else if (axesMode == 7) {
        axes.x = polygonPart->scaleX;
        axes.z = polygonPart->scaleY;
    } else {
        axes.y = polygonPart->scaleX;
        axes.z = polygonPart->scaleY;
    }
    __asm__ __volatile__("lqc2 vf1, 0(%0)\n\t"
                         "vmulx.xyzw vf26, vf26, vf1x\n\t"
                         "vmuly.xyzw vf27, vf27, vf1y\n\t"
                         "vmulz.xyzw vf28, vf28, vf1z\n\t"
                         "vmulw.xyzw vf29, vf29, vf1w"
                         :
                         : "r"(&axes)
                         : "memory");
    if (svRecordMatrix != 0) {
        vu0CurMatrixMulRight((Vector4 *) svRecordMatrix);
    }
    if (svBattleLinkedMatrix != 0) {
        vu0CurMatrixMulRight((Vector4 *) svBattleLinkedMatrix);
    }
    __asm__ __volatile__("sqc2 vf26, 0(%0)\n\t"
                         "sqc2 vf27, 16(%0)\n\t"
                         "sqc2 vf28, 32(%0)\n\t"
                         "sqc2 vf29, 48(%0)"
                         :
                         : "r"(matrix)
                         : "memory");
    clipCount = sefClipPolygon(polygonItem, &matrix, clipVertices);
    if (clipCount == 0) {
        return;
    }

    colorStore = &vertexColor;
    itemColor = polygonItem->color;
    vertexScale = svVertexScale;
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(itemColor) : "memory");
    if (vertexScale != 0) {
        __asm__ __volatile__("lqc2 vf2, 0(%0)\n\t"
                             "lqc2 vf3, 0(%1)\n\t"
                             "vitof0.xyzw vf1, vf1\n\t"
                             "vmul.xyzw vf1, vf1, vf2\n\t"
                             "vmini.xyz vf1, vf1, vf3\n\t"
                             "vftoi0.xyzw vf1, vf1"
                             :
                             : "r"(vertexScale), "r"(&svScaledVertexClampMax)
                             : "memory");
    }
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(colorStore) : "memory");
    if (D_005510D4 != 0) {
        packet = (SvPolygonPacket *) vertexCursor;
        packet->gif.words[0] = 0x1000000000008004ULL;
        packet->gif.words[1] = GIF_AD;
        packet->tex.words[0] = polygonItem->overlayPart->tex0;
        packet->tex.words[1] = GS_TEX0_1;
        packet->test.words[0] = 0x31003;
        packet->test.words[1] = GS_TEST_1;
        packet->alpha.words[0] = 0x4A;
        packet->alpha.words[1] = GS_ALPHA_1;
        packet->clamp = *(Quadword *) &svDrawStatePacket->clamp;
        packet->vertexTag.words[0] = ((u64) 0xC0AB << 46) | (clipCount | 0x8000);
        packet->vertexTag.words[1] = 0x512;
        vertexCursor = (SvPolygonVertex *) (packet + 1);
        for (i = 0; i < clipCount; i++) {
            vertexCursor->texCoord.quad = *(Quadword *) &clipVertices[i].texCoord;
            texCoordOffsetX = polygonItem->overlayOffsetX;
            vertexCursor->texCoord.vector.x += texCoordOffsetX;
            vertexCursor->texCoord.vector.y += polygonItem->overlayOffsetY;
            vertexCursor->color = vertexColor;
            vertexCursor->position = *(Quadword *) &clipVertices[i].position;
            vertexCursor++;
        }
        packet = (SvPolygonPacket *) vertexCursor;
        packet->gif.words[0] = 0x1000000000008004ULL;
        packet->gif.words[1] = GIF_AD;
        packet->tex.words[0] = polygonItem->part->tex0;
        packet->tex.words[1] = GS_TEX0_1;
        packet->test.words[0] = svZTestWord;
        packet->test.words[1] = GS_TEST_1;
        packet->alpha.words[0] = svPairAlphaTable[polygonItem->drawIndex];
        packet->alpha.words[1] = GS_ALPHA_1;
        packet->clamp = *(Quadword *) &svDrawStatePacket->clamp;
        packet->vertexTag.words[0] = ((u64) 0xC0AB << 46) | (clipCount | 0x8000);
        packet->vertexTag.words[1] = 0x512;
        vertexCursor = (SvPolygonVertex *) (packet + 1);
        for (i = 0; i < clipCount; i++) {
            vertexCursor->texCoord.quad = *(Quadword *) &clipVertices[i].texCoord;
            texCoordOffsetX = polygonItem->texOffsetX;
            vertexCursor->texCoord.vector.x += texCoordOffsetX;
            vertexCursor->texCoord.vector.y += polygonItem->texOffsetY;
            vertexCursor->color = vertexColor;
            vertexCursor->position = *(Quadword *) &clipVertices[i].position;
            vertexCursor++;
        }
        svDrawList.end_index += (unsigned int) ((clipCount * 3 + 6) * 2);
    } else {
        packet = (SvPolygonPacket *) vertexCursor;
        packet->gif.words[0] = 0x1000000000008004ULL;
        packet->gif.words[1] = GIF_AD;
        packet->tex.words[0] = polygonItem->part->tex0;
        packet->tex.words[1] = GS_TEX0_1;
        packet->test.words[0] = svZTestWord;
        packet->test.words[1] = GS_TEST_1;
        packet->alpha.words[0] = svAlphaTestTable[polygonItem->drawIndex];
        packet->alpha.words[1] = GS_ALPHA_1;
        packet->clamp = *(Quadword *) &svDrawStatePacket->clamp;
        packet->vertexTag.words[0] = ((u64) 0xC0AB << 46) | (clipCount | 0x8000);
        packet->vertexTag.words[1] = 0x512;
        vertexCursor = (SvPolygonVertex *) (packet + 1);
        for (i = 0; i < clipCount; i++) {
            vertexCursor->texCoord.quad = *(Quadword *) &clipVertices[i].texCoord;
            texCoordOffsetX = polygonItem->texOffsetX;
            vertexCursor->texCoord.vector.x += texCoordOffsetX;
            vertexCursor->texCoord.vector.y += polygonItem->texOffsetY;
            vertexCursor->color = vertexColor;
            vertexCursor->position = *(Quadword *) &clipVertices[i].position;
            vertexCursor++;
        }
        svDrawList.end_index += (unsigned int) (clipCount * 3 + 6);
    }
}

static int D_00551544;

void svDrawListAppendRecordB(SvDrawState *drawState, Vector4 *point, SvDrawCall *drawCall, int useScale);

/* Loads the integer vertex, scales and clamps it by the optional scale vector, and stores it. */
static inline void svScaleVertex(int *vertexDst, const int *vertexSrc) {
    const Vector4 *vertexScale = svVertexScale;

    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(vertexSrc) : "memory");
    if (vertexScale != 0) {
        __asm__ __volatile__("lqc2 vf2, 0(%0)\n\t"
                             "lqc2 vf3, 0(%1)\n\t"
                             "vitof0.xyzw vf1, vf1\n\t"
                             "vmul.xyzw vf1, vf1, vf2\n\t"
                             "vmini.xyz vf1, vf1, vf3\n\t"
                             "vftoi0.xyzw vf1, vf1"
                             :
                             : "r"(vertexScale), "r"(&svScaledVertexClampMax)
                             : "memory");
    }
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(vertexDst));
}

/* Appends one kind-B draw record (17 quadwords, end_index += 17): the GIF tag, the texture and test words, the position,
 * the prim, the vertex, the coordinates placed by svDrawMode, and the scaled matrix rows. */
void svDrawListAppendRecordB(SvDrawState *drawState, Vector4 *point, SvDrawCall *drawCall, int useScale) {
    SvDrawRecordB *drawRecord;
    Vector4 *positionDst;

    drawRecord = (SvDrawRecordB *) (svDrawList.entries + svDrawList.end_index);
    drawRecord->gif.words[0] = 0x002A400000008001ULL;
    drawRecord->gif.words[1] = 0x512512512512EEEEULL;
    drawRecord->texture.words[0] = drawState->uv->tex0;
    drawRecord->texture.words[1] = GS_TEX0_1;
    drawRecord->test.words[0] = svZTestWord;
    drawRecord->test.words[1] = GS_TEST_1;
    positionDst = &drawRecord->position;
    drawRecord->alpha.words[0] = svAlphaTestTable[drawState->tableIndex];
    drawRecord->alpha.words[1] = GS_ALPHA_1;
    QuadwordCopyT0(positionDst, point);
    QuadwordCopyT0(&drawRecord->prim, &drawState->primaryPrim);
    svScaleVertex((int *) &drawRecord->vertex, drawState->vertex);
    QuadwordCopyT0(&drawRecord->extraState, &drawState->extraState);
    if (svDrawMode == 5) {
        drawRecord->coords.vector.x = drawState->uv->u;
        drawRecord->coords.vector.y = drawState->uv->v;
        drawRecord->coords.vector.w = 1.0f;
        drawRecord->coords.vector.z = 0.0f;
    } else if (svDrawMode == 7) {
        drawRecord->coords.vector.x = drawState->uv->u;
        drawRecord->coords.vector.z = drawState->uv->v;
        drawRecord->coords.vector.w = 1.0f;
        drawRecord->coords.vector.y = 0.0f;
    } else {
        drawRecord->coords.vector.y = drawState->uv->u;
        drawRecord->coords.vector.z = drawState->uv->v;
        drawRecord->coords.vector.w = 1.0f;
        drawRecord->coords.vector.x = 0.0f;
    }
    if ((drawState->uv->flags & 0x10) != 0) {
        __asm__ __volatile__("vmaxw.xyzw vf1, vf0, vf0w\n\tsqc2 vf1, 0(%0)"
                             :
                             : "r"(&drawRecord->quad)
                             : "memory");
    } else {
        QuadwordCopyT0(&drawRecord->quad, drawCall->quad);
    }
    __asm__ __volatile__("lqc2 vf26, 0(%0)\n\tlqc2 vf27, 16(%0)\n\tlqc2 vf28, 32(%0)\n\tlqc2 vf29, 48(%0)"
                         :
                         : "r"(drawState->matrix)
                         : "memory");
    if (D_00551544 != 0) {
        __asm__ __volatile__(
            "mfc1 $8, %0\n\tqmtc2 $8, vf1\n\tvmulx.xyz vf26, vf26, vf1x\n\tvmulx.xyz vf27, vf27, vf1x\n\tvmulx.xyz vf28, vf28, vf1x"
            :
            : "f"(gSdvCameraPartTransform.scale)
            : "$8", "memory");
    }
    if (useScale != 0) {
        __asm__ __volatile__(
            "lqc2 vf1, 0(%0)\n\tvmulx.xyz vf26, vf26, vf1x\n\tvmuly.xyz vf27, vf27, vf1y\n\tvmulz.xyz vf28, vf28, vf1z"
            :
            : "r"(&svEffectScaleVector)
            : "memory");
    }
    __asm__ __volatile__("sqc2 vf26, 0(%0)\n\tsqc2 vf27, 16(%0)\n\tsqc2 vf28, 32(%0)\n\tsqc2 vf29, 48(%0)"
                         :
                         : "r"(drawRecord->matrix)
                         : "memory");
    svDrawList.end_index += 17;
}

/* Appends a pair of kind-B draw records (34 quadwords). */
void svDrawListAppendRecordBPair(SvDrawState *drawState, Vector4 *point, SvDrawCall *drawCall, int useScale) {
    Vector4 *positionDst;
    SvDrawRecordB *drawRecord;

    drawRecord = (SvDrawRecordB *) (svDrawList.entries + svDrawList.end_index);
    drawRecord[0].gif.words[0] = 0x002A400000008001ULL;
    drawRecord[0].gif.words[1] = 0x512512512512EEEEULL;
    drawRecord[0].texture.words[0] = drawState->texture->tex0;
    drawRecord[1].texture.words[0] = drawState->uv->tex0;
    drawRecord[1].texture.words[1] = GS_TEX0_1;
    drawRecord[1].test.words[1] = GS_TEST_1;
    drawRecord[0].texture.words[1] = GS_TEX0_1;
    drawRecord[0].test.words[0] = 0x31003;
    drawRecord[1].test.words[0] = svZTestWord;
    drawRecord[0].test.words[1] = GS_TEST_1;
    drawRecord[0].alpha.words[0] = 0x4A;
    drawRecord[1].alpha.words[0] = svPairAlphaTable[drawState->tableIndex];
    drawRecord[1].alpha.words[1] = GS_ALPHA_1;
    drawRecord[0].alpha.words[1] = GS_ALPHA_1;
    positionDst = &drawRecord[0].position;
    __asm__ __volatile__("lq $8, 0(%3)\nsq $8, 0(%2)\n"
                         : "+m"(*positionDst), "+m"(*point)
                         : "r"(positionDst), "r"(point), "m"(*positionDst)
                         : "$8", "memory");
    drawRecord[0].prim.data = drawState->secondaryPrim;
    drawRecord[0].prim.address = 0;
    drawRecord[1].prim.data = drawState->primaryPrim;
    drawRecord[1].prim.address = 0;
    svScaleVertex((int *) &drawRecord[0].vertex, drawState->vertex);
    QuadwordCopyT0(&drawRecord[0].extraState, &drawState->extraState);
    if (svDrawMode == 6) {
        drawRecord[0].coords.vector.x = drawState->uv->u;
        drawRecord[0].coords.vector.y = drawState->uv->v;
        drawRecord[0].coords.vector.w = 1.0f;
        drawRecord[0].coords.vector.z = 0.0f;
    } else if (svDrawMode == 8) {
        drawRecord[0].coords.vector.x = drawState->uv->u;
        drawRecord[0].coords.vector.z = drawState->uv->v;
        drawRecord[0].coords.vector.w = 1.0f;
        drawRecord[0].coords.vector.y = 0.0f;
    } else {
        drawRecord[0].coords.vector.y = drawState->uv->u;
        drawRecord[0].coords.vector.z = drawState->uv->v;
        drawRecord[0].coords.vector.w = 1.0f;
        drawRecord[0].coords.vector.x = 0.0f;
    }
    QuadwordCopyT0(&drawRecord[0].quad, drawCall->quad);
    __asm__ __volatile__("lqc2 vf26, 0(%0)\n"
                         "lqc2 vf27, 16(%0)\n"
                         "lqc2 vf28, 32(%0)\n"
                         "lqc2 vf29, 48(%0)\n"
                         :
                         : "r"(drawState->matrix));
    if (D_00551544 != 0) {
        __asm__ __volatile__("mfc1 $8, %0\n"
                             "qmtc2 $8, vf1\n"
                             "vmulx.xyz vf26, vf26, vf1x\n"
                             "vmulx.xyz vf27, vf27, vf1x\n"
                             "vmulx.xyz vf28, vf28, vf1x\n"
                             :
                             : "f"(gSdvCameraPartTransform.scale)
                             : "$8", "memory");
    }
    if (useScale != 0) {
        __asm__ __volatile__("lqc2 vf1, 0(%0)\n"
                             "vmulx.xyz vf26, vf26, vf1x\n"
                             "vmuly.xyz vf27, vf27, vf1y\n"
                             "vmulz.xyz vf28, vf28, vf1z\n"
                             :
                             : "r"(&svEffectScaleVector));
    }
    if (svBattleLinkedMatrix != 0) {
        vu0CurMatrixMulRight((Vector4 *) svBattleLinkedMatrix);
    }
    __asm__ __volatile__("sqc2 vf26, 0(%0)\n"
                         "sqc2 vf27, 16(%0)\n"
                         "sqc2 vf28, 32(%0)\n"
                         "sqc2 vf29, 48(%0)\n"
                         :
                         : "r"(drawRecord[0].matrix)
                         : "memory");
    svDrawList.end_index += 34;
}

/* Appends one part packet to svDrawList from an SvDrawState: the transform rows (scaled by svEffectScaleVector when scaleRows is set), the position, the extents and the source. */ void svDrawListAppendPart(SvDrawState *drawState, const Vector4 *vertex, SvDrawCall *ref, int scaleRows);

/* Appends one kind-A draw record (14 quadwords). */
void svDrawListAppendRecordA(SvDrawState *drawState, Vector4 *point, SvDrawCall *drawCall, int useFixedMatrix) {
    Vector4 *drawStateExtra;
    Vector4 *drawRecordExtra;
    DoublewordVector4 *coordsDst;
    SvDrawRecordA *drawRecord;
    float zRotation;
    DrawStatePacket *statePacket;

    drawRecord = (SvDrawRecordA *) (svDrawList.entries + svDrawList.end_index);
    drawRecord->gif.words[0] = 0xD02A400000008001ULL;
    drawRecord->gif.words[1] = 0x000525252521EEEEULL;
    drawRecord->texture.words[0] = drawState->uv->tex0;
    drawRecord->texture.words[1] = GS_TEX0_1;
    drawRecord->test.words[0] = svZTestWord;
    drawRecord->test.words[1] = GS_TEST_1;
    drawRecord->alpha.words[0] = svAlphaTestTable[drawState->tableIndex];
    drawRecord->alpha.words[1] = GS_ALPHA_1;
    svScaleVertex(drawRecord->vertex.words, drawState->vertex);
    QuadwordCopyT0(&drawRecord->prim, &drawState->primaryPrim);
    drawRecord->prim.address = 0;
    drawRecordExtra = &drawRecord->extraState;
    drawStateExtra = &drawState->extraState;
    __asm__ __volatile__("lq $8, 0(%2)\nsq $8, 0(%1)\n"
                         : "+m"(*drawStateExtra)
                         : "r"(drawRecordExtra), "r"(drawStateExtra)
                         : "$8", "memory");
    coordsDst = &drawRecord->coords;
    __asm__ __volatile__("lq $8, 0(%3)\nsq $8, 0(%2)\n"
                         : "=m"(*coordsDst), "+m"(*drawState->uv)
                         : "r"(coordsDst), "r"(drawState->uv)
                         : "$8", "memory");
    drawRecord->coords.words[1] = 0x3F80000000000000ULL;
    if ((drawState->uv->flags & 0x10) != 0) {
        __asm__ __volatile__("vmaxw.xyzw vf1, vf0, vf0w\n"
                             "sqc2 vf1, 0(%0)\n"
                             :
                             : "r"(&drawRecord->quad)
                             : "memory");
    } else {
        QuadwordCopyT0(&drawRecord->quad, drawCall->quad);
    }
    if (useFixedMatrix != 0) {
        __asm__ __volatile__("lqc2 vf26, 0(%0)\n"
                             "lqc2 vf27, 16(%0)\n"
                             "lqc2 vf28, 32(%0)\n"
                             "lqc2 vf29, 48(%0)\n"
                             :
                             : "r"(&svEffectMatrix));
        vu0TransformPoint(point);
        vu0CurMatrixMul(drawState->matrix);
        __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(&svEffectScaleVector));
        __asm__ __volatile__("vmulx.xyzw vf26, vf26, vf1x\n"
                             "vmuly.xyzw vf27, vf27, vf1y\n"
                             "vmulz.xyzw vf28, vf28, vf1z\n"
                             "vmulw.xyzw vf29, vf29, vf1w\n"
                             :
                             :);
    } else {
        if (svRecordMatrix != 0) {
            __asm__ __volatile__("lqc2 vf26, 0(%0)\n"
                                 "lqc2 vf27, 16(%0)\n"
                                 "lqc2 vf28, 32(%0)\n"
                                 "lqc2 vf29, 48(%0)\n"
                                 :
                                 : "r"(svRecordMatrix));
            vu0TransformPoint(point);
        } else {
            __asm__ __volatile__("lqc2 vf26, 0(%0)\n"
                                 "lqc2 vf27, 16(%0)\n"
                                 "lqc2 vf28, 32(%0)\n"
                                 "lqc2 vf29, 0(%1)\n"
                                 :
                                 : "r"(vu0IdentityMatrix), "r"(point));
        }
        vu0CurMatrixMul(drawState->matrix);
    }
    if (D_00551544 != 0) {
        __asm__ __volatile__("mfc1 $8, %0\n"
                             "qmtc2 $8, vf1\n"
                             "vmulx.xyzw vf26, vf26, vf1x\n"
                             "vmulx.xyzw vf27, vf27, vf1x\n"
                             "vmulx.xyzw vf28, vf28, vf1x\n"
                             :
                             : "f"(gSdvCameraPartTransform.scale)
                             : "$8", "memory");
    }
    if (svFogValue != 0) {
        __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(drawCall->quad));
        __asm__ __volatile__("vmulx.xyzw vf26, vf26, vf1x\n"
                             "vmuly.xyzw vf27, vf27, vf1y\n"
                             "vmulz.xyzw vf28, vf28, vf1z\n"
                             "vmulw.xyzw vf29, vf29, vf1w\n"
                             :
                             :);
        if (svFogValue >= 0) {
            vu0TransformPoint(drawStateExtra);
        }
    } else {
        vu0TransformPoint(drawStateExtra);
        __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(drawCall->quad));
        __asm__ __volatile__("vmulx.xyzw vf26, vf26, vf1x\n"
                             "vmuly.xyzw vf27, vf27, vf1y\n"
                             "vmulz.xyzw vf28, vf28, vf1z\n"
                             "vmulw.xyzw vf29, vf29, vf1w\n"
                             :
                             :);
    }
    if (svBattleLinkedMatrix != 0) {
        vu0CurMatrixMulRight((Vector4 *) svBattleLinkedMatrix);
    }
    __asm__ __volatile__("sqc2 vf29, 0(%0)" : : "r"(&drawRecord->position) : "memory");
    zRotation = drawState->angleZ + drawState->angleZOffset;
    statePacket = svDrawStatePacket;
    __asm__ __volatile__("lqc2 vf26, 0(%0)\n"
                         "lqc2 vf27, 16(%0)\n"
                         "lqc2 vf28, 32(%0)\n"
                         "lqc2 vf29, 48(%0)\n"
                         :
                         : "r"(statePacket->baseMatrix));
    if (zRotation != 0.0f) {
        vu0RotateCurrentZ(zRotation);
    }
    __asm__ __volatile__("sqc2 vf26, 0(%0)\n"
                         "sqc2 vf27, 16(%0)\n"
                         "sqc2 vf28, 32(%0)\n"
                         "sqc2 vf29, 48(%0)\n"
                         :
                         : "r"(drawRecord->matrix)
                         : "memory");
    svDrawList.end_index += 14;
}

/* Appends a pair of kind-A draw records (28 quadwords). */
void svDrawListAppendRecordAPair(SvDrawState *drawState, Vector4 *point, SvDrawCall *drawCall, int useFixedMatrix) {
    Vector4 *drawStateExtra;
    Vector4 *drawRecordExtra;
    DoublewordVector4 *coordsDst;
    SvDrawRecordA *drawRecord;
    float zRotation;
    DrawStatePacket *statePacket;

    drawRecord = (SvDrawRecordA *) (svDrawList.entries + svDrawList.end_index);
    drawRecord[0].gif.words[0] = 0xD02A400000008001ULL;
    drawRecord[0].gif.words[1] = 0x000525252521EEEEULL;
    drawRecord[0].texture.words[0] = drawState->texture->tex0;
    drawRecord[1].texture.words[0] = drawState->uv->tex0;
    drawRecord[1].texture.words[1] = GS_TEX0_1;
    drawRecord[1].test.words[1] = GS_TEST_1;
    drawRecord[0].texture.words[1] = GS_TEX0_1;
    drawRecord[0].test.words[0] = SV_TEST_FIXED_DATA;
    drawRecord[1].test.words[0] = svZTestWord;
    drawRecord[0].test.words[1] = GS_TEST_1;
    drawRecord[0].alpha.words[0] = SV_ALPHA_FIXED_DATA;
    drawRecord[1].alpha.words[0] = svPairAlphaTable[drawState->tableIndex];
    drawRecord[1].alpha.words[1] = GS_ALPHA_1;
    drawRecord[0].alpha.words[1] = GS_ALPHA_1;
    svScaleVertex(drawRecord[0].vertex.words, drawState->vertex);
    drawRecord[1].vertex.quad = drawRecord[0].vertex.quad;
    drawRecord[0].prim.data = drawState->secondaryPrim;
    drawRecord[0].prim.address = 0;
    drawRecord[1].prim.data = drawState->primaryPrim;
    drawRecord[1].prim.address = 0;
    drawRecordExtra = &drawRecord[0].extraState;
    drawStateExtra = &drawState->extraState;
    __asm__ __volatile__("lq $8, 0(%3)\nsq $8, 0(%2)\n"
                         : "+m"(*drawRecordExtra), "+m"(*drawStateExtra)
                         : "r"(drawRecordExtra), "r"(drawStateExtra)
                         : "$8", "memory");
    coordsDst = &drawRecord[0].coords;
    __asm__ __volatile__("lq $8, 0(%3)\nsq $8, 0(%2)\n"
                         : "=m"(*coordsDst), "+m"(*drawState->uv)
                         : "r"(coordsDst), "r"(drawState->uv)
                         : "$8", "memory");
    drawRecord[0].coords.words[1] = 0x3F80000000000000ULL;
    if ((drawState->uv->flags & 0x10) != 0) {
        __asm__ __volatile__("vmaxw.xyzw vf1, vf0, vf0w\n"
                             "sqc2 vf1, 0(%0)\n"
                             :
                             : "r"(&drawRecord[0].quad)
                             : "memory");
    } else {
        QuadwordCopyT0(&drawRecord[0].quad, drawCall->quad);
    }
    if (useFixedMatrix != 0) {
        __asm__ __volatile__("lqc2 vf26, 0(%0)\n"
                             "lqc2 vf27, 16(%0)\n"
                             "lqc2 vf28, 32(%0)\n"
                             "lqc2 vf29, 48(%0)\n"
                             :
                             : "r"(&svEffectMatrix));
        vu0TransformPoint(point);
        vu0CurMatrixMul(drawState->matrix);
        __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(&svEffectScaleVector));
        __asm__ __volatile__("vmulx.xyzw vf26, vf26, vf1x\n"
                             "vmuly.xyzw vf27, vf27, vf1y\n"
                             "vmulz.xyzw vf28, vf28, vf1z\n"
                             "vmulw.xyzw vf29, vf29, vf1w\n"
                             :
                             :);
    } else {
        if (svRecordMatrix != 0) {
            __asm__ __volatile__("lqc2 vf26, 0(%0)\n"
                                 "lqc2 vf27, 16(%0)\n"
                                 "lqc2 vf28, 32(%0)\n"
                                 "lqc2 vf29, 48(%0)\n"
                                 :
                                 : "r"(svRecordMatrix));
            vu0TransformPoint(point);
        } else {
            __asm__ __volatile__("lqc2 vf26, 0(%0)\n"
                                 "lqc2 vf27, 16(%0)\n"
                                 "lqc2 vf28, 32(%0)\n"
                                 "lqc2 vf29, 0(%1)\n"
                                 :
                                 : "r"(vu0IdentityMatrix), "r"(point));
        }
        vu0CurMatrixMul(drawState->matrix);
    }
    if (D_00551544 != 0) {
        __asm__ __volatile__("mfc1 $8, %0\n"
                             "qmtc2 $8, vf1\n"
                             "vmulx.xyzw vf26, vf26, vf1x\n"
                             "vmulx.xyzw vf27, vf27, vf1x\n"
                             "vmulx.xyzw vf28, vf28, vf1x\n"
                             :
                             : "f"(gSdvCameraPartTransform.scale)
                             : "$8", "memory");
    }
    if (svFogValue != 0) {
        __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(drawCall->quad));
        __asm__ __volatile__("vmulx.xyzw vf26, vf26, vf1x\n"
                             "vmuly.xyzw vf27, vf27, vf1y\n"
                             "vmulz.xyzw vf28, vf28, vf1z\n"
                             "vmulw.xyzw vf29, vf29, vf1w\n"
                             :
                             :);
        if (svFogValue >= 0) {
            vu0TransformPoint(drawStateExtra);
        }
    } else {
        vu0TransformPoint(drawStateExtra);
        __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(drawCall->quad));
        __asm__ __volatile__("vmulx.xyzw vf26, vf26, vf1x\n"
                             "vmuly.xyzw vf27, vf27, vf1y\n"
                             "vmulz.xyzw vf28, vf28, vf1z\n"
                             "vmulw.xyzw vf29, vf29, vf1w\n"
                             :
                             :);
    }
    if (svBattleLinkedMatrix != 0) {
        vu0CurMatrixMulRight((Vector4 *) svBattleLinkedMatrix);
    }
    __asm__ __volatile__("sqc2 vf29, 0(%0)" : : "r"(&drawRecord[0].position) : "memory");
    zRotation = drawState->angleZ + drawState->angleZOffset;
    statePacket = svDrawStatePacket;
    __asm__ __volatile__("lqc2 vf26, 0(%0)\n"
                         "lqc2 vf27, 16(%0)\n"
                         "lqc2 vf28, 32(%0)\n"
                         "lqc2 vf29, 48(%0)\n"
                         :
                         : "r"(statePacket->baseMatrix));
    if (zRotation != 0.0f) {
        vu0RotateCurrentZ(zRotation);
    }
    __asm__ __volatile__("sqc2 vf26, 0(%0)\n"
                         "sqc2 vf27, 16(%0)\n"
                         "sqc2 vf28, 32(%0)\n"
                         "sqc2 vf29, 48(%0)\n"
                         :
                         : "r"(drawRecord[0].matrix)
                         : "memory");
    svDrawList.end_index += 28;
}

/* Appends one part packet to svDrawList from an SvDrawState: the transform rows (scaled by svEffectScaleVector when scaleRows is set), the position, the extents and the source. */
void svDrawListAppendPart(SvDrawState *drawState, const Vector4 *vertex, SvDrawCall *drawCall, int scaleRows) {
    SvPartPacket *packet;

    packet = (SvPartPacket *) (svDrawList.entries + svDrawList.end_index);
    packet->gif.words[0] = 0xD02A400000008001ULL;
    packet->gif.words[1] = 0x000525252521EEEEULL;
    packet->tex.words[0] = drawState->uv->tex0;
    packet->tex.words[1] = GS_TEX0_1;
    packet->test.words[0] = svZTestWord;
    packet->test.words[1] = GS_TEST_1;
    packet->alpha.words[0] = svAlphaTestTable[drawState->tableIndex];
    packet->alpha.words[1] = GS_ALPHA_1;
    svScaleVertex(packet->position, drawState->vertex);
    QuadwordCopyT0(&packet->vertex, vertex);
    QuadwordCopyT0(&packet->extentA, &drawState->primaryPrim);
    packet->extentA.highDoubleword = 0;
    QuadwordCopyT0(&packet->extentB, &drawState->extraState);
    QuadwordCopyT0(&packet->source, drawState->uv);
    packet->source.words[1] = 0x3F80000000000000ULL;
    if ((drawState->uv->flags & 0x10) != 0) {
        __asm__ __volatile__("vmaxw.xyzw vf1, vf0, vf0w\n\tsqc2 vf1, 0(%0)" : : "r"(&packet->ref) : "memory");
    } else {
        svCopyQuadword(&packet->ref, drawCall->quad);
    }
    __asm__ __volatile__("lqc2 vf26, 0(%0)\n\t"
                         "lqc2 vf27, 16(%0)\n\t"
                         "lqc2 vf28, 32(%0)\n\t"
                         "lqc2 vf29, 48(%0)"
                         :
                         : "r"(drawState->matrix)
                         : "memory");
    if (scaleRows != 0) {
        __asm__ __volatile__("lqc2 vf1, 0(%0)\n\t"
                             "vmulx.xyzw vf26, vf26, vf1x\n\t"
                             "vmuly.xyzw vf27, vf27, vf1y\n\t"
                             "vmulz.xyzw vf28, vf28, vf1z\n\t"
                             "vmulw.xyzw vf29, vf29, vf1w"
                             :
                             : "r"(&svEffectScaleVector)
                             : "memory");
    }
    __asm__ __volatile__("sqc2 vf26, 0(%0)\n\t"
                         "sqc2 vf27, 16(%0)\n\t"
                         "sqc2 vf28, 32(%0)\n\t"
                         "sqc2 vf29, 48(%0)"
                         :
                         : "r"(packet->rows)
                         : "memory");
    if (svBattleLinkedMatrix != 0) {
        MMathApplyMatrix(&packet->vertex, svBattleLinkedMatrix, vertex);
    }
    svDrawList.end_index += 14;
}

/* Appends a pair of kind A draw records (28 quadwords) like svDrawListAppendRecordAPair, placing the point directly: the point goes to the first record position, and the matrix is the state's own, scaled per row when scaleRows is set. */
void svDrawListAppendStatePair(SvDrawState *drawState, Vector4 *point, SvDrawCall *drawCall, int scaleRows) {
    Vector4 *pointDst;
    Vector4 *extraStateSrc;
    Vector4 *extraStateDst;
    DoublewordVector4 *coordsDst;
    SvDrawRecordA *pairRecords;

    pairRecords = (SvDrawRecordA *) (svDrawList.entries + svDrawList.end_index);
    pairRecords[0].gif.words[0] = 0xD02A400000008001ULL;
    pairRecords[0].gif.words[1] = 0x000525252521EEEEULL;
    pairRecords[0].texture.words[0] = drawState->texture->tex0;
    pairRecords[1].texture.words[0] = drawState->uv->tex0;
    pairRecords[1].texture.words[1] = GS_TEX0_1;
    pairRecords[1].test.words[1] = GS_TEST_1;
    pairRecords[0].texture.words[1] = GS_TEX0_1;
    pairRecords[0].test.words[0] = SV_TEST_FIXED_DATA;
    pairRecords[1].test.words[0] = svZTestWord;
    pairRecords[0].test.words[1] = GS_TEST_1;
    pairRecords[0].alpha.words[0] = SV_ALPHA_FIXED_DATA;
    pairRecords[1].alpha.words[0] = svPairAlphaTable[drawState->tableIndex];
    pairRecords[1].alpha.words[1] = GS_ALPHA_1;
    pairRecords[0].alpha.words[1] = GS_ALPHA_1;
    svScaleVertex(pairRecords[0].vertex.words, drawState->vertex);
    pairRecords[1].vertex.quad = pairRecords[0].vertex.quad;
    pointDst = &pairRecords[0].position;
    __asm__ __volatile__("lq $8, 0(%3)\nsq $8, 0(%2)\n"
                         : "=m"(*pointDst), "+m"(*point)
                         : "r"(pointDst), "r"(point)
                         : "$8", "memory");
    pairRecords[0].prim.data = drawState->secondaryPrim;
    pairRecords[0].prim.address = 0;
    pairRecords[1].prim.data = drawState->primaryPrim;
    pairRecords[1].prim.address = 0;
    extraStateDst = &pairRecords[0].extraState;
    extraStateSrc = &drawState->extraState;
    __asm__ __volatile__("lq $8, 0(%3)\nsq $8, 0(%2)\n"
                         : "+m"(*extraStateDst), "+m"(*extraStateSrc)
                         : "r"(extraStateDst), "r"(extraStateSrc)
                         : "$8", "memory");
    coordsDst = &pairRecords[0].coords;
    __asm__ __volatile__("lq $8, 0(%3)\nsq $8, 0(%2)\n"
                         : "=m"(*coordsDst), "+m"(*drawState->uv)
                         : "r"(coordsDst), "r"(drawState->uv)
                         : "$8", "memory");
    pairRecords[0].coords.words[1] = 0x3F80000000000000ULL;
    QuadwordCopyT0(&pairRecords[0].quad, drawCall->quad);
    __asm__ __volatile__("lqc2 vf26, 0(%0)\n"
                         "lqc2 vf27, 16(%0)\n"
                         "lqc2 vf28, 32(%0)\n"
                         "lqc2 vf29, 48(%0)\n"
                         :
                         : "r"(drawState->matrix));
    if (scaleRows != 0) {
        __asm__ __volatile__("lqc2 vf1, 0(%0)\n"
                             "vmulx.xyzw vf26, vf26, vf1x\n"
                             "vmuly.xyzw vf27, vf27, vf1y\n"
                             "vmulz.xyzw vf28, vf28, vf1z\n"
                             "vmulw.xyzw vf29, vf29, vf1w\n"
                             :
                             : "r"(&svEffectScaleVector));
    }
    if (svBattleLinkedMatrix != 0) {
        vu0CurMatrixMulRight(svBattleLinkedMatrix);
    }
    __asm__ __volatile__("sqc2 vf26, 0(%0)\n"
                         "sqc2 vf27, 16(%0)\n"
                         "sqc2 vf28, 32(%0)\n"
                         "sqc2 vf29, 48(%0)\n"
                         :
                         : "r"(pairRecords[0].matrix)
                         : "memory");
    svDrawList.end_index += 28;
}

int sefInitPartTransform(SefPartTransform *partTransform, int partKindId, int scaleKey100, int scaleKey200) {
    float partScale = 1.0f;
    const Matrix4 *unitMatrix;

    if ((partKindId & 0x100) != 0 || partKindId == 59 || partKindId == 62) {
        partScale = sefGetEntryScale(scaleKey100);
    } else if ((partKindId & 0x200) != 0 || partKindId == 61 || partKindId == 63 || partKindId == 64 || partKindId == 65) {
        partScale = sefGetEntryScale(scaleKey200);
    }
    partTransform->scaled = 0;
    if (partScale != 1.0f) {
        partTransform->scale = partScale;
        partTransform->scaled = 1;
        partTransform->kind = partKindId;
        sefGetPosition(&partTransform->position, 0, partKindId);
        unitMatrix = &MMathUnitMatrix;
        __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(&partTransform->position) : "memory");
        __asm__ __volatile__("vmove.w vf1, vf0" : : : "memory");
        __asm__ __volatile__("lqc2 vf2, 32(%0)" : : "r"(unitMatrix) : "memory");
        __asm__ __volatile__("lqc2 vf3, 16(%0)" : : "r"(unitMatrix) : "memory");
        __asm__ __volatile__("lqc2 vf4, 0(%0)" : : "r"(unitMatrix) : "memory");
        __asm__ __volatile__("sqc2 vf1, 48(%0)\n\tsqc2 vf2, 32(%0)\n\tsqc2 vf3, 16(%0)\n\tsqc2 vf4, 0(%0)" : : "r"(partTransform) : "memory");
        partTransform->matrix[0][0] = partScale;
        partTransform->matrix[1][1] = partScale;
        partTransform->matrix[2][2] = partScale;
    }
    return partKindId;
}

int svInitItemDrawKind(SefPartTransform *partTransform, SvDrawItem *drawItem, int scaleKey100, int scaleKey200) {
    int partKindId = drawItem->entries[-2];

    if ((unsigned int)(partKindId - 16) < 2) {
        partKindId = drawItem->kindOverride;
    }
    return sefInitPartTransform(partTransform, partKindId, scaleKey100, scaleKey200);
}

void *func_00206A50(short *entries);
void func_00206AA8(Matrix4 *out, Vector4 *basePosition, Vector4 *extraPosition, int kind, void *source);
void func_00206C10(Matrix4 *out, Matrix4 *in, Vector4 *basePosition, Vector4 *extraPosition);
void func_002046F8(Vector4 *out, int mode, int kind);
void sefGetSourceMatrix(Matrix4 *out, int key);

void func_0020A320(Vector4 *point, SvItemTransform *transform, int useFixedMatrix, int useItemRows, int ignoreRecordTranslation);
void svDrawListAppendStatePair(SvDrawState *state, Vector4 *point, SvDrawCall *quadRef, int scaleRows);
void svDrawListAppendRecordAPair(SvDrawState *state, Vector4 *point, SvDrawCall *quadRef, int useFixedMatrix);
void svDrawListAppendRecordBPair(SvDrawState *state, Vector4 *point, SvDrawCall *quadRef, int useScale);
void svDrawListAppendRecordA(SvDrawState *state, Vector4 *point, SvDrawCall *quadRef, int useFixedMatrix);
struct SvPolygonRecord; struct SvDrawCall;
void svDrawPolygonItem(struct SvPolygonRecord *item, Vector4 *position, struct SvDrawCall *call, int scaled);
static Matrix4 *svViewMatrix;
static Vector4 *svRecordScale;
static struct SefRecordOwner *svRecordFlagOwner;
static int D_00551530;
extern const DrawStateWords D_0053F2E0;
extern const DrawStateWords D_0053F320;
extern const DrawStateWords D_0053F360;
extern const DrawStateWords D_0053F3A0;
extern const Vector4 D_0053F3E0;
extern const u64 D_0053F3F0[2];
extern Vector4 D_004755E0[2];
extern Matrix4 D_00770ED0[2];
extern Matrix4 D_00770F10;
extern Vector4 D_0076D7E0[24];
/* Stays asm-labelled: D_00551544 is also stored as an int; a single declaration changes the code of one of the two. */
extern Vector4 *svPivot[1] __asm__("D_00551544");

static inline void svSubVectorXYZ(Vector4 *destination, Vector4 *minuend, Vector4 *subtrahend) {
    __asm__ __volatile__("lqc2 vf1, 0(%4)\nlqc2 vf2, 0(%5)\nvsub.xyz vf1, vf1, vf2\nsqc2 vf1, 0(%3)\n"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z)
                         : "r"(destination), "r"(minuend), "r"(subtrahend));
}

static inline void svAddVectorXYZ(Vector4 *destination, Vector4 *left, Vector4 *right) {
    __asm__ __volatile__("lqc2 vf1, 0(%4)\nlqc2 vf2, 0(%5)\nvadd.xyz vf1, vf1, vf2\nsqc2 vf1, 0(%3)\n"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z)
                         : "r"(destination), "r"(left), "r"(right));
}

static inline void svCopyVector(Vector4 *destination, Vector4 *source) {
    __asm__ __volatile__("lq $8, 0(%5)\nsq $8, 0(%4)\n"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z),
                           "=m"(destination->w)
                         : "r"(destination), "r"(source)
                         : "$8");
}

/* Draws one image item: sets the draw mode and state words from its flags, builds its matrices, then appends the record of each live point handle for the current draw mode and flushes the list. */
void svDrawImageItem(SvDrawItem *drawItem, int renderState) {
    AlignedVector4 matrixRows[4];
    Matrix4 scratchMatrix;
    Matrix4 sourceMatrix;
    AlignedVector4 scaledPosition;
    SvDrawCall drawCall;
    AlignedVector4 basePosition;
    AlignedVector4 extraPosition;
    AlignedVector4 localPosition;
    AlignedVector4 workPosition;
    int i;
    int s19Bit3Flag;
    int hasPartScale;
    int wideModeMatch;
    int sourceFlag;
    int usesItemMatrix;
    int clipFlag;
    int s19Mask8;
    int reflectFlag;
    SefDepthSlot *depthSlots;
    Matrix4 *itemMatrix;
    int s19;
    SefDepthSlot *slot;
    SefPartTransform *cameraPart;
    const DrawStateWords *stateTable;
    Vector4 *slotPosition;
    short pointHandleSlot;
    int drawMode;
    int hasRecordMatrix;
    int isKind8;
    int drawState;
    int flagsMode;
    int showFlagged;
    int modeFlags;
    int imageFlags;
    int remainingRows;
    int battleBits;
    SefLinkedContext *battleData;

    drawMode = _draw3D;
    isKind8 = drawItem->entries[-2] == 8;
    modeFlags = drawItem->modeWords.modeFlags;
    wideModeMatch = (drawItem->modeWide & 0x800100000000ULL) == 0x800100000000ULL;
    sourceFlag = modeFlags & 0x20000;
    s19Bit3Flag = 0;
    hasPartScale = 0;
    flagsMode = modeFlags & 0x6;
    hasRecordMatrix = 0;
    if ((drawItem->entries[-2] & 0x100) != 0) {
        hasRecordMatrix = svRecordMatrix != 0;
    }
    usesItemMatrix = drawItem->entries[-2] == 3 || drawItem->entries[-2] == 7;
    s19 = 0;
    if (drawItem->s19Keys != 0) {
        s19 = drawItem->s19Keys[-1];
    }
    clipFlag = drawItem->imageFlags & 0x4000;
    reflectFlag = modeFlags & 0x1;
    showFlagged = svSceneFlags & 0x2;
    s19Mask8 = 0;
    depthSlots = (SefDepthSlot *) sevGetPtAllocator2(0);
    stateTable = &D_0053F2E0;
    if (svRecordFlagOwner == 0) {
        flagsMode = 0;
    }
    svFogValue = -1;
    D_00551530 = flagsMode;
    itemMatrix = 0;
    imageFlags = drawItem->imageFlags;
    svModelDrawFlags = imageFlags & 0xCB;
    battleBits = (gCameraSelectFlags >> 2) & 3;
    if (battleBits == 0 && drawItem->keyHeaders != 0 && drawItem->keyHeaders[-1].kind != 0) {
        svModelDrawFlags |= 0x40;
    }
    if ((imageFlags & 0x10) != 0 && showFlagged == 0) {
        return;
    }
    if (drawMode == 1 && !isKind8) {
        return;
    }
    if (drawMode != 1 && isKind8) {
        return;
    }

    drawCall.texture = svUnitQuadCornerMatrix;
    drawCall.uv = D_0053F3F0;
    drawCall.mode = s19;
    svPivot[0] = 0;
    cameraPart = &gSdvCameraPartTransform;
    cameraPart->scaled = 0;
    if (hasRecordMatrix != 0) {
        hasRecordMatrix = MMathIsVectorEqual(&svEffectScaleVector, D_004755E0) == 0 ? hasRecordMatrix : 0;
    }
    if (drawMode != 1) {
        if ((drawItem->modeWords.modeFlags & 0x2000) != 0) {
            svInitItemDrawKind(cameraPart, drawItem, sefCurrentEffectRecord->scaleKey80,
                               sefCurrentEffectRecord->scaleKey200);
            hasPartScale = cameraPart->scaled;
        }
        if (wideModeMatch != 0) {
            void *source = func_00206A50(drawItem->entries);
            func_00206AA8(&scratchMatrix, &basePosition, &extraPosition, drawItem->entries[-2], source);
            func_00206C10(&scratchMatrix, &scratchMatrix, &basePosition, &extraPosition);
        }
    }

    if (drawMode == 1) {
        svDrawMode = clipFlag != 0 ? 4 : 3;
        stateTable = &D_0053F3A0;
    } else if (drawMode == 0) {
        if ((drawItem->modeWords.modeFlags & 0x80) != 0 || svZTestEnable == 0) {
            svZTestWord = SV_ZTEST_OFF;
        } else {
            svZTestWord = SV_ZTEST_ON;
        }
        if (drawCall.mode == 0) {
            drawState = clipFlag != 0 ? 2 : 1;
            svModelDrawFlags &= ~2;
            svDrawMode = drawState;
            svPolygonAxesMode = drawState;
        } else {
            if ((s19 & 1) != 0) {
                svDrawMode = 5;
            } else if ((s19 & 2) != 0) {
                svDrawMode = 7;
                stateTable = &D_0053F320;
            } else if ((s19 & 8) != 0) {
                svModelDrawFlags &= ~2;
                s19Bit3Flag = 1;
                svDrawMode = 11;
            } else {
                svDrawMode = 9;
                stateTable = &D_0053F360;
            }
            svPolygonAxesMode = svDrawMode;
            if ((svModelDrawFlags & 2) != 0) {
                svDrawMode = 13;
            }
            if (clipFlag != 0) {
                D_005510D4 = 1;
                svDrawMode = svDrawMode + 1;
            } else {
                D_005510D4 = 0;
            }
        }
    } else {
        s19Mask8 = s19 & 0x8;
        svDrawMode = 0;
        svPolygonAxesMode = 0;
        svCurrentVertexViewBlock =
            ((drawItem->imageFlags & 0x4) != 0) ? (void *) &D_0076D4E0 : (void *) D_0076D7E0;
    }
    svDrawStatePacket->stateWords = *stateTable;

    if (usesItemMatrix != 0) {
        itemMatrix = drawItem->entries[-2] == 3 ? &D_00770ED0[0] : &D_00770F10;
        __asm__ __volatile__("lqc2 vf1, 32(%0)\n\t"
                             "lqc2 vf2, 16(%0)\n\t"
                             "lqc2 vf3, 0(%0)\n\t"
                             "sqc2 vf0, 48(%1)\n\t"
                             "sqc2 vf1, 32(%1)\n\t"
                             "sqc2 vf2, 16(%1)\n\t"
                             "sqc2 vf3, 0(%1)"
                             :
                             : "r"(&MMathUnitMatrix), "r"(&svDrawStatePacket->baseMatrix)
                             : "memory");
        MMathMulMatrix(svDrawStatePacket, svViewMatrix, itemMatrix);
    } else {
        MMathMulMatrix(&svDrawStatePacket->baseMatrix, svViewMatrix, sefCameraMatrix);
        if (hasRecordMatrix == 0) {
            MMathScaleMatrix(&svDrawStatePacket->baseMatrix, &svDrawStatePacket->baseMatrix,
                             (Vector4 *) ((u8 *) sefCurrentEffectRecord + 48));
        }
        QuadwordCopyMatrixT0(svDrawStatePacket, svViewMatrix);
    }
    if (s19Bit3Flag != 0) {
        QuadwordCopyMatrixT0(svDrawStatePacket, svViewMatrix);
    } else if (svRecordMatrix != 0 && (svModelDrawFlags & 0x2) == 0) {
        if (hasRecordMatrix != 0) {
            MMathMulMatrix(svDrawStatePacket, svDrawStatePacket, &svEffectMatrix);
        } else {
            MMathMulMatrix(svDrawStatePacket, svDrawStatePacket, svRecordMatrix);
        }
    }
    if (hasPartScale != 0 && (svModelDrawFlags & 0x2) == 0) {
        MMathScaleMatrixRotS(&svDrawStatePacket->baseMatrix, &svDrawStatePacket->baseMatrix,
                             gSdvCameraPartTransform.scale);
        svDrawStatePacket->baseMatrix[3][3] = 1.0f;
    }
    if (drawItem->fogTable != 0) {
        svFogValue = drawItem->fogTable[-1];
    }
    if (sourceFlag != 0) {
        short sourceKey = drawItem->s19Keys != 0 ? drawItem->s19Keys[-2] : 0;
        short entryKind = drawItem->entries != 0 ? drawItem->entries[-2] : 0;
        sefGetSourceMatrix(&sourceMatrix, sourceKey);
        func_002046F8((Vector4 *) sourceMatrix[3], 0, entryKind);
    }
    svBattleLinkedMatrix = 0;
    if ((flagsMode & 0x2) != 0) {
        battleData = sefGetBattleData();
        svBattleLinkedMatrix = (Matrix4 *) battleData->kind256Matrix;
        if ((unsigned int) (svDrawMode - 5) < 6U) {
            MMathMulMatrix(svDrawStatePacket, svDrawStatePacket, svBattleLinkedMatrix);
        }
    }
    if (svDrawMode != 0) {
        svDrawSetState((drawItem->modeWords.modeFlags & 0x200),
                       drawItem->keyHeaders != 0 ? drawItem->keyHeaders[-1].kind : 0, drawItem->clampRepeat);
        svDrawSubmitMode();
        svDrawListAppend(SV_DRAW_LIST_FLUSH_CODE, 0, svFogValue);
    }
    remainingRows = drawItem->rowCount;

    for (i = 0; i < SV_ENTRY_COUNT && remainingRows > 0; i++) {
        pointHandleSlot = drawItem->pointHandles[i];
        if (pointHandleSlot < 0) {
            continue;
        }
        slot = depthSlots + pointHandleSlot;
        remainingRows--;
        if (slot->drawData == 0) {
            continue;
        }
        svVertexScale = svRecordScale;
        if (drawMode != 2 && sefEffectFlagTest(2) != 0 && slot->drawIndex == 0) {
            svVertexScale = &D_0053F3E0;
        }
        switch (drawMode) {
        case 2:
            if (slot->part->kind != 6) {
                continue;
            }
            if (hasPartScale != 0) {
                Vector4 *position = &slot->position;
                if (reflectFlag != 0) {
                    svSubVectorXYZ(&localPosition, position, &gSdvCameraPartTransform.position);
                    func_0020A320(&localPosition, slot, hasRecordMatrix, s19Mask8, 1);
                    svCopyVector((Vector4 *) &gSdvCameraPartTransform.matrix[3],
                                 &gSdvCameraPartTransform.position);
                    if (svRecordMatrix != 0) {
                        svAddVectorXYZ((Vector4 *) &gSdvCameraPartTransform.matrix[3],
                                       (Vector4 *) &(*svRecordMatrix)[3], &gSdvCameraPartTransform.position);
                    } else {
                        svCopyVector((Vector4 *) &gSdvCameraPartTransform.matrix[3],
                                     &gSdvCameraPartTransform.position);
                    }
                } else {
                    svSubVectorXYZ(&localPosition, position, &slot->pivot);
                    func_0020A320(&localPosition, slot, hasRecordMatrix, s19Mask8, 1);
                    if (svRecordMatrix != 0) {
                        svAddVectorXYZ((Vector4 *) &gSdvCameraPartTransform.matrix[3],
                                       (Vector4 *) &(*svRecordMatrix)[3], &slot->pivot);
                    } else {
                        svCopyVector((Vector4 *) &gSdvCameraPartTransform.matrix[3], &slot->pivot);
                    }
                }
                vu0CurMatrixMulRight(&gSdvCameraPartTransform);
            } else if (sourceFlag != 0) {
                MMathApplyMatrix(&workPosition, &sourceMatrix, &slot->position);
                func_0020A320(&workPosition, slot, hasRecordMatrix, s19Mask8, 0);
            } else {
                func_0020A320(&slot->position, slot, hasRecordMatrix, s19Mask8, 0);
            }
            if (svBattleLinkedMatrix != 0) {
                vu0CurMatrixMulRight(svBattleLinkedMatrix);
            }
            if (usesItemMatrix != 0) {
                vu0CurMatrixMulRight(itemMatrix);
            }
            __asm__ __volatile__("sqc2 vf26, 0(%0)\n\t"
                                 "sqc2 vf27, 16(%0)\n\t"
                                 "sqc2 vf28, 32(%0)\n\t"
                                 "sqc2 vf29, 48(%0)"
                                 :
                                 : "r"(matrixRows)
                                 : "memory");
            svSetModelRenderState((SvRenderItem *) slot, (Matrix4 *) matrixRows, renderState);
            break;
        case 1:
            if (slot->part->kind != 4) {
                continue;
            }
            if (clipFlag != 0) {
                svDrawListAppendSpriteRecordPair((SvDrawState *) slot, svPairAlphaTable[slot->drawIndex]);
            } else {
                svDrawListAppendSpriteRecord((SvDrawState *) slot, svAlphaTestTable[slot->drawIndex]);
            }
            svDrawListTryFlush();
            break;
        case 0:
            if (slot->part->kind != 4) {
                continue;
            }
            drawCall.quad = &slot->scale;
            slotPosition = &slot->position;
            if (hasPartScale != 0) {
                __asm__ __volatile__("lqc2 vf20, 0(%0)" : : "r"(slotPosition));
                if (reflectFlag != 0) {
                    svPivot[0] = &gSdvCameraPartTransform.position;
                    __asm__ __volatile__("lqc2 vf1, 0(%0)\n\t"
                                         "vsub.xyz vf20, vf20, vf1"
                                         :
                                         : "r"(&gSdvCameraPartTransform.position));
                    __asm__ __volatile__("mfc1 $8, %0\n\t"
                                         "qmtc2.ni $8, vf1\n\t"
                                         "vmulx.xyz vf20, vf20, vf1x"
                                         :
                                         : "f"(gSdvCameraPartTransform.scale)
                                         : "$8");
                    __asm__ __volatile__("lqc2 vf1, 0(%0)\n\t"
                                         "vadd.xyz vf20, vf20, vf1"
                                         :
                                         : "r"(&gSdvCameraPartTransform.position));
                } else {
                    svPivot[0] = &slot->pivot;
                    __asm__ __volatile__("lqc2 vf1, 0(%0)\n\t"
                                         "vsub.xyz vf20, vf20, vf1"
                                         :
                                         : "r"(&slot->pivot));
                    __asm__ __volatile__("mfc1 $8, %0\n\t"
                                         "qmtc2.ni $8, vf1\n\t"
                                         "vmulx.xyz vf20, vf20, vf1x"
                                         :
                                         : "f"(gSdvCameraPartTransform.scale)
                                         : "$8");
                    __asm__ __volatile__("lqc2 vf1, 0(%0)\n\t"
                                         "vadd.xyz vf20, vf20, vf1"
                                         :
                                         : "r"(&slot->pivot));
                }
                if (wideModeMatch != 0) {
                    __asm__ __volatile__("lqc2 vf26, 0(%0)\n\t"
                                         "lqc2 vf27, 16(%0)\n\t"
                                         "lqc2 vf28, 32(%0)\n\t"
                                         "lqc2 vf29, 48(%0)\n\t"
                                         "vmulax.xyzw ACC, vf26, vf20x\n\t"
                                         "vmadday.xyzw ACC, vf27, vf20y\n\t"
                                         "vmaddaz.xyzw ACC, vf28, vf20z\n\t"
                                         "vmaddw.xyzw vf20, vf29, vf0w"
                                         :
                                         : "r"(&scratchMatrix)
                                         : "memory");
                }
                __asm__ __volatile__("sqc2 vf20, 0(%4)"
                                     : "=m"(scaledPosition.x), "=m"(scaledPosition.y), "=m"(scaledPosition.z),
                                       "=m"(scaledPosition.w)
                                     : "r"(&scaledPosition));
                slotPosition = &scaledPosition;
            } else if (wideModeMatch != 0) {
                MMathApplyMatrix(&scaledPosition, &scratchMatrix, &slot->position);
                slotPosition = &scaledPosition;
            } else if (sourceFlag != 0) {
                MMathApplyMatrix(&scaledPosition, &sourceMatrix, &slot->position);
                slotPosition = &scaledPosition;
            }
            if (drawCall.mode == 0) {
                if (clipFlag != 0) {
                    svDrawListAppendStatePair(slot, slotPosition, &drawCall, hasRecordMatrix);
                } else {
                    svDrawListAppendPart(slot, slotPosition, &drawCall, hasRecordMatrix);
                }
            } else {
                if (clipFlag != 0) {
                    if (svDrawMode == 12) {
                        svDrawListAppendRecordAPair(slot, slotPosition, &drawCall, hasRecordMatrix);
                    } else if (svDrawMode == 14) {
                        svDrawListFlush(svDrawList.end_index - svDrawList.begin_index);
                        svDrawPolygonItem(slot, slotPosition, &drawCall, hasRecordMatrix);
                        svDrawListFlush(svDrawList.end_index - svDrawList.begin_index);
                    } else {
                        svDrawListAppendRecordBPair(slot, slotPosition, &drawCall, hasRecordMatrix);
                    }
                } else {
                    if (svDrawMode == 11) {
                        svDrawListAppendRecordA(slot, slotPosition, &drawCall, hasRecordMatrix);
                    } else if (svDrawMode == 13) {
                        svDrawListFlush(svDrawList.end_index - svDrawList.begin_index);
                        svDrawPolygonItem(slot, slotPosition, &drawCall, hasRecordMatrix);
                        svDrawListFlush(svDrawList.end_index - svDrawList.begin_index);
                    } else {
                        svDrawListAppendRecordB(slot, slotPosition, &drawCall, hasRecordMatrix);
                    }
                }
            }
            svDrawListTryFlush();
            break;
        }
    }
    svDrawListFlush(svDrawList.end_index - svDrawList.begin_index);
    {
        XglPacket *packet = xglPacketGetCurrent();
        sceVif1PkCnt(packet, 0);
        sceVif1PkAddCode(packet, 0x11000000);
    }
}

typedef struct SefSortRecord {
    unsigned char unmodeled_00[0x20];
    Vector4 scale;
    Vector4 scale2;
} SefSortRecord;

/* Draws the lines of the depth buckets, from bucket 39 down to 0, that the scene flag selects: sets each line's draw mode and state table, moves the matrices for camera-facing or frame-moved lines, and sends every slot to the draw helper of its mode. */
void svDrawSortedLines(SefEffectRecord *effectRecord) {
    SvDrawCall drawRef;
    Matrix4 recordMatrix;
    Matrix4 frameMatrix;
    Vector4 framePointA;
    Vector4 framePointB;
    Vector4 movedPosition;
    int bucketIndex;
    int lastLineIndex;
    unsigned char *slotBase;
    int pendingCount;
    unsigned short additiveBit;
    int partScaled;
    int useRawViewMatrix;
    int usesFrameMatrix;
    SvDrawItem *lineData;
    SortItem *sortItem;
    SvDrawState *slotState;
    Vector4 *position;
    Matrix4 *faceMatrix;
    const DrawStateWords *stateTable;
    const DrawStateWords *lastStateTable;
    short lineMode;
    short itemIndex;
    int facesCamera;
    int drawMode;
    int modeBit80;

    lastStateTable = 0;
    lastLineIndex = -1;
    slotBase = sevGetPtAllocator2(0);
    drawRef.uv = D_0053F3F0;
    drawRef.texture = svUnitQuadCornerMatrix;
    svDrawMode = 5;
    drawRef.mode = -1;
    D_00551544 = 0;
    ((DoublewordVector4 *) &svDrawStatePacket->clamp)->words[1] = 8;
    ((DoublewordVector4 *) &svDrawStatePacket->clamp)->words[0] = 5;
    pendingCount = 0;
    additiveBit = 0;
    partScaled = 0;
    stateTable = &D_0053F2E0;
    MMathMulMatrix(&recordMatrix, svViewMatrix, sefCameraMatrix);
    usesFrameMatrix = 0;
    for (bucketIndex = 39; bucketIndex >= 0; bucketIndex--) {
        sortItem = (SortItem *) &svDepthBuckets[bucketIndex];
        while (sortItem->next >= 0) {
            itemIndex = sortItem->next;
            sortItem = (SortItem *) &svDepthNodes[itemIndex];
            lineData = sefGetLineAdr(sortItem->flags);
            if (lineData == 0) {
                continue;
            }
            if (svEffectSceneFlag != 0 && (lineData->modeWords.modeFlags & 0x4000) == 0) {
                continue;
            }
            if (svEffectSceneFlag == 0 && (lineData->modeWords.modeFlags & 0x4000) != 0) {
                continue;
            }
            if (sortItem->flags != lastLineIndex) {
                lastLineIndex = sortItem->flags;
                facesCamera = 0;
                if (lineData->entries[-2] == 3 || lineData->entries[-2] == 7) {
                    facesCamera = 1;
                }
                additiveBit = lineData->imageFlags & 0x4000;
                lineMode = 0;
                if (lineData->s19Keys != 0) {
                    lineMode = lineData->s19Keys[-1];
                }
                drawRef.mode = lineMode;
                gSdvCameraPartTransform.scaled = 0;
                partScaled = 0;
                useRawViewMatrix = 0;
                D_00551544 = 0;
                usesFrameMatrix = (lineData->modeWide & 0x800100000000ULL) == 0x800100000000ULL;
                if (usesFrameMatrix) {
                    func_00206AA8(&frameMatrix, &framePointA, &framePointB, lineData->entries[-2],
                                  func_00206A50(lineData->entries));
                    func_00206C10(&frameMatrix, &frameMatrix, &framePointA, &framePointB);
                }
                modeBit80 = lineData->modeWords.modeFlags & 0x80;
                svModelDrawFlags = lineData->imageFlags & 2;
                if (modeBit80 != 0 || svZTestEnable == 0) {
                    svZTestWord = 0x3000D;
                } else {
                    svZTestWord = 0x5000D;
                }
                if (pendingCount > 0) {
                    pendingCount = 0;
                    svDrawListFlush(svDrawList.end_index - svDrawList.begin_index);
                }
                if (lineData->fogTable != 0) {
                    svFogValue = lineData->fogTable[-1];
                } else {
                    svFogValue = -1;
                }
                if (lineMode == 0) {
                    drawMode = additiveBit ? 2 : 1;
                    svDrawMode = drawMode;
                    svPolygonAxesMode = drawMode;
                    stateTable = &D_0053F2E0;
                    svModelDrawFlags &= ~2;
                } else {
                    if (lineMode & 1) {
                        stateTable = &D_0053F2E0;
                        svDrawMode = 5;
                    } else if (lineMode & 2) {
                        svDrawMode = 7;
                        stateTable = &D_0053F320;
                    } else if (lineMode & 8) {
                        svDrawMode = 11;
                        stateTable = &D_0053F2E0;
                        useRawViewMatrix = 1;
                    } else {
                        svDrawMode = 9;
                        stateTable = &D_0053F360;
                    }
                    svPolygonAxesMode = svDrawMode;
                    if ((svModelDrawFlags & 2) != 0) {
                        svDrawMode = 13;
                    }
                    if (additiveBit) {
                        svDrawMode++;
                    }
                }
                if (lastStateTable != stateTable) {
                    lastStateTable = stateTable;
                    svDrawStatePacket->textureState[0].words[0] = lastStateTable->words[0];
                    svDrawStatePacket->textureState[0].words[1] = lastStateTable->words[1];
                    svDrawStatePacket->textureState[1].words[0] = lastStateTable->words[2];
                    svDrawStatePacket->textureState[1].words[1] = lastStateTable->words[3];
                    svDrawStatePacket->textureState[2].words[0] = lastStateTable->words[4];
                    svDrawStatePacket->textureState[2].words[1] = lastStateTable->words[5];
                    svDrawStatePacket->textureState[3].words[0] = lastStateTable->words[6];
                    svDrawStatePacket->textureState[3].words[1] = lastStateTable->words[7];
                }
                if ((lineData->modeWords.modeFlags & 0x2000) != 0) {
                    svInitItemDrawKind(&gSdvCameraPartTransform, lineData,
                                       sefCurrentEffectRecord->scaleKey80,
                                       sefCurrentEffectRecord->scaleKey200);
                    partScaled = gSdvCameraPartTransform.scaled;
                }
                if (facesCamera) {
                    if (lineData->entries[-2] == 3) {
                        faceMatrix = &D_00770ED0[0];
                    } else {
                        faceMatrix = &D_00770F10;
                    }
                    __asm__ __volatile__("lqc2 vf1, 32(%1)\n"
                                         "lqc2 vf2, 16(%1)\n"
                                         "lqc2 vf3, 0(%1)\n"
                                         "sqc2 vf0, 48(%0)\n"
                                         "sqc2 vf1, 32(%0)\n"
                                         "sqc2 vf2, 16(%0)\n"
                                         "sqc2 vf3, 0(%0)\n"
                                         :
                                         : "r"(&svDrawStatePacket->baseMatrix), "r"(&MMathUnitMatrix)
                                         : "memory");
                    MMathMulMatrix(&svDrawStatePacket->matrixA, svViewMatrix, faceMatrix);
                } else {
                    QuadwordCopyMatrixT0(&svDrawStatePacket->baseMatrix, &recordMatrix);
                    MMathScaleMatrix(&svDrawStatePacket->baseMatrix, &svDrawStatePacket->baseMatrix,
                                     &((SefSortRecord *) sefCurrentEffectRecord)->scale2);
                    QuadwordCopyMatrixT0(&svDrawStatePacket->matrixA, svViewMatrix);
                }
                if (useRawViewMatrix) {
                    QuadwordCopyMatrixT0(&svDrawStatePacket->matrixA, svViewMatrix);
                } else if (svRecordMatrix != 0 && (svModelDrawFlags & 2) == 0) {
                    MMathMulMatrix(&svDrawStatePacket->matrixA, &svDrawStatePacket->matrixA, svRecordMatrix);
                }
                if (partScaled && (svModelDrawFlags & 2) == 0) {
                    MMathScaleMatrixRotS(&svDrawStatePacket->baseMatrix, &svDrawStatePacket->baseMatrix,
                                         gSdvCameraPartTransform.scale);
                    svDrawStatePacket->baseMatrix[3][3] = 1.0f;
                }
                svDrawSetState(lineData->modeWords.modeFlags & 0x200,
                               lineData->keyHeaders != 0 ? lineData->keyHeaders[-1].kind : 0,
                               lineData->clampRepeat);
                svDrawSubmitMode();
                svDrawListAppend(0x28, 0, svFogValue);
            }
            slotState = (SvDrawState *) (slotBase + sortItem->id * 0x270);
            if (slotState->uv->kind != 4) {
                continue;
            }
            position = &slotState->pointA;
            if (partScaled) {
                __asm__ __volatile__("lqc2 vf20, 0(%0)" : : "r"(position));
                if ((lineData->modeWords.modeFlags & 1) != 0) {
                    D_00551544 = (int) &gSdvCameraPartTransform.position;
                    __asm__ __volatile__("lqc2 vf1, 0(%0)\n"
                                         "vsub.xyz vf20, vf20, vf1\n"
                                         :
                                         : "r"(&gSdvCameraPartTransform.position));
                    __asm__ __volatile__("mfc1 $8, %0\n"
                                         "qmtc2.ni $8, vf1\n"
                                         "vmulx.xyz vf20, vf20, vf1x\n"
                                         :
                                         : "f"(gSdvCameraPartTransform.scale)
                                         : "$8");
                    __asm__ __volatile__("lqc2 vf1, 0(%0)\n"
                                         "vadd.xyz vf20, vf20, vf1\n"
                                         :
                                         : "r"(&gSdvCameraPartTransform.position));
                } else {
                    D_00551544 = (int) &((char *) slotState)[0x120];
                    __asm__ __volatile__("lqc2 vf1, 0(%0)\n"
                                         "vsub.xyz vf20, vf20, vf1\n"
                                         :
                                         : "r"(&((char *) slotState)[0x120]));
                    __asm__ __volatile__("mfc1 $8, %0\n"
                                         "qmtc2.ni $8, vf1\n"
                                         "vmulx.xyz vf20, vf20, vf1x\n"
                                         :
                                         : "f"(gSdvCameraPartTransform.scale)
                                         : "$8");
                    __asm__ __volatile__("lqc2 vf1, 0(%0)\n"
                                         "vadd.xyz vf20, vf20, vf1\n"
                                         :
                                         : "r"(&((char *) slotState)[0x120]));
                }
                if (usesFrameMatrix) {
                    __asm__ __volatile__("lqc2 vf26, 0(%0)\n"
                                         "lqc2 vf27, 16(%0)\n"
                                         "lqc2 vf28, 32(%0)\n"
                                         "lqc2 vf29, 48(%0)\n"
                                         "vmulax.xyzw ACC, vf26, vf20x\n"
                                         "vmadday.xyzw ACC, vf27, vf20y\n"
                                         "vmaddaz.xyzw ACC, vf28, vf20z\n"
                                         "vmaddw.xyzw vf20, vf29, vf0w\n"
                                         :
                                         : "r"(&frameMatrix)
                                         : "memory");
                }
                __asm__ __volatile__("sqc2 vf20, 0(%4)"
                                     : "=m"(movedPosition.x), "=m"(movedPosition.y), "=m"(movedPosition.z),
                                       "=m"(movedPosition.w)
                                     : "r"(&movedPosition));
                position = &movedPosition;
            } else if (usesFrameMatrix) {
                MMathApplyMatrix(&movedPosition, &frameMatrix, position);
                position = &movedPosition;
            }
            drawRef.quad = &slotState->pointB;
            if (drawRef.mode == 0) {
                if (additiveBit) {
                    svDrawListAppendStatePair(slotState, position, &drawRef, 0);
                } else {
                    svDrawListAppendPart(slotState, position, &drawRef, 0);
                }
            } else if (additiveBit) {
                if (svDrawMode == 12) {
                    svDrawListAppendRecordAPair(slotState, position, &drawRef, 0);
                } else if (svDrawMode == 14) {
                    svDrawListFlush(svDrawList.end_index - svDrawList.begin_index);
                    svDrawPolygonItem(slotState, position, &drawRef, 0);
                    svDrawListFlush(svDrawList.end_index - svDrawList.begin_index);
                } else {
                    svDrawListAppendRecordBPair(slotState, position, &drawRef, 0);
                }
            } else if (svDrawMode == 11) {
                svDrawListAppendRecordA(slotState, position, &drawRef, 0);
            } else if (svDrawMode == 13) {
                svDrawListFlush(svDrawList.end_index - svDrawList.begin_index);
                svDrawPolygonItem(slotState, position, &drawRef, 0);
                svDrawListFlush(svDrawList.end_index - svDrawList.begin_index);
            } else {
                svDrawListAppendRecordB(slotState, position, &drawRef, 0);
            }
            pendingCount++;
            svDrawListTryFlush();
        }
    }
    svDrawListFlush(svDrawList.end_index - svDrawList.begin_index);
}

void svDepthListInsert(int id, int flags, float depth);

/* Adds each drawn slot of the pairs' lines to the depth sort, with the view-space z of its position under the current matrix as depth; returns the summed line count. */
int svSortLineSlotsByDepth(SefEffectRecord *effectRecord, SvLineSortKey *sortPairs, int sortPairCount, SvDrawContext *drawContext) {
    unsigned char *depthSlotArena;
    SvDrawItem *lineItem;
    SefDepthSlot *depthSlot;
    SefSlotPartDefinition *partDefinition;
    short *pointHandleCursor;
    int showFlagged;
    int lineTotal;
    int pointSlotIndex;
    int lineIndex;
    int pairsLeft;
    SvLineSortKey *sortPair;
    int slotsLeft;
    int wantedPartKind;
    Vector4 depthVector;

    depthSlotArena = sevGetPtAllocator2(0);
    lineTotal = 0;
    showFlagged = svSceneFlags & 2;
    svSortReset();
    __asm__ __volatile__(
        "lqc2 vf26, 0(%0)\n\t"
        "lqc2 vf27, 16(%0)\n\t"
        "lqc2 vf28, 32(%0)\n\t"
        "lqc2 vf29, 48(%0)"
        : : "r"(drawContext->matrix));
    if (sortPairCount > 0) {
        sortPair = sortPairs;
        for (pairsLeft = sortPairCount; pairsLeft != 0; pairsLeft--, sortPair++) {
            lineIndex = sortPair->lineIndex;
            lineItem = sefGetLineAdr(lineIndex);
            if (lineItem->count == 0 || lineItem->entries[-2] == 8) {
                continue;
            }
            lineTotal += lineItem->count;
            wantedPartKind = 4;
            pointHandleCursor = lineItem->pointHandles;
            for (slotsLeft = 255; slotsLeft >= 0; slotsLeft--, pointHandleCursor++) {
                pointSlotIndex = *pointHandleCursor;
                if (pointSlotIndex < 0) {
                    continue;
                }
                depthSlot = (SefDepthSlot *)(depthSlotArena + pointSlotIndex * sizeof(SefDepthSlot));
                partDefinition = depthSlot->part;
                if ((partDefinition->flags & 0x10) != 0 && showFlagged == 0) {
                    continue;
                }
                if (partDefinition->kind != wantedPartKind) {
                    continue;
                }
                if (depthSlot->drawData == 0) {
                    continue;
                }
                __asm__ __volatile__(
                    "lqc2 vf1, 0(%1)\n\t"
                    "vmulax.xyzw ACC, vf26, vf1x\n\t"
                    "vmadday.xyzw ACC, vf27, vf1y\n\t"
                    "vmaddaz.xyzw ACC, vf28, vf1z\n\t"
                    "vmaddw.xyzw vf20, vf29, vf1w\n\t"
                    "vaddz.xyzw vf1, vf0, vf20z\n\t"
                    "qmfc2 $8, vf1\n\t"
                    "mtc1 $8, %0\n\t"
                    "nop"
                    : "=f"(depthVector.x) : "r"(&depthSlot->position) : "$8");
                svDepthListInsert(pointSlotIndex, lineIndex, depthVector.x);
            }
        }
    }
    return lineTotal;
}

struct SvSchedulerEntryView;
/* Gathers the line index and sort key of every line of the active slots into out, sorts the pairs ascending by sort key and returns their count. */ int svCollectSortedLines(struct SvSchedulerEntryView *record, SvLineSortKey *out);

/* One scheduler entry of sefGetScheduler as svCollectSortedLines reads it: the sub-slot ids at +0xFC and the entry id at +0x13E. */
typedef struct SvSchedulerEntryView {
    unsigned char unmodeled_00[0xFC];
    short subSlotBusy[32]; /* +0xFC: slot ids of the sub-slots, negative when unused */
    unsigned char unmodeled_13C[0x13E - 0x13C];
    short entryId; /* +0x13E */
} SvSchedulerEntryView;

/* One effect slot of sefGetEffectSlot: 32 line indices and their sort keys, and the flags at +0x64. */
typedef struct SvEffectSlotView {
    short lineIndex[32];     /* +0x00 */
    signed char sortKey[32]; /* +0x40 */
    unsigned char unmodeled_60[0x64 - 0x60];
    unsigned int flags; /* +0x64 */
} SvEffectSlotView;

int func_001F8750(int id, SvLineSortKey *requests, int count);
SvEffectSlotView *sefGetEffectSlot(s32 index);

/* Gathers the line index and sort key of every line of the active effect slots into out, sorts the pairs by ascending
 * sort key and returns their count. Like its X2 homolog, without the func_001F8750 step. */
int svCollectSortedLines(SvSchedulerEntryView *schedulerEntry, SvLineSortKey *sortedPairs) {
    SvEffectSlotView *effectSlot;
    SvDrawItem *lineItem;
    SvLineSortKey *pair;
    SvLineSortKey *later;
    SvLineSortKey swapPair;
    int showFlagged;
    int pairCount;
    int i;
    int j;

    showFlagged = svSceneFlags & 2;
    pairCount = 0;
    for (j = 0; j < 32; j++) {
        effectSlot = sefGetEffectSlot(schedulerEntry->subSlotBusy[j]);
        if (effectSlot == 0 || !(effectSlot->flags & 0x80000000)) {
            continue;
        }
        for (i = 0; i < 32; i++) {
            if (effectSlot->lineIndex[i] < 0) {
                continue;
            }
            lineItem = sefGetLineAdr(effectSlot->lineIndex[i]);
            if (lineItem->rowCount == 0) {
                continue;
            }
            if ((lineItem->imageFlags & 0x10) && showFlagged == 0) {
                continue;
            }
            pair = &sortedPairs[pairCount];
            pair->lineIndex = effectSlot->lineIndex[i];
            pair->sortKey = effectSlot->sortKey[i];
            pair->on = 0;
            pairCount++;
        }
    }
    pairCount = func_001F8750(schedulerEntry->entryId, sortedPairs, pairCount);
    for (i = 0; i < pairCount - 1; i++) {
        for (j = i + 1; j < pairCount; j++) {
            later = &sortedPairs[j];
            if (later->sortKey < sortedPairs[i].sortKey) {
                swapPair = sortedPairs[i];
                sortedPairs[i] = *later;
                *later = swapPair;
            }
        }
    }
    return pairCount;
}

void svLoadDrawViewMatrix(void) {
    XglCamera *camera;

    camera = xglStudioGetActiveCamera();
    QuadwordCopyMatrixT0(svDrawStatePacket, camera->volume[0].viewMatrix);
}

/* Builds the draw-state packet: a count, a 0x11000000 tag, two sized blocks (0x14 and 0x6C), then 0x10 bytes from svDrawStatePacket. */
void svBuildDrawStatePacket(void) {
    XglPacket *packet;

    packet = xglPacketGetCurrent();
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddCode(packet, 0x11000000);
    sceVif1PkOpenUpkCode(packet, 0x14, 0x6C, 4, 4);
    sceVif1PkAddUpkData128N(packet, svDrawStatePacket, 0x10);
    sceVif1PkCloseUpkCode(packet);
}

typedef struct SvEffectRecordView {
    int unmodeled_00;    /* +0x00 */
    int ownerGroupFlags; /* +0x04 */
    short entryIndex;    /* +0x08: compared with the index argument of svDrawEntriesByKind */
    short gate;          /* +0x0A */
    unsigned char unmodeled_0C[0x10 - 0x0C];
    Vector4 position;                   /* +0x10 */
    Vector4 rotation;                   /* +0x20 */
    Vector4 scale;                      /* +0x30 */
    Vector4 vertexScale;                /* +0x40: 1.0f in all four when unused */
    int inUse;                          /* +0x50 */
    int castsShadow;                    /* +0x54: draws the shadow block at +0xE4 */
    struct SefRecordOwner *flagOwner;   /* +0x58: owner read under flag 0x2000 */
    struct SefRecordOwner *ownedObject; /* +0x5C: owner read under flag 0x4000 */
    struct SefRecordOwner *actors[4];   /* +0x60: passed on to func_002018E8 */
    int ownerKey;                       /* +0x70 */
    unsigned char unmodeled_74[0xE4 - 0x74];
    unsigned char shadowBlock[0xF4 - 0xE4]; /* +0xE4: drawn by sdvShadowBlockDraw */
    float cullRadius;                       /* +0xF4 */
    float clipRadius;                       /* +0xF8: radius of the clip test in sefCullSchedulerEntries */
    unsigned char unmodeled_FC[0x150 - 0xFC];
} SvEffectRecordView;

/* The request block's owner key and clip radius, as svDrawEntry fills them (SefLinkedContext, include/main/tu132.h). */
typedef struct SvRequestBlock {
    unsigned char unmodeled_000[0x18];
    int ownerKey; /* +0x018 */
    unsigned char unmodeled_01C[0x2C4 - 0x1C];
    float kind1VectorY; /* +0x2C4 */
} SvRequestBlock;

/* The draw item's flag word at +0x694: bit 0x4000 keeps the line for the effect scene. */
typedef struct SvDrawItemFlags {
    unsigned char unmodeled_000[0x694];
    int flags;
} SvDrawItemFlags;

static struct SefRecordOwner *svRecordOwnedObject;
extern Matrix4 svEffectScaledMatrix;
static int svLineEffectPending;
void func_002018E8(struct SefRecordOwner *source, struct SefRecordOwner *target, struct SefRecordOwner *actor0,
    struct SefRecordOwner *actor1, struct SefRecordOwner *actor2, struct SefRecordOwner *actor3);
int sefBuildEffectMatrix(const Vector4 *rotation, const Vector4 *translation, const Vector4 *scale, Matrix4 *effectMatrix);
void sefLineEffectDraw(int lineIndex, int mode);

/* Draws one effect record: sets its owners and matrices as the current ones, then draws its lines in sort order. */
void svDrawEntry(recordIndex)
    int recordIndex;
{
    SvEffectRecordView *record;
    SvRequestBlock *requestBlock;
    XglCamera *camera;
    Matrix4 *viewMatrix;
    Vector4 *scale;
    SvDrawItemFlags *item;
    int pairCount;

    record = &((SvEffectRecordView *) sefGetScheduler())[recordIndex];
    if (record->inUse == 0 || record->gate == 0) {
        return;
    }
    if (_draw3D != SV_RENDER_3D_MODEL) {
        svUploadViewFrame();
        svSetZTestEnv(!((record->ownerGroupFlags >> 2) & 1));
    }
    ((SefCurrentEffectSlot *) &sefCurrentEffectRecord)->record = (SefEffectRecord *) record;
    svRecordFlagOwner = record->flagOwner;
    svRecordOwnedObject = record->ownedObject;
    requestBlock = (SvRequestBlock *) sefGetBattleData();
    requestBlock->ownerKey = record->ownerKey;
    requestBlock->kind1VectorY = record->cullRadius;
    func_002018E8(record->flagOwner, record->ownedObject, record->actors[0], record->actors[1], record->actors[2],
        record->actors[3]);
    svRecordMatrix = 0;
    __asm__ __volatile__(
        "lqc2 vf1, 32(%1)\n"
        "lqc2 vf2, 16(%1)\n"
        "lqc2 vf3, 0(%1)\n"
        "sqc2 vf0, 48(%0)\n"
        "sqc2 vf1, 32(%0)\n"
        "sqc2 vf2, 16(%0)\n"
        "sqc2 vf3, 0(%0)\n"
        : : "r"(svEffectMatrix), "r"(MMathUnitMatrix) : "memory");
    camera = xglStudioGetActiveCamera();
    viewMatrix = (Matrix4 *) camera->volume[0].viewMatrix;
    svViewMatrix = viewMatrix;
    QuadwordCopyMatrixT0(svDrawStatePacket, viewMatrix);
    if (record->ownerGroupFlags & 0x1000000) {
        sefBuildEffectMatrix(&record->rotation, &record->position, 0, &svEffectMatrix);
        scale = &record->scale;
        MMathScaleMatrix(&svEffectScaledMatrix, &svEffectMatrix, scale);
        svRecordMatrix = &svEffectScaledMatrix;
        MMathMulMatrix((Matrix4 *) svDrawStatePacket, svViewMatrix, &svEffectScaledMatrix);
    } else {
        scale = &record->scale;
    }
    __asm__ __volatile__("lq $8, 0(%3)\nsq $8, 0(%2)\n" : "=m"(svEffectScaleVector), "+m"(*scale) : "r"(&svEffectScaleVector), "r"(scale) : "$8", "memory");
    MMathMulMatrix((Matrix4 *) svDrawStatePacket + 1, viewMatrix, sefCameraMatrix);
    pairCount = svCollectSortedLines((SvSchedulerEntryView *) record, SV_SCRATCHPAD_SORT_KEYS);
    if (record->vertexScale.x == 1.0f && record->vertexScale.y == 1.0f && record->vertexScale.z == 1.0f
        && record->vertexScale.w == 1.0f) {
        svVertexScale = 0;
    } else {
        svVertexScale = &record->vertexScale;
    }
    svRecordScale = (Vector4 *) svVertexScale;
    if (_draw3D == SV_RENDER_2D && (record->ownerGroupFlags & 0x800000)) {
        svSortLineSlotsByDepth((SefEffectRecord *) record, SV_SCRATCHPAD_SORT_KEYS, pairCount, (SvDrawContext *) camera);
        svDrawSortedLines((SefEffectRecord *) record);
    } else {
        svLineEffectPending = 0;
        /* The sorted pairs svCollectSortedLines left in the scratchpad. kept: the parameter is the loop counter (a new local changes the code). */
        for (recordIndex = 0; recordIndex < pairCount; recordIndex++) {
            if (SV_SCRATCHPAD_SORT_KEYS[recordIndex].on == 0) {
                item = (SvDrawItemFlags *) sefGetLineAdr(SV_SCRATCHPAD_SORT_KEYS[recordIndex].lineIndex);
                if (_draw3D == SV_RENDER_2D && ((svEffectSceneFlag != 0 && !(item->flags & 0x4000))
                    || (svEffectSceneFlag == 0 && (item->flags & 0x4000)))) {
                    continue;
                }
                if (svLineEffectPending != 0 && _draw3D != SV_RENDER_3D_MODEL) {
                    svUploadViewFrame();
                    svLineEffectPending = 0;
                }
                svDrawImageItem((SvDrawItem *) item, SV_SCRATCHPAD_SORT_KEYS[recordIndex].sortKey);
            } else if (_draw3D == SV_RENDER_2D) {
                sefLineEffectDraw(SV_SCRATCHPAD_SORT_KEYS[recordIndex].lineIndex, 0);
                svLineEffectPending = 1;
            }
        }
    }
}

/* Transforms a point by the clip matrix (vf21-vf24, set by sefInitClipViewVolume); a null input means the origin. */
void sefTransformClipPoint(Vector4 *outPoint, const Vector4 *inPoint) {
    if (inPoint != 0) {
        __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(inPoint) : "memory");
    } else {
        __asm__ __volatile__("vmove.xyzw vf1, vf0" : : : "memory");
    }
    __asm__ __volatile__("vmulax.xyzw ACC, vf21, vf1x" : : : "memory");
    __asm__ __volatile__("vmadday.xyzw ACC, vf22, vf1y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyzw ACC, vf23, vf1z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyzw vf1, vf24, vf1w" : : : "memory");
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(outPoint) : "memory");
}

/* TU-local view of the effect record sefGetScheduler returns (0x150 apart): the gate, the 0x2000 and 0x4000 owners and
 * the cull radius are not in the shared SefEffectRecord. */
extern int sefEffectRecordCount;
void func_001F8860(void);
extern SvDrawSlot svVisibleSlots[];
extern int svVisibleEntryCount;

/* Culls the effect records into the visible slot list (svVisibleEntryCount gets the count), then sorts it by depth, largest first. */
void sefCullSchedulerEntries(void) {
    int recordIndex;
    SvEffectRecordView *record;
    struct XglCamera *camera;
    SvDrawSlot *visibleSlots;
    SvDrawSlot *swapSlot;
    SvDrawSlot *lowSlot;
    SvDrawSlot *highSlot;
    Vector4 *point;
    Vector4 *clipPoint;
    float originZ;
    float radius;
    int visibleCount = 0;
    int lowerIndex;
    int sortGap;
    int sortPos;

    record = (SvEffectRecordView *) sefGetScheduler();
    camera = xglStudioGetActiveCamera();
    clipPoint = (Vector4 *) SV_SCRATCHPAD;
    visibleSlots = svVisibleSlots;
    memset(visibleSlots, 0, 0x280);
    __asm__ __volatile__("lqc2 vf21, 0(%0)\n\t"
                         "lqc2 vf22, 16(%0)\n\t"
                         "lqc2 vf23, 32(%0)\n\t"
                         "lqc2 vf24, 48(%0)"
                         :
                         : "r"(&camera->volume[0].viewMatrix));
    sefInitClipViewVolume(camera->volume[0].planeSet[0][0]);
    sefTransformClipPoint(clipPoint, 0);
    originZ = clipPoint->z;
    for (recordIndex = 0; recordIndex < sefEffectRecordCount; recordIndex++, record++) {
        if (record->inUse == 0 || record->ownerKey == 0 || record->gate == 0) {
            continue;
        }
        visibleSlots[visibleCount].recordIndex = recordIndex;
        visibleSlots[visibleCount].disabled = 0;
        if (record->ownerGroupFlags & 0x01006000) {
            radius = record->clipRadius;
            if (record->ownerGroupFlags & 0x2000) {
                struct SefRecordOwner *flagOwner;
                flagOwner = record->flagOwner;
                point = (flagOwner != 0) ? &flagOwner->position : &record->position;
            } else if (record->ownerGroupFlags & 0x4000) {
                struct SefRecordOwner *ownedObject;
                ownedObject = record->ownedObject;
                point = (ownedObject != 0) ? &ownedObject->position : &record->position;
            } else {
                point = &record->position;
                if (1.0f < record->scale.x) {
                    radius = radius * record->scale.x;
                }
            }
            sefTransformClipPoint(clipPoint, point);
            if (!((record->ownerGroupFlags & 0x10) == 0 && sefClipViewVolumeA(point, radius))) {
                visibleSlots[visibleCount].depth = clipPoint->z;
                visibleCount++;
            }
        } else {
            visibleSlots[visibleCount].depth = originZ;
            visibleCount++;
        }
    }
    svVisibleEntryCount = visibleCount;
    func_001F8860();
    visibleCount = svVisibleEntryCount;
    for (sortGap = visibleCount / 2; sortGap > 0; sortGap /= 2) {
        for (sortPos = sortGap; sortPos < visibleCount; sortPos++) {
            lowerIndex = sortPos - sortGap;
            if (lowerIndex >= 0) {
                do {
                    lowSlot = (SvDrawSlot *) ((lowerIndex << 3) + (int) visibleSlots);
                    highSlot = (SvDrawSlot *) (((lowerIndex + sortGap) << 3) + (int) visibleSlots);
                    lowerIndex -= sortGap;
                    if (!(highSlot->depth > lowSlot->depth)) {
                        break;
                    }
                    swapSlot = (SvDrawSlot *) (SV_SCRATCHPAD + 0x10);
                    *swapSlot = *highSlot;
                    *highSlot = *lowSlot;
                    *lowSlot = *swapSlot;
                } while (lowerIndex >= 0);
            }
        }
    }
}

extern int sefDraw3DForced;
/* Draws one effect record: sets its owners and matrices as the current ones, then draws its lines in sort order. */ void svDrawEntry();

/* Draws the visible records whose entryIndex is index; while sefDraw3DForced is set, only the records with ownerKey 0x1006402A.
 * unusedFlag is not read by the body (the callers pass 0). */
void svDrawEntriesByKind(int entryIndex, int unusedFlag) {
    SvEffectRecordView *records;
    SvDrawSlot *entry;
    int visitedCount = 0;

    records = (SvEffectRecordView *) sefGetScheduler();
    if (svVisibleEntryCount > 0) {
        entry = svVisibleSlots;
        do {
            short recordIndex = entry->recordIndex;
            short disabled = entry->disabled;
            int forcedMode = sefDraw3DForced;
            entry++;
            if (forcedMode == 0) {
                int slotIsFree = disabled == 0;
                if (slotIsFree) {
                    if (records[recordIndex].entryIndex == entryIndex) {
                        svDrawEntry(recordIndex, disabled, svVisibleEntryCount);
                    }
                }
            } else {
                if (records[recordIndex].ownerKey == 0x1006402A) {
                    int slotIsFree = disabled == 0;
                    if (slotIsFree) {
                        if (records[recordIndex].entryIndex == entryIndex) {
                            svDrawEntry(recordIndex, disabled, svVisibleEntryCount);
                        }
                    }
                }
            }
            visitedCount++;
        } while (visitedCount < svVisibleEntryCount);
    }
}

/* Draws the alter layer only in 2D mode (_draw3D == SV_RENDER_2D). */
void svDrawAlters(int layer) {
    if (_draw3D == SV_RENDER_2D) {
        sdvDrawAlters(layer);
    }
}

void sdvShadowBlockDraw(unsigned char *block, int kind);
void sefDrawEffectGroup(int index, int *activeMapper, int flag);
void sdvAlterDrawOwned(int index);
void func_001F98C8(int layer, int drawn);

/*
 * Draws the scene. List 0 is loaded first. In 2D with flag 1 of svSceneFlags every visible effect record is drawn in
 * depth order, switching type lists as the records need them; otherwise each list 2..43 that has images is loaded and
 * its entries and alters are drawn. The records' shadow blocks come last, outside the effect scene.
 */
void svDrawScene(void) {
    SefDrawKindCounts *drawCounts;
    SvEffectRecordView *record;
    int drawMode;
    int hasEntries;
    int activeMapper;
    int visibleIndex;
    int listIndex;
    int recordIndex;

    activeMapper = -1;
    drawMode = _draw3D;
    drawCounts = sefEffectTallyAt(0);
    if (sefDraw3DForced != 0 && drawMode != 1) {
        return;
    }
    svDrawSendStaticRef();
    hasEntries = 0;
    svUploadViewFrame();
    svSetZTestEnv(1);
    if ((drawMode == SV_RENDER_2D && drawCounts[0].counts[0] != 0) || (drawMode == SV_RENDER_2D_SPRITE && drawCounts[0].counts[1] != 0) ||
        (svEffectSceneFlag != 0 && drawCounts[0].counts[2] != 0)) {
        hasEntries = 1;
    }
    if (svResourceTypeLists[0].used != 0) {
        svLoadImageList(&svResourceTypeLists[0]);
        svRefMapperPacket();
    }
    if ((svSceneFlags & 1) && drawMode == SV_RENDER_2D) {
        SvEffectRecordView *records;

        records = (SvEffectRecordView *) sefGetScheduler();
        for (visibleIndex = 0; visibleIndex < svVisibleEntryCount; visibleIndex++) {
            recordIndex = svVisibleSlots[visibleIndex].recordIndex;
            if (svVisibleSlots[visibleIndex].disabled == 0) {
                record = &records[recordIndex];
                if (record->entryIndex >= 2 && record->entryIndex != activeMapper &&
                    svResourceTypeLists[record->entryIndex].entryCount > 0) {
                    activeMapper = record->entryIndex;
                    svLoadImageList(&svResourceTypeLists[record->entryIndex]);
                    svRefMapperPacket();
                }
                if (record->gate != 0) {
                    svDrawEntry(recordIndex);
                    sdvAlterDrawOwned(recordIndex);
                }
            } else if (svVisibleSlots[visibleIndex].disabled == 2) {
                if (_draw3D != SV_RENDER_2D_SPRITE) {
                    sefDrawEffectGroup(recordIndex, &activeMapper, 1);
                }
            }
        }
    } else {
        if (hasEntries) {
            svDrawEntriesByKind(0, 0);
        }
        if (sefDraw3DForced == 0) {
            svDrawAlters(0);
        }
        for (listIndex = 2; listIndex < 44; listIndex++) {
            if (svResourceTypeLists[listIndex].entryCount > 0) {
                hasEntries = 0;
                svLoadImageList(&svResourceTypeLists[listIndex]);
                svRefMapperPacket();
                if ((drawMode == SV_RENDER_2D && drawCounts[listIndex].counts[0] != 0) ||
                    (drawMode == SV_RENDER_2D_SPRITE && drawCounts[listIndex].counts[1] != 0) ||
                    (svEffectSceneFlag != 0 && drawCounts[listIndex].counts[2] != 0)) {
                    hasEntries = 1;
                }
                if (hasEntries) {
                    svDrawEntriesByKind(listIndex, 0);
                }
                if (sefDraw3DForced == 0) {
                    if (svLineEffectPending != 0) {
                        svUploadViewFrame();
                    }
                    svDrawAlters(listIndex);
                    if (_draw3D != SV_RENDER_2D_SPRITE) {
                        func_001F98C8(listIndex, hasEntries);
                    }
                }
            }
        }
    }
    if (svEffectSceneFlag == 0) {
        SvEffectRecordView *records;

        records = (SvEffectRecordView *) sefGetScheduler();
        for (listIndex = 0; listIndex < sefEffectRecordCount; listIndex++) {
            record = &records[listIndex];
            if (record->castsShadow != 0) {
                sefCurrentEffectRecord = (SefEffectDrawState *) record;
                sdvShadowBlockDraw(record->shadowBlock, record->entryIndex);
            }
        }
    }
}

/* Sets the 3D flag and draws the scene when svDrawListBegin(0) reports it. */
void svDrawScene3D(void) {
    if (svDrawListBegin(0) != 0) {
        _draw3D = SV_RENDER_2D_SPRITE;
        svDrawScene();
    }
}

/* Clears the 3D flag and draws the scene when svDrawListBegin(1) reports it. */
void svDrawScene2D(void) {
    _draw3D = SV_RENDER_2D;
    if (svDrawListBegin(1) != 0) {
        svDrawScene();
    }
}

/* Draws the effect scene: list 2 of svDrawListBegin, with svEffectSceneFlag set while svDrawScene runs. */
void svDrawSceneEffects(void) {
    _draw3D = SV_RENDER_2D;
    if (svDrawListBegin(2) != 0) {
        svEffectSceneFlag = 1;
        svDrawScene();
        svEffectSceneFlag = 0;
    }
}

void MMathInvertAffineMatrix(Matrix4 *out, float *src);
extern const Matrix4 D_0053F460;

/* Original names that no function of this file uses. */
void *nmlPacketSetAttributeAlloc16N(int count);
int sefCalcRotTransSMatrix(const Vector4 *rotation, const Vector4 *translation, const Vector4 *scale, Matrix4 *out);
void nmlModelSetRenderLevel(int flags);
extern s16 _sefBattleMode;
typedef struct SvImageMapperStorage {
    SvTypeList typeListStorage[56];
    unsigned char unmodeled_tail[0x2420];
} SvImageMapperStorage;
extern SvImageMapperStorage _imageMapper;

const Matrix4 D_0053F460 = {
    { 0.264445007f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.343434989f, 0.0f, 0.0f },
    { 0.0f, 0.0f, -1.00000203f, -1.0f },
    { 0.0f, 0.0f, -0.0199999996f, 0.0f }
};

/* Draws the effect records in 3D mode: sets the camera matrices, culls the records, then draws each visible kind. */
void svDrawEffectRecords3D(void) {
    Matrix4 volumeMatrix;
    XglCamera *camera;
    SefDrawKindCounts *drawKindCounts;
    int drawKindIndex;

    drawKindCounts = sefEffectTallyAt(0);
    _draw3D = SV_RENDER_3D_MODEL;
    camera = xglStudioGetActiveCamera();
    QuadwordCopyMatrixT0(D_00770ED0, camera->travel.transform.rows);
    MMathInvertAffineMatrix(&volumeMatrix, camera->volume[0].viewMatrix[0]);
    MMathMulMatrix(&D_00770ED0[1], &volumeMatrix, &D_0053F460);
    memset(D_0076D7E0, 0, 0x180);
    QuadwordCopyT0(&D_0076D7E0[6], &vu0IdentityMatrix[0]);
    sefCullSchedulerEntries();
    for (drawKindIndex = 0; drawKindIndex < 44; drawKindIndex++) {
        if (drawKindCounts[drawKindIndex].counts[3] > 0) {
            svDrawEntriesByKind(drawKindIndex, 0);
        }
    }
}

/* <<< CHUNK sv_get_image_item */

Vector4 D_00475440[2] = { { 2048.0f, 2048.0f, 268435360.0f, 0.0f }, { 1.0f, -1.14285719f, 0.0625f, 1.0f } };
SvDrawMode svDrawModeTagTable[16] = {
    { 0, 0 },      { 0x6D0, 14 }, { 0x9D8, 28 }, { 0x1D8, 14 },  { 0x430, 28 },  { 0xD28, 17 }, { 0xD58, 34 }, { 0xD38, 17 },
    { 0xD68, 34 }, { 0xD48, 17 }, { 0xD78, 34 }, { 0x1688, 14 }, { 0x18D8, 28 }, { 0xD88, 3 },  { 0xDD8, 3 },  { 0, 14 },
};
SvGsRegister D_004754E0[8] = {
    { 0x1000000000008007ULL, 0xE },
    { 0x5000D, 0x47 },        /* TEST_1 */
    { 0x60, 0x14 },           /* TEX1_1 */
    { 5, 0x08 },              /* CLAMP_1 */
    { 0x131000000ULL, 0x4E }, /* ZBUF_1 */
    { 0, 0x4A },              /* FBA_1 */
    { 0, 0x4C },              /* FRAME_1 */
    { 0, 0x40 },              /* SCISSOR_1 */
};
/* VIF direct of two quadwords: TEXFLUSH. */
u64 svMapperPacketEnv[6] = { 0, 0x5100000200000000ULL, 0x1000000000008001ULL, 0xE, 0, 0x3F };
/* VIF direct of three quadwords: TEST_1 and ZBUF_1. */
SvDrawTagArea svDrawStateTagArea = {
    { 0, 0x5100000300000000ULL, 0x1000000000008002ULL, 0xE }, 0x5000D, 0x47, { 0x131000000ULL, 0x4E }
};
Vector4 svScaledVertexClampMax = { 255.0f, 255.0f, 255.0f, 255.0f };
Vector4 D_004755E0[2] = { { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f, 0.0f } };
SvTypeList svResourceTypeLists[44] = { 0 };
SefPartTransform gSdvCameraPartTransform = { 0 };
SvDrawSlot svVisibleSlots[82] = { 0 };
SvTypeList svResourceTypeListScratch = { 0 };

void *svCurrentVertexViewBlock = D_0076D7E0;
int svDepthNodeCount = 0;
int _draw3D = 0;
int svFogValue = -1;
extern DrawStatePacket D_00770FE0;
DrawStatePacket *svDrawStatePacket = &D_00770FE0;
int D_005510D4 = 0;
int svVisibleEntryCount = 0;

XglLightSet D_0076D4E0;
XglLightSet sefActiveLightSet;
Vector4 D_0076D7E0[24];
SvDepthNode svDepthNodes[1100];
SvDepthNode svDepthBuckets[40];
Matrix4 D_00770ED0[2];
Matrix4 svEffectScaledMatrix;
Matrix4 svEffectMatrix;
Vector4 svEffectScaleVector;
DrawStatePacket D_00770FE0;
SvViewUpload svViewUpload;

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

extern int D_005510C4;
extern unsigned char D_00770CF0[];
extern unsigned char D_0076D960[];
extern void *memset(void *destination, int value, unsigned int count);

void svSortReset(void)
{
    D_005510C4 = 0;
    memset(D_00770CF0, -1, 480);
    memset(D_0076D960, -1, 13200);
}

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", func_002087F0);

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

typedef struct SvResourceChunk {
    unsigned int byteSize;
    unsigned char unmodeled_04[4];
    short kind;
    unsigned short typeFlags;
    char payload[0x34];
} SvResourceChunk;

typedef struct SvImageChunk {
    unsigned int byteSize;
    unsigned int dataSize;          /* bytes of pixel data after the header */
    unsigned char unmodeled_08[2];
    unsigned short typeFlags;
    unsigned short width;
    unsigned short height;
    short bitsPerPixel;
    unsigned char unmodeled_12[2];
    char name[0x2C];
} SvImageChunk;

/* A CLUT chunk: the payload's leading halfwords are the size and depth, the name follows at +0x14. */
typedef struct SvClutChunk {
    unsigned char unmodeled_00[10];
    unsigned short typeFlags;
    unsigned short width;
    unsigned short height;
    unsigned short bitsPerPixel;
    unsigned char unmodeled_12[2];
    char name[0x2C];
    unsigned char data[1];
} SvClutChunk;

extern int _draw3D;

/* The _draw3D modes: 2D items, 2D kind-8 sprites and 3D models. */
enum SvRenderMode {
    SV_RENDER_2D = 0,
    SV_RENDER_2D_SPRITE = 1,
    SV_RENDER_3D_MODEL = 2
};

int svImageListAlloc(SvTypeList *typeList);
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
extern int svDrawMode;
void svDrawListAppend(int code, int operand_a, int operand_b);
void svDrawListEmit(XglPacket *packet, DrawListEntry *entry, int pending_count);

/* Queued records that trigger svDrawListFlush from svDrawListTryFlush. */
#define SV_DRAW_LIST_FLUSH_THRESHOLD 0x1AE

extern DrawList svDrawList;
extern u32 svDrawModeTagTable[];
extern int svStaticRefQuadwordCount[];
extern unsigned char svStaticRefData[];
extern DrawStatePacket *svDrawStatePacket;
void sceVif1PkAddUpkData128(XglPacket *packet, Quadword word);
extern u8 D_00000080[];
extern u8 D_00000110[];
extern u32 svDrawRecordDivisors[];

/* VRAM size svAddImage reserves for a square 4-bit image of at most 16 pixels. */
#define SV_IMAGE_SQUARE_4BIT_VRAM 0x100
/* Bytes svAddClut advances the list by: a 256-colour or a 16-colour CLUT block. */
#define SV_CLUT_8BIT_BYTES 256
#define SV_CLUT_4BIT_BYTES 128

/* The type lists sit back to back at 0x475600 (scaffold data, no name in config/symbols): svGetTypeList
 * indexes them by 5084 bytes (one SvTypeList). svImageListCreate clears one 5084-byte list at 0x4AC2C0. */
extern SvTypeList D_00475600[];
extern SvTypeList D_004AC2C0;
extern u64 D_0053F258[];
SvTypeList *svGetTypeList(int type);
SvTypeListEntry *svGetImageListItemSub(int type, unsigned int flags);
int sceVif1PkRefLoadImage(void *packet, u16 basePointer, int psm, int bufferWidth, const void *data, int quadwordCount, int, int, int, int);

/*
 * One svAddScript2 sub-record, at +0x1C of a 0x24-byte slot of the type
 * list's item array. Only the fields this function itself writes are
 * modeled, at their byte offset from this sub-record's own base: `codePtr`
 * (+0x00, the script's own +0xC0 address), `active` (+0x08, always 1),
 * `kind` (+0x0C, always 5, matching this function's own "script" name),
 * `id` (+0x0E, the script's own +0xA halfword), two cleared halfwords
 * (+0x18/+0x1A) and `dataPtr` (+0x20, the script's own +0x14 address).
 * +0x20 (slot +0x3C) lands past the slot's own 0x24-byte stride, into the
 * next slot's leading bytes -- evidenced by the original bytes, not
 * treated as a bug.
 */
typedef struct SvScriptSlot {
    void *codePtr;                  /* 0x00 */
    unsigned char unmodeled_04[4];
    int active;                      /* 0x08 */
    short kind;                       /* 0x0C */
    short id;                          /* 0x0E */
    unsigned char unmodeled_10[8];
    short clearedA;                     /* 0x18 */
    short clearedB;                      /* 0x1A */
    unsigned char unmodeled_1C[4];
    void *dataPtr;                        /* 0x20 */
} SvScriptSlot;

int svAddModel(SvTypeList *typeList, SvResourceChunk *chunk);
int svAddScript(SvTypeList *typeList, SvResourceChunk *chunk);
/* * Adds an image chunk to a type list: the entry takes the image's size, depth, * name and data, and the VRAM cursor advances by the storage the image takes. */ int svAddImage(SvTypeList *list, SvImageChunk *image);
int svAddClut(SvTypeList *typeList, SvClutChunk *chunk);
int svAddScript2(SvTypeList *typeList, SvResourceChunk *script);
extern u32 svDrawRecordDivisors[31];

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", svDrawListBegin);

/*
 * Appends one draw record at the end cursor: a VIF FLUSH, STCYCL 4,4, an
 * UNPACK to VU address `vuAddress` and the operands. The record starts at the
 * old end, which becomes begin_index, and end_index moves on by 2.
 */
void svDrawListAppend(int vuAddress, int operand_a, int operand_b)
{
    DrawListEntry *record;

    svDrawList.begin_index = svDrawList.end_index;
    record = (DrawListEntry *) (svDrawList.entries + svDrawList.begin_index);
    record->payload = 0;
    record->flush_word = 0x11000000;
    record->nop_word = 0;
    record->stcycl_word = 0x01000404;
    record->unpack_word = vuAddress | 0x6C010000;
    record->count_word = 0;
    record->operand_a = operand_a;
    record->operand_b = operand_b;
    svDrawList.end_index += 2;
}

/* Emits the VIF packet for `count` records at `entry`: sceVif1PkRef calls and UNPACK headers, split at 0x100 records. */
void svDrawListEmit(XglPacket *packet, DrawListEntry *records, int pending_count)
{
    DrawListEntry *header;
    int drawMode;
    int divisor;
    int countMinusOne;
    u32 sourceAddress;
    Quadword carriedQuad;

    drawMode = svDrawMode;
    sourceAddress = (u32) records & 0xfffffff;
    divisor = svDrawRecordDivisors[drawMode * 2];
    header = (DrawListEntry *) (svDrawList.entries + svDrawList.begin_index);
    countMinusOne = pending_count - 1;
    if (drawMode == 13) {
        header->count_word = (pending_count - 8) / divisor;
        header->operand_a = divisor;
    } else if (drawMode == 14) {
        header->count_word = ((pending_count - 14) / divisor) >> 1;
        header->operand_a = divisor;
    } else {
        header->count_word = countMinusOne / divisor;
        header->operand_a = divisor;
    }
    if (countMinusOne < 256) {
        header->unpack_word = (countMinusOne << 16) | 0x6C000028;
        sceVif1PkRef(packet, (const void *) sourceAddress, pending_count, 0, 0, 0);
        return;
    }
    if (countMinusOne == 256) {
        header->unpack_word = 0x6C000028;
        sceVif1PkRef(packet, (const void *) sourceAddress, pending_count, 0, 0, 0);
        return;
    }
    header->unpack_word = 0x6C000028;
    /* Record 257 (+0x1010) stays a byte offset: header[257] access changed the code. */
    carriedQuad = *(Quadword *) ((u8 *) header + 4112);
    sceVif1PkRef(packet, (const void *) sourceAddress, 257, 0, 0, 0);
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenUpkCode(packet, 296, 108, 4, 4);
    sceVif1PkAddUpkData128(packet, carriedQuad);
    sceVif1PkCloseUpkCode(packet);
    if (countMinusOne == 257) {
        return;
    }
    *(Quadword *) ((u8 *) header + 4112) = 0;
    *(int *) ((u8 *) header + 4124) = ((countMinusOne - 257) << 16) | 0x6C000129;
    sceVif1PkRef(packet, (const void *) ((u8 *) header + 4112), countMinusOne - 256, 0, 0, 0);
}

/*
 * Flushes the queued draw records. Runs only with at least three queued,
 * _draw3D != 2 and svDrawMode > 0: it emits the records, FLUSHes, calls the
 * draw microprogram of the current mode (MSCAL), then appends record 0x28.
 */
void svDrawListFlush(int pending_count)
{
    XglPacket *packet;
    u32 tag;
    DrawListEntry *entry;

    if (pending_count >= 3 && _draw3D != SV_RENDER_3D_MODEL && svDrawMode > 0) {
        packet = xglPacketGetCurrent();
        svDrawListEmit(packet,
                      (DrawListEntry *) (svDrawList.entries + svDrawList.begin_index),
                      pending_count);
        sceVif1PkCnt(packet, 0);
        sceVif1PkAddCode(packet, 0x11000000);
        tag = svDrawModeTagTable[svDrawMode * 2];
        sceVif1PkAddCode(packet, (tag >> 3) | 0x14000000);
        sceVif1PkAddCode(packet, 0x11000000);
        entry = (DrawListEntry *) (svDrawList.entries + svDrawList.begin_index);
        svDrawListAppend(0x28, entry->operand_a, entry->operand_b);
    }
}

void svDrawListTryFlush(void)
{
    int pending_count;

    pending_count = svDrawList.end_index - svDrawList.begin_index;
    if (pending_count >= SV_DRAW_LIST_FLUSH_THRESHOLD) {
        svDrawListFlush(pending_count);
    }
}

void svDrawSendStaticRef(void)
{
    XglPacket *packet;

    packet = xglPacketGetCurrent();
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddCode(packet, 0x11000000);
    sceVif1PkRef(packet, svStaticRefData, svStaticRefQuadwordCount[0], 0, 0, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", func_00208E78);

void svDrawSetState(int clampEnable, int stateIndex, int clampRepeat)
{
    DrawStatePacket *packet;
    DrawStatePacket *state;
    u64 alphaData;

    packet = svDrawStatePacket;
    packet->gif_tag = 0x1000000000008003ULL;
    packet->gif_registers = 0xE;
    if (clampEnable != 0 && clampRepeat == 0) {
        packet->clamp.data = 0;
    } else {
        packet->clamp.data = 5;
    }
    state = svDrawStatePacket;
    state->clamp.address = 8;
    state->zbuf.data = 0x9880ULL << 17;
    state->zbuf.address = 78;
    alphaData = D_0053F258[stateIndex];
    state->alpha.address = 66;
    state->alpha.data = alphaData;
}

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
    memset(&D_004AC2C0, 0, sizeof(SvTypeList));
}

void svImageListDestroy(SvTypeList *typeList)
{
    int index;

    for (index = 0; index < SV_ENTRY_COUNT; index++) {
        if (typeList->entries[index].used != 0) {
            typeList->entries[index].used = 0;
        }
    }
    typeList->typeKind = 0;
    typeList->savedSize = typeList->size;
    do { } while (0);
    typeList->entryCount = 0;
    typeList->used = 0;
    do { } while (0);
    typeList->dimensions = 0;
    typeList->pendingCount = 0;
}

int svImageListAlloc(SvTypeList *typeList)
{
    int index;

    for (index = 0; index < SV_ENTRY_COUNT; index++) {
        if (typeList->entries[index].used == 0) {
            return index;
        }
    }
    return -1;
}

SvTypeList *svGetTypeList(int type)
{
    return &D_00475600[type];
}

SvTypeListEntry *svGetImageListItemSub(int type, unsigned int flags)
{
    SvTypeList *typeList;
    SvTypeListEntry *entry;
    int index;

    typeList = svGetTypeList(type);
    entry = typeList->entries;
    index = 0;
    while (index < SV_ENTRY_COUNT) {
        if (entry->used != 0 && entry->type == (unsigned char) flags) {
            return entry;
        }
        entry++;
        index++;
    }
    return 0;
}

/* Looks the list up with the type 0 for flag 0x100, else with the given type; a negative type returns 0. */
SvTypeListEntry *svGetImageListItem(int type, unsigned int flags) {
    if ((flags & 0x100) != 0) {
        type = 0;
    }
    if (type < 0) {
        return 0;
    }
    return svGetImageListItemSub(type, flags);
}

void svLoadTexture(SvTypeListEntry *entry) {
    int psm;
    int quadwordCount;
    u16 basePointer;
    u16 width;
    u16 height;
    void *packet;

    height = entry->height;
    basePointer = entry->basePointer;
    width = entry->width;
    psm = GS_PSMT8;
    if (entry->bitsPerPixel != 8) {
        psm = GS_PSMT4;
    }
    quadwordCount = (entry->bufferWidthPixels * height) / 16;
    packet = xglPacketGetCurrent();
    sceVif1PkRefLoadImage(packet, basePointer, psm, (entry->bufferWidthPixels >> 6) + 1, entry->data, quadwordCount, 0, 0, width, height);
}

/* Uploads a palette: 16x16 for 8-bit entries, 8x2 for 4-bit, to the current packet. */
void svLoadClut(SvTypeListEntry *entry) {
    u16 basePointer;
    void *packet;

    basePointer = entry->basePointer;
    packet = xglPacketGetCurrent();
    if (entry->bitsPerPixel == 8) {
        sceVif1PkRefLoadImage(packet, basePointer, 0, 1, entry->data, 0x40, 0, 0, 0x10, 0x10);
        return;
    }
    sceVif1PkRefLoadImage(packet, basePointer, 0, 1, entry->data, 4, 0, 0, 8, 2);
}

int svLoadImageList(SvTypeList *typeList)
{
    int index;
    unsigned int imageCount;

    imageCount = 0;
    for (index = 0; index < SV_ENTRY_COUNT; index++) {
        if (typeList->entries[index].used) {
            if (typeList->entries[index].resourceKind == SV_RES_IMAGE) {
                if (imageCount == 0) {
                    void *packet;

                    packet = xglPacketGetCurrent();
                    sceVif1PkCnt(packet, 0);
                    sceVif1PkAddCode(packet, 0x11000000);
                }
                imageCount++;
                svLoadTexture(&typeList->entries[index]);
            } else if (typeList->entries[index].resourceKind == SV_RES_CLUT) {
                svLoadClut(&typeList->entries[index]);
            }
        }
    }
    return imageCount;
}

int svGetSizeBit(unsigned int value)
{
    unsigned int mask;
    int bitIndex;

    mask = 2;
    bitIndex = 1;
    while (bitIndex < 32) {
        if ((value & mask) != 0) {
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
int svAddImage(SvTypeList *list, SvImageChunk *image)
{
    SvTypeListEntry *entry;
    unsigned int width;
    unsigned int height;
    unsigned int storageSize;
    int index;

    index = svImageListAlloc(list);
    if (index < 0) {
        return -1;
    }
    entry = &list->entries[index];
    entry->resourceKind = SV_RES_IMAGE;
    entry->data = image + 1;
    entry->typeFlags = image->typeFlags;
    entry->used = 1;
    entry->width = image->width;
    width = entry->width;
    entry->height = image->height;
    entry->bitsPerPixel = image->bitsPerPixel;
    entry->textureWidthExponent = svGetSizeBit(width);
    height = entry->height;
    entry->textureHeightExponent = svGetSizeBit(height);
    entry->name = image->name;
    storageSize = image->dataSize;
    if (image->bitsPerPixel == 4) {
        unsigned long comparedHeight; /* height widened for the side comparisons */

        entry->bufferWidthPixels = (short)image->width >> 1;
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
        if (entry->height > entry->width) {
            entry->bufferWidthPixels = (short)image->width * (int)height / (int)width;
        } else {
            entry->bufferWidthPixels = image->width;
        }
    }
    entry->basePointer = list->savedSize >> 6;
    list->savedSize += storageSize >> 2;
    return index;
}

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", svAddClut);

int svAddModel(SvTypeList *typeList, SvResourceChunk *chunk)
{
    int index;
    SvTypeListEntry *entry;

    index = svImageListAlloc(typeList);
    if (index < 0) {
        return -1;
    }
    entry = &typeList->entries[index];
    entry->resourceKind = SV_RES_MODEL;
    entry->name = chunk->payload + 8;
    entry->typeFlags = chunk->typeFlags | 0x2000;
    entry->data = chunk + 1;
    entry->used = 1;
    entry->height = 0;
    entry->width = 0;
    return index;
}

int svAddScript(SvTypeList *typeList, SvResourceChunk *chunk)
{
    int index;
    SvTypeListEntry *entry;

    index = svImageListAlloc(typeList);
    if (index < 0) {
        return -1;
    }
    entry = &typeList->entries[index];
    entry->resourceKind = SV_RES_SCRIPT;
    entry->name = chunk->payload + 8;
    entry->typeFlags = chunk->typeFlags;
    entry->data = chunk + 1;
    entry->used = 1;
    entry->height = 0;
    entry->width = 0;
    return index;
}

int svAddScript2(SvTypeList *typeList, SvResourceChunk *script)
{
    int index;
    SvTypeListEntry *entry;

    index = svImageListAlloc(typeList);
    if (index < 0) {
        return -1;
    }
    entry = &typeList->entries[index];
    entry->resourceKind = SV_RES_SCRIPT;
    entry->name = script->payload + 8;
    entry->typeFlags = script->typeFlags;
    entry->data = (char *) script + 0xC0;
    entry->used = 1;
    entry->height = 0;
    entry->width = 0;
    return index;
}

void svAnalyzeChunk(SvTypeList *list, void *chunkFile)
{
    SvResourceChunk *chunk;

    chunk = *(SvResourceChunk **) chunkFile;
    while (chunk->kind != 0) {
        switch (chunk->kind) {
        case 1:
        case 2:
            chunk = chunk + 1;
            continue;
        case SV_RES_IMAGE:
            svAddImage(list, (SvImageChunk *) chunk);
            chunk = (SvResourceChunk *) ((unsigned char *) chunk + chunk->byteSize);
            continue;
        case SV_RES_CLUT:
            svAddClut(list, (SvClutChunk *) chunk);
            chunk = (SvResourceChunk *) ((unsigned char *) chunk + chunk->byteSize);
            continue;
        case 7:
            svAddImage(list, (SvImageChunk *) chunk);
            chunk = (SvResourceChunk *) ((unsigned char *) chunk + chunk->byteSize);
            continue;
        case SV_RES_MODEL:
        case 9:
            svAddModel(list, chunk);
            chunk = (SvResourceChunk *) ((unsigned char *) chunk + chunk->byteSize);
            continue;
        case SV_RES_SCRIPT:
            svAddScript(list, chunk);
            chunk = (SvResourceChunk *) ((unsigned char *) chunk + chunk->byteSize);
            continue;
        case 10:
            svAddScript2(list, chunk);
            chunk = (SvResourceChunk *) ((unsigned char *) chunk + chunk->byteSize);
            continue;
        default:
            return;
        }
    }
}

void svInitImageMapper(void)
{
    int i;
    unsigned int size;
    int capacity;

    memset(D_00475600, 0, 44 * sizeof(SvTypeList));
    for (i = 0; i < 44; i++) {
        if (i == 0) {
            size = 0xE0000;
            capacity = 0x10000;
        } else {
            size = 0xF0000;
            capacity = 0x10000;
        }
        D_00475600[i].size = size;
        D_00475600[i].savedSize = size;
        D_00475600[i].capacity = capacity;
        D_00475600[i].entryCount = 0;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", func_00209C88);

/* Row-major 4x4 float matrix. */
/* A vector kept as a plain float[4] (like sceVu0FVECTOR); its w is element 3. Vector4 here changes the
 * address arithmetic of the w stores, so the casts to Vector4 * at the calls stay. */
void *nmlPacketSetAttributeAlloc16N(int count);

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
 * an SvPart, and the object sefGlobalTransform. */
typedef struct SefPartTransform {
    Matrix4 matrix;   /* +0x00 */
    Vector4 position; /* +0x40 */
    float scale;      /* +0x50 */
    short scaled;     /* +0x54: nonzero when scale applies */
    short kind;       /* +0x56 */
} SefPartTransform;

extern SefPartTransform sefGlobalTransform;
void svLoadDrawViewMatrix(void);
void svBuildDrawStatePacket(void);
void sefTransformClipPoint(Vector4 *out, const Vector4 *in);
float func_00203360(int id);
void sefGetPosition(Vector4 *dest, const Vector4 *source, int kind);
int sefInitPartTransform(SefPartTransform *part, int kind, int scaleKey100, int scaleKey200);
struct SvDrawItem;
void svDrawScene2D(void);
void svDrawScene3D(void);
void sdvDrawAlters(int layer);
void sdvDrawOwnerAlters(int owner);
void sefDrawResources(int slotIndex);
int sefGetActiveCount(void);
extern const Matrix4 MMathUnitMatrix;
short MMathIsVectorEqual(const Vector4 *first, const Vector4 *second);
float MMathVectorDotProduct(const Vector4 *first, const Vector4 *second);
Vector4 *MMathApplyMatrix(Vector4 *out, Matrix4 *matrix, const Vector4 *vector);
Matrix4 *MMathMulMatrix(Matrix4 *out, Matrix4 *rhs, Matrix4 *lhs);
Matrix4 *MMathScaleMatrix(Matrix4 *out, Matrix4 *matrix, const Vector4 *scale);
Matrix4 *MMathScaleMatrixRotS(Matrix4 *out, Matrix4 *matrix, float scale);
struct XglCamera;

#define SEF_TABLE_RECORD_SIZE 0xA70

SefEffectRecord *sefGetScheduler(void);
int sefFlagTest(int mask);
struct SefLocalData;
struct SefGroupObject;
void sefGetPosition(Vector4 *dest, const Vector4 *source, int kind);
int sefCalcRotTransSMatrix(const Vector4 *rotation, const Vector4 *translation, const Vector4 *scale, Matrix4 *out);
void sefLoadRequestBuffer(int p0, int p1, int p2, int p3, int p4, int p5, int p6, int p7);
float sefGetEntryScale(int id);

/* The fog override block sefSetFogOverride fills (the statics sefOverrideFlag and the two override vectors of sef.c,
 * one object in the original): the enabled word, four fog distances at +0x10 and the fog colour at +0x20. */
typedef struct SefFogOverride {
    int enabled;
    unsigned char unmodeled_04[0x10 - 0x04];
    float dist[4];
    Vector4 fogColor;
} SefFogOverride;

SefFogOverride *sefGetFogOverrideBlock(void);
struct SefLocalData *sefGetLineAdr(int entryIndex);
unsigned char *sevGetPtAllocator2(int slotIndex);

#define GS_TEX0_1 0x06
#define GS_ALPHA_1 0x42
#define GS_TEST_1 0x47
/* GIFtag register descriptor of an A+D (address plus data) register. */
#define GIF_AD 0x0E
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
void nmlModelSetStencilAlpha(long long value);
void nmlModelSetDrawMatrix(Matrix4 matrix);
void nmlModelSetStencil(int enabled);
void nmlModelSetFogCol(Vector4 *position);
void nmlModelSetFogDist(float fogNear, float fogFar, float fogMin, float fogMax);
void nmlModelSetRenderLevel(int flags);
void nmlModelSetScale(const Vector4 *src);
void nmlModelSetSortOffset(float value);
void nmlModelSetTexture(TxyBlock *block);
void nmlModelSetTransparency(float value);
void nmlModelSetZwrite(int enabled);
void pxySetAnimTime(float value);
void pxySetLightCopy(XglLightSet *light);
int pxySubmitModel(PxyHeader *header);
struct XglCamera *xglStudioGetActiveCamera(void);
/* The identity matrix the rotation functions start from (test_128.c defines it). */
extern const Vector4 vu0IdentityMatrix[4];
void vu0CurMatrixMulRight(Vector4 *rows);
void vu0CurMatrixMulRows3(Vector4 *rows);
void vu0RotateCurrentZ(float angle);
void vu0TransformPoint(Vector4 *point);
void vu0CurMatrixMul(Matrix4 matrix);
/* Scaffold .bss object of this TU: 384 bytes at svEffectLightSet (the scaffold split it at 0x6AF890, a 16-byte aligned vector at +0xD0). */
extern u8 svEffectLightSet[384];
extern void *svActiveLightSet;
extern int svSortCount;
extern int svFogValue;
/* Scaffold .bss object D_0076D4D0 (16 bytes): the Z-test word the draw packets carry (register 0x47), written by svSetZTestEnv. */
extern u64 D_0076D4D0[2];
/* Scaffold .sbss int D_00551518: the Z-test enable flag written by svSetZTestEnv. */
extern int D_00551518;
/* Scaffold .sbss pointer D_00551520 (4 bytes): the optional scale vector of the draw records, null when unset. */
extern const Vector4 *D_00551520;
extern const Vector4 *svRecordScale;
extern int svRequestedMapperIndex;
extern int svActiveMapperIndex;
extern void *svRecordSourcePointer;
extern void *svRecordTargetPointer;
extern int svItemModeFlags;
extern Vector4 *svRecordSourceRowsPointer;
extern int svDrawMode;
extern int svPolygonAxesMode;
extern Matrix4 *svRecordMatrix;
extern int svPivotAddress;
extern Matrix4 *svViewMatrix;
extern int svItemDrawFlags;
extern void *memset(void *destination, int value, unsigned int count);
void svDrawListAppend(int code, int operand_a, int operand_b);
void svDrawListEmit(XglPacket *packet, DrawListEntry *entry, int pending_count);

/* Byte offsets of the two draw list buffers and the flush record code (svDrawList.flush_code). */
#define SV_DRAW_LIST_FIRST_OFFSET 0xE00
#define SV_DRAW_LIST_SECOND_OFFSET 0x200
#define SV_DRAW_LIST_FLUSH_CODE 0x28
/* Queued records that trigger svDrawListFlush from svDrawListTryFlush. */
#define SV_DRAW_LIST_FLUSH_THRESHOLD 0x1AE
/* Depth-sort buckets (svSortBuckets) and the queued item limit (svSortItems). */
#define SV_SORT_BUCKET_COUNT 512
#define SV_SORT_ITEM_LIMIT 1200

/* One depth-sort item (svSortItems, 12 bytes): the next item index in its bucket, -1 at the end, then the id, flags and depth. */
typedef struct SortItem {
    short next;
    short id;
    short flags;
    float depth;
} SortItem;

/* One depth bucket (svSortBuckets, 12 bytes): the head item index, -1 when empty, and the tail word that the first insert sets. */
typedef struct SortBucket {
    short head;
    unsigned char unmodeled_02[4];
    short tail;
    unsigned char unmodeled_08[4];
} SortBucket;

/* The view block svDrawSendViewSetup uploads to VU1: a unit rotation with the origin, the screen scale and offset (z scaled by 1/16, w cleared) and the view matrix. */
typedef struct SvViewUpload {
    Matrix4 unit;
    Vector4 scale;
    Vector4 offset;
    Matrix4 view;
} SvViewUpload;

/* .bss of this TU in address order (data recovery pass 4): the sort chain, the matrix and vector scratch, the view upload,
 * the draw list header and the draw-list storage that follows it. */
extern SortItem svSortItems[SV_SORT_ITEM_LIMIT];
extern SortBucket svSortBuckets[SV_SORT_BUCKET_COUNT];
extern Matrix4 svEffectCameraMatrices[4];
extern Vector4 svEffectScaleVector;
extern SvViewUpload svViewUpload;
extern DrawList svDrawList;
extern u32 svDrawModeTagTable[];
extern int svStaticRefQuadwordCount[];
extern unsigned char svStaticRefData[];
void svSortReset(void);
void svSortAdd(int id, int flags, float depth);
/* Read-only scaffold items of this TU from svAlphaTestTable up to svEffectProjectionMatrix: GS alpha and state tables, the identity-like matrix and the vector. */
extern const u64 svAlphaTestTable[4];
extern const u64 svPairAlphaTable[4];
extern const Matrix4 svUnitQuadCornerMatrix;
extern const DrawStateWords svDefaultDrawStateTable;
extern const DrawStateWords svMode7DrawStateTable;
extern const DrawStateWords svMode9DrawStateTable;
extern const DrawStateWords svSpriteDrawStateTable;
extern const Vector4 svDefaultVertexScale;
extern const u64 svDrawRefUvWords[2];

/* The two buffer addresses sefEntryCountsEntryGet hands back: the first one is used when the flag is set, the second one otherwise. */
typedef struct DrawBufferPair {
    u32 first;
    u32 second;
} DrawBufferPair;

extern DrawStatePacket *svDrawStatePacket;
extern const u64 svAlphaTestTable[];
u8 *svDrawListBegin(int useFirst);
void svDrawListAppend(int vuAddress, int operand_a, int operand_b);

void sceVif1PkAddUpkData128(XglPacket *packet, Quadword word);
extern u8 D_80[];
extern u8 D_110[];
extern u32 svDrawRecordDivisors[];
void svDrawListEmit(XglPacket *packet, DrawListEntry *records, int pending_count);
void svDrawListFlush(int pending_count);
void svDrawListTryFlush(void);
void svDrawSendStaticRef(void);
extern Vector4 svDefaultScreenParams[];
extern char D_50[];
/* The static VU1 view setup packet, nine quadwords. */
extern unsigned int svViewSetupPacket[];
void svDrawSendViewSetup(void);
#define svDrawSetState ((void (*)(short, int))svDrawSetState)
void svDrawSubmitMode(void);

/* _imageMapper runs from 0x3ED680 to sefGlobalTransform (0x80A40 bytes): 56 whole type lists and 0x2420 bytes more.
 * svInitImageMapper clears and sets up 57 lists (57 * 0x241C bytes, 4 more than the object), so how the
 * last list and the tail divide is not settled. */
typedef struct SvImageMapperStorage {
    SvTypeList typeLists[56];
    unsigned char unmodeled_tail[0x2420];
} SvImageMapperStorage;

extern SvImageMapperStorage _imageMapper;
/* Small data after the bitmap name strings of svGetPrmFromName: the draw path flag and the visible entry count. */
extern short svPolygonClipFlag;
extern int svVisibleEntryCount;
int sceVif1PkRefLoadImage(void *packet, u16 basePointer, int psm, int bufferWidth, const void *data, int quadwordCount, int, int, int, int);
void svLoadTexture(SvTypeListEntry *entry);
void svLoadClut(SvTypeListEntry *entry);
int svLoadImageList(SvTypeList *typeList);

/* The TEST_1 words svSetZTestEnv stores: Z test on (GEQUAL) and always pass. */
#define SV_ZTEST_ON 0x5000D
#define SV_ZTEST_OFF 0x3000D

void sefInitClipViewVolume(void *view);

/* One key of the item's key table: kind and parameter (4 bytes). */
typedef struct SvKeyHeader {
    short kind;
    unsigned short parameter;
} SvKeyHeader;

typedef struct SvDrawItem {
    unsigned char unmodeled_00[0x160];
    short *entries;             /* +0x160: entries[-2] is the mode key read by svSortLineSlots (X3 asm lw 352) */
    unsigned char unmodeled_164[0x69C - 0x164];
    short pointHandles[0x100];  /* +0x69C: one slot handle per row, -1 when unused (X3 asm addiu 1692) */
    unsigned char unmodeled_89C[0x8A0 - 0x89C];
    unsigned short count;       /* +0x8A0: the item's count (X3 asm lhu 2208) */
    unsigned char unmodeled_8A2[0x8A4 - 0x8A2];
    short kindOverride;         /* +0x8A4: read in place of the mode key when that key is 16 or 17 (X3 asm lh 2212) */
} SvDrawItem;

void svDrawScene(void);
extern unsigned char svMapperPacketEnv[];

typedef struct SvDrawTagArea {
    unsigned char unmodeled_00[0x20];
    u64 tag;
    unsigned char unmodeled_28[0x18];
} SvDrawTagArea;

/* Scaffold .data object D_00475590 (0x40 bytes): the draw-state tag area; its tag at +0x20 starts as the ON word. */
extern SvDrawTagArea D_00475590;
void svSetZTestEnv(int enable);
extern s16 _sefBattleMode;
/* Scaffold .data object D_004755D0 (16 bytes): the upper per-lane clamp of the scaled vertex (vmini in the scaled branch). */
extern Vector4 D_004755D0;

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

typedef struct SvDrawState { Matrix4 matrix; /* +0x00 */ unsigned char unmodeled_040[0x60]; Vector4 pointA; /* +0xA0 */ unsigned char unmodeled_0B0[0x10]; Vector4 pointB; /* +0xC0 */ unsigned char unmodeled_0D0[0x60]; int vertex[4]; /* +0x130 */ unsigned char unmodeled_140[0x18]; SvDrawUv *uv; /* +0x158 */ SvDrawTexture *texture; /* +0x15C */ u64 primaryPrim; /* +0x160 */ u64 secondaryPrim; /* +0x168 */ unsigned char unmodeled_170[0x8C]; int tableIndex; /* +0x1FC: the svPairAlphaTable index (X3 asm lw 508) */ Vector4 extraState; /* +0x200 */ } SvDrawState;

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

void func_0020A038(SvDrawState *state, u64 alpha);
void func_0020A190(SvDrawState *state, u64 alpha);
extern Matrix4 svEffectMatrix;

/* The part block of a transform block: its flags at +0x1C and its kind at +0x1E. */
typedef struct SvDrawPart {
    unsigned char unmodeled_00[0x1C];
    unsigned short flags;
    short kind;
} SvDrawPart;

typedef struct SvTransformBlock {
    Matrix4 matrix;
    unsigned char unmodeled_40[0x40];
    Vector4 rotated_rows[3];
    unsigned char unmodeled_B0[0x140 - 0xB0];
    Vector4 rotation_aux;      /* +0x140 */
    unsigned char unmodeled_150[0x160 - 0x150];
    Vector4 scale;
    unsigned char unmodeled_170[0x1C0 - 0x170];
    Vector4 pivot;             /* +0x1C0: the pivot svPivot points at */
    unsigned char unmodeled_1D0[0x1DC - 0x1D0];
    void *drawData;            /* +0x1DC: NULL when the slot is unused */
    unsigned char unmodeled_1E0[0x1E8 - 0x1E0];
    SvDrawPart *part;          /* +0x1E8 */
    unsigned char unmodeled_1EC[0x28C - 0x1EC];
    int index;                 /* +0x28C */
    Vector4 point;             /* +0x290 */
    unsigned char unmodeled_2A0[0x2C0 - 0x2A0]; /* the slot stride is 0x2C0 */
} SvTransformBlock;

void svBuildItemMatrix(Vector4 *point, SvTransformBlock *transform, int useFixedMatrix, int useItemRows);

typedef struct SvRenderItem { unsigned char unmodeled_00[0xC0]; Vector4 scale; /* +0xC0 */ unsigned char unmodeled_0D0[0x130 - 0xD0]; int color[3]; /* +0x130 */ unsigned int transparency; /* +0x13C */ unsigned char unmodeled_140[0x158 - 0x140]; struct SvRenderPart *part; /* +0x158 */ unsigned char unmodeled_15C[0x1B8 - 0x15C]; float animTime; /* +0x1B8 */ unsigned char unmodeled_1BC[0x1FC - 0x1BC]; int stencilIndex; /* +0x1FC */ } SvRenderItem;

/* The part of the render item: the block pxySubmitModel takes at +0x8 and the block nmlModelSetTexture takes at +0xC. */
typedef struct SvRenderPart {
    unsigned char unmodeled_00[0x8];
    PxyHeader *submitBlock;
    TxyBlock *txyBlock;
} SvRenderPart;

/* The view block svSetModelRenderState writes: the scaled vertex position at +0xD0 and its copy at +0xE0. */
typedef struct SvViewBlock {
    unsigned char unmodeled_00[0xD0];
    AlignedVector4 scaled;
    AlignedVector4 scaledCopy;
} SvViewBlock;

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

/* Clips the draw state quad, transformed by matrix, against the first five planes of the active camera's view * volume (Sutherland-Hodgman, two vertex buffers). Each surviving vertex goes to out as a pair: the position * with w = 1, then svUnitQuadCornerMatrix with x and y replaced by the position's projections on the quad's two edges. * Returns the vertex count, 0 when the quad is degenerate or clipped away. */ int sefClipPolygon(void *slot, Matrix4 *matrix, SvClipVertex *out);

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

/* The polygon item svDrawPolygonItem draws: its matrix, the scale vector at +0x160, the fixed-point colour at +0x1D0,
 * the base and overlay parts with their texture offsets, the alpha table index at +0x28C and the point at +0x290. */
typedef struct SvPolygonItem {
    Matrix4 matrix;
    unsigned char unmodeled_40[0x160 - 0x40];
    Vector4 scale;
    unsigned char unmodeled_170[0x1D0 - 0x170];
    int color[4];
    unsigned char unmodeled_1E0[0x1E8 - 0x1E0];
    SvPolygonPart *part;
    SvPolygonPart *overlayPart;
    float texOffsetX;
    float texOffsetY;
    float overlayOffsetX;
    float overlayOffsetY;
    unsigned char unmodeled_200[0x28C - 0x200];
    int drawIndex;
    Vector4 point;
} SvPolygonItem;

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

extern const u64 svPairAlphaTable[];
void svDrawPolygonItem(SvPolygonItem *item, Vector4 *position, SvDrawCall *quadRef, int flag);

/* Part record of a slot: the two floats the draw packet reads, the 64-bit word at +0x10, the flags halfword (bit 0x10 hides the slot unless the flag allows it) and the kind halfword (4 is drawn here). */
typedef struct SefSlotPart {
    float coordinateX;
    float coordinateY;
    unsigned char unmodeled_08[0x10 - 0x08];
    unsigned long long word10;
    unsigned char unmodeled_18[0x1C - 0x18];
    unsigned short flags;
    short kind;
} SefSlotPart;

/* One 624-byte slot (X3 asm: the slot index is multiplied by 624): the matrix rows at +0x00, the position vector at +0xA0, a draw block pointer at +0x13C (null skips the slot) and the part at +0x158. */
typedef struct SefSlotRecord {
    Matrix4 matrix;
    unsigned char unmodeled_40[0xA0 - 0x40];
    Vector4 position;
    unsigned char unmodeled_B0[0x13C - 0xB0];
    void *drawData;
    unsigned char unmodeled_140[0x158 - 0x140];
    SefSlotPart *part;
    unsigned char unmodeled_15C[0x270 - 0x15C];
} SefSlotRecord;

/* A mode word holds either a float coordinate or a zero written as an integer word. */
typedef union SvModeWord {
    float f;
    unsigned int u;
} SvModeWord;

/* One slot draw record appended to svDrawList: 14 quadwords of VIF tags, the slot blocks, the mode words and the matrix rows. */
typedef struct SvSlotDrawPacket {
    DoublewordVector4 gif;          /* +0x00: GIF tag */
    DoublewordVector4 tex;          /* +0x10: the part word and register 6 */
    DoublewordVector4 test;         /* +0x20: svDrawTestWord and register 0x47 */
    DoublewordVector4 alpha;        /* +0x30: the svAlphaTestTable entry and register 0x42 */
    Vector4 position;
    /* DoublewordVector4 here makes the quadword copy ICE (emit_move_insn): kept as two doublewords. */
    unsigned long long partBlock[2];
    Vector4 scaled;
    Vector4 slotBlock;
    SvModeWord modeWord0;
    SvModeWord modeWord1;
    SvModeWord modeWord2;
    SvModeWord modeWord3;
    Vector4 state;
    Matrix4 matrix;
} SvSlotDrawPacket;

void svDrawListAppendSlot(SefSlotRecord *slot, const Vector4 *position, const SvDrawCall *args, int useScale);

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

void svDrawListAppendRecordBPair(SvDrawState *state, Vector4 *point, SvDrawCall *quadRef, int useScale);

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

void svDrawListAppendRecordA(SvDrawState *state, Vector4 *point, SvDrawCall *quadRef, int useFixedMatrix);
void svDrawListAppendRecordAPair(SvDrawState *state, Vector4 *point, SvDrawCall *quadRef, int useFixedMatrix);

/* The part svDrawListAppendPart packs: the transform, then the position, the register source and the extents. */
typedef struct SvPartSource {
    Vector4 header;                 /* +0x00: copied into the packet */
    u64 registerData;               /* +0x10 */
    unsigned char unmodeled_18[0x1C - 0x18];
    unsigned short flags;           /* +0x1C: bit 0x10 selects the copy path */
} SvPartSource;

typedef struct SvPart {
    SefPartTransform transform;     /* +0x000 */
    unsigned char unmodeled_058[0x1D0 - 0x58];
    int position[4];                /* +0x1D0: converted and clamped to the scene bounds */
    unsigned char unmodeled_1E0[0x1E8 - 0x1E0];
    SvPartSource *source;           /* +0x1E8 */
    unsigned char unmodeled_1EC[0x1F0 - 0x1EC];
    Vector4 extentA;                /* +0x1F0 */
    unsigned char unmodeled_200[0x28C - 0x200];
    int index;                      /* +0x28C: selects the svAlphaTestTable entry */
    Vector4 extentB;                /* +0x290 */
} SvPart;

/* One quadword as two doublewords. */
typedef struct DrawQuad {
    u64 low;
    u64 high;
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

static inline void svCopyQuadword(Vector4 *destination, const Vector4 *source)
{
    QuadwordCopyT0(destination, source);
}

void svDrawListAppendPart(SvPart *part, const Vector4 *vertex, SvDrawCall *ref, int scaleRows);
void svDrawListAppendStatePair(SvDrawState *state, Vector4 *point, SvDrawCall *quadRef, int scaleRows);

/* The current effect record (sefCurrentRecord): the scale vector at +0x20, three ints at +0x50, +0x54 and +0x58 and the selector at +0x74. */
typedef struct SefCurrentRecord {
    unsigned char unmodeled_00[0x20];
    Vector4 scale;
    unsigned char unmodeled_30[0x50 - 0x30];
    int scaleKey40;            /* +0x50: the scale key for kind 0x40 */
    int scaleKey100;           /* +0x54: the scale key for kind 0x100 */
    int scaleKey80;            /* +0x58: the scale key for kind 0x80 */
    unsigned char unmodeled_5C[0x74 - 0x5C];
    int drawSelector;          /* +0x74 */
} SefCurrentRecord;

extern SefCurrentRecord *sefCurrentRecord;
extern int svSceneFlags;
extern Vector4 svDefaultScaleVectors[3];
extern unsigned char sefAppliedLightSet[];
extern Matrix4 svEffectCameraProjectionMatrix;
void svDrawImageItem(SvDrawItem *item, int renderState);

/* sefCurrentRecord seen as a one-member record slot. */
typedef struct SefCurrentRecordSlot {
    SefEffectRecord *record;
} SefCurrentRecordSlot;

void svDrawSortedImages(SefEffectRecord *effectRecord);

/* Scratchpad base address: svCollectSortedLines fills the SvEntryPair array here, and the cull code reuses it. */
#define SV_SCRATCHPAD 0x70000000

/* One output pair in the scratchpad: the line index and its sort key, sorted ascending by sortKey. */
typedef struct SvEntryPair {
    short lineIndex;
    short sortKey;
    unsigned char unmodeled_04[2];
} SvEntryPair;

/* Context block passed by svDrawEntry: the current matrix sits at +0x470. */
typedef struct SvDrawContext {
    unsigned char unmodeled_00[0x470];
    Matrix4 matrix;
} SvDrawContext;

int svSortLineSlots(SefEffectRecord *record, SvEntryPair *pairs, int pairCount, SvDrawContext *context);
int svCollectSortedLines(SefEffectRecord *record, SvEntryPair *out);
extern Matrix4 svEffectScaledMatrix;
void svDrawSortedImages(SefEffectRecord *record);
void svDrawEntry(int recordIndex);
extern SvDrawSlot svVisibleSlots[];
void svDrawEntriesByKind(int kind, int flags);

/* One 20-byte entry of the table sefEntryCountsEntryGet indexes: the first two words are the buffer pair, +8 is the count of draw entries of that kind. */
typedef struct DrawKindEntry {
    DrawBufferPair buffers;
    int count;
    unsigned char unmodeled_0c[8];
} DrawKindEntry;

/* Read-only scaffold items after the C-owned literals: the stencil table svStencilTable and the matrix svEffectProjectionMatrix. */
extern const u64 svStencilTable[3];
/* .data of this TU: definitions in address order (data recovery pass 2). */
extern const float svSortOffsetScale;
extern u32 svDrawRecordDivisors[31];
extern unsigned int svViewSetupPacket[36];
extern unsigned char svMapperPacketEnv[48];
extern SvDrawSlot svVisibleSlots[260];

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", eBattleWinInit);

/* Refers the current packet to the three-quadword environment block svMapperPacketEnv; runs after every mapper switch. */
void svRefMapperPacket(void) {
    sceVif1PkRef(xglPacketGetCurrent(), svMapperPacketEnv, 3, 0, 0, 0);
}

/* Selects the Z test: on (GEQUAL, always pass) or off; the flag and the draw-state word follow. */
void svSetZTestEnv(int enable) {
    if (enable != 0) {
        D_00475590.tag = SV_ZTEST_ON;
    } else {
        D_00475590.tag = SV_ZTEST_OFF;
    }
    D_00551518 = enable;
    D_0076D4D0[0] = D_00475590.tag;
}

void sefInitClipViewVolume(void *view) {
    Vu0SetClipMatrices(view);
}

int sefClipViewVolumeA(const Vector4 *model, float limit) {
    __asm__ __volatile__(
        ".set noreorder\n\t"
        "lui $2, %%hi(D_00452D50)\n\t"
        "daddu $5, $4, $0\n\t"
        "ld $3, %%lo(D_00452D50)($2)\n\t"
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

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", func_0020A038);

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", func_0020A190);

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", func_0020A320);

/* Splat labels of the scaffold objects svSetModelRenderState reads (the X3 .data, .sbss, .sdata and .rodata objects). */
#define D_0053F258 ((const u64 *)D_0053F258)
extern void *D_005510C0;
extern int D_0055154C;
extern const f32 D_0054FD88;
/* Callees of the other units, prototyped from their call sites. */
void pxyAddFlags(int level);
void func_0019EF08(const Vector4 *color);
void func_0019F008(float d0, float d1, float d2, float d3);
void func_0019F1B0(float offset);
void func_0019F210(TxyBlock *txy);
void func_0019F308(const Matrix4 *matrix);
void func_0019F338(const Vector4 *scale);
void func_0019F3A0(int zwrite);
void func_0019F3C0(float transparency);
void func_0019F458(int stencil);
void func_0019F488(void *lightSet);
void func_0019F508(u64 stencil);
void pxySetAnimTime(float animTime);
void func_001A1D18(PxyHeader *submit);
SefFogOverride *func_00207948(void);

/* Sets the render state of one model item and submits it: the stencil word from D_0053F258, the scaled colour into the
 * view block, the transparency, the flag bits, the fog override, the sort offset, the scale and the submit. */
void svSetModelRenderState(SvRenderItem *item, Matrix4 *matrix, int scale)
{
    u64 stencil;
    const Vector4 *vertexScale;
    AlignedVector4 vertex;
    AlignedVector4 *vertexOut;
    int *color;
    SvViewBlock *view;
    SefFogOverride *fog;
    int itemFlags;
    float transparency;

    if (item->stencilIndex == 0 && item->transparency == 0x80) {
        stencil = 0;
    } else {
        stencil = D_0053F258[item->stencilIndex];
    }
    vertexOut = &vertex;
    color = item->color;
    vertexScale = D_00551520;
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(color) : "memory");
    if (vertexScale != 0) {
        __asm__ __volatile__(
            "lqc2 vf2, 0(%0)\n\t"
            "lqc2 vf3, 0(%1)\n\t"
            "vitof0.xyzw vf1, vf1\n\t"
            "vmul.xyzw vf1, vf1, vf2\n\t"
            "vmini.xyz vf1, vf1, vf3\n\t"
            "vftoi0.xyzw vf1, vf1"
            : : "r"(vertexScale), "r"(&D_004755D0) : "memory");
    }
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(vertexOut) : "memory");
    view = D_005510C0;
    __asm__ __volatile__("li.s $f0, 0.0078125" : : : "$f0");
    __asm__ __volatile__(
        "mfc1 $8, $f0\n\t"
        "qmtc2 $8, vf1\n\t"
        "lqc2 vf2, 0(%0)\n\t"
        "vitof0.xyzw vf2, vf2\n\t"
        "vmulx.xyz vf2, vf2, vf1x\n\t"
        "sqc2 vf2, 0(%1)"
        : : "r"(&vertex), "r"(&view->scaled) : "$8", "$f0");
    QuadwordCopyT0(&view->scaledCopy, &view->scaled);

    if (stencil != 0) {
        func_0019F458((int) stencil);
        func_0019F508(stencil);
    }
    if (item->part->txyBlock != 0 && item->part->submitBlock != 0) {
        itemFlags = D_0055154C;
        transparency = (float) item->transparency * 0.0078125f;
        func_0019F488(D_005510C0);
        pxySetAnimTime(item->animTime);
        func_0019F210(item->part->txyBlock);
        func_0019F308(matrix);
        if (D_00551520 != 0) {
            transparency *= D_00551520->w;
        }
        func_0019F3C0(transparency);
        if ((itemFlags & 1) != 0) {
            pxyAddFlags(4);
        }
        if ((itemFlags & 8) != 0) {
            func_0019F3A0(1);
        } else {
            func_0019F3A0(0);
        }
        if ((itemFlags & 2) != 0) {
            pxyAddFlags(8);
        }
        if ((itemFlags & 0x80) != 0) {
            pxyAddFlags(0x200);
        }
        if ((itemFlags & 0x40) != 0) {
            pxyAddFlags(0x100);
        } else {
            fog = func_00207948();
            if (fog->enabled != 0) {
                func_0019EF08(&fog->fogColor);
                func_0019F008(fog->dist[0], fog->dist[1], fog->dist[2], fog->dist[3]);
            }
        }
        if (scale > 0) {
            func_0019F1B0((float) scale * D_0054FD88);
        }
        func_0019F338(&item->scale);
        func_001A1D18(item->part->submitBlock);
    }
}

/* Projects a homogeneous vertex to the screen: divides by w, flags 0x8000 when the result lies outside the unit cube, then applies the active camera's screen scale and offset. Returns the flags. */
int svProjectVertex(SvScreenVertex *out, Matrix4 *matrix, const Vector4 *vector) {
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
    out->clipFlags = clipFlags;
    camera = xglStudioGetActiveCamera();
    __asm__ __volatile__("lqc2 vf1, 0(%0)\n\tlqc2 vf2, 0(%1)" : : "r"(&clip), "r"(&camera->screen.screenScale), "r"(&camera->screen.screenOffset) : "memory");
    __asm__ __volatile__("lqc2 vf3, 0(%0)" : : "r"(&camera->screen.screenOffset) : "memory");
    __asm__ __volatile__("vmula.xyz ACC, vf1, vf2" : : : "memory");
    __asm__ __volatile__("vmaddw.xyz vf1, vf3, vf0w" : : : "memory");
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(&clip) : "memory");
    out->z = (int) clip.z;
    out->x = (int) (clip.x * SV_SUBPIXEL_STEPS);
    out->y = (int) (clip.y * SV_SUBPIXEL_STEPS);
    return out->clipFlags;
}

/* Signed distance of a point (its w) from a clip plane whose offset comes from MMathVectorDotProduct. */
f32 sefClipPlaneDistance(Vector4 *plane, Vector4 *point) {
    return -(MMathVectorDotProduct(plane, point) + plane->w);
}

/* Intersects the segment p1-p2 with a plane (VU0 dot products, one divide); writes the crossing point. */
void sefClipEdgeToPlane(Vector4 *out, const Vector4 *plane, const Vector4 *p1, const Vector4 *p2) {
    __asm__ __volatile__("vmove.xyzw vf4, vf0" : : : "memory");
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(p1) : "memory");
    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(p2) : "memory");
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
    __asm__ __volatile__("sqc2 vf4, 0(%0)" : : "r"(out) : "memory");
}

/* vf20 = the matrix loaded in vf26..vf29 applied to the point at source. */
static inline void sefTransformToVf20(Vector4 *source)
{
    __asm__ __volatile__("lqc2 vf1, 0(%0)\n"
                         "vmulax.xyzw ACC, vf26, vf1x\n"
                         "vmadday.xyzw ACC, vf27, vf1y\n"
                         "vmaddaz.xyzw ACC, vf28, vf1z\n"
                         "vmaddw.xyzw vf20, vf29, vf1w\n"
                         : : "r"(source) : "memory");
}

/* Stores vf20 to destination. */
static inline void sefStoreVf20(Vector4 *destination)
{
    __asm__ __volatile__("sqc2 vf20, 0(%1)\n" : "=m"(destination->w) : "r"(destination) : "memory");
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
int sefClipPolygon(void *slot, Matrix4 *matrix, SvClipVertex *out)
{
    int vertexCounts[2];
    float edgeLengths[2];
    Vector4 corners[4];
    Vector4 edges[2];
    SefClipVertex clipBuffers[2][9];
    Vector4 offset;
    int planeIndex;
    XglCamera *camera;
    DrawStatePacket *drawPacket;
    Vector4 *planeVector;
    Vector4 *origin;
    Vector4 *dst;
    int currentBuffer;
    int nextBuffer;
    int packetCorner;
    int clipCount;
    int i;
    int nextVertex;

    camera = xglStudioGetActiveCamera();
    currentBuffer = 0;
    vertexCounts[1] = 0;
    vertexCounts[0] = 0;
    __asm__ __volatile__("lqc2 vf26, 0(%0)\n"
                         "lqc2 vf27, 16(%0)\n"
                         "lqc2 vf28, 32(%0)\n"
                         "lqc2 vf29, 48(%0)\n"
                         : : "r"(matrix) : "memory");
    drawPacket = svDrawStatePacket;
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
        sefStoreVf20(&clipBuffers[currentBuffer][i].vector);
    }
    vertexCounts[currentBuffer] = 4;
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
        nextBuffer = currentBuffer ^ 1;
        for (i = 0; i < vertexCounts[currentBuffer]; i++) {
            clipBuffers[currentBuffer][i].element[3] = sefClipPlaneDistance(planeVector, &clipBuffers[currentBuffer][i].vector);
        }
        clipCount = 0;
        for (i = 0; i < vertexCounts[currentBuffer]; i++) {
            nextVertex = (i + 1) % vertexCounts[currentBuffer];
            if (clipBuffers[currentBuffer][i].element[3] >= 0.0f) {
                clipBuffers[nextBuffer][clipCount++] = clipBuffers[currentBuffer][i];
                if (clipBuffers[currentBuffer][nextVertex].element[3] < 0.0f) {
                    sefClipEdgeToPlane(&clipBuffers[nextBuffer][clipCount++].vector, planeVector, &clipBuffers[currentBuffer][i].vector, &clipBuffers[currentBuffer][nextVertex].vector);
                }
            } else if (clipBuffers[currentBuffer][nextVertex].element[3] >= 0.0f) {
                sefClipEdgeToPlane(&clipBuffers[nextBuffer][clipCount++].vector, planeVector, &clipBuffers[currentBuffer][i].vector, &clipBuffers[currentBuffer][nextVertex].vector);
            }
        }
        vertexCounts[nextBuffer] = clipCount;
        currentBuffer = nextBuffer;
        if (clipCount < 3) {
            return 0;
        }
    }
    for (i = 0; i < vertexCounts[currentBuffer]; i++) {
        dst = &out[i].position;
        __asm__ __volatile__("lq $8, %5\n"
                             "sq $8, 0(%4)\n"
                             : "=m"(dst[0].x), "=m"(dst[0].y), "=m"(dst[0].z), "=m"(dst[0].w)
                             : "r"(&dst[0]), "m"(*(&clipBuffers[currentBuffer][i])) : "$8", "memory");
        dst[0].w = 1.0f;
        SefCopyVector(&dst[1], &svUnitQuadCornerMatrix);
        SefSubVectorXYZ(&offset, &dst[0], origin);
        dst[1].x = MMathVectorDotProduct(&offset, &edges[0]) / edgeLengths[0];
        dst[1].y = MMathVectorDotProduct(&offset, &edges[1]) / edgeLengths[1];
    }
    return vertexCounts[currentBuffer];
}

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", func_0020AD78);

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", func_0020B238);

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", func_0020B470);

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", func_0020B6E8);

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", func_0020BA10);

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", func_0020BD70);

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", func_0020BF50);

int sefInitPartTransform(SefPartTransform *part, int kind, int scaleKey100, int scaleKey200) {
    float partScale = 1.0f;
    const Matrix4 *unitMatrix;

    if ((kind & 0x100) != 0 || kind == 59 || kind == 62) {
        partScale = func_00203360(scaleKey100);
    } else if ((kind & 0x200) != 0 || kind == 61 || kind == 63 || kind == 64 || kind == 65) {
        partScale = func_00203360(scaleKey200);
    }
    part->scaled = 0;
    if (partScale != 1.0f) {
        part->scale = partScale;
        part->scaled = 1;
        part->kind = kind;
        sefGetPosition(&part->position, 0, kind);
        unitMatrix = &MMathUnitMatrix;
        __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(&part->position) : "memory");
        __asm__ __volatile__("vmove.w vf1, vf0" : : : "memory");
        __asm__ __volatile__("lqc2 vf2, 32(%0)" : : "r"(unitMatrix) : "memory");
        __asm__ __volatile__("lqc2 vf3, 16(%0)" : : "r"(unitMatrix) : "memory");
        __asm__ __volatile__("lqc2 vf4, 0(%0)" : : "r"(unitMatrix) : "memory");
        __asm__ __volatile__("sqc2 vf1, 48(%0)\n\tsqc2 vf2, 32(%0)\n\tsqc2 vf3, 16(%0)\n\tsqc2 vf4, 0(%0)" : : "r"(part) : "memory");
        part->matrix[0][0] = partScale;
        part->matrix[1][1] = partScale;
        part->matrix[2][2] = partScale;
    }
    return kind;
}

int svInitItemDrawKind(SefPartTransform *part, SvDrawItem *item, int scaleKey100, int scaleKey200) {
    int kind = item->entries[-2];

    if ((unsigned int)(kind - 16) < 2) {
        kind = item->kindOverride;
    }
    return sefInitPartTransform(part, kind, scaleKey100, scaleKey200);
}

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", func_0020C298);

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", func_0020CF20);

unsigned char *sevGetPtAllocator2(int slotIndex);
void svSortReset(void);
void func_002087F0(int id, int flags, float depth);

/* Adds each drawn slot of the pairs' lines to the depth sort, with the view-space z of its position under the current matrix as depth; returns the summed line count. */
int func_0020D778(SefEffectRecord *record, SvEntryPair *pairs, int pairCount, SvDrawContext *context) {
    unsigned char *slotBase;
    SvDrawItem *lineData;
    SefSlotRecord *slotRecord;
    SefSlotPart *slotPart;
    short *slotIndexPtr;
    int showFlagged;
    int lineTotal;
    int slotIndex;
    int lineIndex;
    int pairsLeft;
    SvEntryPair *pair;
    int slotsLeft;
    int wantedPartKind;
    Vector4 depthVector;

    slotBase = sevGetPtAllocator2(0);
    lineTotal = 0;
    showFlagged = svSceneFlags & 2;
    svSortReset();
    __asm__ __volatile__(
        "lqc2 vf26, 0(%0)\n\t"
        "lqc2 vf27, 16(%0)\n\t"
        "lqc2 vf28, 32(%0)\n\t"
        "lqc2 vf29, 48(%0)"
        : : "r"(context->matrix));
    if (pairCount > 0) {
        pair = pairs;
        for (pairsLeft = pairCount; pairsLeft != 0; pairsLeft--, pair++) {
            lineIndex = pair->lineIndex;
            lineData = (SvDrawItem *) sefGetLineAdr(lineIndex);
            if (lineData->count == 0 || lineData->entries[-2] == 8) {
                continue;
            }
            lineTotal += lineData->count;
            wantedPartKind = 4;
            slotIndexPtr = lineData->pointHandles;
            for (slotsLeft = 255; slotsLeft >= 0; slotsLeft--, slotIndexPtr++) {
                slotIndex = *slotIndexPtr;
                if (slotIndex < 0) {
                    continue;
                }
                slotRecord = (SefSlotRecord *)(slotBase + slotIndex * sizeof(SefSlotRecord));
                slotPart = slotRecord->part;
                if ((slotPart->flags & 0x10) != 0 && showFlagged == 0) {
                    continue;
                }
                if (slotPart->kind != wantedPartKind) {
                    continue;
                }
                if (slotRecord->drawData == 0) {
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
                    : "=f"(depthVector.x) : "r"(&slotRecord->position) : "$8");
                func_002087F0(slotIndex, lineIndex, depthVector.x);
            }
        }
    }
    return lineTotal;
}

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", func_0020D910);

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

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", func_0020DBD8);

/* Transforms a point by the clip matrix (vf21-vf24, set by sefInitClipViewVolume); a null input means the origin. */
void sefTransformClipPoint(Vector4 *out, const Vector4 *in) {
    if (in != 0) {
        __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(in) : "memory");
    } else {
        __asm__ __volatile__("vmove.xyzw vf1, vf0" : : : "memory");
    }
    __asm__ __volatile__("vmulax.xyzw ACC, vf21, vf1x" : : : "memory");
    __asm__ __volatile__("vmadday.xyzw ACC, vf22, vf1y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyzw ACC, vf23, vf1z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyzw vf1, vf24, vf1w" : : : "memory");
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(out) : "memory");
}

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", func_0020DF68);

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", func_0020E1F0);

/* Draws the alter layer only in 2D mode (_draw3D == 0). */
void svDrawAlters(int layer)
{
    if (_draw3D == SV_RENDER_2D) {
        sdvDrawAlters(layer);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", svDrawScene);

/* Sets the 3D flag and draws the scene when svDrawListBegin(0) reports it. */
void svDrawScene3D(void) {
    if (svDrawListBegin(0) != 0) {
        _draw3D = 1;
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

INCLUDE_ASM("asm/main/nonmatchings/sv_get_image_item", func_0020E6F8);

int sefEntryCountsEntryGet(int arg);
void func_0025B558(Matrix4 *out, float *src);
void func_0020DF68(void);
void func_0020E1F0(int index, int flag);
extern Matrix4 D_00770ED0[2];
extern Matrix4 D_0053F460;
extern Vector4 D_0076D7E0[24];

void svDrawEffectRecords3D(void) {
    Matrix4 volumeMatrix;
    XglCamera *camera;
    int *entry;
    int index;
    int table;

    table = sefEntryCountsEntryGet(0);
    _draw3D = 2;
    camera = xglStudioGetActiveCamera();
    QuadwordCopyMatrixT0(D_00770ED0, (char *) camera + 0x170);
    func_0025B558(&volumeMatrix, camera->volume[0].viewMatrix[0]);
    MMathMulMatrix(&D_00770ED0[1], &volumeMatrix, &D_0053F460);
    memset(D_0076D7E0, 0, 0x180);
    QuadwordCopyT0(&D_0076D7E0[6], &vu0IdentityMatrix[0]);
    func_0020DF68();
    entry = (int *) (table + 0xC);
    for (index = 0; index < 44; index++, entry += 6) {
        if (entry[0] > 0) {
            func_0020E1F0(index, 0);
        }
    }
}

/* <<< CHUNK sv_get_image_item */

#include "common.h"
#include "main/xgl_dma.h"
#include "shared.h"

#include "main/Packet.h"
#include "main/xgl_render.h"


XglPacket *xglPacketGetCurrent(void);
extern XglPacket *pCurrentPacket;

/* One texture of a transfer set: its size and VRAM place, and where its image data sits. */
typedef struct XglTextureTransEntry {
    u32 size;       /* low 16 bits: width in pixels; high 16 bits: width in 64-pixel blocks when it does not fit */
    u32 height;
    u32 vramOffset; /* added to the texture buffer base << 11, then >> 6 for the GS block address */
    u32 dataSize;   /* +0x0C: quadword count for sceVif1PkRef */
    u32 dataOffset; /* +0x10: from the set base */
} XglTextureTransEntry;

/* Texture transfer set (a script/package block): the entry table sits entryOffset bytes from the set base. */
typedef struct XglTextureTransSet {
    u8 unmodeled_00[8];
    int count;       /* +0x08 */
    u32 entryOffset; /* +0x0C */
} XglTextureTransSet;

/* The GIF setup the transfer sends before each image (28 words): BITBLTBUF and TRXREG are filled in. */
#define TEXTURE_TRANS_SETUP_WORDS 28
#define TEXTURE_TRANS_BITBLT_AD 6  /* +0x30: BITBLTBUF data */
#define TEXTURE_TRANS_REGION_AD 10 /* +0x50: TRXREG data */
extern u64 D_004C7DC0[TEXTURE_TRANS_SETUP_WORDS / 2];

/* The render state; the texture transfer reads its texture buffer base page. */
extern XglRenderStateBlock D_004C8390;

void sceVif1PkCnt(XglPacket *packet, u32 flag);
void sceVif1PkAddDataN(XglPacket *packet, void *data, u32 count);
void sceVif1PkRef(XglPacket *packet, void *data, u32 count, u32 opt1, u32 opt2, u32 flag);

/* XS2 xglPacketTextureTrans: queues one BITBLTBUF/TRXREG setup and the image data of every texture of set. */
void func_001997D0(XglTextureTransSet *set)
{
    XglTextureTransEntry *entry;
    int i;
    int blockWidth;
    int width;

    entry = (XglTextureTransEntry *) ((u8 *) set + set->entryOffset);
    for (i = 0; i < set->count; i++) {
        width = entry->size & 0xFFFF;
        if (entry->size == width) {
            blockWidth = (width + 63) / 64;
        } else {
            blockWidth = entry->size >> 16;
        }
        D_004C7DC0[TEXTURE_TRANS_BITBLT_AD] = ((u64) (((D_004C8390.texture_base << 11) + entry->vramOffset) >> 6) << 32)
            | ((u64) blockWidth << 48);
        D_004C7DC0[TEXTURE_TRANS_REGION_AD] = width | ((u64) entry->height << 32);
        sceVif1PkCnt(pCurrentPacket, 0);
        sceVif1PkAddDataN(pCurrentPacket, D_004C7DC0, TEXTURE_TRANS_SETUP_WORDS);
        sceVif1PkRef(pCurrentPacket, (u8 *) set + entry->dataOffset, entry->dataSize, 0, 0, 0);
        entry++;
    }
}

XglPacket *xglPacketGetCurrent(void)
{
    return pCurrentPacket;
}

void func_002F4878(XglPacket *packet, u32 packetBase);
extern XglPacket D_008BC260[2];
extern XglPacket *pSendPacket;

void xglPacketInitDoubleBuffer(u32 packetBase, u32 packetHalfSize, float reservePercent)
{
    int reserveSize;
    XglPacket **currentPacket = &pCurrentPacket;
    XglPacket **sendPacket = &pSendPacket;

    if (reservePercent < 1.0f) {
        reservePercent = 1.0f;
    }
    if (reservePercent > 99.0f) {
        reservePercent = 99.0f;
    }
    reserveSize = packetHalfSize * reservePercent / 100.0f;
    if (reserveSize & 0xF) {
        reserveSize = reserveSize - (reserveSize & 0xF) + 0x10;
    }
    func_002F4878(&D_008BC260[0], packetBase);
    func_002F4878(&D_008BC260[1], packetBase + packetHalfSize);
    D_008BC260[0].bufferSize = packetHalfSize;
    D_008BC260[1].bufferSize = packetHalfSize;
    D_008BC260[0].buffer = packetBase;
    D_008BC260[1].buffer = packetBase + packetHalfSize;
    *sendPacket = 0;
    D_008BC260[0].limit = packetBase + packetHalfSize - reserveSize;
    D_008BC260[0].cursor = (u8 *) D_008BC260[0].limit;
    D_008BC260[1].limit = packetBase + packetHalfSize + packetHalfSize - reserveSize;
    D_008BC260[1].cursor = (u8 *) D_008BC260[1].limit;
    *currentPacket = &D_008BC260[0];
}

typedef int s32;
extern s32 xglPacketSendMode;

/* The sceVif1Pk calls of XS2's xglPacketMove. */
void sceVif1PkEnd(XglPacket *packet, u32 flag);
void sceVif1PkTerminate(XglPacket *packet);
u32 sceVif1PkSize(XglPacket *packet);
void sceVif1PkReset(XglPacket *packet);
void FlushCache(int mode);
void func_001936A8(const char *condition, const char *file, int line, const char *format, ...);

/* The assertion of xglPacketMove: "fPacket<0.99f", "Packet.euc.c" and its EUC-JP message (the packet overflowed). */
extern const char packetFullCondition[];
extern const char packetSourceFile[];
extern const char packetFullMessage[];

#define PACKET_FULL_RATIO 0.99f
#define PACKET_MOVE_ASSERT_LINE 132

/* XS2 xglPacketMove: closes the current packet, checks (while sending is on) that it used less than 99 %
 * of its half, makes it the packet to send and resets the other half for the next frame. */
void func_00199A20(void)
{
    float fPacket;

    sceVif1PkEnd(pCurrentPacket, 0);
    sceVif1PkTerminate(pCurrentPacket);
    FlushCache(0);
    if (xglPacketSendMode != 0) {
        fPacket = (float) (sceVif1PkSize(pCurrentPacket) << 4)
                  / (float) (pCurrentPacket->limit - pCurrentPacket->buffer);
        if (!(fPacket < PACKET_FULL_RATIO)) {
            func_001936A8(packetFullCondition, packetSourceFile, PACKET_MOVE_ASSERT_LINE, packetFullMessage);
        }
    }
    pSendPacket = pCurrentPacket;
    pCurrentPacket->cursor = (u8 *) pCurrentPacket->limit;
    if (pCurrentPacket == &D_008BC260[1]) {
        pCurrentPacket = &D_008BC260[0];
    } else {
        pCurrentPacket = &D_008BC260[1];
    }
    sceVif1PkReset(pCurrentPacket);
}


void xglPacketSend(void) {
    if (pSendPacket != 0) {
        xglDmaDirectSrcChain(1, pSendPacket->start);
    }
}


void xglPacketSetSendMode(s32 sendMode) {
    xglPacketSendMode = sendMode;
}

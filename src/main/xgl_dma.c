#include "common.h"
#include "shared.h"

/* DMA tag ID (bits 28..30 of the first tag word). */
#define DMA_TAG_ID_CNT 0x10000000U

#define DMA_TAG_ID_REF 0x30000000U

void xglDmaDirectNormal(unsigned int channel, unsigned int memoryAddress, unsigned int quadwordCount);
void xglDmaDirectSrcChain(unsigned int channel, unsigned int tagAddress);

extern u8 *buffer;

/* Layout of the DMAC_CTRL register. */
typedef struct XglDmacControl {
    u32 dma_enable : 1;
    u32 cycle_steal_release : 1;
    u32 drain_mode : 2;
    u32 stall_source : 2;
    u32 stall_drain : 2;
    u32 release_cycle : 3;
    u32 reserved : 21;
} XglDmacControl;

/* EE DMAC global registers and the SPR-from channel registers.  DMAC_CTRL
 * is not volatile: its drain_mode updates are whole-word read-modify-writes. */
#define DMAC_CTRL      ((XglDmacControl *)0x1000E000)

#define DMAC_STAT      ((volatile u32 *)0x1000E010)
#define DMAC_RBSR      ((volatile u32 *)0x1000E040)
#define DMAC_RBOR      ((volatile u32 *)0x1000E050)
#define D8_CHCR        ((volatile u32 *)0x1000D000)
#define D8_MADR        ((volatile u32 *)0x1000D010)
#define D8_QWC         ((volatile u32 *)0x1000D020)
#define D8_SADR        ((volatile u32 *)0x1000D080)
#define DMA_TAG_ID_CALL 0x50000000U
#define DMA_TAG_ID_END 0x70000000U

/* Dn_CHCR bits. */
#define DMA_CHCR_DIR 0x1

#define DMA_CHCR_MOD_CHAIN 0x4
#define DMA_CHCR_TTE 0x40
#define DMA_CHCR_STR 0x100

/* xglDmaDirect* drive only the first three channels: VIF0, VIF1 and GIF. */
#define XGL_DMA_DIRECT_CHANNELS 3

/* DMAC_CTRL.MFD: which channel drains the MFIFO ring. */
#define DMAC_CTRL_MFD_NONE 0

#define DMAC_CTRL_MFD_GIF 3

/* MFIFO ring sizes and offsets are in quadwords of 16 bytes. */
#define MFIFO_QUADWORD 16

/* The DMAC takes scratchpad addresses (0x7000xxxx) as an offset with the SPR flag in bit 31;
 * the same names as main/xgl_dma.h. */
#define DMA_SPR_ADDRESS_HIGH 0x7000U

#define DMA_SPR_OFFSET_MASK 0x3fffU
#define DMA_SPR_FLAG 0x80000000U

/* D8_SADR is quadword aligned within the 16 KiB scratchpad. */
#define DMA_SADR_MASK 0x3ff0U

/*
 * The DMA channel table and the MFIFO state are provided by the scaffold data
 * for this TU.  Only the register slots touched by these routines are named.
 */
typedef struct XglDmaChannel {
    /* EE DMAC MMIO registers: CHCR +0x00, MADR +0x10, QWC +0x20, TADR +0x30.
     * The hardware updates CHCR while the polling loops wait for completion. */
    volatile u32 chcr;
    u32 reserved_04[3];
    volatile u32 memory_address;
    u32 reserved_14[3];
    volatile u32 quadword_count;
    u32 reserved_24[3];
    volatile u32 tag_address;
} XglDmaChannel;

typedef struct XglDmaBuffer {
    u32 *chainStart;
    u32 *writeCursor;
} XglDmaBuffer;

/* The drain channel of the memory FIFO. */
struct MfifoDrainStorage {
    XglDmaChannel *channel;
};

extern void FlushCache(int mode);
extern int sceGsSyncPath(int mode, int timeout);
extern XglDmaChannel *tbl[12];
extern struct MfifoDrainStorage mfifo_drain;

/* Declared here, not at the top: moving it up changes the original code. */
#define mfifo_drain mfifo_drain.channel

void xglDmaDirectNormal(u32 channel, u32 memoryAddress, u32 quadwordCount)
{
    XglDmaChannel *dma;

    if (channel >= XGL_DMA_DIRECT_CHANNELS)
        return;
    dma = tbl[channel];
    while (dma->chcr & DMA_CHCR_STR)
        ;
    dma->quadword_count = quadwordCount;
    if ((memoryAddress >> 16) != DMA_SPR_ADDRESS_HIGH)
        dma->memory_address = memoryAddress;
    else
        dma->memory_address = (memoryAddress & DMA_SPR_OFFSET_MASK) + DMA_SPR_FLAG;
    *DMAC_STAT = 1U << channel;
    dma->chcr = DMA_CHCR_STR | DMA_CHCR_TTE | DMA_CHCR_DIR;
}

void xglDmaDirectSrcChain(u32 channel, u32 tagAddress)
{
    XglDmaChannel *dma;

    if (channel >= XGL_DMA_DIRECT_CHANNELS)
        return;
    dma = tbl[channel];
    while (dma->chcr & DMA_CHCR_STR)
        ;
    dma->quadword_count = 0;
    if ((tagAddress >> 16) != DMA_SPR_ADDRESS_HIGH)
        dma->tag_address = tagAddress;
    else
        dma->tag_address = (tagAddress & DMA_SPR_OFFSET_MASK) + DMA_SPR_FLAG;
    *DMAC_STAT = 1U << channel;
    dma->chcr = DMA_CHCR_STR | DMA_CHCR_TTE | DMA_CHCR_MOD_CHAIN | DMA_CHCR_DIR;
}

void xglDmaBufferReset(XglDmaBuffer *dmaBuffer, u32 *startAddress)
{
    dmaBuffer->chainStart = startAddress;
    dmaBuffer->writeCursor = startAddress;
}

u32 *xglDmaBufferCnt(XglDmaBuffer *dmaBuffer, const u32 *packetWords, u32 quadwordCount)
{
    u32 *writeCursor = dmaBuffer->writeCursor;
    u32 copiedWords;

    writeCursor[0] = DMA_TAG_ID_CNT + quadwordCount;
    writeCursor[1] = 0;
    writeCursor[2] = 0;
    writeCursor[3] = 0;
    writeCursor += 4;
    /* Copies count quadwords (count * 4 words) of the caller's data. */
    for (copiedWords = 0; copiedWords < quadwordCount * 4; copiedWords++)
        *writeCursor++ = *packetWords++;
    dmaBuffer->writeCursor = writeCursor;
    return writeCursor;
}

u32 *xglDmaBufferRef(XglDmaBuffer *dmaBuffer, u32 dataAddress, u32 quadwordCount)
{
    u32 *writeCursor = dmaBuffer->writeCursor;

    writeCursor[0] = DMA_TAG_ID_REF + quadwordCount;
    writeCursor[1] = dataAddress;
    writeCursor[2] = 0;
    writeCursor[3] = 0;
    writeCursor += 4;
    dmaBuffer->writeCursor = writeCursor;
    return writeCursor;
}

u32 *xglDmaBufferCall(XglDmaBuffer *dmaBuffer, u32 callAddress)
{
    u32 *writeCursor = dmaBuffer->writeCursor;

    writeCursor[0] = DMA_TAG_ID_CALL;
    writeCursor[1] = callAddress;
    writeCursor[2] = 0;
    writeCursor[3] = 0;
    writeCursor += 4;
    dmaBuffer->writeCursor = writeCursor;
    return writeCursor;
}

void xglDmaBufferRequest(XglDmaBuffer *dmaBuffer, u32 channel)
{
    u32 *writeCursor = dmaBuffer->writeCursor;

    writeCursor[0] = DMA_TAG_ID_END;
    writeCursor[3] = writeCursor[2] = writeCursor[1] = 0;
    FlushCache(0);
    xglDmaDirectSrcChain(channel, (u32)dmaBuffer->chainStart);
    dmaBuffer->writeCursor = dmaBuffer->chainStart;
}

void xglDmaMFIFOSetup(u32 ringAddress, u32 ringSize, int channel)
{
    XglDmaChannel *drainChannel;

    if (channel >= XGL_DMA_DIRECT_CHANNELS)
        return;
    DMAC_CTRL->drain_mode = DMAC_CTRL_MFD_GIF;
    *DMAC_RBOR = ringAddress;
    *DMAC_RBSR = ringSize - MFIFO_QUADWORD;
    drainChannel = tbl[channel];
    mfifo_drain = drainChannel;
    *D8_MADR = ringAddress;
    drainChannel->tag_address = ringAddress;
    drainChannel->quadword_count = 0U;
    drainChannel->chcr = DMA_CHCR_STR | DMA_CHCR_MOD_CHAIN;
}

u32 xglDmaMFIFOKick(u32 scratchpadAddress, u32 quadwordCount)
{
    u32 ringSize = *DMAC_RBSR + MFIFO_QUADWORD;
    XglDmaChannel *drainChannel;
    u32 ringFreeBytes;

    while (*D8_CHCR & DMA_CHCR_STR)
        ;
    *D8_SADR = scratchpadAddress & DMA_SADR_MASK;
    *D8_QWC = quadwordCount;
    drainChannel = mfifo_drain;
    do {
        ringFreeBytes = (drainChannel->tag_address - *D8_MADR + ringSize) & (ringSize - MFIFO_QUADWORD);
        if (ringFreeBytes == 0)
            ringFreeBytes = ringSize;
    } while (quadwordCount * MFIFO_QUADWORD >= ringFreeBytes);
    *D8_CHCR = DMA_CHCR_STR;
    return ringFreeBytes;
}

void xglDmaMFIFOLeave(void)
{
    XglDmaChannel *drainChannel;

    sceGsSyncPath(0, 0);
    DMAC_CTRL->drain_mode = DMAC_CTRL_MFD_NONE;
    while (*D8_CHCR & DMA_CHCR_STR)
        ;
    drainChannel = mfifo_drain;
    while (drainChannel->chcr & DMA_CHCR_STR)
        ;
}

/* Returns at once with no effect. Only caller: InitializeSystem. */
void xglDmaInitial(void)
{
}

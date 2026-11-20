#include "common.h"
#include "shared.h"
#include "gs.h"
/* The DMAC takes scratchpad addresses (0x7000xxxx) as an offset with the SPR flag in bit 31. */
#define DMA_SPR_ADDRESS_HIGH 0x7000U
#define DMA_SPR_OFFSET_MASK 0x3fffU
#define DMA_SPR_FLAG 0x80000000U
#define DMA_CHCR_STR 0x100
/* D_ENABLEW bit 16 (CPND): holds all DMA transfers while a channel register is written. */
#define D_ENABLE_HOLD 0x10000
#define D_ENABLEW ((volatile u32 *) 0x1000f590)
#define D3_CHCR   ((volatile u32 *) 0x1000b000)
#define D3_MADR   ((volatile u32 *) 0x1000b010)
#define D3_QWC    ((volatile u32 *) 0x1000b020)

extern int DIntr(void);
extern int EIntr(void);

/* DMAC channel 4 feeds the IPU (memory to IPU input); its control register takes the transfer mode and start bit. */
#define D4_CHCR ((volatile u32 *) 0x1000b400)

extern void func_002F8C18(void);
extern void xglMcReset(void);
extern u8 D_004B3740[];
void *memset(void *destination, int value, unsigned int count);

void xglMcInitial(void)
{
    func_002F8C18();
    xglMcReset();
    memset(D_004B3740, 0, 0x130b0);
}

/* Starts a DMA from the IPU output (DMAC channel 3) into memory; count is in quadwords. */
int myDmaDirectFromIPU(u32 address, u32 count)
{
    if ((address >> 16) != DMA_SPR_ADDRESS_HIGH)
        *D3_MADR = address;
    else
        *D3_MADR = (address & DMA_SPR_OFFSET_MASK) + DMA_SPR_FLAG;
    *D3_QWC = count;

    DIntr();
    *D_ENABLEW = *D_ENABLEW | D_ENABLE_HOLD;
    *D3_CHCR = DMA_CHCR_STR;
    *D_ENABLEW = *D_ENABLEW & ~D_ENABLE_HOLD;
    return EIntr();
}

/* Starts or stops the IPU input transfer by writing D4_CHCR with the DMA interrupts held off.
 * Static in this file, as in XS1's xgl_1.c: checkdma's delay-slot filling depends on the
 * definition being in the same translation unit. */
void setD4_CHCR(u32 value)
{
    DIntr();
    *D_ENABLEW = *D_ENABLEW | D_ENABLE_HOLD;
    *D4_CHCR = value;
    *D_ENABLEW = *D_ENABLEW & ~D_ENABLE_HOLD;
    EIntr();
}

/* DMA tag IDs (bits 28..30 of the tag word): CNT for a count tag, REF for a reference tag.
 * DMA_TAG_ID_CLEAR_MASK clears the ID bits, turning a REF tag into REFE. */
#define DMA_TAG_ID_CNT 0x10000000U
#define DMA_TAG_ID_REF 0x30000000U
#define DMA_TAG_ID_CLEAR_MASK 0x0fffffff

/* DMA tag as the strip packets write it: the tag word, the address and the upper quadword. */
typedef struct XglDmaRefTag {
    unsigned int tag;
    unsigned int address;
    u64 upperQword;
} XglDmaRefTag;

/* GIF tag fields (64-bit view of the tag's low doubleword). */
#define GIF_TAG_EOP 0x8000
#define GIF_TAG_NREG_1 0x1000000000000000
#define GIF_TAG_FLG_IMAGE 0x0800000000000000
/* transMode values: which GS buffer a decoded strip is uploaded to, or memory only. */
#define MOVIE_TRANS_BUFFER_0 0
#define MOVIE_TRANS_BUFFER_1 1
#define MOVIE_TRANS_BITBLTBUF 2
/* A strip is one 16-pixel-high row of macroblocks, each macroblock 16x16 RGBA32 pixels. */
#define MOVIE_MACROBLOCK 16
#define MOVIE_MACROBLOCK_BYTES 0x400
#define MOVIE_MACROBLOCK_QWORDS 0x40
/* GIF packets of the strip upload: A+D settings, then one IMAGE tag per macroblock. */
#define MOVIE_GIF_TAG_AD_2 (GIF_TAG_NREG_1 | GIF_TAG_EOP | 2)
#define MOVIE_GIF_TAG_IMAGE_BLOCK (GIF_TAG_FLG_IMAGE | GIF_TAG_EOP | MOVIE_MACROBLOCK_QWORDS)
#define MOVIE_BITBLTBUF_DBW_512 0x8000000000000
#define MOVIE_TRXREG_MACROBLOCK GS_SET_TRXREG(MOVIE_MACROBLOCK, MOVIE_MACROBLOCK)
/* EE scratchpad RAM; the two strip packet buffers live in it. */
#define SCRATCHPAD_BASE 0x70000000
#define MOVIE_PACKET_ADDRESS_0 (SCRATCHPAD_BASE + 0x40)
#define MOVIE_PACKET_ADDRESS_1 (SCRATCHPAD_BASE + 0x1000)
/* Partial view of the movie work record: only the fields the movie code reads and writes. Offsets are the X3 build's. */
/* "ipum" */
#define MOVIE_SIGNATURE_IPUM 0x6d757069

/* Header at the start of a movie source. */
typedef struct XglMovieSourceHeader {
    unsigned int signature;             /* +0x00 */
    unsigned char unmodeled_04[4];
    unsigned short width;               /* +0x08 */
    unsigned short height;              /* +0x0a */
    unsigned short frameCount;          /* +0x0c */
    unsigned char unmodeled_0e[2];
} XglMovieSourceHeader;

#define MOVIE_DMA_QWORD 16
#define DMA_CHCR_DIR 0x1
#define DMA_CHCR_MOD_CHAIN 0x4
#define MOVIE_D4_CHCR_CHAIN_START (DMA_CHCR_STR | DMA_CHCR_MOD_CHAIN | DMA_CHCR_DIR)
#define MOVIE_D4_CHCR_CHAIN_STOP (DMA_CHCR_MOD_CHAIN | DMA_CHCR_DIR)

typedef struct XglMovieWork {
    unsigned char unmodeled_00[0x3c];
    short width;                        /* +0x3c */
    short height;                       /* +0x3e */
    short frameCount;                   /* +0x40 */
    short currentFrame;                 /* +0x42 */
    int stripQwords;                    /* +0x44: quadwords per strip row */
    int imageQwords;                    /* +0x48: quadwords of one decoded frame */
    unsigned char unmodeled_4c[0x40];
    XglMovieSourceHeader *source;       /* +0x8c */
    int sourceSize;                     /* +0x90 */
    unsigned char unmodeled_94;
    unsigned char streamOpen;           /* +0x95 */
    unsigned char loop;                 /* +0x96 */
    unsigned char unmodeled_97[4];
    unsigned char bitPosition;          /* +0x9b */
    unsigned int dmaControl;            /* +0x9c */
    unsigned int dmaAddress;            /* +0xa0 */
    unsigned int dmaQwords;             /* +0xa4 */
    XglDmaRefTag *dmaTag;               /* +0xa8 */
    unsigned char unmodeled_ac[4];
    unsigned char *imageAddress[2];     /* +0xb0: the two decoded frame buffers */
    unsigned int packetAddress[2];      /* +0xb8 */
    unsigned long long bitbltbuf;       /* +0xc0 */
    unsigned char bufferIndex;          /* +0xc8 */
    unsigned char transMode;            /* +0xc9 */
    unsigned char threshold1;           /* +0xca */
    unsigned char threshold0;           /* +0xcb */
    unsigned char *workBuffer;          /* +0xcc: memory-mode decode buffer; 0 uses the scratchpad */
} XglMovieWork;

extern void func_002E7348(void *start, void *end);

/* Partial view of sRender (the render state): the frame and second page bases the movie strips upload to. */
typedef struct XglRenderBases {
    unsigned char unmodeled_00[0x18];
    unsigned short flip_base;           /* +0x18 */
    unsigned char unmodeled_1a[2];
    unsigned short second_base;         /* +0x1c */
} XglRenderBases;

extern XglRenderBases sRender;

/*
 * Builds the GS upload packet for one 16-line strip at row y: an A+D setting block, then one
 * image block per macroblock with its TRXPOS and TRXDIR, closed by a REF tag. Returns the next free packet.
 */
unsigned long *make_gstrans(XglMovieWork *info, int y)
{
    int width = info->width;
    unsigned long *packet = (unsigned long *) info->packetAddress[info->bufferIndex];
    unsigned int image = (unsigned int) info->imageAddress[info->bufferIndex];
    int x;

    packet[0] = DMA_TAG_ID_CNT | 3;
    packet[2] = MOVIE_GIF_TAG_AD_2;
    packet[3] = GIF_AD;
    packet[1] = 0;
    switch (info->transMode) {
    case MOVIE_TRANS_BUFFER_0:
    default:
        packet[4] = ((unsigned long) sRender.flip_base << 37) | MOVIE_BITBLTBUF_DBW_512;
        break;
    case MOVIE_TRANS_BUFFER_1:
        packet[4] = ((unsigned long) sRender.second_base << 37) | MOVIE_BITBLTBUF_DBW_512;
        break;
    case MOVIE_TRANS_BITBLTBUF:
        packet[4] = info->bitbltbuf;
        break;
    }
    packet[5] = GS_BITBLTBUF;
    packet[6] = MOVIE_TRXREG_MACROBLOCK;
    packet[7] = GS_TRXREG;
    packet += 8;
    for (x = 0; x < width; x += MOVIE_MACROBLOCK) {
        packet[1] = 0;
        packet[4] = ((unsigned long) x << 32) | ((unsigned long) y << 48);
        packet[0] = DMA_TAG_ID_CNT | 4;
        packet[2] = MOVIE_GIF_TAG_AD_2;
        packet[3] = GIF_AD;
        packet[5] = GS_TRXPOS;
        packet[6] = 0;
        packet[7] = GS_TRXDIR;
        packet[8] = MOVIE_GIF_TAG_IMAGE_BLOCK;
        packet[9] = 0;
        packet += 10;
        ((XglDmaRefTag *) packet)->address = image;
        image += MOVIE_MACROBLOCK_BYTES;
        ((XglDmaRefTag *) packet)->tag = DMA_TAG_ID_REF | MOVIE_MACROBLOCK_QWORDS;
        ((XglDmaRefTag *) packet)->upperQword = 0;
        packet += 2;
    }
    ((XglDmaRefTag *) packet)[-1].tag &= DMA_TAG_ID_CLEAR_MASK;
    return packet;
}

int xglMovieIpuReset(void);

/*
 * Opens a movie whose source is in memory: sets the chain DMA when name is null, then checks the
 * "ipum" header and takes the size, frame count and strip sizes. A bad signature shuts the IPU down.
 * The stream branch of X2's xglMovieOpen is absent from this build.
 */
int xglMovieOpen(XglMovieWork *info, const char *name)
{
    XglMovieSourceHeader *source;

    if (name == 0) {
        info->dmaControl = MOVIE_D4_CHCR_CHAIN_START;
        info->dmaAddress = (unsigned int) (info->source + 1);
        info->dmaQwords = (info->sourceSize - 1) / MOVIE_DMA_QWORD;
        info->streamOpen = 0;
        info->dmaTag = 0;
        info->bitPosition = 0;
    }
    source = info->source;
    if (source->signature != MOVIE_SIGNATURE_IPUM) {
        xglMovieIpuReset();
        return 1;
    }
    info->width = source->width;
    info->stripQwords = (info->width >> 4) << 6;
    info->height = source->height;
    info->imageQwords = (info->height >> 4) * info->stripQwords;
    info->frameCount = source->frameCount;
    info->currentFrame = 0;
    info->bufferIndex = 0;
    func_002E7348(info, info + 1);
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_1", func_0018B090);

/* Writes D3_CHCR (the IPU output channel) the same way as setD4_CHCR. Inline: the original has this block in its callers. */
static inline void setD3_CHCR(u32 value)
{
    DIntr();
    *D_ENABLEW = *D_ENABLEW | D_ENABLE_HOLD;
    *D3_CHCR = value;
    *D_ENABLEW = *D_ENABLEW & ~D_ENABLE_HOLD;
    EIntr();
}

int xglMovieRewind(XglMovieWork *info)
{
    info->currentFrame = 0;
    setD4_CHCR(MOVIE_D4_CHCR_CHAIN_STOP);
    setD3_CHCR(0);
    if (info->streamOpen == 0) {
        info->dmaControl = MOVIE_D4_CHCR_CHAIN_START;
        info->dmaAddress = (unsigned int) (info->source + 1);
        info->dmaQwords = (info->sourceSize - 1) / MOVIE_DMA_QWORD;
        info->dmaTag = 0;
        info->bitPosition = 0;
    }
    func_002E7348(info, info + 1);
    return 0;
}

/* IPU registers: the command port, the control register (RESET bit) and the sync timeout. */
#define IPU_CMD ((volatile u32 *) 0x10002000)
#define IPU_CTRL ((volatile u32 *) 0x10002010)
#define IPU_CTRL_RESET 0x40000000
#define IPU_CMD_BCLR 0
#define IPU_SYNC_TIMEOUT 512

extern int func_002FAF50(int id, int timeout);

int xglMovieIpuReset(void)
{
    setD4_CHCR(0);
    *IPU_CTRL = IPU_CTRL_RESET;
    if (func_002FAF50(0, IPU_SYNC_TIMEOUT) < 0) {
        return -1;
    }
    *IPU_CMD = IPU_CMD_BCLR;
    return (func_002FAF50(0, IPU_SYNC_TIMEOUT) >= 0) ? 0 : -1;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_1", func_0018B7E0);

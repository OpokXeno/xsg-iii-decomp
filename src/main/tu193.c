#include "common.h"
#include "main/tu176.h"
#include "main/tu160.h"

extern char *gFontEscapeBuffer;
void sprintf();

s32 streamSlotLoadFrame(s32 streamSlot);

/* Calls sprintf with the slot's file name, mnu\umn\<slot + 1>.jpg, and gFontEscapeBuffer. Returns that buffer address. */
s32 streamSlotLoadFrame(s32 streamSlot) {
    char moviePath[15] = "mnu\\umn\\%d.jpg";

    sprintf(gFontEscapeBuffer, moviePath, streamSlot + 1);
    return (s32) gFontEscapeBuffer;
}

s32 memset(void *dst, s32 value, s32 size);
s32 xglJpegDecode(void *jpegRequest);

/* A zeroed 0x20-byte jpegRequest block for xglJpegDecode: words 0 and 1 hold the two arguments. */
void decodeJpegRequest(void *jpegOutput, void *jpegInput) {
    void *jpegRequest[8];

    memset(&jpegRequest[0], 0, 0x20);
    jpegRequest[0] = jpegInput;
    jpegRequest[1] = jpegOutput;
    xglJpegDecode(&jpegRequest[0]);
}

/* A table link block; its u16 linkStatusBits at +6 get 0x8100 when the table is linked. */
typedef struct PagedTableLink {
    u8 unmodeled_00[6];
    u16 linkStatusBits;          /* +0x06 */
} PagedTableLink;

/* The paged table of the decode block (StreamPlayer.pagedTable, 0x3C bytes). */
typedef struct StreamPagedTable {
    u16 key;                     /* +0x00 */
    u8 ringSlotCount;                /* +0x02: copied to StreamPlayer.ringSlotCount */
    u8 unmodeled_03[0x1D];
    s32 tableSetupFlags;              /* +0x20: |= 0x16 when the table is set up */
    u8 unmodeled_24[4];
    PagedTableLink *linkedBlock;   /* +0x28 */
    u8 unmodeled_2C[8];
    u16 unmodeled_34;            /* +0x34: nonzero test in streamPlayerSetup */
    s16 ringSlot;                /* +0x36 */
    u8 unmodeled_38[4];
} StreamPagedTable;

typedef struct StreamPlayer {
    u8 jpegOutput[8][0xE000];   /* +0x000: one decoded frame per stream slot, written by decodeJpegRequest */
    u8 cdSectorBuffer[0xE000];  /* +0x70000: the buffer address passed to xglCdReadFile */
    u8 decodeBlock[0x340];      /* +0x7E000: the block whose address shardFieldInit takes */
    StreamPagedTable pagedTable;    /* +0x7E340: the paged table of the decode block */
    McGauge prompt;             /* +0x7E37C: the Yes/No prompt setStreamActive starts */
    u8 unmodeled_7E388[8];
    s32 scrollPosition;         /* +0x7E390 */
    s32 channelBaseOffset;      /* +0x7E394 */
    s32 scrollPrevious;         /* +0x7E398 */
    s8 ringOffset;              /* +0x7E39C */
    s8 channel;                 /* +0x7E39D */
    u8 slotFade[3];             /* +0x7E39E: one level per slot */
    u8 ringSlotCount;               /* +0x7E3A1: the ring size */
    s8 readPending[8];          /* +0x7E3A2: per-slot flags, set by startTaggedCdRead */
    s16 fadeInTimer;              /* +0x7E3AA */
    s16 channelStartOffset;     /* +0x7E3AC */
    s8 active;                  /* +0x7E3AE */
    s8 playState;               /* +0x7E3AF */
    s8 stopLatch;               /* +0x7E3B0 */
    s8 setupPending;            /* +0x7E3B1 */
    s8 streamPhase;                   /* +0x7E3B2: 2 after a status header test, the loop ends when nonzero */
    u8 unmodeled_7e3b3[0x7E3C0 - 0x7E3B3];
} StreamPlayer;

static StreamPlayer *gStreamPlayerBlock;

#define gStreamPlayer (*gStreamPlayerBlock)

void startTaggedCdRead(s32 streamSlot);
void streamCdReadCallback();

/* CD read callback: a finished read (status 4) decodes its JPEG and marks the slot ready; the next slot's read then starts while slots remain. */
void streamCdReadCallback(s32 cdReadEvent, s32 unusedArg, s32 streamSlot) {
    switch (cdReadEvent) {
    case -2:
    case -1:
        break;
    case 4:
        decodeJpegRequest(gStreamPlayer.jpegOutput[streamSlot], gStreamPlayer.cdSectorBuffer);
        gStreamPlayer.readPending[streamSlot] = 2;
        break;
    case 0:
    case 1:
    case 2:
    case 3:
    default:
        return;
    }
    streamSlot++;
    if (streamSlot < gStreamPlayer.ringSlotCount) {
        startTaggedCdRead(streamSlot);
    }
}

s32 xglCdSetReadTag(s32 slot);

void startTaggedCdRead(s32 streamSlot) {
    s32 cdFileHandle;
    gStreamPlayer.readPending[streamSlot] = 1;
    xglCdSetReadTag(streamSlot);
    cdFileHandle = streamSlotLoadFrame(streamSlot);
    xglCdReadFile(cdFileHandle, &gStreamPlayer.cdSectorBuffer, 1, streamCdReadCallback);
}

extern s32 xglDirectQueueBuffer;
extern s32 xglDirectQueueCount;
extern u8 gFontVifBlock[];
s32 xglPacketGetCurrent(void);
void sceVif1PkCnt(s32 packet, s32 count);
void sceVif1PkAddDataN(s32 packet, void *data, s32 size);
void sceVif1PkRef(s32 packet, s32 address, s32 size, s32 a, s32 b, s32 c);
void nmlPacketGsInit(void);
void fontQueueColorState(u32 colour);
void fontQueueSprite(s32 a, s32 x, s32 b, s32 y);
s32 xglFontGetColorIndex(void);
void xglDirectQueueSubmitRef(void);

typedef struct GifAdEntry {
    unsigned long gifDataWord;   /* +0x00 */
    unsigned long gifRegAddress; /* +0x08: the A+D register address */
} GifAdEntry;

typedef struct FontVifBlock {
    u8 unmodeled_00[0x40];
    unsigned long vifSetupWords; /* +0x40 */
    u8 unmodeled_48[0x18];
    s32 vifTransferSize;         /* +0x60: 0xE00 for the sceVif1PkRef size */
} FontVifBlock;

/* Draws one decoded stream frame: a VIF setup packet in gFontVifBlock and a GIF queue of font state and quad packets, submitted at the end. */
void streamFrameSpriteSubmit(s32 x, s32 y, s32 packetRefAddress, s32 colour) {
    s32 packet;
    u8 alpha;
    GifAdEntry *queueStart;
    GifAdEntry *queue;
    s32 colourIndex;
    FontVifBlock *vifBlock;
    s32 queueCount;
    unsigned long xyPacked;

    alpha = colour;
    vifBlock = (FontVifBlock *) gFontVifBlock;
    packet = xglPacketGetCurrent();
    vifBlock->vifSetupWords = ((unsigned long) 0x70 << 32) | 0x80;
    vifBlock->vifTransferSize = 0xE00;
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddDataN(packet, vifBlock, 0x1C);
    sceVif1PkRef(packet, packetRefAddress, 0xE00, 0, 0x51000E00, 0);
    fontQueueColorState(alpha << 24);
    fontQueueSprite((((x + 0x6FD) << 4) + 8) & 0xFFF8, (((y + 0x71D) << 4) + 8) & 0xFFF8,
                    (((x + 0x751) << 4) + 8) & 0xFFF8, (((y + 0x767) << 4) + 8) & 0xFFF8);
    fontQueueColorState((unsigned long) alpha << 24 | 0x808080);
    nmlPacketGsInit();

    queue = (GifAdEntry *) (xglDirectQueueBuffer + xglDirectQueueCount * 16);
    queueStart = queue;
    queue->gifRegAddress = 0x3F;
    queue->gifDataWord = 0;
    queue += 1;
    queue->gifDataWord = ((unsigned long) 2 << 16 | 0x6402) << 16 | 0x3800;
    queue->gifRegAddress = 6;
    queue += 1;
    queue->gifDataWord = ((unsigned long) 0x80 << 32) | 0x80;
    queue->gifRegAddress = 0x3B;
    queue += 1;
    queue->gifRegAddress = 8;
    queue->gifDataWord = 5;
    queue += 1;
    queue->gifDataWord = 0x60;
    queue->gifRegAddress = 0x14;
    queue += 1;
    queue->gifRegAddress = 0;
    queue->gifDataWord = 0x156;
    queue += 1;
    queue->gifRegAddress = 3;
    queue->gifDataWord = 0;
    queue += 1;
    colourIndex = xglFontGetColorIndex();
    xyPacked = ((x + 0x700) << 4) | (((unsigned long) ((y + 0x720) << 4)) << 16);
    queue->gifRegAddress = 5;
    queue->gifDataWord = xyPacked | ((unsigned long) colourIndex << 32);
    queue += 1;
    queue->gifDataWord = 0x07000800;
    queue->gifRegAddress = 3;
    queue += 1;
    colourIndex = xglFontGetColorIndex();
    queue->gifRegAddress = 5;
    queueCount = xglDirectQueueCount;
    queue->gifDataWord = ((x + 0x750) << 4) | (((unsigned long) ((y + 0x766) << 4)) << 16) |
                                        ((unsigned long) colourIndex << 32);
    queue += 1;
    xglDirectQueueCount = queueCount + ((u32) queue - (u32) queueStart) / 16;
    xglDirectQueueSubmitRef();
}

s32 messageTextGet(s32);
void sheetSetIndex(s32);
void mcStateSetupRequest(s32, s32);

void setStreamActive(s32 enableStream) {
    s32 primarySheetHandle;
    s32 secondarySheetHandle;
    s32 stopSheetHandle;
    if (enableStream != 0) {
        mcStateSetupRequest(6, 0);
        primarySheetHandle = messageTextGet(0x30F);
        sheetSetIndex(primarySheetHandle);
        secondarySheetHandle = messageTextGet(0x30D);
        choicePromptInit(&gStreamPlayer.prompt, (const char *) secondarySheetHandle, 1, 0);
        gStreamPlayer.active = 1;
        gStreamPlayer.playState = 1;
        gStreamPlayer.stopLatch = -0x80;
        gStreamPlayer.channelStartOffset = (gStreamPlayer.channel * 0x670) - gStreamPlayer.channelBaseOffset;
        return;
    }
    mcStateSetupRequest(7, 0);
    stopSheetHandle = messageTextGet(0x30E);
    sheetSetIndex(stopSheetHandle);
    gStreamPlayer.active = 0;
    if ((gStreamPlayer.stopLatch >= 0) && (gStreamPlayer.playState == 1)) {
        gStreamPlayer.playState = 2;
        return;
    }
    gStreamPlayer.playState = 0;
}

void *memPoolAlloc(s32 size);
s32 bufferPointersRewind();
s32 completionBlockClear();
s32 pagedTablePoolInit();

/* Allocates and zeroes the 0x7E3C0-byte stream state at gStreamPlayerBlock. Returns 1 on success. */
s32 allocStreamState(void) {
    StreamPlayer *newStreamPlayer;

    completionBlockClear();
    pagedTablePoolInit();
    newStreamPlayer = memPoolAlloc(0x7E3C0);
    gStreamPlayerBlock = newStreamPlayer;
    memset(newStreamPlayer, 0, 0x7E3C0);
    bufferPointersRewind();
    return 1;
}

void xglPadSetDefaults(void);
s32 xglCdServiceIfPending(void);
void statusBlockFrameUpdate(void);
void xcBlockFlagsReset(void *block);
void memPoolFree(void *block);
void streamPlayerShutdown(void);

/* Stream shutdown: waits for pending CD reads, frees the stream state and clears gStreamPlayerBlock. */
void streamPlayerShutdown(void) {
    xglPadSetDefaults();
    while (xglCdServiceIfPending() != 0) {
        statusBlockFrameUpdate();
    }
    xcBlockFlagsReset(gStreamPlayer.decodeBlock);
    memPoolFree(gStreamPlayerBlock);
    gStreamPlayerBlock = 0;
}

s32 saveStateIndexGet(void);
void pagedTableAppend(void *table, s32 index, s32 value);
s32 pagedTableGetKeyAt(void *table, s32 index);
void mcModuleReset(void);
void sheetStateReset(void);
void pagedTableInit(void *table, s32 entryCount);
void pagedTableSetPageSizes(void *table, s32 a, s32 b);
void setStreamActive(s32 enableStream);
void xglPadSetRepeat(s32 a, s32 b, s32 c, s32 d);
extern u16 gStreamTrackTable[][4];
void streamPlayerSetup(void);

/* Stream setup after the state allocation: resets the paged table from the save state and track table, sets the scroll origin and starts the first CD read. */
void streamPlayerSetup(void) {
    StreamPagedTable *pagedTable;
    u16 *linkStatusBitsPtr;
    StreamPlayer *streamPlayer;
    s32 channelBase;
    s32 i;

    gStreamPlayer.fadeInTimer = 0;
    choicePromptInit(&gStreamPlayer.prompt, 0, 0, 0);
    mcModuleReset();
    sheetStateReset();
    pagedTable = &gStreamPlayer.pagedTable;
    pagedTableInit(pagedTable, 8);
    pagedTableSetPageSizes(pagedTable, 3, 0);
    for (i = 0; i < 8; i++) {
        if (saveStateIndexGet() >= gStreamTrackTable[i][2]) {
            if (battleRosterEntryIdCountGet(0) < 3) {
                pagedTableAppend(pagedTable, i, 258);
            } else {
                pagedTableAppend(pagedTable, i, 2);
            }
        }
    }
    if (saveStateIndexGet() < 199 && (pagedTableGetKeyAt(pagedTable, 3) & 0x100) == 0) {
        if (pagedTable->unmodeled_34 != 0) {
            linkStatusBitsPtr = &pagedTable->linkedBlock->linkStatusBits;
            *linkStatusBitsPtr |= 0x8100;
        }
    }
    gStreamPlayer.ringSlotCount = pagedTable->ringSlotCount;
    streamPlayer = gStreamPlayerBlock;
    pagedTable->tableSetupFlags |= 0x16;
    streamPlayer->ringOffset = 0;
    gStreamPlayer.slotFade[0] = -128;
    gStreamPlayer.slotFade[1] = 0;
    gStreamPlayer.slotFade[2] = 0;
    channelBase = gStreamPlayer.pagedTable.key * 1648 - 1008;
    gStreamPlayer.scrollPrevious = channelBase;
    gStreamPlayer.channelBaseOffset = channelBase;
    gStreamPlayer.scrollPosition = channelBase;
    setStreamActive(0);
    startTaggedCdRead(0);
    xglPadSetRepeat(0, 0xF000, 10, 3);
}

int xglFontDrawRightAlignedHalfCell(s32 a, s32 x, s32 y, s32 width);

/* Draws the frame and count message lines starting at messageBase for one row. */
void streamMessageLinesDraw(s32 row, s32 messageBase, s32 messageLineCount, s32 colour) {
    s32 i;
    u8 alpha;

    alpha = colour >> 24;
    fontQueueColorState(alpha << 24);
    fontQueueSprite(0x87B8, (((row + 0x738) << 4) + 8) & 0xFFF8, 0x8CF8, (((row + 0x782) << 4) + 8) & 0xFFF8);
    fontQueueColorState((unsigned long) alpha << 24 | 0x404040);
    fontQueueSprite(0x87D8, (((row + 0x73A) << 4) + 8) & 0xFFF8, 0x8CD8, (((row + 0x780) << 4) + 8) & 0xFFF8);
    fontQueueColorState(colour);
    row = row - messageLineCount * 8 + 64;
    for (i = 0; i < messageLineCount; i++) {
        xglFontDrawRightAlignedHalfCell(0x8A58, (u16) (((row + i * 16 + 0x71F) << 4) + 8),
                                        messageTextGet(messageBase + i), 12);
    }
}

void xglFontDrawBoxBlend(u16 *box, s32 a, s32 b, s32 c);
void fontQueueRectTwoColors(u16 *box, u32 colour, u32 colourTwo, u32 a);
void fontQueueLine(u16 x0, u16 y0, u16 x1, u16 y1);
s32 xglFontDrawCharScaled(u16 x, u16 y, s32 a, s32 b);
s32 xglFontDrawStringScaled(u16 x, u16 y, s32 a, s32 b);
s32 xglFontDrawUnsignedNumber(u16 x, u16 y, s32 value, s32 width);
void xglFontFreeBuffer(void);
void xglFontPrintTitle(void);
void xglFontPrintAlphaCommand(s32 a, s32 b, s32 c);
void xglFontPrintExtendedColor(s32 a, s32 b, s32 c);
void xglFontPrintEscapeByte(s32 a);
void xglFontSetColor(s32 r, s32 g, s32 b);
void xglFontPrintIndexed(s32 a, s32 b, s32 c);

static inline unsigned long long alphaMul(s32 left, s32 right) {
    return (unsigned long long) (left * right) >> 7;
}

/* Draws one stream slot's panel at a row: backgrounds, title texts, then its decoded frame when ready or the placeholder messages. */
void streamSlotPanelDraw(s32 streamSlot, s32 row, u8 alpha) {
    u16 box[4];
    unsigned long long shade;
    s32 colour;
    u16 y;
    s32 column;

    shade = (unsigned long long) alphaMul(alpha, 0x73) << 24;
    y = ((row + 0x71F) << 4) + 8;
    box[0] = 0x7288;
    box[2] = 0x8D78;
    box[1] = y;
    box[3] = y + 0x670;
    fontQueueColorState(shade | 0x221E1D);
    xglFontDrawBoxBlend(box, 2, 0, 0);
    box[0] = 0x72C8;
    box[1] = y + 0x40;
    box[2] = box[0] + 0x1A80;
    box[3] = box[1] + 0x120;
    fontQueueRectTwoColors(box, ((unsigned long long) alpha << 24) | 0x3A2827, 0x3A2827, 0x101);
    fontQueueColorState(((unsigned long long) alpha << 24) | 0x804443);
    xglFontDrawCharScaled(0x7348, y + 0x60, 0x2A, 0x10);
    colour = ((unsigned long long) alpha << 24) | 0x562E2D;
    fontQueueColorState(colour);
    column = xglFontDrawStringScaled(0x7448, y + 0x60, messageTextGet(0x30C), 0x10) + 4;
    column += xglFontDrawUnsignedNumber(((column << 4) + 0x7448) & 0xFFF8, y + 0x50, 0, 0xF);
    xglFontDrawUnsignedNumber(((column << 4) + 0x7448) & 0xFFF8, y + 0x50, streamSlot + 1, 0xF);
    fontQueueColorState(shade | 0x423732);
    fontQueueLine(0x73C8, y + 0x310, 0x86C8, y + 0x310);
    fontQueueLine(0x73C8, y + 0x470, 0x86C8, y + 0x470);
    fontQueueLine(0x73C8, y + 0x5D0, 0x86C8, y + 0x5D0);
    xglFontPrintTitle();
    xglFontPrintAlphaCommand(0x44, 0, alpha);
    xglFontPrintExtendedColor(0, 0, 0);
    xglFontPrintEscapeByte(0x10);
    xglFontSetColor(0x80, 0x80, 0x80);
    xglFontPrintIndexed(0x3F, row + 0x31, messageTextGet(streamSlot * 2 + 0x320));
    xglFontPrintIndexed(0x3F, row + 0x47, messageTextGet(streamSlot * 2 + 0x321));
    if ((u8) gStreamPlayerBlock->readPending[streamSlot] == 2) {
        streamFrameSpriteSubmit(0x17E, row + 0x1B, (s32) gStreamPlayerBlock->jpegOutput[streamSlot], alpha);
        xglFontFreeBuffer();
    } else {
        streamMessageLinesDraw(row, 0x264, 2, colour);
    }
}

typedef struct StreamSpritePos {
    s16 x;
    u16 y;
} StreamSpritePos;

typedef struct StreamWord {
    s32 channelWord;
} StreamWord;

void xglFontFlush(void);
void xglFontSetDigitStep(s32);
void xglFontSetAlphaScale(s32);
void xglFontSetColorScale(s32);
void xglFontResetState(void);
void fontQueueSpriteReset(void);
void shardFieldUpdate(void *block);
s32 colorModulateByte(u8 level, u8 flag);
u8 statusByte0Get(void);
u8 statusByte1Get(void);
void sheetDrawFrame(s32);
void mcStateSeekEntry(s32);
void func_0021A7E0(s32, s32, s32, s32);
void func_0021ABB0(s32, s32, s32, s32);
void fontDrawRotatedSpriteU143(StreamSpritePos *pos, s32, s32, s32);
void fontDrawPanelFrame(StreamSpritePos *pos, s32, s32);
u8 pagedTableGetSlotByte(StreamPagedTable *table, s32 slot);
u8 pagedTableGetFlagByte(StreamPagedTable *table, s32 slot);

typedef struct StreamPlayerRef {
    StreamPlayer *player;
} StreamPlayerRef;

static inline StreamPlayer *streamPlayer(void) {
    return ((StreamPlayerRef *) &gStreamPlayerBlock)->player;
}

static inline unsigned long long alphaMulView(s32 scale, s32 alpha) {
    return (unsigned long long) (alpha * scale) >> 7;
}

/* Draws the stream player screen: scrolling slot panels, status sprites and the setup prompt. */
void streamPlayerDraw(void) {
    StreamPagedTable *pagedTableView = &streamPlayer()->pagedTable;
    StreamSpritePos sprite;
    s32 alpha;
    s32 i;
    s32 pos;
    u32 nextPos;
    s32 streamSlot;
    s32 smoothed;
    s32 colour;
    u32 level;
    u8 levelFlagByte;
    s32 headerText;
    s32 messageText;
    u16 key;

    xglFontFlush();
    xglFontSetDigitStep(10);
    xglFontSetAlphaScale(0x80);
    fontQueueSpriteReset();
    xglFontSetAlphaScale(streamPlayer()->fadeInTimer < 16 ? ((u8) streamPlayer()->fadeInTimer * 8) & 0xFF : 0x80);
    shardFieldUpdate(streamPlayer()->decodeBlock);
    xglFontFreeBuffer();
    xglFontSetAlphaScale((0x80 - colorModulateByte((u8) streamPlayer()->prompt.fadeIn,
                                                   streamPlayer()->prompt.flag)) &
                         0xFF);

    smoothed =
        streamPlayer()->scrollPosition +
        ((streamPlayer()->scrollPosition - streamPlayer()->scrollPrevious) +
         ((((StreamWord *) &streamPlayer()->channelBaseOffset)->channelWord - streamPlayer()->scrollPosition) >>
          3));
    streamPlayer()->scrollPosition = smoothed;
    streamPlayer()->scrollPrevious = smoothed;

    if ((u32) streamPlayer()->ringSlotCount < 3) {
        pos = 0x3F0;
        for (i = 0; i < streamPlayer()->ringSlotCount; i++) {
            nextPos = pos + 0x670;
            if (nextPos < 0x2270U) {
                if (pos < 0x3F0) {
                    alpha = 0x80 - ((0x3F0 - pos) >> 3);
                } else if (pos > 0x10D0) {
                    alpha = 0x80 - ((pos - 0x10D0) >> 3);
                } else {
                    alpha = 0x80;
                }
                streamSlot = (i + streamPlayer()->ringSlotCount) % streamPlayer()->ringSlotCount;
                level = pagedTableGetSlotByte(pagedTableView, streamSlot);
                level >>= 1;
                level += 0x60;
                level &= 0xFF;
                levelFlagByte = pagedTableGetFlagByte(pagedTableView, streamSlot);
                colour = colorModulateByte(level, levelFlagByte);
                xglFontSetColorScale((colour >> streamPlayer()->setupPending) & 0xFF);
                if (alpha > 0 && (streamPlayer()->playState == 0 || streamSlot != streamPlayer()->channel)) {
                    streamSlotPanelDraw(streamSlot, pos >> 4, alpha);
                }
            }
            pos = nextPos;
        }
    } else {
        pos = -streamPlayer()->scrollPosition - streamPlayer()->ringSlotCount * 0x670;
        for (i = -streamPlayer()->ringSlotCount; i < streamPlayer()->ringSlotCount * 2; i++) {
            nextPos = pos + 0x670;
            if (nextPos < 0x2270U) {
                if (pos < 0x3F0) {
                    alpha = 0x80 - ((0x3F0 - pos) >> 3);
                } else if (pos > 0x10D0) {
                    alpha = 0x80 - ((pos - 0x10D0) >> 3);
                } else {
                    alpha = 0x80;
                }
                streamSlot = (i + streamPlayer()->ringSlotCount) % streamPlayer()->ringSlotCount;
                level = pagedTableGetSlotByte(pagedTableView, streamSlot);
                level >>= 1;
                level += 0x60;
                level &= 0xFF;
                levelFlagByte = pagedTableGetFlagByte(pagedTableView, streamSlot);
                colour = colorModulateByte(level, levelFlagByte);
                xglFontSetColorScale((colour >> streamPlayer()->setupPending) & 0xFF);
                if (alpha > 0 && (streamPlayer()->playState == 0 || streamSlot != streamPlayer()->channel)) {
                    streamSlotPanelDraw(streamSlot, pos >> 4, alpha);
                }
            }
            pos = nextPos;
        }
    }

    xglFontSetColorScale(streamPlayer()->setupPending != 0 ? 0x40 : 0x80);
    xglFontResetState();
    sprite.x = 0x7288;
    sprite.y = 0x75E8;
    for (i = 0; i < 3; i++) {
        fontQueueColorState(
            (alphaMulView(statusByte0Get() / 2 + 0x40, streamPlayer()->slotFade[i]) << 24) | 0x71802F);
        fontDrawPanelFrame(&sprite, 0x1AF0, 0x670);
        sprite.y += 0x670;
    }

    if (streamPlayer()->ringSlotCount >= 3) {
        xglFontSetColorScale((statusByte1Get() + 0x40) & 0xFF);
        fontQueueColorState(0x80562E2D);
        sprite.x = 0x7F78;
        sprite.y = 0x74C8;
        fontDrawRotatedSpriteU143(&sprite, 0, 0x110, 0x110);
        sprite.y = -0x76B8;
        fontDrawRotatedSpriteU143(&sprite, 2, 0x110, 0x110);
        xglFontSetColorScale(0x80);
    }

    xglFontSetAlphaScale(0x80);
    if (streamPlayer()->playState != 0) {
        pos =
            ((((streamPlayer()->channelStartOffset - 0x3F0) * (u8) streamPlayer()->stopLatch) >> 7) + 0x3F0);
        streamSlotPanelDraw(streamPlayer()->channel, pos >> 4, 0x80);
    }
    choicePromptDraw(&streamPlayer()->prompt, 0, 0xAE);
    xglFontSetColorScale(0x80);

    if (streamPlayer()->setupPending != 0) {
        headerText = messageTextGet(0x313);
        key = pagedTableGetKeyAt(&streamPlayer()->pagedTable,
                                                                             streamPlayer()->pagedTable.key);
        if (key & 0x8000) {
            messageText = messageTextGet(0x314);
        } else {
            messageText = messageTextGet(0x312);
        }
        func_0021A7E0(0x100, 0xA0, headerText, messageText);
    }

    xglFontSetAlphaScale(streamPlayer()->fadeInTimer < 16 ? ((u8) streamPlayer()->fadeInTimer * 8) & 0xFF : 0x80);
    sheetDrawFrame(0x19F);
    xglFontSetAlphaScale(0x80);
    xglFontSetAlphaScale(streamPlayer()->fadeInTimer < 16 ? ((u8) streamPlayer()->fadeInTimer * 8) & 0xFF : 0x80);
    mcStateSeekEntry(0x18E - sheetLatchedPositionGet());
    messageText = messageTextGet(9);
    func_0021ABB0(0, 0xB, messageText, 0);
    xglFontSetAlphaScale(0x80);
}

extern u8 gStreamTrackIndex;
extern s8 gVsyncsPerFrame;
void seVoiceStartDefault(unsigned int handle);
void pagedTableAnimateSlots(void *cursor);
void pagedTableMoveCursor(void *block, s32 delta);
s32 statusHeaderIsReadyMode20(void);
s32 statusHeaderIsReadyMode40(void);
void func_00220E00(void);
void func_002209C0(void);

/* Per-frame stream update: fade timers, ring movement from the status header, slot fades and the play/stop state. */
void streamPlayerUpdate(void) {
    StreamPagedTable *cursor;
    s32 i;

    if (streamPlayer()->fadeInTimer < 255) {
        streamPlayer()->fadeInTimer += gVsyncsPerFrame << 1;
    }
    cursor = &streamPlayer()->pagedTable;
    pagedTableAnimateSlots(cursor);
    if (streamPlayer()->active == 0 && streamPlayer()->setupPending == 0) {
        if (streamPlayer()->ringSlotCount < 3) {
            if ((getStatusHeader()->pressed & 0x1000) || (getStatusHeader()->pressed & 0x4000)) {
                if (streamPlayer()->ringSlotCount >= 2) {
                    pagedTableMoveCursor(cursor, 1);
                    streamPlayer()->ringOffset ^= 1;
                    seVoiceStartDefault(1);
                }
            }
        } else {
            if ((getStatusHeader()->repeated & 0x1000) && !(getStatusHeader()->held & 0x4000) &&
                (cursor->key != 0 || (getStatusHeader()->pressed & 0x1000))) {
                pagedTableMoveCursor(cursor, -1);
                if (streamPlayer()->ringOffset > 0) {
                    streamPlayer()->ringOffset -= 1;
                } else {
                    streamPlayer()->channelBaseOffset -= 0x670;
                }
                cursor->ringSlot = (cursor->key - streamPlayer()->ringOffset + streamPlayer()->ringSlotCount) %
                                 streamPlayer()->ringSlotCount;
                if (cursor->key == streamPlayer()->ringSlotCount - 1) {
                    streamPlayer()->scrollPosition += streamPlayer()->ringSlotCount * 0x670;
                    streamPlayer()->scrollPrevious += streamPlayer()->ringSlotCount * 0x670;
                    streamPlayer()->channelBaseOffset += streamPlayer()->ringSlotCount * 0x670;
                }
                seVoiceStartDefault(1);
            }
            if ((getStatusHeader()->repeated & 0x4000) && !(getStatusHeader()->held & 0x1000) &&
                (cursor->key != streamPlayer()->ringSlotCount - 1 || (getStatusHeader()->pressed & 0x4000))) {
                pagedTableMoveCursor(cursor, 1);
                if (streamPlayer()->ringOffset < 2) {
                    streamPlayer()->ringOffset += 1;
                } else {
                    streamPlayer()->channelBaseOffset += 0x670;
                }
                cursor->ringSlot = (cursor->key - streamPlayer()->ringOffset + streamPlayer()->ringSlotCount) %
                                 streamPlayer()->ringSlotCount;
                if (cursor->key == 0) {
                    streamPlayer()->scrollPosition -= streamPlayer()->ringSlotCount * 0x670;
                    streamPlayer()->scrollPrevious -= streamPlayer()->ringSlotCount * 0x670;
                    streamPlayer()->channelBaseOffset -= streamPlayer()->ringSlotCount * 0x670;
                }
                seVoiceStartDefault(1);
            }
        }
    }
    for (i = 0; i < 3; i++) {
        if (streamPlayer()->ringOffset == i) {
            if ((s8) streamPlayer()->slotFade[i] >= 0) {
                streamPlayer()->slotFade[i] =
                    (u8) (streamPlayer()->slotFade[i] + (u8) gVsyncsPerFrame * 0x10);
            }
        } else if ((u8) streamPlayer()->slotFade[i] != 0) {
            streamPlayer()->slotFade[i] = (u8) (streamPlayer()->slotFade[i] - (u8) gVsyncsPerFrame * 0x10);
        }
    }
    if (streamPlayer()->setupPending != 0) {
        if (statusHeaderIsReadyMode20() != 0 || statusHeaderIsReadyMode40() != 0) {
            streamPlayer()->setupPending = 0;
            setStreamActive(0);
            seVoiceStartDefault(8);
        } else if (getStatusHeader()->pressed & 0x10) {
            streamPlayer()->streamPhase = 1;
            seVoiceStartDefault(7);
        }
    } else if (streamPlayer()->active != 0) {
        if (choicePromptUpdate(&streamPlayer()->prompt, 1) != 0) {
            if (streamPlayer()->prompt.state == 0) {
                gStreamTrackIndex = streamPlayer()->channel;
                streamPlayer()->streamPhase = 2;
                seVoiceStartDefault(2);
            } else {
                seVoiceStartDefault(8);
            }
            setStreamActive(0);
        }
    } else {
        choicePromptUpdate(&streamPlayer()->prompt, 0);
        if (statusHeaderIsReadyMode20() != 0) {
            if (pagedTableGetKeyAt(&streamPlayer()->pagedTable,
                                                                     streamPlayer()->pagedTable.key) &
                0x100) {
                streamPlayer()->setupPending = 1;
                mcStateSetupRequest(7, 0);
                seVoiceStartDefault(3);
            } else {
                streamPlayer()->channel = streamPlayer()->pagedTable.key;
                setStreamActive(1);
                seVoiceStartDefault(2);
            }
        } else if (statusHeaderIsReadyMode40() != 0 || (getStatusHeader()->pressed & 0x10)) {
            streamPlayer()->streamPhase = 1;
            seVoiceStartDefault(7);
        }
    }
    if (streamPlayer()->playState == 1 && (u8) streamPlayer()->stopLatch != 0) {
        streamPlayer()->stopLatch = (s8) ((u8) streamPlayer()->stopLatch - (u8) gVsyncsPerFrame * 0x10);
    } else if (streamPlayer()->playState == 2) {
        if (streamPlayer()->stopLatch >= 0) {
            streamPlayer()->stopLatch = (s8) ((u8) streamPlayer()->stopLatch + (u8) gVsyncsPerFrame * 0x10);
        } else {
            streamPlayer()->playState = 0;
        }
    }
    func_00220E00();
    func_002209C0();
}

s32 statusHeaderTestFlags(void);
void sefFlagSetBit26(void);
void swapWordsSendCommand(s32 command);
void swapWordsRestore(void);
void pxyEnableFlag8(void);
s32 streamPlayerRunLoop();

/* Stream main loop: runs streamPlayerUpdate until the phase is set, sending the swap-word commands; returns 0. */
s32 streamPlayerRunLoop(void) {
    do {
        streamPlayerUpdate();
        if (statusHeaderTestFlags()) {
            sefFlagSetBit26();
            gStreamPlayer.streamPhase = 2;
        }
        if (gStreamPlayer.streamPhase != 0) {
            swapWordsSendCommand((gStreamPlayer.streamPhase == 2) ? 30 : 5);
        }
        pxyEnableFlag8();
        completionCallbackAdd(streamPlayerDraw);
        statusBlockFrameUpdate();
    } while (gStreamPlayer.streamPhase == 0);
    swapWordsRestore();
    return 0;
}

void memPoolInit(s32 base, s32 size);
void memPoolReset();
void func_00225938(s32 a, s16 b, s32 c);
void pagedTablePoolShutdown();
s32 runDecodePipeline();

s32 runStreamPlayback(void) {
    s32 pipelineResult;
    s32 savedPipelineGuard;
    u32 trackRecordWord;
    u32 *trackRecord;

    savedPipelineGuard = gVsyncsPerFrame;
    gVsyncsPerFrame = 1;
    memPoolInit(0x980000, 0x100000);
    gStreamTrackIndex = -1;
    pipelineResult = runDecodePipeline();
    pagedTablePoolShutdown();
    memPoolReset();
    gVsyncsPerFrame = savedPipelineGuard;
    if (gStreamTrackIndex != 0xFF) {
        trackRecord = &((u32 *) gStreamTrackTable)[gStreamTrackIndex * 2];
        trackRecordWord = trackRecord[0];
        func_00225938(trackRecordWord & 0x3FFF, ((s16 *) trackRecord)[1], (trackRecordWord >> 0xE) & 3);
    }
    return pipelineResult;
}

s32 shardFieldInit(void *block);
s32 allocStreamState();

s32 runDecodePipeline(void) {
    s32 pipelineResult;

    if (allocStreamState() == 0) {
        return -1;
    }
    shardFieldInit(&gStreamPlayer.decodeBlock);
    seVoiceStartDefault(0xB);
    streamPlayerSetup();
    pipelineResult = streamPlayerRunLoop();
    streamPlayerShutdown();
    return pipelineResult;
}

u16 gStreamTrackTable[][4] = {
    { 1010, 1, 0, 0 },
    { 1150, 101, 102, 0 },
    { 1810, 100, 102, 0 },
    { 18494, 3, 116, 0 },
    { 2670, 101, 257, 0 },
    { 20094, 101, 269, 0 },
    { 3830, 100, 284, 0 },
    { 20394, 100, 314, 0 }
};

u8 gFontVifBlock[112] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 6, 0, 0, 81,
    4, 0, 0, 0, 0, 0, 0, 16,
    14, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 56, 8, 0,
    80, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    81, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    82, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    83, 0, 0, 0, 0, 0, 0, 0,
    0, 112, 0, 0, 0, 0, 0, 8,
    0, 0, 0, 0, 0, 0, 0, 0
};

u8 gStreamTrackIndex = 0xFF;
s32 D_00550C34 = 0;

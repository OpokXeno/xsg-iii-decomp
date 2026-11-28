#include "common.h"

#include "gs.h"
#include "main/Packet.h"
#include "main/xgl_render.h"
#include "main/tu007.h"

/* One value of the scratchpad block at 0x70000000 (the effect work area). */
#define SEF_SCRATCHPAD(type, offset) (*(type *) (0x70000000 + (offset)))
typedef signed char s8;

typedef int s32;

typedef unsigned int u32;

typedef float f32;

typedef unsigned char u8;

extern s8 gXglCdDiskStatus;

void xglCdSetDiskStatus(s8 diskStatus) {
    gXglCdDiskStatus = diskStatus;
}

extern u64 D_003EBBC0[];
extern u64 D_003EBC20[];
extern u64 D_003EBC40[];
extern u16 D_004C8390[];
/* The render state block at 0x004C8390 (include/main/xgl_render.h). */
#define gRenderState (*(XglRenderStateBlock *) &D_004C8390)

/* Stride of one NarrEv entry (the NarrEvEntryTable records) and of one 0x7EC-byte text block of entryTextBlock. */
#define NARR_EV_ENTRY_SIZE 0x290
#define NARR_EV_TEXT_STRIDE 0x7EC
extern s32 D_00550D98;
extern s32 D_00550D9C;
extern s32 D_00550DA4;
extern s32 D_00550DA8;
extern s32 D_00550DAC;
extern s32 D_00550DB0;
XglPacket *xglPacketGetCurrent(void);
void sceVif1PkCnt(void *packet, u32 count);
void sceVif1PkAddDataN(void *packet, void *data, u32 qwc);
void sceVif1PkRef(void *packet, void *data, u32 qwc, u32 a, u32 b, u32 c);
void sceVif1PkOpenDirectHLCode(void *packet, u32 count);
void sceVif1PkCloseDirectHLCode(void *packet);
void gifStateReset(void);
void gifStateAddReg(s32 gifAddress, u64 gifData);
void gifStateFlush(void);
s32 narrEvPushPictureGif(s32 status);

/* Sends one GIF packet that shows the two picture pages at imageAddress (the second one 0x70000 further on), then
 * advances the 8-bit alpha in gXglCdDiskStatus by 3, wrapping to -128 once it passes 128. */
s32 narrEvPushPictureGif(s32 imageAddress) {
    void *packet;
    s32 nextAlpha;
    u64 *gifHeader;

    gifHeader = D_003EBBC0;
    packet = xglPacketGetCurrent();
    gifHeader[4] = ((u64) gRenderState.work_buffer_base << 37) | ((u64) 0x8000 << 36);
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddDataN(packet, gifHeader, 24);
    {
        u32 zeroWord = 0;
        sceVif1PkRef(packet, D_003EBC20, 2, zeroWord, zeroWord, zeroWord);
    }
    sceVif1PkRef(packet, (void *) imageAddress, 28672, 0, 0x51007000, 0);
    sceVif1PkRef(packet, D_003EBC20, 2, 0, 0, 0);
    sceVif1PkRef(packet, (void *) (imageAddress + 0x70000), 28672, 0, 0x51007000, 0);
    sceVif1PkRef(packet, D_003EBC40, 3, 0, 0, 0);
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_TEXFLUSH, 0);
    gifStateAddReg(GS_ZBUF_1, GS_SET_ZBUF(0, GS_PSMZ24, 1));
    gifStateAddReg(GS_TEST_1, GS_TEST_ZTE_ALWAYS);
    gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(0, 1, 2, 1, (u8) gXglCdDiskStatus));
    gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(gRenderState.display_buffer_base, 8, GS_PSMCT32,
                                            0xFF000000));
    gifStateAddReg(GS_TEX0_1, GS_SET_TEX0(gRenderState.work_buffer_base << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
    gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, 0x3F800000));
    gifStateAddReg(GS_PRIM, GS_PRIM_TEXTURED_SPRITE);
    gifStateAddReg(GS_UV, GS_SET_UV(0, 0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(((D_00550D98 + D_00550DA4) << 4) - 8,
                                        ((D_00550D9C + D_00550DA8) << 4) - 9, 0xFFFFFF));
    gifStateAddReg(GS_UV, GS_SET_UV(512 * 16, 448 * 16));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(((D_00550D98 + D_00550DAC) << 4) - 8,
                                        ((D_00550D9C + D_00550DB0) << 4) - 9, 0xFFFFFF));
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);
    nextAlpha = (u8) gXglCdDiskStatus + 3;
    if ((u8) nextAlpha >= 129) {
        gXglCdDiskStatus = -128;
    } else {
        gXglCdDiskStatus = nextAlpha;
    }
}

extern unsigned char gXglCdReadJob[];
void *memset(void *, int, unsigned int);

/* The 0x38-byte CD read request block (gXglCdReadJob). xglCdReadJobReset resets it, xglCdReadFileBlocking waits for it
 * and reads loadTarget and completionFlags, and xglCdReadJobDecodeNext hands it to xglJpegDecode once the read is done. */
typedef struct XglCdReadJob {
    s32 jpegSource;                 /* +0x00: the request word xglJpegDecode reads, set to loadTarget */
    s32 unk_04;                     /* +0x04: set to 0xE00000 by xglCdReadJobReset */
    u8 unk_08[0x18];
    s32 pageBufferAddress[2];       /* +0x20: the two page buffers, picked by pageBufferIndex */
    s32 loadTarget;                /* +0x28 */
    u8 unk_2C[4];
    s32 pageBufferIndex;            /* +0x30: toggled 0 / 1 after every decode */
    s32 completionFlags;            /* +0x34: bit 0 = the read finished */
} XglCdReadJob;

#define xglCdReadJob (*(XglCdReadJob *) gXglCdReadJob)

/* Resets the read request block and points it at the two page buffers (0xEE0000, 0xFC0000). */
void xglCdReadJobReset(void) {
    memset(gXglCdReadJob, 0, 0x38);
    xglCdReadJob.pageBufferAddress[0] = 0xEE0000;
    xglCdReadJob.pageBufferAddress[1] = 0xFC0000;
    xglCdReadJob.unk_04 = 0xE00000;
    xglCdReadJob.loadTarget = *(s32 *) &xglCdReadJob.pageBufferAddress[0];
    xglCdReadJob.pageBufferIndex = 0;
    xglCdReadJob.completionFlags = 0;
}

s32 xglCdReadFile(s32 fileId, s32 destination, s32 mode, s32 notify);
s32 xglCdServiceIfPending();

/* Waits until the CD service is idle, starts the read of fileId into the load address and marks the request done. */
void xglCdReadFileBlocking(s32 fileId) {
    while (xglCdServiceIfPending() != 0) {
    }
    xglCdReadFile(fileId, xglCdReadJob.loadTarget, 1, 0);
    xglCdReadJob.completionFlags |= 1;
}

void pxyProfileBegin(void *p);
void pxyProfileEnd(void *p);
s32 xglJpegDecode(s32 *request);

/* Once the CD read is done, decodes the JPEG it loaded, flips the page buffer and points the next read at it. */
void xglCdReadJobDecodeNext(void) {
    s32 pageBufferIndex;
    s32 nextPage;
    XglCdReadJob *job;

    if (xglCdServiceIfPending() == 0 && (xglCdReadJob.completionFlags & 1) != 0) {
        xglCdReadJob.jpegSource = xglCdReadJob.loadTarget;
        pxyProfileBegin("DecodeJpeg");
        xglJpegDecode((s32 *) &xglCdReadJob);
        pxyProfileEnd("DecodeJpeg");
        if (xglCdReadJob.pageBufferIndex != 0) {
            xglCdReadJob.pageBufferIndex = 0;
        } else {
            xglCdReadJob.pageBufferIndex = 1;
        }
        job = &xglCdReadJob;
        pageBufferIndex = job->pageBufferIndex;
        nextPage = job->pageBufferAddress[pageBufferIndex];
        job->completionFlags &= ~1;
        job->loadTarget = nextPage;
        xglCdSetDiskStatus(0);
        /* kept: the empty do-while(0) fixes the instruction order around the flag update */
        do {
        } while (0);
    }
}

void xglCdFlushStatus(void) {
    narrEvPushPictureGif(((s32 *) gXglCdReadJob)[1]);
}

/* The pad record PadData up to its button words: buttons.allButtons is the doubleword of the four button halfwords,
 * buttons.buttonHalves[1] the newly pressed ones (kept apart: the union member decides the load width). */
typedef struct PadRecordButtons {
    u8 unk_00[0x28];
    union {
        u64 allButtons;
        u16 buttonHalves[4];
    } buttons;
} PadRecordButtons;

extern s32 D_00550DB4;
extern s32 D_00550DB8;
extern PadRecordButtons PadData;
extern const char D_0053B040[];
extern const char D_0053B068[];
extern const char D_0053B0B0[];
extern u8 D_005A8080[];
s32 xglSleep();
void xglRenderClearDepth(void);
void xglFontPrint(s32 x, s32 y, s32 color, const u8 *text);
void xglFontPrintDirect(const u8 *text);

/* Test loop of the cat pictures (_prog/iwase): clears the screen, then streams cat.jpg and cat2.jpg while it types
 * the test text (D_0053B040 or D_0053B068) into D_005A8080, until the buttons of mask 0x08000100 are pressed. */
void narrEvCatPictureTest(void) {
    XglPacket *packet;
    s32 frameCounter;
    s32 i;
    char *sourceText;
    char *visibleText;
    char *defaultText;
    char *alternateText;
    u64 depthBits;

    defaultText = D_0053B040;
    alternateText = D_0053B068;
    depthBits = (u64) -1 << 40 >> 8;
    frameCounter = 0;
    xglPacketGetCurrent();
    packet = xglPacketGetCurrent();
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_FRAME_1, gRenderState.display_buffer_base |
                             ((0xFF000000ULL << 16 | 8) << 16));
    gifStateAddReg(1, (u64) 0xFE00 << 46);
    gifStateAddReg(GS_PRIM, GS_PRIM_SPRITE);
    gifStateAddReg(GS_XYZF2, ((D_00550D98 * 16) - 8) | (((u64) ((D_00550D9C * 16) - 9)) << 16) | depthBits);
    gifStateAddReg(GS_XYZF2, ((D_00550D98 * 16) + 0x1FF8) | (((u64) ((D_00550D9C * 16) + 0x1BF7)) << 16) | depthBits);
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);
    xglSleep();
    xglRenderClearDepth();
    xglCdReadJobReset();
    xglCdReadFileBlocking((s32) "_prog/iwase/cat.jpg");
    if ((PadData.buttons.allButtons & 0x08000100) != 0x08000100) {
        do {
            if (frameCounter == 60) {
                xglCdReadFileBlocking((s32) "_prog/iwase/cat2.jpg");
            } else if (frameCounter == 0) {
                xglCdReadFileBlocking((s32) "_prog/iwase/cat.jpg");
            }
            frameCounter += 1;
            if (frameCounter >= 121) {
                frameCounter = 0;
            }
            if (PadData.buttons.buttonHalves[1] & 0x10) {
                xglCdReadFileBlocking((s32) "_prog/iwase/cat.jpg");
            }
            if (PadData.buttons.buttonHalves[1] & 0x40) {
                xglCdReadFileBlocking((s32) "_prog/iwase/cat2.jpg");
            }
            visibleText = D_005A8080;
            xglCdReadJobDecodeNext();
            xglCdFlushStatus();
            sourceText = alternateText;
            if (D_00550DB8 == 0) {
                sourceText = defaultText;
            }
            memset(visibleText, 0, 256);
            for (i = 0; i < D_00550DB4; i++) {
                if (*sourceText == 0) {
                    *visibleText = *sourceText;
                    break;
                }
                if (*sourceText == '\n') {
                    *visibleText++ = *sourceText++;
                } else {
                    *visibleText++ = *sourceText++;
                    *visibleText++ = *sourceText++;
                }
            }
            xglFontPrint(20, 30, 0xFFFFFF, D_0053B0B0);
            xglFontPrintDirect(D_005A8080);
            D_00550DB4++;
            if (D_00550DB4 >= 64) {
                D_00550DB4 = 0;
                if (D_00550DB8 != 0) {
                    D_00550DB8 = 0;
                } else {
                    D_00550DB8 = 1;
                }
            }
            xglSleep();
        } while ((PadData.buttons.allButtons & 0x08000100) != 0x08000100);
    }
}

const char D_0053B040[40] = {
    186, 199, 182, 175, 164, 206, 192, 239,
    187, 206, 164, 203, 161, 162, 10, 186,
    199, 182, 175, 164, 206, 165, 181, 165,
    208, 165, 164, 165, 208, 165, 235, 165,
    213, 161, 188, 165, 201, 161, 163, 0
};
const char D_0053B068[72] = {
    165, 171, 165, 237, 165, 234, 161, 188,
    165, 225, 165, 164, 165, 200, 10, 192,
    184, 164, 173, 200, 180, 164, 173, 164,
    191, 164, 164, 164, 202, 164, 233, 161,
    162, 191, 169, 164, 217, 164, 235, 164,
    179, 164, 200, 164, 192, 161, 163, 10,
    10, 165, 202, 165, 236, 164, 206, 165,
    198, 165, 185, 165, 200, 164, 199, 164,
    185, 161, 163, 0, 0, 0, 0, 0
};
const char D_0053B0B0[48] = "$bg3;$prop1;$han1;$xx0,36;$xx1,8;$xx2,0;";

/* Returns the number of CR LF line ends in the zero-terminated script text. */
s32 narrEvCountLineEnds(s8 *text) {
    s32 lineEndCount;

    lineEndCount = 0;
    while (*text != 0) {
        if (*text == 13) {
            if (text[1] == 10) {
                lineEndCount++;
            }
        }
        text++;
    }
    return lineEndCount;
}

extern const u8 D_00550DC0[];

typedef struct EnvelopeRecordView {
    AlignedVector4 head;
    u8 unmodeled_10[0x20];
    s32 unk_30;
    s32 unk_34;
    f32 pictureWidth;               /* 512: the picture size narrEvDrawEnvelopeSprite draws */
    f32 pictureHeight;              /* 448 */
    s32 time;
    f32 rate;
    s32 level;
    u8 unk_4C;
    u8 unmodeled_4D[0x43];
} EnvelopeRecordView;

typedef struct EnvelopeTableView {
    s32 activeSteps;
    u8 unmodeled_04[0xC];
    EnvelopeRecordView envelopeRecords[4];
} EnvelopeTableView;

/* Resets the envelope table: four records at 512x448 with rate 25.6 and level 0. */
void narrEvResetEnvelopeTable(EnvelopeTableView *envelopeTable) {
    s32 recordNumber;
    f32 pictureWidth = 512.0f;
    f32 pictureHeight = 448.0f;
    f32 defaultRate = 25.6f;

    envelopeTable->activeSteps = 0;
    for (recordNumber = 0; recordNumber < 4; recordNumber++) {
        envelopeTable->envelopeRecords[recordNumber].head.x = 0.0f;
        envelopeTable->envelopeRecords[recordNumber].unk_34 = 0;
        envelopeTable->envelopeRecords[recordNumber].pictureWidth = pictureWidth;
        envelopeTable->envelopeRecords[recordNumber].pictureHeight = pictureHeight;
        envelopeTable->envelopeRecords[recordNumber].unk_30 = 0;
        envelopeTable->envelopeRecords[recordNumber].rate = defaultRate;
        envelopeTable->envelopeRecords[recordNumber].level = 0;
        envelopeTable->envelopeRecords[recordNumber].time = 0;
        envelopeTable->envelopeRecords[recordNumber].unk_4C = D_00550DC0[0];
    }
}

typedef struct NarrEvEntryTableView {
    s32 elapsedTicks; /* 0x00, advanced by narrEvIncrementCounter */
    s32 unk_04;
    s32 narrEntryCount;           /* 0x08 */
    unsigned char *narrEntryBase; /* 0x0C */
    unsigned char unk_10[4];
    unsigned char unk_14; /* 0x14, reset from D_00550DC0 */
    unsigned char unk_15[0x4B];
    s32 timer; /* 0x60 */
    unsigned char unk_64[4];
    s32 unk_68;
    s32 elapsedPaused; /* 0x6C */
    s32 unk_70;        /* 0x70 */
} NarrEvEntryTableView;

/* Resets the NarrEv entry table to its start state (the mode comes from D_00550DC0). */
void narrEvResetEntryTable(NarrEvEntryTableView *narrTable) {
    narrTable->elapsedTicks = 0;
    narrTable->unk_04 = 0;
    narrTable->narrEntryCount = 0;
    narrTable->narrEntryBase = 0;
    narrTable->timer = 0;
    narrTable->unk_68 = 0;
    narrTable->elapsedPaused = 0;
    narrTable->unk_70 = 0;
    narrTable->unk_14 = D_00550DC0[0];
}

extern u64 D_003EBC70[];
extern u64 D_003EBCD0[];
extern u64 D_003EBCF0[];

/* Uploads one picture: a BITBLT packet that copies the pixel block behind pictureHeader (its offset is the u16 at +0x1C)
 * into the work page, then sets the GS state for drawing it. */
void narrEvUploadPicture(u8 *pictureHeader) {
    XglPacket *packet;
    u8 *pictureData;
    u64 *bitbltPacket;

    bitbltPacket = D_003EBC70;
    pictureData = pictureHeader + 16;
    pictureData += *(u16 *) (pictureData + 12);
    packet = xglPacketGetCurrent();
    bitbltPacket[4] = ((u64) gRenderState.work_buffer_base << 37) | (0x8000ULL << 36);
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddDataN(packet, bitbltPacket, 0x18);
    sceVif1PkRef(packet, D_003EBCD0, 2, 0, 0, 0);
    sceVif1PkRef(packet, pictureData, 0x7000, 0, 0x51007000, 0);
    sceVif1PkRef(packet, D_003EBCF0, 3, 0, 0, 0);
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_TEXFLUSH, 0);
    gifStateAddReg(GS_ZBUF_1, GS_SET_ZBUF(0, GS_PSMZ24, 1));
    gifStateAddReg(GS_CLAMP_1, GS_SET_CLAMP(1, 1, 0, 0, 0, 0));
    gifStateAddReg(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
    gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(0, 1, 0, 1, 0x80));
    gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(gRenderState.display_buffer_base, 8,
                                            GS_PSMCT32, 0xFF000000));
    gifStateAddReg(GS_TEX0_1, GS_SET_TEX0(gRenderState.work_buffer_base << 5, 8,
                                          GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
    gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, 0x3F800000));
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
    gifStateAddReg(GS_UV, GS_SET_UV(0, 0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ((D_00550D98 << 4) + 72, (D_00550D9C << 4) + 5735, 0xFFFFFF));
    gifStateAddReg(GS_UV, GS_SET_UV(0x880, 0x5A0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ((D_00550D98 << 4) + 2248, (D_00550D9C << 4) + 7175, 0xFFFFFF));
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);
}

int func_003098D0(f32 value);
extern u64 D_003EBD20[];
extern u64 D_003EBD80[];
extern u64 D_003EBDA0[];

typedef struct SpriteRect {
    u8 unk_00[0x30];
    f32 offsetX0; /* +0x30 */
    f32 offsetY0; /* +0x34 */
    f32 offsetX1; /* +0x38 */
    f32 offsetY1; /* +0x3C */
    u8 unk_40[8];
    f32 alpha; /* +0x48 */
} SpriteRect;

void narrEvDrawEnvelopeSprite(s32 id, void *payload);

/* Draws one envelope sprite: the two pages at imageAddress as a BITBLT, then the textured sprite whose corners
 * come from the SpriteRect payload (offsets at +0x30..+0x3C, alpha at +0x48). */
void narrEvDrawEnvelopeSprite(s32 imageAddress, void *spriteRecord) {
    XglPacket *packet;
    u8 *imageData;
    SpriteRect *sprite;
    u64 *bitbltPacket;
    u64 xyCoords;
    s32 workPageBase;

    imageData = (u8 *) imageAddress;
    sprite = spriteRecord;
    bitbltPacket = D_003EBD20;
    packet = xglPacketGetCurrent();
    bitbltPacket[4] = ((u64) gRenderState.work_buffer_base << 37) | (0x8000ULL << 36);
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddDataN(packet, bitbltPacket, 0x18);
    sceVif1PkRef(packet, D_003EBD80, 2, 0, 0, 0);
    sceVif1PkRef(packet, imageData, 0x7000, 0, 0x51007000, 0);
    sceVif1PkRef(packet, D_003EBD80, 2, 0, 0, 0);
    sceVif1PkRef(packet, imageData + 0x70000, 0x7000, 0, 0x51007000, 0);
    sceVif1PkRef(packet, D_003EBDA0, 3, 0, 0, 0);
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_TEXFLUSH, 0);
    gifStateAddReg(GS_ZBUF_1, GS_SET_ZBUF(0, GS_PSMZ24, 1));
    gifStateAddReg(GS_CLAMP_1, GS_SET_CLAMP(1, 1, 0, 0, 0, 0));
    gifStateAddReg(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
    gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(0, 1, 2, 1, (s32) sprite->alpha));
    gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(gRenderState.display_buffer_base, 8,
                                            GS_PSMCT32, 0xFF000000));
    workPageBase = gRenderState.work_buffer_base;
    gifStateAddReg(GS_TEX0_1, GS_SET_TEX0(workPageBase << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
    gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, 0x3F800000));
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
    gifStateAddReg(GS_UV, GS_SET_UV(0, 0));
    xyCoords = func_003098D0(((f32) D_00550D98 + sprite->offsetX0) * 16.0f - 8.0f);
    xyCoords |= (u64) func_003098D0(((f32) D_00550D9C + sprite->offsetY0) * 16.0f - 9.0f) << 16;
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(0, 0, 0xFFFFFF) | xyCoords);
    gifStateAddReg(GS_UV, GS_SET_UV(0x2000, 0x1C00));
    xyCoords = func_003098D0(((f32) D_00550D98 + sprite->offsetX1) * 16.0f - 8.0f);
    xyCoords |= (u64) func_003098D0(((f32) D_00550D9C + sprite->offsetY1) * 16.0f - 9.0f) << 16;
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(0, 0, 0xFFFFFF) | xyCoords);
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);
}

/* One 0x90-byte envelope record. The view starts at the timing words (+0x40 of the record), so the record's own
 * payload and id live at the tail of the previous view. */
typedef struct EnvelopeEntry {
    s32 time;
    f32 rate;
    f32 level;
    u8 unk_0C[0x68];
    s32 id;
    u8 unk_78[0x18];
} EnvelopeEntry;

typedef struct EnvelopePayload {
    u8 envelopeBytes[0x90];
} EnvelopePayload;

/* Count-prefixed table: the same records seen from the payload start (+0x10) and from the timing words (+0x50). */
typedef struct EnvelopeTable {
    u32 activeSteps;
    u8 unk_04[0xC];
    union {
        EnvelopePayload payloads[1];
        struct {
            u8 unk_00[0x40];
            EnvelopeEntry envelopeEntries[1];
        } timing;
    } envelopeSteps;
} EnvelopeTable;

void xglEnvelopeTableUpdate(EnvelopeTable *envelopeTable, s32 time) {
    f32 level;
    u32 envelopeStepIndex;
    EnvelopeEntry *envelopeEntry;
    EnvelopePayload *envelopePayload;

    envelopeStepIndex = 0;
    if (envelopeTable->activeSteps != 0) {
        envelopePayload = envelopeTable->envelopeSteps.payloads;
        envelopeEntry = envelopeTable->envelopeSteps.timing.envelopeEntries;
        do {
            if (time >= envelopeEntry->time) {
                narrEvDrawEnvelopeSprite(envelopeEntry[-1].id, envelopePayload);
                level = envelopeEntry->level + envelopeEntry->rate;
                envelopeEntry->level = level;
                if (level > 128.0f) {
                    envelopeEntry->level = 128.0f;
                }
            }
            envelopeStepIndex += 1;
            envelopePayload += 1;
            envelopeEntry += 1;
        } while (envelopeStepIndex < envelopeTable->activeSteps);
    }
}

void func_001B6798(int handle);

typedef struct NarrEvSlot {
    int primaryHandle;                       /* +0 */
    s32 decodeRequest[8];                    /* 4..35: decode buffer, words 0 and 1 written after the memset */
    int secondaryHandle;                     /* +36 */
    unsigned char unmodeled_28[32];   /* 40..71 */
    int unk_48;                       /* +72 */
    unsigned char unmodeled_4C[68];   /* 76..143 */
} NarrEvSlot;

#define NARR_EV_SLOT_COUNT 4

typedef struct NarrEvState {
    unsigned char unmodeled_00[16];   /* 0..15 */
    NarrEvSlot decodeSlots[NARR_EV_SLOT_COUNT];  /* 16..591 */
} NarrEvState;

/* Frees both handles of every occupied decode slot of the state and clears the slot. */
void narrEvReleaseSlots(NarrEvState *narrState) {
    int i;

    for (i = 0; i < NARR_EV_SLOT_COUNT; i++) {
        NarrEvSlot *decodeSlot = &narrState->decodeSlots[i];
        if (decodeSlot->primaryHandle != 0) {
            func_001B6798(decodeSlot->primaryHandle);
            decodeSlot->primaryHandle = 0;
            func_001B6798(decodeSlot->secondaryHandle);
            decodeSlot->secondaryHandle = 0;
            decodeSlot->unk_48 = 0;
        }
    }
}

void assertFailed(const char *message, const char *file, int line);
s32 func_001B6520(s32);
s32 xglCdGetFileSize(s32);
s32 xglCdReadFile(s32, s32, s32, s32);

/* Allocates a buffer for the file (rounded up to 2 KiB), stores its address in *outLoadedAddr and reads the file in. */
void narrEvLoadFile(s32 *outLoadedAddr, s32 fileHandle) {
    s32 loadedAddr;
    s32 fileSize;

    fileSize = xglCdGetFileSize(fileHandle);
    loadedAddr = func_001B6520((fileSize + 0x800) & ~0x7FF);
    *outLoadedAddr = loadedAddr;
    if (loadedAddr == 0) {
        assertFailed("(*ppLogoImage)", "NarrEv.euc.c", 0x260);
    }
    xglCdReadFile(fileHandle, *outLoadedAddr, 0, 0);
}

/* Builds the two-word decode request of an occupied slot and runs the JPEG decoder on it. */
void narrEvDecodeSlot(NarrEvSlot *decodeSlot) {
    s32 secondaryHandle;
    s32 primaryHandle;

    if (decodeSlot->primaryHandle != 0) {
        memset(decodeSlot->decodeRequest, 0, 0x20);
        primaryHandle = decodeSlot->primaryHandle;
        secondaryHandle = decodeSlot->secondaryHandle;
        decodeSlot->decodeRequest[0] = primaryHandle;
        decodeSlot->decodeRequest[1] = secondaryHandle;
        xglJpegDecode(decodeSlot->decodeRequest);
    }
}

typedef struct NarrEvSlotView {
    int primaryHandle;              /* +0 */
    s32 decodeRequest[8];           /* 4..35: decode buffer, words 0 and 1 written after the memset */
    int secondaryHandle;            /* +36 */
    unsigned char unmodeled_28[32]; /* 40..71 */
    int unk_48;                     /* +72 */
    char fileName[68];              /* 76..143 */
} NarrEvSlotView;

typedef struct NarrEvStateView {
    s32 decodeSlotCount;                               /* 0 */
    unsigned char unmodeled_04[12];                    /* 4..15 */
    NarrEvSlotView decodeSlots[NARR_EV_SLOT_COUNT]; /* 16..591 */
} NarrEvStateView;

s32 strcmp();

/* Decodes every slot whose file name passes the test against D_00550DC0. */
void narrEvDecodeSlots(NarrEvStateView *narrState) {
    unsigned int slotNumber;

    for (slotNumber = 0; slotNumber < narrState->decodeSlotCount; slotNumber++) {
        if (strcmp(narrState->decodeSlots[slotNumber].fileName, D_00550DC0) != 0) {
            narrEvDecodeSlot(&narrState->decodeSlots[slotNumber]);
        }
    }
}

/* Frees the slot's handles, allocates the picture and decode buffers and reads the slot's file into the picture. */
void narrEvLoadSlotFile(NarrEvSlotView *decodeSlot) {
    s32 fileSize;

    if (decodeSlot->primaryHandle != 0) {
        func_001B6798(decodeSlot->primaryHandle);
        func_001B6798(decodeSlot->secondaryHandle);
        decodeSlot->primaryHandle = 0;
        decodeSlot->secondaryHandle = 0;
    }
    fileSize = xglCdGetFileSize(decodeSlot->fileName);
    decodeSlot->primaryHandle = func_001B6520((fileSize + 0x800) & ~0x7FF);
    decodeSlot->secondaryHandle = func_001B6520(0xE0000);
    if (decodeSlot->primaryHandle == 0 || decodeSlot->secondaryHandle == 0) {
        assertFailed("pData->pImage!=NULL && pData->pDecodeBuf!=NULL", "NarrEv.euc.c", 662);
    }
    xglCdReadFile(decodeSlot->fileName, decodeSlot->primaryHandle, 1, 0);
}

/* Loads every slot whose file name passes the test against D_00550DC0 (narrEvLoadSlotFile per slot). */
void narrEvLoadMatchingSlots(NarrEvStateView *narrState) {
    unsigned int slotNumber;

    for (slotNumber = 0; slotNumber < narrState->decodeSlotCount; slotNumber++) {
        if (strcmp(narrState->decodeSlots[slotNumber].fileName, D_00550DC0) != 0) {
            narrEvLoadSlotFile(&narrState->decodeSlots[slotNumber]);
        }
    }
}

typedef struct NarrEvEntryTableAlt {
    s32 elapsedTicks;             /* 0x00, advanced by narrEvIncrementCounter */
    s32 narrEntryLimit;           /* 0x04 */
    s32 narrEntryCount;           /* 0x08 */
    unsigned char *narrEntryBase; /* 0x0C */
    unsigned char unk_10[0x5C];
    s32 elapsedPaused; /* 0x6C */
} NarrEvEntryTableAlt;

void narrEvReleaseSlots(NarrEvState *narrState);

/* Moves to the next entry (wrapping at narrEntryLimit), frees its slots and loads its file names. */
void narrEvAdvanceEntry(NarrEvEntryTableAlt *narrTable) {
    unsigned int nextEntryNumber;

    nextEntryNumber = narrTable->narrEntryCount + 1;
    if (nextEntryNumber >= narrTable->narrEntryLimit) {
        nextEntryNumber = 0;
    }
    narrEvReleaseSlots(narrTable->narrEntryBase + nextEntryNumber * NARR_EV_ENTRY_SIZE);
    narrEvLoadMatchingSlots(narrTable->narrEntryBase + nextEntryNumber * NARR_EV_ENTRY_SIZE);
}

extern s32 gNarrEvActionCode[];
extern const char gNarrEvMatchName[];
extern s32 gNarrEvMatchFound;

/* Table read by narrEvStartOnNameMatch and narrEvIncrementCounter: entries of 0x290 bytes start at narrEntryBase. */
typedef struct NarrEvEntryTable {
    s32 elapsedTicks;                    /* 0x00, advanced by narrEvIncrementCounter */
    unsigned char unk_04[4];
    s32 narrEntryCount;                 /* 0x08 */
    unsigned char *narrEntryBase;       /* 0x0C */
    unsigned char unk_10[0x5C];
    s32 elapsedPaused;                       /* 0x6C */
} NarrEvEntryTable;

void narrEvStartOnNameMatch(NarrEvEntryTable *narrTable) {
    if (strcmp(narrTable->narrEntryBase + (narrTable->narrEntryCount * NARR_EV_ENTRY_SIZE) + 0x250, gNarrEvMatchName) != 0) {
        gNarrEvActionCode[0] = 0x3C;
        gNarrEvMatchFound = 1;
    }
}

void narrEvIncrementCounter(NarrEvEntryTable *narrTable) {
    if (narrTable->elapsedPaused == 0) {
        narrTable->elapsedTicks += 1;
    }
}

s32 xglCdGetServicePending();
void narrEvStartOnNameMatch(NarrEvEntryTable *narrTable);

/* Advances to the next entry once the CD is idle, starts its entry and returns 1 when the table is used up. */
s32 narrEvAdvanceWhenIdle(NarrEvEntryTableAlt *narrTable) {
    unsigned int nextEntryNumber;

    while (xglCdGetServicePending() != 0) {
    }
    narrEvReleaseSlots(narrTable->narrEntryBase + narrTable->narrEntryCount * NARR_EV_ENTRY_SIZE);
    nextEntryNumber = narrTable->narrEntryCount + 1;
    narrTable->narrEntryCount = nextEntryNumber;
    if (nextEntryNumber >= narrTable->narrEntryLimit) {
        return 1;
    }
    narrEvDecodeSlots(narrTable->narrEntryBase + nextEntryNumber * NARR_EV_ENTRY_SIZE);
    narrEvStartOnNameMatch(narrTable);
    narrEvAdvanceEntry(narrTable);
    narrTable->elapsedTicks = 0;
    gNarrEvMatchFound = 1;
    return 0;
}

unsigned int strlen(const char *text);

/* Returns 0 when text starts with pattern (a line starting with # is skipped), else 1. */
s32 narrEvTextStartsWith(const char *text, const char *pattern) {
    s32 patternLength;
    s32 charIndex;

    patternLength = strlen(pattern);
    for (charIndex = 0; charIndex < patternLength; charIndex++) {
        if (text[charIndex] == ' ' || text[charIndex] == '\t') {
            if (text[0] == '#') {
                return 0;
            }
        }
        if (text[charIndex] != pattern[charIndex]) {
            return 1;
        }
    }
    return 0;
}

/* Finds the next line that starts with pattern within limit bytes and returns it, or 0. */
const char *narrEvFindText(const char *text, s32 limit, const char *pattern) {
    s32 scanCount;

    scanCount = 0;
    do {
        while (*text != *pattern) {
            scanCount++;
            text++;
            if (scanCount >= limit) {
                return 0;
            }
        }
        if (narrEvTextStartsWith(text, pattern) == 0) {
            return text;
        }
        scanCount++;
        text++;
    } while (scanCount < limit);
    return 0;
}

/* Returns the start of the line after the CR LF that ends text. */
const char *narrEvSkipLine(const char *text) {
    while (*text != 13 || text[1] != 10) {
        text++;
    }
    return text + 2;
}

s32 func_0030E250(const char *text, const char *format, s32 *value);
void func_0030E3D0(char *dest, const void *source);
int xglFontGetStringWidth(const char *text);
int printf(const char *format, ...);

const char gNarrEvMatchName[192] = {
    78, 85, 76, 76, 0, 0, 0, 0,
    99, 97, 110, 39, 116, 32, 102, 105,
    110, 100, 32, 115, 116, 114, 32, 37,
    115, 10, 0, 0, 0, 0, 0, 0,
    35, 77, 65, 88, 67, 85, 84, 32,
    37, 100, 10, 0, 0, 0, 0, 0,
    35, 77, 69, 83, 70, 73, 76, 69,
    32, 37, 115, 10, 0, 0, 0, 0,
    35, 77, 69, 83, 83, 65, 71, 69,
    32, 37, 100, 10, 0, 0, 0, 0,
    35, 67, 85, 84, 32, 37, 100, 10,
    0, 0, 0, 0, 0, 0, 0, 0,
    35, 65, 68, 88, 32, 37, 115, 10,
    0, 0, 0, 0, 0, 0, 0, 0,
    35, 73, 77, 71, 95, 82, 69, 65,
    68, 32, 37, 100, 32, 37, 115, 10,
    0, 0, 0, 0, 0, 0, 0, 0,
    35, 73, 77, 71, 95, 70, 82, 65,
    77, 69, 32, 37, 100, 32, 37, 100,
    32, 37, 100, 32, 37, 100, 32, 37,
    100, 10, 0, 0, 0, 0, 0, 0,
    35, 73, 77, 71, 95, 65, 78, 73,
    77, 32, 37, 100, 32, 37, 100, 10,
    0, 0, 0, 0, 0, 0, 0, 0
};

/* Appends the script text entry at text to dest: each CR LF line becomes a width-centred header block followed by its
 * text; FRAME:n$ commands are printed and skipped. */
void narrEvCopyEntryText(char *entryText, char *text) {
    char *lineBuffer;
    char lineHeader[4];
    s32 frameNumber;
    s32 lineWidth;
    char ch;
    char nextCh;

    xglFontGetStringWidth(
        "$bg0;$prop1;$han1;$xx0,24;$xx1,8;$xx2,0;$xx6,1;$alpha0,2,0,2,0,0x80;$shadow+1,+1,01000000;");
    *entryText = D_00550DC0[0];
    lineBuffer = &SEF_SCRATCHPAD(char, 0x0);
    while (*text != 0 && *text != '#') {
        if (*text == 'F' && text[1] == 'R' && text[2] == 'A' && text[3] == 'M' && text[4] == 'E') {
            func_0030E250(text, "FRAME:%d$", &frameNumber);
            printf("frame %d\n", frameNumber);
            if (*text != '$') {
                do {
                    text++;
                } while (*text != '$');
                text++;
            } else {
                text++;
            }
        } else {
            ch = *text;
            if (ch == '\r' && text[1] == '\n') {
                nextCh = text[1];
                *lineBuffer = 0;
                lineBuffer = &SEF_SCRATCHPAD(char, 0x0);
                text += 2;
                lineWidth = xglFontGetStringWidth(lineBuffer);
                if (lineWidth >= 501) {
                    lineWidth = 500;
                }
                lineHeader[0] = 1;
                lineHeader[2] = ch;
                lineHeader[3] = nextCh;
                lineHeader[1] = ~(lineWidth / 2);
                func_0030E3D0(entryText, lineHeader);
                func_0030E3D0(entryText, lineBuffer);
            } else {
                *lineBuffer++ = *text;
                text++;
            }
        }
    }
}

extern char *D_00550DD0;

typedef struct NarrEvLoad {
    unsigned char unmodeled_00[16]; /* 0..15 */
    char *fileBuffer;               /* +16 */
    unsigned char unmodeled_14[64]; /* 20..83 */
    s32 loadedEntryCount;           /* +84 */
    char *entryTextBlock;           /* +88 */
} NarrEvLoad;

/* Reads the script file, counts its entries and builds one 0x7EC-byte text block per entry (narrEvCopyEntryText). */
s32 narrEvLoadEntryText(s32 fileHandle, char *fileBuffer, NarrEvLoad *narrLoad) {
    char *cursor;
    s32 fileBytes;
    s32 entriesFound;
    s32 entryNumber;

    fileBytes = xglCdReadFile(fileHandle, (s32) fileBuffer, 0, 0);
    cursor = fileBuffer;
    narrLoad->loadedEntryCount = 0;
    narrLoad->fileBuffer = cursor;
    entriesFound = 0;
    while ((cursor = ((char *(*) (char *text, s32 remaining, s32 limit)) narrEvFindText)(
                cursor, fileBytes - (cursor - fileBuffer), D_00550DD0)) != 0) {
        cursor = narrEvSkipLine(cursor);
        entriesFound++;
    }
    if (entriesFound == 0) {
        return 0;
    }
    narrLoad->loadedEntryCount = entriesFound;
    {
        s32 *loadedCountPtr = &narrLoad->loadedEntryCount;

        narrLoad->entryTextBlock = (char *) func_001B6520(*loadedCountPtr * 2028);
    }
    if (narrLoad->entryTextBlock == 0) {
        assertFailed("pInfo->pMes", "NarrEv.euc.c", 0x37E);
    }
    cursor = fileBuffer;
    entryNumber = 0;
    while ((cursor = ((char *(*) (char *text, s32 remaining, s32 limit)) narrEvFindText)(
                cursor, fileBytes - (cursor - fileBuffer), D_00550DD0)) != 0) {
        cursor = narrEvSkipLine(cursor);
        narrEvCopyEntryText(narrLoad->entryTextBlock + entryNumber * 2028, cursor);
        entryNumber++;
    }
    return 1;
}

typedef struct NarrHudState {
    s32 time;                         /* +0 */
    unsigned int scriptEntryCount;    /* +4 */
    s32 unk_08;                 /* +8 */
    char *narrEntryBase;               /* +12 */
    unsigned char unmodeled_10[0x48]; /* 16..87 */
    char *textSource;                 /* +88 */
    char *textDest;                   /* +92 */
    s32 scrollTicks;                  /* +96 */
    unsigned char unmodeled_64[8];    /* 100..107 */
    s32 pausedFlag;                   /* +108 */
    void *pendingCall;                /* +112 */
} NarrHudState;

/* The NarrHudState view of the episode table gNarrEvEntryTable (kept apart from the entry-table views). */
#define gNarrHud (*(NarrHudState *) gNarrEvEntryTable)

extern unsigned char gNarrEvEntryTable[];

typedef struct NarrEvLabel9 {
    unsigned char labelBytes[9];
} NarrEvLabel9;

extern char *D_00550DC8;
extern char *D_00550DCC;
extern char *D_00550DD4;
extern char *D_00550DD8;
extern char *D_00550DDC;
extern char *D_00550DE0;
extern char *D_00550DE4;

const u8 D_00550DC0[] = "";
char *D_00550DC4 = (char *) gNarrEvMatchName + 8;
char *D_00550DC8 = (char *) gNarrEvMatchName + 32;
char *D_00550DCC = (char *) gNarrEvMatchName + 48;
char *D_00550DD0 = (char *) gNarrEvMatchName + 64;
char *D_00550DD4 = (char *) gNarrEvMatchName + 80;
char *D_00550DD8 = (char *) gNarrEvMatchName + 96;
char *D_00550DDC = (char *) gNarrEvMatchName + 112;
char *D_00550DE0 = (char *) gNarrEvMatchName + 136;
char *D_00550DE4 = (char *) gNarrEvMatchName + 168;

/* Parses the episode script of the file: the entry count and the per-entry lines (labels, positions, scales and
 * envelope), reading into gNarrEvEntryTable and the entry blocks. Returns 0 when the file is malformed. */
s32 narrEvParseScript(s32 fileHandle, char *fileBuffer) {
    char textBuf[64];
    s32 entryNumber;
    s32 labelSlot;
    s32 positionSlot;
    s32 positionUnk;
    s32 originX;
    s32 originY;
    s32 scaleSource;
    s32 scaleSlot;
    s32 scaleValue;
    const char *cursor;
    const char *matchPos;
    unsigned char *hudRecord;
    s32 fileSize;
    s32 entriesDone;
    s32 recordOffset;
    float fractionDivisor;

    fileSize = xglCdReadFile(fileHandle, fileBuffer, 0, 0);
    if (fileSize == 0) {
        return 0;
    }
    cursor = narrEvFindText(fileBuffer, fileSize, D_00550DC8);
    if (cursor == 0) {
        assertFailed("0", "NarrEv.euc.c", 927);
    }
    func_0030E250(cursor, D_00550DC8, &gNarrHud.scriptEntryCount);
    cursor = narrEvFindText(fileBuffer, fileSize, D_00550DCC);
    if (cursor == 0) {
        return 0;
    }
    func_0030E250(cursor, D_00550DCC, gNarrEvEntryTable + 20);
    gNarrHud.narrEntryBase =
        (char *) func_001B6520(gNarrHud.scriptEntryCount * NARR_EV_ENTRY_SIZE);
    if (gNarrHud.narrEntryBase == 0) {
        assertFailed("g_sEpisode.pCut!=NULL", "NarrEv.euc.c", 944);
    }
    entriesDone = 0;
    if (gNarrHud.scriptEntryCount != 0) {
        recordOffset = 0;
        do {
            entriesDone++;
            narrEvResetEnvelopeTable((unsigned char *) gNarrHud.narrEntryBase + recordOffset);
            recordOffset += 0x290;
        } while (entriesDone < gNarrHud.scriptEntryCount);
    }
    cursor = fileBuffer;
    entriesDone = 0;
    if (gNarrHud.scriptEntryCount != 0) {
        do {
            if (!((cursor - fileBuffer) < fileSize)) {
                return 0;
            }
            matchPos = narrEvFindText(cursor, fileSize - (cursor - fileBuffer), D_00550DD4);
            if (matchPos == 0) {
                return 0;
            }
            func_0030E250(matchPos, D_00550DD4, &entryNumber);
            cursor = narrEvSkipLine(matchPos);
            if (!(entryNumber < gNarrHud.scriptEntryCount)) {
                return 0;
            }
            hudRecord = (unsigned char *) gNarrHud.narrEntryBase + entryNumber * NARR_EV_ENTRY_SIZE;
            matchPos = narrEvFindText(cursor, fileSize - (cursor - fileBuffer), D_00550DD8);
            if (matchPos == 0) {
                return 0;
            }
            func_0030E250(matchPos, D_00550DD8, hudRecord + 592);
            fractionDivisor = 100.0f;
            cursor = narrEvSkipLine(matchPos);
            /* kept: the HUD record is addressed as bytes; the label, position and scale slots are 144-byte
             * strides with fields at +64..+92, which the struct view would not reproduce in the same codegen. */
            do {
                if (narrEvTextStartsWith(cursor, D_00550DDC) == 0) {
                    memset(textBuf, 0, 64);
                    ((s32 (*)()) func_0030E250)(cursor, D_00550DDC, &labelSlot, textBuf);
                    if (!(labelSlot < 4)) {
                        return 0;
                    }
                    *(NarrEvLabel9 *) (hudRecord + labelSlot * 144 + 92) = *(NarrEvLabel9 *) "mnu/pse/";
                    ((s32 (*)()) func_0030E3D0)(hudRecord + labelSlot * 144 + 92, textBuf);
                    *(s32 *) hudRecord = *(s32 *) hudRecord + 1;
                    cursor = narrEvSkipLine(cursor);
                } else if (narrEvTextStartsWith(cursor, D_00550DE0) == 0) {
                    ((s32 (*)()) func_0030E250)(cursor, D_00550DE0, &positionSlot, &positionUnk, &originX, &originY, &scaleSource);
                    if (!(positionSlot < 4)) {
                        return 0;
                    }
                    *(s32 *) (hudRecord + positionSlot * 144 + 80) = positionUnk;
                    *(float *) (hudRecord + positionSlot * 144 + 64) = (float) originX;
                    *(float *) (hudRecord + positionSlot * 144 + 68) = (float) originY;
                    *(float *) (hudRecord + positionSlot * 144 + 72) = ((float) scaleSource / fractionDivisor) * 512.0f + (float) originX;
                    *(float *) (hudRecord + positionSlot * 144 + 76) = ((float) scaleSource / fractionDivisor) * 448.0f + (float) originY;
                    cursor = narrEvSkipLine(cursor);
                } else if (narrEvTextStartsWith(cursor, D_00550DE4) == 0) {
                    ((s32 (*)()) func_0030E250)(cursor, D_00550DE4, &scaleSlot, &scaleValue);
                    if (!(scaleSlot < 4)) {
                        return 0;
                    }
                    *(float *) (hudRecord + scaleSlot * 144 + 84) = 128.0f / (float) scaleValue;
                    cursor = narrEvSkipLine(cursor);
                } else {
                    if (narrEvTextStartsWith(cursor, D_00550DD4) == 0) {
                        break;
                    }
                    cursor++;
                }
            } while ((cursor - fileBuffer) < fileSize);
            entriesDone++;
        } while (entriesDone < gNarrHud.scriptEntryCount);
    }
    return 1;
}

typedef struct NarrEvEntryTableLocal {
    s32 elapsedTicks; /* 0x00, advanced by narrEvIncrementCounter */
    unsigned char unk_04[4];
    s32 narrEntryCount;           /* 0x08 */
    unsigned char *narrEntryBase; /* 0x0C */
    unsigned char unk_10[0x48];
    unsigned char *entryTextBlock; /* 0x58, stride 0x7EC per entry */
    char *textBuffer;          /* 0x5C */
    s32 timer;                 /* 0x60 */
    unsigned char unk_64[8];
    s32 elapsedPaused;    /* 0x6C */
    unsigned char *unk70; /* 0x70: image passed to narrEvUploadPicture */
} NarrEvEntryTableLocal;

typedef long long s64;

/* Views of the NarrEv entry table at gNarrEvEntryTable. They stay apart: the field types decide the alias sets. */
#define gNarrEvEntry (*(NarrEvEntryTableLocal *) &gNarrEvEntryTable)

void sprintf(char *, const char *, const char *, const char *, s32);
void xglFontFlush(void);

static inline char NarrEvPrevChar(char *textEnd, s32 back) {
    return textEnd[-back];
}

/* Per-frame NarrEv HUD: advances the envelope table, draws the text lines of the current entry (the centred block when
 * gNarrEvMatchFound is 2 or more), advances the entry timer and sends the scissor GIF packet. */
void narrEvDrawHud(void) {
    char hudText[0x100];
    s32 lineCount;
    s32 textOffset;
    char *textOut;
    unsigned char *textSrc;
    s32 lineX;
    s32 lineY;
    XglPacket *packet;
    u64 depthWide;
    u64 depthMask;

    xglEnvelopeTableUpdate(
        (EnvelopeTable *) (gNarrEvEntry.narrEntryBase +
                           gNarrEvEntry.narrEntryCount * NARR_EV_ENTRY_SIZE),
        gNarrEvEntry.elapsedTicks);
    textOffset = gNarrEvEntry.narrEntryCount * NARR_EV_TEXT_STRIDE;
    textSrc = gNarrEvEntry.entryTextBlock;
    textSrc += textOffset;
    memset(gNarrEvEntry.textBuffer, 0, 0x1000);
    textOut = gNarrEvEntry.textBuffer;
    lineCount = 0;
    while (lineCount < gNarrEvMatchFound) {
        *textOut++ = *textSrc++;
        if (textOut[-1] == 0) {
            break;
        }
        *textOut++ = *textSrc++;
        if (textOut[-1] == 0) {
            break;
        }
        while (textOut[-2] != 0xD || NarrEvPrevChar(textOut, 1) != 0xA) {
            *textOut++ = *textSrc++;
        }
        lineCount++;
    }
    *textOut = 0;
    if (gNarrEvMatchFound == 1) {
        sprintf(
            hudText, "%s%s%d;",
            "$bg0;$prop1;$han1;$xx0,24;$xx1,8;$xx2,0;$xx6,1;$alpha0,2,0,2,0,0x80;$shadow+1,+1,01000000;",
            "$rubyy2;$xx0,36;$alpha0,1,0,1,0,",
            (s32) ((f32) gNarrEvEntry.timer * 2.1333334f));
        xglFontPrint(0x14, 0x28, 0xFFFFF0, hudText);
        xglFontPrintDirect(gNarrEvEntry.textBuffer);
    } else if (gNarrEvMatchFound >= 2) {
        textOut -= 2;
        lineY = (s32) ((f32) gNarrEvEntry.timer * 2.1333334f);
        lineX =
            (s32) (224.0f -
                   ((f32) narrEvCountLineEnds(gNarrEvEntry.entryTextBlock +
                                        gNarrEvEntry.narrEntryCount * NARR_EV_TEXT_STRIDE) *
                    36.0f * 0.5f));
        for (;;) {
            textOut--;
            if (textOut[0] != 0xD) {
                continue;
            }
            if (textOut[1] != 0xA) {
                continue;
            }
            if (textOut[-4] != textOut[0] || textOut[-3] != textOut[1]) {
                textOut[0] = 0;
                break;
            }
        }
        sprintf(
            hudText, "%s%s%d;",
            "$bg0;$prop1;$han1;$xx0,24;$xx1,8;$xx2,0;$xx6,1;$alpha0,2,0,2,0,0x80;$shadow+1,+1,01000000;",
            "$rubyy2;$xx0,36;$alpha0,1,0,1,0,", 0x80);
        xglFontPrint(0x14, lineX, 0xFFFFFF, hudText);
        xglFontPrintDirect(gNarrEvEntry.textBuffer);
        textOut[0] = 0xD;
        sprintf(
            hudText, "%s%s%d;",
            "$bg0;$prop1;$han1;$xx0,24;$xx1,8;$xx2,0;$xx6,1;$alpha0,2,0,2,0,0x80;$shadow+1,+1,01000000;",
            "$rubyy2;$xx0,36;$alpha0,1,0,1,0,", lineY);
        xglFontPrint(0x14, lineX, 0xFFFFF0, hudText);
        xglFontPrintDirect(gNarrEvEntry.textBuffer);
    }
    if (gNarrEvEntry.elapsedPaused == 0) {
        if (gNarrEvEntry.timer < 0x3C) {
            gNarrEvEntry.timer =
                gNarrEvEntry.timer + 1;
        } else if (gNarrEvMatchFound <
                   narrEvCountLineEnds(gNarrEvEntry.entryTextBlock +
                                 gNarrEvEntry.narrEntryCount * NARR_EV_TEXT_STRIDE)) {
            gNarrEvEntry.timer = 0;
            gNarrEvMatchFound = gNarrEvMatchFound + 1;
        }
    }
    packet = xglPacketGetCurrent();
    sceVif1PkCnt(packet, 0);
    depthWide = ((u64) -1) << 0x28;
    depthMask = depthWide >> 8;
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(0x3F, 0);
    gifStateAddReg(0x4E, (u64) 0x9880 << 0x11);
    gifStateAddReg(8, 5);
    gifStateAddReg(0x47, 0x30000);
    gifStateAddReg(0x42, ((s64) 0x40 << 0x20) | 0x64);
    gifStateAddReg(0x4C, gRenderState.display_buffer_base |
                             ((((u64) 0xFF000000 << 0x10) | 8) << 0x10));
    gifStateAddReg(6, (u64) 0x24020000 | (gRenderState.work_buffer_base << 5) |
                          ((u64) 0xC800 << 0x13));
    gifStateAddReg(1, ((s64) 0xFE00) << 0x2E);
    gifStateAddReg(0, 0x46);
    gifStateAddReg(4, ((D_00550D98 * 0x10) - 8) | ((u64) ((D_00550D9C * 0x10) - 9) << 0x10) | depthMask);
    gifStateAddReg(4, ((D_00550D98 * 0x10) + 0x1FF8) | ((u64) ((D_00550D9C * 0x10) + 0x1BF7) << 0x10) |
                          depthMask);
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);
    xglFontFlush();
    if (gNarrEvEntry.unk70 != 0) {
        narrEvUploadPicture(gNarrEvEntry.unk70);
    }
}

typedef struct NarrEvEntryTableOther {
    s32 elapsedTicks;                 /* 0x00, advanced by narrEvIncrementCounter */
    unsigned int scriptEntryCount;    /* 0x04 */
    s32 narrEntryCount;               /* 0x08 */
    unsigned char *narrEntryBase;     /* 0x0C */
    s32 unk_10;                       /* 0x10 */
    unsigned char unmodeled_14[0x44]; /* 0x14..0x57 */
    unsigned char *pCut;              /* 0x58 */
    s32 pszStrWork;                   /* 0x5C */
    unsigned char unmodeled_60[4];    /* 0x60: gNarrEvActionCode[0] */
    s32 pSoundBuf;                    /* 0x64 */
    s32 pauseCount;                   /* 0x68 */
    s32 elapsedPaused;                /* 0x6C */
    s32 loadedAddr;                   /* 0x70 */
} NarrEvEntryTableOther;

void narrEvLoadFile(s32 *outLoadedAddr, s32 fileHandle);
void narrEvIncrementCounter(NarrEvEntryTable *narrTable);

#define gNarrEvEntryOther (*(NarrEvEntryTableOther *) &gNarrEvEntryTable)

typedef struct NarrEvFileList {
    const char *names[5];
} NarrEvFileList;

typedef struct NarrEvFrameHook {
    unsigned char unmodeled_00[0x44];
    void (*frameCallback)(void);
} NarrEvFrameHook;

extern const NarrEvFileList D_0053B380;
extern const NarrEvFileList D_0053B428;
extern const char D_0053B2F0[];
extern const char D_0053B398[];
extern char *D_00550DEC;
extern char *D_00550DF0;
extern signed char gVsyncsPerFrame;
void func_001B6AB0(void);
void xglRenderLockClear(void);
s32 func_00185CB0(s32, s32);
s32 func_0018F7E8(s32);
void soundWorkSource(s32, s32, s32);
s32 soundSlotStatus(s32);
void soundSlotStop(s32);
void xglGetLanguageCode(s32);

/* Plays one NarrEv episode (episode 0..4): loads its script and logo, runs the scripted text and the NarrEv frame hook
 * until the pad button is pressed or the script ends, then frees the buffers. */
void narrEvPlayEpisode(s32 episode) {
    NarrEvFileList episodeFiles;
    unsigned char movieInfo[0x40];
    NarrEvFileList logoFiles;
    unsigned char logoInfo[0x20];
    s32 soundWorkSize;
    s32 scriptBuffer;
    s32 scriptDone = 0;
    s32 lineCount;
    unsigned int entryNumber;

    gNarrEvMatchFound = 1;
    func_001B6AB0();
    xglRenderLockClear();
    while (xglCdGetServicePending() != 0) {
        xglSleep();
    }
    func_00185CB0(0, 0);
    gVsyncsPerFrame = 1;
    soundWorkSize = func_0018F7E8(12);
    gNarrEvEntryOther.pSoundBuf = func_001B6520(soundWorkSize);
    if (gNarrEvEntryOther.pSoundBuf == 0) {
        assertFailed("g_sEpisode.pSoundBuf", "NarrEv.euc.c", 1241);
    }
    soundWorkSource(0, gNarrEvEntryOther.pSoundBuf, soundWorkSize);
    gNarrEvEntryOther.pszStrWork = func_001B6520(0x1000);
    if (gNarrEvEntryOther.pszStrWork == 0) {
        assertFailed(D_0053B2F0, "NarrEv.euc.c", 1246);
    }
    while (xglCdGetServicePending() != 0) {
        xglSleep();
    }
    narrEvResetEntryTable(&gNarrEvEntryTable);
    episodeFiles = D_0053B380;
    scriptBuffer = func_001B6520(0x19000);
    if (narrEvParseScript(episodeFiles.names[episode], scriptBuffer) == 0) {
        assertFailed(D_0053B398, "NarrEv.euc.c", 1267);
    }
    xglGetLanguageCode(D_00550DEC + 8);
    ((void (*)(void *dst, s32 a, const char *name)) sprintf)(
        movieInfo, D_00550DEC, (const char *) &gNarrEvEntryOther.unmodeled_14);
    narrEvLoadEntryText(
        movieInfo, scriptBuffer, &gNarrEvEntryTable);
    narrEvLoadMatchingSlots(gNarrEvEntryOther.narrEntryBase +
                  gNarrEvEntryOther.narrEntryCount * NARR_EV_ENTRY_SIZE);
    while (xglCdGetServicePending() != 0) {
        xglSleep();
    }
    narrEvDecodeSlots(gNarrEvEntryOther.narrEntryBase +
                  gNarrEvEntryOther.narrEntryCount * NARR_EV_ENTRY_SIZE);
    logoFiles = D_0053B428;
    xglGetLanguageCode(D_00550DF0 + 8);
    ((void (*)(void *dst, s32 a, const char *name)) sprintf)(logoInfo, D_00550DF0,
                                                                   logoFiles.names[episode]);
    narrEvLoadFile(&gNarrEvEntryOther.loadedAddr, (s32) logoInfo);
    narrEvStartOnNameMatch(&gNarrEvEntryTable);
    narrEvAdvanceEntry(&gNarrEvEntryTable);
    (*(NarrEvFrameHook *) &D_004C8390).frameCallback = narrEvDrawHud;
    if (((*(PadRecord *) &PadData).pressed & 0x40) == 0) {
        do {
            narrEvIncrementCounter(&gNarrEvEntryTable);
            if (soundSlotStatus(0) == 5) {
                gNarrEvEntryOther.pauseCount += 1;
            }
            if ((*(PadRecord *) &PadData).pressed & 0x20) {
                lineCount = narrEvCountLineEnds(gNarrEvEntryOther.pCut +
                                      gNarrEvEntryOther.narrEntryCount * NARR_EV_TEXT_STRIDE);
                if (gNarrEvMatchFound < lineCount) {
                    gNarrEvMatchFound = lineCount;
                } else {
                    gNarrEvEntryOther.pauseCount = 30;
                }
            }
            if (xglCdServiceIfPending() == 0 && gNarrEvEntryOther.pauseCount >= 30 &&
                gNarrEvMatchFound >=
                    narrEvCountLineEnds(gNarrEvEntryOther.pCut +
                                  gNarrEvEntryOther.narrEntryCount * NARR_EV_TEXT_STRIDE)) {
                scriptDone = narrEvAdvanceWhenIdle(&gNarrEvEntryTable);
                gNarrEvMatchFound = 1;
                gNarrEvEntryOther.pauseCount = 0;
            }
            if (scriptDone != 0) {
                break;
            }
            ((void (*)(void)) xglSleep)();
        } while (((*(PadRecord *) &PadData).pressed & 0x40) == 0);
    }
    (*(NarrEvFrameHook *) &D_004C8390).frameCallback = 0;
    soundSlotStop(0);
    entryNumber = 0;
    while (entryNumber < gNarrEvEntryOther.scriptEntryCount) {
        narrEvReleaseSlots(
            (NarrEvState *) (gNarrEvEntryOther.narrEntryBase + entryNumber * NARR_EV_ENTRY_SIZE));
        entryNumber++;
    }
    func_001B6798(gNarrEvEntryOther.unk_10);
    func_001B6798((s32) gNarrEvEntryOther.pCut);
    func_001B6798((s32) gNarrEvEntryOther.narrEntryBase);
    func_001B6798(gNarrEvEntryOther.pszStrWork);
    func_001B6798(gNarrEvEntryOther.pSoundBuf);
    func_001B6798(gNarrEvEntryOther.loadedAddr);
    func_001B6AB0();
    while (xglCdGetServicePending() != 0) {
        xglSleep();
    }
    func_00185CB0(1, 0);
    gVsyncsPerFrame = 1;
}

void xglCdWaitForService(void) {
    while (xglCdGetServicePending() != 0) {
        xglSleep();
    }
}

s32 D_00550D98 = 1792;
s32 D_00550D9C = 1824;
s8 gXglCdDiskStatus = -128;
s32 D_00550DA4 = 0;
s32 D_00550DA8 = 0;
s32 D_00550DAC = 512;
s32 D_00550DB0 = 448;
s32 D_00550DB4 = 0;
s32 D_00550DB8 = 0;
s32 gNarrEvMatchFound = 1;
u64 D_003EBBC0[12] = {
    0, 0x5100000500000000ULL, 0x1000000000000004ULL, 14, 0, 80, 0, 81,
    0x1C000000200ULL, 82, 0, 83
};
u64 D_003EBC20[4] = {
    0, 0x5100000100000000ULL, 0x800000000007000ULL, 0
};
u64 D_003EBC40[6] = {
    0, 0x5100000200000000ULL, 0x1000000000008001ULL, 14, 0, 63
};
u64 D_003EBC70[12] = {
    0, 0x5100000500000000ULL, 0x1000000000000004ULL, 14, 0, 80, 0, 81,
    0x5A00000088ULL, 82, 0, 83
};
u64 D_003EBCD0[4] = {
    0, 0x5100000100000000ULL, 0x800000000007000ULL, 0
};
u64 D_003EBCF0[6] = {
    0, 0x5100000200000000ULL, 0x1000000000008001ULL, 14, 0, 63
};
u64 D_003EBD20[12] = {
    0, 0x5100000500000000ULL, 0x1000000000000004ULL, 14, 0, 80, 0, 81,
    0x1C000000200ULL, 82, 0, 83
};
u64 D_003EBD80[4] = {
    0, 0x5100000100000000ULL, 0x800000000007000ULL, 0
};
u64 D_003EBDA0[6] = {
    0, 0x5100000200000000ULL, 0x1000000000008001ULL, 14, 0, 63
};
unsigned char gXglCdReadJob[56] = { 0 };
unsigned char gNarrEvEntryTable[96] = { 0 };
s32 gNarrEvActionCode[6] = { 0 };
u8 D_005A8080[256];

const char D_0053B2F0[24] = "g_sEpisode.pszStrWork";
const char D_0053B308[24] = "mnu/pse/episode1.txt";
const char D_0053B320[24] = "mnu/pse/episode2.txt";
const char D_0053B338[24] = "mnu/pse/ep_action.txt";
const char D_0053B350[24] = "mnu/pse/ep_pp.txt";
const char D_0053B368[24] = "mnu/pse/test.txt";
const NarrEvFileList D_0053B380 = {
    { D_0053B308, D_0053B320, D_0053B338, D_0053B350, D_0053B368 },
};
const char D_0053B398[64] = "psEpReadScript(pszEpisode[nEpisode], pWork)\0\0\0\0\0mnu/pse/xx/%s";
const char D_0053B3D8[16] = "logo_ep1.tm2";
const char D_0053B3E8[16] = "logo_ep2.tm2";
const char D_0053B3F8[16] = "logo_act.tm2";
const char D_0053B408[16] = "logo_pp.tm2";
const char D_0053B418[16] = "logo_test.tm2";
const NarrEvFileList D_0053B428 = {
    { D_0053B3D8, D_0053B3E8, D_0053B3F8, D_0053B408, D_0053B418 },
};
char *D_00550DEC = (char *) D_0053B398 + 48;
char *D_00550DF0 = (char *) D_0053B398 + 48;
s32 D_00550DF4 = 0;
s32 D_00550DF8 = 0;
s32 D_00550DFC = 0;

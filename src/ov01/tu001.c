#include "common.h"
#include "shared.h"

int xglMcMain(void);
void xglSleep(void);
void deviceCheckStepEmpty(void);
void xglFontPrintDirect(const char *text);
extern const char gDeviceCheckText[];

/* Device check of the logo screen: runs the memory-card query, then prints the device-check text. */
int device_check_sync(void) {
    int memoryCardStatus;

    memoryCardStatus = xglMcMain();
    deviceCheckStepEmpty();
    xglSleep();
    xglFontPrintDirect(gDeviceCheckText);
    return memoryCardStatus;
}

INCLUDE_ASM("asm/nonmatchings/ov01/tu001", func_00A80050);

/* Card-check gate of the logo screen: always returns 1 in this build, so the card check always runs. */
int hdd_check(void) {
    return 1;
}

typedef int s32;
typedef signed char s8;
void xglMcReset();

s32 logoMcStatusCheck(s32 memoryCardStatus) {
    if (memoryCardStatus >= 0) {
        xglMcReset();
    }
    return -1;
}

INCLUDE_ASM("asm/nonmatchings/ov01/tu001", func_00A800F8);

INCLUDE_ASM("asm/nonmatchings/ov01/tu001", func_00A804A0);

INCLUDE_ASM("asm/nonmatchings/ov01/tu001", func_00A80668);

/* A GS direct-data buffer of the title frame: the tag dword at +0x30 and two words at +0x70 and +0x74. */
typedef struct TitleVifBuffer {
    unsigned char unmodeled_00[0x30];
    u64 gs_tag;
    unsigned char unmodeled_38[0x38];
    u32 ofs_70;
    u32 ofs_74;
} TitleVifBuffer;

/* The render-state halfwords read here: the frame width at +0x0c, the height at +0x0e, the display base at +0x16. */
typedef struct TitleRenderBase {
    unsigned char unmodeled_00[0x0c];
    unsigned short width;
    unsigned short height;
    unsigned char unmodeled_10[0x06];
    unsigned short display_base;
} TitleRenderBase;

/* The splat piece gTitleVifPiece: the blank-row words at +0x40..+0x4c and the frame tag buffer at +0xB0. */
typedef struct TitleVifPiece {
    unsigned char unmodeled_00[0x40];
    u32 blank_rows[4];
    unsigned char unmodeled_50[0x60];
    TitleVifBuffer frameTags;               /* +0xB0 */
} TitleVifPiece;

void *xglPacketGetCurrent(void);
void sceVif1PkCnt(void *packet, unsigned int count);
void sceVif1PkAddDataN(void *packet, const void *data, unsigned int qwc);
void sceVif1PkRef(void *packet, const void *data, unsigned int qwc, unsigned int a3, unsigned int a4, unsigned int a5);
extern TitleRenderBase D_4C8390;
extern TitleVifPiece gTitleVifPiece;

extern TitleVifBuffer gTitleSpritePacket;
extern TitleVifBuffer gTitleWidthTagPacket;
extern u64 gCreditHeaderPacket[];
extern u64 gCreditSpacerPacket[];
extern u64 gCreditFooterPacket[];
extern u8 gBlankRowValue;

/* The title fade state: the fade level is a halfword at +0x28. */
typedef struct TitleFadeState {
    s32 tagValue;                           /* +0x00: passed to func_00A80EF8 as the tag value */
    unsigned char unmodeled_04[0x28 - 0x04];
    short fade_level;                       /* +0x28 */
} TitleFadeState;

extern TitleFadeState gTitleFadeState;
extern u64 gTitleFadeBuffer[];
/* The same fade buffer read as 32-bit words: the fade level in words 16..18. */
#define titleFadeWords ((s32 *) gTitleFadeBuffer)

/* One half of the credit roll band: 0x7000 quadwords (0x70000 bytes), so the second half is the next element. */
typedef struct CreditHalf {
    unsigned char quadwords[0x70000];
} CreditHalf;

/* Credit roll band: sends the header and the two credit halves of the band, then the blank rows below it. */
void logoDrawCreditBand(CreditHalf *creditBand, int rowsDrawn) {
    void *packet;

    packet = xglPacketGetCurrent();
    if (creditBand != 0) {
        gCreditHeaderPacket[4] = ((u64)D_4C8390.display_base << 37) | (0x8000ULL << 36);
        sceVif1PkCnt(packet, 0);
        sceVif1PkAddDataN(packet, gCreditHeaderPacket, 0x18);
        sceVif1PkRef(packet, gCreditSpacerPacket, 2, 0, 0, 0);
        sceVif1PkRef(packet, creditBand, 0x7000, 0, 0x51007000, 0);
        sceVif1PkRef(packet, gCreditSpacerPacket, 2, 0, 0, 0);
        sceVif1PkRef(packet, creditBand + 1, 0x7000, 0, 0x51007000, 0);
        sceVif1PkRef(packet, gCreditFooterPacket, 3, 0, 0, 0);
    }
    if (rowsDrawn < 128) {
        gTitleVifPiece.blank_rows[3] = 128 - rowsDrawn;
        gTitleVifPiece.blank_rows[2] = gBlankRowValue;
        gTitleVifPiece.blank_rows[1] = gBlankRowValue;
        gTitleVifPiece.blank_rows[0] = gBlankRowValue;
        sceVif1PkCnt(packet, 0);
        sceVif1PkAddDataN(packet, &gTitleVifPiece, 28);
    }
}

INCLUDE_ASM("asm/nonmatchings/ov01/tu001", func_00A80A28);

int titleNoGroupResult(void) {
    return 0;
}

void titleSendFrameTags(void) {
    void *packet;
    TitleVifBuffer *frameTagBuffer;
    unsigned int tagBit;

    tagBit = 0x80000;

    packet = xglPacketGetCurrent();
    frameTagBuffer = &gTitleVifPiece.frameTags;
    frameTagBuffer->gs_tag = 0x24020000 | (D_4C8390.width << 5) | (0xE400ULL << 20);
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddDataN(packet, frameTagBuffer, 20);
    gTitleSpritePacket.gs_tag = D_4C8390.height | tagBit;
    gTitleSpritePacket.ofs_70 = 0x8FF8;
    gTitleSpritePacket.ofs_74 = 0x8DF7;
    sceVif1PkAddDataN(packet, &gTitleSpritePacket, 32);
    gTitleSpritePacket.gs_tag = 0x400C0;
    gTitleSpritePacket.ofs_70 = 32760;
    gTitleSpritePacket.ofs_74 = 32759;
    sceVif1PkAddDataN(packet, &gTitleSpritePacket, 32);
    gTitleWidthTagPacket.gs_tag = D_4C8390.width | tagBit;
    sceVif1PkAddDataN(packet, &gTitleWidthTagPacket, 16);
}

void titleFrameHookNoOp(void) {

}

extern s32 gTitleDirectBlock;

void titleSendDirectBlock(void) {
    void *packet;

    packet = xglPacketGetCurrent();
    sceVif1PkCnt(packet, 0U);
    sceVif1PkAddDataN(packet, &gTitleDirectBlock, 0x20U);
}

INCLUDE_ASM("asm/nonmatchings/ov01/tu001", func_00A80EF8);

/* The VIF packet the title frame writes into: the write position at +0x00, the direct-HL open flag at +0x08, the open tag at +0x0C. */
typedef struct VifPacket {
    s32 writePosition;                      /* +0x00 */
    unsigned char unmodeled_04[0x08 - 0x04];
    s32 directOpen;                         /* +0x08 */
    s32 *openTag;                           /* +0x0C */
} VifPacket;

void func_00A80EF8(VifPacket *packet, s32 tagValue, s32 mode);
void sceVif1PkCloseDirectHLCode(VifPacket *packet);
void sceVif1PkOpenDirectHLCode(VifPacket *packet, s32 flags);

void titleVifPacketReopen(VifPacket **packetSlot) {
    s32 *openTagWord;
    s32 rewoundPosition;
    VifPacket *slotPacket;
    VifPacket *reopenedPacket;

    slotPacket = *packetSlot;
    if (slotPacket->directOpen != 0) {
        openTagWord = slotPacket->openTag;
        if (openTagWord != 0) {
            sceVif1PkCloseDirectHLCode(slotPacket);
            reopenedPacket = *packetSlot;
            if (*openTagWord == 0x51000000) {
                rewoundPosition = reopenedPacket->writePosition - 0x10;
                reopenedPacket->writePosition = rewoundPosition;
            }
        }
    }
    func_00A80EF8(*packetSlot, gTitleFadeState.tagValue, -1);
    sceVif1PkOpenDirectHLCode(*packetSlot, 0);
}

INCLUDE_ASM("asm/nonmatchings/ov01/tu001", func_00A810A0);

#define TITLE_FADE_MAX 0x80

/* Title fade: steps the fade level down to zero, sends the fade packet on each pass, then draws the frame. */
void titleFadeTransition(int fadeMode) {
    void *packet;
    int fullLevel;

    gTitleFadeState.fade_level = TITLE_FADE_MAX;
    if (fadeMode < 0) {
        gTitleFadeState.fade_level = 1;
    }
    if (gTitleFadeState.fade_level > 0) {
        do {
            fullLevel = TITLE_FADE_MAX;
            packet = xglPacketGetCurrent();
            if (fadeMode == 0) {
                gTitleFadeState.fade_level -= 16;
            } else {
                gTitleFadeState.fade_level -= 4;
            }
            if (gTitleFadeState.fade_level < 0) {
                gTitleFadeState.fade_level = 0;
            }
            gTitleFadeBuffer[6] = 0x24120000 | (D_4C8390.height << 5) | (0xC800ULL << 19);
            if (fadeMode == 0) {
                titleFadeWords[18] = gTitleFadeState.fade_level;
                titleFadeWords[17] = gTitleFadeState.fade_level;
                titleFadeWords[16] = gTitleFadeState.fade_level;
            } else {
                int progress = fullLevel - gTitleFadeState.fade_level;
                int scaledProgress = progress * 3;

                titleFadeWords[18] = gTitleFadeState.fade_level;
                titleFadeWords[16] = fullLevel - progress * 2;
                if (titleFadeWords[16] < 0) {
                    titleFadeWords[16] = 0;
                }
                titleFadeWords[17] = fullLevel - scaledProgress / 2;
                if (titleFadeWords[17] < 0) {
                    titleFadeWords[17] = 0;
                }
            }
            sceVif1PkCnt(packet, 0);
            sceVif1PkAddDataN(packet, gTitleFadeBuffer, 0x24);
            xglSleep();
            if (gTitleFadeState.fade_level == 0) {
                sceVif1PkCnt(packet, 0);
                sceVif1PkAddDataN(packet, gTitleFadeBuffer, 0x24);
                xglSleep();
            }
        } while (gTitleFadeState.fade_level > 0);
    }
    titleSendFrameTags();
    xglSleep();
}

extern unsigned char gMovieIndexTable[];
/* The movie slot of the title: gTitleMovieSlot holds a pointer to it, and its movie index is the byte at +0x07. */
typedef struct TitleMovieSlot {
    unsigned char unmodeled_00[0x07];
    s8 movieIndex;                          /* +0x07 */
} TitleMovieSlot;
extern TitleMovieSlot *gTitleMovieSlot;

void moviePlayerStartFile(s32 movie, TitleMovieSlot *slot);
s32 xglSRand();

void titleAdvanceMovie(s32 movie, s32 restartMovie) {
    if (restartMovie != 0) {
        gTitleMovieSlot->movieIndex = 0x31;
    } else {
        gTitleMovieSlot->movieIndex = gMovieIndexTable[(gTitleMovieSlot->movieIndex * 2) + (xglSRand() & 1)];
    }
    moviePlayerStartFile(movie, gTitleMovieSlot);
}

INCLUDE_ASM("asm/nonmatchings/ov01/tu001", func_00A81718);

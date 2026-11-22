#include "common.h"

typedef signed char s8;
typedef int s32;
typedef unsigned char u8;
typedef float f32;
typedef short s16;

/* Two floats: an x and a y position. */
typedef struct PanelVec2 {
    f32 x;                              /* +0x0 */
    f32 y;                              /* +0x4 */
} PanelVec2;

/* The 0x614-byte block that panelBlockInit fills: three words, a 0x100-byte header and five 0x100-byte stringEntries. */
typedef struct PanelBlock {
    s32 cleared[3];                     /* +0x00: zeroed by panelBlockInit (role not recovered) */
    unsigned char unmodeled_00C[4];     /* +0x0C */
    unsigned char planeHeader[0x100];        /* +0x10 */
    unsigned char stringEntries[5][0x100];    /* +0x110 */
    s32 trailer;                        /* +0x610: cleared at the end of panelBlockInit */
} PanelBlock;

/* The data block at gPanelData: an origin, the object at +0x5F0 and a three-word slot at +0xC10. */
typedef struct PanelDataBlock {
    PanelVec2 origin;                   /* +0x000 */
    unsigned char unmodeled_008[0x5F0 - 0x008];
    PanelBlock inner;                  /* +0x5F0 */
    unsigned char unmodeled_bbb[0xC10 - 0x5F0 - sizeof(PanelBlock)];
    s32 outerCleared[3];                /* +0xC10 */
} PanelDataBlock;

/* The 0xE0 bytes in front of the frame: the frame origin sits at +0xD0. */
typedef struct PanelLead {
    unsigned char unmodeled_000[0xD0];
    f32 x;                              /* +0xD0 */
    f32 y;                              /* +0xD4 */
    unsigned char unmodeled_0D8[8];
} PanelLead;

/* The frame record at gPanelFrame. */
typedef struct PanelFrame {
    unsigned char unmodeled_000[0x1C0];
    unsigned char second[0x20];         /* +0x1C0 */
    unsigned char unmodeled_1E0[0x5E0 - 0x1E0];
    unsigned char object[0x30];         /* +0x5E0 */
    unsigned char unmodeled_610[0xC00 - 0x610];
    unsigned char slot[0x9D];           /* +0xC00 */
    s8 collapsed;                       /* +0xC9D */
} PanelFrame;

/* The panel: the lead and the frame in one record, starting at the lowest address panelDrawFrame reads. */
typedef struct PanelRecord {
    PanelLead lead;                     /* +0x000 */
    PanelFrame frame;                   /* +0x0E0 */
} PanelRecord;

/* Four halfwords: two corners of a rectangle in GS coordinates (x, y at +0x0 and +0x2, then the far corner). */
typedef struct GsScreenRect {
    s16 left;                                    /* 0x0 */
    s16 top;                                     /* 0x2 */
    s16 right;                                   /* 0x4 */
    s16 bottom;                                  /* 0x6 */
} GsScreenRect;

extern unsigned char gPanelStateWord[];
extern unsigned char gPanelStatusByte[];
/* Views of the untyped globals gPanelStateWord (one word) and gPanelStatusByte (one byte); the extern stays unsized so the object is unchanged. */
#define panelStateWord (*(s32 *) gPanelStateWord)
#define panelStatusByte (*(u8 *) gPanelStatusByte)
extern unsigned char gPanelData[];
#define panelData (*(PanelDataBlock *) gPanelData)
extern f32 gPanelLayoutValues[];
extern u8 gPanelFrame[];

void panelStateWordSet(s32 nextPanelState) {
    s32 stored;

    stored = nextPanelState;
    panelStateWord = stored;
}

INCLUDE_ASM("asm/main/nonmatchings/tu397", func_002DD4A8);

void stateNoopEnter(void) {

}

void stateNoopLeave(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu397", func_002DD600);

INCLUDE_ASM("asm/main/nonmatchings/tu397", func_002DD990);

INCLUDE_ASM("asm/main/nonmatchings/tu397", func_002DE000);

void xglFontSetColorIndex(s32 index);
s32 xglFontGetColorIndex(void);
void panelDrawFrame(void);

void panelDrawKeepFontColor(void) {
    s32 savedState;

    savedState = xglFontGetColorIndex();
    panelDrawFrame();
    xglFontSetColorIndex(savedState);
}

s8 panelStatusByteGet(void) {
    return panelStatusByte;
}

INCLUDE_ASM("asm/main/nonmatchings/tu397", func_002DE2A8);

void vec2SetPosition(PanelVec2 *vec, f32 x, f32 y) {
    vec->x = x;
    vec->y = y;
}

INCLUDE_ASM("asm/main/nonmatchings/tu397", func_002DE4C8);

INCLUDE_ASM("asm/main/nonmatchings/tu397", func_002DE668);

INCLUDE_ASM("asm/main/nonmatchings/tu397", func_002DE788);

s32 seGetChannelHandle(s32 index);
void planeHostInit(void *header);
void planeHostSetChannelHandle(void *header, s32 kind);
void planeHostSetWordFC(void *header, s32 count);
void planeHostSetLevel(void *header, f32 scale);
void planeHostSetWordE4(void *header, s32 value);
void planeHostSetWordE0(void *header, s32 value);
void seVoiceHeaderSetPosition(void *header, f32 a, f32 b);
void func_002D6978(void *entry);
void njStringSetZoom(void *entry, s32 a, s32 b);
void njStringSetFlags(void *string, s32 flags);

void panelBlockInit(PanelBlock *panelBlock) {
    u8 *planeHeader;
    s32 channelHandle;
    s32 i;

    panelBlock->cleared[0] = 0;
    panelBlock->cleared[1] = 0;
    panelBlock->cleared[2] = 0;
    planeHeader = panelBlock->planeHeader;
    planeHostInit(planeHeader);
    channelHandle = seGetChannelHandle(2);
    planeHostSetChannelHandle(planeHeader, channelHandle);
    planeHostSetWordFC(planeHeader, 5);
    planeHostSetLevel(planeHeader, 253.0f);
    planeHostSetWordE4(planeHeader, 0xE);
    planeHostSetWordE0(planeHeader, 0x12);
    seVoiceHeaderSetPosition(planeHeader, 10.0f, 2.0f);
    for (i = 0; i < 5; i++) {
        func_002D6978(panelBlock->stringEntries[i]);
        njStringSetZoom(panelBlock->stringEntries[i], 0xE, 0xE);
        njStringSetFlags(panelBlock->stringEntries[i], 0xC);
    }
    panelBlock->trailer = 0;
}

void vec2SetSize(PanelVec2 *vec, f32 x, f32 y) {
    vec->x = x;
    vec->y = y;
}

INCLUDE_ASM("asm/main/nonmatchings/tu397", func_002DEC98);

INCLUDE_ASM("asm/main/nonmatchings/tu397", func_002DF018);

void panelOuterClear(s32 *outerCleared) {
    outerCleared[0] = 0;
    outerCleared[1] = 0;
    outerCleared[2] = 0;
}

void vec2SetOffset(PanelVec2 *vec, f32 x, f32 y) {
    vec->x = x;
    vec->y = y;
}

INCLUDE_ASM("asm/main/nonmatchings/tu397", func_002DF350);

INCLUDE_ASM("asm/main/nonmatchings/tu397", func_002DF868);

void func_002DE2A8(void *block);
void panelBlockInit(PanelBlock *panelBlock);

void panelLayoutInit(void) {
    func_002DE2A8(gPanelData);
    vec2SetPosition(&panelData.origin, gPanelLayoutValues[0], gPanelLayoutValues[1]);
    panelBlockInit(&panelData.inner);
    vec2SetSize((PanelVec2 *) &panelData.inner, gPanelLayoutValues[2], gPanelLayoutValues[3]);
    panelOuterClear(panelData.outerCleared);
    vec2SetOffset((PanelVec2 *) panelData.outerCleared, gPanelLayoutValues[14], gPanelLayoutValues[15]);
}

INCLUDE_ASM("asm/main/nonmatchings/tu397", func_002DFA28);

void func_00215250();
void func_002D0D88();
void func_002DE788();
void func_002DF018();
void func_002DF868();

void panelDrawFrame(void)
{
    u8 *frame;
    GsScreenRect rect;
    f32 y;
    f32 x;
    s32 right;
    s32 bottom;
    PanelRecord *panel;

    frame = gPanelFrame;
    func_002D0D88(frame, 0x22, 0x73201C1B, 0);
    panel = (PanelRecord *) (frame - sizeof(PanelLead));
    func_002D0D88(panel->frame.second, 0x22, 0x73201C1B, 0);
    x = panel->lead.x + 4.0f;
    y = panel->lead.y + 4.0f;
    right = (((s32) (x + 406.0f)) * 0x10) + 0x6FF8;
    bottom = (((s32) (y + 18.0f)) * 0x10) + 0x71F8;
    rect.left = (((s32) x) * 0x10) + 0x6FF8;
    rect.top = (((s32) y) * 0x10) + 0x71F8;
    rect.right = right;
    rect.bottom = bottom;
    func_00215250(&rect.left, 0x5A402C2C, 0x402C2C, 0x101);
    if (panel->frame.collapsed != 0)
    {
        func_002DE788(&panel->lead.x);
        return;
    }
    func_002DE788(&panel->lead.x);
    func_002DF018(panel->frame.object);
    func_002DF868(panel->frame.slot);
}

INCLUDE_ASM("asm/main/nonmatchings/tu397", func_002DFD48);

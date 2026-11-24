#include "common.h"

#include "shared.h"
#include "gs.h"
/* The draw environment record returned by drawEnvGetRecord (its own view; tu108.c's DrawEnvRecord has other offsets). */
typedef struct DrawWindowRecord {
    unsigned char unmodeled_00[0x14];
    int frameBase;
    int drawMode;
    int originX;
    int originY;
    int width;
    int height;
    int clipX;
    int clipY;
    int drawWidth;
    int drawHeight;
} DrawWindowRecord;

void *drawEnvGetRecord(void);
void func_001D56D8(void);

/* Sets the draw environment to a 256x224 window at the origin with a 256x224 clip and a 512x448 draw size, then
   calls func_001D56D8. */
void drawEnvSetWindow256x224(void) {
    DrawWindowRecord *drawArea;

    drawArea = drawEnvGetRecord();
    drawArea->originY = drawArea->originX = 0;
    drawArea->width = 0x100;
    drawArea->height = 0xE0;
    drawArea->frameBase = 0x80;
    drawArea->drawMode = 3;
    drawArea->clipX = 0x100;
    drawArea->clipY = 0xE0;
    drawArea->drawWidth = 0x200;
    drawArea->drawHeight = 0x1C0;
    func_001D56D8();
}

typedef struct RenderPages {
    unsigned char unmodeled_00[8];
    u16 depthPage;
    unsigned char unmodeled_0A[6];
    u16 copyPage;
    unsigned char unmodeled_12[4];
    u16 screenPage;
} RenderPages;

extern RenderPages D_004C8390;
int xglPacketBufferInit(int spr_address);
void xglPacketAppendEntry(int word0, int word1, int word2, int word3);
void xglPacketAppendQuad(u64 reg, u64 value);
void xglPacketFinishTag(void);
void xglPacketSendDirect(int address, int qwordCount);

/* Builds one GS packet list at the scratchpad: a direct-mode GIF tag, the state registers (test, z, texture, alpha,
   colour, frame, primitive) and two textured quads that copy the screen page into the copy page and back, then sends
   it. */
void xglPacketScreenCopyPass(void) {
    xglPacketBufferInit(0x70000000);
    xglPacketAppendEntry(0x8000, 0x10000000, GIF_AD, 0);
    xglPacketAppendQuad(GS_FBA_1, 0);
    xglPacketAppendQuad(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
    xglPacketAppendQuad(GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.depthPage, 0x31, 1));
    xglPacketAppendQuad(GS_CLAMP_1, 0);
    xglPacketAppendQuad(GS_TEX1_1, GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0));
    xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 2, 0x80));
    xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0, 0x3F800000));
    xglPacketAppendQuad(GS_TEX0_1, GS_SET_TEX0(D_004C8390.screenPage << 5, 8, 0, 9, 9, 0, 0, 0, 0, 0, 0, 0));
    xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.copyPage, 4, 0, 0xFF000000));
    xglPacketAppendQuad(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
    xglPacketAppendQuad(GS_UV, 0);
    xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x6FF8, 0x71F7, 0));
    xglPacketAppendQuad(GS_UV, GS_SET_UV(0x2000, 0x1C00));
    xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x7FF8, 0x81F7, 0));
    xglPacketAppendQuad(GS_TEX0_1, GS_SET_TEX0(D_004C8390.copyPage << 5, 4, 0, 8, 8, 0, 0, 0, 0, 0, 0, 0));
    xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.screenPage, 8, 0, 0xFF000000));
    xglPacketAppendQuad(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
    xglPacketAppendQuad(GS_UV, GS_SET_UV(0x1000, 0x1000));
    xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x7FF8, 0x7FF7, 0));
    xglPacketAppendQuad(GS_UV, GS_SET_UV(0x2000, 0x2000));
    xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x8FF8, 0x8DF7, 0));
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);
}

/* Empty step: no body in the original. */
void tu114EmptyStep(void) {
}

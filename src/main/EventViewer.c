#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/EventViewer", func_002437A0);

typedef int s32;
typedef unsigned char u8;
extern s32 gEventViewerReadItem;
s32 xglCdAddReadyBits(s32);
s32 xglCdGetDriveStatus();
s32 xglCdGetSectorAlignedFileSize();
s32 xglCdDriveStatusLowBitSet();
s32 func_00251800();
s32 eventViewerSilenceSound();

void eventViewerCdPoll(void) {
    eventViewerSilenceSound();
    if ((xglCdDriveStatusLowBitSet() != 0) && !(xglCdGetDriveStatus() & 8)) {
        if (xglCdGetSectorAlignedFileSize() > 0) {
            gEventViewerReadItem = func_00251800();
        }
        xglCdAddReadyBits(8);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/EventViewer", func_002439D8);

extern unsigned char gEventViewerTitleText[];
extern unsigned char gEventViewerFooterText[];
extern unsigned char gEventViewerFooterTail[];
s32 xglFontPrintDirect();
s32 xglFontPrint();

void eventViewerDrawCaption(u8 *caption) {
    xglFontPrint(0xA, 0x1E, 0xFFFFFF, gEventViewerTitleText);
    xglFontPrintDirect(caption);
    xglFontPrint(0x12C, 0x15E, 0xFFFFFF, gEventViewerFooterText);
    xglFontPrintDirect(gEventViewerFooterTail);
}

INCLUDE_ASM("asm/main/nonmatchings/EventViewer", func_00243C00);

INCLUDE_ASM("asm/main/nonmatchings/EventViewer", func_00243D78);

INCLUDE_ASM("asm/main/nonmatchings/EventViewer", func_002441D8);

INCLUDE_ASM("asm/main/nonmatchings/EventViewer", func_00244C38);

extern s32 gEventViewerExchangeSuspended;
extern s32 D_00550C90;
extern s32 gEventViewerWindowTop;
s32 func_00185CB0(s32, s32);
s32 xglCdSetReadyFlag();
s32 gifStateResetTimings();
s32 XenoActorPoolInitDefault();
s32 xglCdServiceIfPending();

void eventViewerReset(void) {
    func_00185CB0(0, 0);
    do {

    } while (xglCdServiceIfPending() != 0);
    gEventViewerWindowTop = 0x37;
    gifStateResetTimings();
    XenoActorPoolInitDefault();
    xglCdSetReadyFlag();
    gEventViewerExchangeSuspended = 0;
    D_00550C90 = 0;
    gEventViewerReadItem = 0;
}

void func_00195050(s32, s32, s32, s32, s32);
void stateExchangeOff();
void stateExchangeOn();
void stateExchangeOverrideSet();
void xglRenderClearFrame();
s32 xglStudioSelectGetActiveCamera(s32);

void eventViewerRender(void) {
    s32 camera;
    s32 windowTop;
    xglRenderClearFrame();
    if (gEventViewerExchangeSuspended == 0) {
        stateExchangeOn();
    } else {
        stateExchangeOff();
    }
    stateExchangeOverrideSet();
    camera = xglStudioSelectGetActiveCamera(0);
    windowTop = gEventViewerWindowTop;
    func_00195050(camera, 0, windowTop, 0x200, windowTop + 0x140);
}

void eventViewerNoop(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/EventViewer", func_00244DB8);

INCLUDE_ASM("asm/main/nonmatchings/EventViewer", func_00244F68);

INCLUDE_ASM("asm/main/nonmatchings/EventViewer", func_00245040);

INCLUDE_ASM("asm/main/nonmatchings/EventViewer", func_00245098);

INCLUDE_ASM("asm/main/nonmatchings/EventViewer", func_00245198);

void eventViewerReset();
s32 func_0024F3C0(s32);

void eventViewerResetStart(s32 mode) {
    eventViewerReset();
    func_0024F3C0(mode);
}

extern unsigned char D_004C83F0[];
#define gStateFrameCallback (*(u8 **) D_004C83F0)
s32 func_0019AC80();
s32 func_001E8AF0();
s32 func_001E8CB8();
s32 func_001E8E70();
extern unsigned char func_00244C38[];
s32 soundSlotSetVolume();

void eventViewerInit(void) {
    func_0019AC80(0);
    gStateFrameCallback = func_00244C38;
    gifStateResetTimings();
    func_001E8E70();
    func_001E8AF0();
    func_001E8CB8(2);
    soundSlotSetVolume(1, 0x7F, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/EventViewer", func_00245308);

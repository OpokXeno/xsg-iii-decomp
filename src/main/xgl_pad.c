#include "common.h"
#include "shared.h"

#include "main/tu007.h"

extern PadRecord PadData[2];
extern unsigned char gPadDmaBuffer[];
int func_002F51B8(int mode);
void func_002F5610(int port, int slot, unsigned char *dma_buffer);
void xglPadSetDefaults(void);
void xglPadSetRepeat(int port, unsigned short mask, int delay, int interval);

void xglPadInitial(void) {
    unsigned int clearedByteCount;
    unsigned char *portZeroRecordBytes;
    unsigned char *portOneRecordBytes;
    int button;
    int button_index;
    int dead_zone_index;

    func_002F51B8(0);
    func_002F5610(0, 0, gPadDmaBuffer);
    func_002F5610(1, 0, gPadDmaBuffer + 256);

    portZeroRecordBytes = (unsigned char *)&PadData[0];
    portOneRecordBytes = (unsigned char *)&PadData[1];
    for (clearedByteCount = 0; clearedByteCount < sizeof(PadRecord); ++clearedByteCount) {
        *portZeroRecordBytes++ = 0;
        *portOneRecordBytes++ = 0;
    }

    xglPadSetDefaults();

    button = 1;
    for (button_index = 0; button_index < 8; ++button_index) {
        PadData[0].buttonMap[button_index] = button;
        PadData[1].buttonMap[button_index] = button;
        button <<= 1;
    }

    PadData[0].horizontalDeadZone = 64;
    PadData[0].verticalDeadZone = 64;
    PadData[1].horizontalDeadZone = 64;
    PadData[1].verticalDeadZone = 64;
    for (dead_zone_index = 0; dead_zone_index < 4; ++dead_zone_index) {
        PadData[0].axisDeadZone[dead_zone_index] = 32;
        PadData[1].axisDeadZone[dead_zone_index] = 32;
    }
}

void xglPadSetRepeat(int port, unsigned short mask, int delay, int interval) {
    PadRecord *pad;

    if (port == -1) {
        xglPadSetRepeat(0, mask, delay, interval);
        xglPadSetRepeat(1, mask, delay, interval);
        return;
    }
    pad = &PadData[port];
    if (delay > 128) {
        delay = 128;
    }
    if (interval >= 128) {
        interval = 127;
    }
    pad->repeatMask = mask;
    pad->repeatInterval = interval;
    pad->repeatDelay = delay;
    pad->unmodeled_4e = -delay;
}

/* Sets the repeat mask of the button group at +0x68 of a pad, or of both pads for port -1. */
void func_00186358(int port, unsigned short mask)
{
    if (port == -1) {
        func_00186358(0, mask);
        func_00186358(1, mask);
        return;
    }
    PadData[port].groupRepeatMask = mask;
}

typedef int s32;
void func_00186358(int port, unsigned short mask);

void xglPadSetDefaults(void) {
    xglPadSetRepeat(-1, 0xF000U, 5, 1);
    func_00186358(-1, 0xF0F0);
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_pad", xglPadRead);

#include "common.h"
#include "shared.h"

#include "main/tu007.h"

extern PadRecord PadData[2];
extern unsigned char D_00867380[];
int func_002F51B8(int mode);
void func_002F5610(int port, int slot, unsigned char *dma_buffer);
void func_001863C0(void);
void xglPadSetRepeat(int port, unsigned short mask, int delay, int interval);

void xglPadInitial(void) {
    unsigned int byte_index;
    unsigned char *first_record_bytes;
    unsigned char *second_record_bytes;
    int button;
    int button_index;
    int dead_zone_index;

    func_002F51B8(0);
    func_002F5610(0, 0, D_00867380);
    func_002F5610(1, 0, D_00867380 + 256);

    first_record_bytes = (unsigned char *)&PadData[0];
    second_record_bytes = (unsigned char *)&PadData[1];
    for (byte_index = 0; byte_index < sizeof(PadRecord); ++byte_index) {
        *first_record_bytes++ = 0;
        *second_record_bytes++ = 0;
    }

    func_001863C0();

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

INCLUDE_ASM("asm/main/nonmatchings/xgl_pad", func_00186358);

INCLUDE_ASM("asm/main/nonmatchings/xgl_pad", func_001863C0);

INCLUDE_ASM("asm/main/nonmatchings/xgl_pad", xglPadRead);

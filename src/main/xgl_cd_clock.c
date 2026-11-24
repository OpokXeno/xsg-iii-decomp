#include "common.h"
#include "shared.h"
#include "main/xgl_cd.h"

typedef int s32;

s32 stubReturnZeroPrimary(void) {
    return 0;
}

s32 stubReturnZeroSecondary(void) {
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_cd_clock", func_00225780);

INCLUDE_ASM("asm/main/nonmatchings/xgl_cd_clock", func_002257F0);

void xglClockRead(XglClock *clock);

unsigned int xglClockDayTime2UInt(XglClock *clock_time);

void xglClockSampleDayTime(void);

extern int xglClockLastDayTime;

INCLUDE_ASM("asm/main/nonmatchings/xgl_cd_clock", func_00225938);

INCLUDE_ASM("asm/main/nonmatchings/xgl_cd_clock", func_00225A60);

/* xglClockSampleDayTime: reads the clock and stores its day time in xglClockLastDayTime. */
void xglClockSampleDayTime(void) {
    XglClock clock_time;

    xglClockRead(&clock_time);
    xglClockLastDayTime = xglClockDayTime2UInt(&clock_time);
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_cd_clock", func_00225B90);

#include "common.h"
#include "main/tu007.h"

typedef int s32;
typedef unsigned int u32;

extern char gPadPortAssertMessage[];
extern char gPadSourceFile[];
extern PadRecord PadData[];
void assertFailed(const char *message, const char *file, int line);

#define PAD_PORT_COUNT 8U

PadRecord *padGetPortData(u32 port) {
    if (port >= PAD_PORT_COUNT) {
        assertFailed(gPadPortAssertMessage, gPadSourceFile, 0x19A);
    }
    return &PadData[port];
}

s32 padPressedAll(PadRecord *pad, s32 buttons) {
    return (~pad->pressed & buttons) == 0;
}

s32 padHeldAll(PadRecord *pad, s32 buttons) {
    return (~pad->held & buttons) == 0;
}

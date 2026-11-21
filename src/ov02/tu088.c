/*
 * OV02 original TU 88: 0x00ab12b8..0x00ab1310 (3 functions)
 */
#include "common.h"

typedef int s32;
s32 func_001B6520();

void bufferAllocThunk(void) {
    func_001B6520();
}

s32 func_001B6798();

void RgDispModelSetLocal(s32 enable) {
    if (enable != 0) {
        func_001B6798();
    }
}

void seResetInitialized(void);

void seInitFlagClearThunk(void) {
    seResetInitialized();
}

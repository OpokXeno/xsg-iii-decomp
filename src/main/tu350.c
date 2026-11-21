#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu350", func_002A6038);

typedef int s32;
s32 func_002A6038(s32, s32, s32, s32);

void mapProbeFlagOn(s32 source, s32 id, s32 value) {
    func_002A6038(source, id, value, 1);
}

void mapProbeFlagOff(s32 source, s32 id, s32 value) {
    func_002A6038(source, id, value, 0);
}

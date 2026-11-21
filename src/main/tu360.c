#include "common.h"

typedef int s32;
s32 func_002BD2D0(s32, s32, s32, s32);

void blockSubmitPlain(s32 submitHandle, s32 id, s32 submitValue) {
    func_002BD2D0(submitHandle, id, 0, submitValue);
}

INCLUDE_ASM("asm/main/nonmatchings/tu360", func_002AEAA0);

s32 func_002C1ED0(s32, s32, s32, s32);

void listSubmitPlain(s32 submitHandle, s32 id, s32 submitValue) {
    func_002C1ED0(submitHandle, id, 0, submitValue);
}

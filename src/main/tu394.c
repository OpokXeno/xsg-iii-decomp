#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu394", func_002DB120);

typedef unsigned char u8;
typedef int s32;
typedef struct RenderStepRecord {
    /* 0x00 */ s32 renderHandler;
    /* 0x04 */ u8 unk04[0x20];
} RenderStepRecord;

extern RenderStepRecord gRenderStepTable[];
extern s32 gRenderStepIndex;
extern s32 gRenderStepStatus;
extern s32 D_00550F84;
void xglRenderIoBegin(void);

s32 entryTableSelectCurrent(void) {
    s32 renderHandler;
    s32 result;

    renderHandler = gRenderStepTable[gRenderStepIndex].renderHandler;
    result = gRenderStepStatus == 2;
    if (renderHandler != 0) {
        D_00550F84 = renderHandler;
    } else {
        result = 1;
    }
    xglRenderIoBegin();
    return result;
}

INCLUDE_ASM("asm/main/nonmatchings/tu394", func_002DB1D0);

INCLUDE_ASM("asm/main/nonmatchings/tu394", func_002DB270);

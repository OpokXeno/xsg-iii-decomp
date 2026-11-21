#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu080", func_001C6158);

INCLUDE_ASM("asm/main/nonmatchings/tu080", func_001C6330);

INCLUDE_ASM("asm/main/nonmatchings/tu080", func_001C6440);

typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
typedef int s32;
typedef float f32;
s32 XenoActorIsDone(s32);

/* Mode block of a XenoActor, at +0x150. */
typedef struct XenoActorModeBlock {
    u8 pad_00[0x08];
    s32 unk_08;
    u8 padBeforeRequest[0x90 - 0x0C];
    s16 requestedMode; /* +0x90 */
    s16 appliedMode;   /* +0x92 */
} XenoActorModeBlock;

/* The head of a XenoActor as the mode code reads it: the channel at +0x24 and the mode block at +0x150. */
typedef struct XenoActorModeHead {
    u8 pad_00[0x24];
    s32 channel;                        /* +0x24 */
    u8 pad_28[0x150 - 0x28];
    XenoActorModeBlock channelMode;     /* +0x150 */
} XenoActorModeHead;

void XenoActorClearModeWhenDone(XenoActorModeHead *head) {
    XenoActorModeBlock *channelMode = &head->channelMode;

    if (XenoActorIsDone(head->channel) != 0) {
        channelMode->unk_08 = 0;
        channelMode->requestedMode = 0;
    }
}

void XenoActorCheckMode();
void XenoActorEnableChannel();
void XenoActorSetChannelValue(s32, f32);

void XenoActorApplyMode(XenoActorModeHead *head) {
    XenoActorModeBlock *channelMode = &head->channelMode;
    s32 channel;
    u16 requested;

    channel = head->channel;
    requested = channelMode->requestedMode;
    if (channelMode->appliedMode != channelMode->requestedMode) {
        channelMode->appliedMode = requested;
        XenoActorSetChannelValue(channel, 12.0f);
        XenoActorEnableChannel(channel);
        XenoActorCheckMode(channel, channelMode->requestedMode);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu080", func_001C65B0);

INCLUDE_ASM("asm/main/nonmatchings/tu080", func_001C66B0);

INCLUDE_ASM("asm/main/nonmatchings/tu080", func_001C6830);

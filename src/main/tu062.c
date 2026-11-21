#include "common.h"

typedef char *va_list;
#define va_start(args, last) ((args) = (va_list)__builtin_next_arg(last) - (8 - __builtin_args_info(2)) * 8)
typedef unsigned char u8;
typedef int s32;
typedef long long s64;
typedef float f32;
extern unsigned char gDebugInfoFormat[];
extern unsigned char gDebugLineText[];
s32 GameMpeg2Play();
s32 vprintf();

void debugInfoPrintf(s32 messageText, ...) {
    va_list formatArgs;
    va_start(formatArgs, messageText);

    GameMpeg2Play(gDebugLineText, gDebugInfoFormat, messageText);
    vprintf(gDebugLineText, formatArgs);
}

extern unsigned char gDebugNoteFormat[];

void debugNotePrintf(s32 messageText, ...) {
    va_list formatArgs;
    va_start(formatArgs, messageText);

    GameMpeg2Play(gDebugLineText, gDebugNoteFormat, messageText);
    vprintf(gDebugLineText, formatArgs);
}

extern unsigned char gDebugWarnFormat[];

void debugWarnPrintf(s32 messageText, ...) {
    va_list formatArgs;
    va_start(formatArgs, messageText);

    GameMpeg2Play(gDebugLineText, gDebugWarnFormat, messageText);
    vprintf(gDebugLineText, formatArgs);
}

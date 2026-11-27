#include "common.h"

typedef char *va_list;
#define va_start(args, last) ((args) = (va_list)__builtin_next_arg(last) - (8 - __builtin_args_info(2)) * 8)
typedef unsigned char u8;
typedef int s32;
typedef long long s64;
typedef float f32;
static unsigned char gDebugLineText[256];
s32 sprintf();
s32 vprintf();

void debugInfoPrintf(s32 messageText, ...) {
    va_list formatArgs;
    va_start(formatArgs, messageText);

    sprintf(gDebugLineText, "\033[1;36mCF3:\033[m %s\n", messageText);
    vprintf(gDebugLineText, formatArgs);
}


void debugNotePrintf(s32 messageText, ...) {
    va_list formatArgs;
    va_start(formatArgs, messageText);

    sprintf(gDebugLineText, "\033[1;35mCF3: %s\033[m\n", messageText);
    vprintf(gDebugLineText, formatArgs);
}


void debugWarnPrintf(s32 messageText, ...) {
    va_list formatArgs;
    va_start(formatArgs, messageText);

    sprintf(gDebugLineText, "\033[1;33mCF3:\033[m %s\n", messageText);
    vprintf(gDebugLineText, formatArgs);
}

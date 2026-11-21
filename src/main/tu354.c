#include "common.h"

typedef int s32;
typedef char *va_list;
#define va_start(args, last) ((args) = (va_list)__builtin_next_arg(last) - (8 - __builtin_args_info(2)) * 8)
typedef unsigned char u8;
extern unsigned char idNativeTable[];
extern char idModuleName[];
s32 sbNativeOwnerAdd(const char *name, void *methods);

void sbRegisterIdNatives(void) {
    sbNativeOwnerAdd(idModuleName, idNativeTable);
}

INCLUDE_ASM("asm/main/nonmatchings/tu354", func_002AB288);

extern char gDebugTestBannerText[];
void func_AB7538(void);
s32 printf(const char *format, ...);

void debugPrintTestBanner(void) {
    printf(gDebugTestBannerText);
    func_AB7538();
}

extern char gDebugNewline[];
s32 vsprintf(char *str, const char *format, va_list args);

void debugPrintf(const char *format, ...) {
    va_list formatArgs;
    u8 messageBuffer[0x100];
    va_start(formatArgs, format);

    vsprintf(messageBuffer, format, formatArgs);
    printf(messageBuffer);
    printf(gDebugNewline);
}

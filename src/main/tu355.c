#include "common.h"

typedef char *va_list;
#define va_start(ap, last) ((ap) = (va_list)__builtin_next_arg(last) - (8 - __builtin_args_info(2)) * 8)
typedef int s32;
s32 vsprintf(char *, const char *, va_list);
void xglFontDebugPrintf(int x, int y, const char *format, ...);

void xglFontDebugPrintfCell(s32 column, s32 row, const char *format, ...) {
    va_list ap;
    char formattedText[0x100];
    va_start(ap, format);

    vsprintf(formattedText, format, ap);
    xglFontDebugPrintf(column * 4, row * 0xA, formattedText);
}

extern s32 gXglFontDebugPrefixDefault;
void xglFontDebugPrefixSet(s32 *);

void xglFontDebugPrefixReset(void) {
    xglFontDebugPrefixSet(&gXglFontDebugPrefixDefault);
}

INCLUDE_ASM("asm/main/nonmatchings/tu355", func_002AB418);

INCLUDE_ASM("asm/main/nonmatchings/tu355", func_002AB548);

INCLUDE_ASM("asm/main/nonmatchings/tu355", func_002AB618);

INCLUDE_ASM("asm/main/nonmatchings/tu355", func_002AB6F8);

#include "common.h"

typedef int s32;
typedef float f32;

INCLUDE_ASM("asm/main/nonmatchings/tu038", func_0019CF80);

unsigned int func_0030E8E4(const char *text);
s32 func_0019CF80(unsigned char character, s32 x, s32 y, f32 weight, s32 kind, s32 count, s32 mode);

/* Draws text from x: each character goes to func_0019CF80 at x plus the width so far times weight;
 * returns the total width. func_0030E8E4 (the length of text) is called again on every pass. */
s32 func_0019D3F8(const char *text, s32 x, s32 y, f32 weight, s32 kind, s32 count, s32 mode)
{
    unsigned int i;
    s32 width;

    width = 0;
    for (i = 0; i < func_0030E8E4(text); i++) {
        width += func_0019CF80(text[i], (f32) x + width * weight, y, weight, kind, count, mode);
    }
    return width;
}

/* func_0019D3F8 with a shadow: the text is drawn first at x + 1, y + 1 in kind 0, then at x, y in kind;
 * returns the width of the second pass. */
s32 func_0019D4E8(const char *text, s32 x, s32 y, f32 weight, s32 kind, s32 count, s32 mode)
{
    unsigned int i;
    s32 width;

    width = 0;
    for (i = 0; i < func_0030E8E4(text); i++) {
        width += func_0019CF80(text[i], (f32) (x + 1) + width * weight, y + 1, weight, 0, count, mode);
    }
    width = 0;
    for (i = 0; i < func_0030E8E4(text); i++) {
        width += func_0019CF80(text[i], (f32) x + width * weight, y, weight, kind, count, mode);
    }
    return width;
}

extern s32 D_00551230;
extern f32 D_00551234;
extern s32 D_00551238;

void setKindWeightCount(s32 kind, s32 count, f32 weight) {
    D_00551230 = kind;
    D_00551234 = weight;
    D_00551238 = count;
}

typedef char *va_list;

#define va_start(ap, last) ((ap) = (va_list)__builtin_next_arg(last) - (8 - __builtin_args_info(2)) * 8)

int vsprintf(char *destination, const char *format, va_list formatArgs);

#define DEBUG_TEXT_BUFFER_SIZE 0x200

/* printf-style front end of func_0019D3F8: formats into a local buffer and draws it at x, y with the
 * kind, weight and count setKindWeightCount stored (mode 1). */
void func_0019D660(s32 x, s32 y, const char *format, ...)
{
    char text[DEBUG_TEXT_BUFFER_SIZE];
    va_list formatArgs;

    va_start(formatArgs, format);
    vsprintf(text, format, formatArgs);
    func_0019D3F8(text, x, y, D_00551234, D_00551230, D_00551238, 1);
}

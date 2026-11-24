#include "common.h"

typedef int s32;

extern char *gFontEscapeBuffer;
s32 func_0030E3D0(void *text, s32 value);
s32 xglFontGetStringWidth(const char *text);

s32 xglFontGetFormattedWidth(s32 formattedNumber) {
    gFontEscapeBuffer[0] = 0x19;
    gFontEscapeBuffer[1] = 3;
    gFontEscapeBuffer[2] = 0;
    func_0030E3D0(gFontEscapeBuffer, formattedNumber);
    return xglFontGetStringWidth(gFontEscapeBuffer);
}

/* Number of lines in text: each newline counts once. A byte with the high bit set is a lead byte, so the byte after it is skipped. */
s32 xglFontCountLines(const char *text) {
    s32 lines = 0;
    s32 result = 0;

    if (text != 0) {
        while (*text != 0) {
            if (*text & 0x80) {
                text++;
            } else {
                if (*text == '\n') {
                    lines++;
                }
            }
            text++;
        }
        result = lines;
    }
    return result;
}

s32 xglFontGetSPcodeSize(s32 code, const char *text);

/* Widest line of text in pixels: the lines end at "\r\n" or the two-character "\n" escape; the widths are
 * measured with xglFontGetStringWidth on a copy of each line in gFontEscapeBuffer. */
s32 xglFontGetMultilineWidth(const char *text) {
    s32 maxWidth = 0;
    s32 lineWidth;
    s32 spCodeBytes;
    char *lineCursor;

    gFontEscapeBuffer[0] = 0x19;
    gFontEscapeBuffer[1] = 3;
    lineCursor = gFontEscapeBuffer + 2;
    while (text != 0 && *text != 0) {
        if (*text == '\r' && text[1] == '\n') {
            *lineCursor = 0;
            text++;
            lineWidth = xglFontGetStringWidth(gFontEscapeBuffer);
            text++;
            if (maxWidth < lineWidth) {
                maxWidth = lineWidth;
            }
            lineCursor = gFontEscapeBuffer + 2;
            continue;
        }
        if ((unsigned char) *text < 0x20) {
            spCodeBytes = xglFontGetSPcodeSize((unsigned char) *text, text);
            for (; spCodeBytes > 0; spCodeBytes--) {
                *lineCursor++ = *text++;
            }
        } else if (*text == '\\' && text[1] == 'n') {
            *lineCursor++ = ' ';
            *lineCursor++ = 7;
            text += 2;
            continue;
        }
        *lineCursor++ = *text++;
    }
    *lineCursor = 0;
    lineWidth = xglFontGetStringWidth(gFontEscapeBuffer);
    if (maxWidth < lineWidth) {
        maxWidth = lineWidth;
    }
    return (maxWidth * 15) >> 4;
}

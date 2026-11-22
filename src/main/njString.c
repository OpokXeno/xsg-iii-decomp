#include "common.h"
#include "main/xgl_font_codes.h"
#include "main/njString.h"

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D6978);


void njStringSetCounter(NjString *string, s32 limit) {
    string->layoutCount = 0;
    string->layoutLimit = limit;
}

void njStringSetPosition(NjString *string, f32 x, f32 y) {
    string->x = x;
    string->y = y;
}

void njStringSetZoom(NjString *string, s8 zoomX, s8 zoomY) {
    string->zoomX = zoomX;
    string->zoomY = zoomY;
}

void njStringSetTextNoLayout(NjString *string, unsigned char *text) {
    string->text = text;
}

void njStringSetWord20(NjString *string, s32 value) {
    string->word20 = value;
}

void njStringSetColor(NjString *string, s32 red, s32 green, s32 blue) {
    s32 redByte;
    s32 greenByte;
    s32 blueByte;

    redByte = red;
    if (red < 0) {
        redByte = 0;
    }
    greenByte = green;
    if (green < 0) {
        greenByte = 0;
    }
    blueByte = blue;
    if (blue < 0) {
        blueByte = 0;
    }
    if (redByte >= 0x100) {
        redByte = 0xFF;
    }
    string->red = (f32) redByte;
    if (greenByte >= 0x100) {
        greenByte = 0xFF;
    }
    string->green = (f32) greenByte;
    if (blueByte >= 0x100) {
        blueByte = 0xFF;
    }
    string->blue = (f32) blueByte;
}

void njStringSetAlpha(NjString *string, s32 alpha) {
    s32 alphaValue;

    alphaValue = alpha;
    if (alpha < 0) {
        alphaValue = 0;
    }
    if (alphaValue >= 0x80) {
        alphaValue = 0x7F;
    }
    string->alpha = (f32) alphaValue;
}

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D6AA8);

void njStringSetFlags(NjString *string, s32 styleFlags) {
    string->styleFlags = styleFlags;
}

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D6B28);

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D6BA0);

/* FONT_OP_SCALE 16 16; njStringTextTooWide replaces the scale. */
extern NjStringScaleCode gNjStringMeasureScale;
char *func_0030E3D0(char *destination, const char *source);
void *memset(void *destination, int value, unsigned int count);
char *strcpy(char *destination, const char *source);
int xglFontGetStringWidth(const char *text);

/* Measures `text` drawn at `scale` (FONT_OP_SCALE, both axes; 16 is 1.0) and returns whether it is wider than
 * maxWidth. A null text measures an uninitialized buffer, as the original does. */
s32 njStringTextTooWide(const char *text, s32 maxWidth, s32 scale)
{
  char measured[0x400];
  NjStringScaleCode scaleCode;
  if (text != 0)
  {
    scaleCode = gNjStringMeasureScale;
    scaleCode.bytes[1] = scale;
    scaleCode.bytes[2] = scale;
    memset(&measured[0], 0, 0x400);
    strcpy(&measured[0], scaleCode.bytes);
    func_0030E3D0(&measured[0], text);
  }
  return maxWidth < xglFontGetStringWidth(&measured[0]);
}

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D6CA8);

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D8358);

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D8458);

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D86C8);

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D88D8);

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D8A00);

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D8B30);

void njStringSetText(NjString *string, unsigned char *text) {
    char display[0x800];

    string->text = text;
    string->currentLine = 1;
    if ((text != 0) && (string->styleFlags & NJ_STRING_STYLE_LAYOUT_ON_SET)) {
        njStringLayoutText(string, display);
    }
}

void njStringSetPositionB(NjString *string, f32 x, f32 y) {
    string->x = x;
    string->y = y;
}

void njStringSetZoomB(NjString *string, s8 zoomX, s8 zoomY) {
    string->zoomX = zoomX;
    string->zoomY = zoomY;
}

void njStringSetColorB(NjString *string, s32 red, s32 green, s32 blue) {
    s32 redByte;
    s32 greenByte;
    s32 blueByte;

    redByte = red;
    if (red < 0) {
        redByte = 0;
    }
    greenByte = green;
    if (green < 0) {
        greenByte = 0;
    }
    blueByte = blue;
    if (blue < 0) {
        blueByte = 0;
    }
    if (redByte >= 0x100) {
        redByte = 0xFF;
    }
    string->red = (f32) redByte;
    if (greenByte >= 0x100) {
        greenByte = 0xFF;
    }
    string->green = (f32) greenByte;
    if (blueByte >= 0x100) {
        blueByte = 0xFF;
    }
    string->blue = (f32) blueByte;
}

void njStringSetAlphaB(NjString *string, s32 alpha) {
    s32 alphaValue;

    alphaValue = alpha;
    if (alpha < 0) {
        alphaValue = 0;
    }
    if (alphaValue >= 0x80) {
        alphaValue = 0x7F;
    }
    string->alpha = (f32) alphaValue;
}

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D8C80);

void njStringSetFlagsB(NjString *string, s32 styleFlags) {
    string->styleFlags = styleFlags;
}

void njStringSetFirstVisibleLine(NjString *string, s32 line) {
    string->firstVisibleLine = line & 0xFF;
}

void njStringSetVisibleLineSpan(NjString *string, s32 span) {
    string->visibleLineSpan = span & 0xFF;
}

void njStringSetWordFC(NjString *string, s32 value) {
    string->wordFC = value;
}

const char *xglFontCopyTextUnit(char *textUnitBuffer, const char *text, int *length);

/*
 * The font codes the layout writes. Each is copied by value from its template, and each template has
 * its own type: struct copies of one type are serialized by the scheduler, distinct types are not, and
 * the original orders them as distinct types.
 */

/* FONT_OP_STYLE, FONT_STYLE_PROPORTIONAL | FONT_STYLE_WIDE_ASCII: one-byte text. */
typedef struct NjStringProportionalStyle {
    char bytes[3];
} NjStringProportionalStyle;

/* FONT_OP_STYLE, FONT_STYLE_WIDE_ASCII: two-byte (EUC-JP) text, fixed pitch. */
typedef struct NjStringFixedStyle {
    char bytes[3];
} NjStringFixedStyle;

/* FONT_OP_POS_X_0 <x>, or FONT_OP_POS_X_1 <x - 255>: the pen x at the start of each line. */
typedef struct NjStringLineStartX {
    char bytes[3];
} NjStringLineStartX;

/* FONT_OP_CONFIG 2 FONT_CONFIG_DEFAULT_WIDTH <width>: the character width that follows. */
typedef struct NjStringWidthCode {
    char bytes[5];
} NjStringWidthCode;

extern NjStringProportionalStyle gNjStringProportionalStyle;
extern NjStringFixedStyle gNjStringFixedStyle;
extern NjStringLineStartX gNjStringLineStartX;
extern NjStringScaleCode gNjStringDefaultScale;            /* FONT_OP_SCALE 15 15 */
extern NjStringWidthCode gNjStringWidthCode;            /* width NJ_TEXT_WIDTH */
extern char gNjStringHighlightRgb[];                       /* "f24040": highlight color, RRGGBB hex */

/* The character width of text; icons and the long dash change it and put it back. */
#define NJ_TEXT_WIDTH 4

/* Lengths of the codes and "$...;" commands written to the display text. */
#define NJ_STYLE_CODE_LENGTH 2                  /* FONT_OP_STYLE <bits> */
#define NJ_WIDTH_CODE_LENGTH 4                  /* FONT_OP_CONFIG 2 FONT_CONFIG_DEFAULT_WIDTH <width> */
#define NJ_ZOOM_COMMAND_LENGTH 11               /* "$zoomXX,YY;" */
#define NJ_COLOR_COMMAND_LENGTH 11              /* "$colBBGGRR;" */
#define NJ_RUBY_COMMAND_LENGTH 7                /* "$rubys;", "$rubye;", "$rubyb;" */
#define NJ_ICON_BLANK_LENGTH 9                  /* width code, ' ', width code */

/* Length of a markup tag, without the terminator. */
#define TAG_LENGTH(tag) ((int)sizeof(tag) - 1)

/* The line being laid out lies inside the window of shown lines. */
#define NJ_STRING_LINE_VISIBLE(string)                                                                   \
    ((string)->firstVisibleLine <= (string)->currentLine &&                                              \
     (string)->firstVisibleLine + (string)->visibleLineSpan >= (string)->currentLine)

/* Byte writers for the command macros below: to the display text, to the measured line, or to both. */
#define PUT_DISPLAY(k, value) out[k] = (value)
#define PUT_LINE(k, value) lineText[lineLength + (k)] = (value)
#define PUT_BOTH(k, value) out[k] = lineText[lineLength + (k)] = (value)
#define PUT_COLOR_SPAN(k, value) colorSpanTag[k] = (value)

/* "$zoomXX,YY;": the glyph zoom, two decimal digits per axis. */
#define NJ_ZOOM_COMMAND(PUT, x1, x0, y1, y0)                                                             \
    PUT(0, '$'); PUT(1, 'z'); PUT(2, 'o'); PUT(3, 'o'); PUT(4, 'm');                                     \
    PUT(5, x1); PUT(6, x0); PUT(7, ','); PUT(8, y1); PUT(9, y0); PUT(10, ';')

/* "$colBBGGRR;": the text color, hexadecimal. */
#define NJ_COLOR_COMMAND(PUT, b1, b0, g1, g0, r1, r0)                                                    \
    PUT(0, '$'); PUT(1, 'c'); PUT(2, 'o'); PUT(3, 'l');                                                  \
    PUT(4, b1); PUT(5, b0); PUT(6, g1); PUT(7, g0); PUT(8, r1); PUT(9, r0); PUT(10, ';')

/* "$ruby<kind>;": ruby (furigana) base start 'b', annotation start 's', end 'e'. */
#define NJ_RUBY_COMMAND(PUT, kind)                                                                       \
    PUT(0, '$'); PUT(1, 'r'); PUT(2, 'u'); PUT(3, 'b'); PUT(4, 'y'); PUT(5, kind); PUT(6, ';')

/* FONT_OP_CONFIG 2 FONT_CONFIG_DEFAULT_WIDTH <width> at offset `at`. */
#define NJ_WIDTH_CODE(PUT, at, width)                                                                    \
    PUT(at, FONT_OP_CONFIG); PUT((at) + 1, 2); PUT((at) + 2, FONT_CONFIG_DEFAULT_WIDTH); PUT((at) + 3, width)

/* A blank of width `width` for an icon, then the text width again. */
#define NJ_ICON_BLANK(PUT, width)                                                                        \
    NJ_WIDTH_CODE(PUT, 0, width); PUT(4, ' '); NJ_WIDTH_CODE(PUT, 5, NJ_TEXT_WIDTH)

/* Records a button icon at the pen position, then leaves a blank of width `blankWidth` for it and skips
 * its 4-byte tag ("<L1>", or '<' + a two-byte EUC-JP button symbol + '>'). */
#define NJ_STRING_PUT_ICON(icon, blankWidth)                                                             \
    {                                                                                                    \
        if (string->iconCount < NJ_STRING_MAX_ICONS) {                                                   \
            lineText[lineLength] = '\0';                                                                 \
            iconX = xglFontGetStringWidth(lineText) + penOffsetX;                                        \
            string->icons[string->iconCount].line = lineIndex;                                           \
            string->icons[string->iconCount].button = (icon);                                            \
            string->icons[string->iconCount].x = iconX;                                                  \
            string->iconCount++;                                                                         \
        }                                                                                                \
        if (NJ_STRING_LINE_VISIBLE(string)) {                                                            \
            displayLength += NJ_ICON_BLANK_LENGTH;                                                       \
            NJ_ICON_BLANK(PUT_DISPLAY, blankWidth);                                                      \
            out += NJ_ICON_BLANK_LENGTH;                                                                 \
        }                                                                                                \
        NJ_ICON_BLANK(PUT_LINE, blankWidth);                                                             \
        lineLength += NJ_ICON_BLANK_LENGTH;                                                              \
        text += TAG_LENGTH("<L1>");                                                                      \
    }

/*
 * Lays out the string's markup text (string->text, EUC-JP and ASCII) into `display`: the font codes and
 * "$...;" commands xglFont draws, with only the lines from firstVisibleLine to firstVisibleLine +
 * visibleLineSpan. It also places the button icons in string->icons and leaves the line count in
 * string->currentLine. Each line is also copied to lineText, whose width (xglFontGetStringWidth) gives
 * the x of an icon.
 *
 * Markup:   "\n" (backslash, n)  new line; open color and zoom spans continue on it
 *           <colBBGGRR> ... <cole>  color span; <name> ... '>'  highlighted span
 *           <sizeNN> ... <sizee>  zoom span; <x+N> / <x-N> / <xN>  pen x
 *           <rubyb> <rubys> <ruby e>  ruby; <○> <×> <□> <△> <L1> <L2> <R1> <R2> <ATR0>..<ATR4>  icons
 *           "──" (EUC-JP A8A1 A8A1)  one long dash
 */
void njStringLayoutText(NjString *string, char *display)
{
    char lineText[0x400];
    char colorSpanTag[NJ_COLOR_COMMAND_LENGTH];
    char defaultZoomDigits[4];
    char zoomSpanDigits[4];
    NjStringProportionalStyle proportionalStyle = gNjStringProportionalStyle;
    NjStringFixedStyle fixedStyle = gNjStringFixedStyle;
    NjStringLineStartX lineStartX = gNjStringLineStartX;
    NjStringScaleCode defaultScale = gNjStringDefaultScale;
    char textUnit[16];
    int unitLength;
    unsigned char *text;
    char *out;
    int displayLength;
    int lineLength;
    s8 lineIndex;
    int penOffsetX;
    int colorSpanOpen;
    int zoomSpanOpen;
    int proportional;
    f32 x;
    int zoomX;
    int zoomY;
    int iconX;

    penOffsetX = 0;
    zoomX = string->zoomX;
    zoomY = string->zoomY;
    text = string->text;
    defaultZoomDigits[0] = zoomX / 10 + '0';
    defaultZoomDigits[1] = zoomX % 10 + '0';
    defaultZoomDigits[2] = zoomY / 10 + '0';
    defaultZoomDigits[3] = zoomY % 10 + '0';
    x = string->x;
    if (!(x <= 255.0f)) {
        x -= 255.0f;
        lineStartX.bytes[0] = FONT_OP_POS_X_1;
        lineStartX.bytes[1] = x;
    } else {
        lineStartX.bytes[1] = x;
    }
    colorSpanOpen = 0;
    memset(lineText, 0, sizeof(lineText));
    string->currentLine = 0;
    displayLength = NJ_STYLE_CODE_LENGTH;
    zoomSpanOpen = 0;
    lineIndex = 0;
    proportional = 1;
    string->iconCount = 0;
    lineText[0] = display[0] = proportionalStyle.bytes[0];
    lineText[1] = display[1] = proportionalStyle.bytes[1];
    lineText[2] = defaultScale.bytes[0];
    lineText[3] = defaultScale.bytes[1];
    lineText[4] = defaultScale.bytes[2];
    lineLength = 5;
    if (*text != 0) {
        out = display + NJ_STYLE_CODE_LENGTH;
        do {
            xglFontCopyTextUnit(textUnit, (const char *)text, &unitLength);
            switch (unitLength) {
            case 2:
                if (proportional) {
                    if (NJ_STRING_LINE_VISIBLE(string)) {
                        displayLength += NJ_STYLE_CODE_LENGTH;
                        out[0] = fixedStyle.bytes[0];
                        out[1] = fixedStyle.bytes[1];
                        out += NJ_STYLE_CODE_LENGTH;
                    }
                    lineText[lineLength] = fixedStyle.bytes[0];
                    lineText[lineLength + 1] = fixedStyle.bytes[1];
                    lineLength += NJ_STYLE_CODE_LENGTH;
                    proportional = 0;
                }
                if (text[0] == 0xA8 && text[1] == 0xA1 && text[2] == 0xA8 && text[3] == 0xA1) {
                    /* "──": one dash at width 2 and zoom 26,15, stretched over both cells. Always shown. */
                    NjStringWidthCode widthCode = gNjStringWidthCode;

                    PUT_BOTH(0, proportionalStyle.bytes[0]);
                    PUT_BOTH(1, proportionalStyle.bytes[1]);
                    lineLength += NJ_STYLE_CODE_LENGTH;
                    out += NJ_STYLE_CODE_LENGTH;
                    widthCode.bytes[3] = 2;
                    PUT_BOTH(0, widthCode.bytes[0]);
                    PUT_BOTH(1, widthCode.bytes[1]);
                    PUT_BOTH(2, widthCode.bytes[2]);
                    PUT_BOTH(3, widthCode.bytes[3]);
                    PUT_BOTH(4, ' ');
                    lineLength += NJ_WIDTH_CODE_LENGTH + 1;
                    out += NJ_WIDTH_CODE_LENGTH + 1;
                    NJ_ZOOM_COMMAND(PUT_BOTH, '2', '6', '1', '5');
                    PUT_BOTH(11, text[0]);
                    PUT_BOTH(12, text[1]);
                    lineLength += NJ_ZOOM_COMMAND_LENGTH + 2;
                    out += NJ_ZOOM_COMMAND_LENGTH + 2;
                    text += 4;
                    NJ_ZOOM_COMMAND(PUT_BOTH, defaultZoomDigits[0], defaultZoomDigits[1],
                                    defaultZoomDigits[2], defaultZoomDigits[3]);
                    lineLength += NJ_ZOOM_COMMAND_LENGTH;
                    out += NJ_ZOOM_COMMAND_LENGTH;
                    PUT_BOTH(0, ' ');
                    widthCode.bytes[3] = NJ_TEXT_WIDTH;
                    PUT_BOTH(1, widthCode.bytes[0]);
                    PUT_BOTH(2, widthCode.bytes[1]);
                    PUT_BOTH(3, widthCode.bytes[2]);
                    PUT_BOTH(4, widthCode.bytes[3]);
                    lineLength += 1 + NJ_WIDTH_CODE_LENGTH;
                    out += 1 + NJ_WIDTH_CODE_LENGTH;
                    PUT_BOTH(0, fixedStyle.bytes[0]);
                    PUT_BOTH(1, fixedStyle.bytes[1]);
                    lineLength += NJ_STYLE_CODE_LENGTH;
                    out += NJ_STYLE_CODE_LENGTH;
                    displayLength += NJ_STYLE_CODE_LENGTH + (NJ_WIDTH_CODE_LENGTH + 1) + (NJ_ZOOM_COMMAND_LENGTH + 2)
                                   + NJ_ZOOM_COMMAND_LENGTH + (1 + NJ_WIDTH_CODE_LENGTH) + NJ_STYLE_CODE_LENGTH;
                } else {
                    if (NJ_STRING_LINE_VISIBLE(string)) {
                        out[0] = text[0];
                        displayLength += 2;
                        out[1] = text[1];
                        out += 2;
                    }
                    lineText[lineLength] = text[0];
                    lineText[lineLength + 1] = text[1];
                    lineLength += 2;
                    text += 2;
                }
                break;
            case 1:
                if (!proportional) {
                    out[0] = proportionalStyle.bytes[0];
                    out[1] = proportionalStyle.bytes[1];
                    lineText[lineLength] = proportionalStyle.bytes[0];
                    lineText[lineLength + 1] = proportionalStyle.bytes[1];
                    lineLength += NJ_STYLE_CODE_LENGTH;
                    displayLength += NJ_STYLE_CODE_LENGTH;
                    out += NJ_STYLE_CODE_LENGTH;
                    proportional = 1;
                }
                if (text[0] == '\\' && text[1] == 'n') {
                    displayLength++;
                    text += TAG_LENGTH("\\n");
                    *out++ = '\n';
                    if (colorSpanOpen) {
                        displayLength += NJ_COLOR_COMMAND_LENGTH;
                        out[0] = colorSpanTag[0];
                        out[1] = colorSpanTag[1];
                        out[2] = colorSpanTag[2];
                        out[3] = colorSpanTag[3];
                        out[4] = colorSpanTag[4];
                        out[5] = colorSpanTag[5];
                        out[6] = colorSpanTag[6];
                        out[7] = colorSpanTag[7];
                        out[8] = colorSpanTag[8];
                        out[9] = colorSpanTag[9];
                        out[10] = colorSpanTag[10];
                        out += NJ_COLOR_COMMAND_LENGTH;
                    }
                    if (zoomSpanOpen) {
                        displayLength += NJ_ZOOM_COMMAND_LENGTH;
                        NJ_ZOOM_COMMAND(PUT_DISPLAY, zoomSpanDigits[0], zoomSpanDigits[1],
                                        zoomSpanDigits[2], zoomSpanDigits[3]);
                        out += NJ_ZOOM_COMMAND_LENGTH;
                    }
                    penOffsetX = 0;
                    string->currentLine++;
                    memset(lineText, 0, sizeof(lineText));
                    lineLength = 0;
                    lineIndex++;
                    if (NJ_STRING_LINE_VISIBLE(string)) {
                        displayLength += NJ_STYLE_CODE_LENGTH;
                        out[0] = proportionalStyle.bytes[0];
                        out[1] = proportionalStyle.bytes[1];
                        out += NJ_STYLE_CODE_LENGTH;
                    }
                    lineText[lineLength] = proportionalStyle.bytes[0];
                    lineText[lineLength + 1] = proportionalStyle.bytes[1];
                    if (NJ_STRING_LINE_VISIBLE(string)) {
                        displayLength += 2;
                        out[0] = lineStartX.bytes[0];
                        out[1] = lineStartX.bytes[1];
                        out += 2;
                    }
                    lineText[lineLength + 2] = lineStartX.bytes[0];
                    lineText[lineLength + 3] = lineStartX.bytes[1];
                    lineText[lineLength + 4] = defaultScale.bytes[0];
                    lineText[lineLength + 5] = defaultScale.bytes[1];
                    lineText[lineLength + 6] = defaultScale.bytes[2];
                    lineLength += 7;
                } else if (text[0] == '<') {
                    if (text[1] == 'c' && text[2] == 'o' && text[3] == 'l' && text[4] == 'e' && text[5] == '>') {
                        /* <cole>: back to the text color, gray c0c0c0. */
                        NJ_COLOR_COMMAND(PUT_DISPLAY, 'c', '0', 'c', '0', 'c', '0');
                        out += NJ_COLOR_COMMAND_LENGTH;
                        displayLength += NJ_COLOR_COMMAND_LENGTH;
                        text += TAG_LENGTH("<cole>");
                        colorSpanOpen = 0;
                    } else if (text[1] == 'c' && text[2] == 'o' && text[3] == 'l') {
                        /* <colBBGGRR> */
                        displayLength += NJ_COLOR_COMMAND_LENGTH;
                        colorSpanOpen = 1;
                        NJ_COLOR_COMMAND(PUT_DISPLAY, text[4], text[5], text[6], text[7], text[8], text[9]);
                        out += NJ_COLOR_COMMAND_LENGTH;
                        NJ_COLOR_COMMAND(PUT_COLOR_SPAN, text[4], text[5], text[6], text[7], text[8], text[9]);
                        text += TAG_LENGTH("<colBBGGRR>");
                    } else if (text[1] == 'x') {
                        /* <x+N>, <x-N>, <xN>: "$axN;" moves the pen to x N, and the line is measured from there. */
                        int negative = 0;
                        int penX = 0;
                        unsigned char digit;
                        unsigned char *scan;

                        if (NJ_STRING_LINE_VISIBLE(string)) {
                            out[0] = '$';
                            out[1] = 'a';
                            out[2] = 'x';
                            displayLength += 3;
                            out += 3;
                        }
                        lineText[lineLength] = '$';
                        lineText[lineLength + 1] = 'a';
                        lineText[lineLength + 2] = 'x';
                        text += TAG_LENGTH("<x");
                        lineLength += 3;
                        if (*text == '-' || *text == '+') {
                            negative = *text == '-';
                            if (NJ_STRING_LINE_VISIBLE(string)) {
                                *out++ = *text;
                                displayLength++;
                            }
                            lineText[lineLength++] = *text++;
                        }
                        scan = text;
                        if ((digit = *scan++) != '>' && (unsigned char)(digit - '0') < 10) {
                            for (;;) {
                                penX = penX * 10 + digit - '0';
                                if (NJ_STRING_LINE_VISIBLE(string)) {
                                    displayLength++;
                                    *out++ = *text;
                                }
                                lineText[lineLength++] = *text++;
                                if ((digit = *scan++) == '>' || (unsigned char)(digit - '0') >= 10) {
                                    break;
                                }
                            }
                        }
                        if (negative) {
                            penX = -penX;
                        }
                        if (NJ_STRING_LINE_VISIBLE(string)) {
                            *out++ = ';';
                            displayLength++;
                        }
                        lineText[lineLength] = ';';
                        lineLength = 0;
                        penOffsetX = penX - string->x;
                        text++;
                        memset(lineText, 0, sizeof(lineText));
                    } else if (text[1] == 'r' && text[2] == 'u' && text[3] == 'b' && text[4] == 'y' && text[5] == 's' && text[6] == '>') {
                        if (NJ_STRING_LINE_VISIBLE(string)) {
                            displayLength += NJ_RUBY_COMMAND_LENGTH + 3;
                            NJ_RUBY_COMMAND(PUT_DISPLAY, 's');
                            out += NJ_RUBY_COMMAND_LENGTH;
                            out[0] = defaultScale.bytes[0];
                            out[1] = defaultScale.bytes[1];
                            out[2] = defaultScale.bytes[2];
                            out += 3;
                        }
                        text += TAG_LENGTH("<rubys>");
                    } else if (text[1] == 'r' && text[2] == 'u' && text[3] == 'b' && text[4] == 'y' && text[5] == 'e' && text[6] == '>') {
                        if (NJ_STRING_LINE_VISIBLE(string)) {
                            displayLength += NJ_RUBY_COMMAND_LENGTH + 3;
                            NJ_RUBY_COMMAND(PUT_DISPLAY, 'e');
                            out += NJ_RUBY_COMMAND_LENGTH;
                            out[0] = defaultScale.bytes[0];
                            out[1] = defaultScale.bytes[1];
                            out[2] = defaultScale.bytes[2];
                            out += 3;
                        }
                        text += TAG_LENGTH("<rubye>");
                    } else if (text[1] == 'r' && text[2] == 'u' && text[3] == 'b' && text[4] == 'y' && text[5] == 'b' && text[6] == '>') {
                        if (NJ_STRING_LINE_VISIBLE(string)) {
                            displayLength += NJ_RUBY_COMMAND_LENGTH + 3;
                            NJ_RUBY_COMMAND(PUT_DISPLAY, 'b');
                            out += NJ_RUBY_COMMAND_LENGTH;
                            out[0] = defaultScale.bytes[0];
                            out[1] = defaultScale.bytes[1];
                            out[2] = defaultScale.bytes[2];
                            out += 3;
                        }
                        text += TAG_LENGTH("<rubyb>");
                    } else if (text[1] == 's' && text[2] == 'i' && text[3] == 'z' && text[4] == 'e' && text[5] == 'e' && text[6] == '>') {
                        /* <sizee>: back to the string's zoom. */
                        if (NJ_STRING_LINE_VISIBLE(string)) {
                            displayLength += NJ_ZOOM_COMMAND_LENGTH;
                            NJ_ZOOM_COMMAND(PUT_DISPLAY, defaultZoomDigits[0], defaultZoomDigits[1],
                                            defaultZoomDigits[2], defaultZoomDigits[3]);
                            out += NJ_ZOOM_COMMAND_LENGTH;
                        }
                        text += TAG_LENGTH("<sizee>");
                        zoomSpanOpen = 0;
                    } else if (text[1] == 's' && text[2] == 'i' && text[3] == 'z' && text[4] == 'e') {
                        /* <sizeNN>: zoom NN on both axes. */
                        if (NJ_STRING_LINE_VISIBLE(string)) {
                            displayLength += NJ_ZOOM_COMMAND_LENGTH;
                            NJ_ZOOM_COMMAND(PUT_DISPLAY, text[5], text[6], text[5], text[6]);
                            out += NJ_ZOOM_COMMAND_LENGTH;
                        }
                        NJ_ZOOM_COMMAND(PUT_LINE, text[5], text[6], text[5], text[6]);
                        lineLength += NJ_ZOOM_COMMAND_LENGTH;
                        zoomSpanOpen = 1;
                        zoomSpanDigits[0] = text[5];
                        zoomSpanDigits[1] = text[6];
                        zoomSpanDigits[2] = text[5];
                        zoomSpanDigits[3] = text[6];
                        text += TAG_LENGTH("<sizeNN>");
                    } else if (text[1] == 0xA1 && text[2] == 0xFB) {            /* <○> */
                        NJ_STRING_PUT_ICON(NJ_STRING_ICON_CIRCLE, 20);
                    } else if (text[1] == 0xA1 && text[2] == 0xDF) {            /* <×> */
                        NJ_STRING_PUT_ICON(NJ_STRING_ICON_CROSS, 20);
                    } else if (text[1] == 0xA2 && text[2] == 0xA2) {            /* <□> */
                        NJ_STRING_PUT_ICON(NJ_STRING_ICON_SQUARE, 20);
                    } else if (text[1] == 0xA2 && text[2] == 0xA4) {            /* <△> */
                        NJ_STRING_PUT_ICON(NJ_STRING_ICON_TRIANGLE, 20);
                    } else if (text[1] == 'L' && text[2] == '1') {
                        NJ_STRING_PUT_ICON(NJ_STRING_ICON_L1, 24);
                    } else if (text[1] == 'L' && text[2] == '2') {
                        NJ_STRING_PUT_ICON(NJ_STRING_ICON_L2, 24);
                    } else if (text[1] == 'R' && text[2] == '1') {
                        NJ_STRING_PUT_ICON(NJ_STRING_ICON_R1, 24);
                    } else if (text[1] == 'R' && text[2] == '2') {
                        NJ_STRING_PUT_ICON(NJ_STRING_ICON_R2, 24);
                    } else if (text[1] == 'A' && text[2] == 'T' && text[3] == 'R') {
                        /* <ATRn>: an element attribute icon, 2 pixels right of the pen; always shown. */
                        int icon;

                        if (text[4] == '0') {
                            icon = NJ_STRING_ICON_ATTRIBUTE(0);
                        } else if (text[4] == '1') {
                            icon = NJ_STRING_ICON_ATTRIBUTE(1);
                        } else if (text[4] == '2') {
                            icon = NJ_STRING_ICON_ATTRIBUTE(2);
                        } else if (text[4] == '3') {
                            icon = NJ_STRING_ICON_ATTRIBUTE(3);
                        } else if (text[4] == '4') {
                            icon = NJ_STRING_ICON_ATTRIBUTE(4);
                        } else {
                            icon = 0;
                        }
                        if (string->iconCount < NJ_STRING_MAX_ICONS) {
                            lineText[lineLength] = '\0';
                            iconX = xglFontGetStringWidth(lineText) + penOffsetX;
                            string->icons[string->iconCount].line = lineIndex;
                            string->icons[string->iconCount].button = icon;
                            string->icons[string->iconCount].x = iconX + 2;
                            string->iconCount++;
                        }
                        displayLength += NJ_ICON_BLANK_LENGTH;
                        NJ_ICON_BLANK(PUT_DISPLAY, 16);
                        out += NJ_ICON_BLANK_LENGTH;
                        NJ_ICON_BLANK(PUT_LINE, 16);
                        lineLength += NJ_ICON_BLANK_LENGTH;
                        text += TAG_LENGTH("<ATR0>");
                    } else {
                        /* Any other '<' opens a highlighted span, closed by '>'. */
                        displayLength += NJ_COLOR_COMMAND_LENGTH;
                        text++;
                        NJ_COLOR_COMMAND(PUT_DISPLAY, gNjStringHighlightRgb[4], gNjStringHighlightRgb[5], gNjStringHighlightRgb[2], gNjStringHighlightRgb[3],
                                         gNjStringHighlightRgb[0], gNjStringHighlightRgb[1]);
                        out += NJ_COLOR_COMMAND_LENGTH;
                        colorSpanOpen = 1;
                        NJ_COLOR_COMMAND(PUT_COLOR_SPAN, gNjStringHighlightRgb[4], gNjStringHighlightRgb[5], gNjStringHighlightRgb[2], gNjStringHighlightRgb[3],
                                         gNjStringHighlightRgb[0], gNjStringHighlightRgb[1]);
                    }
                } else if (text[0] == '>') {
                    /* End of a highlighted span: back to the text color. */
                    displayLength += NJ_COLOR_COMMAND_LENGTH;
                    NJ_COLOR_COMMAND(PUT_DISPLAY, 'c', '0', 'c', '0', 'c', '0');
                    text++;
                    colorSpanOpen = 0;
                    out += NJ_COLOR_COMMAND_LENGTH;
                } else {
                    if (NJ_STRING_LINE_VISIBLE(string)) {
                        *out++ = text[0];
                        displayLength++;
                    }
                    lineText[lineLength++] = *text++;
                }
                break;
            }
        } while (*text != 0);
    }
    display[displayLength] = '\0';
    string->currentLine++;
}

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002DAA08);

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002DAEE8);

s32 func_002D6CA8(NjString *string, s32 limit, char *display);

/* Lays out a scratch string at (x, y) with zoom 15,15 and returns its layoutCount. */
s32 njStringLayoutCount(s32 limit, s32 x, s32 y) {
    char display[0x800];
    s32 layoutStringWords[0x40];

    NjString *layoutString = (NjString *) &layoutStringWords[0];

    njStringSetZoom(layoutString, 0xF, 0xF);
    njStringSetFlags(layoutString, 8);
    njStringSetCounter(layoutString, limit);
    njStringSetPosition(layoutString, x, y);
    func_002D6CA8(layoutString, limit, display);
    return layoutString->layoutCount;
}

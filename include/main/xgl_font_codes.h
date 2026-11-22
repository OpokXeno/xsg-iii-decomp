/* header_harvest: hand-maintained (font control codes shared by xgl_font.c and njString.c; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_MAIN_XGL_FONT_CODES_H
#define INCLUDE_MAIN_XGL_FONT_CODES_H

/*
 * Control codes embedded in queued text. Bytes below 32 start a command whose length
 * xglFontGetSPcodeSize returns; the compact position codes 1..5 are the high byte of an
 * X (1..3) or Y (4..5) coordinate.
 */
typedef enum FontOpcode {
    FONT_OP_POS_X_0 = 1,
    FONT_OP_POS_X_1 = 2,
    FONT_OP_POS_X_2 = 3,
    FONT_OP_POS_Y_0 = 4,
    FONT_OP_POS_Y_1 = 5,
    FONT_OP_DEPTH = 6,
    FONT_OP_NOP = 7,
    FONT_OP_POSITION = 8,
    FONT_OP_TAB = 9,
    FONT_OP_NEWLINE = 10,
    FONT_OP_RESET = 11,
    FONT_OP_COLOR = 12,
    FONT_OP_EFFECT_MODE = 13,
    FONT_OP_EFFECT_PARAMS = 14,
    FONT_OP_BLEND = 15,
    FONT_OP_HEX_DIGITS = 16,
    FONT_OP_NUMBER = 17,
    FONT_OP_SCALE = 18,
    FONT_OP_EFFECT_PACKET = 19,
    FONT_OP_CONFIG = 21,
    FONT_OP_INCLUDE = 22,
    FONT_OP_CALLBACK = 23,
    FONT_OP_COMPACT = 24,
    FONT_OP_STYLE = 25,
    FONT_OP_GLYPH = 27
} FontOpcode;

/* FONT_OP_CONFIG <length> <sub-command> <value...>: the sub-commands (X2 xgl_font.c). */
#define FONT_CONFIG_LINE_HEIGHT 1
#define FONT_CONFIG_DEFAULT_WIDTH 2
#define FONT_CONFIG_WRAP_WIDTH 3
#define FONT_CONFIG_PROPORTIONAL_MODE 4
#define FONT_CONFIG_HORIZONTAL_SHIFT 5

/* FONT_OP_STYLE <bits>: proportional spacing, and ASCII drawn with the wide (EUC) glyphs. */
#define FONT_STYLE_PROPORTIONAL 1
#define FONT_STYLE_WIDE_ASCII 2

#endif

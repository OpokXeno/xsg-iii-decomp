#include "common.h"
#include "shared.h"

struct XglPacket;

/* Packet that FONT_OP_EFFECT_PACKET builds in the cursor's work area: an A+D GIF tag and one
 * register write. */
struct FontEffectScratchPacket {
    u64 vif_tag;
    u64 vif_count;
    u64 effect_state;
    u64 payload_count;
};

typedef struct FontDrawCursor { struct XglPacket *packet; short origin_x; unsigned char unmodeled_06[2]; unsigned short x; unsigned short y; int depth; unsigned short offset_x; unsigned short offset_y; unsigned char red; unsigned char green; unsigned char blue; unsigned char unmodeled_17; unsigned char flags; unsigned char effect; unsigned char attributes; unsigned char draw_state_high; unsigned char unmodeled_1c[4]; signed char effect_x; signed char effect_y; unsigned char unmodeled_22[2]; unsigned char effect_red; unsigned char effect_green; unsigned char effect_blue; unsigned char effect_alpha; /* The GS ALPHA_1 value: the A/B/C/D selectors and FIX. */ unsigned char alpha_abcd; unsigned char alpha_fix; unsigned char alpha; unsigned char unmodeled_2b; unsigned short scale_x; unsigned short scale_y; union { struct FontEffectScratchPacket effect_packet; u64 qwords[(0x4000 - 0x30) / sizeof(u64)]; } work; } FontDrawCursor;

int xglFontGetStringWidth(const char *text);
int xglFontGetSPcodeSize(int code, const unsigned char *text);
const char *xglFontCopyTextUnit(char *word, const char *text, int *length);

#define GS_ALPHA_1 0x42
/* GIFtag register descriptor of an A+D (address plus data) register. */
#define GIF_AD 0x0E
#include "main/Packet.h"

void sceVif1PkAddDirectDataN(XglPacket *packet, const void *data, unsigned int qwc);
void *sceVif1PkReserve(XglPacket *packet, unsigned int words);

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

/* FontState.text_mode bits, set by the $prop and $han inline commands. */
#define FONT_MODE_PROP 1
#define FONT_MODE_HAN 2
/* Glyph indices below this have proportional bounds; others advance a fixed FONT_GLYPH_ADVANCE. */
#define FONT_PROPORTIONAL_GLYPHS 0x180
#define FONT_GLYPH_ADVANCE 19
/* EUC-JP code points and rows. */
#define FONT_JIS_ROW_SIZE 94
#define EUC_FIRST 0xa1
#define EUC_ROW_SYMBOLS 0xa1
#define EUC_ROW_ALNUM 0xa3
#define EUC_ROW_HIRAGANA 0xa4
#define EUC_ROW_KATAKANA 0xa5
#define EUC_ROW_GREEK 0xa6
#define EUC_ROW_NEC_SPECIAL 0xad
#define EUC_ROW_SYMBOLS_2 0xa2
#define EUC_ROW_KANJI_LEVEL1 0xb0
#define EUC_ROW_KANJI_LEVEL2 0xd0
#define EUC_SPACE 0xa1a1

struct FontImage {
    unsigned char kanji_texture_0[0x3c020];
    unsigned char kanji_texture_1[0x3c020];
    unsigned short proportional_widths[256];
};

extern unsigned char spCodeParameterSizeTable[];
extern unsigned char spcode_0[32];
int xglFontGetStringWidth2(const char *text, int delimiter);
/* Defined here, not with the data above: moving it before the functions that use it changes the original code or data order. */
extern unsigned char spCodeParameterSizeTable[8];

/*
 * The FontState fields from offset 4 on, read through the scaffold symbol fontStateFromWindow (FS + 4),
 * up to the queue write cursor that sits just below the print-queue slot array. The queue
 * functions write their command bytes through queue_cursor; buffer_reset points it
 * at the end of the slot array, where the first record goes.
 */
struct FontStateFromWindow {
    void *window_image;
    unsigned short flags;
    unsigned char font_selection;
    unsigned char debug_mode;
    unsigned short line_height;
    unsigned short character_spacing;
    unsigned char default_width;
    unsigned char text_mode;
    unsigned char proportional_mode;
    unsigned char horizontal_shift;
    int stored_value;
    /* "$rubyb;" marks where the base text of a ruby annotation starts. */
    const unsigned char *ruby_base;
    /* From the end of the annotation back to the end of its base text; set by "$rubys;". */
    unsigned short ruby_advance;
    /* How far the annotation is raised above the line, besides half its glyph height ("$rubyy"). */
    unsigned short ruby_offset_y;
    /* The default glyph scale in 1/16 units (FONT_SCALE_ONE each); xglFontSetTextureFilter tests both bytes at once. */
    union {
        unsigned short default_scale;
        struct { signed char compact_width; signed char compact_height; };
    };
    unsigned char unmodeled_1e[6];
    /* Scratch copy of one word while xglFontFitWords measures a line. */
    char word_buffer[0x100];
    unsigned char *queue_cursor;
};

extern struct FontStateFromWindow fontStateFromWindow;
int xglFontIsTextureGlyphCode(unsigned char high, unsigned char low);

#define FONT_CURSOR_PROPORTIONAL 2
#define FONT_CURSOR_WIDE_ASCII 4
/* Second half-width mode of the $han inline command ("n2"). */
#define FONT_CURSOR_WIDE_ALL 8

/* One A+D register write after the GIF tag xglFontAllocGifPacket writes. */
struct FontAlphaWrite {
    unsigned int alpha_abcd;
    unsigned int alpha_fix;
    unsigned int register_address;
    unsigned int register_address_high;
};

#define FONT_SCALE_ONE 16
#define FONT_SCALE_PAIR_ONE ((FONT_SCALE_ONE << 16) | FONT_SCALE_ONE)
#define FONT_DEFAULT_SCALE_ONE ((FONT_SCALE_ONE << 8) | FONT_SCALE_ONE)

/* The two texture-filter packets: 1:1 glyphs and scaled glyphs. */
extern unsigned int nearest_5[8];
extern unsigned int biliner_6[8];
void xglFontSetTextureFilter(FontDrawCursor *cursor, int mode);
/* Advance of each printable ASCII character (32..127) in the debug font. */
extern int fontAsciiWidthTable[];
int xglFontParseNumber(const char **cursor, int *value);
unsigned short xglFontCharToEuc(signed char character, unsigned char **text);
unsigned int xglFontGetDebugCell(unsigned char high, unsigned char low);
unsigned short xglFontGetGlyphIndex(unsigned char high, unsigned char low);
/* * Reads a hex number up to ';' or ',' and moves the text cursor past the delimiter; returns 0 on any * other character. */
int xglFontParseHex(const char **cursor, int *value);

/* Texture coordinate / size / draw-flag arguments of xglFontFlushSub. */
#define FONT_ASCII_V_LOW 0x2f00
#define FONT_ASCII_V_HIGH 0x2f80
#define FONT_SYMBOL_V 0x2e80

typedef int (*FontInlineHandler)(FontDrawCursor *cursor, const char **text);

/* One entry of the inline "$name" command table; the table ends with a null name. */
struct FontInlineCommand {
    const char *name;
    FontInlineHandler handler;
};

/* ax ay az rx ry init save load col bg shadow alpha zoom xx ruby prop han mark */
extern struct FontInlineCommand fontInlineCommands[];
/* "[*error*:%c%c%c%c]\n" */
extern const char fontInlineErrorFormat[24];
int printf(const char *format, ...);
/* Two-byte code pairs, terminated by a 0 first byte. */
extern unsigned char fontPairGlyphCodes[];
int xglFontFindPairGlyph(unsigned char high, unsigned char low);

/* Glyph index of codes the font does not have (also xglFontFindPairGlyph's miss). */
#define FONT_GLYPH_NONE 0x7F
/* The level-1 kanji glyphs follow the 0x400 symbol and kana glyphs. */
#define FONT_GLYPH_KANJI_BASE 0x400

int xglFontInlineCommand(FontDrawCursor *cursor, unsigned char **text);
/* The kana cell codes of the debug font, one per EUC column (copied to the stack before use). */
extern const unsigned char kanaFontCodeTable[104];

/* The glyph rows of the debug font texture: V of a glyph cell is its row << 23. */
#define FONT_GLYPH_ROW(row) ((unsigned int)(row) << 23)
/* The blank cell: last row of the high ASCII block. */
#define FONT_GLYPH_BLANK (FONT_GLYPH_ROW(0x3F) + FONT_ASCII_V_HIGH)
/* xglFontGetProportionalSize's packed result: the glyph's left texel bound in
 * the low byte, its right bound in the high byte (the caller draws right - left
 * texels from left on). */
#define FONT_BOUNDS(left, right) (((right) << 8) | (left))
/* The fixed bounds while proportional_mode is set (XS1 computes a pair here). */
#define FONT_FIXED_BOUNDS FONT_BOUNDS(6, 15)

int xglFontGetProportionalSize(int code);
/* ASCII character to EUC-JP code, indexed by the raw character; a control character becomes a full-width space. */
extern unsigned short asciiEucTable[];
extern unsigned short asciiEucTable[128];
extern int fontAsciiWidthTable[98];
extern struct FontInlineCommand fontInlineCommands[20];

unsigned short xglFontCharToEuc(signed char character, unsigned char **text)
{
    if (character < 0x20) {
        return EUC_SPACE;
    }
    return asciiEucTable[character];
}

/* Two byte-pair tables of (high, low) codes, zero-terminated; D_007E7E06 selects the first one when set. */
extern unsigned char D_004B2F50[];
extern unsigned char D_004B3020[];
extern unsigned char D_007E7E06[];

/* Index of the first pair equal to (high, low) plus this base; FONT_GLYPH_NONE when no pair matches. */
#define FONT_PAIR_GLYPH_BASE 3989

int func_00181D40(unsigned char high, unsigned char low)
{
    const unsigned char *pair;
    int index;

    if (D_007E7E06[0] == 0) {
        pair = D_004B3020;
    } else {
        pair = D_004B2F50;
    }
    for (index = 0; pair[0] != 0; index++, pair += 2) {
        if (high == pair[0] && low == pair[1]) {
            return index + FONT_PAIR_GLYPH_BASE;
        }
    }
    return FONT_GLYPH_NONE;
}

int xglFontGetProportionalSize(int code)
{
    if (code >= 192 && code < 384) {
        return code;
    }
    switch (code & 0xfff) {
    case 0:
        return 0;
    case 5:
        return 382;
    case 27:
    case 60:
        return 381;
    case 128:
    case 129:
    case 130:
    case 131:
    case 132:
    case 133:
    case 134:
    case 135:
    case 136:
    case 137:
    case 138:
    case 139:
    case 140:
        return code + 147;
    default:
        return 383;
    }
}

int xglFontIsTextureGlyphCode(unsigned char high, unsigned char low)
{
    if (low == 0) {
        return 0;
    }
    if (high == 164 || high == 166) {
        return 1;
    }
    if (high == 161 && (low == 189 || low == 218 || low == 219 || low == 223 || low == 238 || low == 249 ||
                        low == 250 || low == 251 || low == 252)) {
        return 1;
    }
    if (high == 162 && ((unsigned char) (low + 94) < 5 || low == 170 || low == 171 || low == 172 || low == 173 ||
                        low == 246)) {
        return 1;
    }
    if (high == 163 && low >= 176 && low < 186) {
        return 1;
    }
    if (high == 173 && (unsigned char) (low + 75) < 3) {
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00181FA0);

/*
 * Reserves count A+D writes in the cursor's packet and writes their GIF tag: NLOOP = count with
 * the EOP bit 0x8000, one register (NREG = 1 in the upper word) and the register list GIF_AD.
 * Returns the address 16 bytes in.
 */
void *xglFontAllocGifPacket(FontDrawCursor *cursor, int count)
{
    unsigned int *packet;

    packet = sceVif1PkReserve(cursor->packet, count * 4 + 4);
    packet[0] = count + 0x8000;
    packet[1] = 0x10000000;
    packet[2] = GIF_AD;
    return packet + 4;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_001820D0);

/* The two texture-filter packets: 1:1 glyphs and scaled glyphs. */
extern unsigned int D_004B3110[8];
extern unsigned int D_004B3130[8];

struct FontStateQueue { unsigned char *texture_buffer; unsigned short flags; unsigned char font_selection; unsigned char unmodeled_07[5]; unsigned char default_width; unsigned char debug_mode; unsigned short unmodeled_0e; unsigned char unmodeled_10; unsigned char unmodeled_11[3]; unsigned char unmodeled_14; unsigned char unmodeled_15[7]; /* Text address of the include command that prefixes each debug line; 0 when none is set. */ unsigned int debug_prefix; unsigned char unmodeled_20[6]; unsigned short ruby_offset_y; unsigned char compact_width; unsigned char compact_height; unsigned char unmodeled_2a[6]; unsigned char word_buffer[256]; unsigned char *queue_cursor; };

extern struct FontStateQueue D_007E7E00;

/* The FontState halfwords the texture filter reads: the flags at +4 and the default glyph scale at +40. */
struct FontStateFilter {
    unsigned char unmodeled_00[4];
    unsigned short flags;
    unsigned char unmodeled_06[34];
    unsigned short default_scale;
};

void xglFontSetTextureFilter(FontDrawCursor *cursor, int mode)
{
    int scaled;

    if (mode == 0) {
        scaled = ((struct FontStateFilter *) &D_007E7E00)->default_scale != FONT_DEFAULT_SCALE_ONE;
    } else {
        scaled = *(unsigned int *)&cursor->scale_x != FONT_SCALE_PAIR_ONE;
    }
    if (!(((struct FontStateFilter *) &D_007E7E00)->flags & 0x10)) {
        scaled = 0;
    }
    if (!scaled) {
        sceVif1PkAddDirectDataN(cursor->packet, D_004B3110, 2);
    } else {
        sceVif1PkAddDirectDataN(cursor->packet, D_004B3130, 2);
    }
}

/* Flags of a position command: which of x, y and depth follow. */
#define FONT_POS_SET_X 0x01
#define FONT_POS_SET_Y 0x02
#define FONT_POS_SET_DEPTH 0x04

struct FontPositionCommand {
    unsigned char opcode;
    unsigned char flags;
    unsigned char x_high;
    unsigned char x_low;
    unsigned char y_high;
    unsigned char y_low;
    unsigned char color_high;
    unsigned char color_middle;
    unsigned char color_low;
};

/* Appends a 9-byte FONT_OP_POSITION command (x, y and color) to the queue. */
void xglFontAppendPosition(int x, int y, int color)
{
    struct FontPositionCommand *command = (struct FontPositionCommand *) D_007E7E00.queue_cursor;

    command->opcode = FONT_OP_POSITION;
    command->flags = FONT_POS_SET_X | FONT_POS_SET_Y | FONT_POS_SET_DEPTH;
    command->x_high = x >> 8;
    command->x_low = x;
    command->y_high = y >> 8;
    command->y_low = y;
    command->color_high = color >> 16;
    command->color_middle = color >> 8;
    command->color_low = color;
    D_007E7E00.queue_cursor += sizeof(*command);
}

int xglFontGetSPcodeSize(int code, const unsigned char *text)
{
    unsigned char size;

    code &= 0xff;
    size = spcode_0[code];

    if (size == 255) {
        switch (code) {
        case FONT_OP_POSITION: {
            unsigned char parameter_sizes[8];
            unsigned char flags = text[1];
            int bit;

            __builtin_memcpy(parameter_sizes, spCodeParameterSizeTable,
                             sizeof(parameter_sizes));
            size = 1;
            for (bit = 0; bit < 8; bit++) {
                if (flags & 1) {
                    size += parameter_sizes[bit];
                }
                flags >>= 1;
            }
            break;
        }
        case FONT_OP_CONFIG:
            size = text[1] + 1;
            break;
        }
    }

    return size;
}

#define FONT_NUMERIC_FLOAT 0xff

struct FontNumericCommand {
    unsigned char opcode;
    unsigned char radix;
    unsigned char digits_minus_one;
    unsigned char value_high;
    unsigned char value_mid_high;
    unsigned char value_mid_low;
    unsigned char value_low;
};

float func_0030A4E0(unsigned long long value);

void xglFontPrintSub(void *args)
{
    unsigned long long *list = (unsigned long long *)args;
    const char *text = (const char *)(int)list[0];
    unsigned char ahead = *text;

    list++;
    for (;;) {
        unsigned int code;

        if (*text == 0) {
            break;
        }
        code = *text & 0xff;

        if (code < 32) {
            int remaining = xglFontGetSPcodeSize(code, (const unsigned char *)text);

            for (; remaining >= 0; remaining--) {
                *fontStateFromWindow.queue_cursor++ = *text++;
            }
        } else if (code == '%') {
            int width = 0;
            unsigned int radix;

            text++;
            for (;;) {
                code = (unsigned char)*text++;

                switch (code) {
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                    width = width * 10 + (code - '0');
                    continue;
                case 'b':
                    radix = 2;
                    goto numeric_tail;
                case 'd':
                    radix = 10;
                    goto numeric_tail;
                case 'x':
                    radix = 16;
                numeric_tail:
                    {
                        unsigned long long value = (unsigned int)list[0];
                        struct FontNumericCommand *command = (struct FontNumericCommand *)D_007E7E00.queue_cursor;

                        list++;
                        if (width == 0) {
                            width = 1;
                        }
                        command->opcode = FONT_OP_NUMBER;
                        command->radix = radix;
                        command->digits_minus_one = (unsigned char)(width - 1);
                        command->value_high = value >> 24;
                        command->value_mid_high = value >> 16;
                        command->value_mid_low = value >> 8;
                        command->value_low = value;
                        D_007E7E00.queue_cursor += sizeof(*command);
                    }
                    break;
                case 'f':
                    {
                        float value = func_0030A4E0(list[0]);
                        unsigned int bits = *(unsigned int *)&value;
                        struct FontNumericCommand *command = (struct FontNumericCommand *)D_007E7E00.queue_cursor;

                        list++;
                        command->opcode = FONT_OP_NUMBER;
                        command->radix = FONT_NUMERIC_FLOAT;
                        command->digits_minus_one = 0xff;
                        command->value_high = bits >> 24;
                        command->value_mid_high = bits >> 16;
                        command->value_mid_low = bits >> 8;
                        command->value_low = bits;
                        D_007E7E00.queue_cursor += sizeof(*command);
                    }
                    break;
                case 's': {
                    const unsigned char *string = (const unsigned char *)(int)list[0];
                    int terminator_test;

                    list++;
                    while ((terminator_test = *string) != 0) {
                        *fontStateFromWindow.queue_cursor++ = *string++;
                    }
                    break;
                }
                case 'c':
                    code = (unsigned char)list[0];
                    list++;
                    /* fall through */
                case '$':
                case '%':
                    *D_007E7E00.queue_cursor++ = code;
                    break;
                default:
                    break;
                }
                break;
            }
        } else {
            text++;
            *D_007E7E00.queue_cursor++ = ahead;
        }
        ahead = *text;
    }
    *D_007E7E00.queue_cursor++ = 0;
}

void xglFontPrintDirectCore(const char *text)
{
    while (*text != 0) {
        unsigned char code = *text;

        if (code < 32) {
            int remaining = xglFontGetSPcodeSize(code, (const unsigned char *)text);

            while (remaining > 0) {
                *D_007E7E00.queue_cursor++ = *text++;
                remaining--;
            }
        }
        *D_007E7E00.queue_cursor++ = *text++;
    }
    *D_007E7E00.queue_cursor++ = 0;
}

void xglFontPrintSub(void *args);

void xglFontPrintf(int x, int y, unsigned int color, void *args)
{
    xglFontAppendPosition(x, y, color);
    xglFontPrintSub(args);
}

void xglFontPrintDirectCore(const char *text);

void xglFontPrint(int x, int y, unsigned int color, const char *text)
{
    xglFontAppendPosition(x, y, color);
    xglFontPrintDirectCore(text);
}

void func_00182708(const char *text)
{
    xglFontPrintDirectCore(text);
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00182720);

void xglFontDebugPrefixSet(int stored_value)
{
    fontStateFromWindow.stored_value = stored_value;
}

#include "gs.h"

/* The EE va_start spelling of the variadic debug print (as in the other variadic definitions of this tree). */
typedef char *va_list;

#define va_start(ap, last) ((ap) = (va_list)__builtin_next_arg(last) - (8 - __builtin_args_info(2)) * 8)

void xglFontDebugPrintf(int x, int y, const char *format, ...)
{
    unsigned long long arguments[32];
    va_list args;
    unsigned long long *list;
    unsigned long long *output;
    int count;

    if (!(D_007E7E00.flags & 0x2)) {
        return;
    }
    D_007E7E00.flags |= 0x80;
    output = arguments + 1;
    arguments[0] = (unsigned int) format;
    va_start(args, format);
    list = (unsigned long long *) args;
    count = 30;
    do {
        unsigned long long *argument = list++;
        unsigned long long value = *argument;

        count--;
        *output = value;
        output++;
    } while (count >= 0);
    xglFontAppendPosition(x << 1, y << 1, -1);
    if (D_007E7E00.debug_prefix != 0) {
        unsigned int text = D_007E7E00.debug_prefix;
        unsigned char *command = D_007E7E00.queue_cursor;

        command[0] = FONT_OP_INCLUDE;
        command[1] = text >> 24;
        command[2] = text >> 16;
        command[3] = text >> 8;
        command[4] = text;
        D_007E7E00.queue_cursor += 5;
    } else {
        *D_007E7E00.queue_cursor++ = FONT_OP_RESET;
    }
    xglFontPrintSub(arguments);
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", xglFontGetStringWidth2);

int xglFontGetStringWidth(const char *text)
{
    return xglFontGetStringWidth2(text, 0);
}

/*
 * Copies the next unit of `text` into `word` (the font state's word buffer when null): any control
 * codes with their parameters and "$...;" commands (a "$rubys;" block up to its "$rubye"), then one
 * character of one or two bytes. Stores the bytes taken in *length (if given) and returns the text
 * after them.
 */
const char *xglFontCopyTextUnit(char *word, const char *text, int *length)
{
    const unsigned char *cursor = (const unsigned char *)text;
    unsigned char high;

    if (word == 0) {
        word = fontStateFromWindow.word_buffer;
    }
    while (*cursor != 0) {
        if (*cursor == '$') {
            if (cursor[1] == 'r' && cursor[2] == 'u' && cursor[3] == 'b' && cursor[4] == 'y' &&
                cursor[5] == 's' && cursor[6] == ';') {
                const unsigned char *scan = cursor + 7;

                while (*scan != 0) {
                    if (scan[0] == '$' && scan[1] == 'r' && scan[2] == 'u' && scan[3] == 'b' &&
                        scan[4] == 'y' && scan[5] == 'e' && scan[6] == ';') {
                        cursor = scan;
                        break;
                    }
                    if (*scan < 0x20) {
                        scan += xglFontGetSPcodeSize(*scan, scan + 1);
                    }
                    scan++;
                }
            }
            while (*cursor != ';') {
                *word++ = *cursor++;
            }
        } else if (*cursor < 0x20) {
            int remaining = xglFontGetSPcodeSize(*cursor, cursor);

            while (remaining >= 0) {
                *word++ = *cursor++;
                remaining--;
            }
        } else {
            int wide;

            high = *cursor;
            wide = xglFontIsTextureGlyphCode(high, cursor[1]);
            *word = high;
            if (wide) {
                word[1] = cursor[1];
                cursor += 2;
            } else {
                cursor++;
            }
            break;
        }
    }
    if (length != 0) {
        *length = (const char *)cursor - text;
    }
    return (const char *)cursor;
}

int xglFontFitWords(const char *text, int *width)
{
    const char *cursor = text;
    int total = 0;

    while (*cursor != 0) {
        int length;
        const char *next = xglFontCopyTextUnit(D_007E7E00.word_buffer, cursor, &length);

        D_007E7E00.word_buffer[length] = 0;
        total += xglFontGetStringWidth2(D_007E7E00.word_buffer, 0);
        if (*width < total) {
            break;
        }
        cursor = next;
    }
    *width = total;
    return cursor - text;
}

/*
 * Reads a hex number up to ';' or ',' and moves the text cursor past the delimiter; returns 0 on any
 * other character.
 */
int xglFontParseHex(const char **cursor, int *value)
{
    const unsigned char *text = (const unsigned char *)*cursor;
    unsigned int result = 0;
    unsigned char ch = *text++;

    while (ch != ';' && ch != ',') {
        result = (result << 4) + ch;
        switch (ch) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            result -= '0';
            break;
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            result += 'a' - 'A';
            /* fall through */
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
            result -= 'a' - 10;
            break;
        default:
            return 0;
        }
        ch = *text++;
    }
    *value = result;
    *cursor = (const char *)text;
    return 1;
}

/*
 * Reads a number up to ';' or ',' and moves the text cursor past the delimiter: an optional sign, then
 * decimal digits or "0x" and a hex number (xglFontParseHex). Returns 0 on any other character.
 */
int xglFontParseNumber(const char **cursor, int *value)
{
    const unsigned char *text = (const unsigned char *)*cursor;
    int sign = 1;
    int result = 0;
    unsigned char ch;

    switch (text[0]) {
    case '-':
        sign = -1;
        /* fall through */
    case '+':
        text++;
        break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        break;
    default:
        return 0;
    }
    if (text[0] == '0' && text[1] == 'x') {
        *cursor = (const char *)(text + 2);
        if (!xglFontParseHex(cursor, &result)) {
            return 0;
        }
        *value = sign * result;
        return 1;
    }
    ch = *text++;
    while (ch != ';' && ch != ',') {
        if ((unsigned char)(ch - '0') >= 10) {
            return 0;
        }
        result = result * 10 + ch - '0';
        ch = *text++;
    }
    *cursor = (const char *)text;
    *value = sign * result;
    return 1;
}

/* Inline position command: the letter before the number picks x (also the line origin), y or depth. */
int xglFontCmdAx(FontDrawCursor *cursor, const char **text)
{
    const char *next = *text;
    unsigned char axis = next[-1];
    int value;

    if (!xglFontParseNumber(&next, &value)) {
        return 0;
    }
    switch (axis) {
    case 'x':
        cursor->x = value;
        cursor->origin_x = value;
        break;
    case 'y':
        cursor->y = value;
        break;
    case 'z':
        cursor->depth = value;
        break;
    default:
        return 0;
    }
    *text = next;
    return 1;
}

/* Inline move command: adds the number to x or y, picked by the letter before it. */
int xglFontCmdRx(FontDrawCursor *cursor, const char **text)
{
    const char *next = *text;
    unsigned char axis = next[-1];
    int value;

    if (!xglFontParseNumber(&next, &value)) {
        return 0;
    }
    switch (axis) {
    case 'x':
        cursor->x += value;
        break;
    case 'y':
        cursor->y += value;
        break;
    default:
        return 0;
    }
    *text = next;
    return 1;
}

#define FONT_COLOR_NEUTRAL 0x80
#define FONT_ALPHA_NEUTRAL 0x80
#define FONT_EFFECT_PLAIN 1

extern unsigned int D_004B3170[];

int xglFontCmdInit(FontDrawCursor *cursor, const char **text)
{
    cursor->blue = FONT_COLOR_NEUTRAL;
    cursor->flags &= 0xF8;
    cursor->green = FONT_COLOR_NEUTRAL;
    cursor->red = FONT_COLOR_NEUTRAL;
    cursor->alpha = FONT_ALPHA_NEUTRAL;
    cursor->effect = FONT_EFFECT_PLAIN;
    cursor->effect_x = 1;
    cursor->effect_y = 1;
    cursor->scale_x = FONT_SCALE_ONE;
    cursor->scale_y = FONT_SCALE_ONE;
    cursor->effect_red = 0;
    cursor->effect_green = 0;
    cursor->effect_blue = 0;
    cursor->effect_alpha = 0;
    cursor->alpha_abcd = 0;
    cursor->alpha_fix = 0;
    sceVif1PkAddDirectDataN(cursor->packet, D_004B3170, 4);
    D_007E7E00.default_width = 24;
    D_007E7E00.unmodeled_14 = 1;
    D_007E7E00.compact_width = FONT_SCALE_ONE;
    D_007E7E00.compact_height = FONT_SCALE_ONE;
    D_007E7E00.unmodeled_10 = 4;
    D_007E7E00.debug_mode = 0;
    D_007E7E00.unmodeled_11[1] = 0;
    D_007E7E00.unmodeled_11[2] = 0;
    (*text)++;
    D_007E7E00.ruby_offset_y = 2;
    D_007E7E00.unmodeled_0e = 0;
    return 1;
}

/* Inline colour command "col": reads a 0xBBGGRR value after the text cursor into the draw colour. */
int xglFontCmdCol(FontDrawCursor *cursor, const char **text)
{
    const char *next = *text;
    const char *end;
    int value;

    if (!xglFontParseHex(&next, &value)) {
        return 0;
    }
    end = next;
    cursor->blue = (unsigned int)value >> 16;
    cursor->green = (unsigned int)value >> 8;
    cursor->red = value;
    *text = end;
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00183660);

int xglFontCmdShadow(FontDrawCursor *cursor, const char **text)
{
    struct {
        const char *next;
        int x;
        int y;
        int color;
    } work;

    work.next = *text;
    if (!xglFontParseNumber(&work.next, &work.x)) {
        return 0;
    }
    if (!xglFontParseNumber(&work.next, &work.y)) {
        return 0;
    }
    if (!xglFontParseHex(&work.next, &work.color)) {
        return 0;
    }
    cursor->effect_x = work.x;
    cursor->effect_y = work.y;
    cursor->effect_red = work.color;
    cursor->effect_green = (unsigned int)work.color >> 8;
    cursor->effect_blue = (unsigned int)work.color >> 16;
    cursor->effect_alpha = (unsigned int)work.color >> 24;
    *text = work.next;
    return 1;
}

/*
 * Inline blend command: six numbers A, B, C, D (2 bits each of ALPHA_1), the mode byte and the
 * alpha value; writes ALPHA_1 at once.
 */
int xglFontCmdAlpha(FontDrawCursor *cursor, const char **text)
{
    int values[6];
    const char *next = *text;
    int index;
    struct FontAlphaWrite *packet;

    for (index = 0; index < 6; index++) {
        if (!xglFontParseNumber(&next, &values[index])) {
            return 0;
        }
    }
    *text = next;
    cursor->alpha_abcd = values[0] + (values[1] << 2) + (values[2] << 4) + (values[3] << 6);
    cursor->alpha_fix = values[4];
    cursor->alpha = values[5];
    packet = xglFontAllocGifPacket(cursor, 1);
    packet->alpha_abcd = cursor->alpha_abcd;
    packet->alpha_fix = cursor->alpha_fix;
    packet->register_address = GS_ALPHA_1;
    return 1;
}

/*
 * Inline zoom command: "x;" or "x,y;" sets the glyph scale (1/16 units) and picks the texture filter.
 * The parse cursor and the parsed values are one local record: the parsers take their addresses, and
 * only as members of one object do the reloads after the calls match the original.
 */
int xglFontCmdZoom(FontDrawCursor *cursor, const char **text)
{
    struct {
        const char *next;
        int scale_x;
        int scale_y;
    } work;

    work.next = *text;
    if (!xglFontParseNumber(&work.next, &work.scale_x)) {
        return 0;
    }
    if (((const unsigned char *)work.next)[-1] == ';') {
        work.scale_y = work.scale_x;
    } else if (!xglFontParseNumber(&work.next, &work.scale_y)) {
        return 0;
    }
    *text = work.next;
    cursor->scale_x = work.scale_x;
    cursor->scale_y = work.scale_y;
    xglFontSetTextureFilter(cursor, 1);
    return 1;
}

/* Splat labels of the FontState bytes the $xx inline command writes; each is its own object in the original. */
extern unsigned char D_007E7E0D[];
extern unsigned short D_007E7E0E[];
extern unsigned char D_007E7E10[];
extern unsigned char D_007E7E12[];
extern unsigned char D_007E7E13[];
extern unsigned char D_007E7E14[];
int xglSRand(void);

/* One A+D register write of the clamp packet: a 64-bit value, then the register address. */
struct FontGsRegisterWrite {
    unsigned long long data;
    unsigned int register_address;
    unsigned int register_address_high;
};

int xglFontCmdXx(FontDrawCursor *cursor, const char **text)
{
    const char *next = *text;
    int value;

    if (!xglFontParseNumber(&next, &value)) {
        return 0;
    }
    switch (value) {
    case 0:
        if (!xglFontParseNumber(&next, &value)) {
            return 0;
        }
        *(unsigned char *) &fontStateFromWindow = value;
        break;
    case 1:
        if (!xglFontParseNumber(&next, &value)) {
            return 0;
        }
        D_007E7E10[0] = value;
        break;
    case 2:
        if (!xglFontParseNumber(&next, &value)) {
            return 0;
        }
        D_007E7E0E[0] = value;
        break;
    case 3:
        if (!xglFontParseNumber(&next, &value)) {
            return 0;
        }
        D_007E7E12[0] = value;
        break;
    case 4:
        if (!xglFontParseNumber(&next, &value)) {
            return 0;
        }
        D_007E7E13[0] = value;
        break;
    case 5: {
        int horizontal_repeat;
        int vertical_repeat;
        struct FontGsRegisterWrite *clamp;
        long long horizontal_mode;
        unsigned short horizontal_mask;
        unsigned short horizontal_random;
        long long vertical_mode;
        unsigned short vertical_mask;
        unsigned short vertical_random;

        if (!xglFontParseNumber(&next, &horizontal_repeat)) {
            return 0;
        }
        if ((unsigned char) next[-1] == ';') {
            vertical_repeat = horizontal_repeat;
        } else if (!xglFontParseNumber(&next, &vertical_repeat)) {
            return 0;
        }
        clamp = xglFontAllocGifPacket(cursor, 1);
        clamp->register_address = GS_CLAMP_1;
        horizontal_mode = 1;
        horizontal_random = 0;
        horizontal_mask = 0;
        if (horizontal_repeat != 0) {
            horizontal_mode = 3;
            horizontal_mask = horizontal_repeat ^ 0x3FF;
            if (horizontal_repeat & 0x80) {
                horizontal_random = horizontal_repeat & xglSRand();
            }
        }
        vertical_mode = 1;
        vertical_random = 0;
        vertical_mask = 0;
        if (vertical_repeat != 0) {
            vertical_mode = 3;
            vertical_mask = vertical_repeat ^ 0x3FF;
            if (vertical_repeat & 0x80) {
                vertical_random = vertical_repeat & xglSRand();
            }
        }
        clamp->data = horizontal_mode | (vertical_mode << 2) | ((long long) horizontal_mask << 4) |
            ((long long) horizontal_random << 14) | ((long long) vertical_mask << 24) |
            ((long long) vertical_random << 34);
        break;
    }
    case 6:
        if (!xglFontParseNumber(&next, &value)) {
            return 0;
        }
        D_007E7E14[0] = value;
        break;
    case 7:
        if (!xglFontParseNumber(&next, &value)) {
            return 0;
        }
        D_007E7E0D[0] = value;
        break;
    default:
        return 0;
    }
    *text = next;
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00183B68);

/*
 * Inline width command: "n0;" / "n1;" / "n2;" pick the half-width mode, "p0;" / "p1;" proportional
 * spacing; the letter stands in front of the text cursor.
 */
int xglFontCmdHan(FontDrawCursor *cursor, const char **text)
{
    const unsigned char *next = (const unsigned char *)*text;
    unsigned char flags = cursor->flags;

    if (next[1] != ';') {
        return 0;
    }
    switch (next[-1]) {
    case 'p':
        flags &= ~FONT_CURSOR_PROPORTIONAL;
        switch (next[0]) {
        case '0':
            break;
        case '1':
            flags |= FONT_CURSOR_PROPORTIONAL;
            break;
        default:
            return 0;
        }
        break;
    case 'n':
        flags &= ~(FONT_CURSOR_WIDE_ASCII | FONT_CURSOR_WIDE_ALL);
        switch (next[0]) {
        case '0':
            break;
        case '1':
            flags |= FONT_CURSOR_WIDE_ASCII;
            break;
        case '2':
            flags |= FONT_CURSOR_WIDE_ALL;
            break;
        default:
            return 0;
        }
        break;
    default:
        return 0;
    }
    cursor->flags = flags;
    *text = (const char *)(next + 2);
    return 1;
}

/* Inline "$mark;" command: a hex index is parsed from the text and drawn through func_001848D8. */
int xglFontCmdMark(FontDrawCursor *cursor, const char **text)
{
    const char *position = *text;
    int index;

    if (!xglFontParseHex(&position, &index)) {
        return 0;
    }
    func_001848D8(index);
    *text = position;
    return 1;
}

/*
 * Runs the inline command after the '$' at *text: the first table name that is a prefix of the text
 * wins, its handler parses the arguments. Unknown names are reported with their first four characters.
 */
int xglFontInlineCommand(FontDrawCursor *cursor, unsigned char **text)
{
    const char *next = (const char *)*text + 1;
    struct FontInlineCommand *command = fontInlineCommands;
    FontInlineHandler handler = 0;
    while (command->name != 0) {
        const char *name = command->name;
        const char *scan = next;

        while (*name == *scan) {
            name++;
            scan++;
        }
        if (*name == 0) {
            next = scan;
            handler = command->handler;
            break;
        }
        command++;
    }
    if (handler != 0) {
        if (handler(cursor, &next)) {
            *text = (unsigned char *)next;
            return 1;
        }
    } else {
        printf(fontInlineErrorFormat, (unsigned char)next[0], (unsigned char)next[1], (unsigned char)next[2],
               (unsigned char)next[3]);
    }
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00184168);

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00184188);

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00184798);

void func_00184798(int code, int v);

void func_001848D8(int code)
{
    func_00184798(code, 128);
}

void xglFontFlushSubHex(int code)
{
    func_00184798(code, 96);
}

void xglFontFlushSubCRLF(void)
{
    FontDrawCursor *cursor = (FontDrawCursor *) 0x70000000;

    cursor->x = cursor->origin_x;
    cursor->y += (signed char) D_007E7E00.default_width;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00184938);

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00184B30);

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00185770);

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00185810);

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00185A20);

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00185B30);

/* The FontState flags halfword (0x007E7E04), read and written through the splat symbol. */
extern unsigned short D_007E7E04[];

#define FONT_STATE_DEBUG_PRINT 2

unsigned short xglFontGetFlags(void)
{
    return D_007E7E04[0];
}

void xglFontSetFlags(int flags)
{
    D_007E7E04[0] = flags & (0xFFFF & ~FONT_STATE_DEBUG_PRINT);
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00185B68);

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00185CB0);

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00185D58);

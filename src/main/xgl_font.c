#include "common.h"
#include "shared.h"

struct XglPacket;

/* Packet that FONT_OP_EFFECT_PACKET builds in the cursor's effect area: an A+D GIF tag and one
 * register write. */
struct FontEffectScratchPacket {
    u64 vif_tag;
    u64 vif_count;
    u64 effect_state;
    u64 payload_count;
};

typedef struct FontDrawCursor { struct XglPacket *packet; short origin_x; unsigned char unmodeled_06[2]; unsigned short x; unsigned short y; int depth; unsigned short offset_x; unsigned short offset_y; unsigned char red; unsigned char green; unsigned char blue; unsigned char unmodeled_17; unsigned char drawFlags; unsigned char effect; unsigned char attributes; unsigned char draw_state_high; unsigned char unmodeled_1c[4]; signed char effect_x; signed char effect_y; unsigned char unmodeled_22[2]; unsigned char effect_red; unsigned char effect_green; unsigned char effect_blue; unsigned char effect_alpha; /* The GS ALPHA_1 value: the A/B/C/D selectors and FIX. */ unsigned char alpha_abcd; unsigned char alpha_fix; unsigned char alpha; unsigned char unmodeled_2b; unsigned short scale_x; unsigned short scale_y; union { struct FontEffectScratchPacket effect_packet; u64 qwords[(0x4000 - 0x30) / sizeof(u64)]; } effectArea; } FontDrawCursor;

int xglFontGetStringWidth(const char *text);
int xglFontGetSPcodeSize(int opcode, const unsigned char *text);
const char *xglFontCopyTextUnit(char *textUnitBuffer, const char *text, int *length);

#define GS_ALPHA_1 0x42
/* GIFtag register descriptor of an A+D (address plus data) register. */
#define GIF_AD 0x0E
#include "main/Packet.h"
#include "main/xgl_font_codes.h"
#include "gs.h"

void sceVif1PkAddDirectDataN(XglPacket *packet, const void *data, unsigned int qwc);
void *sceVif1PkReserve(XglPacket *packet, unsigned int words);


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
    unsigned short fontFlags;
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
int xglFontIsTextureGlyphCode(unsigned char eucRow, unsigned char eucColumn);

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
void xglFontSetTextureFilter(FontDrawCursor *cursor, int useCursorScale);
/* Advance of each printable ASCII character (32..127) in the debug font. */
extern int fontAsciiWidthTable[];
int xglFontParseNumber(const char **cursor, int *parsedNumber);
unsigned short xglFontCharToEuc(signed char character, unsigned char **text);
unsigned int xglFontGetDebugCell(unsigned char eucRow, unsigned char eucColumn);
unsigned short xglFontGetGlyphIndex(unsigned char eucRow, unsigned char eucColumn);
/* * Reads a hex number up to ';' or ',' and moves the text cursor past the delimiter; returns 0 on any * other character. */
int xglFontParseHex(const char **cursor, int *parsedNumber);

/* Texture coordinate / size / draw-flag arguments of xglFontFlushSub. */
#define FONT_ASCII_V_LOW 0x2f00
#define FONT_ASCII_V_HIGH 0x2f80
#define FONT_SYMBOL_V 0x2e80

typedef int (*FontInlineHandler)(FontDrawCursor *cursor, const char **text);

/* One entry of the inline "$name" command table; the table ends with a null name. */
struct FontInlineCommand {
    const char *name;
    FontInlineHandler parseArguments;
};

/* ax ay az rx ry init save load col bg shadow alpha zoom xx ruby prop han mark */
extern struct FontInlineCommand fontInlineCommands[];
/* "[*error*:%c%c%c%c]\n" */
extern const char fontInlineErrorFormat[24];
int printf(const char *format, ...);
/* Two-byte code pairs, terminated by a 0 first byte. */
extern unsigned char fontPairGlyphCodes[];
int xglFontFindPairGlyph(unsigned char eucRow, unsigned char eucColumn);

/* Glyph index of codes the font does not have (also xglFontFindPairGlyph's miss). */
#define FONT_GLYPH_NONE 0x7F
/* The level-1 kanji glyphs follow the 0x400 symbol and kana glyphs. */
#define FONT_GLYPH_KANJI_BASE 0x400

int xglFontInlineCommand(FontDrawCursor *cursor, const char **text);
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

int xglFontGetProportionalSize(int glyphCode);
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

/* Two byte-pair tables of (high, low) codes, zero-terminated; xglFontPairTableSelect selects the first one when set. */
extern unsigned char xglFontPairTableFlagged[];
extern unsigned char xglFontPairTablePlain[];
extern unsigned char xglFontPairTableSelect[];

/* Index of the first pair equal to (high, low) plus this base; FONT_GLYPH_NONE when no pair matches. */
#define FONT_PAIR_GLYPH_BASE 3989

int xglFontFindPairGlyph(unsigned char eucRow, unsigned char eucColumn)
{
    const unsigned char *pairCursor;
    int pairMatchIndex;

    if (xglFontPairTableSelect[0] == 0) {
        pairCursor = xglFontPairTablePlain;
    } else {
        pairCursor = xglFontPairTableFlagged;
    }
    for (pairMatchIndex = 0; pairCursor[0] != 0; pairMatchIndex++, pairCursor += 2) {
        if (eucRow == pairCursor[0] && eucColumn == pairCursor[1]) {
            return pairMatchIndex + FONT_PAIR_GLYPH_BASE;
        }
    }
    return FONT_GLYPH_NONE;
}

int xglFontGetProportionalSize(int glyphCode)
{
    if (glyphCode >= 192 && glyphCode < 384) {
        return glyphCode;
    }
    switch (glyphCode & 0xfff) {
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
        return glyphCode + 147;
    default:
        return 383;
    }
}

int xglFontIsTextureGlyphCode(unsigned char eucRow, unsigned char eucColumn)
{
    if (eucColumn == 0) {
        return 0;
    }
    if (eucRow == 164 || eucRow == 166) {
        return 1;
    }
    if (eucRow == 161 && (eucColumn == 189 || eucColumn == 218 || eucColumn == 219 || eucColumn == 223 || eucColumn == 238 || eucColumn == 249 ||
                        eucColumn == 250 || eucColumn == 251 || eucColumn == 252)) {
        return 1;
    }
    if (eucRow == 162 && ((unsigned char) (eucColumn + 94) < 5 || eucColumn == 170 || eucColumn == 171 || eucColumn == 172 || eucColumn == 173 ||
                        eucColumn == 246)) {
        return 1;
    }
    if (eucRow == 163 && eucColumn >= 176 && eucColumn < 186) {
        return 1;
    }
    if (eucRow == 173 && (unsigned char) (eucColumn + 75) < 3) {
        return 1;
    }
    return 0;
}

struct FontStateQueue { unsigned char *texture_buffer; unsigned short fontFlags; unsigned char font_selection; unsigned char unmodeled_07; unsigned short unmodeled_08; unsigned char unmodeled_0a[2]; unsigned char default_width; unsigned char debug_mode; unsigned short unmodeled_0e; unsigned char unmodeled_10; unsigned char unmodeled_11[3]; unsigned char unmodeled_14; unsigned char unmodeled_15[7]; /* Text address of the include command that prefixes each debug line; 0 when none is set. */ unsigned int debug_prefix; unsigned char unmodeled_20[6]; unsigned short ruby_offset_y; union { unsigned short default_scale; struct { unsigned char compact_width; unsigned char compact_height; }; }; unsigned char unmodeled_2a[6]; unsigned char word_buffer[256]; unsigned char *queue_cursor; };

extern struct FontStateQueue xglFontState;
void sceVif1PkRef();

/* The 1-bit glyph plane behind the font texture: 0x800 bytes at texture_buffer + 0x78400, eight pixels
 * per byte, expanded through fontNibbleMasks (four pixels per nibble, 0xFF per set bit) into
 * fontExpandedPlane and sent to the GS after the setup packet fontPlaneUploadSetup. */
#define FONT_PLANE_OFFSET 0x78400
#define FONT_PLANE_BYTES 0x800
#define FONT_PLANE_QUADWORDS 0x400
#define FONT_PLANE_SETUP_QUADWORDS 7
#define VIF_DIRECT_FONT_PLANE 0x51000400 /* VIF DIRECT, 0x400 quadwords */

extern u32 fontNibbleMasks[16];
extern u32 fontExpandedPlane[FONT_PLANE_BYTES * 2];
extern unsigned char fontPlaneUploadSetup[];
XglPacket *xglPacketGetCurrent(void);

void func_00181FA0(void)
{
    unsigned char *source;
    u32 *plane;
    u32 *destination;
    int i;
    XglPacket *packet;

    source = xglFontState.texture_buffer + FONT_PLANE_OFFSET;
    plane = fontExpandedPlane;
    destination = plane;
    for (i = 0; i < FONT_PLANE_BYTES; i++) {
        unsigned char bits;
        u32 high;
        u32 low;

        bits = *source++;
        high = fontNibbleMasks[bits >> 4];
        low = fontNibbleMasks[bits & 0xF];
        destination[0] = high;
        destination[1] = low;
        destination += 2;
    }
    packet = xglPacketGetCurrent();
    sceVif1PkRef(packet, fontPlaneUploadSetup, FONT_PLANE_SETUP_QUADWORDS, 0, 0, 0);
    sceVif1PkRef(packet, plane, FONT_PLANE_QUADWORDS, 0, VIF_DIRECT_FONT_PLANE, 0);
}

/*
 * Reserves count A+D writes in the cursor's packet and writes their GIF tag: NLOOP = count with
 * the EOP bit 0x8000, one register (NREG = 1 in the upper word) and the register list GIF_AD.
 * Returns the address 16 bytes in.
 */
void *xglFontAllocGifPacket(FontDrawCursor *cursor, int adWriteCount)
{
    unsigned int *packet;

    packet = sceVif1PkReserve(cursor->packet, adWriteCount * 4 + 4);
    packet[0] = adWriteCount + 0x8000;
    packet[1] = 0x10000000;
    packet[2] = GIF_AD;
    return packet + 4;
}

#define GIF_TAG_NREG_1 0x1000000000000000ULL
/* The font's Z test when xglFontState.unmodeled_14 (the depth-test flag) is set, else always. */
#define FONT_ZTST_GEQUAL 2
#define FONT_ZTST_ALWAYS 1
/* The alpha test of the plain effects: pass where alpha is not equal to 0. */
#define FONT_ATST_NOTEQUAL 6

/* Queues the font's depth state as two A+D writes (ZBUF_1, TEST_1) from the 16-byte aligned
 * start of the cursor's effect area: Z24 at page 0, written only without the depth test; the
 * Z test from the depth-test flag, and the alpha test for effects 0 and 2..4. */
void func_001820D0(FontDrawCursor *cursor)
{
    u64 *packet;
    int ztst;

    packet = (u64 *) (((unsigned int) cursor->effectArea.qwords + 0xF) & ~0xF);
    ztst = xglFontState.unmodeled_14 != 0 ? FONT_ZTST_GEQUAL : FONT_ZTST_ALWAYS;
    packet[0] = GIF_TAG_NREG_1 | GIF_TAG_EOP | 2;
    packet[1] = GIF_AD;
    packet[2] = GS_SET_ZBUF(0, GS_PSMZ24, xglFontState.unmodeled_14 ^ 1);
    packet[3] = GS_ZBUF_1;
    packet[5] = GS_TEST_1;
    switch (cursor->effect) {
    case 0:
    case 2:
    case 3:
    case 4:
        packet[4] = GS_SET_TEST(1, FONT_ATST_NOTEQUAL, 0, 0, 0, 0, 1, ztst);
        break;
    /* case 1 is its own test in the original's decision tree, with the default's code */
    case 1:
    default:
        packet[4] = GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, ztst);
        break;
    }
    sceVif1PkAddDirectDataN(cursor->packet, packet, 3);
}

/* The two texture-filter packets: 1:1 glyphs and scaled glyphs. */
extern unsigned int D_004B3110[8];
extern unsigned int D_004B3130[8];

void xglFontSetTextureFilter(FontDrawCursor *cursor, int useCursorScale)
{
    int scaled;

    if (useCursorScale == 0) {
        scaled = xglFontState.default_scale != FONT_DEFAULT_SCALE_ONE;
    } else {
        scaled = *(unsigned int *)&cursor->scale_x != FONT_SCALE_PAIR_ONE;
    }
    if (!(xglFontState.fontFlags & 0x10)) {
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
    unsigned char presentFlags;
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
    struct FontPositionCommand *command = (struct FontPositionCommand *) xglFontState.queue_cursor;

    command->opcode = FONT_OP_POSITION;
    command->presentFlags = FONT_POS_SET_X | FONT_POS_SET_Y | FONT_POS_SET_DEPTH;
    command->x_high = x >> 8;
    command->x_low = x;
    command->y_high = y >> 8;
    command->y_low = y;
    command->color_high = color >> 16;
    command->color_middle = color >> 8;
    command->color_low = color;
    xglFontState.queue_cursor += sizeof(*command);
}

int xglFontGetSPcodeSize(int opcode, const unsigned char *text)
{
    unsigned char commandSize;

    opcode &= 0xff;
    commandSize = spcode_0[opcode];

    if (commandSize == 255) {
        switch (opcode) {
        case FONT_OP_POSITION: {
            unsigned char parameter_sizes[8];
            unsigned char parameterMask = text[1];
            int bit;

            __builtin_memcpy(parameter_sizes, spCodeParameterSizeTable,
                             sizeof(parameter_sizes));
            commandSize = 1;
            for (bit = 0; bit < 8; bit++) {
                if (parameterMask & 1) {
                    commandSize += parameter_sizes[bit];
                }
                parameterMask >>= 1;
            }
            break;
        }
        case FONT_OP_CONFIG:
            commandSize = text[1] + 1;
            break;
        }
    }

    return commandSize;
}

#define FONT_NUMERIC_FLOAT 0xff

struct FontNumericCommand {
    unsigned char opcode;
    unsigned char radix;
    unsigned char digits_minus_one;
    unsigned char numberTopByte;
    unsigned char numberSecondByte;
    unsigned char numberThirdByte;
    unsigned char numberLowByte;
};

float func_0030A4E0(unsigned long long value);

void xglFontPrintSub(void *argumentList)
{
    unsigned long long *list = argumentList;
    const char *text = (const char *)(int)list[0];
    unsigned char ahead = *text;

    list++;
    for (;;) {
        unsigned int character;

        if (*text == 0) {
            break;
        }
        character = *text & 0xff;

        if (character < 32) {
            int remaining = xglFontGetSPcodeSize(character, (const unsigned char *)text);

            for (; remaining >= 0; remaining--) {
                *fontStateFromWindow.queue_cursor++ = *text++;
            }
        } else if (character == '%') {
            int width = 0;
            unsigned int radix;

            text++;
            for (;;) {
                character = (unsigned char)*text++;

                switch (character) {
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                    width = width * 10 + (character - '0');
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
                        unsigned long long numberValue = (unsigned int)list[0];
                        struct FontNumericCommand *command = (struct FontNumericCommand *)xglFontState.queue_cursor;

                        list++;
                        if (width == 0) {
                            width = 1;
                        }
                        command->opcode = FONT_OP_NUMBER;
                        command->radix = radix;
                        command->digits_minus_one = width - 1;
                        command->numberTopByte = numberValue >> 24;
                        command->numberSecondByte = numberValue >> 16;
                        command->numberThirdByte = numberValue >> 8;
                        command->numberLowByte = numberValue;
                        xglFontState.queue_cursor += sizeof(*command);
                    }
                    break;
                case 'f':
                    {
                        float realValue = func_0030A4E0(list[0]);
                        unsigned int bits = *(unsigned int *)&realValue;
                        struct FontNumericCommand *command = (struct FontNumericCommand *)xglFontState.queue_cursor;

                        list++;
                        command->opcode = FONT_OP_NUMBER;
                        command->radix = FONT_NUMERIC_FLOAT;
                        command->digits_minus_one = 0xff;
                        command->numberTopByte = bits >> 24;
                        command->numberSecondByte = bits >> 16;
                        command->numberThirdByte = bits >> 8;
                        command->numberLowByte = bits;
                        xglFontState.queue_cursor += sizeof(*command);
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
                    character = (unsigned char)list[0];
                    list++;
                    /* fall through */
                case '$':
                case '%':
                    *xglFontState.queue_cursor++ = character;
                    break;
                default:
                    break;
                }
                break;
            }
        } else {
            text++;
            *xglFontState.queue_cursor++ = ahead;
        }
        ahead = *text;
    }
    *xglFontState.queue_cursor++ = 0;
}

void xglFontPrintDirectCore(const char *text)
{
    while (*text != 0) {
        unsigned char character = *text;

        if (character < 32) {
            int remaining = xglFontGetSPcodeSize(character, (const unsigned char *)text);

            while (remaining > 0) {
                *xglFontState.queue_cursor++ = *text++;
                remaining--;
            }
        }
        *xglFontState.queue_cursor++ = *text++;
    }
    *xglFontState.queue_cursor++ = 0;
}

void xglFontPrintSub(void *argumentList);

void xglFontPrintf(int x, int y, unsigned int color, void *argumentList)
{
    xglFontAppendPosition(x, y, color);
    xglFontPrintSub(argumentList);
}

void xglFontPrintDirectCore(const char *text);

void xglFontPrint(int x, int y, unsigned int color, const char *text)
{
    xglFontAppendPosition(x, y, color);
    xglFontPrintDirectCore(text);
}

void xglFontPrintDirect(const char *text)
{
    xglFontPrintDirectCore(text);
}

/* A FONT_OP_CALLBACK command: a 24-bit function address and a 32-bit argument, big-endian, then the
 * terminator byte that closes the queue (X2 FontExtRecord). */
struct FontCallbackRecord {
    unsigned char opcode;
    unsigned char function_high;
    unsigned char function_middle;
    unsigned char function_low;
    unsigned char argument_high;
    unsigned char argument_mid_high;
    unsigned char argument_mid_low;
    unsigned char argument_low;
    unsigned char terminator;
};

/* Queues a call of function(argument) (X2 xglFontPrintExtFunc; this build ignores the OT argument). */
void xglFontPrintExtFunc(int ot, void *function, void *argument)
{
    struct FontCallbackRecord *record;
    unsigned int function_address = (unsigned int)function;
    unsigned int argument_word = (unsigned int)argument;

    record = (struct FontCallbackRecord *)xglFontState.queue_cursor;
    record->opcode = FONT_OP_CALLBACK;
    record->function_high = function_address >> 16;
    record->function_middle = function_address >> 8;
    record->function_low = function_address;
    record->argument_high = argument_word >> 24;
    record->argument_mid_high = argument_word >> 16;
    record->argument_mid_low = argument_word >> 8;
    record->argument_low = argument_word;
    record->terminator = 0;
    xglFontState.queue_cursor += sizeof(*record);
}

void xglFontDebugPrefixSet(int stored_value)
{
    fontStateFromWindow.stored_value = stored_value;
}

/* The EE va_start spelling of the variadic debug print (as in the other variadic definitions of this tree). */
typedef char *va_list;

#define va_start(ap, last) ((ap) = (va_list)__builtin_next_arg(last) - (8 - __builtin_args_info(2)) * 8)

void xglFontDebugPrintf(int x, int y, const char *format, ...)
{
    unsigned long long arguments[32];
    va_list vaList;
    unsigned long long *list;
    unsigned long long *output;
    int remainingArgs;

    if (!(xglFontState.fontFlags & 0x2)) {
        return;
    }
    xglFontState.fontFlags |= 0x80;
    output = arguments + 1;
    arguments[0] = (unsigned int) format;
    va_start(vaList, format);
    list = (unsigned long long *) vaList;
    remainingArgs = 30;
    do {
        unsigned long long *argument = list++;
        unsigned long long argumentWord = *argument;

        remainingArgs--;
        *output = argumentWord;
        output++;
    } while (remainingArgs >= 0);
    xglFontAppendPosition(x << 1, y << 1, -1);
    if (xglFontState.debug_prefix != 0) {
        unsigned int text = xglFontState.debug_prefix;
        unsigned char *command = xglFontState.queue_cursor;

        command[0] = FONT_OP_INCLUDE;
        command[1] = text >> 24;
        command[2] = text >> 16;
        command[3] = text >> 8;
        command[4] = text;
        xglFontState.queue_cursor += 5;
    } else {
        *xglFontState.queue_cursor++ = FONT_OP_RESET;
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
const char *xglFontCopyTextUnit(char *textUnitBuffer, const char *text, int *length)
{
    const unsigned char *cursor = (const unsigned char *)text;
    unsigned char eucRow;

    if (textUnitBuffer == 0) {
        textUnitBuffer = fontStateFromWindow.word_buffer;
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
                *textUnitBuffer++ = *cursor++;
            }
        } else if (*cursor < 0x20) {
            int remaining = xglFontGetSPcodeSize(*cursor, cursor);

            while (remaining >= 0) {
                *textUnitBuffer++ = *cursor++;
                remaining--;
            }
        } else {
            int wide;

            eucRow = *cursor;
            wide = xglFontIsTextureGlyphCode(eucRow, cursor[1]);
            *textUnitBuffer = eucRow;
            if (wide) {
                textUnitBuffer[1] = cursor[1];
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
    int totalWidth = 0;

    while (*cursor != 0) {
        int length;
        const char *nextCursor = xglFontCopyTextUnit(xglFontState.word_buffer, cursor, &length);

        xglFontState.word_buffer[length] = 0;
        totalWidth += xglFontGetStringWidth2(xglFontState.word_buffer, 0);
        if (*width < totalWidth) {
            break;
        }
        cursor = nextCursor;
    }
    *width = totalWidth;
    return cursor - text;
}

/*
 * Reads a hex number up to ';' or ',' and moves the text cursor past the delimiter; returns 0 on any
 * other character.
 */
int xglFontParseHex(const char **cursor, int *parsedNumber)
{
    const char *text = *cursor;
    unsigned int hexValue = 0;
    unsigned char ch = *text++;

    while (ch != ';' && ch != ',') {
        hexValue = (hexValue << 4) + ch;
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
            hexValue -= '0';
            break;
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            hexValue += 'a' - 'A';
            /* fall through */
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
            hexValue -= 'a' - 10;
            break;
        default:
            return 0;
        }
        ch = *text++;
    }
    *parsedNumber = hexValue;
    *cursor = text;
    return 1;
}

/*
 * Reads a number up to ';' or ',' and moves the text cursor past the delimiter: an optional sign, then
 * decimal digits or "0x" and a hex number (xglFontParseHex). Returns 0 on any other character.
 */
int xglFontParseNumber(const char **cursor, int *parsedNumber)
{
    const unsigned char *text = (const unsigned char *)*cursor;
    int sign = 1;
    int magnitude = 0;
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
        *cursor = text + 2;
        if (!xglFontParseHex(cursor, &magnitude)) {
            return 0;
        }
        *parsedNumber = sign * magnitude;
        return 1;
    }
    ch = *text++;
    while (ch != ';' && ch != ',') {
        if ((unsigned char)(ch - '0') >= 10) {
            return 0;
        }
        magnitude = magnitude * 10 + ch - '0';
        ch = *text++;
    }
    *cursor = text;
    *parsedNumber = sign * magnitude;
    return 1;
}

/* Inline position command: the letter before the number picks x (also the line origin), y or depth. */
int xglFontCmdAx(FontDrawCursor *cursor, const char **text)
{
    const char *parseCursor = *text;
    unsigned char axis = parseCursor[-1];
    int axisPosition;

    if (!xglFontParseNumber(&parseCursor, &axisPosition)) {
        return 0;
    }
    switch (axis) {
    case 'x':
        cursor->x = axisPosition;
        cursor->origin_x = axisPosition;
        break;
    case 'y':
        cursor->y = axisPosition;
        break;
    case 'z':
        cursor->depth = axisPosition;
        break;
    default:
        return 0;
    }
    *text = parseCursor;
    return 1;
}

/* Inline move command: adds the number to x or y, picked by the letter before it. */
int xglFontCmdRx(FontDrawCursor *cursor, const char **text)
{
    const char *parseCursor = *text;
    unsigned char axis = parseCursor[-1];
    int axisDelta;

    if (!xglFontParseNumber(&parseCursor, &axisDelta)) {
        return 0;
    }
    switch (axis) {
    case 'x':
        cursor->x += axisDelta;
        break;
    case 'y':
        cursor->y += axisDelta;
        break;
    default:
        return 0;
    }
    *text = parseCursor;
    return 1;
}

#define FONT_COLOR_NEUTRAL 0x80
#define FONT_ALPHA_NEUTRAL 0x80
#define FONT_EFFECT_PLAIN 1

extern unsigned int xglFontResetPacket[];

int xglFontCmdInit(FontDrawCursor *cursor, const char **text)
{
    cursor->blue = FONT_COLOR_NEUTRAL;
    cursor->drawFlags &= 0xF8;
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
    sceVif1PkAddDirectDataN(cursor->packet, xglFontResetPacket, 4);
    xglFontState.default_width = 24;
    xglFontState.unmodeled_14 = 1;
    xglFontState.compact_width = FONT_SCALE_ONE;
    xglFontState.compact_height = FONT_SCALE_ONE;
    xglFontState.unmodeled_10 = 4;
    xglFontState.debug_mode = 0;
    xglFontState.unmodeled_11[1] = 0;
    xglFontState.unmodeled_11[2] = 0;
    (*text)++;
    xglFontState.ruby_offset_y = 2;
    xglFontState.unmodeled_0e = 0;
    return 1;
}

/* Inline colour command "col": reads a 0xBBGGRR value after the text cursor into the draw colour. */
int xglFontCmdCol(FontDrawCursor *cursor, const char **text)
{
    const char *parseCursor = *text;
    const char *textAfterColor;
    int packedColor;

    if (!xglFontParseHex(&parseCursor, &packedColor)) {
        return 0;
    }
    textAfterColor = parseCursor;
    cursor->blue = (unsigned int)packedColor >> 16;
    cursor->green = (unsigned int)packedColor >> 8;
    cursor->red = packedColor;
    *text = textAfterColor;
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00183660);

int xglFontCmdShadow(FontDrawCursor *cursor, const char **text)
{
    struct {
        const char *parseCursor;
        int x;
        int y;
        int color;
    } shadowParse;

    shadowParse.parseCursor = *text;
    if (!xglFontParseNumber(&shadowParse.parseCursor, &shadowParse.x)) {
        return 0;
    }
    if (!xglFontParseNumber(&shadowParse.parseCursor, &shadowParse.y)) {
        return 0;
    }
    if (!xglFontParseHex(&shadowParse.parseCursor, &shadowParse.color)) {
        return 0;
    }
    cursor->effect_x = shadowParse.x;
    cursor->effect_y = shadowParse.y;
    cursor->effect_red = shadowParse.color;
    cursor->effect_green = (unsigned int)shadowParse.color >> 8;
    cursor->effect_blue = (unsigned int)shadowParse.color >> 16;
    cursor->effect_alpha = (unsigned int)shadowParse.color >> 24;
    *text = shadowParse.parseCursor;
    return 1;
}

/*
 * Inline blend command: six numbers A, B, C, D (2 bits each of ALPHA_1), the mode byte and the
 * alpha value; writes ALPHA_1 at once.
 */
int xglFontCmdAlpha(FontDrawCursor *cursor, const char **text)
{
    int alphaParameters[6];
    const char *parseCursor = *text;
    int alphaIndex;
    struct FontAlphaWrite *packet;

    for (alphaIndex = 0; alphaIndex < 6; alphaIndex++) {
        if (!xglFontParseNumber(&parseCursor, &alphaParameters[alphaIndex])) {
            return 0;
        }
    }
    *text = parseCursor;
    cursor->alpha_abcd = alphaParameters[0] + (alphaParameters[1] << 2) + (alphaParameters[2] << 4) + (alphaParameters[3] << 6);
    cursor->alpha_fix = alphaParameters[4];
    cursor->alpha = alphaParameters[5];
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
        const char *parseCursor;
        int scale_x;
        int scale_y;
    } zoomParse;

    zoomParse.parseCursor = *text;
    if (!xglFontParseNumber(&zoomParse.parseCursor, &zoomParse.scale_x)) {
        return 0;
    }
    if (((const unsigned char *)zoomParse.parseCursor)[-1] == ';') {
        zoomParse.scale_y = zoomParse.scale_x;
    } else if (!xglFontParseNumber(&zoomParse.parseCursor, &zoomParse.scale_y)) {
        return 0;
    }
    *text = zoomParse.parseCursor;
    cursor->scale_x = zoomParse.scale_x;
    cursor->scale_y = zoomParse.scale_y;
    xglFontSetTextureFilter(cursor, 1);
    return 1;
}

/* Splat labels of the FontState bytes the $xx inline command writes; each is its own object in the original. */
extern unsigned char xglFontStateDebugMode[];
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
    const char *parseCursor = *text;
    int xxValue;

    if (!xglFontParseNumber(&parseCursor, &xxValue)) {
        return 0;
    }
    switch (xxValue) {
    case 0:
        if (!xglFontParseNumber(&parseCursor, &xxValue)) {
            return 0;
        }
        *(unsigned char *) &fontStateFromWindow = xxValue;
        break;
    case 1:
        if (!xglFontParseNumber(&parseCursor, &xxValue)) {
            return 0;
        }
        D_007E7E10[0] = xxValue;
        break;
    case 2:
        if (!xglFontParseNumber(&parseCursor, &xxValue)) {
            return 0;
        }
        D_007E7E0E[0] = xxValue;
        break;
    case 3:
        if (!xglFontParseNumber(&parseCursor, &xxValue)) {
            return 0;
        }
        D_007E7E12[0] = xxValue;
        break;
    case 4:
        if (!xglFontParseNumber(&parseCursor, &xxValue)) {
            return 0;
        }
        D_007E7E13[0] = xxValue;
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

        if (!xglFontParseNumber(&parseCursor, &horizontal_repeat)) {
            return 0;
        }
        if ((unsigned char) parseCursor[-1] == ';') {
            vertical_repeat = horizontal_repeat;
        } else if (!xglFontParseNumber(&parseCursor, &vertical_repeat)) {
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
        if (!xglFontParseNumber(&parseCursor, &xxValue)) {
            return 0;
        }
        D_007E7E14[0] = xxValue;
        break;
    case 7:
        if (!xglFontParseNumber(&parseCursor, &xxValue)) {
            return 0;
        }
        xglFontStateDebugMode[0] = xxValue;
        break;
    default:
        return 0;
    }
    *text = parseCursor;
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00183B68);

/*
 * Inline width command: "n0;" / "n1;" / "n2;" pick the half-width mode, "p0;" / "p1;" proportional
 * spacing; the letter stands in front of the text cursor.
 */
int xglFontCmdHan(FontDrawCursor *cursor, const char **text)
{
    const unsigned char *parseCursor = (const unsigned char *)*text;
    unsigned char drawFlags = cursor->drawFlags;

    if (parseCursor[1] != ';') {
        return 0;
    }
    switch (parseCursor[-1]) {
    case 'p':
        drawFlags &= ~FONT_CURSOR_PROPORTIONAL;
        switch (parseCursor[0]) {
        case '0':
            break;
        case '1':
            drawFlags |= FONT_CURSOR_PROPORTIONAL;
            break;
        default:
            return 0;
        }
        break;
    case 'n':
        drawFlags &= ~(FONT_CURSOR_WIDE_ASCII | FONT_CURSOR_WIDE_ALL);
        switch (parseCursor[0]) {
        case '0':
            break;
        case '1':
            drawFlags |= FONT_CURSOR_WIDE_ASCII;
            break;
        case '2':
            drawFlags |= FONT_CURSOR_WIDE_ALL;
            break;
        default:
            return 0;
        }
        break;
    default:
        return 0;
    }
    cursor->drawFlags = drawFlags;
    *text = parseCursor + 2;
    return 1;
}

void xglFontFlushSubMark(int markIndex);

/* Inline "$mark;" command: a hex index is parsed from the text and drawn through xglFontFlushSubMark. */
int xglFontCmdMark(FontDrawCursor *cursor, const char **text)
{
    const char *position = *text;
    int markIndex;

    if (!xglFontParseHex(&position, &markIndex)) {
        return 0;
    }
    xglFontFlushSubMark(markIndex);
    *text = position;
    return 1;
}

/*
 * Runs the inline command after the '$' at *text: the first table name that is a prefix of the text
 * wins, its handler parses the arguments. Unknown names are reported with their first four characters.
 */
int xglFontInlineCommand(FontDrawCursor *cursor, const char **text)
{
    const char *parseCursor = *text + 1;
    struct FontInlineCommand *command = fontInlineCommands;
    FontInlineHandler commandParser = 0;
    while (command->name != 0) {
        const char *name = command->name;
        const char *scan = parseCursor;

        while (*name == *scan) {
            name++;
            scan++;
        }
        if (*name == 0) {
            parseCursor = scan;
            commandParser = command->parseArguments;
            break;
        }
        command++;
    }
    if (commandParser != 0) {
        if (commandParser(cursor, &parseCursor)) {
            *text = parseCursor;
            return 1;
        }
    } else {
        printf(fontInlineErrorFormat, (unsigned char)parseCursor[0], (unsigned char)parseCursor[1], (unsigned char)parseCursor[2],
               (unsigned char)parseCursor[3]);
    }
    return 0;
}

void xglFontBufferReset(void)
{
    struct FontStateQueue *queue_end;

    xglFontState.unmodeled_11[0] = 0;
    queue_end = (&xglFontState) + 1;
    xglFontState.queue_cursor = (u8 *) queue_end;
    xglFontState.debug_prefix = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00184188);

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00184798);

void func_00184798(int code, int v);

void xglFontFlushSubMark(int markIndex)
{
    func_00184798(markIndex, 128);
}

void xglFontFlushSubHex(int hexValue)
{
    func_00184798(hexValue, 96);
}

void xglFontFlushSubCRLF(void)
{
    FontDrawCursor *cursor = (FontDrawCursor *) 0x70000000;

    cursor->x = cursor->origin_x;
    cursor->y += (signed char) xglFontState.default_width;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00184938);

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00184B30);

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00185770);

extern unsigned char D_004B1E20[];
extern unsigned char D_004B3320[];
extern unsigned char D_004B33A0[];
void sceVif1PkCloseDirectHLCode();
void sceVif1PkCnt();
void sceVif1PkOpenDirectHLCode();

/* reloadMode: the flush passes 0; this build does not read it. */
void xglFontBuildPacket(XglPacket **packet_slot, int reloadMode)
{
    XglPacket *packet;
    u32 *direct_state;
    u8 *texture;

    packet = *packet_slot;
    if (packet->directCode != 0) {
        direct_state = packet->directState;
        if (direct_state != 0) {
            sceVif1PkCloseDirectHLCode(packet);
            packet = *packet_slot;
            if (direct_state[0] == 0x51000000) {
                packet->write_cursor -= 4;
            }
        }
    }
    sceVif1PkRef(*packet_slot, D_004B1E20, 0x107, 0, 0, 0);
    sceVif1PkRef(*packet_slot, D_004B3320, 8, 0, 0, 0);
    texture = xglFontState.texture_buffer + 0x1A0;
    sceVif1PkRef(*packet_slot, D_004B33A0, 7, 0, 0, 0);
    sceVif1PkRef(*packet_slot, texture, 8, 0, 0x51000008, 0);
    sceVif1PkRef(*packet_slot, texture, 8, 0, 0x51000008, 0);
    sceVif1PkRef(*packet_slot, texture, 8, 0, 0x51000008, 0);
    sceVif1PkRef(*packet_slot, texture, 8, 0, 0x51000008, 0);
    sceVif1PkRef(*packet_slot, texture, 8, 0, 0x51000008, 0);
    sceVif1PkRef(*packet_slot, texture, 8, 0, 0x51000008, 0);
    sceVif1PkRef(*packet_slot, texture, 8, 0, 0x51000008, 0);
    sceVif1PkRef(*packet_slot, texture, 8, 0, 0x51000008, 0);
    if (xglFontState.fontFlags & 0x80) {
        func_00181FA0();
    }
    sceVif1PkCnt(*packet_slot, 0);
    sceVif1PkOpenDirectHLCode(*packet_slot, 0);
    /* packet_slot is the packet member at the start of the draw cursor */
    func_001820D0((FontDrawCursor *) packet_slot);
}

#define SCRATCHPAD_BASE 0x70000000
#define FONT_SCREEN_ORIGIN_X 0x700
#define FONT_SCREEN_ORIGIN_Y 0x720
#define FONT_COLOR_INITIAL 127
#define FONT_ALPHA_SOURCE_BLEND 0x44
#define GS_ALPHA_OPAQUE 0x80

/* Draws one queued text from text and returns the address of its terminating zero. */
unsigned char *func_00184B30(FontDrawCursor *cursor, unsigned char *text);
/* The two quadwords that end the font packet (X2 test_15). */
extern unsigned char D_004B3410[];

/* XS2 xglFontFlush: sets up the drawing cursor at the base of the scratchpad, starts the font packet,
 * draws every text queued behind xglFontState (zero-separated, ended by an empty one), closes the packet
 * and empties the queue. */
void func_00185A20(void)
{
    FontDrawCursor *cursor = (FontDrawCursor *) SCRATCHPAD_BASE;
    unsigned char *text;

    *xglFontState.queue_cursor = 0;
    cursor->depth = -1;
    cursor->blue = FONT_COLOR_INITIAL;
    cursor->green = FONT_COLOR_INITIAL;
    cursor->red = FONT_COLOR_INITIAL;
    cursor->offset_x = FONT_SCREEN_ORIGIN_X;
    cursor->offset_y = FONT_SCREEN_ORIGIN_Y;
    cursor->drawFlags = 0;
    cursor->effect_y = cursor->effect_x = cursor->effect = 1;
    cursor->effect_red = 0;
    cursor->scale_x = FONT_SCALE_ONE;
    cursor->effect_green = 0;
    cursor->alpha_abcd = FONT_ALPHA_SOURCE_BLEND;
    cursor->alpha = GS_ALPHA_OPAQUE;
    cursor->effect_blue = 0;
    cursor->effect_alpha = 0;
    cursor->alpha_fix = 0;
    cursor->scale_y = FONT_SCALE_ONE;
    cursor->packet = xglPacketGetCurrent();
    xglFontBuildPacket(&cursor->packet, 0);
    xglFontState.unmodeled_08 = 0;
    text = (unsigned char *) (&xglFontState + 1);
    while (*text != 0) {
        text = func_00184B30(cursor, text) + 1;
    }
    sceVif1PkAddDirectDataN(cursor->packet, D_004B3410, 2);
    sceVif1PkCloseDirectHLCode(cursor->packet);
    xglFontBufferReset();
}


void xglFontUpdateState(void)
{
    func_00185A20();
}

/* The FontState flags halfword (0x007E7E04), read and written through the splat symbol. */
extern unsigned short xglFontStateFlags[];

#define FONT_STATE_DEBUG_PRINT 2

unsigned short xglFontGetFlags(void)
{
    return xglFontStateFlags[0];
}

void xglFontSetFlags(int fontFlags)
{
    xglFontStateFlags[0] = fontFlags & (0xFFFF & ~FONT_STATE_DEBUG_PRINT);
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00185B68);

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00185CB0);

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00185D58);

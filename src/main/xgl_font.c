#include "common.h"
#include "shared.h"
#include "main/xgl_font.h"
#include "main/xgl_cd.h"
#include "main/xgl_debug_file_004.h"

struct XglPacket;

/* Packet that FONT_OP_EFFECT_PACKET builds in the cursor's effect area: an A+D GIF tag and one
 * register write. */
struct FontEffectScratchPacket {
    u64 vif_tag;
    u64 vif_count;
    u64 effect_state;
    u64 payload_count;
};

/* One corner of the mark sprite: texture u/v, two unused words, x, y and z. */
struct FontMarkCorner {
    unsigned int texture_u_or_x;
    unsigned int texture_v_or_y;
    unsigned int backup_x;
    unsigned int backup_y;
    unsigned int x;
    unsigned int y;
    unsigned int depth;
    unsigned int depth_upper;
};

/* The GIF packet a mark is drawn with, in the scratchpad at +0x100 (the draw cursor's effect area). */
struct FontMarkPacket {
    unsigned int tag_low;
    unsigned int tag_high;
    unsigned int registers_low;
    unsigned int registers_high;
    unsigned long long texture;
    unsigned char unmodeled_18[8];
    unsigned int red;
    unsigned int green;
    unsigned int blue;
    unsigned int alpha;
    struct FontMarkCorner corners[4];
};

typedef struct FontDrawCursor { struct XglPacket *packet; short origin_x; unsigned char unmodeled_06[2]; unsigned short x; unsigned short y; int depth; unsigned short offset_x; unsigned short offset_y; unsigned char red; unsigned char green; unsigned char blue; unsigned char unmodeled_17; unsigned char drawFlags; unsigned char effect; unsigned char attributes; unsigned char draw_state_high; unsigned char unmodeled_1c[4]; signed char effect_x; signed char effect_y; unsigned char unmodeled_22[2]; unsigned char effect_red; unsigned char effect_green; unsigned char effect_blue; unsigned char effect_alpha; /* The GS ALPHA_1 value: the A/B/C/D selectors and FIX. */ unsigned char alpha_abcd; unsigned char alpha_fix; unsigned char alpha; unsigned char unmodeled_2b; unsigned short scale_x; unsigned short scale_y; union { struct FontEffectScratchPacket effect_packet; u64 qwords[(0x4000 - 0x30) / sizeof(u64)]; struct { unsigned char unmodeled_30[0x100 - 0x30]; struct FontMarkPacket sprite; } mark; } effectArea; } FontDrawCursor;

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
#define EUC_ROW_CYRILLIC 0xa7
#define EUC_ROW_BOX_DRAWING 0xa8
#define EUC_ROW_HALF_WIDTH 0xa9
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
int func_00185770(int glyph, int *leftBound);
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
/* Set by "$rubys;" while the annotation is drawn in the compact size. */
#define FONT_CURSOR_COMPACT 1
/* Set between "$rubyb;" and "$rubye;": the base text of a ruby annotation was marked. */
#define FONT_CURSOR_RUBY 0x10
/* The annotation is raised by ruby_offset_y plus this many sixteenths of its compact glyph height (XS2: 8). */
#define FONT_RUBY_RAISE_SIXTEENTHS 12

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
unsigned int xglFontGetDebugCell(unsigned char eucRow, unsigned char eucColumn);
int xglFontGetGlyphIndex(unsigned char eucRow, unsigned char eucColumn);
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
/* Set above the 12-bit glyph index on some symbol glyphs; xglFontGetProportionalSize masks it off. */
#define FONT_GLYPH_SYMBOL_FLAG 0x1000
/* The symbol-row columns whose glyphs carry FONT_GLYPH_SYMBOL_FLAG: 0xB9, 0xBC and all from 0xF9 on. */
#define EUC_SYMBOL_FLAGGED_A 0xb9
#define EUC_SYMBOL_FLAGGED_B 0xbc
#define EUC_SYMBOL_FLAGGED_FROM 0xf9

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

unsigned short xglFontCharToEuc(signed char character)
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

struct FontStateQueue {
    unsigned char *texture_buffer;
    unsigned short fontFlags;
    unsigned char font_selection;
    unsigned char unmodeled_07;
    /* Glyph images uploaded since the font packet was built (the glyph draw counts them). */
    unsigned short glyph_upload_count;
    unsigned char unmodeled_0a[2];
    unsigned char default_width;
    /* Added to each glyph advance; "$xx7,N;". */
    signed char character_spacing;
    /* FONT_CONFIG_WRAP_WIDTH: a space starts a new line when the next word would end past it (0: no wrap). */
    unsigned short wrap_width;
    /* FONT_CONFIG_DEFAULT_WIDTH, "$xx1,N;". */
    unsigned char glyph_default_width;
    /* FONT_MODE_* bits, set by $prop/$han and the ruby annotation. */
    unsigned char text_mode;
    /* FONT_CONFIG_PROPORTIONAL_MODE, "$xx3,N;". */
    unsigned char proportional_mode;
    /* Slant of the glyphs in pixels: the glyph draw shifts the top corners right by it (X2 horizontal_shift). */
    unsigned char horizontal_shift;
    /* Set: the glyphs are depth tested (TEST_1 GEQUAL) and do not write Z. */
    unsigned char depth_test;
    unsigned char unmodeled_15[7];
    /* Text address of the include command that prefixes each debug line; 0 when none is set. */
    unsigned int debug_prefix;
    /* "$rubyb;" marks where the base text of a ruby annotation starts. */
    const unsigned char *ruby_base;
    /* From the end of the annotation back to the end of its base text; set by "$rubys;". */
    unsigned short ruby_advance;
    unsigned short ruby_offset_y;
    union {
        unsigned short default_scale;
        struct {
            signed char compact_width;
            signed char compact_height;
        };
    };
    unsigned char unmodeled_2a[6];
    unsigned char word_buffer[256];
    unsigned char *queue_cursor;
};

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
/* The font's Z test when xglFontState.depth_test is set, else always. */
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
    ztst = xglFontState.depth_test != 0 ? FONT_ZTST_GEQUAL : FONT_ZTST_ALWAYS;
    packet[0] = GIF_TAG_NREG_1 | GIF_TAG_EOP | 2;
    packet[1] = GIF_AD;
    packet[2] = GS_SET_ZBUF(0, GS_PSMZ24, xglFontState.depth_test ^ 1);
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

/* Advance of an ASCII glyph drawn with the narrow font, before the glyph scale. */
#define FONT_ASCII_ADVANCE 16

/*
 * Width of `text` in pixels up to its end or, when `delimiter` is set, up to the next space after the
 * first character. Inline "$...;" commands and control codes update the text mode, the glyph scale and
 * the character spacing on the way, as the drawing code would.
 */
int xglFontGetStringWidth2(const char *text, int delimiter)
{
    int compact = 0;
    int width = 0;
    int scale = FONT_SCALE_ONE;
    int first = 1;
    unsigned char high;

    while (*text != 0 && (high = *text, !(!first && delimiter != 0 && high == ' '))) {
        if (high == '$') {
            if (text[1] == 'p' && text[2] == 'r' && text[3] == 'o' && text[4] == 'p') {
                xglFontState.text_mode &= ~FONT_MODE_PROP;
                xglFontState.text_mode |= text[5] & FONT_MODE_PROP;
            } else if (text[1] == 'h' && text[2] == 'a' && text[3] == 'n') {
                xglFontState.text_mode &= ~FONT_MODE_HAN;
                xglFontState.text_mode |= (text[4] & 1) << 1;
            } else if (text[1] == 'x' && text[2] == 'x') {
                if (text[3] == '1' && text[4] == ',') {
                    const char *number = text + 5;
                    int value;

                    xglFontParseNumber(&number, &value);
                    xglFontState.glyph_default_width = value;
                }
                if (text[3] == '3' && text[4] == ',') {
                    const char *number = text + 5;
                    int value;

                    xglFontParseNumber(&number, &value);
                    xglFontState.proportional_mode = value;
                }
                if (text[3] == '7' && text[4] == ',') {
                    const char *number = text + 5;
                    int value;

                    xglFontParseNumber(&number, &value);
                    xglFontState.character_spacing = value;
                }
            } else if (text[1] == 'r' && text[2] == 'u' && text[3] == 'b' && text[4] == 'y' &&
                       text[5] == 's' && text[6] == ';') {
                const unsigned char *scan = (const unsigned char *) text + 7;

                while (*scan != 0) {
                    if (scan[0] == '$' && scan[1] == 'r' && scan[2] == 'u' && scan[3] == 'b' &&
                        scan[4] == 'y' && scan[5] == 'e' && scan[6] == ';') {
                        text = (const char *) scan;
                        break;
                    }
                    if (*scan < 0x20) {
                        scan += xglFontGetSPcodeSize(*scan, scan + 1);
                    }
                    scan++;
                }
            } else if (text[1] == 'z' && text[2] == 'o' && text[3] == 'o' && text[4] == 'm') {
                const char *number = text + 5;
                int value;

                xglFontParseNumber(&number, &value);
                scale = value;
            }
            do {
                high = *text++;
            } while (high != ';');
        } else if (high < 0x20) {
            switch (high) {
            case FONT_OP_SCALE:
                scale = text[1];
                break;
            case FONT_OP_CONFIG:
                switch (text[2]) {
                case FONT_CONFIG_DEFAULT_WIDTH:
                    xglFontState.glyph_default_width = text[3];
                    break;
                case FONT_CONFIG_PROPORTIONAL_MODE:
                    xglFontState.proportional_mode = text[3];
                    break;
                }
                break;
            case FONT_OP_COMPACT:
                compact = text[1];
                break;
            case FONT_OP_STYLE:
                xglFontState.text_mode = text[1];
                break;
            }
            text += xglFontGetSPcodeSize(high, (const unsigned char *) text) + 1;
        } else {
            unsigned char low;
            /* The glyph index, then its advance. */
            int glyph;

            if (high < 0xa0) {
                if ((signed char) high >= 0) {
                    if (!(xglFontState.text_mode & FONT_MODE_HAN)) {
                        text++;
                        width += (FONT_ASCII_ADVANCE * scale) >> 4;
                        first = 0;
                        continue;
                    }
                }
            }
            if ((signed char) high >= 0) {
                unsigned short code = xglFontCharToEuc(high);

                text++;
                low = code;
                high = code >> 8;
            } else {
                low = text[1];
                if (xglFontIsTextureGlyphCode(high, low)) {
                    text += 2;
                } else {
                    text++;
                    low = high + 0x80;
                    high = EUC_ROW_HALF_WIDTH;
                }
            }
            glyph = xglFontGetGlyphIndex(high, low);
            if (compact) {
                glyph = func_00185770(xglFontGetProportionalSize(glyph), 0);
            } else if ((xglFontState.text_mode & FONT_MODE_PROP) && glyph < FONT_PROPORTIONAL_GLYPHS) {
                glyph = func_00185770(glyph, 0);
            } else {
                glyph = FONT_GLYPH_ADVANCE;
            }
            width += ((glyph + xglFontState.character_spacing) * scale) >> 4;
        }
        first = 0;
    }
    return width;
}

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
    xglFontState.depth_test = 1;
    xglFontState.compact_width = FONT_SCALE_ONE;
    xglFontState.compact_height = FONT_SCALE_ONE;
    xglFontState.glyph_default_width = 4;
    xglFontState.character_spacing = 0;
    xglFontState.proportional_mode = 0;
    xglFontState.horizontal_shift = 0;
    (*text)++;
    xglFontState.ruby_offset_y = 2;
    xglFontState.wrap_width = 0;
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

/* Inline background command: "N;" with N = 0..3 selects the glyph effect; func_001820D0 sends its
 * TEST_1 setting. */
int xglFontCmdBg(FontDrawCursor *cursor, const char **text)
{
    const unsigned char *next = (const unsigned char *) *text;
    int digit;

    if (next[1] == ';') {
        digit = next[0];
        if (digit < '4') {
            if (digit >= '0') {
                *text = (const char *) (next + 2);
                cursor->effect = next[0] - '0';
                func_001820D0(cursor);
                return 1;
            }
        }
    }
    return 0;
}

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

int xglFontCmdRuby(FontDrawCursor *cursor, const char **text)
{
    const unsigned char *next;
    /* The cursor's address is taken before its first assignment and used for the first parser call:
     * every access to `next` is then a memory access of one kind, and the stores and the reloads
     * after the calls share a register as in the original. The cursor is read as bytes like in bg and
     * han; xglFontParseNumber takes the const char ** of the inline command interface, hence the casts
     * at its three calls. */
    const char **parse = (const char **)&next;
    unsigned char flags;
    int width;
    int height;
    int offset;

    next = (const unsigned char *)*text;
    flags = cursor->drawFlags;
    next += 2;
    switch (next[-2]) {
    case 's':
        if (next[-1] != ';') {
            return 0;
        }
        if (flags & FONT_CURSOR_RUBY) {
            const unsigned char *base;
            unsigned char *word;
            const unsigned char *source;
            unsigned char *out;
            unsigned char c;
            int rise;
            int mode;
            int base_width;
            int ruby_width;

            rise = xglFontState.ruby_offset_y + ((xglFontState.compact_height * FONT_RUBY_RAISE_SIXTEENTHS) >> 4);
            cursor->y -= rise;
            mode = 0;
            if (cursor->drawFlags & FONT_CURSOR_PROPORTIONAL) {
                mode = FONT_MODE_PROP;
            }
            if (cursor->drawFlags & FONT_CURSOR_WIDE_ASCII) {
                mode |= FONT_MODE_HAN;
            }
            xglFontState.text_mode = mode;
            base = xglFontState.ruby_base;
            word = xglFontState.word_buffer;
            while (base < next - 7) {
                *word++ = *base++;
            }
            *word = 0;
            base_width = xglFontGetStringWidth2((const char *)xglFontState.word_buffer, 0);
            out = xglFontState.word_buffer;
            out[0] = FONT_OP_COMPACT;
            out[1] = 1;
            out += 2;
            source = next;
            while (*source != 0) {
                if (*source == '$') {
                    if (source[1] == 'r' && source[2] == 'u' && source[3] == 'b' && source[4] == 'y' &&
                        source[5] == 'e' && source[6] == ';') {
                        break;
                    }
                    while (*source != ';') {
                        *out++ = *source++;
                    }
                } else if (*source < 0x20) {
                    int remaining = xglFontGetSPcodeSize(*source, source);

                    for (; remaining >= 0; remaining--) {
                        *out++ = *source++;
                    }
                } else {
                    if (*source < 0xA0) {
                        *out++ = *source++;
                    } else {
                        out[0] = source[0];
                        out[1] = source[1];
                        out += 2;
                        source += 2;
                    }
                }
            }
            *out = 0;
            ruby_width = xglFontGetStringWidth2((const char *)xglFontState.word_buffer, 0);
            xglFontState.ruby_advance = base_width / 2 - ruby_width / 2;
            cursor->x += -(base_width / 2) - ruby_width / 2;
        }
        xglFontSetTextureFilter(cursor, 0);
        flags |= FONT_CURSOR_COMPACT;
        break;
    case 'e':
        if (next[-1] != ';') {
            return 0;
        }
        if (flags & FONT_CURSOR_RUBY) {
            int rise = xglFontState.ruby_offset_y + ((xglFontState.compact_height * FONT_RUBY_RAISE_SIXTEENTHS) >> 4);

            cursor->y += rise;
            cursor->x += xglFontState.ruby_advance;
        }
        xglFontSetTextureFilter(cursor, 1);
        flags &= ~(FONT_CURSOR_RUBY | FONT_CURSOR_COMPACT);
        break;
    case 'b':
        if (next[-1] != ';') {
            return 0;
        }
        flags |= FONT_CURSOR_RUBY;
        xglFontState.ruby_base = next;
        break;
    case 'z':
        next--;
        if (xglFontParseNumber(parse, &width)) {
            if (next[-1] == ';') {
                height = width;
            } else if (!xglFontParseNumber((const char **)&next, &height)) {
                break;
            }
            xglFontState.compact_width = width;
            xglFontState.compact_height = height;
        }
        break;
    case 'y':
        next--;
        if (xglFontParseNumber((const char **)&next, &offset)) {
            xglFontState.ruby_offset_y = offset;
        }
        break;
    default:
        return 0;
    }
    cursor->drawFlags = flags;
    *text = (const char *)next;
    return 1;
}

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

    xglFontState.text_mode = 0;
    queue_end = (&xglFontState) + 1;
    xglFontState.queue_cursor = (u8 *) queue_end;
    xglFontState.debug_prefix = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00184188);


/* The mark sprite: one textured sprite (GIF tag NLOOP 1, EOP, PRE with PRIM 0x116, six
 * registers TEX0, RGBAQ, UV, XYZ2, UV, XYZ2) from the mark texture (TEX0 word). */
#define FONT_MARK_TAG_LOW 0x8001
#define FONT_MARK_TAG_HIGH 0x608B4000
#define FONT_MARK_REGISTERS 0x535316
#define FONT_MARK_TEX0 0x2007F80665B20000ULL
/* Marks are 64 to a texture row pair: column (code & 0x3F) * 128 texels, row (code & 0xC0) * 2. */
#define FONT_MARK_COLUMN_MASK 0x3F
#define FONT_MARK_ROW_MASK 0xC0
#define FONT_MARK_HEIGHT 0x80
#define FONT_MARK_QWORDS 7

/* Draws mark `code` (width texels wide, 128 high) at the scratchpad draw cursor, scaled by its
 * scale, and moves the cursor on by the drawn width. */
void func_00184798(int code, int width)
{
    FontDrawCursor *cursor = (FontDrawCursor *) 0x70000000;
    struct FontMarkPacket *sprite = &cursor->effectArea.mark.sprite;
    unsigned int left = (((short) cursor->offset_x + (short) cursor->x) * 16) - 8;
    unsigned int top = (((short) cursor->offset_y + (short) cursor->y) * 16) - 9;
    int texture_u = (code & FONT_MARK_COLUMN_MASK) << 7;
    int texture_v = (code & FONT_MARK_ROW_MASK) << 1;
    int draw_width = (width * (short) cursor->scale_x) >> 3;
    int advance = draw_width / 16;
    /* kept: the mark height and the full alpha are both 0x80; the original adds the register it stores the alpha
     * from (written as two constants, the add takes an immediate). */
    int full = FONT_MARK_HEIGHT;

    sprite->tag_low = FONT_MARK_TAG_LOW;
    sprite->tag_high = FONT_MARK_TAG_HIGH;
    sprite->registers_low = FONT_MARK_REGISTERS;
    sprite->registers_high = 0;
    sprite->texture = FONT_MARK_TEX0;
    sprite->red = cursor->red;
    sprite->green = cursor->green;
    sprite->blue = cursor->blue;
    sprite->alpha = full;
    sprite->corners[0].texture_u_or_x = texture_u;
    sprite->corners[0].texture_v_or_y = texture_v;
    sprite->corners[0].x = left;
    sprite->corners[0].y = top;
    sprite->corners[0].depth = cursor->depth;
    sprite->corners[0].depth_upper = 0;
    sprite->corners[1].texture_u_or_x = texture_u + width;
    sprite->corners[1].texture_v_or_y = texture_v + full;
    sprite->corners[1].x = left + draw_width;
    sprite->corners[1].y = top + (short) cursor->scale_y * 16;
    sprite->corners[1].depth = cursor->depth;
    sprite->corners[1].depth_upper = 0;
    cursor->x += advance;
    sceVif1PkAddDirectDataN(cursor->packet, sprite, FONT_MARK_QWORDS);
}

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

/* Glyph index of an EUC-JP code. Returns int, unlike XS2's unsigned short: the shared return is not
 * truncated, only the kanji and pair results are (an unsigned short return adds the masks). */
int xglFontGetGlyphIndex(unsigned char eucRow, unsigned char eucColumn)
{
    int glyph;

    if (eucRow < EUC_ROW_KANJI_LEVEL1) {
        glyph = FONT_GLYPH_NONE;
        switch (eucRow) {
        case EUC_ROW_SYMBOLS:
            glyph = (unsigned short)(eucColumn - EUC_FIRST);
            if (eucColumn == EUC_SYMBOL_FLAGGED_A || eucColumn == EUC_SYMBOL_FLAGGED_B
                || eucColumn >= EUC_SYMBOL_FLAGGED_FROM) {
                glyph |= FONT_GLYPH_SYMBOL_FLAG;
            }
            break;
        case EUC_ROW_SYMBOLS_2:
            switch (eucColumn & 0xF0) {
            case 0xA0:
                glyph = (eucColumn & 0xF) + FONT_GLYPH_SYMBOL_FLAG + 93;
                break;
            case 0xE0:
                glyph = (eucColumn & 0xF) + 108;
                break;
            case 0xF0:
                glyph = (eucColumn & 0xF) + 117;
                break;
            }
            break;
        case EUC_ROW_ALNUM:
            switch (eucColumn & 0xE0) {
            case 0xA0:
                glyph = (eucColumn & 0xF) + 128;
                break;
            case 0xC0:
                glyph = (eucColumn & 0x1F) + 137;
                break;
            case 0xE0:
                glyph = (eucColumn & 0x1F) + 163;
                break;
            }
            break;
        case EUC_ROW_HIRAGANA:
            glyph = eucColumn + 31;
            break;
        case EUC_ROW_KATAKANA:
            glyph = eucColumn + 127;
            break;
        case EUC_ROW_GREEK:
            if (eucColumn < 0xC0) {
                glyph = (eucColumn & 0x1F) + 383;
            } else {
                glyph = (eucColumn & 0x1F) + 407;
            }
            break;
        case EUC_ROW_CYRILLIC:
            glyph = (unsigned short)(eucColumn < 0xD0 ? eucColumn + 271 : eucColumn + 256);
            break;
        case EUC_ROW_BOX_DRAWING:
            glyph = eucColumn + 351;
            break;
        case EUC_ROW_HALF_WIDTH:
            glyph = eucColumn + 256;
            break;
        case EUC_ROW_NEC_SPECIAL:
            glyph = eucColumn + 383;
            break;
        }
    } else if (eucRow < EUC_ROW_KANJI_LEVEL2) {
        glyph = (unsigned short)((eucRow - EUC_ROW_KANJI_LEVEL1) * FONT_JIS_ROW_SIZE + (eucColumn - EUC_FIRST)
                                 + FONT_GLYPH_KANJI_BASE);
    } else {
        glyph = (unsigned short)xglFontFindPairGlyph(eucRow, eucColumn);
    }
    return glyph;
}

/* Position commands: the coordinate bias of the compact x/y codes, the add and keep-origin bits of
 * FONT_OP_POSITION and the tab stop width. */
#define FONT_POS_ADD_X 0x08
#define FONT_POS_ADD_Y 0x10
#define FONT_POS_KEEP_ORIGIN 0x80
#define FONT_POS_X_BIAS 0x100
#define FONT_POS_Y_BIAS 0x400
#define FONT_TAB_WIDTH 64
/* The cursor style bits FONT_OP_RESET clears. */
#define FONT_CURSOR_STYLE_BITS (FONT_CURSOR_COMPACT | FONT_CURSOR_PROPORTIONAL | FONT_CURSOR_WIDE_ASCII)
/* An effect mode of FONT_OP_NEWLINE's value starts a new line instead. */
#define FONT_EFFECT_NEWLINE 10
/* Hex glyphs that are not digits. */
#define FONT_HEX_MINUS 0x10
#define FONT_HEX_POINT 0x11
#define FONT_HEX_BLANK 0x20
/* A real number: digits 0..7 of the integer part, then eight fraction digits. */
#define FONT_REAL_INTEGER_DIGITS 8
#define FONT_REAL_PRINTED_DIGITS 9
#define FONT_FLOAT_MANTISSA_MASK 0x7FFFFF
#define FONT_FLOAT_HIDDEN_BIT 0x800000
#define FONT_FLOAT_EXPONENT_SHIFT_BASE 150
/* The mark glyph drawn for the codes 0x80..0x9F. */
#define FONT_MARK_UNKNOWN 0x7F
/* Glyph images: the proportional and compact glyphs from index FONT_GLYPH_SPAN_BASE on, fixed glyphs FONT_GLYPH_FIXED_SPAN
 * texels wide; heights in 12.4 texels. */
#define FONT_GLYPH_SPAN_BASE 0x240
#define FONT_GLYPH_FIXED_SPAN 0x130
#define FONT_GLYPH_HEIGHT 0x180
#define FONT_GLYPH_COMPACT_HEIGHT 0xC0
#define FONT_PROPORTIONAL_GLYPHS 0x180

/* FONT_OP_HEX_DIGITS: digit_index + 1 hex digits of a 32-bit value. */
struct FontLiteralHexCommand {
    unsigned char opcode;
    unsigned char digit_index;
    unsigned char value_high;
    unsigned char value_mid_high;
    unsigned char value_mid_low;
    unsigned char value_low;
};

/* FONT_OP_INCLUDE: the address of a text that is drawn in place. */
struct FontIncludeCommand {
    unsigned char opcode;
    unsigned char text_high;
    unsigned char text_mid_high;
    unsigned char text_mid_low;
    unsigned char text_low;
};

typedef void (*FontExtFunc)(FontDrawCursor *cursor, void *argument);

/* FONT_OP_CALLBACK: a function (24-bit address) and its argument. */
struct FontExtCommand {
    unsigned char opcode;
    unsigned char function_high;
    unsigned char function_middle;
    unsigned char function_low;
    unsigned char argument_high;
    unsigned char argument_mid_high;
    unsigned char argument_mid_low;
    unsigned char argument_low;
};

extern unsigned char fontGsResetPacket[];
void func_00184188(int code, int textureU, int size, int height);

/* XS2 xglFontFlushCore: draws the queued text at the cursor, running the control codes below 32 and the inline
 * "$" commands; returns the text position it stopped at (the terminator). */
unsigned char *xglFontFlushCore(FontDrawCursor *cursor, unsigned char *text)
{
    signed char real_digits[16];
    signed char integer_digits[32];

    while (*text != 0) {
        unsigned char opcode;

        if (*text == '$' && xglFontInlineCommand(cursor, (const char **) &text) != 0) {
            continue;
        }
        opcode = *text;
        switch (opcode) {
        case FONT_OP_POS_X_0:
        case FONT_OP_POS_X_1:
        case FONT_OP_POS_X_2: {
            unsigned int position_high = text[0];
            unsigned int position_low = text[1];
            int position;

            text += 2;
            position = position_low + (position_high << 8) - FONT_POS_X_BIAS;
            cursor->x = position;
            cursor->origin_x = position;
            break;
        }
        case FONT_OP_POS_Y_0:
        case FONT_OP_POS_Y_1: {
            unsigned int position_high = text[0];
            unsigned int position_low = text[1];
            int position;

            text += 2;
            position = position_low + (position_high << 8) - FONT_POS_Y_BIAS;
            cursor->y = position;
            break;
        }
        case FONT_OP_DEPTH: {
            int depth = (text[1] << 16) + (text[2] << 8) + text[3];

            text += 4;
            cursor->depth = depth;
            break;
        }
        case FONT_OP_NOP:
            text++;
            break;
        case FONT_OP_POSITION: {
            unsigned char flags = text[1];

            text += 2;
            if (flags & FONT_POS_SET_X) {
                cursor->x = text[1] + (text[0] << 8);
                if (!(flags & FONT_POS_KEEP_ORIGIN)) {
                    cursor->origin_x = cursor->x;
                }
                text += 2;
            }
            if (flags & FONT_POS_SET_Y) {
                cursor->y = text[1] + (text[0] << 8);
                text += 2;
            }
            if (flags & FONT_POS_SET_DEPTH) {
                cursor->depth = (text[0] << 16) + (text[1] << 8) + text[2];
                text += 3;
            }
            if (flags & FONT_POS_ADD_X) {
                cursor->x += text[1] + (text[0] << 8);
                if (!(flags & FONT_POS_KEEP_ORIGIN)) {
                    cursor->origin_x = cursor->x;
                }
                text += 2;
            }
            if (flags & FONT_POS_ADD_Y) {
                cursor->y += text[1] + (text[0] << 8);
                text += 2;
            }
            break;
        }
        case FONT_OP_TAB:
            text++;
            cursor->x = (cursor->x + FONT_TAB_WIDTH) & ~(FONT_TAB_WIDTH - 1);
            break;
        case FONT_OP_NEWLINE:
            xglFontFlushSubCRLF();
            text++;
            break;
        case FONT_OP_RESET:
            cursor->effect = FONT_EFFECT_PLAIN;
            cursor->red = cursor->green = cursor->blue = FONT_COLOR_NEUTRAL;
            cursor->alpha_abcd = 0;
            cursor->scale_x = FONT_SCALE_ONE;
            cursor->alpha = FONT_ALPHA_NEUTRAL;
            cursor->scale_y = FONT_SCALE_ONE;
            cursor->drawFlags &= ~FONT_CURSOR_STYLE_BITS;
            sceVif1PkAddDirectDataN(cursor->packet, fontGsResetPacket, 4);
            text++;
            break;
        case FONT_OP_COLOR:
            cursor->red = text[1];
            cursor->green = text[2];
            cursor->blue = text[3];
            text += 4;
            break;
        case FONT_OP_EFFECT_MODE:
            if (text[1] == FONT_EFFECT_NEWLINE) {
                xglFontFlushSubCRLF();
            } else {
                cursor->effect = text[1];
                func_001820D0(cursor);
            }
            text += 2;
            break;
        case FONT_OP_EFFECT_PARAMS:
            cursor->effect_x = text[1];
            cursor->effect_y = text[2];
            cursor->effect_red = text[3];
            cursor->effect_green = text[4];
            cursor->effect_blue = text[5];
            text += 6;
            break;
        case FONT_OP_BLEND: {
            struct FontAlphaWrite *packet;

            cursor->alpha_abcd = text[1];
            cursor->alpha_fix = text[2];
            cursor->alpha = text[3];
            packet = xglFontAllocGifPacket(cursor, 1);
            packet->alpha_abcd = cursor->alpha_abcd;
            packet->alpha_fix = cursor->alpha_fix;
            packet->register_address = GS_ALPHA_1;
            text += 4;
            break;
        }
        case FONT_OP_HEX_DIGITS: {
            const struct FontLiteralHexCommand *command = (const struct FontLiteralHexCommand *) text;
            unsigned int value = ((unsigned int) command->value_high << 24) + ((unsigned int) command->value_mid_high << 16)
                + ((unsigned int) command->value_mid_low << 8) + command->value_low;
            int digit = command->digit_index;

            while (digit >= 0) {
                xglFontFlushSubHex((value >> (digit * 4)) & 15);
                digit--;
            }
            text += sizeof(*command);
            break;
        }
        case FONT_OP_NUMBER: {
            const struct FontNumericCommand *command = (const struct FontNumericCommand *) text;

            if (command->radix == FONT_NUMERIC_FLOAT) {
                unsigned int mid_high = (unsigned int) command->numberSecondByte << 16;
                unsigned int high = (unsigned int) command->numberTopByte << 24;
                unsigned int mid = ((unsigned int) command->numberThirdByte << 8) + mid_high;
                unsigned int bits = high + command->numberLowByte + mid;
                unsigned int mantissa = (bits & FONT_FLOAT_MANTISSA_MASK) + FONT_FLOAT_HIDDEN_BIT;
                unsigned int shift = FONT_FLOAT_EXPONENT_SHIFT_BASE - ((bits >> 23) & 255);
                long long integer;
                unsigned long long mask;
                unsigned long long fraction;
                int digit;
                int printed;
                int remaining;

                if ((int) bits < 0) {
                    xglFontFlushSubHex(FONT_HEX_MINUS);
                } else {
                    xglFontFlushSubHex(FONT_HEX_BLANK);
                }
                if (shift >= 32) {
                    integer = 0;
                    mask = (0x100000000ULL << (shift - 32)) - 1;
                } else {
                    integer = mantissa >> shift;
                    mask = (int) ((1U << shift) - 1);
                }
                fraction = mantissa & mask;
                for (digit = FONT_REAL_INTEGER_DIGITS - 1; digit >= 0; digit--) {
                    real_digits[digit] = integer % 10;
                    integer /= 10;
                    fraction *= 10;
                    if (shift >= 32) {
                        real_digits[digit + FONT_REAL_INTEGER_DIGITS] = ((long long) fraction >> 32) >> (shift - 32);
                    } else {
                        real_digits[digit + FONT_REAL_INTEGER_DIGITS] = (long long) fraction >> shift;
                    }
                    fraction &= mask;
                }
                printed = 0;
                for (digit = 0; digit < FONT_REAL_INTEGER_DIGITS - 1; digit++) {
                    if (real_digits[digit] || printed) {
                        printed++;
                        xglFontFlushSubHex(real_digits[digit]);
                    }
                }
                xglFontFlushSubHex(real_digits[FONT_REAL_INTEGER_DIGITS - 1]);
                xglFontFlushSubHex(FONT_HEX_POINT);
                printed += 2;
                remaining = FONT_REAL_INTEGER_DIGITS - 1;
                for (; remaining >= 0 && printed < FONT_REAL_PRINTED_DIGITS; remaining--) {
                    printed++;
                    xglFontFlushSubHex(real_digits[remaining + FONT_REAL_INTEGER_DIGITS]);
                }
            } else {
                int suppress_zero = 0;
                int radix = command->radix;
                int digit = command->digits_minus_one;
                unsigned int value = ((unsigned int) command->numberTopByte << 24)
                    + ((unsigned int) command->numberSecondByte << 16) + ((unsigned int) command->numberThirdByte << 8)
                    + command->numberLowByte;

                if (radix == 10) {
                    suppress_zero = 1;
                    if ((int) value < 0) {
                        value = -value;
                        suppress_zero = 2;
                    }
                }
                integer_digits[digit + 1] = -1;
                while (digit >= 0) {
                    integer_digits[digit] = value % radix;
                    value /= radix;
                    digit--;
                }
                for (digit = 0; integer_digits[digit] >= 0; digit++) {
                    if (suppress_zero && !integer_digits[digit] && integer_digits[digit + 1] >= 0) {
                        if (suppress_zero == 2 && integer_digits[digit + 1] > 0) {
                            xglFontFlushSubHex(FONT_HEX_MINUS);
                        } else {
                            xglFontFlushSubHex(FONT_HEX_BLANK);
                        }
                    } else {
                        xglFontFlushSubHex(integer_digits[digit]);
                        suppress_zero = 0;
                    }
                }
            }
            text += sizeof(*command);
            break;
        }
        case FONT_OP_SCALE:
            cursor->scale_x = text[1];
            cursor->scale_y = text[2];
            xglFontSetTextureFilter(cursor, 1);
            text += 3;
            break;
        case FONT_OP_EFFECT_PACKET: {
            struct FontEffectScratchPacket *packet =
                (struct FontEffectScratchPacket *) (((unsigned int) cursor->effectArea.qwords + 0xF) & ~0xF);
            long long horizontal_mode;
            long long horizontal_mask;
            long long vertical_mask;
            long long vertical_mode;
            long long horizontal_random;
            long long vertical_random;

            packet->vif_tag = GIF_TAG_NREG_1 | GIF_TAG_EOP | 1;
            packet->vif_count = GIF_AD;
            if (!text[1]) {
                horizontal_mode = 1;
                horizontal_mask = horizontal_random = 0;
            } else {
                unsigned long long mask_byte = text[1];

                horizontal_mode = 3;
                horizontal_mask = mask_byte ^ 0x3FF;
                if (mask_byte & 0x80) {
                    horizontal_random = text[1] & xglSRand();
                } else {
                    horizontal_random = 0;
                }
            }
            if (!text[2]) {
                vertical_mode = 1;
                vertical_mask = vertical_random = 0;
            } else {
                vertical_mode = 3;
                vertical_mask = text[1] ^ 0x3FF;
                if (text[2] & 0x80) {
                    vertical_random = text[1] & xglSRand();
                } else {
                    vertical_random = 0;
                }
            }
            packet->payload_count = 8;
            packet->effect_state = horizontal_mode | (vertical_mode << 2) | (horizontal_mask << 4)
                | (horizontal_random << 14) | (vertical_mask << 24) | (vertical_random << 34);
            sceVif1PkAddDirectDataN(cursor->packet, packet, 2);
            text += 3;
            break;
        }
        case FONT_OP_CONFIG: {
            const unsigned char *payload = text + 2;
            int length = text[1];

            text += length + 2;
            switch (payload[0]) {
            case FONT_CONFIG_LINE_HEIGHT:
                if (length == 2) {
                    xglFontState.default_width = payload[1];
                }
                break;
            case FONT_CONFIG_DEFAULT_WIDTH:
                if (length == 2) {
                    xglFontState.glyph_default_width = payload[1];
                }
                break;
            case FONT_CONFIG_WRAP_WIDTH:
                if (length == 3) {
                    xglFontState.wrap_width = (payload[1] << 8) + payload[2];
                }
                break;
            case FONT_CONFIG_PROPORTIONAL_MODE:
                if (length == 2) {
                    xglFontState.proportional_mode = payload[1];
                }
                break;
            case FONT_CONFIG_HORIZONTAL_SHIFT:
                if (length == 2) {
                    xglFontState.horizontal_shift = payload[1];
                }
                break;
            }
            break;
        }
        case FONT_OP_INCLUDE: {
            const struct FontIncludeCommand *command = (const struct FontIncludeCommand *) text;
            unsigned char *included = (unsigned char *) (((unsigned int) command->text_high << 24)
                + ((unsigned int) command->text_mid_high << 16) + ((unsigned int) command->text_mid_low << 8)
                + command->text_low);

            text += sizeof(*command);
            xglFontFlushCore(cursor, included);
            break;
        }
        case FONT_OP_CALLBACK: {
            const struct FontExtCommand *command = (const struct FontExtCommand *) text;
            FontExtFunc callback = (FontExtFunc) (((unsigned int) command->function_high << 16)
                + ((unsigned int) command->function_middle << 8) + command->function_low);
            void *argument = (void *) (((unsigned int) command->argument_high << 24)
                + ((unsigned int) command->argument_mid_high << 16) + ((unsigned int) command->argument_mid_low << 8)
                + command->argument_low);

            text += sizeof(*command);
            callback(cursor, argument);
            break;
        }
        case FONT_OP_COMPACT:
            if (text[1]) {
                xglFontSetTextureFilter(cursor, 0);
                cursor->drawFlags |= FONT_CURSOR_COMPACT;
            } else {
                xglFontSetTextureFilter(cursor, 1);
                cursor->drawFlags &= ~FONT_CURSOR_COMPACT;
            }
            text += 2;
            break;
        case FONT_OP_STYLE: {
            unsigned char bits = text[1];
            unsigned char flags = cursor->drawFlags & (~(FONT_CURSOR_PROPORTIONAL | FONT_CURSOR_WIDE_ASCII) & 0xFF);

            text += 2;
            if (bits & FONT_STYLE_PROPORTIONAL) {
                flags |= FONT_CURSOR_PROPORTIONAL;
            }
            if (bits & FONT_STYLE_WIDE_ASCII) {
                flags |= FONT_CURSOR_WIDE_ASCII;
            }
            cursor->drawFlags = flags;
            break;
        }
        case FONT_OP_GLYPH:
            xglFontFlushSubMark(text[1]);
            text += 2;
            break;
        case 20:
        case 26:
        case 28:
        case 29:
        case 30:
        case 31:
            break;
        default: {
            struct FontStateQueue *state;
            unsigned char high = opcode;
            unsigned char low;
            unsigned int code;

            if ((signed char) high >= 0) {
                if (cursor->drawFlags & FONT_CURSOR_WIDE_ASCII) {
                    code = xglFontCharToEuc(high);
                    high = code >> 8;
                    low = code;
                } else {
                    code = high;
                    low = 0;
                }
                text++;
            } else {
                /* kept: the original reads the two-byte code through this copy and does not load text again after
                 * the call below; written with text itself it does. */
                unsigned char *glyph = text;

                low = glyph[1];
                if (xglFontIsTextureGlyphCode(high, low)) {
                    text = glyph + 2;
                    code = ((high << 8) + low) & 0xFFFF;
                } else {
                    low = high + 0x80;
                    high = 0xA9;
                    text = glyph + 1;
                    code = ((high << 8) + low) & 0xFFFF;
                }
            }
            state = &xglFontState;
            if ((short) state->wrap_width > 0 && opcode == ' '
                && (short) cursor->x - cursor->origin_x + xglFontGetStringWidth2((const char *) text, 1)
                    > (short) state->wrap_width) {
                xglFontFlushSubCRLF();
            } else if (code < 0x80) {
                xglFontFlushSubMark(opcode);
            } else if (code < 0xA0) {
                xglFontFlushSubMark(FONT_MARK_UNKNOWN);
            } else {
                code = xglFontGetGlyphIndex(high, low);
                if (cursor->drawFlags & FONT_CURSOR_COMPACT) {
                    int left;
                    int width;

                    code = (unsigned short) xglFontGetProportionalSize(code);
                    width = func_00185770(code, &left);
                    func_00184188(code + FONT_GLYPH_SPAN_BASE, left, width << 4, FONT_GLYPH_COMPACT_HEIGHT);
                } else if ((cursor->drawFlags & FONT_CURSOR_PROPORTIONAL) && code < FONT_PROPORTIONAL_GLYPHS) {
                    int left;
                    int width = func_00185770(code, &left);

                    func_00184188(code + FONT_GLYPH_SPAN_BASE, left, width << 4, FONT_GLYPH_HEIGHT);
                } else {
                    func_00184188(code, 0, FONT_GLYPH_FIXED_SPAN, FONT_GLYPH_HEIGHT);
                }
            }
            break;
        }
        }
    }
    return text;
}

/* The glyph span table behind the glyph pixels of the font texture: per glyph a halfword with the
 * first column in the low byte and the column after the last in the high byte, both stored minus one
 * (the read adds 0x100). */
#define FONT_GLYPH_SPAN_OFFSET 0x78000
#define FONT_GLYPH_SPAN_BIAS 0x100
/* The fixed cell (proportional_mode set): half = mode / 2, FONT_FIXED_DEFAULT_HALF when that is 0;
 * the glyph spans columns FONT_FIXED_CENTER + 1 - half up to FONT_FIXED_CENTER + half. */
#define FONT_FIXED_CENTER 10
#define FONT_FIXED_DEFAULT_HALF 5

/* The drawn width of a glyph: the fixed cell when proportional_mode is set, the default width for
 * glyph 0, else the glyph's span from the table. A non-null leftBound receives the first column in
 * 1/16 pixel units. */
int func_00185770(int glyph, int *leftBound)
{
    unsigned char left;
    int right;
    unsigned char mode;

    if (xglFontState.proportional_mode != 0) {
        mode = xglFontState.proportional_mode;
        left = mode >> 1;
        if (left == 0) {
            left = FONT_FIXED_DEFAULT_HALF;
        }
        right = left + FONT_FIXED_CENTER;
        left = FONT_FIXED_CENTER + 1 - left;
    } else if (glyph == 0) {
        right = xglFontState.glyph_default_width;
        left = 0;
    } else {
        unsigned short span;

        span = ((unsigned short *) (xglFontState.texture_buffer + FONT_GLYPH_SPAN_OFFSET))[glyph]
            + FONT_GLYPH_SPAN_BIAS;
        left = span;
        right = span >> 8;
        if (left != 0) {
            left = left - 1;
        }
    }
    if (leftBound != 0) {
        *leftBound = left << 4;
    }
    return right - left + 1;
}

extern unsigned char D_004B1E20[];
extern unsigned char D_004B3320[];
extern unsigned char D_004B33A0[];
void sceVif1PkCloseDirectHLCode();
void sceVif1PkCnt();
void sceVif1PkOpenDirectHLCode();

/* reloadMode: the flush passes 0; this build does not read it. */
void xglFontBuildPacket(FontDrawCursor *cursor, int reloadMode)
{
    XglPacket *packet;
    u32 *direct_state;
    u8 *texture;

    packet = cursor->packet;
    if (packet->directCode != 0) {
        direct_state = packet->directState;
        if (direct_state != 0) {
            sceVif1PkCloseDirectHLCode(packet);
            packet = cursor->packet;
            if (direct_state[0] == 0x51000000) {
                packet->write_cursor -= 4;
            }
        }
    }
    sceVif1PkRef(cursor->packet, D_004B1E20, 0x107, 0, 0, 0);
    sceVif1PkRef(cursor->packet, D_004B3320, 8, 0, 0, 0);
    texture = xglFontState.texture_buffer + 0x1A0;
    sceVif1PkRef(cursor->packet, D_004B33A0, 7, 0, 0, 0);
    sceVif1PkRef(cursor->packet, texture, 8, 0, 0x51000008, 0);
    sceVif1PkRef(cursor->packet, texture, 8, 0, 0x51000008, 0);
    sceVif1PkRef(cursor->packet, texture, 8, 0, 0x51000008, 0);
    sceVif1PkRef(cursor->packet, texture, 8, 0, 0x51000008, 0);
    sceVif1PkRef(cursor->packet, texture, 8, 0, 0x51000008, 0);
    sceVif1PkRef(cursor->packet, texture, 8, 0, 0x51000008, 0);
    sceVif1PkRef(cursor->packet, texture, 8, 0, 0x51000008, 0);
    sceVif1PkRef(cursor->packet, texture, 8, 0, 0x51000008, 0);
    if (xglFontState.fontFlags & 0x80) {
        func_00181FA0();
    }
    sceVif1PkCnt(cursor->packet, 0);
    sceVif1PkOpenDirectHLCode(cursor->packet, 0);
    func_001820D0(cursor);
}

#define SCRATCHPAD_BASE 0x70000000
#define FONT_SCREEN_ORIGIN_X 0x700
#define FONT_SCREEN_ORIGIN_Y 0x720
#define FONT_COLOR_INITIAL 127
#define FONT_ALPHA_SOURCE_BLEND 0x44
#define GS_ALPHA_OPAQUE 0x80

/* Draws one queued text from text and returns the address of its terminating zero. */
/* The two quadwords that end the font packet (X2 test_15). */
extern unsigned char D_004B3410[];

/* Sets up the drawing cursor at the base of the scratchpad, starts the font packet,
 * draws every text queued behind xglFontState (zero-separated, ended by an empty one), closes the packet
 * and empties the queue. */
void xglFontFlush(void)
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
    xglFontBuildPacket(cursor, 0);
    xglFontState.glyph_upload_count = 0;
    text = (unsigned char *) (&xglFontState + 1);
    while (*text != 0) {
        text = xglFontFlushCore(cursor, text) + 1;
    }
    sceVif1PkAddDirectDataN(cursor->packet, D_004B3410, 2);
    sceVif1PkCloseDirectHLCode(cursor->packet);
    xglFontBufferReset();
}


void xglFontUpdateState(void)
{
    xglFontFlush();
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

/* Four 16x16 colour patterns: the left and right halves of two images of 16 rows x 32 words
 * (the splat labels D_004B1ED0 / D_004B26D0 mark the right halves). */
#define FONT_PATTERN_SIZE 16
extern unsigned int fontColorPatternA[FONT_PATTERN_SIZE][FONT_PATTERN_SIZE * 2];
extern unsigned int fontColorPatternB[FONT_PATTERN_SIZE][FONT_PATTERN_SIZE * 2];

/* Fills the four patterns from the four colours: the colour index runs along the row (x & 3),
 * in 4x2 cells ((x >> 2) & 1, (y & 1) << 1), in 8x2-row cells ((x >> 3) & 1, y & 2), and down
 * the rows ((y >> 2) & 3). */
void func_00185B68(const unsigned int *colors)
{
    unsigned int *pixel;
    int x;
    int y;

    pixel = &fontColorPatternA[0][0];
    for (y = 0; y < FONT_PATTERN_SIZE; y++) {
        for (x = 0; x < FONT_PATTERN_SIZE; x++) {
            *pixel++ = colors[x & 3];
        }
        pixel += FONT_PATTERN_SIZE;
    }
    pixel = &fontColorPatternA[0][FONT_PATTERN_SIZE];
    for (y = 0; y < FONT_PATTERN_SIZE; y++) {
        for (x = 0; x < FONT_PATTERN_SIZE; x++) {
            *pixel++ = colors[((x >> 2) & 1) | ((y << 1) & 2)];
        }
        pixel += FONT_PATTERN_SIZE;
    }
    pixel = &fontColorPatternB[0][0];
    for (y = 0; y < FONT_PATTERN_SIZE; y++) {
        for (x = 0; x < FONT_PATTERN_SIZE; x++) {
            *pixel++ = colors[((x >> 3) & 1) | (y & 2)];
        }
        pixel += FONT_PATTERN_SIZE;
    }
    pixel = &fontColorPatternB[0][FONT_PATTERN_SIZE];
    for (y = 0; y < FONT_PATTERN_SIZE; y++) {
        for (x = 0; x < FONT_PATTERN_SIZE; x++) {
            *pixel++ = colors[(y >> 2) & 3];
        }
        pixel += FONT_PATTERN_SIZE;
    }
}

/* The font texture file name; the language code goes in at +4 and the font digit at +0xB. */
extern char *xglFontTexturePath;
#define FONT_PATH_LANGUAGE_OFFSET 4
#define FONT_PATH_DIGIT_OFFSET 0xB

/* XS2 xglFontLoad: selects font fontIndex and reads its texture (waiting for the read when notify is
 * CD_NOTIFY_CURRENT, with the dummy callback); a negative index only reports the current font. Returns
 * the previous selection. */
unsigned char xglFontLoad(int fontIndex, CdCompletionCallback *notify)
{
    unsigned char previousFont;
    int readMode;

    previousFont = xglFontState.font_selection;
    if (fontIndex < 0) {
        return previousFont;
    }
    /* kept: the same statements in both branches; GCC merges them, and the original's store and register
     * choice come out only with this branch before the notify test (one copy before it differs). */
    if (notify != CD_NOTIFY_CURRENT) {
        xglFontState.font_selection = fontIndex;
        readMode = CD_READ_ASYNC;
    } else {
        xglFontState.font_selection = fontIndex;
        readMode = CD_READ_ASYNC;
    }
    if (notify == CD_NOTIFY_CURRENT) {
        readMode = 0;
        notify = CD_NOTIFY_DUMMY;
    }
    xglGetLanguageCode(xglFontTexturePath + FONT_PATH_LANGUAGE_OFFSET);
    xglFontTexturePath[FONT_PATH_DIGIT_OFFSET] = fontIndex + '0';
    xglCdReadFile(xglFontTexturePath, xglFontState.texture_buffer, readMode, notify);
    func_00181FA0();
    return previousFont;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_font", func_00185D58);

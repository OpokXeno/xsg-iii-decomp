/* header_harvest: hand-maintained (owner header of main/tu393 njString.c; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_MAIN_NJSTRING_H
#define INCLUDE_MAIN_NJSTRING_H

typedef signed char s8;
typedef int s32;
typedef float f32;
/* A FONT_OP_SCALE code: opcode, x scale, y scale (16 is 1.0) and a terminator. */
typedef struct NjStringScaleCode { unsigned char bytes[4]; } NjStringScaleCode;
typedef unsigned char u8;
typedef short s16;

/* Button icons a string can show: the SCE pad button bits shifted right by 8, and the element attributes. */
#define NJ_STRING_ICON_L2 0x01
#define NJ_STRING_ICON_R2 0x02
#define NJ_STRING_ICON_L1 0x04
#define NJ_STRING_ICON_R1 0x08
#define NJ_STRING_ICON_TRIANGLE 0x10
#define NJ_STRING_ICON_CIRCLE 0x20
#define NJ_STRING_ICON_CROSS 0x40
#define NJ_STRING_ICON_SQUARE 0x80
#define NJ_STRING_ICON_ATTRIBUTE(n) (0x10000 + (n))

#define NJ_STRING_MAX_ICONS 16

/* styleFlags bit: njStringSetText lays the text out at once, which counts its lines and places its icons. */
#define NJ_STRING_STYLE_LAYOUT_ON_SET 0x20

/* A button icon the layout left room for; the caller draws it over that blank. */
typedef struct NjStringIcon {
    s32 x;                              /* 0x00 pen x of the blank, from the string's x */
    s32 button;                         /* 0x04 NJ_STRING_ICON_* */
    s8 line;                            /* 0x08 line index */
} NjStringIcon;

/* The string record the njString setters write. Callers pass it as the object that starts 0xD0 bytes into their own record. */
typedef struct NjString {
    f32 x;                              /* 0x00 */
    f32 y;                              /* 0x04 */
    f32 red;                            /* 0x08 */
    f32 green;                          /* 0x0C */
    f32 blue;                           /* 0x10 */
    f32 alpha;                          /* 0x14 */
    s32 styleFlags;                     /* 0x18 NJ_STRING_STYLE_* */
    unsigned char *text;                /* 0x1C markup text, see njStringLayoutText */
    s32 word20;                         /* 0x20 */
    s32 layoutCount;                    /* 0x24 */
    s32 layoutLimit;                    /* 0x28 */
    unsigned char unmodeled_2C[0x38 - 0x2C];
    struct NjStringIcon icons[NJ_STRING_MAX_ICONS]; /* 0x38 */
    s8 iconCount;                       /* 0xF8 */
    u8 zoomX;                           /* 0xF9 default "$zoom" x, two decimal digits */
    u8 zoomY;                           /* 0xFA default "$zoom" y */
    unsigned char unmodeled_FB[0xFC - 0xFB];
    s32 wordFC;                         /* 0xFC */
    s16 visibleLineSpan;                /* 0x100 lines shown after firstVisibleLine */
    s16 currentLine;                    /* 0x102 line being laid out; the line count after a layout */
    s16 firstVisibleLine;               /* 0x104 */
} NjString;


void njStringSetCounter(NjString *string, s32 limit);
void njStringSetPosition(NjString *string, f32 x, f32 y);
void njStringSetZoom(NjString *string, s8 zoomX, s8 zoomY);
void njStringSetTextNoLayout(NjString *string, unsigned char *text);
void njStringSetWord20(NjString *string, s32 value);
void njStringSetColor(NjString *string, s32 red, s32 green, s32 blue);
void njStringSetAlpha(NjString *string, s32 alpha);
void njStringSetFlags(NjString *string, s32 styleFlags);
s32 njStringTextTooWide(const char *text, s32 maxWidth, s32 scale);
void njStringSetText(NjString *string, unsigned char *text);
void njStringSetPositionB(NjString *string, f32 x, f32 y);
void njStringSetZoomB(NjString *string, s8 zoomX, s8 zoomY);
void njStringSetColorB(NjString *string, s32 red, s32 green, s32 blue);
void njStringSetAlphaB(NjString *string, s32 alpha);
void njStringSetFlagsB(NjString *string, s32 styleFlags);
void njStringSetFirstVisibleLine(NjString *string, s32 line);
void njStringSetVisibleLineSpan(NjString *string, s32 span);
void njStringSetWordFC(NjString *string, s32 value);
void njStringLayoutText(NjString *string, char *display);
s32 njStringLayoutCount(s32 limit, s32 x, s32 y);

#endif

#ifndef INCLUDE_MAIN_SV_GET_IMAGE_ITEM_142_H
#define INCLUDE_MAIN_SV_GET_IMAGE_ITEM_142_H

/* Types owned by main/tu142 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"

/* One 128-bit quadword (sq/lq width). */
typedef unsigned int Quadword __attribute__((mode(TI)));

/* The draw list at svDrawList (static in sv_get_image_item.c, reached by draw_list.c through the linker symbol):
 * entries is the base of the 16-byte slots, flush_code is the word at +0x4 (40 (0x28) at setup; the same code
 * svDrawListFlush appends as its closing record, and drawListSetFlushCode writes it through its own symbol
 * svDrawListFlushCode), end_index is the number of slots queued so far and begin_index the first slot of the
 * record being built. */
typedef struct DrawList {
    Quadword *entries;
    int flush_code;
    int end_index;
    int begin_index;
} DrawList;

typedef struct DrawListEntry {
    unsigned int flush_word;        /* +0x00: VIF FLUSH, 0x11000000 */
    unsigned int nop_word;          /* +0x04 */
    unsigned int stcycl_word;       /* +0x08: STCYCL 4,4, 0x01000404 */
    unsigned int unpack_word;       /* +0x0C: UNPACK header, code | 0x6C010000 */
    union {
        Quadword payload;           /* +0x10: cleared as one quadword */
        struct {
            unsigned int count_word; /* +0x10 */
            int operand_a;
            int operand_b;
            unsigned int tail_word; /* +0x1C */
        };
    };
} DrawListEntry;

/* One A+D entry: register data, then register address (the GS environment of sdvDrawAlter and the draw packets). */
typedef struct DrawStateRegister {
    u64 data;
    u64 address;
} DrawStateRegister;

/* Eight doublewords of GS state, one row of the draw state tables. */
typedef struct DrawStateWords {
    u64 words[8];
} DrawStateWords;

/* The GS state block at the pointer in svDrawStatePacket: two matrices, one row of the draw state tables, a GIF tag with its
 * A+D register list and three A+D entries. */
typedef struct DrawStatePacket {
    Matrix4 matrixA;                        /* +0x00: the view product written by the matrix pipeline */
    Matrix4 baseMatrix;                     /* +0x40: the base matrix svDrawListAppendRecordA loads (the item matrix) */
    union {
        DrawStateWords stateWords;          /* +0x80: copied from the state table of the item's mode */
        DoublewordVector4 textureState[4];  /* the same row as four A+D entries */
        Vector4 corners[4];                 /* the same quadwords as the quad sefClipPolygon transforms */
    };
    u64 gif_tag;
    u64 gif_registers;
    DrawStateRegister clamp;
    DrawStateRegister zbuf;
    DrawStateRegister alpha;
} DrawStatePacket;

/*
 * The image-mapper table has a 0x2C-byte header followed by 140 entries
 * of 0x24 bytes (5084 bytes per list). These entry fields are evidenced by the lookup routines:
 * the active word is at +0x08, the type byte at +0x0E, and the resource
 * name pointer at +0x20.
 */
typedef struct SvTypeListEntry {
    void *data;
    unsigned char unmodeled_04[4];
    int used;
    short resourceKind;
    union {
        unsigned short typeFlags; /* +0x0e */
        struct { unsigned char type; unsigned char unmodeled_0f; };
    };
    unsigned char unmodeled_10[2];
    unsigned short basePointer;
    unsigned short bitsPerPixel;
    unsigned short bufferWidthPixels;
    unsigned short width;
    unsigned short height;
    unsigned short textureWidthExponent;
    unsigned short textureHeightExponent;
    const char *name;
} SvTypeListEntry;

/* One mapper record: 5084 bytes, a 44-byte header and 140 entries. */
typedef struct SvTypeList {
    int used;                       /* +0x00: nonzero when the mapper slot is in use, cleared by svImageListDestroy */
    int size;                       /* +0x04: 0xE0000 for list 0, 0xF0000 for the others */
    int capacity;                   /* +0x08: 0x10000 */
    unsigned int savedSize;         /* +0x0C: running VRAM cursor, reset to size by svImageListDestroy */
    unsigned char dimensions;       /* +0x10: cleared as one byte by svImageListDestroy */
    unsigned char unmodeled_11[0x13];
    int pendingCount;               /* +0x24: cleared by svImageListDestroy */
    short entryCount;               /* +0x28: cleared by svImageListCreate's loop and svImageListDestroy */
    signed char typeKind;           /* +0x2A: read by tu153 as the script type, cleared by svImageListDestroy */
    unsigned char unmodeled_2B[1];
    SvTypeListEntry entries[140];   /* +0x2C */
} SvTypeList;

#endif

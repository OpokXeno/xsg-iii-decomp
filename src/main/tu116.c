#include "common.h"

typedef int s32;

typedef unsigned char u8;
typedef unsigned long long u64;

/* The draw environment record that drawEnvGetRecord returns, as the window and extended setters write it. The word
   fields at +0x00..+0x10 and +0x3C..+0x48 are not named yet (their order is the parameter order of the setters). */
typedef struct DrawArea {
    s32 unk_00;
    s32 unk_04;
    s32 unk_08;
    s32 unk_0C;
    s32 unk_10;
    s32 frameBase;
    s32 drawMode;
    s32 originX;
    s32 originY;
    s32 width;
    s32 height;
    s32 clipX;
    s32 clipY;
    s32 drawWidth;
    s32 drawHeight;
    s32 unk_3C;
    s32 unk_40;
    s32 unk_44;
    s32 unk_48;
    u8 unmodeled_4C[0x1C];
    u64 drawFlags;
} DrawArea;

void *drawEnvGetRecord();

/* Sets the window words of the draw environment record (the frame base, draw mode, origin, size, clip and draw size). */
void drawAreaSetWindow(s32 frameBase, s32 drawMode, s32 originX, s32 originY, s32 width, s32 height, s32 clipX,
                       s32 clipY, s32 drawWidth, s32 drawHeight) {
    DrawArea *drawArea;

    drawArea = drawEnvGetRecord();
    drawArea->frameBase = frameBase;
    drawArea->drawMode = drawMode;
    drawArea->originX = originX;
    drawArea->originY = originY;
    drawArea->width = width;
    drawArea->height = height;
    drawArea->clipX = clipX;
    drawArea->clipY = clipY;
    drawArea->drawWidth = drawWidth;
    drawArea->drawHeight = drawHeight;
}

/* drawAreaSetWindow plus the unnamed words +0x00, +0x04, +0x3C, +0x40, +0x44 and +0x48 of the record. */
void drawAreaSetExtended(s32 unk_00, s32 unk_3C, s32 unk_44, s32 unk_04, s32 unk_40, s32 unk_48, s32 frameBase,
                         s32 drawMode, s32 originX, s32 originY, s32 width, s32 height, s32 clipX, s32 clipY,
                         s32 drawWidth, s32 drawHeight) {
    DrawArea *drawArea;

    drawArea = drawEnvGetRecord();
    drawArea->unk_00 = unk_00;
    drawArea->unk_3C = unk_3C;
    drawArea->unk_44 = unk_44;
    drawArea->unk_04 = unk_04;
    drawArea->unk_40 = unk_40;
    drawArea->unk_48 = unk_48;
    drawArea->frameBase = frameBase;
    drawArea->drawMode = drawMode;
    drawArea->originX = originX;
    drawArea->originY = originY;
    drawArea->width = width;
    drawArea->height = height;
    drawArea->clipX = clipX;
    drawArea->clipY = clipY;
    drawArea->drawWidth = drawWidth;
    drawArea->drawHeight = drawHeight;
}

/* drawAreaSetExtended's words plus +0x08, +0x0C and +0x10 and the flag bit 48 of the 64-bit flags word (set here). */
void drawAreaSetExtendedWide(s32 unk_00, s32 unk_3C, s32 unk_44, s32 unk_04, s32 unk_40, s32 unk_48, s32 unk_08,
                   s32 unk_0C, s32 unk_10, s32 frameBase, s32 drawMode, s32 originX, s32 originY,
                   s32 width, s32 height, s32 clipX, s32 clipY, s32 drawWidth, s32 drawHeight) {
    DrawArea *drawArea;

    drawArea = drawEnvGetRecord();
    drawArea->unk_00 = unk_00;
    drawArea->unk_3C = unk_3C;
    drawArea->unk_44 = unk_44;
    drawArea->unk_04 = unk_04;
    drawArea->unk_40 = unk_40;
    drawArea->unk_48 = unk_48;
    drawArea->unk_08 = unk_08;
    drawArea->unk_0C = unk_0C;
    drawArea->unk_10 = unk_10;
    drawArea->frameBase = frameBase;
    drawArea->drawMode = drawMode;
    drawArea->originX = originX;
    drawArea->originY = originY;
    drawArea->width = width;
    drawArea->height = height;
    drawArea->clipX = clipX;
    drawArea->clipY = clipY;
    drawArea->drawWidth = drawWidth;
    drawArea->drawHeight = drawHeight;
    drawArea->drawFlags |= 0x0001000000000000ULL;
}

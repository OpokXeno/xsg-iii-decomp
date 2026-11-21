#include "common.h"

typedef int s32;
typedef unsigned char u8;
typedef struct DrawArea
{
  s32 word_00;
  s32 word_04;
  u8 unk_08[0xC];
  s32 frameBase;
  s32 drawMode;
  s32 origin_x;
  s32 origin_y;
  s32 width;
  s32 height;
  s32 clip_x;
  s32 clip_y;
  s32 draw_width;
  s32 draw_height;
  s32 word_3C;
  s32 word_40;
  s32 word_44;
  s32 word_48;
} DrawArea;

void *drawEnvGetRecord();

void drawAreaSetWindow(s32 frameBase, s32 drawMode, s32 origin_x, s32 origin_y, s32 width, s32 height, s32 clip_x, s32 clip_y, s32 draw_width, s32 draw_height) {
    DrawArea *drawArea;

    drawArea = drawEnvGetRecord();
    drawArea->frameBase = frameBase;
    drawArea->drawMode = drawMode;
    drawArea->origin_x = origin_x;
    drawArea->origin_y = origin_y;
    drawArea->width = width;
    drawArea->height = height;
    drawArea->clip_x = clip_x;
    drawArea->clip_y = clip_y;
    drawArea->draw_width = draw_width;
    drawArea->draw_height = draw_height;
}

void drawAreaSetExtended(s32 word_00, s32 word_3C, s32 word_44, s32 word_04, s32 word_40, s32 word_48, s32 frameBase, s32 drawMode, s32 origin_x, s32 origin_y, s32 width, s32 height, s32 clip_x, s32 clip_y, s32 draw_width, s32 draw_height) {
    DrawArea *drawArea;

    drawArea = drawEnvGetRecord();
    drawArea->word_00 = word_00;
    drawArea->word_3C = word_3C;
    drawArea->word_44 = word_44;
    drawArea->word_04 = word_04;
    drawArea->word_40 = word_40;
    drawArea->word_48 = word_48;
    drawArea->frameBase = frameBase;
    drawArea->drawMode = drawMode;
    drawArea->origin_x = origin_x;
    drawArea->origin_y = origin_y;
    drawArea->width = width;
    drawArea->height = height;
    drawArea->clip_x = clip_x;
    drawArea->clip_y = clip_y;
    drawArea->draw_width = draw_width;
    drawArea->draw_height = draw_height;
}

INCLUDE_ASM("asm/main/nonmatchings/tu116", func_001E89E0);

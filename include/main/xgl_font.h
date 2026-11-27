#ifndef INCLUDE_MAIN_XGL_FONT_H
#define INCLUDE_MAIN_XGL_FONT_H

#include "main/xgl_cd.h"

/* The two-byte EUC code of a single-byte character (xgl_font.c). */
unsigned short xglFontCharToEuc(signed char character);
/* Selects a font and reads its texture; returns the previous selection (XS2 xglFontLoad). */
unsigned char xglFontLoad(int fontIndex, CdCompletionCallback *notify);

#endif /* INCLUDE_MAIN_XGL_FONT_H */

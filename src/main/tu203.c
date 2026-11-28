#include "common.h"

#include "main/tu007.h"

/* One value of the scratchpad block at 0x70000000 (the effect work area). */
#define SEF_SCRATCHPAD(type, offset) (*(type *) (0x70000000 + (offset)))
extern struct TextLine {
    char *text;
} *movieListLines;

extern int xglCdGetFileSize(const char *name);
extern void *memset(void *dst, int value, unsigned int size);

const char movieListPageFormat[16] = "%2d / %2d";
const char movieListTextColourGrey[56] = "$bg3;$prop1;$han1;$xx0,36;$xx1,8;$xx2,0;$col707070;";
const char movieListTextColourGreen[56] = "$bg3;$prop1;$han1;$xx0,36;$xx1,8;$xx2,0;$col60c000;";
const char D_0053AFB0[40] = "\xa2" "\xa2" "\xa1" "\xa7" "\xa3" "\xc4" "\xa3" "\xef" "\xa3" "\xec" "\xa3" "\xe2" "\xa3" "\xf9" "\xa3" "\xc4" "\xa3" "\xe9" "\xa3" "\xe7" "\xa3" "\xe9" "\xa3" "\xf4" "\xa3" "\xe1" "\xa3" "\xec" " .%s";
const char D_0053AFD8[24] = "\xa3" "\xcc" "\xa3" "\xd2" "\xa1" "\xa7" "\xa3" "\xd6" "\xa3" "\xcf" "\xa3" "\xcc" " .%d";

/* Loads the movie list file `name` into `buffer` and splits it into text lines. The table at 0x00980000
 * holds one TextLine pointer per slot (256 slots); the text of slot n lives in 128-byte rows from 0x00980400.
 * CR LF ends a line, a bare LF is dropped, a '.' ends the visible text of its line. Returns the line count,
 * or -1 when the read fails. */
int movieListLoadLines(char *fileBuffer, const char *name) {
    int fileSize;
    int lineCount;
    int column;
    int lineIndex;
    char *lineText;
    int i;

    fileSize = xglCdGetFileSize(name);
    memset(fileBuffer, 0, fileSize);
    if (xglCdReadFile(name, fileBuffer, 0, 0) <= 0) {
        return -1;
    }
    memset((void *) 0x00980000, 0, 0x100000);
    movieListLines = (struct TextLine *) 0x00980000;
    lineText = (char *) 0x00980400;
    for (i = 0; i < 256; i++) {
        movieListLines[i].text = lineText;
        lineText += 128;
    }
    lineCount = 0;
    column = 0;
    lineIndex = 0;
    for (i = 0; i < fileSize; i++) {
        if (fileBuffer[i] == '\r' && fileBuffer[i + 1] == '\n') {
            movieListLines[lineIndex].text[column] = 0;
            lineIndex++;
            lineCount++;
            column = 0;
        } else if (fileBuffer[i] != '\n') {
            movieListLines[lineIndex].text[column] = fileBuffer[i];
            if (movieListLines[lineIndex].text[column] == '.') {
                movieListLines[lineIndex].text[column] = 0;
            }
            column++;
        }
    }
    movieListLines[lineIndex].text[column] = 0;
    return lineCount;
}

extern int movieListCount;
extern int movieListCursor;
extern int movieVolume;
extern unsigned char gXglMcAgedEntries[];
extern PadRecord PadData;
extern char movieDolbyOnText[];
extern char movieDolbyOffText[];
int sprintf(char *destination, const char *format, ...);
void xglFontPrint(int x, int y, int color, const char *text);
void xglFontPrintDirect(const char *text);

/* Movie list menu, one frame: the pad moves the cursor line (movieListCursor) and pages through the list,
 * the pressed bit 0x80 toggles gXglMcAgedEntries[41] (the ON/OFF row), the repeated bits 0x4 and 0x8 move
 * the volume (movieVolume, clamped to 0..127), then the page, the cursor line and the two value rows are drawn. */
void movieListMenuFrame(void) {
    int pageIndex;
    int pageFirst;
    int pageEnd;
    int i;
    int volumeCopy;
    int nextLineIndex;
    int previousPageFirst;
    int nextPageFirst;
    int volume;
    int repeatedButtons;

    pageIndex = movieListCursor / 15;
    pageFirst = pageIndex * 15;
    pageEnd = pageFirst + 15;

    if (PadData.directionRepeated & 0x1000) {
        if (movieListCursor - 1 < pageFirst) {
            movieListCursor = pageFirst + 14;
        } else {
            movieListCursor = movieListCursor - 1;
        }
        if (movieListCursor >= movieListCount) {
            movieListCursor = movieListCount - 1;
        }
    }
    if (PadData.directionRepeated & 0x4000) {
        nextLineIndex = movieListCursor + 1;
        if (nextLineIndex >= pageEnd) {
            nextLineIndex = pageFirst;
        }
        if (nextLineIndex >= movieListCount) {
            nextLineIndex = pageFirst;
        }
        movieListCursor = nextLineIndex;
    }
    if (PadData.directionRepeated & 0x8000) {
        previousPageFirst = movieListCursor - movieListCursor % 15 - 15;
        if (previousPageFirst < 0) {
            movieListCursor = movieListCount - 1;
        } else {
            movieListCursor = previousPageFirst;
        }
    }
    if (PadData.directionRepeated & 0x2000) {
        nextPageFirst = movieListCursor - movieListCursor % 15 + 15;
        if (nextPageFirst >= movieListCount) {
            nextPageFirst = 0;
        }
        movieListCursor = nextPageFirst;
    }

    sprintf(&SEF_SCRATCHPAD(char, 0x0), movieListPageFormat, pageIndex + 1, (movieListCount - 1) / 15 + 1);
    xglFontPrint(10, 5, 0xFFFFFF, movieListTextColourGrey);
    xglFontPrintDirect(&SEF_SCRATCHPAD(char, 0x0));

    for (i = pageFirst; i < pageEnd && i < movieListCount; i++) {
        if (i == movieListCursor) {
            xglFontPrint(10, (i - pageIndex * 15) * 20 + 30, 0xFFFFFF, movieListTextColourGreen);
        } else {
            xglFontPrint(10, (i - pageIndex * 15) * 20 + 30, 0xFFFFFF, movieListTextColourGrey);
        }
        xglFontPrintDirect(movieListLines[i].text);
    }

    if (PadData.pressed & 0x80) {
        gXglMcAgedEntries[41] ^= 1;
    }
    repeatedButtons = PadData.repeated;
    if (repeatedButtons & 0xC) {
        if (repeatedButtons & 0x4) {
            volume = movieVolume;
            volume--;
        } else {
            volume = movieVolume;
        }
        if (repeatedButtons & 0x8) {
            volumeCopy = volume + 1;
            volume = volumeCopy;
        }
        if (volume < 0) {
            volume = 0;
        }
        volumeCopy = volume;
        if (volumeCopy >= 128) {
            volume = 127;
            movieVolume = volume;
        } else {
            movieVolume = volumeCopy;
        }
    }

    sprintf(&SEF_SCRATCHPAD(char, 0x0), D_0053AFB0, gXglMcAgedEntries[41] ? movieDolbyOnText : movieDolbyOffText);
    xglFontPrint(10, 360, 0xFFFFFF, movieListTextColourGreen);
    xglFontPrintDirect(&SEF_SCRATCHPAD(char, 0x0));
    sprintf(&SEF_SCRATCHPAD(char, 0x0), D_0053AFD8, movieVolume);
    xglFontPrint(300, 360, 0xFFFFFF, movieListTextColourGreen);
    xglFontPrintDirect(&SEF_SCRATCHPAD(char, 0x0));
}

typedef int s32;

extern s32 movieListCount;
extern s32 movieListCursor;
extern u8 D_004B377F[];
extern PadRecord PadData;
s32 func_0018F7E8(s32 size);
s32 func_001B6520(s32 value);
s32 func_001B6798(s32 value);
s32 func_001B6840(void);
s32 func_001B3858(s32 value);
s32 func_001B64D8(s32 a, s32 b);
void movieListMenuFrame(void);
void soundWorkSource(s32 a, s32 b, s32 c);
void xglSleep(void);
void xglRenderClearFrame(void);
void xglRenderLockClear(void);

/* Movie list screen: loads mov/sfd.lst into a buffer from func_001B6520 and shows it through movieListMenuFrame until
 * the held-and-pressed combination 0x08000100 is seen. The pressed bit 0x20 passes the selected line's text to func_001B3858. */
void movieListScreen(void) {
    s32 listText;
    s32 lineCount;
    s32 soundSlot;
    PadRecord pad;

    xglSleep();
    func_001B64D8(0xE00000, 0xFEE50000);
    func_001B6840();
    listText = func_001B6520(func_001B6840());
    lineCount = movieListLoadLines((char *) listText, (const char *) "mov/sfd.lst");
    movieListCount = lineCount;
    if (lineCount > 0) {
        func_001B6798(listText);
        soundSlot = func_0018F7E8(8);
        soundWorkSource(0, func_001B6520(soundSlot), soundSlot);
        D_004B377F[0] = 1;
        if ((PadData.heldAndPressed & 0x08000100) != 0x08000100) {
            do {
                if (PadData.pressed & 0x20) {
                    func_001B3858((s32) movieListLines[movieListCursor].text);
                }
                movieListMenuFrame();
                xglSleep();
            } while ((PadData.heldAndPressed & 0x08000100) != 0x08000100);
        }
        xglRenderClearFrame();
        xglRenderLockClear();
    }
}

struct TextLine *movieListLines = 0;
s32 movieListCount = 0;
s32 movieListCursor = 0;
s32 movieVolume = 127;
char movieDolbyOnText[] = "ON";
char movieDolbyOffText[] = "OFF";

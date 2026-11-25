#include "common.h"

typedef signed char s8;
typedef int s32;
typedef unsigned char u8;
typedef short s16;

/* Movie request block at D_005A7E50: text window at 0x10, requested file name at 0x11C. */
typedef struct MovieTextRequest {
    s32 unk_00;
    s32 textX;
    s32 textY;
    s32 fileRequestPending;           /* 0x0C: set to 1 while a file request is pending */
    char message[0x100];        /* 0x10 */
    s32 unk_110;
    s32 unk_114;
    void *printFunction;        /* 0x118: xglFontDebugPrintf */
    char fileName[0x102];       /* 0x11C */
    s16 unk_21E;
    s16 unk_220;
} MovieTextRequest;

typedef struct MovieSlot {
    u8 unk_00[0x2C];
} MovieSlot;

#define gMovieTextRequest (*(MovieTextRequest *) D_005A7E50)

/* The file name block is the fileName field of the request block (0x11C in). */
#define offsetof(type, field) ((s32) &((type *) 0)->field)
#define CONTAINER_OF(member, type, field) ((type *) ((void *) (member) - offsetof(type, field)))
typedef struct MovieRequestFileName {
    char fileName[0x102];
} MovieRequestFileName;
#define gMovieFileName (*(MovieRequestFileName *) D_005A7F6C)

extern unsigned char D_005A7E50[];
s32 strcpy();
extern unsigned char xglFontDebugPrintf[];

void movieTextWindowInitAlt(const char *text)
{
  gMovieTextRequest.textX = 0x14;
  gMovieTextRequest.textY = 0x14;
  gMovieTextRequest.unk_00 = 8;
  gMovieTextRequest.fileRequestPending = 0;
  strcpy(gMovieTextRequest.message, text);
  gMovieTextRequest.printFunction = xglFontDebugPrintf;
  gMovieTextRequest.unk_110 = 0x027B0000;
  gMovieTextRequest.unk_114 = 0x100000;
  gMovieTextRequest.fileName[0] = 0;
}

extern unsigned char D_005A7F6C[];
s32 xglRenderIoBegin();
s32 xglRenderIoEnd();
s32 xglSleep();

s32 movieFileRequestAlt(const char *fileName)
{
  MovieTextRequest *fileRequest;

  strcpy(gMovieFileName.fileName, fileName);
  fileRequest = CONTAINER_OF(&gMovieFileName, MovieTextRequest, fileName);
  fileRequest->unk_220 = fileRequest->unk_21E = 0;
  fileRequest->fileRequestPending = 1;
  xglRenderIoBegin();
  xglSleep();
  xglRenderIoEnd();
  xglSleep();
  return -1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu202", func_00248580);

INCLUDE_ASM("asm/main/nonmatchings/tu202", func_00248650);

INCLUDE_ASM("asm/main/nonmatchings/tu202", func_00248728);

INCLUDE_ASM("asm/main/nonmatchings/tu202", func_002487D0);

INCLUDE_ASM("asm/main/nonmatchings/tu202", func_00248880);

extern unsigned char D_003EBA80[];
extern unsigned char D_0053AEC0[];
extern s32 D_00550D5C;
extern char *D_00550D68;
extern unsigned char D_00550D70[];
s32 sprintf();
s32 func_00248060();
void movieTextWindowInitAlt(const char *text);
s32 movieFileRequestAlt(const char *fileName);
s32 strcmp();
s32 memset();

void movieDecodeToFixedAlt(void) {
    u8 formattedMessage[0x100];

    memset(formattedMessage, 0, 0x100);
    movieTextWindowInitAlt(D_00550D68);
    if ((movieFileRequestAlt(D_003EBA80) == 1) && (strcmp(D_005A7F6C, D_00550D70) != 0)) {
        sprintf(formattedMessage, D_0053AEC0, D_00550D68, D_005A7F6C);
        func_00248060(&D_00550D5C, formattedMessage);
    }
}

extern s32 D_00550D58;
extern MovieSlot D_005A7D10[];
s32 func_002483C8();

void movieDecodeToSlotAlt(void) {
    u8 formattedMessage[0x100];

    memset(formattedMessage, 0, 0x100);
    movieTextWindowInitAlt(D_00550D68);
    if ((movieFileRequestAlt(D_003EBA80) == 1) && (strcmp(D_005A7F6C, D_00550D70) != 0)) {
        sprintf(formattedMessage, D_0053AEC0, D_00550D68, D_005A7F6C);
        func_002483C8(&D_005A7D10[D_00550D58], formattedMessage);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu202", func_00248AF0);

INCLUDE_ASM("asm/main/nonmatchings/tu202", func_00248B48);

INCLUDE_ASM("asm/main/nonmatchings/tu202", func_00248C80);

INCLUDE_ASM("asm/main/nonmatchings/tu202", func_00248E00);

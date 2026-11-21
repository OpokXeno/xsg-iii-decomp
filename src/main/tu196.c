#include "common.h"

typedef signed char s8;
typedef int s32;
typedef unsigned char u8;
typedef short s16;

/* Movie request block at gMovieRequestStorage: text window at 0x10, requested file name at 0x11C. */
typedef struct MovieRequestState {
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
} MovieRequestState;

typedef struct MovieSlot {
    u8 unk_00[0x2C];
} MovieSlot;

#define gMovieRequest (*(MovieRequestState *) gMovieRequestStorage)

/* The file name block is the fileName field of the request block (0x11C in). */
#define offsetof(type, field) ((s32) &((type *) 0)->field)
#define CONTAINER_OF(member, type, field) ((type *) ((void *) (member) - offsetof(type, field)))
typedef struct MovieFileNameBlock {
    char fileName[0x102];
} MovieFileNameBlock;
#define gMovieFileName (*(MovieFileNameBlock *) gMovieFileNameStorage)

extern unsigned char gMovieRequestStorage[];
s32 strcpy();
extern unsigned char xglFontDebugPrintf[];

void movieTextWindowInit(const char *text)
{
  gMovieRequest.textX = 0x14;
  gMovieRequest.textY = 0x14;
  gMovieRequest.unk_00 = 8;
  gMovieRequest.fileRequestPending = 0;
  strcpy(gMovieRequest.message, text);
  gMovieRequest.printFunction = xglFontDebugPrintf;
  gMovieRequest.unk_110 = 0x027B0000;
  gMovieRequest.unk_114 = 0x100000;
  gMovieRequest.fileName[0] = 0;
}

extern unsigned char gMovieFileNameStorage[];
s32 xglRenderIoBegin();
s32 xglRenderIoEnd();
s32 xglSleep();

s32 movieFileRequest(const char *fileName)
{
  MovieRequestState *fileRequest;

  strcpy(gMovieFileName.fileName, fileName);
  fileRequest = CONTAINER_OF(&gMovieFileName, MovieRequestState, fileName);
  fileRequest->unk_220 = fileRequest->unk_21E = 0;
  fileRequest->fileRequestPending = 1;
  xglRenderIoBegin();
  xglSleep();
  xglRenderIoEnd();
  xglSleep();
  return -1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu196", func_002425D8);

INCLUDE_ASM("asm/main/nonmatchings/tu196", func_002426A8);

INCLUDE_ASM("asm/main/nonmatchings/tu196", func_00242780);

INCLUDE_ASM("asm/main/nonmatchings/tu196", func_00242828);

INCLUDE_ASM("asm/main/nonmatchings/tu196", func_002428D8);

extern unsigned char gMovieSourceFileName[];
extern unsigned char gMovieMessageFormat[];
extern s32 D_00550D2C;
extern char *gMovieMessageText;
extern unsigned char D_00550D40[];
s32 GameMpeg2Play();
s32 func_00242080();
void movieTextWindowInit(const char *text);
s32 movieFileRequest(const char *fileName);
s32 func_0030E68C();
s32 memset();

void movieDecodeToFixed(void) {
    u8 formattedMessage[0x100];

    memset(formattedMessage, 0, 0x100);
    movieTextWindowInit(gMovieMessageText);
    if ((movieFileRequest(gMovieSourceFileName) == 1) && (func_0030E68C(gMovieFileNameStorage, D_00550D40) != 0)) {
        GameMpeg2Play(formattedMessage, gMovieMessageFormat, gMovieMessageText, gMovieFileNameStorage);
        func_00242080(&D_00550D2C, formattedMessage);
    }
}

extern s32 gMovieSlotIndex;
extern MovieSlot gMovieSlotEntries[];
s32 func_00242420();

void movieDecodeToSlot(void) {
    u8 formattedMessage[0x100];

    memset(formattedMessage, 0, 0x100);
    movieTextWindowInit(gMovieMessageText);
    if ((movieFileRequest(gMovieSourceFileName) == 1) && (func_0030E68C(gMovieFileNameStorage, D_00550D40) != 0)) {
        GameMpeg2Play(formattedMessage, gMovieMessageFormat, gMovieMessageText, gMovieFileNameStorage);
        func_00242420(&gMovieSlotEntries[gMovieSlotIndex], formattedMessage);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu196", func_00242B68);

INCLUDE_ASM("asm/main/nonmatchings/tu196", func_00242BC0);

INCLUDE_ASM("asm/main/nonmatchings/tu196", func_00242CF8);

INCLUDE_ASM("asm/main/nonmatchings/tu196", func_00242E78);

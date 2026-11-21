#include "common.h"

typedef int s32;
void *memPoolAlloc(s32);
void memPoolFree(void *);
void *memset(void *s, int c, unsigned int n);
extern unsigned char gJpegImageSizeStorage[];
extern unsigned char gJpegDecodeStateStorage[];

/* Image size at gJpegImageSizeStorage: width, then height. */
typedef struct JpegImageSize {
    s32 width;
    s32 height;
} JpegImageSize;

/* Decode state at gJpegDecodeStateStorage: the two image buffers, the decode status and a result word. */
typedef struct JpegDecodeState {
    void *wideBuffer;   /* 4 bytes per pixel */
    void *pixelBuffer;  /* 1 byte per pixel */
    s32 decodeStage;         /* 2 once decoded */
    s32 pollResult;
} JpegDecodeState;

#define gJpegImageSize (*(JpegImageSize *) gJpegImageSizeStorage)
#define gJpegDecodeState (*(JpegDecodeState *) gJpegDecodeStateStorage)

/* Allocates the two decode buffers for the image size, each rounded up to 0x800 bytes. */
void jpegBuffersAlloc(void)
{
    JpegImageSize *imageSize = &gJpegImageSize;
    JpegDecodeState *decodeState = &gJpegDecodeState;
    s32 pixelCount;
    s32 wideBufferBytes;
    s32 height;
    s32 biased;
    s32 rounded;

    pixelCount = imageSize->width * imageSize->height;
    biased = pixelCount + 0x7FF;
    rounded = biased;
    if (biased < 0) {
        rounded = pixelCount + 0xFFE;
    }
    decodeState->pixelBuffer = memPoolAlloc((rounded >> 0xB) << 0xB);
    height = imageSize->height;
    wideBufferBytes = (imageSize->width * height) * 4;
    biased = wideBufferBytes + 0x7FF;
    rounded = biased;
    if (biased < 0) {
        rounded = wideBufferBytes + 0xFFE;
    }
    decodeState->wideBuffer = memPoolAlloc((rounded >> 0xB) << 0xB);
}

INCLUDE_ASM("asm/main/nonmatchings/tu381", func_002C8A50);

extern s32 gJpegDecodeStage;

s32 jpegDecodeIsDone(void) {
    return gJpegDecodeStage == 2;
}

s32 func_001B60B8(s32 mode);
s32 xglJpegDecode(void **request);
/* Decodes the image once func_001B60B8 reports 1: the decoder gets the pixel buffer and the wide buffer. */
void jpegDecodeOnce(void)
{
    JpegDecodeState *decodeState;
    void *decodeArgs[8];
    s32 pollResult;

    pollResult = func_001B60B8(0);
    decodeState = &gJpegDecodeState;
    decodeState->pollResult = pollResult;
    if (pollResult == 1) {
        memset(&decodeArgs[0], 0, 0x20);
        decodeArgs[0] = decodeState->pixelBuffer;
        decodeArgs[1] = decodeState->wideBuffer;
        xglJpegDecode(&decodeArgs[0]);
        decodeState->decodeStage = 2;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu381", func_002C8BF0);

/* Frees the two decode buffers. */
void jpegBuffersFree(void) {
    JpegDecodeState *decodeState = &gJpegDecodeState;

    memPoolFree(decodeState->pixelBuffer);
    memPoolFree(decodeState->wideBuffer);
}

s32 func_002CF448(void *buffer, s32 first, s32 second);

void jpegSetDefaultMode(void) {
    func_002CF448(gJpegDecodeState.wideBuffer, 0xD3, 0x86);
}

void jpegSetImageSize(void) {
    func_002CF448(gJpegDecodeState.wideBuffer,
                  gJpegImageSize.width,
                  gJpegImageSize.height);
}

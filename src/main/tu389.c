#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu389", func_002CF098);

typedef int s32;
s32 func_001B5F28();
s32 func_001B6520();
s32 func_001B6798(s32 bitstreamBuffer);
void xcBlockClear(void *block);
s32 func_001B70F0(s32 *jpegDecoder);
void *memset();
s32 xglJpegDecode(s32 *params);
typedef struct JpegDecodeRequest {
    s32 unk_00;
    s32 decoderSource;
    s32 imageDestination;
    s32 width;
    s32 height;
} JpegDecodeRequest;

void xglJpegDecodeImage(JpegDecodeRequest *jpegRequest) {
    s32 jpegDecoder[8];
    s32 jpeg_params[8];
    s32 bitstreamBuffer;

    bitstreamBuffer = func_001B6520(jpegRequest->width * jpegRequest->height);
    xcBlockClear(&jpegDecoder[0]);
    jpegDecoder[2] = bitstreamBuffer;
    jpegDecoder[1] = jpegRequest->decoderSource;
    func_001B70F0(&jpegDecoder[0]);
    func_001B5F28(0, 0, 0);
    memset(&jpeg_params[0], 0, 0x20);
    jpeg_params[0] = bitstreamBuffer;
    jpeg_params[1] = jpegRequest->imageDestination;
    xglJpegDecode(&jpeg_params[0]);
    func_001B6798(bitstreamBuffer);
}

INCLUDE_ASM("asm/main/nonmatchings/tu389", func_002CF1A0);

INCLUDE_ASM("asm/main/nonmatchings/tu389", func_002CF448);

INCLUDE_ASM("asm/main/nonmatchings/tu389", func_002CF518);

INCLUDE_ASM("asm/main/nonmatchings/tu389", func_002CF728);

INCLUDE_ASM("asm/main/nonmatchings/tu389", func_002CF8A8);

#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/xc_decode", func_001B6BD8);

INCLUDE_ASM("asm/main/nonmatchings/xc_decode", func_001B6C28);

INCLUDE_ASM("asm/main/nonmatchings/xc_decode", func_001B6CA8);

INCLUDE_ASM("asm/main/nonmatchings/xc_decode", func_001B6DA0);

INCLUDE_ASM("asm/main/nonmatchings/xc_decode", func_001B6E18);

INCLUDE_ASM("asm/main/nonmatchings/xc_decode", func_001B6EB8);

INCLUDE_ASM("asm/main/nonmatchings/xc_decode", func_001B6F18);

INCLUDE_ASM("asm/main/nonmatchings/xc_decode", func_001B6FF8);

typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
s32 memset();

#define XC_HEADER_MAGIC 0x6358

/* The header of one XC block: the magic word, a version word that must be 1, and a flag at +0x0C that selects the long payload at +0x40. */
typedef struct XcBlockEnvelope {
    u16 magic;                          /* +0x00: XC_HEADER_MAGIC */
    u16 version;                        /* +0x02 */
    unsigned char unmodeled_04[8];
    s16 useLongForm;                  /* +0x0C: nonzero selects longFormPayload */
    unsigned char unmodeled_0E[2];
    unsigned char shortFormPayload[0x30];        /* +0x10: the short payload */
    unsigned char longFormPayload[0];       /* +0x40: the long payload */
} XcBlockEnvelope;

/* A stream copy request: source and destination pointers, the byte count, and the copied count written back at +0x10. */
typedef struct XcStream {
    u8 *sourceAddress;                            /* +0x00 */
    s32 copyLength;                          /* +0x04 */
    u8 *destinationAddress;                            /* +0x08 */
    unsigned char unmodeled_0C[4];
    s32 copied;                         /* +0x10 */
} XcStream;

/* An XC object: a handle at +0x1C, a size argument at +0x18, and the completion callback at +0x20. */
typedef struct XcPoolAllocation {
    unsigned char unmodeled_00[0x18];
    s32 allocationSize;                           /* +0x18 */
    s32 poolBlockId;                         /* +0x1C */
    s32 (*completionCallback)();                  /* +0x20: 0 when none is set */
} XcPoolAllocation;

void xcBlockClear(void *clearedBlock) {
    memset(clearedBlock, 0, 0x18);
}

INCLUDE_ASM("asm/main/nonmatchings/xc_decode", func_001B70F0);

INCLUDE_ASM("asm/main/nonmatchings/xc_decode", func_001B7190);

extern unsigned char gXcMpeg2PackedInput[];
extern unsigned char gXcMpeg2OutputBuffer[];
s32 sprintf(void *, void *, u32, s32);
u32 func_00181C18(s32);

void *xcMpeg2PlayPacked(void) {
    u32 packed;

    packed = func_00181C18(0);
    sprintf(gXcMpeg2OutputBuffer, gXcMpeg2PackedInput, packed, (packed >> 8) & 0xFFFF);
    return gXcMpeg2OutputBuffer;
}

void xcRunCompletionCallback(XcPoolAllocation *allocation) {
    s32 (*completionCallback)();

    completionCallback = allocation->completionCallback;
    if (completionCallback != 0) {
        completionCallback();
    }
}

s32 poolBlockTagWordSet(s32, s32, s32);

void xcObjectReleaseNotify(XcPoolAllocation *allocation) {
    s32 (*completionCallback)();

    poolBlockTagWordSet(allocation->poolBlockId, 0, allocation->allocationSize);
    completionCallback = allocation->completionCallback;
    if (completionCallback != 0) {
        completionCallback(allocation);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/xc_decode", func_001B7330);

s32 xcStreamCopy(XcStream *stream) {
    s32 copyLength;
    s32 bytesRemaining;
    u8 *destinationCursor;
    u8 *sourceCursor;
    s32 copiedByte;

    copyLength = stream->copyLength;
    sourceCursor = stream->sourceAddress;
    destinationCursor = stream->destinationAddress;
    bytesRemaining = copyLength - 1;
    if (copyLength != 0) {
        do {
            copiedByte = *sourceCursor;
            sourceCursor += 1;
            bytesRemaining -= 1;
            *destinationCursor = copiedByte;
            destinationCursor += 1;
        } while (bytesRemaining != -1);
    }
    stream->copied = stream->copyLength;
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/xc_decode", func_001B7480);

s32 func_001B7480();

void xcRunDecoder(void) {
    func_001B7480();
}

INCLUDE_ASM("asm/main/nonmatchings/xc_decode", func_001B7558);

INCLUDE_ASM("asm/main/nonmatchings/xc_decode", func_001B7620);

extern const char xcAssertFileName[];
extern const char xcAssertMessage[];
void assertFailed(const char *message, const char *file, int line);

s32 xcLocatePayload(void **selectedPayload, XcBlockEnvelope *envelope)
{
  if (envelope == 0)
  {
    assertFailed(xcAssertMessage, xcAssertFileName, 0x17F);
  }
  if (envelope->magic != XC_HEADER_MAGIC)
  {
    *selectedPayload = 0;
    return 0;
  }
  *selectedPayload = (envelope->useLongForm != 0) ? envelope->longFormPayload : envelope->shortFormPayload;
  return envelope->version == 1;
}

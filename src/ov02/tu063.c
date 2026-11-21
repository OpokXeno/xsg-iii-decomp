/*
 * OV02 original TU 63: 0x00a9acf0..0x00a9ba10 (14 functions)
 */
#include "common.h"

typedef signed char s8;
typedef int s32;
typedef float f32;
typedef unsigned char u8;
typedef unsigned int u32;
struct XglCamera *xglStudioGetCamera2(s32);

/* The part of the camera record xglCameraGetDefaultScale reads: the float at +0x5F4 (XglCamera.defaultScale in include/main/xgl_2.h). */
typedef struct CameraTail {
    unsigned char unmodeled_000[0x5F4];
    f32 defaultScale;                    /* +0x5F4 */
} CameraTail;

f32 xglCameraGetDefaultScale(void) {
    return ((CameraTail *) xglStudioGetCamera2(0))->defaultScale;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu063", func_00A9AD10);

extern unsigned char D_00AB91D0[];
s32 GameMpeg2Play();
s32 func_0030ED88();

s32 formatCopyName(s32 nameBuffer, s32 nameBufferSize) {
    s32 formattedName[0x80];

    GameMpeg2Play(&formattedName[0], D_00AB91D0);
    func_0030ED88(nameBuffer, &formattedName[0], nameBufferSize);
    return nameBuffer;
}

extern const char gPathCopyAssertMessage[];
extern const char gPathCopyAssertFile[];
void assertFailed(const char *message, const char *file, int line);
s8 *func_0030EF48(const s8 *, s32);

/* Copies the directory part of the path src (everything before its last '\\' or '/') into dst. */
s8 *pathDirectoryCopy(s8 *dirBuffer, s32 dirBufferSize, const s8 *pathSource) {
    s32 dirLength;
    s32 maxLength;
    s8 *lastBackslash;
    s8 *lastSlash;
    s8 *lastSeparator;

    lastBackslash = func_0030EF48(pathSource, 0x5C);
    lastSlash = func_0030EF48(pathSource, 0x2F);
    lastSeparator = lastBackslash;
    if (lastBackslash < lastSlash) {
        lastSeparator = lastSlash;
    }
    if (dirBufferSize <= 0) {
        assertFailed(gPathCopyAssertMessage, gPathCopyAssertFile, 0xA4);
    }
    *dirBuffer = 0;
    if (lastSeparator != 0) {
        dirLength = lastSeparator - pathSource;
        maxLength = dirBufferSize - 1;
        func_0030ED88(dirBuffer, pathSource, (maxLength >= dirLength) ? dirLength : maxLength);
    } else {
        func_0030ED88(dirBuffer, pathSource, dirBufferSize);
    }
    return dirBuffer;
}

/* Copies the file name part of the path src (everything after its last '\\' or '/') into dst. */
s8 *pathFileNameCopy(s8 *nameBuffer, s32 nameBufferSize, const s8 *pathSource) {
    s8 *lastBackslash;
    s8 *lastSlash;
    s8 *lastSeparator;

    lastBackslash = func_0030EF48(pathSource, 0x5C);
    lastSlash = func_0030EF48(pathSource, 0x2F);
    lastSeparator = lastBackslash;
    if (lastBackslash < lastSlash) {
        lastSeparator = lastSlash;
    }
    if (nameBufferSize <= 0) {
        assertFailed(gPathCopyAssertMessage, gPathCopyAssertFile, 0xBA);
    }
    *nameBuffer = 0;
    if (lastSeparator != 0) {
        func_0030ED88(nameBuffer, lastSeparator + 1, nameBufferSize);
    } else {
        func_0030ED88(nameBuffer, pathSource, nameBufferSize);
    }
    return nameBuffer;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu063", func_00A9B088);

INCLUDE_ASM("asm/nonmatchings/ov02/tu063", func_00A9B140);

INCLUDE_ASM("asm/nonmatchings/ov02/tu063", func_00A9B1B8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu063", func_00A9B230);

INCLUDE_ASM("asm/nonmatchings/ov02/tu063", func_00A9B2B8);

void *memset(void *s, int c, unsigned int n);

void vectorRowsZero(void *vectorRows) {
    memset(vectorRows, 0, 0x30);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu063", func_00A9B508);

INCLUDE_ASM("asm/nonmatchings/ov02/tu063", func_00A9B5A8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu063", func_00A9B8D8);

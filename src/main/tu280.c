#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu280", func_0026B580);

typedef int s32;
typedef float f32;
s32 func_0026A6D0(void *, s32, s32, s32, void *);
s32 textFindCloseAngle(s32);

s32 vec4DivideByScale(f32 *vec, s32 mode, s32 textCursor, f32 *scale) {
    s32 hasTextRect;
    s32 tagEnd;

    tagEnd = textFindCloseAngle(textCursor);
    hasTextRect = func_0026A6D0(vec, mode, textCursor, tagEnd, scale);
    if (hasTextRect != 0) {
        hasTextRect = 1;
        vec[0] = vec[0] / scale[4];
        vec[1] = vec[1] / scale[5];
        vec[2] = vec[2] / scale[4];
        vec[3] = vec[3] / scale[5];
    }
    return hasTextRect;
}

INCLUDE_ASM("asm/main/nonmatchings/tu280", func_0026B728);

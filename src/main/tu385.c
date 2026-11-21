#include "common.h"

typedef float f32;

/* Voice header block: the position pair sits at 0xF0 (x) and 0xF4 (y). */
typedef struct SeVoiceHeader {
    unsigned char unk_00[0xF0];
    f32 x;                      /* 0xF0 */
    f32 y;                      /* 0xF4 */
} SeVoiceHeader;

void seVoiceHeaderSetPosition(SeVoiceHeader *voiceHeader, f32 x, f32 y) {
    voiceHeader->x = x;
    voiceHeader->y = y;
}

INCLUDE_ASM("asm/main/nonmatchings/tu385", func_002C9138);

INCLUDE_ASM("asm/main/nonmatchings/tu385", func_002C91B8);

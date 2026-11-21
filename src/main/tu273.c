#include "common.h"

typedef unsigned char u8;
typedef signed char s8;
typedef float f32;

/* Object record: type byte at +0x02, scale at +0x0C. */
typedef struct TuObject {
    u8 unk_00[0x02];
    s8 type;
    u8 unk_03[0x0C - 0x03];
    f32 scale;
} TuObject;

void objectSetType(TuObject *object, s8 type) {
    object->type = type;
}

void objectResetScale(TuObject *object) {
    object->scale = 1.0f;
}

INCLUDE_ASM("asm/main/nonmatchings/tu273", func_00269150);

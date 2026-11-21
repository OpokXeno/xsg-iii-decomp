#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu373", func_002C1C70);

INCLUDE_ASM("asm/main/nonmatchings/tu373", func_002C1E08);

INCLUDE_ASM("asm/main/nonmatchings/tu373", func_002C1E28);

INCLUDE_ASM("asm/main/nonmatchings/tu373", func_002C1ED0);

typedef int s32;
typedef unsigned char u8;

/* Fields of the block at D_005EA640 that stateTailReset clears; offsets are relative to the block start. */
typedef struct Tu373State {
    u8 unk_000[0x51C];
    s32 field_51C; /* 0x51C */
    s32 field_520; /* 0x520 */
    u8 unk_524[0x538 - 0x524];
    s32 field_538; /* 0x538 */
    u8 field_53C[6]; /* 0x53C */
} Tu373State;

extern Tu373State D_005EA640;
void *memset(void *, int, unsigned int);

void stateTailReset(void)
{
    D_005EA640.field_538 = 0;
    memset(D_005EA640.field_53C, 0, 6);
    D_005EA640.field_520 = 0;
    D_005EA640.field_51C = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu373", func_002C20A0);

INCLUDE_ASM("asm/main/nonmatchings/tu373", func_002C2630);

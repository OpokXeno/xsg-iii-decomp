#include "common.h"

void func_001B6798(int handle);

typedef struct NarrEvSlot {
    int handle;                       /* +0 */
    unsigned char unmodeled_04[32];   /* 4..35 */
    int handle_b;                     /* +36 */
    unsigned char unmodeled_28[32];   /* 40..71 */
    int flag;                         /* +72 */
    unsigned char unmodeled_4C[68];   /* 76..143 */
} NarrEvSlot;

typedef struct NarrEvState {
    unsigned char unmodeled_00[16];   /* 0..15 */
    NarrEvSlot slots[4];              /* 16..591 */
} NarrEvState;

void func_0024A720(NarrEvState *state) {
    int i;

    for (i = 0; i < 4; i++) {
        NarrEvSlot *slot = &state->slots[i];
        if (slot->handle != 0) {
            func_001B6798(slot->handle);
            slot->handle = 0;
            func_001B6798(slot->handle_b);
            slot->handle_b = 0;
            slot->flag = 0;
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024A780);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024A7F8);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024A868);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024A8F0);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024A9A8);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024AA30);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024AA98);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024AAF8);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024AB18);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024ABC8);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024AC68);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024AD18);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024AD60);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024AF50);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024B080);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024B540);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024BA78);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024BF28);

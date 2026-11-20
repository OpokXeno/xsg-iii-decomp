#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu267", func_00267CA0);

INCLUDE_ASM("asm/main/nonmatchings/tu267", func_00267CE8);

INCLUDE_ASM("asm/main/nonmatchings/tu267", func_00267D50);

INCLUDE_ASM("asm/main/nonmatchings/tu267", func_00267D78);

INCLUDE_ASM("asm/main/nonmatchings/tu267", func_00267ED0);

struct TextTag {
    int key;
    unsigned char unmodeled_04[0x1c];
};

extern struct TextTag D_0055AFA0[20];
void *memset(void *destination, int value, unsigned int size);

/* Clears every table entry whose key equals key. */
void func_00267F40(int key)
{
    int index;

    for (index = 0; index < 20; index++) {
        if (D_0055AFA0[index].key == key) {
            memset(&D_0055AFA0[index], 0, sizeof(struct TextTag));
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu267", func_00267FA8);

INCLUDE_ASM("asm/main/nonmatchings/tu267", func_00267FE0);

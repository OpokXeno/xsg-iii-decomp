#include "common.h"

extern void *memset(void *destination, int value, unsigned int count);
extern struct CdReadRecord D_00552118[3][64];
extern unsigned char D_00557818[];
extern int D_00557828[3];
extern unsigned char D_00557838[];

void func_001B5DC0(void) {
    memset(D_00552118, 0, 22272);
    memset(D_00557818, 0, 12);
    memset(D_00557828, 0, 12);
    memset(D_00557838, 0, 12);
}

INCLUDE_ASM("asm/main/nonmatchings/tu054", func_001B5E20);

INCLUDE_ASM("asm/main/nonmatchings/tu054", func_001B5E40);

INCLUDE_ASM("asm/main/nonmatchings/tu054", func_001B5EB0);

INCLUDE_ASM("asm/main/nonmatchings/tu054", func_001B5F28);

INCLUDE_ASM("asm/main/nonmatchings/tu054", func_001B5FD0);

INCLUDE_ASM("asm/main/nonmatchings/tu054", func_001B6000);

INCLUDE_ASM("asm/main/nonmatchings/tu054", func_001B6018);

INCLUDE_ASM("asm/main/nonmatchings/tu054", func_001B60B8);

INCLUDE_ASM("asm/main/nonmatchings/tu054", func_001B6118);

INCLUDE_ASM("asm/main/nonmatchings/tu054", func_001B6228);

INCLUDE_ASM("asm/main/nonmatchings/tu054", func_001B6320);

extern int func_001B60B8(int channel);
extern void func_001B6418(int event, int value, int tag);
extern void xglCdSetReadTag(int value);
extern int xglCdReadFile(const char *name, void *buffer, int mode, void (*notify)(int, int, int));

struct CdReadRecord {
    unsigned char unmodeled_00[8];
    void *buffer;
    unsigned char unmodeled_0c[24];
    char name[80];
};

int func_001B6360(int channel)
{
    int *head;
    struct CdReadRecord *record;

    if (func_001B60B8(channel) == 0) {
        return 0;
    }
    head = &D_00557828[channel];
    record = &D_00552118[channel][*head];
    xglCdSetReadTag((channel << 16) | *head);
    xglCdReadFile(record->name, record->buffer, 1, func_001B6418);
    *head = (*head + 1) & 63;
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu054", func_001B6418);

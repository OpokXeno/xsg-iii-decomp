#include "common.h"
#include "shared.h"

/* Five signed halfwords in the TU's .data (scaffold label gSefDraw2DOverrides, 10 bytes). */
extern s16 gSefDraw2DOverrides[5];

s16 sefGetDraw2DOverride(void) {
    return gSefDraw2DOverrides[0];
}

typedef signed char s8;
typedef int s32;
extern s16 gSefDraw3DOverrides[];

s16 sefGetDraw3DOverride(void) {
    return gSefDraw3DOverrides[0];
}

/* One 0x40-byte entry of the record table at D_004516B0 (0x28 entries, indexed by clearRecordEntry). */
typedef struct RecordTableEntry {
    unsigned char unmodeled_00[0x40];
} RecordTableEntry;

extern RecordTableEntry D_004516B0[];
extern unsigned char D_004081F0[];
void *memset(void *destination, s32 value, u32 size);

void clearRecordTables(void) {
    memset(D_004081F0, 0, 0x494C0);
    memset(D_004516B0, 0, 0xA00);
}

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F5320);

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F5440);

/* The object record read by objectHasFlag5E: only the byte at +0x5E is claimed here. */
typedef struct RecordObject {
    unsigned char unmodeled_00[0x5E];
    signed char unk_5E;
} RecordObject;

s32 objectHasFlag5E(RecordObject *object) {
    if (!(object != 0)) {
        return 0;
    }
    return object->unk_5E != 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F5590);

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F56B0);

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F5768);

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F5850);

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F58F8);

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F5988);

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F5BD0);

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F5CA8);

void clearRecordEntry(u32 index) {
    if (index < 0x28U) {
        memset(&D_004516B0[index], 0, 0x40);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F5D48);

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F62F8);

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F6590);

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F6698);

extern unsigned char gEffectTable[];

void clearEffectTable(void) {
    memset(gEffectTable, 0, 0xC80);
}

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F6DE0);

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F6EE8);

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F7138);

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F7248);

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F72C8);

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F7348);

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F73C8);

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F74A8);

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F7588);

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F7668);

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F7740);

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F77D8);

INCLUDE_ASM("asm/main/nonmatchings/tu128", func_001F7870);

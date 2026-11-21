#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu129", func_001F7900);

INCLUDE_ASM("asm/main/nonmatchings/tu129", func_001F79B8);

INCLUDE_ASM("asm/main/nonmatchings/tu129", func_001F7A60);

INCLUDE_ASM("asm/main/nonmatchings/tu129", func_001F7B08);

INCLUDE_ASM("asm/main/nonmatchings/tu129", func_001F7BB0);

INCLUDE_ASM("asm/main/nonmatchings/tu129", func_001F7E38);

INCLUDE_ASM("asm/main/nonmatchings/tu129", func_001F7EC8);

INCLUDE_ASM("asm/main/nonmatchings/tu129", func_001F7F58);

INCLUDE_ASM("asm/main/nonmatchings/tu129", func_001F8010);

INCLUDE_ASM("asm/main/nonmatchings/tu129", func_001F8680);

INCLUDE_ASM("asm/main/nonmatchings/tu129", func_001F8750);

INCLUDE_ASM("asm/main/nonmatchings/tu129", func_001F8860);

INCLUDE_ASM("asm/main/nonmatchings/tu129", func_001F89C8);

INCLUDE_ASM("asm/main/nonmatchings/tu129", func_001F8D38);

INCLUDE_ASM("asm/main/nonmatchings/tu129", func_001F8E28);

INCLUDE_ASM("asm/main/nonmatchings/tu129", func_001F9658);

INCLUDE_ASM("asm/main/nonmatchings/tu129", func_001F98C8);

INCLUDE_ASM("asm/main/nonmatchings/tu129", func_001F9998);

INCLUDE_ASM("asm/main/nonmatchings/tu129", func_001F99F0);

INCLUDE_ASM("asm/main/nonmatchings/tu129", func_001F9A68);

typedef unsigned int u32;

/* One 128-byte record of the table at gSdvSpecialRecords (the same record sdv_alter.c indexes); inUse is its word at +0x04. */
typedef struct SdvSpecialRecord {
    unsigned char unmodeled_00[0x04];
    unsigned int inUse;
    unsigned char unmodeled_08[0x78];
} SdvSpecialRecord;

extern SdvSpecialRecord gSdvSpecialRecords[];

SdvSpecialRecord *sdvGetSpecialRecord(u32 specialRecordIndex) {
    if (!(specialRecordIndex < 0x20U)) {
        return 0;
    }
    return &gSdvSpecialRecords[specialRecordIndex];
}

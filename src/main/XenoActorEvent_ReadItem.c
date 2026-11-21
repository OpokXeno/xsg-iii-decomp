#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent_ReadItem", func_00251278);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent_ReadItem", func_002512B0);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent_ReadItem", func_00251350);

typedef signed char s8;
typedef short s16;
typedef int s32;
typedef unsigned char u8;

/* Item entry returned by xglCdRegisterEntry: link pointers at 0x504, 0x508, 0x50C and 0x530; an s16 at 0x9C. */
typedef struct CdRegisteredEntry {
    u8 unk_00[0x9C];
    s16 unk_9C;
    u8 unk_9E[0x504 - 0x9E];
    void *resolvedFirstLink;
    void *resolvedSecondLink;
    void *resolvedThirdLink;
    u8 unk_510[0x530 - 0x510];
    void *resolvedFourthLink;
} CdRegisteredEntry;

CdRegisteredEntry *xglCdRegisterEntry();
s32 func_0028D760(void *);

/* Source item record: four offsets relative to the relativeBase address passed to the reader (0 = none). */
struct RelativeOffsetRecord {
    s8 unk00[4];
    s32 firstRel;
    s32 secondRel;
    s32 thirdRel;
    s32 fourthRel;
};

s32 itemSourceResolveEntries(u8 *relativeBase, struct RelativeOffsetRecord *relativeRecords, s32 registerCount) {
    struct RelativeOffsetRecord *relativeRecord;
    CdRegisteredEntry *registeredEntry;
    s32 entriesRemaining;
    s32 firstRelative;
    s32 secondRelative;
    s32 thirdRelative;
    s32 fourthRelative;

    if (registerCount > 0) {
        relativeRecord = relativeRecords;
        entriesRemaining = registerCount;
        do {
            registeredEntry = xglCdRegisterEntry();
            firstRelative = relativeRecord->firstRel;
            if (firstRelative != 0) {
                registeredEntry->resolvedFirstLink = relativeBase + firstRelative;
            } else {
                registeredEntry->resolvedFirstLink = 0;
            }
            secondRelative = relativeRecord->secondRel;
            if (secondRelative != 0) {
                registeredEntry->resolvedSecondLink = relativeBase + secondRelative;
            } else {
                registeredEntry->resolvedSecondLink = 0;
            }
            thirdRelative = relativeRecord->thirdRel;
            if (thirdRelative != 0) {
                registeredEntry->resolvedThirdLink = relativeBase + thirdRelative;
            } else {
                registeredEntry->resolvedThirdLink = 0;
            }
            fourthRelative = relativeRecord->fourthRel;
            if (fourthRelative != 0) {
                registeredEntry->resolvedFourthLink = relativeBase + fourthRelative;
            } else {
                registeredEntry->resolvedFourthLink = 0;
            }
            entriesRemaining -= 1;
            func_0028D760(registeredEntry);
            registeredEntry->unk_9C = 0;
        } while (entriesRemaining != 0);
    }
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent_ReadItem", func_00251648);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent_ReadItem", func_00251800);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent_ReadItem", func_002519E8);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent_ReadItem", func_00251D58);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent_ReadItem", func_002520E0);

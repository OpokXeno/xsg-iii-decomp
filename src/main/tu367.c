#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu367", func_002B7808);

typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;
extern s32 gSlotUsedFlags[];

/* One 0x370-byte record: the first word is the only field the code in this unit touches. */
typedef struct SceneRecord {
    s32 sceneFlag;
    u8 unk_04[0x370 - 0x04];
} SceneRecord;

/* Stores value into the 32-bit slot of gSlotUsedFlags at index (negative indices are ignored). */
void slotUsedFlagSet(s32 slotNumber, s32 usedFlag) {
    if (slotNumber >= 0) {
        gSlotUsedFlags[slotNumber] = usedFlag;
    }
}

extern SceneRecord gSceneRecords[];

/* Stores value into the first word of the 0x370-byte record selected by index (only records 0 and 1 exist). */
void sceneRecordFlagWordSet(u32 sceneRecordNumber, s32 sceneFlagValue) {
    if (sceneRecordNumber < 2U) {
        gSceneRecords[sceneRecordNumber].sceneFlag = sceneFlagValue;
    }
}

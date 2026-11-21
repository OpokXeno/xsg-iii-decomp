#include "common.h"

typedef unsigned char u8;
typedef int s32;
typedef short s16;
typedef unsigned short u16;
typedef unsigned int u32;
typedef float f32;

typedef struct GameStateBlock {
    s32 headValue;
    u16 pendingValue;
    u8 unk_06[0x2];
    u8 pendingRecord[0x304];
    s16 committedValue;
    u8 unk_30E[0x2];
    u8 committedRecord[0x304];
    u8 unk_614[0x21C];
    s32 unk_830;
    u8 unk_834[0x444];
    s32 unk_C78;
    s32 unk_C7C;
    s32 unk_C80;
    s32 unk_C84;
    s32 unk_C88;
    s32 unk_C8C;
    u8 unk_C90[0x4];
    s32 unk_C94;
    u8 unk_C98[0x508];
} GameStateBlock;

typedef struct SlotEntry {
    u8 unk_00[0x10];
    s16 id;
    u8 unk_12[0xE];
} SlotEntry;

#define SLOT_COUNT 16

extern unsigned char D_0055CC00[];
#define gGameStateBlock (*(GameStateBlock *) D_0055CC00)
void *memset();

void stateBlockClear(void) {
    memset(&gGameStateBlock, 0, sizeof(GameStateBlock));
}

void stateBlockSetHead(s32 newHeadValue) {
    gGameStateBlock.headValue = newHeadValue;
}

GameStateBlock *stateBlockGet(void) {
    return &gGameStateBlock;
}

INCLUDE_ASM("asm/main/nonmatchings/tu067", func_001BE470);

INCLUDE_ASM("asm/main/nonmatchings/tu067", func_001BE5B0);

void func_0030C130();

void stateBlockCommit(void)
{
    gGameStateBlock.committedValue = gGameStateBlock.pendingValue;
    func_0030C130(gGameStateBlock.pendingRecord, gGameStateBlock.committedRecord, 0x304);
}

INCLUDE_ASM("asm/main/nonmatchings/tu067", func_001BE6A8);

INCLUDE_ASM("asm/main/nonmatchings/tu067", func_001BE7C0);

INCLUDE_ASM("asm/main/nonmatchings/tu067", func_001BE828);

INCLUDE_ASM("asm/main/nonmatchings/tu067", func_001BE860);

GameStateBlock *stateBlockGet(void);
void slotTableClearIds();
void func_001BEF78();
void mapCollisionInit(s32 value);

void stateBlockReset(void) {
    GameStateBlock *gameStateBlock;

    gameStateBlock = stateBlockGet();
    gameStateBlock->unk_830 = 0;
    gameStateBlock->unk_C78 = 0;
    gameStateBlock->unk_C7C = 0;
    gameStateBlock->unk_C80 = 0;
    gameStateBlock->unk_C84 = 0;
    gameStateBlock->unk_C88 = 0;
    gameStateBlock->unk_C8C = 0;
    gameStateBlock->unk_C94 = 0;
    slotTableClearIds();
    func_001BEF78();
    mapCollisionInit(gameStateBlock->unk_C80);
}

INCLUDE_ASM("asm/main/nonmatchings/tu067", func_001BE8E0);

INCLUDE_ASM("asm/main/nonmatchings/tu067", func_001BE990);

INCLUDE_ASM("asm/main/nonmatchings/tu067", func_001BEC28);

extern SlotEntry gSlotIdTable[];

void slotTableClearIds(void) {
    s32 i;
    SlotEntry *slotCursor;

    slotCursor = gSlotIdTable;
    memset(gSlotIdTable, 0, SLOT_COUNT * sizeof(SlotEntry));
    for (i = 0; i < SLOT_COUNT; i++) {
        slotCursor->id = -1;
        slotCursor++;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu067", func_001BED60);

INCLUDE_ASM("asm/main/nonmatchings/tu067", func_001BEE18);

INCLUDE_ASM("asm/main/nonmatchings/tu067", func_001BEE80);

INCLUDE_ASM("asm/main/nonmatchings/tu067", func_001BEF78);

INCLUDE_ASM("asm/main/nonmatchings/tu067", func_001BEFD8);

INCLUDE_ASM("asm/main/nonmatchings/tu067", func_001BF058);

INCLUDE_ASM("asm/main/nonmatchings/tu067", func_001BF0B0);

extern s32 D_0055D6E4[];

#define WORD_TABLE_COUNT 0x64

s32 wordTableGet(u32 wordIndex) {
    if (!(wordIndex < WORD_TABLE_COUNT)) {
        return 0;
    }
    return D_0055D6E4[wordIndex];
}

s32 wordTableSet(u32 wordIndex, s32 wordValue) {
    if (!(wordIndex < WORD_TABLE_COUNT)) {
        return 0;
    }
    D_0055D6E4[wordIndex] = wordValue;
    return 1;
}

void func_001B64D8(s32 begin, s32 end);

void memoryRangeInitDefault(void) {
    func_001B64D8(0xE00000, 0x01200000);
}

INCLUDE_ASM("asm/main/nonmatchings/tu067", func_001BF1B8);

s32 func_001BF1B8(s32 length, s32 address);

s32 memoryBanksTest(s32 memoryBankMask) {
    s32 errors;

    errors = 0;
    if (memoryBankMask & 1) {
        errors = func_001BF1B8(0x100000, 0xCF800000) == 0;
        if (func_001BF1B8(0x39800, 0xCF801000) == 0) {
            errors++;
        }
    }
    if (memoryBankMask & 2) {
        if (func_001BF1B8(0x200000, 0xCF010000) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x120080, 0xCF010001) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x70880, 0xCF010100) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x70880, 0xCF010101) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x70880, 0xCF010102) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x70880, 0xCF010100) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x10080, 0xCF010200) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x10080, 0xCF010201) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x10080, 0xCF010210) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x10080, 0xCF010211) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x10080, 0xCF010220) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x10080, 0xCF010221) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x4E080, 0xCF010300) == 0) {
            errors++;
        }
    }
    if (memoryBankMask & 4) {
        if (func_001BF1B8(0xA3880, 0xCF020000) == 0) {
            errors++;
        }
        if (func_001BF1B8(0xA3880, 0xCF020001) == 0) {
            errors++;
        }
        if (func_001BF1B8(0xA3880, 0xCF020002) == 0) {
            errors++;
        }
    }
    if (memoryBankMask & 8) {
        if (func_001BF1B8(0x64080, 0xCF040000) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x64080, 0xCF040001) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x64080, 0xCF040002) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x64080, 0xCF040003) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x85080, 0xCF040100) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x85080, 0xCF040101) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x85080, 0xCF040102) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x85080, 0xCF040103) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x1C080, 0xCF040200) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x1C080, 0xCF040201) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x1C080, 0xCF040202) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x1C080, 0xCF040203) == 0) {
            errors++;
        }
    }
    return errors == 0;
}

s32 poolReleaseMatching(s32 mode, s32 expected, s32 mask);

void memoryBanksReset(s32 memoryBankMask) {
    if (memoryBankMask & 1) {
        poolReleaseMatching(1, 0xCF800000, 0xFFFF0000);
    }
    if (memoryBankMask & 2) {
        poolReleaseMatching(1, 0xCF010000, 0xFFFF0000);
    }
    if (memoryBankMask & 4) {
        poolReleaseMatching(1, 0xCF020000, 0xFFFF0000);
    }
    if (memoryBankMask & 8) {
        poolReleaseMatching(1, 0xCF040000, 0xFFFF0000);
    }
    if (memoryBankMask & 0x10) {
        poolReleaseMatching(1, 0xCF080000, 0xFFFF0000);
    }
    if (memoryBankMask & 0x20) {
        poolReleaseMatching(1, 0xCF100000, 0xFFFF0000);
    }
}

void func_0019B8D8(void);
void xglStudioMainCameraInit(void);

void studioCameraInit(void) {
    func_0019B8D8();
    xglStudioMainCameraInit();
}

INCLUDE_ASM("asm/main/nonmatchings/tu067", func_001BF688);

INCLUDE_ASM("asm/main/nonmatchings/tu067", func_001BF6F8);

extern f32 gAngleUnitsToDegrees;
s32 func_001BF7C8(f32 angle, f32 turns);

s32 angleWrapTurns(f32 angle, f32 turns) {
    s32 scaled;
    s32 period;
    s32 remainder;

    scaled = func_001BF7C8(angle * gAngleUnitsToDegrees, turns);
    period = turns * 360.0f;
    remainder = scaled % period;
    return ((remainder < 0) * period) + remainder;
}

INCLUDE_ASM("asm/main/nonmatchings/tu067", func_001BF7C8);

INCLUDE_ASM("asm/main/nonmatchings/tu067", func_001BF810);

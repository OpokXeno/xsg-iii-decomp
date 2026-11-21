/*
 * OV02 original TU 9: 0x00a84170..0x00a84998 (28 functions)
 */
#include "common.h"

typedef unsigned char u8;
typedef int s32;
typedef unsigned int u32;
extern unsigned char ov02DataSectionTable[];

u8 *ov02DataSectionGet(s32 sectionOffset) {
    return &ov02DataSectionTable[*(s32 *) (0x4CC8C0 + sectionOffset)];
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu009", func_00A84190);

INCLUDE_ASM("asm/nonmatchings/ov02/tu009", func_00A841B0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu009", func_00A841D0);

u8 *ov02DataSectionGet(s32 sectionOffset);

u8 *ov02Record14Get(s32 entryNumber) {
    if (entryNumber >= 0x12) {
        return 0;
    }
    return ov02DataSectionGet(0x10) + ((entryNumber - 1) * 0x14);
}

u8 *roboKeyTableGet(s32 entryNumber) {
    if (entryNumber == 0) {
        return 0;
    }
    return ov02DataSectionGet(0x14) + ((entryNumber - 1) * 0xE);
}

u8 *ov02ChunkedEntryGet(s32 entryNumber)
{
  s32 sectionOffset;
  s32 entryWithinSection;

  if (entryNumber == 0)
  {
    return 0;
  }
  if (entryNumber >= 0x63)
  {
    entryWithinSection = entryNumber - 0x63;
    sectionOffset = 0x1C;
  }
  else
  {
    entryWithinSection = entryNumber - 1;
    sectionOffset = 0x18;
  }
  return ov02DataSectionGet(sectionOffset) + (entryWithinSection << 5);
}

u8 *ov02ItemEntryGet(s32 entryNumber) {
    if (entryNumber == 0) {
        return 0;
    }
    return ov02DataSectionGet(0x24) + ((entryNumber - 1) * 0x28);
}

u8 *ov02SlotEntryGet(s32 entryNumber) {
    if (entryNumber == 0) {
        return 0;
    }
    return (ov02DataSectionGet(0x20) + (entryNumber << 5)) - 0x20;
}

u8 *ov02ActionEntryGet(s32 entryNumber) {
    if (entryNumber == 0) {
        return 0;
    }
    return ov02DataSectionGet(0x28) + ((entryNumber - 1) * 0x28);
}

u8 *ov02BattleSlotGet(s32 entryNumber) {
    if (entryNumber == 0) {
        return 0;
    }
    return ov02DataSectionGet(0x2C) + ((entryNumber - 1) * 0x28);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu009", func_00A84440);

u8 *ov02PoolEntryGet(s32 entryNumber) {
    if (entryNumber == 0) {
        return 0;
    }
    return ov02DataSectionGet(0x38) + ((entryNumber - 1) * 0x14);
}

u8 *ov02TargetEntryGet(s32 entryNumber) {
    if (entryNumber == 0) {
        return 0;
    }
    return ov02DataSectionGet(0x3C) + ((entryNumber - 1) * 0x14);
}

u8 *ov02TemplateEntryGet(s32 entryNumber) {
    if (entryNumber == 0) {
        return 0;
    }
    return ov02DataSectionGet(0x40) + ((entryNumber - 1) * 0x1C);
}

u8 *ov02ProfileEntryGet(s32 entryNumber) {
    if (entryNumber == 0) {
        return 0;
    }
    return ov02DataSectionGet(0x44) + ((entryNumber - 1) * 0x18);
}

u8 *ov02DefEntryGet(s32 entryNumber) {
    if (entryNumber == 0) {
        return 0;
    }
    return ov02DataSectionGet(0x48) + ((entryNumber - 1) * 0x14);
}

u8 *ov02SequenceEntryGet(s32 entryNumber) {
    if (entryNumber == 0) {
        return 0;
    }
    return ov02DataSectionGet(0x4C) + ((entryNumber - 1) * 0x14);
}

u8 *ov02EffectEntryGet(s32 entryNumber) {
    if (entryNumber == 0) {
        return 0;
    }
    return ov02DataSectionGet(0x50) + ((entryNumber - 1) * 0x14);
}

u8 *ov02MotionEntryGet(s32 entryNumber) {
    if (entryNumber == 0) {
        return 0;
    }
    return ov02DataSectionGet(0x54) + ((entryNumber - 1) * 0x18);
}

u8 *ov02WordEntryGet(s32 entryNumber) {
    if (entryNumber == 0) {
        return 0;
    }
    return ov02DataSectionGet(0x60) + ((entryNumber - 1) * 4);
}

u8 *ov02FormEntryGet(s32 entryNumber) {
    if (entryNumber == 0) {
        return 0;
    }
    return ov02DataSectionGet(0x5C) + ((entryNumber - 1) * 0x2C);
}

u8 *ov02DropRecordGet(s32 entryNumber) {
    if (entryNumber < 0x12) {
        return 0;
    }
    return ov02DataSectionGet(0x58) + ((entryNumber - 0x12) * 0x14);
}

u8 *ov02GroupEntryGet(s32 entryNumber) {
    if (entryNumber == 0) {
        return 0;
    }
    return ov02DataSectionGet(0x68) + ((entryNumber - 1) * 0x14);
}

u8 *ov02ShortEntryGet(s32 entryNumber) {
    if (entryNumber < 0x12) {
        return 0;
    }
    return ov02DataSectionGet(0x6C) + ((entryNumber - 0x12) * 2);
}

u8 *ov02BlockEntryGet(s32 entryNumber) {
    u32 zeroBasedEntry;

    zeroBasedEntry = entryNumber - 1;
    if (zeroBasedEntry >= 0x190U) {
        return 0;
    }
    return ov02DataSectionGet(0x70) + ((entryNumber - 1) * 8);
}

u8 *ov02SmallBlockEntryGet(s32 entryNumber) {
    u32 zeroBasedEntry;

    zeroBasedEntry = entryNumber - 1;
    if (zeroBasedEntry >= 0x40U) {
        return 0;
    }
    return ov02DataSectionGet(0x74) + ((entryNumber - 1) * 8);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu009", func_00A848C8);

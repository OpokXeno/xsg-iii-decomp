/*
 * OV02 original TU 91: 0x00ab1a40..0x00ab2f40 (35 functions)
 */
#include "common.h"

typedef int s32;
typedef signed char s8;
typedef unsigned char u8;

/* Owner record reached through a link's +0x2B0 pointer: the rank byte at +0x12 and the order word at +0x38. */
typedef struct RoboOwner {
    u8 unk_00[0x12];
    s8 rank;                            /* +0x12 */
    u8 unk_13[0x38 - 0x13];
    s32 order;                          /* +0x38 */
} RoboOwner;

/* Link record: the owner pointer sits at +0x2B0. */
typedef struct RoboOwnerLink {
    u8 unk_00[0x2B0];
    RoboOwner *owner;                   /* +0x2B0 */
} RoboOwnerLink;

/* Object record: the link at +0xA0, the flags word at +0xBC and the result word at +0xE4. */
typedef struct RoboBattleUnit {
    u8 unk_00[0xA0];
    RoboOwnerLink *ownerLink;           /* +0xA0 */
    u8 unk_A4[0xBC - 0xA4];
    s32 battleFlags;                      /* +0xBC */
    u8 unk_C0[0xE4 - 0xC0];
    s32 storedResult;                   /* +0xE4 */
} RoboBattleUnit;

s32 sbNativeBufferGet(s32);

s32 roboScriptLoadImage(void) {
    return sbNativeBufferGet(1);
}
extern const char D_00ABB6C0[];
void assertFailed(const char *message, const char *file, int line);
s32 sbRunThreads();
s32 func_001D1A50(s32);
s32 sbStartImage(s32);

void roboScriptStart(void) {
    extern const char D_00ABB6D0[];
    s32 imageId;

    imageId = roboScriptLoadImage();
    if (func_001D1A50(imageId) == 0) {
        assertFailed(D_00ABB6C0, D_00ABB6D0, 0x3B);
    }
    sbStartImage(imageId);
    sbRunThreads();
}

void roboScriptRunThreads(void) {
    sbRunThreads();
}

s32 sbThreadsRelease(s32);

void roboScriptReleaseThreads(void) {
    s32 imageId;

    imageId = roboScriptLoadImage();
    sbThreadsRelease(imageId);
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_script", func_00AB1AE8);

INCLUDE_ASM("asm/nonmatchings/ov02/robo_script", func_00AB1B80);

INCLUDE_ASM("asm/nonmatchings/ov02/robo_script", func_00AB1C48);

INCLUDE_ASM("asm/nonmatchings/ov02/robo_script", func_00AB1CA0);

s32 roboActorCompareOrder(RoboBattleUnit **firstBattleUnitSlot, RoboBattleUnit **secondBattleUnitSlot)
{
  RoboOwnerLink *secondOwnerLink;
  RoboOwnerLink *firstOwnerLink;
  RoboOwner *second_owner_2;
  RoboBattleUnit *secondBattleUnit;
  s32 firstOrder;
  RoboOwner *first_owner;
  RoboOwner *first_owner_2;
  RoboOwner *second_owner;
  first_owner = 0;
  secondBattleUnit = *secondBattleUnitSlot;
  firstOwnerLink = (*firstBattleUnitSlot)->ownerLink;
  if (firstOwnerLink != 0)
  {
    first_owner = firstOwnerLink->owner;
  }
  secondOwnerLink = secondBattleUnit->ownerLink;
  second_owner = 0;
  if (secondOwnerLink != 0)
  {
    second_owner = secondOwnerLink->owner;
  }
  if (!(first_owner->order >= second_owner->order))
  {
    return -1;
  }
  first_owner_2 = 0;
  if (firstOwnerLink != 0)
  {
    first_owner_2 = firstOwnerLink->owner;
  }
  second_owner_2 = 0;
  if (secondOwnerLink != 0)
  {
    second_owner_2 = secondOwnerLink->owner;
  }
  firstOrder = first_owner_2->order;
  return second_owner_2->order < firstOrder;
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_script", func_00AB1D58);

s32 roboActorCompareRank(RoboBattleUnit **firstBattleUnitSlot, RoboBattleUnit **secondBattleUnitSlot)
{
  RoboOwnerLink *secondOwnerLink;
  RoboOwnerLink *firstOwnerLink;
  RoboOwner *second_owner_2;
  RoboBattleUnit *secondBattleUnit;
  RoboOwner *first_owner;
  RoboOwner *first_owner_2;
  RoboOwner *second_owner;
  first_owner = 0;
  firstOwnerLink = (*firstBattleUnitSlot)->ownerLink;
  secondBattleUnit = *secondBattleUnitSlot;
  if (firstOwnerLink != 0)
  {
    first_owner = firstOwnerLink->owner;
  }
  secondOwnerLink = secondBattleUnit->ownerLink;
  second_owner = 0;
  if (secondOwnerLink != 0)
  {
    second_owner = secondOwnerLink->owner;
  }
  if (!(first_owner->rank >= second_owner->rank))
  {
    return -1;
  }
  first_owner_2 = 0;
  if (firstOwnerLink != 0)
  {
    first_owner_2 = firstOwnerLink->owner;
  }
  second_owner_2 = 0;
  if (secondOwnerLink != 0)
  {
    second_owner_2 = secondOwnerLink->owner;
  }
  return first_owner_2->rank > second_owner_2->rank;
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_script", func_00AB1E28);

INCLUDE_ASM("asm/nonmatchings/ov02/robo_script", func_00AB1E90);

extern const char D_00ABB6E8[];
extern const char D_00ABB6F0[];
s32 func_00193730();
s32 func_00AAC210();
s32 vmArgGet();

void *roboCurrentBattleUnitGet();

s32 roboVmObjectSetValue(s32 thread) {
    extern const char D_00ABB6D0[];
    s32 newValue;
    s32 unitId;
    void *battleUnit;

    unitId = vmArgGet(thread, 1);
    newValue = vmArgGet(thread, 2);
    battleUnit = roboCurrentBattleUnitGet(unitId);
    if (battleUnit == 0) {
        func_00193730(D_00ABB6E8, D_00ABB6D0, 0x100, D_00ABB6F0, unitId);
    } else {
        func_00AAC210(battleUnit, newValue);
    }
    return 1;
}

s32 queueAssertNotEmptyAndFlag();

s32 roboVmObjectReset(s32 thread) {
    extern const char D_00ABB6D0[];
    s32 unitId;
    void *battleUnit;

    unitId = vmArgGet(thread, 1);
    battleUnit = roboCurrentBattleUnitGet(unitId);
    if (battleUnit == 0) {
        func_00193730(D_00ABB6E8, D_00ABB6D0, 0x115, D_00ABB6F0, unitId);
    } else {
        queueAssertNotEmptyAndFlag(battleUnit);
    }
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_script", func_00AB2000);

INCLUDE_ASM("asm/nonmatchings/ov02/robo_script", func_00AB2078);

INCLUDE_ASM("asm/nonmatchings/ov02/robo_script", func_00AB2148);

s32 entryStopWait(s32);
s32 func_00AB4CD0(s32);
s32 scWaitParseMovieScript();

s32 roboWaitMovieScript(s32 unused, s32 *done) {
    if (scWaitParseMovieScript() == 0) {
        *done = 1;
    } else {
        entryStopWait(1);
        func_00AB4CD0(0);
    }
    return 1;
}

s32 roboScriptApplySecond();
s32 roboScriptApplyThird();

s32 roboVmDispatchFourArgs(s32 thread) {
    extern void func_00A9DFB0();
    s32 appliedSecond;
    s32 third_arg;
    s32 second_arg;
    s32 fourth_arg;
    s32 first_arg;
    s32 appliedThird;

    first_arg = vmArgGet(thread, 1);
    second_arg = vmArgGet(thread, 2);
    third_arg = vmArgGet(thread, 3);
    fourth_arg = vmArgGet(thread, 4);
    appliedSecond = roboScriptApplySecond(second_arg);
    appliedThird = roboScriptApplyThird(third_arg);
    func_00A9DFB0(first_arg, appliedSecond, appliedThird, fourth_arg);
    return 1;
}

extern s32 roboKeyedEntryArg[];
s32 vmKeyedEntryGet();

s32 roboVmDispatchKeyed(s32 thread) {
    extern void func_00A9DFB0();
    s32 keyedEntry;
    s32 second_arg;
    s32 first_arg;
    s32 fourth_arg;
    s32 entryKey;
    s32 appliedSecond;

    first_arg = vmArgGet(thread, 1);
    second_arg = vmArgGet(thread, 2);
    entryKey = vmArgGet(thread, 3);
    keyedEntry = vmKeyedEntryGet(thread, entryKey);
    fourth_arg = vmArgGet(thread, 4);
    roboKeyedEntryArg[0] = keyedEntry;
    appliedSecond = roboScriptApplySecond(second_arg);
    func_00A9DFB0(first_arg, appliedSecond, roboKeyedEntryArg, fourth_arg);
    return 1;
}

s32 func_0026D7B8();
s32 func_00A9E4D8();

s32 func_0026D7B8();
s32 func_00A9E4D8();

s32 msgPutWait(s32 unused, s32 *done) {
    if (func_00A9E4D8() == 0) {
        *done = 1;
    }
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_script", func_00AB2370);

INCLUDE_ASM("asm/nonmatchings/ov02/robo_script", func_00AB2450);

extern const char D_00ABB7A8[];
extern const char D_00ABB7B0[];
s32 func_00AA1300();

s32 roboVmUnitStoreResult(s32 thread) {
    extern const char D_00ABB6D0[];
    s32 unitId;
    s32 targetUnitId;
    s32 targetBattleSlot;
    RoboBattleUnit *battleUnit;
    void *targetBattleUnit;

    unitId = vmArgGet(thread, 1);
    targetUnitId = vmArgGet(thread, 2);
    battleUnit = roboCurrentBattleUnitGet(unitId);
    if (battleUnit != 0) {
        targetBattleUnit = roboCurrentBattleUnitGet(targetUnitId);
        if (targetBattleUnit == 0) {
            func_00193730(D_00ABB7A8, D_00ABB6D0, 0x2A4, D_00ABB7B0, targetUnitId);
        }
        targetBattleSlot = func_00AA1300(targetBattleUnit);
        battleUnit->storedResult = targetBattleSlot;
    }
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_script", func_00AB2A18);

extern const char D_00ABB7D8[];
s32 func_001936F0();

s32 roboVmUnitGetValue(s32 thread) {
    extern const char D_00ABB6D0[];
    void *battleUnit;
    s32 unitId;

    unitId = vmArgGet(thread, 1);
    battleUnit = roboCurrentBattleUnitGet(unitId);
    if (battleUnit != 0) {
        return func_00AA1300(battleUnit);
    }
    func_001936F0(D_00ABB6D0, 0x2D9, D_00ABB7D8, unitId);
    return -1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_script", func_00AB2B60);

INCLUDE_ASM("asm/nonmatchings/ov02/robo_script", func_00AB2BF0);

s32 roboVmUnitAddFlags(s32 thread) {
    extern const char D_00ABB6D0[];
    s32 addedFlags;
    s32 unitId;
    s32 combinedFlags;
    RoboBattleUnit *battleUnit;

    unitId = vmArgGet(thread, 1);
    addedFlags = vmArgGet(thread, 2);
    battleUnit = roboCurrentBattleUnitGet(unitId);
    if (battleUnit == 0) {
        func_00193730(D_00ABB6E8, D_00ABB6D0, 0x318, D_00ABB6F0, unitId);
    } else {
        combinedFlags = battleUnit->battleFlags | addedFlags;
        battleUnit->battleFlags = combinedFlags;
    }
    return 1;
}

s32 battleEntryStartWithMode();

s32 roboVmStartEntry(s32 thread) {
    s32 keyedEntry;
    s32 unitId;
    void *targetBattleUnit;
    s32 entryKey;

    targetBattleUnit = 0;
    unitId = vmArgGet(thread, 1);
    entryKey = vmArgGet(thread, 2);
    keyedEntry = vmKeyedEntryGet(thread, entryKey);
    if (unitId != -1) {
        targetBattleUnit = roboCurrentBattleUnitGet(unitId);
    }
    battleEntryStartWithMode(keyedEntry, targetBattleUnit, 0);
    return 1;
}

s32 func_002C3590();
s32 seVoiceEnable();

s32 roboVmSeVoiceStart(s32 thread) {
    s32 keyedEntry;
    s32 second_arg;
    s32 first_arg;
    s32 entryKey;

    first_arg = vmArgGet(thread, 1);
    second_arg = vmArgGet(thread, 2);
    entryKey = vmArgGet(thread, 3);
    keyedEntry = vmKeyedEntryGet(thread, entryKey);
    vmArgGet(thread, 4);
    func_002C3590(first_arg, second_arg, keyedEntry);
    seVoiceEnable();
    return 1;
}

s32 seVoiceIsEnabled();

s32 roboWaitSeVoiceEnabled(s32 unused, s32 *done) {
    if (seVoiceIsEnabled() != 0) {
        *done = 1;
    }
    return 1;
}

s32 seVoiceDisable();

s32 roboVmSeVoiceStop(void) {
    seVoiceDisable();
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_script", func_00AB2E90);

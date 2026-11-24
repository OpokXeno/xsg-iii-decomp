/*
 * OV02 original TU 71: 0x00aa2268..0x00aa4a50 (50 functions)
 */
#include "common.h"

typedef signed char s8;
typedef int s32;
typedef short s16;
typedef long long s64;
typedef unsigned long long u64;
typedef unsigned int u32;
typedef float f32;
typedef unsigned char u8;

typedef struct BattleSlot {
    u8 unmodeled_00[0x90];
    s32 sefActorFlags;
    u8 unmodeled_94[0x518 - 0x94];
    s32 unmodeled_518;
} BattleSlot;

typedef struct BattleEntry {
    s32 unmodeled_00;
    s16 unmodeled_04;
    u8 unmodeled_06[0x76 - 0x06];
    s16 unmodeled_76;
    u8 unmodeled_78[0x8C - 0x78];
    s16 unmodeled_8C;
    u8 unmodeled_8E[0xDA - 0x8E];
    s16 unmodeled_DA;
} BattleEntry;

typedef struct BattleUnitSide {
    u8 unmodeled_00[0x0C];
    s32 unitSideFlags;
    u8 commandQueueHeader[0x20];
    u8 commandQueueStorage[0x20];
    u8 unmodeled_50[0x2B0 - 0x50];
    BattleEntry *battleEntry;
    BattleSlot *unitActor;
} BattleUnitSide;

typedef struct BattleCommand {
    u8 unmodeled_00[0x1C];
    s8 unmodeled_1C;
} BattleCommand;

typedef struct BattleActor {
    u8 unmodeled_00[0x24];
    f32 unmodeled_24;
    u8 unmodeled_28[0x18];
    u8 unmodeled_40[0x10];   /* argument of func_00AA3610 */
    u8 unmodeled_50[0x40];   /* argument of func_00AA3648 / func_00AA3668 */
    BattleSlot *childActorA;
    BattleSlot *childActorB;
    u8 unmodeled_98[0x08];
    BattleUnitSide *unit_side;
    s32 unitSefHandle;
    u8 unmodeled_A8[0x04];
    s32 battle_key;
    u8 unmodeled_B0[0x08];
    s32 messageParam;
    s32 actorFlags;
    f32 unmodeled_C0;
    f32 unmodeled_C4;
    u8 unmodeled_C8[0x10];
    s32 actorState;
    u8 unmodeled_DC[0x04];
    s32 unmodeled_e0;
    s32 unitIndex;
    s32 updateFrameCount;
    u8 unmodeled_EC[0x144];
    s32 unmodeled_230;
    u8 unmodeled_234[0x10];
    s32 modeWaitCount;
    s32 modeSefHandle;
    u8 unmodeled_24C[0x90];
    s32 unmodeled_2dc;
    s32 resultCode;
} BattleActor;

s32 battleActorHasUnitHandle(BattleActor *actor) {
    s32 hasUnitActor;
    BattleUnitSide *unitSide;

    hasUnitActor = 0;
    if (actor != 0) {
        unitSide = actor->unit_side;
        if (unitSide != 0) {
            hasUnitActor = unitSide->unitActor != 0;
        }
    }
    return hasUnitActor;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA2290);

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA22D0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA2420);

extern s32 gBattleSourceFile;
extern s32 gBattleAssertUnitSideNull;
extern s32 gBattleAssertEntryNull;
BattleEntry *battleEntryAcquire(void *, s32);
s32 battleEntryFindByKey(s32);
s32 commandQueueInit(void *, void *, s32);
s32 unitSideInitEntry(void *, s32);
s32 roboActorModeClear(void *, void *);
void *unitSideClaim();

void battleActorAttachUnitSide(BattleActor *actor) {
    extern s32 assertFailed(s32 *, s32 *, s32);
    s32 hasEntryForKey;
    BattleEntry *acquiredEntry;
    BattleUnitSide *unitSide;
    BattleEntry *battleEntry;
    BattleEntry *resetBattleEntry;

    unitSide = unitSideClaim();
    actor->unit_side = unitSide;
    if (unitSide == 0) {
        assertFailed(&gBattleAssertUnitSideNull, &gBattleSourceFile, 0xB5);
    }
    unitSide->unitSideFlags = 0;
    commandQueueInit(unitSide->commandQueueHeader, unitSide->commandQueueStorage, 0x20);
    hasEntryForKey = battleEntryFindByKey(actor->battle_key);
    acquiredEntry = battleEntryAcquire(actor->unit_side, actor->battle_key);
    actor->unit_side->battleEntry = acquiredEntry;
    if (acquiredEntry == 0) {
        assertFailed(&gBattleAssertEntryNull, &gBattleSourceFile, 0xBD);
    }
    if (hasEntryForKey == 0) {
        unitSideInitEntry(actor->unit_side, actor->battle_key);
    }
    battleEntry = 0;
    if (actor->unit_side != 0) {
        battleEntry = actor->unit_side->battleEntry;
    }
    resetBattleEntry = 0;
    battleEntry->unmodeled_76 = 1;
    if (actor->unit_side != 0) {
        resetBattleEntry = actor->unit_side->battleEntry;
    }
    resetBattleEntry->unmodeled_DA = -1;
    roboActorModeClear(actor, resetBattleEntry);
}

s32 battleEntryRelease(BattleUnitSide *);
s32 roboCurrentIsEnemy(void *);
s32 func_00AA3B40();
s32 unitSideRelease(BattleUnitSide *);

void battleActorDetachUnitSide(BattleActor *actor) {
    func_00AA3B40();
    if (roboCurrentIsEnemy(actor) != 0) {
        battleEntryRelease(actor->unit_side);
    }
    unitSideRelease(actor->unit_side);
    actor->unit_side = 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA2698);

void XenoActorRelease(BattleSlot *);
s32 unitSoundSlotRelease(void *);
s32 roboCurrentIsPlayer();
s32 func_00AB1880(s32);
s32 unitTblRemove(void *);

void battleActorReleaseUnit(BattleActor *actor) {
    s32 releasedSefHandle;
    BattleSlot *unitActor;
    BattleUnitSide *unitSide;

    if (roboCurrentIsPlayer() != 0) {
        unitSoundSlotRelease(actor->unit_side);
    }
    unitTblRemove(actor->unit_side);
    releasedSefHandle = actor->unitSefHandle;
    if (releasedSefHandle != 0) {
        func_00AB1880(releasedSefHandle);
        actor->unitSefHandle = 0;
    }
    unitSide = actor->unit_side;
    if (unitSide != 0) {
        unitActor = unitSide->unitActor;
        if (unitActor != 0) {
            XenoActorRelease(unitActor);
            actor->unit_side->unitActor = 0;
        }
    }
    actor->actorFlags &= ~2;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA2A30);

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA2D88);

extern s32 gBattleStatusSetArgument;
extern s32 gBattleStatusClearArgument;
s32 func_001936F0(s32 *, s32, s32 *, s32);
s64 sefActorFlagsGet(BattleSlot *);
s32 unitClearStatusBits(void *, s32);
s32 unitSetStatusBits(void *, s32);

void battleActorApplyEntryFlags(BattleActor *actor)
{
  BattleSlot *unitActor;
  u64 unitSefFlags;
  BattleUnitSide *unitSide;
  BattleUnitSide *unitSideAgain;
  unitSide = actor->unit_side;
  unitActor = 0;
  if (unitSide != 0)
  {
    unitActor = unitSide->unitActor;
  }
  unitSefFlags = ((u64) (sefActorFlagsGet(unitActor) << 0x20)) >> 0x20;
  unitSideAgain = actor->unit_side;
  if ((unitSideAgain != 0) && (unitSideAgain->unitActor != 0))
  {
    if (unitSefFlags & 0x10)
    {
      unitSetStatusBits(actor, 1);
      func_001936F0(&gBattleSourceFile, 0x1D7, &gBattleStatusSetArgument, actor->messageParam);
    }
    if (unitSefFlags & 8)
    {
      unitClearStatusBits(actor, -2);
      func_001936F0(&gBattleSourceFile, 0x1DB, &gBattleStatusClearArgument, actor->messageParam);
    }
    if (unitSefFlags & 0x10000000)
    {
      actor->unmodeled_230 = 1;
    }
  }
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA2EC8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA3340);

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA3380);

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA33D0);

extern s32 D_00ABA1E0;
extern s32 gBattleMoviePlayer;
s32 GameMpeg2Play(s32 *, s32 *, s32, s32);
s32 func_00AA1300();

s32 *battleActorPlayMovie(BattleActor *actor) {
    s32 movieId;
    movieId = func_00AA1300();
    GameMpeg2Play(&gBattleMoviePlayer, &D_00ABA1E0, movieId, actor->battle_key);
    return &gBattleMoviePlayer;
}

s32 func_00290548(BattleSlot *, s32);

void battleActorRequestSlotState(BattleActor *actor) {
    BattleSlot *unitActor;
    BattleUnitSide *unitSide;

    unitSide = actor->unit_side;
    unitActor = 0;
    if (unitSide != 0) {
        unitActor = unitSide->unitActor;
    }
    func_00290548(unitActor, 0x1E);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA3610);

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA3630);

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA3648);

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA3668);

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA3680);

s32 func_00AA3648();
s32 func_00AA3668();

void battleActorSetDispatchMode(BattleActor *actor, s32 isStarting) {
    actor->actorFlags &= ~4;
    if (isStarting != 0) {
        func_00AA3648(actor, actor->unmodeled_50);
        return;
    }
    func_00AA3668(actor, actor->unmodeled_50);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA3738);

s32 battleActorIsStateActive(BattleActor *actor) {
    return (u32) (actor->actorState - 1) < 2U;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA3828);

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA3898);

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA3920);

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA39A8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA3B40);

s32 func_00AA3680();
s32 battleSecondaryAddClamped();
s32 func_00AABE48();
s32 func_00AADBA0();
s32 func_00AADFC8();
s32 func_00AAE040();

void battleActorBeginEntry(BattleActor *actor) {
    extern s32 publishEntryRecords();
    s32 unitFlagsCleared;
    BattleUnitSide *unitSide;
    BattleEntry *battleEntry;

    func_00AA3680(actor, 0);
    unitSide = actor->unit_side;
    actor->unmodeled_e0 = 1;
    battleEntry = 0;
    unitFlagsCleared = unitSide->unitSideFlags & ~0x2000;
    unitSide->unitSideFlags = unitFlagsCleared;
    if (unitSide != 0) {
        battleEntry = unitSide->battleEntry;
    }
    battleSecondaryAddClamped(actor, battleEntry->unmodeled_04);
    func_00AABE48(actor);
    publishEntryRecords();
    func_00AADBA0(actor);
    func_00AADFC8(actor);
    func_00AAE040(actor->unitIndex);
}

void battleActorSetDispatchMode(BattleActor *actor, s32 mode);
extern s32 gBattleAssertActorNull;
void soundSlotReleaseCurrent();
void func_00AA4278(void *, s32);
void roboActorModeEnd(void *);
void func_00AAE158(void *);

void battleActorUpdate(BattleActor *actor) {
    extern void assertFailed(s32 *, s32 *, s32);
    extern void battleActorSetDispatchMode();
    extern void publishEntryRecords();
    s32 waitCount;
    BattleUnitSide *unitSide;
    BattleSlot *childActorA;
    BattleSlot *childActorB;
    BattleEntry *battleEntry;

    if (actor == 0) {
        assertFailed(&gBattleAssertActorNull, &gBattleSourceFile, 0x3BF);
    }
    unitSide = actor->unit_side;
    battleEntry = 0;
    if (unitSide != 0) {
        battleEntry = unitSide->battleEntry;
    }
    if (battleEntry->unmodeled_8C > 0) {
        waitCount = actor->modeWaitCount;
        if (waitCount >= 2) {
            actor->modeWaitCount = waitCount - 1;
        } else if (waitCount == 1) {
            roboActorModeEnd(actor);
        }
    }
    soundSlotReleaseCurrent();
    func_00AA4278(actor, 3);
    childActorA = actor->childActorA;
    if (childActorA != 0) {
        childActorA->sefActorFlags &= ~4;
    }
    childActorB = actor->childActorB;
    if (childActorB != 0) {
        childActorB->sefActorFlags &= ~4;
    }
    battleActorSetDispatchMode(actor, 0);
    func_00AAE158(actor);
    actor->actorFlags |= 0x8000;
    publishEntryRecords();
    actor->updateFrameCount += 1;
}

void battleActorRequestSlotState(BattleActor *actor);
void battleActorApplyModeFlags(BattleActor *actor, s32 mode);
void battleActorPostStartEvent(BattleActor *actor);
s32 calcStatGet(void *, s32, s32);
s32 func_00A81868(void *, s32);
s32 func_00A94218(void *);
void *roboBattleUnitGet(u32 battleSlotIndex, const char *caller_file, s32 caller_line);
s32 roboSetActive(s32);
s32 func_00AA3610(void *, void *);
s32 func_00AA5B20(void *, s32, s32, s32);
s32 func_00AAB820(s32, void *, void *, s32 *, s32);
s32 entryTeardown();
s32 battleEntryStartDefault(s32);
s32 battleUnitSubmitQueuedValue(void *, s32, s32);
s32 stringTableEntryGet(s32);

s32 battleActorTryStart(BattleActor *actor) {
    extern s32 func_00AA4780(void *, void *, s32, s32);
    extern s32 publishEntryRecords();
    extern s32 seStartDefaultVoice(s32);
    BattleEntry *battleEntry;
    s32 startAction;
    s32 scaledAmount;
    s32 started;
    BattleUnitSide *unitSide;
    void *targetRoboUnit;
    s32 startEntryId;

    started = 0;
    startAction = calcStatGet(actor->unit_side, 0, 0x4000);
    if (startAction > 0) {
        entryTeardown();
        roboSetActive(1);
        battleActorApplyModeFlags(actor, 3);
        func_00AA3610(actor, actor->unmodeled_40);
        battleActorSetDispatchMode(actor, 1);
        targetRoboUnit = roboBattleUnitGet(actor->unitIndex, &gBattleSourceFile, 0x3F6);
        func_00AAB820(0x10, actor, targetRoboUnit, &gBattleSourceFile, 0x3F6);
        startEntryId = stringTableEntryGet(startAction + 0x32);
        battleEntryStartDefault(startEntryId);
        if (startAction == 1) {
            func_00AA5B20(actor, 0x1E, 0, 0);
            seStartDefaultVoice(0x29);
            func_00AA4780(actor, actor, 0x136, 0xD);
            unitSide = actor->unit_side;
            battleEntry = 0;
            if (unitSide != 0) {
                battleEntry = unitSide->battleEntry;
            }
            scaledAmount = (battleEntry->unmodeled_00 * 0x50) / 100;
            func_00A81868(unitSide, -scaledAmount);
            publishEntryRecords();
            battleUnitSubmitQueuedValue(actor, scaledAmount, 1);
            battleActorPostStartEvent(actor);
        } else {
            func_00AA5B20(actor, 1, 0, 0);
        }
        started = 1;
        battleActorRequestSlotState(actor);
    }
    func_00A94218(actor->unit_side);
    return started;
}

s32 roboBattleStateBlockGet();
s32 curActorModeIsOneOrTwo(s32);

s32 battleActorIsReady(BattleActor *actor) {
    s32 isReady;
    s32 activeRoboMode;

    activeRoboMode = roboBattleStateBlockGet();
    isReady = curActorModeIsOneOrTwo(activeRoboMode);
    if ((isReady != 0) || !(actor->unmodeled_24 < 0.4f)) {
        isReady = 1;
    }
    return isReady;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA3EF8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA4278);

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA4308);

s32 func_00AA4890(void *);
s32 roboModeEffectRelease();

void battleActorApplyModeFlags(BattleActor *actor, s32 actionFlags)
{
    extern s32 func_00AA4780(void *, s32, s32, s32);
    s32 modeLowBit;
    BattleUnitSide *unitSideAgain;
    BattleUnitSide *unitSide;
    BattleSlot *unitActor;
    BattleSlot *unitActorAgain;

    unitSide = actor->unit_side;
    if ((unitSide != 0) && (unitSide->unitActor != 0))
    {
        modeLowBit = 1;
        modeLowBit = actionFlags & modeLowBit;
        if (actionFlags & 2)
        {
            if (unitSide->battleEntry->unmodeled_8C > 0)
            {
                roboModeEffectRelease();
                actor->modeSefHandle = func_00AA4780(actor, 0, 0x136, 0x15);
            }
            ;
        }
        if ((actionFlags & 1) != 0)
        {
            func_00AA4890(actor);
        }
        unitActorAgain = 0;
        unitSideAgain = actor->unit_side;
        actor->unmodeled_C4 = 1.0f;
        actor->unmodeled_C0 = 1.0f;
        if (unitSideAgain != 0)
        {
            unitActorAgain = unitSideAgain->unitActor;
        }
        unitActor = 0;
        if (unitSideAgain != 0)
        {
            unitActor = unitSideAgain->unitActor;
        }
        unitActorAgain->sefActorFlags = unitActor->sefActorFlags & (~4);
    }
}

void battleActorPostResetEvents(BattleActor *actor);

void battleActorReset(BattleActor *actor) {
    actor->unmodeled_C4 = 0;
    roboModeEffectRelease();
    battleActorPostResetEvents(actor);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA4488);

extern s32 gBattleResultEventOwner;
extern s32 gBattleResultEventArgument;
s32 func_00193730(s32 *, s32 *, s32, s32 *, s32);
s32 battleResultActivate(s32);

void battleActorFinishEntry(BattleActor *actor) {
    BattleUnitSide *unitSide;
    BattleSlot *childActorA;
    BattleSlot *childActorB;
    BattleSlot *unitActor;

    unitSide = actor->unit_side;
    unitActor = 0;
    if (unitSide != 0) {
        unitActor = unitSide->unitActor;
    }
    childActorA = actor->childActorA;
    unitActor->unmodeled_518 = actor->unmodeled_2dc;
    if (childActorA != 0) {
        childActorA->sefActorFlags |= 4;
    }
    childActorB = actor->childActorB;
    if (childActorB != 0) {
        childActorB->sefActorFlags |= 4;
    }
    if (actor->resultCode == 0) {
        func_00193730(&gBattleResultEventOwner, &gBattleSourceFile, 0x4DB, &gBattleResultEventArgument, actor->messageParam);
    }
    battleResultActivate(actor->resultCode);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA46F0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA4780);

s32 sefReleaseGroupMatches(BattleSlot *, s32);

void battleActorPostEvent(BattleActor *actor, s32 category, s32 event) {
    BattleSlot *unitActor;
    BattleUnitSide *unitSide;

    unitSide = actor->unit_side;
    unitActor = 0;
    if (unitSide != 0) {
        unitActor = unitSide->unitActor;
    }
    sefReleaseGroupMatches(unitActor, (category << 0xC) | event);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu071", func_00AA4890);

void battleActorPostEvent(BattleActor *actor, s32 category, s32 event);

void battleActorPostResetEvents(BattleActor *actor) {
    battleActorPostEvent(actor, 0x136, 0x1C);
    battleActorPostEvent(actor, 0x136, 0x1D);
    battleActorPostEvent(actor, 0x136, 0x1A);
}

void battleActorPostStartEvent(BattleActor *actor) {
    battleActorPostEvent(actor, 0x136, 0x1A);
}

void battleActorPostEndEvent(BattleActor *actor) {
    battleActorPostEvent(actor, 0x136, 0x1C);
}

s32 unitSoundSlotStart(BattleUnitSide *);

void battleActorClearSideRecord(BattleActor *actor) {
    unitSoundSlotStart(actor->unit_side);
}

s32 unitSoundStartRecord(BattleUnitSide *, BattleCommand *);

void battleActorHandleCommand(BattleActor *actor, BattleCommand *command) {
    extern s32 seStartDefaultVoice();
    if (command->unmodeled_1C == 0x62) {
        seStartDefaultVoice(0x25);
        return;
    }
    unitSoundStartRecord(actor->unit_side, command);
}

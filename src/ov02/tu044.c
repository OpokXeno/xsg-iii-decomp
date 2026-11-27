/*
 * OV02 original TU 44: 0x00a8fb00..0x00a914d0 (36 functions)
 */
#include "common.h"
#include "main/tu176.h"

/* Cursor of a command queue; the slot index sits at +0x04. */
typedef struct MonsQueueCursor {
    u8 unk_00[4];
    s32 queueSlotIndex;
} MonsQueueCursor;

/* An actor of the monster set; the sef handle sits at +0x2B4. */
typedef struct MonsActor {
    u8 unk_00[0x2B4];
    s32 sefActorHandle;
} MonsActor;

/* Template data of a monster slot (the pointer at +0x2B0 of MonsActorSlot). */
typedef struct MonsTemplate {
    u8 unk_00[0x1A];
    u8 field_1A;
    u8 unk_1B[0x46 - 0x1B];
    s16 templateId;
    u8 unk_48[0x4E - 0x48];
    s16 field_4E;
} MonsTemplate;

/* Catalogue entry returned by ov02TemplateEntryGet (u8 at +0x1A). */
typedef struct MonsCatalogEntry {
    u8 unk_00[0x1A];
    s8 field_1A;
} MonsCatalogEntry;

/* Spawn entry returned by ov02ActionEntryGet (template id at +0x26). */
typedef struct MonsSpawnEntry {
    u8 unk_00[0x26];
    s16 templateId;
} MonsSpawnEntry;

/* A monster slot record: its actor, its template and its actor handle. */
typedef struct MonsActorSlot {
    MonsActor *actor;                 /* 0x00 */
    u8 unk_04[4];
    s32 unk_08;                       /* 0x08 */
    u8 unk_0C[0x2B0 - 0x0C];
    MonsTemplate *templateData;       /* 0x2B0 */
    u8 unk_2B4[0x2C4 - 0x2B4];
    s32 actorHandle;                 /* 0x2C4 */
} MonsActorSlot;

/* Argument of monsActorTickCallback: the handle at +0x04 and the state passed by address at +0x08. */
typedef struct MonsActorTickArg {
    u8 unk_00[4];
    s32 actorHandle;
    u8 queueState[0x18];
} MonsActorTickArg;

/* One 0x18-byte entry of the table at monsCommandSlots (indexed by the queue slot). */
typedef struct MonsCommandSlot {
    u8 unk_00[0x18];
} MonsCommandSlot;

/* One 0x1C-byte entry of the table at monsSecondaryCommandSlots (indexed by the queue slot). */
typedef struct MonsSecondaryCommandSlot {
    u8 unk_00[0x1C];
} MonsSecondaryCommandSlot;

/* Session state at gBattleSessionState (battleRecordBlockGet returns it); the word at +0x428 is read by monsActionReady. */
typedef struct MonsSessionState {
    u8 unk_000[0x428];
    s32 unk_428;
} MonsSessionState;

extern MonsQueueCursor monsCommandQueueCursor;
extern s32 monsCommandQueueStorage;
extern s32 monsSecondaryCommandQueueCursor;
extern s32 monsSecondaryCommandQueueStorage;
void *commandQueueInit(void *queue, void *storage, s32 count);
s32 memset(void *dest, s32 value, s32 size);
void *commandQueueNext();

void monsCommandQueuesInit(void) {
    extern MonsCommandSlot monsCommandSlots[];
    extern MonsSecondaryCommandSlot monsSecondaryCommandSlots[];
    memset(&monsCommandQueueCursor, 0, 0x20);
    memset(&monsCommandQueueStorage, 0, 0x500);
    memset(&monsCommandSlots, 0, 0x600);
    commandQueueInit(&monsCommandQueueCursor, &monsCommandQueueStorage, 0x40);
    memset(&monsSecondaryCommandQueueCursor, 0, 0x20);
    memset(&monsSecondaryCommandQueueStorage, 0, 0x3C0);
    memset(&monsSecondaryCommandSlots, 0, 0x540);
    commandQueueInit(&monsSecondaryCommandQueueCursor, &monsSecondaryCommandQueueStorage, 0x30);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu044", func_00A8FBC8);

MonsCommandSlot *monsCommandSlotFor(MonsQueueCursor *cursor) {
    extern MonsCommandSlot monsCommandSlots[];
    MonsCommandSlot *commandSlot;

    commandSlot = commandQueueNext(cursor);
    if (commandSlot != 0) {
        commandSlot = &monsCommandSlots[cursor->queueSlotIndex];
    }
    return commandSlot;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu044", func_00A8FCB8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu044", func_00A8FD08);

MonsSecondaryCommandSlot *monsSecondaryEntryFor(MonsQueueCursor *cursor) {
    extern MonsSecondaryCommandSlot monsSecondaryCommandSlots[];
    MonsSecondaryCommandSlot *secondaryCommandSlot;

    secondaryCommandSlot = commandQueueNext(cursor);
    if (secondaryCommandSlot != 0) {
        secondaryCommandSlot = &monsSecondaryCommandSlots[cursor->queueSlotIndex];
    }
    return secondaryCommandSlot;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu044", func_00A8FE08);

INCLUDE_ASM("asm/nonmatchings/ov02/tu044", func_00A8FE58);

MonsCommandSlot *monsCommandSlotFor(MonsQueueCursor *cursor);

s32 monsSlotQueueEmpty(void) {
    return monsCommandSlotFor(&monsCommandQueueCursor) == 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu044", func_00A8FED0);

void monsLinkActorToTarget(s32 actorEntryId, s32 targetActorHandle) {
    s32 actorSlotIndex;
    s32 targetActorSlotIndex;

    actorSlotIndex = battleRosterEntryIdIndexGet(actorEntryId);
    targetActorSlotIndex = battleRosterEntryIdIndexGet(targetActorHandle);
    if ((actorSlotIndex != -1) && (targetActorSlotIndex != -1)) {
        battleRosterEntrySwap(actorSlotIndex, targetActorSlotIndex, 0);
    }
}

s32 scratchpadClearSendCmd29();
s32 battleSetPendingFlag(s32);
s32 seVoiceStartDefaultCall(s32);

void monsSessionBegin(void) {
    scratchpadClearSendCmd29();
    seVoiceStartDefaultCall(0x23);
    battleSetPendingFlag(1);
}

s32 slotStateActive();
s32 slotStateInactive();

s32 monsSessionTryEnd(void) {
    if ((slotStateActive() == 0) && (slotStateInactive() != 0)) {
        battleSetPendingFlag(0);
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu044", func_00A90290);

INCLUDE_ASM("asm/nonmatchings/ov02/tu044", func_00A90480);

INCLUDE_ASM("asm/nonmatchings/ov02/tu044", func_00A904F8);

s32 func_00A94C68();

void monsSessionStep(void) {
    func_00A94C68();
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu044", func_00A90568);

INCLUDE_ASM("asm/nonmatchings/ov02/tu044", func_00A90708);

INCLUDE_ASM("asm/nonmatchings/ov02/tu044", func_00A90A18);

INCLUDE_ASM("asm/nonmatchings/ov02/tu044", func_00A90B08);

INCLUDE_ASM("asm/nonmatchings/ov02/tu044", func_00A90EC8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu044", func_00A90F40);

s32 monsSlotShowActor();
s32 monsSlotHideActor();
s32 func_00A93610();
s32 monsActiveSlotsGet(s32 **slots);

void monsApplyActorFlags(MonsActorSlot *monsterSlot) {
    extern s32 sefActorFlagsGet();
    s32 *activeSlots;
    s32 actorFlags;

    monsActiveSlotsGet(&activeSlots);
    actorFlags = sefActorFlagsGet(monsterSlot->actor->sefActorHandle);
    if (actorFlags & 0x20) {
        func_00A93610(monsterSlot->actor, monsterSlot->unk_08);
    }
    if (actorFlags & 0x80) {
        monsSlotShowActor(activeSlots[0]);
    }
    if (actorFlags & 0x100) {
        monsSlotHideActor(activeSlots[0]);
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu044", func_00A910C8);

s32 monsActorHasSefFlagOne(s32 unusedValue, MonsActor **actorRef)
{
    extern s32 sefActorFlagsGet(s32);
  if ((sefActorFlagsGet((*actorRef)->sefActorHandle) & 1) != 0)
  {
    return 1;
  }
  return 0;
}

s32 sefGroupHasMatch();
s32 *battleRecordBlockGet(void);

s32 monsActionReady(MonsActorSlot *monsterSlot) {
    extern s32 sefActorFlagsGet();
    s32 sefFlagBits;
    MonsSessionState *sessionState;

    sefFlagBits = sefActorFlagsGet(monsterSlot->actor->sefActorHandle) & 0x08000000;
    if (sefFlagBits != 0) {
        return 1;
    }
    sessionState = battleRecordBlockGet();
    return sefGroupHasMatch(monsterSlot->actor->sefActorHandle, sessionState->unk_428) == 0;
}

s32 monsActorSefFlagBit28(s32 unusedValue, MonsActor **actorRef)
{
    extern s32 sefActorFlagsGet(s32);
  if ((sefActorFlagsGet((*actorRef)->sefActorHandle) & 0x10000000) != 0)
  {
    return 1;
  }
  return 0;
}

s32 func_0028D670();
MonsSpawnEntry *ov02ActionEntryGet(s32);
MonsCatalogEntry *ov02TemplateEntryGet(s16);

void monsAttachActor(MonsActorSlot *monsterSlot, s32 spawnId) {
    s32 templateId;

    templateId = ov02ActionEntryGet(spawnId)->templateId;
    if (templateId != 0) {
        monsterSlot->actorHandle = func_0028D670();
        monsterSlot->templateData->templateId = templateId;
        monsterSlot->templateData->field_4E = ov02TemplateEntryGet(templateId)->field_1A;
    }
}

void XenoActorRelease(s32);

void monsDetachActor(MonsActorSlot *monsterSlot) {
    s32 actorHandle;

    actorHandle = monsterSlot->actorHandle;
    if (actorHandle != 0) {
        XenoActorRelease(actorHandle);
        monsterSlot->actorHandle = 0;
        monsterSlot->templateData->templateId = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu044", func_00A91308);

INCLUDE_ASM("asm/nonmatchings/ov02/tu044", func_00A91360);

INCLUDE_ASM("asm/nonmatchings/ov02/tu044", func_00A913B0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu044", func_00A91400);

s32 monsAlwaysReady(void) {
    return 1;
}

s32 unitCommandQueueClear(s32, void *);

s32 monsActorTickCallback(s32 unusedValue, MonsActorTickArg *actorTickArg) {
    unitCommandQueueClear(actorTickArg->actorHandle, actorTickArg->queueState);
    return 1;
}

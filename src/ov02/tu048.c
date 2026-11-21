/*
 * OV02 original TU 48: 0x00a92c88..0x00a936b8 (28 functions)
 */
#include "common.h"

typedef int s32;
#include "ov02/tu060.h"

/* 8-byte sound table record: the id is the only word this TU reads. */
typedef struct SoundSlotEntry {
    s32 id;                             /* +0x00 */
    unsigned char unmodeled_04[4];      /* +0x04 */
} SoundSlotEntry;

/* Sound driver state block (0x40 bytes), cleared by soundSlotSystemInit. */
typedef struct SoundDriverState {
    unsigned char unmodeled_00[0x30];
    s32 currentSoundId;                 /* +0x30 */
    s32 currentValue;                   /* +0x34 */
    unsigned char unmodeled_38[0x8];
} SoundDriverState;

/* The sound slot halfword of a unit pool entry at +0xDA (past what tu060.h claims). */
typedef struct UnitPoolSoundSlot {
    unsigned char unmodeled_00[0xDA];
    short soundSlot;                    /* +0xDA */
} UnitPoolSoundSlot;
#define unitPoolSoundSlot(unit) (((UnitPoolSoundSlot *) (unit)->lookupKeyRecord)->soundSlot)

/* Sound request record read by unitSoundStartRecord. */
typedef struct SoundRequest {
    unsigned char unmodeled_00[7];
    unsigned char playMode;             /* +0x07 */
    unsigned char unmodeled_08[0x14];
    signed char number;                 /* +0x1C */
} SoundRequest;

extern s32 gSoundSlotHandles[8];
extern SoundDriverState gSoundDriverState;
extern s32 gCurrentSoundSlot;
extern s32 D_00AC340C;
extern SoundSlotEntry gSoundCurrentSlots[];
extern SoundSlotEntry gSoundHandleSlots[];
extern SoundSlotEntry gSoundVoiceSlots[];
extern s32 gCurrentSoundId;
extern s32 gSoundStartFlag;
extern s32 gSoundAlternateToggle;
void func_0018EC68(s32 soundId);

void soundSlotTableClear(void) {
    extern void *memset(void *, int, unsigned int);
    memset(&gSoundSlotHandles, 0, 0x20);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A92CB0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A92D48);

void soundSlotHandleClear(s32 soundSlot) {
    if ((soundSlot != -1) && (soundSlot < 8)) {
        gSoundSlotHandles[soundSlot] = 0;
    }
}

void soundSlotVoiceStop(s32 soundSlot, s32 idOffset);
void soundSlotStop(s32 index);
void soundSlotSetDriverParam(s32 index, s32 value);
void soundDriverSetEnable(s32 unusedArg, s32 flag);

void soundSlotSystemInit(void) {
    extern void *memset(void *, int, unsigned int);
    memset(&gSoundDriverState, 0, 0x40);
    gCurrentSoundSlot = 0;
    D_00AC340C = 0;
    soundSlotStop(1);
    soundSlotVoiceStop(6, 1);
    soundDriverSetEnable(0, 0);
    soundSlotSetDriverParam(1, 1);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A92ED8);

s32 func_0018EBE8();

s32 soundSlotCurrentIdle(void) {
    return func_0018EBE8(gSoundCurrentSlots[gCurrentSoundSlot].id) == 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A92FC8);

s32 func_00A92D48();

void unitSoundSlotAssign(BattleUnit *battleUnit, s32 soundKind) {
    s32 soundSlot;

    soundSlot = func_00A92D48(battleUnit->lookupKeyRecord->lookupKey, soundKind);
    if (soundSlot != -1) {
        unitPoolSoundSlot(battleUnit) = soundSlot;
    }
}

void soundSlotHandleClear(s32 soundSlot);

void unitSoundSlotRelease(BattleUnit *battleUnit) {
    s32 soundSlot;

    soundSlot = unitPoolSoundSlot(battleUnit);
    func_0018EC68(gSoundHandleSlots[soundSlot].id);
    soundSlotHandleClear(soundSlot);
}

void soundSlotStateSet(s32 currentValue) {
    SoundDriverState *driverState = &gSoundDriverState;

    driverState->currentSoundId = gCurrentSoundId;
    driverState->currentValue = currentValue;
}

void soundSlotReleaseCurrent(void) {
    soundSlotVoiceStop(6, 1);
    func_0018EC68(gCurrentSoundId);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A93148);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A93170);

void seVoiceStartDefault();

void seVoiceStartDefaultCall(void) {
    seVoiceStartDefault();
}

s32 seVoiceStartDriver(s32 handle);

void soundSlotVoiceStart(s32 soundSlot, s32 idOffset) {
    if (soundSlot != -1) {
        if (soundSlot < 8) {
            seVoiceStartDriver(gSoundVoiceSlots[soundSlot].id + idOffset - 1);
        }
    }
}

s32 func_0018F098(s32 handle);

void soundSlotVoiceStop(s32 soundSlot, s32 idOffset) {
    if (soundSlot != -1) {
        if (soundSlot < 8) {
            func_0018F098(gSoundVoiceSlots[soundSlot].id + idOffset - 1);
        }
    }
}

void soundSlotVoiceStart(s32 soundSlot, s32 idOffset);

void unitSoundSlotStart(BattleUnit *battleUnit, s32 number) {
    u32 numberIndex;

    numberIndex = number - 1;
    if (numberIndex < 0x11U) {
        soundSlotVoiceStart(unitPoolSoundSlot(battleUnit), number);
    }
}

void unitSoundSlotStart(BattleUnit *battleUnit, s32 number);

void unitSoundStartRecord(BattleUnit *battleUnit, SoundRequest *soundRequest) {
    s32 playMode;

    playMode = soundRequest->playMode;
    if (playMode < 2U) {
        unitSoundSlotStart(battleUnit, soundRequest->number);
        return;
    }
    soundSlotVoiceStart(6, 1);
}

void soundSlotStartIfFlag(void) {
    if (gSoundStartFlag != 0) {
        soundSlotVoiceStart(7, 1);
    }
}

void soundSlotStartNumbered(s32 index, s32 number);

void soundSlotStartPrimary(s32 soundId) {
    soundSlotStartNumbered(1, soundId);
}

s32 soundSlotStatus(s32 slot);

s32 soundSlotPrimaryActive(void) {
    u32 primarySlotStatus;

    primarySlotStatus = soundSlotStatus(1) - 1;
    return primarySlotStatus < 4U;
}

#include "ov02/tu060.h"


extern char *gSoundNamePrefix;
extern char *gSoundNameKeyTable[];
extern char *gSoundNameKindTable[];
extern char D_00AB8C48[];
extern char D_00AB8C50[];
extern char gSoundNameSuffixCommon[];
extern char gSoundNameSuffixHighRoll[];
extern char gSoundNameBuffer[];
char *strcpy(char *dst, const char *src);
char *func_0030E3D0(char *dst, const char *src);
int func_00A86AC0(void);
int func_00A818D0(BattleUnit *unit);
int rnd(int range);

char *unitSoundNameGet(BattleUnit *battleUnit, int soundKind)
{
    char *suffix;
    short key;
    int roll;
    u32 zeroBasedKey;
    u32 zeroBasedKind;

    key = battleUnit->lookupKeyRecord->lookupKey;
    zeroBasedKey = key - 1;
    if (zeroBasedKey >= 10) {
        return 0;
    }
    zeroBasedKind = soundKind - 1;
    if (zeroBasedKind >= 14) {
        return 0;
    }
    strcpy(gSoundNameBuffer, gSoundNamePrefix);
    func_0030E3D0(gSoundNameBuffer, gSoundNameKeyTable[key]);
    if (key == 5) {
        if (func_00A86AC0() != 0) {
            suffix = D_00AB8C48;
        } else {
            suffix = D_00AB8C50;
        }
        func_0030E3D0(gSoundNameBuffer, suffix);
    }
    func_0030E3D0(gSoundNameBuffer, gSoundNameSuffixCommon);
    func_0030E3D0(gSoundNameBuffer, gSoundNameKindTable[soundKind]);
    if (soundKind == 1) {
        roll = rnd(100);
        if (roll >= 81) {
            suffix = gSoundNameSuffixHighRoll;
        } else if (roll >= 51) {
            suffix = D_00AB8C48;
        } else {
            suffix = D_00AB8C50;
        }
        func_0030E3D0(gSoundNameBuffer, suffix);
    } else if (soundKind == 2) {
        if (func_00A818D0(battleUnit) != 0) {
            suffix = D_00AB8C48;
        } else {
            suffix = D_00AB8C50;
        }
        func_0030E3D0(gSoundNameBuffer, suffix);
    }
    return gSoundNameBuffer;
}

void soundSlotStartPrimary(s32 soundId);

void unitSoundPlay(BattleUnit *battleUnit, s32 soundKind) {
    char *text;

    text = unitSoundNameGet(battleUnit, soundKind);
    if (text != 0) {
        soundSlotStartPrimary((s32) text);
    }
}

void unitSoundPlay(BattleUnit *battleUnit, s32 soundKind);
s32 unitTblGet(s32 side, BattleUnit ***table);

void unitSoundPlayRandom(s32 soundKind)
{
    s32 sideUnitCount;
    s32 *sideUnitCountPtr;
    BattleUnit **sideUnits;
    BattleUnit *pickedUnit;

    sideUnitCount = unitTblGet(0, &sideUnits);
    do
    {
        do
        {
            pickedUnit = sideUnits[rnd((*(sideUnitCountPtr = &sideUnitCount)) - 1)];
        }
        while (pickedUnit == 0);
    }
    while (pickedUnit->targetingFlags & 0x8000);
    unitSoundPlay(pickedUnit, soundKind);
}

void soundToggleReset(void) {
    gSoundAlternateToggle = 0;
}

s32 soundSlotAlternateStart(void) {
    s32 nextToggle;

    soundSlotVoiceStart(6, gSoundAlternateToggle + 2);
    nextToggle = (gSoundAlternateToggle + 1) & 1;
    gSoundAlternateToggle = nextToggle;
    return nextToggle;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A93610);

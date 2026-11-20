#ifndef INCLUDE_MAIN_RES_GET_PATH_H
#define INCLUDE_MAIN_RES_GET_PATH_H

/* Types owned by main/tu184 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"
#include "ov02/tu060.h"

/* The command queue item every handler of unitCommandTable receives (the queue copies its first 0x18 bytes; callers build it as a 0x20-byte local). */
typedef struct Ov01Command Ov01Command;

/*
 * Slide block: the 0x80-byte subobject the slide steps work on. The callers pass it by address
 * (the menu panels at +0x10, the ov01 unit pose at +0x150, the unit_cmd.c record pairLow at +0x150).
 * position moves toward goal by velocity each frame (objStdMove also accelerates it); angle moves toward goalAngle by angleRate.
 * objStdInit clears it and sets position.w and the four scale values to 1.0.
 */
typedef struct SlideBlock SlideBlock;

typedef struct StatBlock StatBlock;

/* The command queue item every handler of unitCommandTable receives (the queue copies its first 0x18 bytes; callers build it as a 0x20-byte local). */
struct Ov01Command {
    int command; /* +0x0: the table index unitRunCommands dispatches on; the move dispatchers store the command they run */
    int state; /* +0x4: zero until the one-shot start has run */
    int code; /* +0x8: the key, id or motion code the command carries */
    union {
        int wait; /* the wait flag of the mode commands */
        BattleUnit *owner; /* compared with the object by unitCmdPlayVoice */
    } arg; /* +0xC */
    BattleUnit *faceTarget; /* +0x10 */
    int poseIndex; /* +0x14: the index unitPoseRamp takes (also the goal-key flag unit_cmd.c reads) */
    unsigned char unmodeled_18[0x8];
};

/*
 * Slide block: the 0x80-byte subobject the slide steps work on. The callers pass it by address
 * (the menu panels at +0x10, the ov01 unit pose at +0x150, the unit_cmd.c record pairLow at +0x150).
 * position moves toward goal by velocity each frame (objStdMove also accelerates it); angle moves toward goalAngle by angleRate.
 * objStdInit clears it and sets position.w and the four scale values to 1.0.
 */
struct SlideBlock {
    DoublewordAlignedVector4 position;  /* 0x00: x, y, z of the moved point; w is 1.0 after objStdInit */
    u8 unmodeled_10[0x14];
    f32 angle;                          /* 0x24: current angle, radians */
    u8 unmodeled_28[0x8];
    Vector4 scale;                      /* 0x30: all four values 1.0 after objStdInit */
    Vector4 velocity;                   /* 0x40: per-frame step, written by objHatoVec, advanced by objStdMove (w unused) */
    Vector4 acceleration;               /* 0x50: added to velocity by objStdMove (w unused) */
    DoublewordAlignedVector4 goal;      /* 0x60: destination; x and z are used by slideLimitGoalXZ */
    f32 goalAngle;                      /* 0x70: target angle */
    f32 angleRate;                      /* 0x74: radians per call, read by slideAngleStep */
    u8 unmodeled_78[0x8];
};

/*
 * The 0x108-byte battle block of a unit (its BattleUnit points at it through +0x144). The same
 * record fills the entry pools battleEntryPoolA and battleEntryPoolB and the slot table
 * battleSlotTable: calcPara2OrgSub copies hp and ep of a unit into its slot, calcCopyParaMake
 * copies a whole slot into a unit. Only the members the functions of this file touch are named.
 */
struct StatBlock {
    unsigned char unmodeled_00[0x2];        /* +0x00..+0x01 */
    unsigned char maxHp[4];           /* +0x02: the HP cap, a signed word read unaligned (calcHp clamps HP to it; battleEntryLoad starts HP at it) */
    short maxEp;                            /* +0x06: EP cap read by calcEp */
    short physAttack;                       /* +0x08: physical attack, calcAtkPhy */
    short physDefence;                      /* +0x0a: physical defence, calcDefPhy */
    short magAttack;                        /* +0x0c: magic attack, calcAtkMag */
    short magDefence;                       /* +0x0e: magic defence, calcDefMag */
    signed char hitRate;                    /* +0x10: base of calcHitParaGet */
    signed char evadeRate;                  /* +0x11: base of calcEvadeRate */
    union {
        unsigned char counter12;            /* bumped by calcSkillInit, capped at 15 */
        signed char valueBias;              /* the same byte as the bias calcWct subtracts from valueLevel */
    };                                      /* +0x12 */
    signed char expLevel;                   /* +0x13: the unit experience table entry (dataPlUnitInit); calcLvUp steps it up, unitGrowthStep divides by 100 - expLevel */
    signed char state;                      /* +0x14: at 4 or more the pose turn is damped */
    signed char subState;                   /* +0x15: the move kind the move dispatchers switch on; compared with 4 by unitSetPhase for phase 5 */
    signed char chanceLevel;                /* +0x16: the percent actLevelChanceRoll rolls, capped at 100 */
    signed char inflictBase;                /* +0x17: chance base read by calcStatInflictRate; calcCopyParaMake fills it */
    signed char defaultCode;                /* +0x18: unitStateFlagGet's fallback code */
    unsigned char unmodeled_19[0x1A - 0x19]; /* +0x19 */
    short slidePercent;                     /* +0x1A: signed percent unitDefArea divides by 100 */
    unsigned short flags;                   /* +0x1C: the command handlers read the sequence bits 0x80 and 0x2000; low two bits pick the modeGuard byte, bits 0x10, 0x20, 0x40, 0x800 and 0x4000 are tested */
    short sequenceIndex;                    /* +0x1E: the low half of the effect key */
    signed char liveRows[4];                /* +0x20: nonzero for each live row calcHitRowGate walks */
    signed char slotValue[8];               /* +0x24: per status bit, read by calcDmg's status loop */
    signed char slotRoll[8];                /* +0x2C: percent rolled for each status bit by calcDmg */
    signed char effectCode[4];              /* +0x34: four effect codes, zero when empty (calcSkillEquipOrg writes them) */
    unsigned char unmodeled_38[0x52 - 0x38]; /* +0x38..+0x51 */
    unsigned short altMask[5];              /* +0x52: calcStatProtectSet/Reset use indices 3 and 4 */
    int hp;                                 /* +0x5C: HP (calcHp); calcPara2OrgSub copies it into the slot */
    short ep;                               /* +0x60: EP read by calcEp; calcPara2OrgSub copies it into the slot */
    union {
        short key;                          /* the unit's key: below 16 a party slot (calcTrg2PE's side split), 16 to 140 pool B; 0 marks a free pool entry */
        unsigned short keyUnsigned;         /* the same halfword as the unsigned value the data panels range-test (battle_init.c) */
    };                                      /* +0x62 */
    short valueLevel;                       /* +0x64: the level calcWct scales */
    short unitValue;                        /* +0x66: calcWct's value, lowered by calcWctDec */
    short unitValueSaved;                   /* +0x68: the value unitValueReset saves */
    short posIndex;                         /* +0x6A: the effectPosGet row of the unit */
    float facing;                           /* +0x6C: the unit's facing angle, read by unitIsInFacingArc */
    signed char state70;                    /* +0x70: a signed byte, nonzero while calcHitRowGate is in its battle state */
    signed char level;                      /* +0x71: 0..3 (unitLevelAdd), compared with a record's required level */
    unsigned char effectFlag72;             /* +0x72: set to 4 by calcSkillInit */
    union {
        unsigned char dirty;                /* cleared by calcTurnEnd */
        signed char keysSpent;              /* the same byte as the counter of keys the data panels spend (battle_init.c) */
    };                                      /* +0x73 */
    union {
        short pairFirst[3];                 /* +0x74: first halfword of each pair (ShortPair.first), cleared by battleEntryActivate */
        unsigned short pairFirstUnsigned[3]; /* the same halfwords as the unsigned kinds unitBindActors range-tests */
    };
    unsigned short pairSecond[3];           /* +0x7A: the second halfword of each pair dataPlUnitInit loads next to pairFirst */
    short rollPercent;                      /* +0x80: percent 0..100 (unitPercentAdd), rolled by calcStatusHitRoll */
    short current;                          /* +0x82: current value, capped at currentMax (unitCurrentAdd) */
    short currentMax;                       /* +0x84 */
    short equipId[4];                       /* +0x86: four equipment ids, zero when empty (calcAccEquipOrg writes them) */
    short linkedSlots[2];                   /* +0x8E: slots 11 to 15 only (battleSlotLinkedSlotSet); [0] is the slot calcCopyParaMake reads, [1] the sub pilot id (0..0xA, configSubPilot) */
    unsigned short activeMask[4];           /* +0x92: one value per row; set bits are active statuses */
    unsigned char turnCount[4][16];         /* +0x9a: per row and status bit, turns left */
    signed char entryKind[8];               /* +0xDA: unitEntryKindAdd's entry table, ANDed with liveRows by calcHitRowGate */
    signed char entryMark[8];               /* +0xE2: written by calcHitRowGate, read by unitEntriesAllMarked */
    int entryCount;                         /* +0xEC: entries held, at most 8 */
    int comboMask;                          /* +0xF0: status mask of the current combo (calcComboUpdate) */
    int comboHits;                          /* +0xF4: hit count of the current combo, capped at 99 */
    int spawnArgC;                          /* +0xF8: the spawn record's spawnArgC (scene_load.c), unitCreate arg 4 */
    int spawnArgD;                          /* +0xFC: the spawn record's spawnArgD (scene_load.c), unitCreate arg 5 */
    BattleUnit *source;                      /* +0x100: the unit whose effect sum monsGainSet adds; unitFaceTarget's target argument */
    short sourceAction;                     /* +0x104: the action index of source (unitJobTypeAtk) */
    short handle;                           /* +0x106: passed to handleFree by unitCmdRelease */
};


/* The encoder request of xglJpegEncode: the GS frame region (frameBufferBase, frameBufferWidth, origin, width, height),
 * the output buffer and the size the encoder writes back. Quality and sampling are single bytes. */
typedef struct JpegEncodeRequest {
    unsigned char unmodeled_00[4];
    unsigned char *outputBuffer;        /* +0x04 */
    int outputSize;                     /* +0x08 */
    short frameBufferBase;              /* +0x0C */
    short frameBufferWidth;             /* +0x0E */
    short originX;                      /* +0x10 */
    short originY;                      /* +0x12 */
    short width;                        /* +0x14 */
    short height;                       /* +0x16 */
    unsigned char luminanceQuality;     /* +0x18 */
    unsigned char chrominanceQuality;   /* +0x19 */
    unsigned char samplingMode;         /* +0x1A */
    unsigned char unmodeled_1b;
    unsigned char unmodeled_1c[4];
} JpegEncodeRequest;

#endif

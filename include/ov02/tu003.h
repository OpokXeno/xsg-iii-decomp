#ifndef INCLUDE_OV02_TU003_H
#define INCLUDE_OV02_TU003_H

typedef int s32;

typedef short s16;

/* Link record of a unit: its handler kind byte (+8) selects the handler. */
typedef struct BattleUnitLink {
    unsigned char unk_00[8];
    signed char battleHandlerKind;
} BattleUnitLink;

/* Per-unit stats block at BattleUnit +0x2B0. */
typedef struct BattleUnitStats {
    s32 unk_00;
    unsigned short unk_04;
    unsigned char unk_06[2];
    s16 bonusForKindZero;
    unsigned char unk_0A[2];
    s16 bonusForOtherKinds;
    unsigned char unk_0E[5];
    signed char unk_13;
} BattleUnitStats;

#endif /* INCLUDE_OV02_TU003_H */

#ifndef INCLUDE_OV02_TU075_H
#define INCLUDE_OV02_TU075_H

typedef unsigned char u8;

typedef int s32;

/* The battle actor. The unit record at +0xA0 is its owner (BattleActor.unit_side in ov02/tu071). */
struct BattleActor {
    u8 unk_00[0xA0];
    BattleUnit *owner;                      /* +0xA0 */
    u8 unk_A4[0xE4 - 0xA4];
    s32 refreshValue;                       /* +0xE4: written by battleActorStoreAndRefresh */
};

#endif /* INCLUDE_OV02_TU075_H */

#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu074", func_001C2F78);

typedef signed char s8;
typedef short s16;
typedef int s32;
typedef float f32;
typedef unsigned char u8;
s32 func_002A6FA8();
void func_002A70E8(u8 *collisionObject);

/* Sub-block at ActorObject.displayState (+0x24): the 0x6A0 state block, the 0x6AA flag, the 0x6B3 mode
 * byte, the stored result at 0x6BC and the scale copied from baseScale at 0x6CC. */
typedef struct DisplayModelState {
    unsigned char unmodeled_000[0x1C];
    f32 baseScale;              /* 0x1C */
    unsigned char unmodeled_020[0x6A0 - 0x20];
    unsigned char collisionObject[0xA]; /* 0x6A0 */
    s16 stateFlag;              /* 0x6AA */
    unsigned char unmodeled_6AC[0x6B3 - 0x6AC];
    s8 mode;                    /* 0x6B3 */
    unsigned char unmodeled_6B4[0x6BC - 0x6B4];
    s32 resultId;               /* 0x6BC */
    unsigned char unmodeled_6C0[0x6CC - 0x6C0];
    f32 scale;                  /* 0x6CC */
} DisplayModelState;

/* Object record: +0x00 actorFlags, +0x1C table, +0x24 sub-block pointer, +0x2C id, +0x154 pending count. */
typedef struct ActorObject {
    s32 actorFlags;                  /* 0x00 */
    unsigned char unmodeled_04[0x1C - 0x04];
    u8 *table;                  /* 0x1C */
    unsigned char unmodeled_20[0x24 - 0x20];
    DisplayModelState *displayState;    /* 0x24 */
    unsigned char unmodeled_28[0x2C - 0x28];
    s32 id;                     /* 0x2C */
    unsigned char unmodeled_30[0x154 - 0x30];
    s32 pending;                /* 0x154 */
} ActorObject;

void actorSetId(ActorObject *actor, s32 id) {
    f32 baseScale;
    s32 resultId;
    DisplayModelState *displayState;

    actor->id = id;
    displayState = actor->displayState;
    if (displayState != 0) {
        func_002A70E8(displayState->collisionObject);
        displayState->mode = 3;
        displayState->stateFlag = 0;
        resultId = func_002A6FA8(0x100, actor->id);
        baseScale = displayState->baseScale;
        displayState->scale = baseScale;
        displayState->resultId = resultId;
    }
}

extern unsigned char func_001C33D8[];

void actorInit(ActorObject *actor) {
    s32 newActorFlags;

    newActorFlags = (actor->actorFlags & ~8) | 0x11000;
    actor->actorFlags = newActorFlags;
    actor->pending = 0;
    actor->table = func_001C33D8;
}

extern unsigned char gIndexedTableDefault[];
extern u8 *gIndexedTableFallback;
u8 *func_00A85208(s32 index);

u8 *indexedTableGet(s32 tableIndex) {
    u8 *selectedTable;

    if (tableIndex != 0xA) {
        selectedTable = func_00A85208(tableIndex + 1);
    } else {
        selectedTable = gIndexedTableDefault;
    }
    return (selectedTable != 0) ? selectedTable : gIndexedTableFallback;
}

s32 func_00A848C8(s32);

s32 indexedCountGet(s32 tableIndex) {
    if (tableIndex != 0xA) {
        return func_00A848C8(tableIndex + 1) - 1;
    }
    return 0x11;
}

extern s32 gIndexedValueFallback;
s32 func_00A85028(s32);

s32 indexedValueGet(s32 valueIndex) {
    s32 lookedUpValue;

    lookedUpValue = func_00A85028(valueIndex + 0x4C);
    return (lookedUpValue == 0) ? gIndexedValueFallback : lookedUpValue;
}

s32 func_001B93C0();

void actorFlushPending(void) {
    func_001B93C0();
}

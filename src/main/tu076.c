#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu076", func_001C3838);

INCLUDE_ASM("asm/main/nonmatchings/tu076", func_001C3928);

INCLUDE_ASM("asm/main/nonmatchings/tu076", func_001C3A40);

typedef signed char s8;
typedef short s16;
typedef int s32;
typedef float f32;
typedef unsigned short u16;

/* The state block of a display model (object +0x24): its collision block at +0x6A0 is initialized by func_002A70E8. */
typedef struct DispModelState {
    unsigned char unmodeled_000[0x1C];
    f32 scale;                          /* +0x1C */
    unsigned char unmodeled_020[0x6A0 - 0x20];
    unsigned char collision[0x6AA - 0x6A0];
    s16 counter6AA;                     /* +0x6AA: cleared to 0 by dispModelBindValue */
    unsigned char unmodeled_6AC[0x6B3 - 0x6AC];
    s8 mode6B3;                         /* +0x6B3: set to 3 by dispModelBindValue */
    unsigned char unmodeled_6B4[0x6BC - 0x6B4];
    s32 allocatedHandle;                         /* +0x6BC: the allocatedHandle allocated by func_002A6FA8 */
    unsigned char unmodeled_6C0[0x6CC - 0x6C0];
    f32 savedScale;                     /* +0x6CC: a copy of scale */
} DispModelState;

/* A display model object (the arg of dispModelBindValue and dispModelDetachEntry), partial. */
typedef struct DispModel {
    unsigned char unmodeled_00[0x24];
    DispModelState *modelState;              /* +0x24 */
    unsigned char unmodeled_28[0x2C - 0x28];
    s32 bindValue;                          /* +0x2C */
    s32 drawHandle;                     /* +0x30: released with func_0027AC90, then set to -1 */
    s32 displayEntryKey;                    /* +0x34: looked up with func_001C0570 */
} DispModel;

/* A display entry looked up by DispModel.entryHandle, partial. */
typedef struct DispModelEntry {
    unsigned char unmodeled_00[0x10];
    s32 kind;                           /* +0x10: 0x200 for the entries dispModelDetachEntry releases */
    unsigned char unmodeled_14[0x150 - 0x14];
    u16 flags;                          /* +0x150: bit 0x10 is tested */
} DispModelEntry;

extern unsigned char gDispModelProbeDelta[];
s32 func_002A6FA8();
void func_002A70E8();
void func_002AA490();

void dispModelBindValue(DispModel *dispModel, s32 bindValue) {
    s32 probePosition[4];
    f32 scale;
    void *collision;
    s32 allocatedHandle;
    DispModelState *modelState;

    dispModel->bindValue = bindValue;
    modelState = dispModel->modelState;
    collision = modelState->collision;
    if (modelState != 0) {
        func_002A70E8(collision);
        modelState->mode6B3 = 3;
        modelState->counter6AA = 0;
        allocatedHandle = func_002A6FA8(0x100, dispModel->bindValue);
        scale = modelState->scale;
        modelState->allocatedHandle = allocatedHandle;
        modelState->savedScale = scale;
        func_002AA490(&probePosition[0], gDispModelProbeDelta, collision);
    }
}

void RgDispModelSetLocal(void *);
DispModelEntry *func_001C0570(s32);
void objectSetDefaults(void *);
void func_0027AC90(s32);

void dispModelDetachEntry(DispModel *dispModel) {
    s32 displayEntryKey;
    DispModelEntry *displayEntry;

    displayEntryKey = dispModel->displayEntryKey;
    if (displayEntryKey != 0) {
        displayEntry = func_001C0570(displayEntryKey);
        if ((displayEntry != 0) && (displayEntry->kind == 0x200) && (displayEntry->flags & 0x10)) {
            func_0027AC90(dispModel->drawHandle);
            dispModel->drawHandle = -1;
            RgDispModelSetLocal(dispModel);
            objectSetDefaults(displayEntry);
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu076", func_001C3DE8);

INCLUDE_ASM("asm/main/nonmatchings/tu076", func_001C3E40);

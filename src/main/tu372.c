#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu372", func_002C13E0);

typedef signed char s8;
typedef unsigned char u8;
typedef int s32;
typedef float f32;
extern u8 gNjPlaneRegistryStorage[];

/* One entry of the njState table: the index fields select a table slot whose value is copied to the out field. */
typedef struct NjSelectEntry {
    u8 unk_00[0x28];
    s32 candidateValues[4];
    s32 firstSelector;
    s32 secondSelector;
    s32 firstSelectedValue;
    s32 secondSelectedValue;
} NjSelectEntry;

/* The nj plane registry at gNjPlaneRegistryStorage. */
typedef struct NjPlaneRegistry {
    u8 unk_00000[0x11670];
    f32 offset_x;
    f32 offset_y;
    NjSelectEntry *currentSelectEntry;
    s32 currentFirstSelector;
    s32 currentSecondSelector;
} NjPlaneRegistry;

#define gNjPlaneRegistry (*(NjPlaneRegistry *) gNjPlaneRegistryStorage)

void njStateBindEntry(NjSelectEntry *selectEntry)
{
    gNjPlaneRegistry.currentFirstSelector = selectEntry->firstSelector;
    gNjPlaneRegistry.currentSelectEntry = selectEntry;
    gNjPlaneRegistry.currentSecondSelector = selectEntry->secondSelector;
    selectEntry->firstSelectedValue = selectEntry->candidateValues[selectEntry->firstSelector];
    selectEntry->secondSelectedValue = selectEntry->candidateValues[selectEntry->secondSelector];
}

extern u8 D_005E2550[];
void *memset(void *, s32, unsigned int);

void njStateClearTable(void) {
    memset(&D_005E2550, 0, 0x20);
}

void njPlaneSetCurrentOffset(f32 x, f32 y)
{
    gNjPlaneRegistry.offset_x = x;
    gNjPlaneRegistry.offset_y = y;
}

void njStateNoOp(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu372", func_002C14E8);

INCLUDE_ASM("asm/main/nonmatchings/tu372", func_002C18A8);

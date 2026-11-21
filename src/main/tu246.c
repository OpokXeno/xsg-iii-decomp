#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu246", func_0025FD90);

typedef signed char s8;
typedef unsigned char u8;
typedef short s16;
typedef int s32;
typedef float f32;
extern unsigned char gMapProbeObjectStorage[];

/* The probe object at gMapProbeObjectStorage, the MapCollisionObject layout of include/main/tu352.h; only the fields this file writes are claimed. */
typedef struct MapProbeObject {
    s32 hitRecord;                         /* +0x00: MapRecord * (0 for none) */
    s32 previousRecord;                 /* +0x04: MapRecord * */
    unsigned char unmodeled_08[0xA - 0x8];
    unsigned short bits;                /* +0x0A: the attribute mask selecting what a probe hits */
    unsigned char unmodeled_0C[0x14 - 0xC];
    float heightTolerance;              /* +0x14 */
} MapProbeObject;

#define gMapProbeObject (*(MapProbeObject *) gMapProbeObjectStorage)

s32 func_002A70E8();

void mapProbeObjectInit(void) {
    func_002A70E8(gMapProbeObjectStorage);
    gMapProbeObject.previousRecord = 0;
    gMapProbeObject.bits = 0x803;
    gMapProbeObject.heightTolerance = 1000.0f;
    gMapProbeObject.hitRecord = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu246", func_0025FEE0);

INCLUDE_ASM("asm/main/nonmatchings/tu246", func_0025FF18);

INCLUDE_ASM("asm/main/nonmatchings/tu246", func_0025FFF8);

INCLUDE_ASM("asm/main/nonmatchings/tu246", func_00260140);

void mapProbeObjectInit();

void mapProbeObjectReset(void) {
    mapProbeObjectInit();
}

INCLUDE_ASM("asm/main/nonmatchings/tu246", func_00260238);

INCLUDE_ASM("asm/main/nonmatchings/tu246", func_00260398);

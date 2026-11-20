#include "common.h"
#include "shared.h"

#include "main/xenoActor.h"
#include "ov02/tu018.h"
#include "ov02/tu060.h"

struct XenoLight;
struct XenoLight *lightBlockGet(void);
void runtimeFogPtrGet(union DoublewordVector4 **pos, union DoublewordVector4 **dir);
/* The runtime position and direction; unit_cmd.c copies them whole into every battle actor's color and colorBlock. */
extern DoublewordVector4 runtimePos;
extern DoublewordVector4 runtimeDir;
extern XenoLight lightBlock;
void func_001975C8(XenoLight *light);
void func_0019E830(XenoLight *light);



struct BattleUnit;



void monsActiveSlotsGet(struct MonsSlot ***slots);
void sefSetFogOverride(Vector4 *pos, Vector4 *dir);
int unitTblGet(int side, BattleUnit ***table);
struct SbThread;
int vmArgGet(struct SbThread *call, int index);
float func_001D4058(struct SbThread *call, int index);
void func_00A8D330(DoublewordVector4 *pos, DoublewordVector4 *dir);
void unitTblBodySetFog(DoublewordVector4 *pos, DoublewordVector4 *dir);

/* Script command: fills the runtime position and direction from the call's arguments, then fogs the units that the
 * flags select. */
int runtimeSetFog(struct SbThread *call)
{
    int flags;

    flags = vmArgGet(call, 1);
    runtimePos.vector.x = func_001D4058(call, 2);
    runtimePos.vector.y = func_001D4058(call, 3);
    runtimePos.vector.z = func_001D4058(call, 4);
    runtimePos.vector.w = func_001D4058(call, 5);
    runtimeDir.vector.x = (float) vmArgGet(call, 6);
    runtimeDir.vector.y = (float) vmArgGet(call, 7);
    runtimeDir.vector.z = (float) vmArgGet(call, 8);
    if (flags & 1) {
        func_00A8D330(&runtimePos, &runtimeDir);
    }
    if (flags & 2) {
        unitTblBodySetFog(&runtimePos, &runtimeDir);
    }
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/sb_natives_unit_035", func_00A8D330);

/* Fogs every unit of side 2: each non-null unit's actor gets the flag bit and the pos and dir values. */
void unitTblBodySetFog(DoublewordVector4 *pos, DoublewordVector4 *dir)
{
    BattleUnit **units;
    BattleUnit *unit;
    struct XenoActor *actor;
    int count;
    int i;

    count = unitTblGet(2, &units);
    for (i = 0; i < count; i++) {
        unit = units[i];
        if (unit != 0) {
            actor = unit->unit;
            actor->drawFlags |= 1;
            actor->color[0] = pos->vector.x;
            actor->color[1] = pos->vector.y;
            actor->color[2] = pos->vector.z;
            actor->color[3] = pos->vector.w;
            actor->colorBlock[0] = dir->vector.x;
            actor->colorBlock[1] = dir->vector.y;
            actor->colorBlock[2] = dir->vector.z;
        }
    }
}

/* Hands out the addresses of the runtime position and direction vectors through its out-pointers. */
void runtimeFogPtrGet(DoublewordVector4 **pos, DoublewordVector4 **dir)
{
    *pos = &runtimePos;
    *dir = &runtimeDir;
}

/* Sets up the light block through two helpers, then returns 1. */
int initLight(void)
{
    XenoLight *light;

    light = lightBlockGet();
    func_001975C8(light);
    func_0019E830(light);
    return 1;
}

/* Returns the light block that initLight and runtime.setLight start from. */
XenoLight *lightBlockGet(void) {
    return &lightBlock;
}

INCLUDE_ASM("asm/nonmatchings/ov02/sb_natives_unit_035", func_00A8D4E8);

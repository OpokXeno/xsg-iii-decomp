#include "common.h"
#include "shared.h"

int bgmIndexGet(void);
extern int bgmIndex;

INCLUDE_ASM("asm/nonmatchings/ov02/sb_natives_unit_037", func_00A8D900);

INCLUDE_ASM("asm/nonmatchings/ov02/sb_natives_unit_037", func_00A8D928);

INCLUDE_ASM("asm/nonmatchings/ov02/sb_natives_unit_037", func_00A8DA50);

struct SbThread;
int vmArgGet(struct SbThread *thread, int argIndex);

int func_00A8DB28(struct SbThread *call)
{
    bgmIndex = vmArgGet(call, 1);
    return 1;
}

/* Returns the background music index that bgmPlay stored. */
int bgmIndexGet(void) {
    return bgmIndex;
}

void func_0018FAC8(int value);

int func_00A8DB60(struct SbThread *call)
{
    func_0018FAC8(vmArgGet(call, 1));
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/sb_natives_unit_037", func_00A8DB88);

INCLUDE_ASM("asm/nonmatchings/ov02/sb_natives_unit_037", func_00A8DBE0);

INCLUDE_ASM("asm/nonmatchings/ov02/sb_natives_unit_037", func_00A8DC38);

void func_002AC2E8(int value);
int vmKeyedEntryGet(struct SbThread *thread, int value);
void func_00A9DFB0(int a, int b, int *c, int d);
extern int D_00AC1BA0;

int runEffSet(struct SbThread *call)
{
    func_002AC2E8(0);
    D_00AC1BA0 = vmKeyedEntryGet(call, vmArgGet(call, 1));
    func_00A9DFB0(0, 0, &D_00AC1BA0, 0);
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/sb_natives_unit_037", func_00A8DCE0);

#include "main/xenoActor.h"
#include "main/xgl_2.h"

struct BattleUnitPartial {
    unsigned char unmodeled_00[0x2b4];
    struct XenoActor *unit;
};


struct BattleUnitPartial *unitTblEntryGet(int index);

int batScale(struct SbThread *call)
{
    struct BattleUnitPartial *unit;
    float scale;

    unit = unitTblEntryGet(vmArgGet(call, 1));
    scale = vmArgGet(call, 2) / 100.0f;
    ((Vector4Elements *)&unit->unit->scale)->component[0] = scale;
    ((Vector4Elements *)&unit->unit->scale)->component[1] = scale;
    ((Vector4Elements *)&unit->unit->scale)->component[2] = scale;
    return 1;
}

#include "common.h"
#include "shared.h"

/* Rebuilds the key list: zeroes the buffer, copies the lookup key of every live unit into it in table order, then walks it. */ void keyBuffClear(void);
void dataVPadModeSet_002ACA60(int mode);
void dataVPadReset(void);
void menuWepSpPanelReset(void);
/* Uninitialized storage of this file, in address order (only this file names these labels). */
extern int keyBuff[8];
extern int keyBuffIdx;
extern int menuWepSpPanelCursor;
extern int menuWepSpPanelRowCount;
extern int padMode;
extern short padData;

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89128);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A891F0);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A892E8);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89340);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A893B8);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A893E8);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89400);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89768);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A897A0);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A897C8);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89838);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", xglStudioMainCameraMove);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89878);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89970);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89A40);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89AB0);

/* Virtual pad reset: pad mode and pad data, then the two counter pairs (prototype in battle_init.h). */
void dataVPadReset(void) {
    dataVPadModeSet_002ACA60(0);
    keyBuffClear();
    menuWepSpPanelReset();
}

/* Zero-filled 128-byte key list written by keyBuffClear; splat name, no config symbol. */
extern int D_00ABD3C0[32];

#include "ov02/tu060.h"


void *memset(void *destination, int value, unsigned int count);
struct BattleUnit *func_00A8C418(int index);
int func_00A8C468(void);
void func_002AD3D0(int *list);

/* Rebuilds the key list: zeroes the buffer, copies the lookup key of every live unit into it in table order, then walks it. */
void keyBuffClear(void) {
    int *out;
    int i;
    struct BattleUnit *unit;

    memset(D_00ABD3C0, 0, 0x80);
    out = D_00ABD3C0;
    for (i = 0; i < func_00A8C468(); i++) {
        unit = func_00A8C418(i);
        if (unit != 0 && !(unit->flags & 0x8000)) {
            *out = unit->poolEntry->lookupKey;
            out++;
        }
    }
    func_002AD3D0(D_00ABD3C0);
}

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89C68);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89D68);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89E28);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89E40);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89E58);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89E80);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89EC8);

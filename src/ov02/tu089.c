/*
 * OV02 original TU 89: 0x00ab1310..0x00ab1918 (15 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu089", func_00AB1310);

INCLUDE_ASM("asm/nonmatchings/ov02/tu089", func_00AB1390);

INCLUDE_ASM("asm/nonmatchings/ov02/tu089", func_00AB1430);

INCLUDE_ASM("asm/nonmatchings/ov02/tu089", func_00AB1470);

INCLUDE_ASM("asm/nonmatchings/ov02/tu089", func_00AB1570);

INCLUDE_ASM("asm/nonmatchings/ov02/tu089", func_00AB1660);

void assertFailed(const char *message, const char *file, int line);
int func_00AB1660(const char *name);
void RgDispModelSetLocal(int dispModel);
extern const char D_00ABB658[];
extern const char D_00ABB668[];

void RgDispModelSetLocalByName(const char *name) {
    int dispModel;

    dispModel = func_00AB1660(name);
    if (dispModel == 0) {
        assertFailed(D_00ABB668, D_00ABB658, 178);
    }
    RgDispModelSetLocal(dispModel);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu089", func_00AB16D8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu089", func_00AB16F8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu089", func_00AB1728);

INCLUDE_ASM("asm/nonmatchings/ov02/tu089", func_00AB1740);

INCLUDE_ASM("asm/nonmatchings/ov02/tu089", battleResultReset);

INCLUDE_ASM("asm/nonmatchings/ov02/tu089", func_00AB1798);

INCLUDE_ASM("asm/nonmatchings/ov02/tu089", func_00AB17B0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu089", func_00AB1880);

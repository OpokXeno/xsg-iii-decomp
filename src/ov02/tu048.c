/*
 * OV02 original TU 48: 0x00a92c88..0x00a936b8 (28 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A92C88);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A92CB0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A92D48);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A92E50);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A92E78);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A92ED8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A92F90);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A92FC8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A93078);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A930C0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A93100);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A93120);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A93148);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A93170);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A931C0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A931D8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A93228);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A93278);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A932B0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A932F0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A93328);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A93348);

#include "ov02/tu060.h"


extern char *D_00AB7F08;
extern char *D_00AB7F0C[];
extern char *D_00AB7F34[];
extern char D_00AB8C48[];
extern char D_00AB8C50[];
extern char D_00AB8C58[];
extern char D_00AB8C60[];
extern char D_00AC3328[];
char *strcpy(char *dst, const char *src);
char *func_0030E3D0(char *dst, const char *src);
int func_00A86AC0(void);
int func_00A818D0(BattleUnit *unit);
int rnd(int range);

char *func_00A93370(BattleUnit *unit, int kind)
{
    char *suffix;
    short key;
    int roll;

    key = unit->poolEntry->lookupKey;
    if ((unsigned int)(key - 1) >= 10) {
        return 0;
    }
    if ((unsigned int)(kind - 1) >= 14) {
        return 0;
    }
    strcpy(D_00AC3328, D_00AB7F08);
    func_0030E3D0(D_00AC3328, D_00AB7F0C[key]);
    if (key == 5) {
        if (func_00A86AC0() != 0) {
            suffix = D_00AB8C48;
        } else {
            suffix = D_00AB8C50;
        }
        func_0030E3D0(D_00AC3328, suffix);
    }
    func_0030E3D0(D_00AC3328, D_00AB8C58);
    func_0030E3D0(D_00AC3328, D_00AB7F34[kind]);
    if (kind == 1) {
        roll = rnd(100);
        if (roll >= 81) {
            suffix = D_00AB8C60;
        } else if (roll >= 51) {
            suffix = D_00AB8C48;
        } else {
            suffix = D_00AB8C50;
        }
        func_0030E3D0(D_00AC3328, suffix);
    } else if (kind == 2) {
        if (func_00A818D0(unit) != 0) {
            suffix = D_00AB8C48;
        } else {
            suffix = D_00AB8C50;
        }
        func_0030E3D0(D_00AC3328, suffix);
    }
    return D_00AC3328;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A93508);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A93540);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A935B8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A935C8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu048", func_00A93610);

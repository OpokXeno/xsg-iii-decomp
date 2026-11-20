#include "common.h"

/* Sprintf-shaped varargs callee; the link name of 0x0030E1B8 (test_050.c declares it the same way). */
int GameMpeg2Play(char *destination, const char *format, ...);
/* Kind-prefix pointer table at 0x004AD6A0 (scaffold data of this TU). */
extern char *D_004AD6A0[4];

void srsFormatEffectStem(int effectNo, char *buffer, int withSubNo)
{
    int effectKind = (effectNo >> 28) & 3;
    int effectIndex = (effectNo >> 12) & 0x3ff;
    int effectSub = effectNo & 0x3ff;

    if (effectNo < 0) {
        GameMpeg2Play(buffer, "bad effect no:%d", effectNo);
        return;
    }
    if (effectKind == 3) {
        GameMpeg2Play(buffer, "%s%04d", D_004AD6A0[3], effectNo & 0x0fffffff);
        return;
    }
    if (withSubNo != 0) {
        GameMpeg2Play(buffer, "%s%03d_%03d", D_004AD6A0[effectKind], effectIndex, effectSub);
        return;
    }
    GameMpeg2Play(buffer, "%s%03d", D_004AD6A0[effectKind], effectIndex);
}

INCLUDE_ASM("asm/main/nonmatchings/srs", func_0020F790);

INCLUDE_ASM("asm/main/nonmatchings/srs", func_0020F9E0);

INCLUDE_ASM("asm/main/nonmatchings/srs", func_0020FA70);

INCLUDE_ASM("asm/main/nonmatchings/srs", func_0020FBE8);

INCLUDE_ASM("asm/main/nonmatchings/srs", func_0020FC48);

/* Maps a kind-0 key whose index is 300..340 to slot (key - 1) & 0xF; any other key gives 100. */
int srsGetEffectSlot(int effectKey)
{
    int effectIndex = (effectKey >> 12) & 0x3ff;
    int slot = 100;

    if (((effectKey >> 28) & 3) == 0 && (unsigned int) (effectIndex - 300) < 41) {
        slot = (effectKey - 1) & 0xf;
    }
    return slot;
}

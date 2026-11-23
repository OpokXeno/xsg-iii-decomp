#include "common.h"
#include "shared.h"

int scGetImmAdrImmIdx2(int scriptBase, unsigned short *immediateTable, int immediateIndex);
short scGetImmNumIdx(short *immediateTable, int immediateIndex);
int scOfsToAdr(int encodedOffset);

#define SCRIPT_ADDRESS_NONE 0xFFFF    /* absent table entry or address */

INCLUDE_ASM("asm/main/nonmatchings/sc_get", func_001ECA60);

typedef int s32;
s32 soundCueSeCommand(s32, s32, s32, s32);

void scQueueDefaultRequest(void) {
    soundCueSeCommand(0, -3, 0, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/sc_get", func_001ECE98);

INCLUDE_ASM("asm/main/nonmatchings/sc_get", func_001ECEB0);

INCLUDE_ASM("asm/main/nonmatchings/sc_get", func_001ECEC8);

INCLUDE_ASM("asm/main/nonmatchings/sc_get", func_001ECF70);

INCLUDE_ASM("asm/main/nonmatchings/sc_get", func_001ECFF0);

INCLUDE_ASM("asm/main/nonmatchings/sc_get", func_001ED068);

extern int gSefExecSteps;
void func_002022F0(int arg);

void sefAdvanceStepCounter(void)
{
    gSefExecSteps += 1;
    func_002022F0(0);
}

int scGetImmAdrImmIdx2(int scriptBase, unsigned short *immediateTable, int immediateIndex)
{
    int halfwordOffset;
    int address;

    halfwordOffset = immediateTable[immediateIndex];
    if (halfwordOffset == SCRIPT_ADDRESS_NONE || halfwordOffset == -1) {
        halfwordOffset = 0;
    }
    address = 0;
    if (halfwordOffset != 0) {
        address = scriptBase + halfwordOffset * 2;
    }
    return address;
}

short scGetImmNumIdx(short *immediateTable, int immediateIndex)
{
    return immediateTable[immediateIndex];
}

INCLUDE_ASM("asm/main/nonmatchings/sc_get", func_001ED360);

int scOfsToAdr(int encodedOffset)
{
    if (encodedOffset == SCRIPT_ADDRESS_NONE || encodedOffset == -1) {
        encodedOffset = 0;
    }
    return encodedOffset;
}

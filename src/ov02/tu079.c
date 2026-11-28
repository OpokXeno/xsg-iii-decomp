/*
 * OV02 original TU 79: 0x00aab628..0x00aabbc8 (5 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu079", func_00AAB628);

INCLUDE_ASM("asm/nonmatchings/ov02/tu079", func_00AAB6C8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu079", func_00AAB798);

typedef int s32;
typedef struct Ov02ArmedTrigger {
    unsigned char unk00[0x50];
    s32 armed;
    s32 active;
} Ov02ArmedTrigger;

extern Ov02ArmedTrigger gOv02ArmedTrigger;
void battleUpdateStep();

void ov02FlagPoll(void) {
    s32 isArmed;

    if (gOv02ArmedTrigger.active != 0) {
        gOv02ArmedTrigger.active = 0;
    } else {
        isArmed = gOv02ArmedTrigger.armed != 0;
        /* The do/while(0) keeps the call out of tail position; plain if/else changes the frame. */
        do { if (isArmed) { battleUpdateStep(); } } while (0);
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu079", func_00AAB820);

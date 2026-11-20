#include "common.h"
#include "shared.h"

#include "main/tu105.h"

int sbNativeOwnerAdd(const char *name, SbNativeMethod *methods);

void scriptRegisterNatives(void);

extern char talkModuleName[];
extern char g2pathModuleName[];
extern char segModuleName[];
extern SbNativeMethod talkNativeTable[];
extern SbNativeMethod g2pathNativeTable[];
extern SbNativeMethod segNativeTable[];
extern SbNativeMethod talkNativeTable[17];
extern SbNativeMethod g2pathNativeTable[5];
extern SbNativeMethod segNativeTable[3];

/* .sdata of this file (0x69D7B8-0x69D7D0): the module names of scriptRegisterNatives. */
extern char talkModuleName[8];

extern char g2pathModuleName[8];
extern char segModuleName[8];

/*
 * Registers three native groups with the script VM: "talk" (table 0x3ec868),
 * "g2path" (table 0x3ec8f0) and "seg" (table 0x3ec918); the group names sit at
 * talkModuleName, g2pathModuleName and segModuleName.
 */
void scriptRegisterNatives(void)
{
    sbNativeOwnerAdd(talkModuleName, talkNativeTable);
    sbNativeOwnerAdd(g2pathModuleName, g2pathNativeTable);
    sbNativeOwnerAdd(segModuleName, segNativeTable);
}

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D3BE8);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D3D38);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D3EC0);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D3F30);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D3F70);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4100);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4110);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4120);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4130);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4230);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D42D8);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4390);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4578);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4710);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4788);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D47F0);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4870);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D48C8);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4A00);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4A08);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4A10);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4C40);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4C50);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4D18);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4D90);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D55E8);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D5658);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D5720);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D58D0);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D59D8);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D59E0);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D5C80);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D5E20);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D5E28);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D5FF8);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6030);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6048);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6060);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6098);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D60B0);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D60C8);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6100);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6118);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6130);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6158);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6180);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6270);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D62F8);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6348);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6430);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6478);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6550);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6598);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6678);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6740);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D67C0);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6828);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6890);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D68E0);

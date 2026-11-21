#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu059", func_001B8508);

INCLUDE_ASM("asm/main/nonmatchings/tu059", func_001B86F0);

INCLUDE_ASM("asm/main/nonmatchings/tu059", func_001B8780);

INCLUDE_ASM("asm/main/nonmatchings/tu059", func_001B88B0);

typedef unsigned char u8;
typedef int s32;

/* Command record: commandTargetId at +0x8 names the pool block, commandParam at +0x10 is the command parameter. */
typedef struct MailboxCommandRecord {
    u8 unk_00[0x8];
    s32 commandTargetId; /* 0x8 */
    u8 unk_0C[0x10 - 0xC];
    s32 commandParam; /* 0x10 */
} MailboxCommandRecord;

s32 poolBlockTagWordSet(s32, s32, s32);
s32 func_00210480(s32, s32);

void mailboxSubmitCommandParam(MailboxCommandRecord *commandRecord) {
    func_00210480(commandRecord->commandTargetId, 0);
    poolBlockTagWordSet(commandRecord->commandTargetId, 1, 0xCF102000);
    poolBlockTagWordSet(commandRecord->commandTargetId, 2, commandRecord->commandParam);
}

INCLUDE_ASM("asm/main/nonmatchings/tu059", func_001B8960);

void mailboxSubmitCommand(MailboxCommandRecord *commandRecord) {
    func_00210480(commandRecord->commandTargetId, 0);
    poolBlockTagWordSet(commandRecord->commandTargetId, 1, 0xCF102001);
}

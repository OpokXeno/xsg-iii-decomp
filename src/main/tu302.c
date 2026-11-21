#include "common.h"

typedef int s32;

/* Object whose word at +0x04 is the child record pointer. */
typedef struct EnepcObject {
    unsigned char unk_00[0x4];
    struct EnepcChild *child;
} EnepcObject;

/* Child record: the state word at 0x4EC has its two low bits cleared. */
typedef struct EnepcChild {
    unsigned char unk_0000[0x4EC];
    s32 stateFlags;
} EnepcChild;

void sbNativeChildClearStateBits(EnepcObject *enepcObject) {
    EnepcChild *child;

    child = enepcObject->child;
    child->stateFlags &= ~3;
}

INCLUDE_ASM("asm/main/nonmatchings/tu302", func_00283C98);

INCLUDE_ASM("asm/main/nonmatchings/tu302", func_00283DD0);

INCLUDE_ASM("asm/main/nonmatchings/tu302", func_00283F18);

INCLUDE_ASM("asm/main/nonmatchings/tu302", func_00283FD0);

INCLUDE_ASM("asm/main/nonmatchings/tu302", func_00284080);

INCLUDE_ASM("asm/main/nonmatchings/tu302", func_00284130);

INCLUDE_ASM("asm/main/nonmatchings/tu302", func_002841F8);

INCLUDE_ASM("asm/main/nonmatchings/tu302", func_00284338);

INCLUDE_ASM("asm/main/nonmatchings/tu302", func_00284508);

INCLUDE_ASM("asm/main/nonmatchings/tu302", func_00284630);

INCLUDE_ASM("asm/main/nonmatchings/tu302", func_00284820);

INCLUDE_ASM("asm/main/nonmatchings/tu302", func_00284930);

extern char enepcNativeTable[];
extern char enepcModuleName[];
int sbNativeOwnerAdd(const char *name, void *methods);

void enepcRegisterNatives(void) {
    sbNativeOwnerAdd(enepcModuleName, enepcNativeTable);
}

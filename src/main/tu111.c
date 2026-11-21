#include "common.h"
#include "main/Packet.h"

typedef int s32;

INCLUDE_ASM("asm/main/nonmatchings/tu111", func_001E2450);

void gifStateFlush(void);
void sceVif1PkCloseDirectHLCode(XglPacket *packet);

void gifStateFlushAndCloseDirect(XglPacket *packet) {
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);
}

INCLUDE_ASM("asm/main/nonmatchings/tu111", func_001E2518);

INCLUDE_ASM("asm/main/nonmatchings/tu111", func_001E25F0);

INCLUDE_ASM("asm/main/nonmatchings/tu111", func_001E34E0);

INCLUDE_ASM("asm/main/nonmatchings/tu111", func_001E4348);

INCLUDE_ASM("asm/main/nonmatchings/tu111", func_001E4818);

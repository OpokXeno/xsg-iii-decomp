#include "common.h"
#include "shared.h"

void sefPacketFlush(void);

struct XglPacket;

#include "main/Packet.h"


XglPacket *xglPacketGetCurrent(void);

void sceVif1PkCnt(XglPacket *packet, unsigned int count);
void sceVif1PkAddCode(XglPacket *packet, unsigned int tag);

/* VIF codes (command in bits 24..30): FLUSH, MSCNT, and MSCAL whose immediate is the micro program address / 8. */
#define VIF_CODE_FLUSH 0x11000000

void sefPacketFlush(void) {
    XglPacket *packet;

    packet = xglPacketGetCurrent();
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddCode(packet, VIF_CODE_FLUSH);
}

INCLUDE_ASM("asm/main/nonmatchings/sdv_packet", func_001F3700);

INCLUDE_ASM("asm/main/nonmatchings/sdv_packet", func_001F3788);

INCLUDE_ASM("asm/main/nonmatchings/sdv_packet", func_001F3828);

INCLUDE_ASM("asm/main/nonmatchings/sdv_packet", func_001F38B0);

INCLUDE_ASM("asm/main/nonmatchings/sdv_packet", func_001F3900);

INCLUDE_ASM("asm/main/nonmatchings/sdv_packet", func_001F3968);

INCLUDE_ASM("asm/main/nonmatchings/sdv_packet", func_001F3A98);

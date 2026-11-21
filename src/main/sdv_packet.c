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

typedef int s32;
extern unsigned char gSdvPacketTable[];
void *func_001F9B18(s32);
void *memset(void *destination, s32 value, unsigned int count);

/* Claims one slot of the packet table: returns the slot's base, or 0 when the allocation fails. */
void *sdvPacketClaim(s32 packetSlotIndex) {
    unsigned char *packetSlot;

    packetSlot = func_001F9B18(packetSlotIndex);
    if (packetSlot != 0) {
        memset(packetSlot, 0, 0x18);
        packetSlot = &gSdvPacketTable[packetSlotIndex];
        packetSlot[0xF10] = 0;
        gSdvPacketTable[0xF51] = packetSlotIndex;
    }
    return packetSlot;
}

INCLUDE_ASM("asm/main/nonmatchings/sdv_packet", func_001F3900);

INCLUDE_ASM("asm/main/nonmatchings/sdv_packet", func_001F3968);

INCLUDE_ASM("asm/main/nonmatchings/sdv_packet", func_001F3A98);

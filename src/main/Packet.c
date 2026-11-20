#include "common.h"
#include "shared.h"

#include "main/Packet.h"


XglPacket *xglPacketGetCurrent(void);
extern XglPacket *pCurrentPacket;

INCLUDE_ASM("asm/main/nonmatchings/Packet", func_001997D0);

XglPacket *xglPacketGetCurrent(void)
{
    return pCurrentPacket;
}

void func_002F4878(XglPacket *packet, u32 buffer);
extern XglPacket D_008BC260[2];
extern XglPacket *D_00551588;

void func_001998E0(u32 buffer, u32 size, float rate)
{
    int reserve;
    XglPacket **currentPacket = &pCurrentPacket;
    XglPacket **sendPacket = &D_00551588;

    if (rate < 1.0f) {
        rate = 1.0f;
    }
    if (rate > 99.0f) {
        rate = 99.0f;
    }
    reserve = size * rate / 100.0f;
    if (reserve & 0xF) {
        reserve = reserve - (reserve & 0xF) + 0x10;
    }
    func_002F4878(&D_008BC260[0], buffer);
    func_002F4878(&D_008BC260[1], buffer + size);
    D_008BC260[0].bufferSize = size;
    D_008BC260[1].bufferSize = size;
    D_008BC260[0].buffer = buffer;
    D_008BC260[1].buffer = buffer + size;
    *sendPacket = 0;
    D_008BC260[0].limit = buffer + size - reserve;
    D_008BC260[0].cursor = (u8 *) D_008BC260[0].limit;
    D_008BC260[1].limit = buffer + size + size - reserve;
    D_008BC260[1].cursor = (u8 *) D_008BC260[1].limit;
    *currentPacket = &D_008BC260[0];
}

INCLUDE_ASM("asm/main/nonmatchings/Packet", func_00199A20);

INCLUDE_ASM("asm/main/nonmatchings/Packet", xglPacketSend);

INCLUDE_ASM("asm/main/nonmatchings/Packet", func_00199B88);

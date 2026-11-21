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

void func_002F4878(XglPacket *packet, u32 packetBase);
extern XglPacket D_008BC260[2];
extern XglPacket *pSendPacket;

void xglPacketInitDoubleBuffer(u32 packetBase, u32 packetHalfSize, float reservePercent)
{
    int reserveSize;
    XglPacket **currentPacket = &pCurrentPacket;
    XglPacket **sendPacket = &pSendPacket;

    if (reservePercent < 1.0f) {
        reservePercent = 1.0f;
    }
    if (reservePercent > 99.0f) {
        reservePercent = 99.0f;
    }
    reserveSize = packetHalfSize * reservePercent / 100.0f;
    if (reserveSize & 0xF) {
        reserveSize = reserveSize - (reserveSize & 0xF) + 0x10;
    }
    func_002F4878(&D_008BC260[0], packetBase);
    func_002F4878(&D_008BC260[1], packetBase + packetHalfSize);
    D_008BC260[0].bufferSize = packetHalfSize;
    D_008BC260[1].bufferSize = packetHalfSize;
    D_008BC260[0].buffer = packetBase;
    D_008BC260[1].buffer = packetBase + packetHalfSize;
    *sendPacket = 0;
    D_008BC260[0].limit = packetBase + packetHalfSize - reserveSize;
    D_008BC260[0].cursor = (u8 *) D_008BC260[0].limit;
    D_008BC260[1].limit = packetBase + packetHalfSize + packetHalfSize - reserveSize;
    D_008BC260[1].cursor = (u8 *) D_008BC260[1].limit;
    *currentPacket = &D_008BC260[0];
}

INCLUDE_ASM("asm/main/nonmatchings/Packet", func_00199A20);

typedef int s32;
void xglDmaDirectSrcChain(u32 channel, u32 address);

void xglPacketSend(void) {
    if (pSendPacket != 0) {
        xglDmaDirectSrcChain(1, pSendPacket->start);
    }
}

extern s32 xglPacketSendMode;

void xglPacketSetSendMode(s32 sendMode) {
    xglPacketSendMode = sendMode;
}

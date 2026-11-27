#include "common.h"
#include "shared.h"

void testWindowSetColumnTable(int, void *);

extern unsigned char heapOverviewMenuColumnTable[];

void testOverlayMenuOpen(int, int, int, int, int);

extern unsigned char heapOverviewMenuColumnTable[160];

void heapOverviewMenuAttach(int windowIndex)
{
    testWindowSetColumnTable(windowIndex, heapOverviewMenuColumnTable);
}

void testOverlayEntryCall(int menuIndex)
{
    testOverlayMenuOpen(menuIndex, 0, 0, 0, 0);
}

typedef int s32;
extern char gHeapOverviewTitle[];
void xglPacketTextureTrans(void *);
const char *messageTextGet(int id);
void sceVif1PkAddDataN(s32, void *, s32);
void sceVif1PkCnt(s32, unsigned int);
void xglFontPrint(int x, int y, unsigned int color, const char *text);
s32 xglPacketGetCurrent();

typedef struct HeapOverviewRecord {
    unsigned char unk_00[0x04];
    s32 packetDataOffset;
    unsigned char unk_08[0x40 - 0x08];
    unsigned char packetBase[0];
} HeapOverviewRecord;

typedef struct HeapOverviewData {
    unsigned char unk_00[0x0C];
    u16 quadwordCount;
} HeapOverviewData;

void heapOverviewMenuDraw(HeapOverviewRecord *overview) {
    s32 packet;
    HeapOverviewData *packetData;
    void *packetBase;
    const char *message140;
    const char *message1E;
    const char *message142;
    const char *message143;
    const char *message144;

    packet = xglPacketGetCurrent();
    packetBase = overview->packetBase;
    packetData = (HeapOverviewData *) (overview->packetBase + overview->packetDataOffset);
    if (packetBase != 0) {
        xglPacketTextureTrans(packetBase);
    }
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddDataN(packet, packetData, (packetData->quadwordCount * 4) + 4);
    xglFontPrint(0, 0, -1, gHeapOverviewTitle);
    message140 = messageTextGet(0x140);
    xglFontPrint(0x56, 0x13D, -1, message140);
    message1E = messageTextGet(0x1E);
    xglFontPrint(0x56, 0x159, -1, message1E);
    message142 = messageTextGet(0x142);
    xglFontPrint(0x56, 0x172, -1, message142);
    message143 = messageTextGet(0x143);
    xglFontPrint(0x56, 0x18B, -1, message143);
    message144 = messageTextGet(0x144);
    xglFontPrint(0x28, 0x60, -1, message144);
}

extern signed char D_004C8417[];

void testStoreScaledLevel(s32 level) {
    if (level < 0) {
        D_004C8417[0] = level - 0x80;
        return;
    }
    D_004C8417[0] = (level * 2) - 0x80;
}

INCLUDE_ASM("asm/main/nonmatchings/tu051", func_001B1D28);

INCLUDE_ASM("asm/main/nonmatchings/tu051", func_001B2200);

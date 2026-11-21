#include "common.h"

typedef int s32;
typedef long long s64;
typedef float f32;
typedef unsigned char u8;
extern s32 gPacketBufferBase;
extern s32 gPacketEntryCount;

typedef struct PacketEntry {
    s32 word0;
    s32 word1;
    s32 word2;
    s32 word3;
} PacketEntry;

typedef struct XglObject {
    u8 unk_00[0x3C];
    s32 modeId;
} XglObject;

void xglPacketBufferInit(s32 spr_address)
{
    extern s32 gPacketCursor;
  gPacketCursor = spr_address;
  gPacketEntryCount = 0;
  gPacketBufferBase = spr_address;
}

extern s32 gTagQuadCount;

void xglPacketAppendEntry(s32 word0, s32 word1, s32 word2, s32 word3) {
    extern void *gPacketCursor;
    extern void *gPacketLastTag;
    s32 *packetCursor;

    packetCursor = gPacketCursor;
    packetCursor[0] = word0;
    packetCursor[1] = word1;
    packetCursor[2] = word2;
    packetCursor[3] = word3;
    gPacketCursor = packetCursor + 4;
    gPacketLastTag = packetCursor;
    gTagQuadCount = 0;
    gPacketEntryCount += 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001D63B8);

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001D63F0);

void xglPacketAppendQuad(s64 highDoubleword, s64 lowDoubleword)
{
    extern void *gPacketCursor;
    s64 *packetCursor = gPacketCursor;
    s32 oldPacketEntryCount;
    s32 nextPacketEntryCount;
    s32 nextTagQuadCount;
    s32 tagQuadCountStore;
    s64 *nextPacketCursor;

    oldPacketEntryCount = gPacketEntryCount;
    nextTagQuadCount = gTagQuadCount + 1;
    nextPacketCursor = packetCursor + 2;
    nextPacketEntryCount = oldPacketEntryCount + 1;
    packetCursor[0] = lowDoubleword;
    tagQuadCountStore = nextTagQuadCount;
 do { packetCursor[1] = highDoubleword; gPacketCursor = nextPacketCursor; gPacketEntryCount = nextPacketEntryCount; gTagQuadCount = tagQuadCountStore; } while (0);
}

void xglPacketFinishTag(void) {
    extern s32 *gPacketLastTag;
    *gPacketLastTag |= gTagQuadCount;
}

s32 sceVif1PkAddDirectDataN(s32 packet, s32 address, s32 qword_count);
s32 sceVif1PkCloseDirectHLCode(s32 packet);
s32 sceVif1PkCnt(s32 packet, s32 count);
s32 sceVif1PkOpenDirectHLCode(s32 packet, s32 flags);
s32 xglPacketGetCurrent();

void xglPacketSendDirect(s32 address, s32 qwordCount) {
    s32 packet;

    packet = xglPacketGetCurrent();
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    sceVif1PkAddDirectDataN(packet, (address != 0) ? address : gPacketBufferBase, (qwordCount != 0) ? qwordCount : gPacketEntryCount);
    sceVif1PkCloseDirectHLCode(packet);
}

s32 xglPacketGetEntryCount(void) {
    return gPacketEntryCount;
}

s32 xglPacketGetQuadCount(void) {
    return gTagQuadCount;
}

void xglPacketSetEntryWords(PacketEntry *packetEntry, s32 word0, s32 word1, s32 word2, s32 word3) {
    packetEntry->word0 = word0;
    packetEntry->word1 = word1;
    packetEntry->word2 = word2;
    packetEntry->word3 = word3;
}

void xglPacketSetEntryQuad(s64 *packetEntry, s64 highDoubleword, s64 lowDoubleword) {
    packetEntry[0] = lowDoubleword;
    packetEntry[1] = highDoubleword;
}

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001D6518);

void xglPacketBufferInit(s32 spr_address);
void xglPacketAppendEntry(s32 word0, s32 word1, s32 word2, s32 word3);
void xglPacketAppendQuad(s64 highDoubleword, s64 lowDoubleword);
void xglPacketFinishTag(void);
void xglPacketSendDirect(s32 address, s32 qwordCount);

void xglPacketSendFixedHeader(void)
{
  xglPacketBufferInit(0x70000000);
  xglPacketAppendEntry(0x8000, 0x10000000, 0xE, 0);
  xglPacketAppendQuad(0x40, (((0x01BF0000L << 0x10) | 0x1FF) << 0x10));
  xglPacketFinishTag();
  xglPacketSendDirect(0, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001D66C0);

extern f32 D_00550FFC;

void xglSetOffsetValue(f32 offsetAmount) {
    D_00550FFC = offsetAmount;
}

f32 func_001AEE50();
f32 func_001AEF08();

s32 xglIsOffsetWithinLimits(void) {
    f32 x_limit;
    f32 y_limit;
    f32 remaining;
    s32 x_in_range;
    s32 y_in_range;

    x_limit = func_001AEE50();
    y_limit = func_001AEF08();
    y_in_range = 1;
    remaining = 85.0f - D_00550FFC;
    x_in_range = 1;
    if (!(remaining < x_limit)) {
        x_in_range = 0;
    }
    if (!(remaining < y_limit)) {
        y_in_range = 0;
    }
    return x_in_range | y_in_range;
}

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001D67F8);

s32 interpolateLinearFixed16(s32 mapInput, s32 mapMin, s32 mapMax, s32 scaledMin, s32 scaledMax) {
    f32 mapOffset;
    f32 scaledSpan;
    f32 mapSpan;

    mapOffset = mapInput - mapMin;
    scaledSpan = scaledMax - scaledMin;
    mapSpan = mapMax - mapMin;
    return ((mapOffset * (scaledSpan / mapSpan)) + scaledMin) * 16.0f;
}

s32 interpolateLinear(s32 mapInput, s32 mapMin, s32 mapMax, s32 scaledMin, s32 scaledMax) {
    f32 mapOffset;
    f32 scaledSpan;
    f32 mapSpan;

    mapOffset = mapInput - mapMin;
    scaledSpan = scaledMax - scaledMin;
    mapSpan = mapMax - mapMin;
    return (mapOffset * (scaledSpan / mapSpan)) + scaledMin;
}

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001D6928);

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001D7180);

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001D7440);

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001D7638);

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001D87F8);

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001D8D38);

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001D9360);

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001DA050);

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001DA858);

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001DAC58);

s32 func_001DB658();

void xglObjectSetModeOne(XglObject *object) {
    object->modeId = 1;
    func_001DB658();
}

void xglObjectSetModeTwo(XglObject *object) {
    object->modeId = 2;
    func_001DB658();
}

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001DB658);

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001DC0F0);

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001DC710);

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001DCCF0);

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001DCF70);

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001DD998);

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001DE530);

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001DE9F8);

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001DEA48);

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001DEAC8);

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001DF7D8);

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001E0630);

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001E0E10);

INCLUDE_ASM("asm/main/nonmatchings/tu110", func_001E1128);

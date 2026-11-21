#include "common.h"
#include "shared.h"

#include "main/Packet.h"
#include "main/tu045.h"
#include "main/tu141.h"
#include "main/xgl_2.h"

/* Row-major 4x4 float matrix. */










/* A vector kept as a plain float[4] (like sceVu0FVECTOR); its w is element 3. Vector4 here changes the
 * address arithmetic of the w stores, so the casts to Vector4 * at the calls stay. */




/* One 16-byte entry of the direct queue (a GIF packet of A+D pairs): data and register address, or four words (the tag,
 * then the header words, are written through the word view). */
typedef union XglDirectQueueEntry {
    struct {
        u64 registerValue;
        u64 address;
    };
    u32 tagAndHeaderWords[4];
} XglDirectQueueEntry;





struct PxyDrawPartRange;
union Matrix;
struct PxyShadow;
struct PxyHideMask;



extern XglDirectQueueEntry *xglDirectQueueBuffer;
extern int xglDirectQueueCount;
void nmlPacketClrTextureCache(void);
void nmlPacketClrModelCache(void);
void xglPacketModeReset(void);
void nmlPacketGsInit(void);
void xglPacketSelectDefaultMode(void);
void nmlPacketAddGsFlush(void);
void xglPacketInitVuState(void);
void xglPacketInitVuStateArg(int value);



/* Appends one A+D register write (value, then register address) at the u64 cursor p. */
#define GS_PUT(p, reg, value) ((p)[0] = (value), (p)[1] = (reg), (p) += 2)
#define GS_SET_ZBUF(zbp, psm, zmsk) ((u64)(zbp) | ((u64)(psm) << 24) | ((u64)(zmsk) << 32))

XglPacket *xglPacketGetCurrent(void);
void sceVif1PkCnt(XglPacket *packet, unsigned int count);
void sceVif1PkAddCode(XglPacket *packet, unsigned int tag);
void sceVif1PkRef(XglPacket *packet, const void *data, unsigned int qwc, unsigned int a3, unsigned int a4,
                  unsigned int a5);
void sceVif1PkOpenUpkCode(XglPacket *packet, unsigned int address, unsigned int format, unsigned int a3,
                          unsigned int a4);
void sceVif1PkCloseUpkCode(XglPacket *packet);
void sceVif1PkAddUpkData128N(XglPacket *packet, const void *data, unsigned int count);
struct XglCamera;
extern XglCamera *xglStudioSelectedCamera;
extern int xglPacketMode;
extern TxyBlock *pxyTextureTableCache;
extern int pxyTextureChannelCache;
extern PxyPacket *pxyUploadedPacket;
extern int pxyReflectionType;

typedef int s32;

/* The reflection matrix and the words packed into its last column and row. */
typedef union PxyReflectMatrix {
    Matrix4 rows;
    s32 intRows[4][4];
} PxyReflectMatrix;

/* Static block of the reflection state (.bss 0x9899F0-0x989A50): the colour vector, the reflection matrix,
 * then the cleared vector and the rotation angle. */
typedef struct PxyReflectBlock {
    Vector4 colour;     /* +0x00 pxyReflectBlock.colour (colorScale copy) */
    /* +0x10 reflection matrix (xglMatrixStackSave target); pxyUploadViewMatrices fills its last column with the
     * blend, shade and mode words and its last row with the position vector and the flag. */
    PxyReflectMatrix matrix;
    s32 zeroVec[3];     /* +0x50 cleared by pxyResetReflectionState */
    float angle;        /* +0x5C reflection rotation angle */
} PxyReflectBlock;

extern PxyReflectBlock pxyReflectBlock;
extern u8 xglPlainMicrocodeSize[];
extern u8 xglPlainMicrocodeProgram[];
extern u8 xglEnvMicrocodeSize[];
extern u8 xglEnvMicrocodeProgram[];
extern u8 xglTexturePassMicrocodeSize[];
extern u8 xglTexturePassMicrocodeProgram[];
extern u8 xglMode3MicrocodeSize[];
extern u8 xglMode3MicrocodeProgram[];
extern u8 xglMode4MicrocodeSize[];
extern u8 xglMode4MicrocodeProgram[];
extern u8 D_2C90[];
extern u8 D_3290[];
extern u8 D_3478[];
extern u8 D_39D0[];
extern u8 D_3B28[];
extern u8 D_D48[];
extern u8 D_D60[];
extern u8 D_D70[];
extern u8 D_D88[];
extern u8 D_DB0[];
extern u8 D_DD8[];
u8 *xglPacketMicrocodeEntry(int kind);

typedef unsigned int Quadword __attribute__((mode(TI)));

typedef union PxyQword {
    u64 reg[2];
    Vector4 vec;
} PxyQword;

extern u16 sRenderDepthBase;
void sceVif1PkAddUpkData128(XglPacket *packet, Quadword data);
void xglPacketUploadVu1Data(void *data, int count, int address);
float tanf(float x);
void *memset(void *s, int c, unsigned int n);
extern const u8 vuStateTable[256];
extern const u8 vuStateTableReflection[256];
extern const u8 vuSmallStateTable[32];
extern const u8 vuSmallStateTableReflection[32];
extern const u8 vuSecondSmallStateTable[32];
extern const u8 vuStateHeaderQuad[16];

#define GS_ZBUF_2 0x4F

/* One 16-byte A+D entry of the register list: two data words, then the register and its high word. */
typedef struct GsRegEntry {
    u32 dataDoubleword[2];
    u32 reg;
    u32 regHigh;
} GsRegEntry;

/* One 16-byte record of the five-record VU1 parameter block uploaded to 994: a count, an address, a float and a spare word. */
typedef struct PxyVu1ParamBlockEntry {
    u32 qwordCount;
    u32 address;
    float floatValue;
    u32 spare;
} PxyVu1ParamBlockEntry;

/* One quadword of a GIF packet, as four words (the tag) or as two register-data pairs. */
typedef union GifQword {
    u32 tagWords[4];
    u64 reg[2];
    float floats[4];
} GifQword;

u8 *xglPacketMicrocodeEntry(int mode);
void nmlPacketAddWaitMicrocode(void);

/* The packet state pxyPacketSelectMode reads: the flag word at +0x00 (bit 0x8000 clear picks the mode group that tests
 * the record), then the record at +0x14, whose flag word at +0x08 (bit 0x1 clear picks mode 4 or the mode 3/0 group). */
typedef struct PxyModeGate {
    u8 unk00[8];
    s32 gateFlags; /* +0x08 */
} PxyModeGate;

typedef struct PxyPacketModeState {
    s32 selectFlags; /* +0x00 */
    u8 unk04[0x10];
    PxyModeGate *gateRecord; /* +0x14 */
} PxyPacketModeState;

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AA9D0);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AAA00);

extern s32 pxyPendingValue;

void pxyPacketSetPendingValue(s32 pendingValue) {
    pxyPendingValue = pendingValue;
}

extern s32 pxyPendingIndex;

void pxyPacketSetPendingIndex(s32 pendingIndex) {
    pxyPendingIndex = pendingIndex;
}

extern s32 pxyPendingEnabled;

void pxyPacketEnablePending(void) {
    pxyPendingEnabled = 1;
}

void pxyPacketDisablePending(void) {
    pxyPendingEnabled = 0;
}

extern s32 pxyModeThreeEnabled;

/* Zero selects mode 0 instead of 3 inside pxyPacketSelectMode's branch where the flag at 0x551290 is zero. */
void pxyPacketSetModeFlagThree(s32 modeThreeEnabled) {
    pxyModeThreeEnabled = modeThreeEnabled;
}

extern s32 pxyModeFourEnabled;

/* Zero makes pxyPacketSelectMode choose the mode 3/0 group instead of mode 4. */
void pxyPacketSetModeFlagFour(s32 modeFourEnabled) {
    pxyModeFourEnabled = modeFourEnabled;
}

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AACD0);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AAD00);

extern s32 pxyLastArgWasZero;

void pxyPacketSetFlagIfZero(s32 testedValue) {
    if (testedValue != 0) {
        pxyLastArgWasZero = 0;
        return;
    }
    pxyLastArgWasZero = 1;
}

extern s32 pxyCachedCameraArg;
extern s32 pxyCachedCamera;
s32 xglStudioSelectGetActiveCamera();

void pxyPacketCacheCamera(s32 cameraArg) {
    pxyCachedCameraArg = cameraArg;
    pxyCachedCamera = xglStudioSelectGetActiveCamera();
}

extern s32 pxyCacheValue;

void pxyPacketSetCacheValue(s32 cacheValue) {
    pxyCacheValue = cacheValue;
}

s32 pxyPacketGetCacheValue(void) {
    return pxyCacheValue;
}

void nmlPacketClrTextureCache(void)
{
    pxyTextureTableCache = 0;
    pxyTextureChannelCache = -1;
}

extern s32 pxyMatrixCache;

void nmlPacketClrMatrixCache(void) {
    pxyMatrixCache = 0;
}

void xglPacketModeReset(void)
{
    xglPacketMode = -1;
}

void nmlPacketClrModelCache(void)
{
    pxyUploadedPacket = 0;
}

void xglPacketInitVuStateArg(int passIndex)
{
    xglPacketInitVuState();
}

void nmlPacketClrMatrixCache();

void nmlPacketClrAll(void) {
    xglPacketModeReset();
    nmlPacketClrTextureCache();
    nmlPacketClrModelCache();
    nmlPacketClrMatrixCache();
    xglPacketInitVuState();
}

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AAE70);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AB4E0);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001ABB48);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001ABCA0);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001ABD60);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AC0D0);

void pxyPacketUploadUnpack(void *unpackData, u32 quadwordCount, s32 address) {
    XglPacket *packet;

    packet = xglPacketGetCurrent();
    sceVif1PkCnt(packet, 0U);
    sceVif1PkOpenUpkCode(packet, address & 0xFFFF, 0x6CU, 1U, 1U);
    sceVif1PkAddUpkData128N(packet, unpackData, quadwordCount);
    sceVif1PkCloseUpkCode(packet);
}

extern unsigned char pxyLightColorTableMain[];
extern unsigned char pxyLightColorTableAlternate[];
extern unsigned char pxyLightParamTableMain[];
extern unsigned char pxyLightParamTableAlternate[];
extern unsigned char pxyLightExtraTable[];

void pxyPacketUploadLightTables(s32 alternate) {
    XglPacket *packet;
    u8 *lightColorTable;
    u8 *lightParamTable;

    packet = xglPacketGetCurrent();
    lightColorTable = pxyLightColorTableMain;
    lightParamTable = pxyLightParamTableMain;
    if (alternate != 0) {
        lightColorTable = pxyLightColorTableAlternate;
        lightParamTable = pxyLightParamTableAlternate;
    }
    sceVif1PkOpenUpkCode(packet, 0x35CU, 0x6CU, 1U, 1U);
    sceVif1PkAddUpkData128N(packet, lightColorTable, 0x10U);
    sceVif1PkCloseUpkCode(packet);
    sceVif1PkOpenUpkCode(packet, 0x3BAU, 0x6CU, 1U, 1U);
    sceVif1PkAddUpkData128N(packet, lightParamTable, 2U);
    sceVif1PkCloseUpkCode(packet);
    sceVif1PkOpenUpkCode(packet, 0x33CU, 0x6CU, 1U, 1U);
    sceVif1PkAddUpkData128N(packet, pxyLightExtraTable, 1U);
    sceVif1PkCloseUpkCode(packet);
}

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", xglPacketInitVuState);

extern s32 pxyModeZeroPending;
s32 func_001AAA00(s32 index);

void pxyPacketEnterPlainMode(void) {
    XglPacket *packet;
    void *programData;
    s32 programQwordCount;
    s32 mscalTag;

    packet = xglPacketGetCurrent();
    xglPacketMode = 0;
    programData = (void *) func_001AAA00(0);
    programQwordCount = func_001AAA00(1);
    sceVif1PkRef(packet, programData, programQwordCount, 0U, 0U, 0U);
    sceVif1PkCnt(packet, 0U);
    mscalTag = func_001AAA00(3);
    sceVif1PkAddCode(packet, mscalTag | 0x14000000);
    pxyModeZeroPending = 0;
}

void pxyPacketSelectMode(PxyPacketModeState *modeSelector) {
    XglPacket *packet;
    s32 packetMode;
    void *programData;
    s32 programQwordCount;

    packet = xglPacketGetCurrent();
    packetMode = 2;
    if (!(modeSelector->selectFlags & 0x8000)) {
        packetMode = 1;
        if (!(modeSelector->gateRecord->gateFlags & 1)) {
            packetMode = 4;
            if (pxyModeFourEnabled == 0) {
                packetMode = 3;
                if (pxyModeThreeEnabled == 0) {
                    packetMode = 0;
                }
            }
        }
    }
    if (xglPacketMode != packetMode) {
        xglPacketMode = packetMode;
        programData = (void *) func_001AAA00(0);
        programQwordCount = func_001AAA00(1);
        sceVif1PkRef(packet, programData, programQwordCount, 0U, 0U, 0U);
        sceVif1PkCnt(packet, 0U);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001ACB50);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001ACD30);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001ACE80);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001ACFD8);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AD190);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AD278);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AD360);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AD480);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AD5A0);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AD628);

void xglPacketAddVifFlushIrq(void) {
    XglPacket *packet;

    packet = xglPacketGetCurrent();
    sceVif1PkCnt(packet, 0U);
    sceVif1PkAddCode(packet, 0x91000000U);
}

void xglPacketAddVifFlush(void) {
    XglPacket *packet;

    packet = xglPacketGetCurrent();
    sceVif1PkCnt(packet, 0U);
    sceVif1PkAddCode(packet, 0x11000000U);
}

void xglPacketAddRefFlagged(void *refAddress, u32 qwc, u32 refControlBits) {
    XglPacket *packet;
    packet = xglPacketGetCurrent();
    sceVif1PkRef(packet, refAddress, qwc, 0U, refControlBits, 0U);
}

void xglPacketAddRef(void *refAddress, u32 qwc) {
    XglPacket *packet;
    packet = xglPacketGetCurrent();
    sceVif1PkRef(packet, refAddress, qwc, 0U, 0U, 0U);
}

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AD7D8);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AD878);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AD998);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001ADBD8);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001ADE78);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AE090);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AE680);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AE748);

void *memcpy(void *dest, const void *src, s32 size);

u8 *xglPacketCopyBytes(const void *sourceAddress, s32 byteLength) {
    XglPacket *packet;
    u8 *newCursor;

    packet = xglPacketGetCurrent();
    memcpy(packet->cursor, sourceAddress, byteLength);
    newCursor = &packet->cursor[byteLength];
    packet->cursor = newCursor;
    return newCursor - byteLength;
}

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AE870);

void xglMemCopy64(u8 *dest, const void *src, s32 count);

u8 *xglPacketCopy64(const void *sourceAddress, s32 sixtyFourByteBlockCount) {
    XglPacket *packet;
    s32 copiedBytes;
    u8 *newCursor;

    packet = xglPacketGetCurrent();
    copiedBytes = sixtyFourByteBlockCount << 6;
    xglMemCopy64(packet->cursor, sourceAddress, sixtyFourByteBlockCount);
    newCursor = packet->cursor + copiedBytes;
    packet->cursor = newCursor;
    return newCursor - copiedBytes;
}

void xglMemCopy16(u8 *dest, const void *src, s32 count);

u8 *xglPacketCopy16(const void *sourceAddress, s32 quadwordCount) {
    XglPacket *packet;
    s32 copiedBytes;
    u8 *newCursor;

    packet = xglPacketGetCurrent();
    copiedBytes = quadwordCount * 0x10;
    xglMemCopy16(packet->cursor, sourceAddress, quadwordCount);
    newCursor = packet->cursor + copiedBytes;
    packet->cursor = newCursor;
    return newCursor - copiedBytes;
}

/* Reserves count quadwords plus one in the current packet and returns the old write cursor, or 0 when the
 * reservation would run past the packet's top region. */
u8 *xglPacketReserve(int quadwordCount)
{
    XglPacket *packet;
    u8 *cursor;
    u8 *newCursor;
    u32 reservedBytes;
    u32 packetTop;
    u32 limit;
    const XglPacket *constPacket;

    packet = xglPacketGetCurrent();
    constPacket = packet;
    limit = constPacket->limit;
    packetTop = constPacket->buffer + constPacket->bufferSize;
    cursor = packet->cursor;
    if (packetTop - limit - 0x8010 - (quadwordCount << 4) <= (u32) cursor - limit) {
        return 0;
    }
    reservedBytes = (quadwordCount << 4) + 16;
    newCursor = cursor + reservedBytes;
    packet->cursor = newCursor;
    return cursor;
}

s32 xglPacketRoundUpToPowerOfTwo(s32 requestedCount) {
    if (!(requestedCount >= 3)) {
        return 2;
    }
    if (!(requestedCount >= 5)) {
        return 4;
    }
    if (!(requestedCount >= 9)) {
        return 8;
    }
    if (!(requestedCount >= 0x11)) {
        return 0x10;
    }
    if (!(requestedCount >= 0x21)) {
        return 0x20;
    }
    if (!(requestedCount >= 0x41)) {
        return 0x40;
    }
    if (!(requestedCount >= 0x81)) {
        return 0x80;
    }
    if (!(requestedCount >= 0x101)) {
        return 0x100;
    }
    if (!(requestedCount >= 0x201)) {
        return 0x200;
    }
    if (!(requestedCount >= 0x401)) {
        return 0x400;
    }
    if (!(requestedCount >= 0x801)) {
        return 0x800;
    }
    return 1;
}

s32 xglPacketCeilLogTwo(s32 requestedCount) {
    if (!(requestedCount >= 3)) {
        return 1;
    }
    if (!(requestedCount >= 5)) {
        return 2;
    }
    if (!(requestedCount >= 9)) {
        return 3;
    }
    if (!(requestedCount >= 0x11)) {
        return 4;
    }
    if (!(requestedCount >= 0x21)) {
        return 5;
    }
    if (!(requestedCount >= 0x41)) {
        return 6;
    }
    if (!(requestedCount >= 0x81)) {
        return 7;
    }
    if (!(requestedCount >= 0x101)) {
        return 8;
    }
    if (!(requestedCount >= 0x201)) {
        return 9;
    }
    if (!(requestedCount >= 0x401)) {
        return 0xA;
    }
    if (!(requestedCount >= 0x801)) {
        return 0xB;
    }
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AEB20);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AEBB0);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AEC40);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AECC0);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AECF8);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AED28);

extern s32 D_0055127C;

void pxyPacketSetSecondCount(s32 secondaryCount) {
    D_0055127C = secondaryCount;
}

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AEDC0);

extern s32 pxyPacketCount;

void pxyPacketResetCount(void) {
    pxyPacketCount = 0;
}

s32 pxyPacketGetCount(void) {
    return pxyPacketCount;
}

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AEE50);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AEF08);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AEFD0);

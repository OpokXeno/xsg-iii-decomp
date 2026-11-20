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
        u64 data;
        u64 address;
    };
    u32 word[4];
} XglDirectQueueEntry;





struct PxyPartRange;
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
    s32 words[4][4];
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
    u32 data[2];
    u32 reg;
    u32 regHigh;
} GsRegEntry;

/* One 16-byte record of the five-record VU1 parameter block uploaded to 994: a count, an address, a float and a spare word. */
typedef struct PxyVu1ParamRecord {
    u32 count;
    u32 address;
    float value;
    u32 spare;
} PxyVu1ParamRecord;

/* One quadword of a GIF packet, as four words (the tag) or as two register-data pairs. */
typedef union GifQword {
    u32 word[4];
    u64 reg[2];
    float value[4];
} GifQword;

u8 *xglPacketMicrocodeEntry(int mode);
void nmlPacketAddWaitMicrocode(void);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AA9D0);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AAA00);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AAC98);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AACA0);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AACA8);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AACB8);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AACC0);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AACC8);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AACD0);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AAD00);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AADA0);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AADB8);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AADE0);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AADE8);

void nmlPacketClrTextureCache(void)
{
    pxyTextureTableCache = 0;
    pxyTextureChannelCache = -1;
}

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", nmlPacketClrMatrixCache);

void xglPacketModeReset(void)
{
    xglPacketMode = -1;
}

void nmlPacketClrModelCache(void)
{
    pxyUploadedPacket = 0;
}

void xglPacketInitVuStateArg(int value)
{
    xglPacketInitVuState();
}

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AAE38);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AAE70);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AB4E0);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001ABB48);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001ABCA0);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001ABD60);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AC0D0);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AC478);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AC4F8);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", xglPacketInitVuState);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AC9F8);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001ACA88);

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

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AD6B8);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AD6F8);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AD738);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AD790);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AD7D8);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AD878);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AD998);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001ADBD8);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001ADE78);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AE090);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AE680);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AE748);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AE810);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AE870);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AE8C8);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AE928);

/* Reserves count quadwords plus one in the current packet and returns the old write cursor, or 0 when the
 * reservation would run past the packet's top region. */
u8 *func_001AE988(int count)
{
    XglPacket *packet;
    u8 *cursor;
    u8 *newCursor;
    u32 size;
    u8 *bufferEnd;
    u32 limit;
    const XglPacket *view;

    packet = xglPacketGetCurrent();
    view = packet;
    limit = view->limit;
    bufferEnd = (u8 *) view->buffer + view->bufferSize;
    packet->bufferSize = packet->bufferSize;
    cursor = packet->cursor;
    if (bufferEnd - limit - 0x8010 - (count << 4) <= (u32) cursor - limit) {
        return 0;
    }
    size = (count << 4) + 16;
    newCursor = cursor + size;
    *(u8 **) &packet->cursor = newCursor;
    return cursor;
}

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AEA00);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AEA90);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AEB20);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AEBB0);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AEC40);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AECC0);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AECF8);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AED28);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AEDB8);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AEDC0);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AEE40);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AEE48);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AEE50);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AEF08);

INCLUDE_ASM("asm/main/nonmatchings/pxy_packet_state", func_001AEFD0);

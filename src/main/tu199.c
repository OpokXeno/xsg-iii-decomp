#include "common.h"

#include "shared.h"
#include "main/Packet.h"
typedef int s32;

void sceVif1PkCnt(void *packet, unsigned int count);
void sceVif1PkOpenDirectHLCode(XglPacket *packet, unsigned int count);
void sceVif1PkCloseDirectHLCode(XglPacket *packet);
void gifStateReset(void);
void gifStateAddReg(int gifAddress, u64 gifData);
void gifStateFlush(void);
extern int D_00550D48;
extern u16 D_004C8390[];

/* Writes one GIF state block (A+D registers) and counts D_00550D48 down by 3 when it is
 * positive. */
void func_00245470(XglPacket *packet) {
    if (D_00550D48 > 0) {
        sceVif1PkCnt(packet, 0);
        sceVif1PkOpenDirectHLCode(packet, 0);
        gifStateReset();
        gifStateAddReg(63, 0);
        gifStateAddReg(8, 5);
        gifStateAddReg(78, (u64) 0x9880 << 17);
        gifStateAddReg(74, 1);
        gifStateAddReg(71, 0x30000);
        gifStateAddReg(66, ((u64) D_00550D48 << 32) | 100);
        gifStateAddReg(20, 96);
        gifStateAddReg(76, (u64) D_004C8390[11] | 0xff00000000080000ULL);
        gifStateAddReg(6, 0x24020000ULL | ((u64) (D_004C8390[12] << 5)) | ((u64) 0xc800 << 19));
        gifStateAddReg(1, (u64) D_00550D48 | ((u64) D_00550D48 << 8) | ((u64) D_00550D48 << 16) |
                              ((u64) D_00550D48 << 24) | 0x3f80000000000000ULL);
        gifStateAddReg(0, 342);
        gifStateAddReg(3, 0x00700070);
        gifStateAddReg(4, 0x71F76FF8);
        gifStateAddReg(3, 0x1B901F90);
        gifStateAddReg(4, 0x8DF78FF8);
        gifStateFlush();
        sceVif1PkCloseDirectHLCode(packet);
        D_00550D48 -= 3;
    }
}

extern u16 gMovieAutoSelectFlags[];
void func_0019AC80(int active);

/* Calls func_0019AC80(0) when bit 0x2 of gMovieAutoSelectFlags[0] is set. */
void func_00245608(void) {
    if (gMovieAutoSelectFlags[0] & 0x2) {
        /* kept: the do/while(0) keeps the call in its own block. */
        do {
            func_0019AC80(0);
        } while (0);
    }
}

/* Five corners (0-3 one quad, 4 the extra corner) and the enable word. */
typedef struct QuadVertexSlot {
    Vector4 corner[5];
    s32 unk_50;
    unsigned char unmodeled_54[4];
    s32 enabled;
} QuadVertexSlot;

/* Resets a QuadVertexSlot to the unit quad (x,y,z 0; w 1) and clears unk_50 and
 * sets enabled; the corner stores keep the order of the original. */
void quadVertexSlotReset(QuadVertexSlot *slot) {
    f32 zeroFloat;
    f32 oneFloat;
    zeroFloat = 0.0f;
    oneFloat = 1.0f;
    slot->enabled = 1;
    slot->corner[3].z = zeroFloat;
    slot->corner[3].w = oneFloat;
    slot->corner[0].x = zeroFloat;
    slot->corner[4].x = zeroFloat;
    slot->corner[0].y = zeroFloat;
    slot->corner[4].y = zeroFloat;
    slot->corner[0].z = zeroFloat;
    slot->corner[4].z = zeroFloat;
    slot->corner[0].w = oneFloat;
    slot->corner[4].w = oneFloat;
    slot->corner[1].x = zeroFloat;
    slot->corner[2].x = zeroFloat;
    slot->corner[1].y = zeroFloat;
    slot->corner[2].y = zeroFloat;
    slot->corner[1].z = zeroFloat;
    slot->corner[2].z = zeroFloat;
    slot->corner[1].w = oneFloat;
    slot->corner[2].w = oneFloat;
    /* kept: the do/while(0) is needed by the match (removing it changes the code). */
    do {
        slot->corner[3].x = oneFloat;
        slot->corner[3].y = zeroFloat;
        slot->unk_50 = 0;
    } while (0);
}

#include "common.h"
#include "shared.h"

int xglMcMain(void);
void xglSleep(void);
void func_00A820D0(void);
void func_00182708(const char *text);
extern const char D_00A852E0[];

/* Device check of the logo screen: runs the memory-card query, then prints the device-check text. */
int device_check_sync(void) {
    int status;

    status = xglMcMain();
    func_00A820D0();
    xglSleep();
    func_00182708(D_00A852E0);
    return status;
}

INCLUDE_ASM("asm/nonmatchings/ov01/tu001", func_00A80050);

/* Card-check gate of the logo screen: always returns 1 in this build, so the card check always runs. */
int hdd_check(void) {
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/ov01/tu001", func_00A800D0);

INCLUDE_ASM("asm/nonmatchings/ov01/tu001", func_00A800F8);

INCLUDE_ASM("asm/nonmatchings/ov01/tu001", func_00A804A0);

INCLUDE_ASM("asm/nonmatchings/ov01/tu001", func_00A80668);

/* A GS direct-data buffer of the title frame: the tag dword at +0x30 and two words at +0x70 and +0x74. */
typedef struct TitleVifBuffer {
    unsigned char unmodeled_00[0x30];
    u64 gs_tag;
    unsigned char unmodeled_38[0x38];
    u32 ofs_70;
    u32 ofs_74;
} TitleVifBuffer;

/* The render-state halfwords read here: the frame width at +0x0c, the height at +0x0e, the display base at +0x16. */
typedef struct TitleRenderBase {
    unsigned char unmodeled_00[0x0c];
    unsigned short width;
    unsigned short height;
    unsigned char unmodeled_10[0x06];
    unsigned short display_base;
} TitleRenderBase;

/* The splat piece D_00A829B0: the blank-row words at +0x40..+0x4c and a GS buffer at +0x70. */
typedef struct TitleVifPiece {
    unsigned char unmodeled_00[0x40];
    u32 blank_rows[4];
    unsigned char unmodeled_50[0x20];
    TitleVifBuffer buffer;
} TitleVifPiece;

void *xglPacketGetCurrent(void);
void sceVif1PkCnt(void *packet, unsigned int count);
void sceVif1PkAddDataN(void *packet, const void *data, unsigned int qwc);
void sceVif1PkRef(void *packet, const void *data, unsigned int qwc, unsigned int a3, unsigned int a4, unsigned int a5);
extern TitleRenderBase D_4C8390;
extern TitleVifPiece D_00A829B0;
extern TitleVifBuffer D_00A82AB0;
extern TitleVifBuffer D_00A82B30;
extern u64 D_00A82900[];
extern u64 D_00A82960[];
extern u64 D_00A82980[];
extern u8 D_00A82861;

/* The title fade state: the fade level is a halfword at +0x28. */
typedef struct TitleFadeState {
    unsigned char unmodeled_00[0x28];
    short fade_level;
} TitleFadeState;

extern TitleFadeState D_00A853B0;
extern u64 D_00A83160[];

/* Credit roll band: sends the header and the two credit halves of the band, then the blank rows below it. */
void logoDrawCreditBand(u8 *creditBand, int rowsDrawn) {
    void *packet;

    packet = xglPacketGetCurrent();
    if (creditBand != 0) {
        D_00A82900[4] = ((u64)D_4C8390.display_base << 37) | ((u64)0x8000 << 36);
        sceVif1PkCnt(packet, 0);
        sceVif1PkAddDataN(packet, D_00A82900, 0x18);
        sceVif1PkRef(packet, D_00A82960, 2, 0, 0, 0);
        sceVif1PkRef(packet, creditBand, 0x7000, 0, 0x51007000, 0);
        sceVif1PkRef(packet, D_00A82960, 2, 0, 0, 0);
        sceVif1PkRef(packet, creditBand + 0x70000, 0x7000, 0, 0x51007000, 0);
        sceVif1PkRef(packet, D_00A82980, 3, 0, 0, 0);
    }
    if (rowsDrawn < 128) {
        D_00A829B0.blank_rows[3] = 128 - rowsDrawn;
        D_00A829B0.blank_rows[2] = D_00A82861;
        D_00A829B0.blank_rows[1] = D_00A82861;
        D_00A829B0.blank_rows[0] = D_00A82861;
        sceVif1PkCnt(packet, 0);
        sceVif1PkAddDataN(packet, &D_00A829B0, 28);
    }
}

INCLUDE_ASM("asm/nonmatchings/ov01/tu001", func_00A80A28);

int titleGroupMinus2Dummy(void) {
    return 0;
}

void func_00A80D98(void) {
    void *packet;
    TitleVifBuffer *buffer;
    unsigned int tagBit;

    tagBit = 0x80000;

    packet = xglPacketGetCurrent();
    buffer = (TitleVifBuffer *)((u8 *)&D_00A829B0 + 0xB0);
    buffer->gs_tag = 0x24020000 | (D_4C8390.width << 5) | ((u64)0xE400 << 20);
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddDataN(packet, buffer, 20);
    D_00A82AB0.gs_tag = D_4C8390.height | tagBit;
    D_00A82AB0.ofs_70 = 0x8FF8;
    D_00A82AB0.ofs_74 = 0x8DF7;
    sceVif1PkAddDataN(packet, &D_00A82AB0, 32);
    D_00A82AB0.gs_tag = 0x400C0;
    D_00A82AB0.ofs_70 = 32760;
    D_00A82AB0.ofs_74 = 32759;
    sceVif1PkAddDataN(packet, &D_00A82AB0, 32);
    D_00A82B30.gs_tag = (u64)(D_4C8390.width | tagBit);
    sceVif1PkAddDataN(packet, &D_00A82B30, 16);
}

void titleFrameHookDummy(void) {

}

INCLUDE_ASM("asm/nonmatchings/ov01/tu001", func_00A80EB0);

INCLUDE_ASM("asm/nonmatchings/ov01/tu001", func_00A80EF8);

INCLUDE_ASM("asm/nonmatchings/ov01/tu001", func_00A81020);

INCLUDE_ASM("asm/nonmatchings/ov01/tu001", func_00A810A0);

/* Title fade: steps the fade level down to zero, sends the fade packet on each pass, then draws the frame. */
void titleFadeTransition(int mode) {
    void *packet;
    int fullLevel;

    D_00A853B0.fade_level = 0x80;
    if (mode < 0) {
        D_00A853B0.fade_level = 1;
    }
    if (D_00A853B0.fade_level > 0) {
        do {
            fullLevel = 0x80;
        packet = xglPacketGetCurrent();
            if (mode == 0) {
                D_00A853B0.fade_level -= 16;
            } else {
                D_00A853B0.fade_level -= 4;
            }
            if (D_00A853B0.fade_level < 0) {
                D_00A853B0.fade_level = 0;
            }
            D_00A83160[6] = 0x24120000 | (D_4C8390.height << 5) | ((u64)0xC800 << 19);
            if (mode == 0) {
                ((int *)D_00A83160)[18] = D_00A853B0.fade_level;
                ((int *)D_00A83160)[17] = D_00A853B0.fade_level;
                ((int *)D_00A83160)[16] = D_00A853B0.fade_level;
            } else {
                int progress = fullLevel - D_00A853B0.fade_level;
                int scaled = progress * 3;

                ((int *)D_00A83160)[18] = D_00A853B0.fade_level;
                ((int *)D_00A83160)[16] = fullLevel - progress * 2;
                if (((int *)D_00A83160)[16] < 0) {
                    ((int *)D_00A83160)[16] = 0;
                }
                ((int *)D_00A83160)[17] = fullLevel - scaled / 2;
                if (((int *)D_00A83160)[17] < 0) {
                    ((int *)D_00A83160)[17] = 0;
                }
            }
            sceVif1PkCnt(packet, 0);
            sceVif1PkAddDataN(packet, D_00A83160, 0x24);
            xglSleep();
            if (D_00A853B0.fade_level == 0) {
                sceVif1PkCnt(packet, 0);
                sceVif1PkAddDataN(packet, D_00A83160, 0x24);
                xglSleep();
            }
        } while (D_00A853B0.fade_level > 0);
    }
    func_00A80D98();
    xglSleep();
}

INCLUDE_ASM("asm/nonmatchings/ov01/tu001", func_00A816A8);

INCLUDE_ASM("asm/nonmatchings/ov01/tu001", func_00A81718);

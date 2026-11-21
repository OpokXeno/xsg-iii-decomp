#include "common.h"

typedef int s32;
typedef float f32;
typedef unsigned char u8;
typedef signed char s8;
void njPlaneSetOffset();
void njPlaneSetOrigin(s32 planeElement, f32 scale, f32 y);

typedef struct PlaneHost {
    f32 offset_x;           /* 0x00 */
    f32 offset_y;           /* 0x04 */
    u8 unk_08[0x8];         /* 0x08 */
    f32 level;              /* 0x10 */
    f32 unk_14;             /* 0x14 */
    u8 unk_18[0x8];         /* 0x18 */
    u8 plane[0xC0];         /* 0x20, njPlane record passed to njPlaneInit */
    s32 unk_E0;             /* 0xE0 */
    s32 unk_E4;             /* 0xE4 */
    u8 unk_E8[0x10];        /* 0xE8 */
    s32 channelHandle;             /* 0xF8 */
    s32 unk_FC;             /* 0xFC */
    u8 unk_100[0xD0];       /* 0x100 */
    u8 planeMode;                /* 0x1D0 */
} PlaneHost;

void njPlaneSetPlacement(s32 planeElement, s32 x, s32 y) {
    njPlaneSetOffset();
    njPlaneSetOrigin(planeElement, (f32) x, (f32) y);
}

void njPlaneSetTint(f32 red, f32 green, f32 blue);

void njPlaneSetTintInt(s32 unused_arg0, s32 x, s32 y, s32 z) {
    njPlaneSetTint((f32) x, (f32) y, (f32) z);
}

void njPlaneSetLevel(f32 level);

void njPlaneSetLevelInt(s32 unused_arg0, s32 level) {
    njPlaneSetLevel((f32) level);
}

void planeHostSetMode(PlaneHost *host, u8 planeMode) {
    host->planeMode = planeMode;
}

void func_002D0F50(PlaneHost *host, u8 mode);

void planeHostFlushMode(PlaneHost *host) {
    func_002D0F50(host, host->planeMode);
}

INCLUDE_ASM("asm/main/nonmatchings/tu384", func_002C8F48);

s32 planeHostGetLevel(PlaneHost *host) {
    return (s32) host->level;
}

s32 planeHostGetSecondaryValue(PlaneHost *host) {
    return (s32) host->unk_14;
}

void njPlaneInit(u8 *plane);

void planeHostInit(PlaneHost *host) {
    host->offset_x = 0;
    host->offset_y = 0;
    host->level = 0;
    host->unk_14 = 0;
    host->unk_E4 = 0;
    host->unk_E0 = 0;
    njPlaneInit(host->plane);
    host->unk_FC = 0;
}

void planeHostSetChannelHandle(PlaneHost *host, s32 channelHandle) {
    if (channelHandle != 0) {
        host->channelHandle = channelHandle;
    }
}

void planeHostSetWordFC(PlaneHost *host, s32 value) {
    host->unk_FC = value;
}

void planeHostSetLevel(PlaneHost *host, f32 level) {
    host->level = level;
}

void planeHostSetWordE4(PlaneHost *host, s32 value) {
    host->unk_E4 = value;
}

void planeHostSetWordE0(PlaneHost *host, s32 value) {
    host->unk_E0 = value;
}

void planeHostSetOffset(PlaneHost *host, f32 x, f32 y) {
    host->offset_x = x;
    host->offset_y = y;
}

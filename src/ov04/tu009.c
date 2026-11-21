/*
 * OV04 original TU 9: 0x00a9a2a0..0x00a9a640 (3 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov04/tu009", func_00A9A2A0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu009", func_00A9A3C8);

typedef signed char s8;
typedef int s32;
typedef float f32;

typedef struct EdgeVertexBuffer {
    s32 unk_00;  /* 0x00 */
    s32 unk_04;  /* 0x04 */
    s32 unk_08;  /* 0x08 */
    f32 unk_0C;  /* 0x0C */
    s32 unk_10;  /* 0x10 */
    s32 unk_14;  /* 0x14 */
    s32 unk_18;  /* 0x18 */
    f32 unk_1C;  /* 0x1C */
    s32 unk_20;  /* 0x20 */
    s32 unk_24;  /* 0x24 */
    s32 unk_28;  /* 0x28 */
    f32 unk_2C;  /* 0x2C */
    s32 unk_30;  /* 0x30 */
    s32 unk_34;  /* 0x34 */
    s32 unk_38;  /* 0x38 */
    f32 unk_3C;  /* 0x3C */
    s32 unk_40;  /* 0x40 */
    s32 unk_44;  /* 0x44 */
    s32 unk_48;  /* 0x48 */
    f32 unk_4C;  /* 0x4C */
    s32 unk_50;  /* 0x50 */
    s32 unk_54;  /* 0x54 */
    s32 unk_58;  /* 0x58 */
    f32 unk_5C;  /* 0x5C */
    s32 unk_60;  /* 0x60 */
    s32 unk_64;  /* 0x64 */
    s32 unk_68;  /* 0x68 */
    f32 unk_6C;  /* 0x6C */
    f32 unk_70;  /* 0x70 */
    f32 unk_74;  /* 0x74 */
    f32 unk_78;  /* 0x78 */
    f32 unk_7C;  /* 0x7C */
    f32 unk_80;  /* 0x80 */
    f32 unk_84;  /* 0x84 */
    f32 unk_88;  /* 0x88 */
    f32 unk_8C;  /* 0x8C */
    f32 unk_90;  /* 0x90 */
    f32 unk_94;  /* 0x94 */
    f32 unk_98;  /* 0x98 */
    f32 unk_9C;  /* 0x9C */
    f32 unk_A0;  /* 0xA0 */
    f32 unk_A4;  /* 0xA4 */
    f32 unk_A8;  /* 0xA8 */
    f32 unk_AC;  /* 0xAC */
    f32 unk_B0;  /* 0xB0 */
    f32 unk_B4;  /* 0xB4 */
    f32 unk_B8;  /* 0xB8 */
    f32 unk_BC;  /* 0xBC */
    f32 unk_C0;  /* 0xC0 */
    f32 unk_C4;  /* 0xC4 */
    f32 unk_C8;  /* 0xC8 */
    f32 unk_CC;  /* 0xCC */
    f32 unk_D0;  /* 0xD0 */
    f32 unk_D4;  /* 0xD4 */
    f32 unk_D8;  /* 0xD8 */
    f32 unk_DC;  /* 0xDC */
    f32 unk_E0;  /* 0xE0 */
    f32 unk_E4;  /* 0xE4 */
    f32 unk_E8;  /* 0xE8 */
    f32 unk_EC;  /* 0xEC */
    s32 unk_F0;  /* 0xF0 */
    s32 unk_F4;  /* 0xF4 */
    s32 unk_F8;  /* 0xF8 */
    s32 unk_FC;  /* 0xFC */
    s32 unk_100;  /* 0x100 */
    s32 unk_104;  /* 0x104 */
    s32 unk_108;  /* 0x108 */
    s32 unk_10C;  /* 0x10C */
    s32 unk_110;  /* 0x110 */
    s32 unk_114;  /* 0x114 */
    s32 unk_118;  /* 0x118 */
    s32 unk_11C;  /* 0x11C */
    s32 unk_120;  /* 0x120 */
    s32 unk_124;  /* 0x124 */
    s32 unk_128;  /* 0x128 */
    f32 unk_12C;  /* 0x12C */
    s32 unk_130;  /* 0x130 */
    s32 unk_134;  /* 0x134 */
    s32 unk_138;  /* 0x138 */
    f32 unk_13C;  /* 0x13C */
    s32 unk_140;  /* 0x140 */
    s32 unk_144;  /* 0x144 */
    s32 unk_148;  /* 0x148 */
    f32 unk_14C;  /* 0x14C */
    s32 unk_150;  /* 0x150 */
    s32 unk_154;  /* 0x154 */
    s32 unk_158;  /* 0x158 */
    f32 unk_15C;  /* 0x15C */
    s32 unk_160;  /* 0x160 */
    s32 unk_164;  /* 0x164 */
    s32 unk_168;  /* 0x168 */
    f32 unk_16C;  /* 0x16C */
    s32 unk_170;  /* 0x170 */
    s32 unk_174;  /* 0x174 */
    s32 unk_178;  /* 0x178 */
    f32 unk_17C;  /* 0x17C */
} EdgeVertexBuffer;

extern EdgeVertexBuffer D_00AAD0F0;
extern EdgeVertexBuffer D_00AAD270;
extern EdgeVertexBuffer gEdgeVertexBuffers;

extern EdgeVertexBuffer D_00AAD0F0;
extern EdgeVertexBuffer D_00AAD270;
extern EdgeVertexBuffer gEdgeVertexBuffers;

/* Sets the defaults of the three edge vertex buffers (0x180 bytes each = 24 homogeneous vertices): zero positions with w = 1.0, white 1.0 entries and 0.8 at 0xD0..0xD8. */
void ov04InitEntryDefaults(void)
{
    EdgeVertexBuffer *vertexBuffer;
    s32 edgeBufferPass;

    vertexBuffer = 0;
    edgeBufferPass = 0;
    do {
        switch (edgeBufferPass) {
          case 0:
            vertexBuffer = &D_00AAD0F0;
            break;

          case 2:
            vertexBuffer = &gEdgeVertexBuffers;
            break;

          case 1:
            /* Both arms store the same vertexBuffer; the compiler's branch layout depends on this if/else. */
            if (vertexBuffer) {
                vertexBuffer = &D_00AAD270;
                break;
            } else {
                vertexBuffer = &D_00AAD270;
                break;
            }
        }

        edgeBufferPass += 1;
        vertexBuffer->unk_00 = 0;
        vertexBuffer->unk_04 = 0;
        vertexBuffer->unk_08 = 0;
        vertexBuffer->unk_0C = 1.0f;
        vertexBuffer->unk_10 = 0;
        vertexBuffer->unk_14 = 0;
        vertexBuffer->unk_18 = 0;
        vertexBuffer->unk_1C = 1.0f;
        vertexBuffer->unk_20 = 0;
        vertexBuffer->unk_24 = 0;
        vertexBuffer->unk_28 = 0;
        vertexBuffer->unk_2C = 1.0f;
        vertexBuffer->unk_30 = 0;
        vertexBuffer->unk_34 = 0;
        vertexBuffer->unk_38 = 0;
        vertexBuffer->unk_3C = 1.0f;
        vertexBuffer->unk_48 = (vertexBuffer->unk_44 = (vertexBuffer->unk_40 = 0));
        vertexBuffer->unk_4C = 1.0f;
        vertexBuffer->unk_50 = 0;
        vertexBuffer->unk_54 = 0;
        vertexBuffer->unk_58 = 0;
        vertexBuffer->unk_5C = 1.0f;
        vertexBuffer->unk_60 = 0;
        vertexBuffer->unk_64 = 0;
        vertexBuffer->unk_68 = 0;
        vertexBuffer->unk_6C = 1.0f;
        vertexBuffer->unk_70 = 1.0f;
        vertexBuffer->unk_74 = 1.0f;
        vertexBuffer->unk_78 = 1.0f;
        vertexBuffer->unk_7C = 1.0f;
        vertexBuffer->unk_80 = 1.0f;
        vertexBuffer->unk_84 = 1.0f;
        vertexBuffer->unk_88 = 1.0f;
        vertexBuffer->unk_8C = 1.0f;
        vertexBuffer->unk_90 = 1.0f;
        vertexBuffer->unk_94 = 1.0f;
        vertexBuffer->unk_98 = 1.0f;
        vertexBuffer->unk_9C = 1.0f;
        vertexBuffer->unk_A0 = 1.0f;
        vertexBuffer->unk_A4 = 1.0f;
        vertexBuffer->unk_A8 = 1.0f;
        vertexBuffer->unk_AC = 1.0f;
        vertexBuffer->unk_B0 = 1.0f;
        vertexBuffer->unk_B4 = 1.0f;
        vertexBuffer->unk_B8 = 1.0f;
        vertexBuffer->unk_BC = 1.0f;
        vertexBuffer->unk_C0 = 1.0f;
        vertexBuffer->unk_C4 = 1.0f;
        vertexBuffer->unk_C8 = 1.0f;
        vertexBuffer->unk_CC = 1.0f;
        vertexBuffer->unk_D0 = 0.8f;
        vertexBuffer->unk_D4 = 0.8f;
        vertexBuffer->unk_D8 = 0.8f;
        vertexBuffer->unk_DC = 1.0f;
        vertexBuffer->unk_E0 = 1.0f;
        vertexBuffer->unk_E4 = 1.0f;
        vertexBuffer->unk_E8 = 1.0f;
        vertexBuffer->unk_EC = 1.0f;
        vertexBuffer->unk_F0 = 0;
        vertexBuffer->unk_F4 = 0;
        vertexBuffer->unk_F8 = 0;
        vertexBuffer->unk_FC = 0;
        vertexBuffer->unk_100 = 0;
        vertexBuffer->unk_104 = 0;
        vertexBuffer->unk_108 = 0;
        vertexBuffer->unk_10C = 0;
        vertexBuffer->unk_110 = 0;
        vertexBuffer->unk_114 = 0;
        vertexBuffer->unk_118 = 0;
        vertexBuffer->unk_11C = 0;
        vertexBuffer->unk_120 = 0;
        vertexBuffer->unk_124 = 0;
        vertexBuffer->unk_128 = 0;
        vertexBuffer->unk_12C = 1.0f;
        vertexBuffer->unk_130 = 0;
        vertexBuffer->unk_134 = 0;
        vertexBuffer->unk_138 = 0;
        vertexBuffer->unk_13C = 1.0f;
        vertexBuffer->unk_140 = 0;
        vertexBuffer->unk_144 = 0;
        vertexBuffer->unk_148 = 0;
        vertexBuffer->unk_14C = 1.0f;
        vertexBuffer->unk_150 = 0;
        vertexBuffer->unk_154 = 0;
        vertexBuffer->unk_158 = 0;
        vertexBuffer->unk_15C = 1.0f;
        vertexBuffer->unk_160 = 0;
        vertexBuffer->unk_164 = 0;
        vertexBuffer->unk_168 = 0;
        vertexBuffer->unk_16C = 1.0f;
        vertexBuffer->unk_170 = 0;
        vertexBuffer->unk_174 = 0;
        vertexBuffer->unk_178 = 0;
        vertexBuffer->unk_17C = 1.0f;
    } while (edgeBufferPass < 3);
}

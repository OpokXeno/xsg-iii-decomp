#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4668);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E47F8);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4820);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4858);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4998);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E49D0);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4A08);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4A58);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4A90);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4AC8);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4B00);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4B50);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4B68);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4BA8);

struct Tu401Globals {
    float origin_x;
    unsigned char unmodeled_0004[0x262C - 0x4];
    int span_a;
    int span_b;
};

extern struct Tu401Globals D_00609040;
extern float D_005514F8;
extern float D_005514FC;
float xglSin(float angle);
double fptodp(float value);
double func_00309E20(double lhs, double rhs);
float func_0030A4E0(double value);
void func_002CFA88(void *element);
void func_002CFB50(void *element, float x, float y);
void func_002CFC10(void *element, float scale, float y);
void func_002CFD48(void *element, float a, float b, float c);
void func_002CFE88(void *element, float value);
void func_002CFF10(void *element, int count);
void func_002D10D0(void *element);

void func_002E4BB8(void)
{
    unsigned char buf[8][192];
    float diff;
    int i;

    D_005514F8 = func_0030A4E0(func_00309E20(fptodp(xglSin(D_005514FC)), 1.5)) * 8.0f;
    diff = (float)(D_00609040.span_b - D_00609040.span_a);
    if (!(diff < 0.0f)) {
        for (i = 0; i < 8; i++) {
            func_002CFA88(buf[i]);
            func_002CFD48(buf[i], 89.0f, 232.0f, 209.0f);
            func_002CFE88(buf[i], D_005514F8);
            func_002CFF10(buf[i], 32);
            func_002CFC10(buf[i], 6.0f, diff);
        }
        func_002CFB50(buf[0], D_00609040.origin_x + 17.0f - 2.0f, (float)D_00609040.span_a);
        func_002CFB50(buf[1], D_00609040.origin_x + 17.0f, (float)(D_00609040.span_a - 2));
        func_002CFB50(buf[2], D_00609040.origin_x + 17.0f + 2.0f, (float)D_00609040.span_a);
        func_002CFB50(buf[3], D_00609040.origin_x + 17.0f, (float)(D_00609040.span_a + 2));
        func_002CFB50(buf[4], D_00609040.origin_x + 17.0f - 1.0f, (float)(D_00609040.span_a - 1));
        func_002CFB50(buf[5], D_00609040.origin_x + 17.0f + 1.0f, (float)(D_00609040.span_a - 1));
        func_002CFB50(buf[6], D_00609040.origin_x + 17.0f + 1.0f, (float)(D_00609040.span_a + 1));
        func_002CFB50(buf[7], D_00609040.origin_x + 17.0f - 1.0f, (float)(D_00609040.span_a + 1));
        for (i = 0; i < 8; i++) {
            func_002D10D0(buf[i]);
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4E38);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E5008);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E51B8);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E51E0);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E51F0);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E56A0);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E5908);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E5E50);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E5F80);

/*
 * OV02 original TU 53: 0x00a95088..0x00a958d0 (15 functions)
 */
#include "common.h"

typedef int s32;
typedef unsigned char u8;
typedef signed char s8;
extern s32 D_00AC34A8;

/* Same layout as struct BoostGauge in ov02/boost_gauge.c. */
struct BoostGauge {
    signed char maxStock;                   /* +0x00 */
    signed char stock;                      /* +0x01 */
    signed char progress;                   /* +0x02 */
};

void sbLastResultClear(void) {
    D_00AC34A8 = 0;
}

s32 sbLastResultGet(void) {
    return D_00AC34A8;
}

extern unsigned char gValueThreadName[];
s32 sbScriptImageGet();
int sbThreadEnqueueByName(s32 thread, unsigned char *name, s32 *message);

void sbThreadPostValue(s32 postedValue) {
    s32 messageValue;
    s32 threadId;

    messageValue = postedValue;
    threadId = sbScriptImageGet();
    sbThreadEnqueueByName(threadId, gValueThreadName, &messageValue);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu053", func_00A950D8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu053", func_00A95160);

INCLUDE_ASM("asm/nonmatchings/ov02/tu053", func_00A95258);

INCLUDE_ASM("asm/nonmatchings/ov02/tu053", func_00A95288);

INCLUDE_ASM("asm/nonmatchings/ov02/tu053", func_00A95300);

INCLUDE_ASM("asm/nonmatchings/ov02/tu053", func_00A95350);

INCLUDE_ASM("asm/nonmatchings/ov02/tu053", func_00A95470);

INCLUDE_ASM("asm/nonmatchings/ov02/tu053", func_00A955D8);

struct BoostGauge *boostGaugeAt(s32 index);

void boostGaugeSetField(s32 gaugeIndex, s32 gaugeFieldId, s32 gaugeByteValue) {
    struct BoostGauge *gauge;

    gauge = boostGaugeAt(gaugeIndex);
    switch (gaugeFieldId) {
    case 0:
        gauge->maxStock = gaugeByteValue;
        return;
    case 1:
        gauge->stock = gaugeByteValue;
        break;
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu053", func_00A95678);

INCLUDE_ASM("asm/nonmatchings/ov02/tu053", func_00A956C8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu053", func_00A95708);

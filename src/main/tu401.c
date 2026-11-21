#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4668);

struct PlaneRingLayout {
    float ringOriginX;
    unsigned char unmodeled_0004[0x262C - 0x4];
    int ringSpanStart;
    int ringSpanEnd;
};
typedef int s32;
typedef float f32;
typedef unsigned char u8;
/* The word at gPublishMaskStorage (an unsized table in the original: kept unsized so the code keeps its addressing). */
#define gPublishMask (*(s32 *) gPublishMaskStorage)
extern unsigned char gPublishMaskStorage[];

void publishMaskSet(s32 publishEnabled) {
    if (publishEnabled != 0) {
        gPublishMask = 0xFFFF;
        return;
    }
    gPublishMask = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4820);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4858);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4998);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E49D0);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4A08);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4A58);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4A90);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4AC8);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4B00);

s32 slotUsedFlagSet();

void slotMarkUsed(s32 usedSlotId) {
    slotUsedFlagSet(usedSlotId, 1);
}

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4B68);

#define gPublishedCount (*(s32 *) gPublishedCountStorage)
extern unsigned char gPublishedCountStorage[];

void publishedCountSet(s32 newPublishedCount) {
    gPublishedCount = newPublishedCount;
}








extern struct PlaneRingLayout D_00609040;
extern float gPlaneRingLevel;
extern float D_005514FC;
float xglSin(float angle);
double fptodp(float value);
double func_00309E20(double lhs, double rhs);
float func_0030A4E0(double value);
#include "main/njPlane.h"
void njPlaneInit(NjPlane *element);
void njPlaneSetOffset(NjPlane *element, float x, float y);
void njPlaneSetOrigin(NjPlane *element, float scale, float y);
void njPlaneSetLevel(NjPlane *element, float value);
void njPlaneSetMode(NjPlane *element, int count);
void func_002D10D0(NjPlane *element);

/* One plane of the ring: the NjPlane record at +0x00, then 0x40 bytes this TU does not model (the ring stride is 0xC0). */
typedef struct PlaneRingSlot {
    NjPlane plane;
    unsigned char unmodeled_80[0x40];
} PlaneRingSlot;

void planeRingDraw(void)
{
    PlaneRingSlot planes[8];
    float spanLength;
    int planeIndex;

    gPlaneRingLevel = func_0030A4E0(func_00309E20(fptodp(xglSin(D_005514FC)), 1.5)) * 8.0f;
    spanLength = D_00609040.ringSpanEnd - D_00609040.ringSpanStart;
    if (!(spanLength < 0.0f)) {
        for (planeIndex = 0; planeIndex < 8; planeIndex++) {
            njPlaneInit(&planes[planeIndex].plane);
            njPlaneSetTint(&planes[planeIndex].plane, 89.0f, 232.0f, 209.0f);
            njPlaneSetLevel(&planes[planeIndex].plane, gPlaneRingLevel);
            njPlaneSetMode(&planes[planeIndex].plane, 32);
            njPlaneSetOrigin(&planes[planeIndex].plane, 6.0f, spanLength);
        }
        njPlaneSetOffset(&planes[0].plane, D_00609040.ringOriginX + 17.0f - 2.0f, D_00609040.ringSpanStart);
        njPlaneSetOffset(&planes[1].plane, D_00609040.ringOriginX + 17.0f, D_00609040.ringSpanStart - 2);
        njPlaneSetOffset(&planes[2].plane, D_00609040.ringOriginX + 17.0f + 2.0f, D_00609040.ringSpanStart);
        njPlaneSetOffset(&planes[3].plane, D_00609040.ringOriginX + 17.0f, D_00609040.ringSpanStart + 2);
        njPlaneSetOffset(&planes[4].plane, D_00609040.ringOriginX + 17.0f - 1.0f, D_00609040.ringSpanStart - 1);
        njPlaneSetOffset(&planes[5].plane, D_00609040.ringOriginX + 17.0f + 1.0f, D_00609040.ringSpanStart - 1);
        njPlaneSetOffset(&planes[6].plane, D_00609040.ringOriginX + 17.0f + 1.0f, D_00609040.ringSpanStart + 1);
        njPlaneSetOffset(&planes[7].plane, D_00609040.ringOriginX + 17.0f - 1.0f, D_00609040.ringSpanStart + 1);
        for (planeIndex = 0; planeIndex < 8; planeIndex++) {
            func_002D10D0(&planes[planeIndex].plane);
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E4E38);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E5008);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E51B8);

#define gRecordWord (*(s32 *) D_0060B67C)
extern unsigned char D_0060B67C[];

void recordWordSet(s32 value) {
    gRecordWord = value;
}

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E51F0);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E56A0);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E5908);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E5E50);

INCLUDE_ASM("asm/main/nonmatchings/tu401", func_002E5F80);

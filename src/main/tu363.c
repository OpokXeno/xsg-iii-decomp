#include "common.h"

typedef signed char s8;
typedef unsigned char u8;
typedef int s32;
typedef float f32;
typedef short s16;

/* One 0xCF0-byte slot record: the three slots at D_005C25C0 (stride 0xCF0) and the global record gFallbackSlotRecord. */
typedef struct SlotRecord {
    f32 x;                  /* 0x000 */
    f32 y;                  /* 0x004 */
    s32 pairFirst;             /* 0x008 */
    s32 pairSecond;             /* 0x00C */
    u8 text[0x118];         /* 0x010, njString object handle */
    s32 pairAFirst;            /* 0x128 */
    s32 pairASecond;            /* 0x12C */
    u8 unk_130[0x8];        /* 0x130 */
    s32 pairBFirst;            /* 0x138 */
    s32 pairBSecond;            /* 0x13C */
    u8 unk_140[0x10];       /* 0x140 */
    f32 home_x;             /* 0x150 */
    f32 home_y;             /* 0x154 */
    u8 unk_158[0x18];       /* 0x158 */
    f32 label_x;            /* 0x170 */
    f32 label_y;            /* 0x174 */
    u8 unk_178[0x8];        /* 0x178 */
    f32 anchor_x;           /* 0x180 */
    f32 anchor_y;           /* 0x184 */
    u8 unk_188[0x7A8];      /* 0x188 */
    u8 plane_a[0x80];       /* 0x930, NjPlane (main/njPlane.h) */
    u8 unk_9B0[0x40];       /* 0x9B0 */
    u8 plane_b[0x80];       /* 0x9F0, NjPlane (main/njPlane.h) */
    u8 unk_A70[0x48];       /* 0xA70 */
    u8 label[0x214];        /* 0xAB8, second string object handle */
    s16 resetHalf;            /* 0xCCC */
    s16 level;              /* 0xCCE, clamped to 0..128 */
    u8 unk_CD0[0xC];        /* 0xCD0 */
    s16 draw_count;         /* 0xCDC */
    u8 unk_CDE[0x2];        /* 0xCDE */
    s8 displayMode;                /* 0xCE0 */
    u8 unk_CE1[0x6];        /* 0xCE1 */
    s8 resetByteA;             /* 0xCE7 */
    s8 resetByteB;             /* 0xCE8 */
    u8 unk_CE9[0x7];        /* 0xCE9 */
} SlotRecord;

/* The word-0 view of one 0xCF0-byte record in the slot tables (stride 0xCF0 = sizeof(SlotRecord)). */
typedef struct SlotCell {
    s32 headWord;            /* 0x000 */
    u8 unk_04[0xCEC];       /* 0x004 */
} SlotCell;

/* Shared state block at D_005D0670; only its tail words are used here (tu364 reads 0x84C..0x850). */
typedef struct MrPackStateBlock {
    u8 unk_000[0x854];      /* 0x000 */
    s32 tail[4];            /* 0x854 */
} MrPackStateBlock;

void slotSetLevel(SlotRecord *levelSlot, f32 level);
void slotSetPosition(SlotRecord *positionedSlot, f32 x, f32 y);
extern s32 gSlotLayoutTableA[];
extern s32 gSlotLayoutTableB[];
extern f32 gSlotLevelScaleB;
extern f32 gSlotLevelScaleA;
extern SlotRecord D_005C25C0[];
s32 func_002B1948(SlotRecord *slot, f32, f32);

void slotLayoutUpdate(s32 layoutIndex, s32 useSecondTable) {
    f32 layoutIndexFloat;
    f32 min_0;
    f32 min_1;
    f32 min_2;
    f32 offset_0;
    f32 offset_1;
    f32 pos_0;
    f32 pos_1;
    f32 pos_2;
    f32 levelScale;
    SlotRecord *secondVisibleSlot;
    SlotRecord *thirdVisibleSlot;

    if (useSecondTable != 0) {
        f32 layoutIndexFloat;
        f32 min_0;
        f32 min_1;
        f32 min_2;
        f32 offset_0;
        f32 offset_1;
        f32 pos_0;
        f32 pos_1;
        f32 pos_2;
        f32 levelScale;
        SlotRecord *secondVisibleSlot;
        SlotRecord *thirdVisibleSlot;

        layoutIndexFloat = (f32) layoutIndex;
        min_0 = (f32) gSlotLayoutTableB[0];
        offset_0 = layoutIndexFloat - 4.0f;
        pos_0 = min_0 - (offset_0 * 10.0f);
        if (pos_0 < min_0) {
            pos_0 = min_0;
        }
        levelScale = gSlotLevelScaleB;
        slotSetPosition(D_005C25C0, pos_0, (f32) gSlotLayoutTableB[1]);
        slotSetLevel(D_005C25C0, (layoutIndexFloat - 2.0f) * levelScale);
        min_1 = (f32) gSlotLayoutTableB[2];
        offset_1 = layoutIndexFloat - 6.0f;
        pos_1 = min_1 - (offset_1 * 10.0f);
        if (pos_1 < min_1) {
            pos_1 = min_1;
        }
        secondVisibleSlot = &D_005C25C0[1];
        slotSetPosition(secondVisibleSlot, pos_1, (f32) gSlotLayoutTableB[3]);
        slotSetLevel(secondVisibleSlot, offset_0 * levelScale);
        min_2 = (f32) gSlotLayoutTableB[4];
        pos_2 = min_2 - ((layoutIndexFloat - 8.0f) * 10.0f);
        if (pos_2 < min_2) {
            pos_2 = min_2;
        }
        thirdVisibleSlot = &D_005C25C0[2];
        slotSetPosition(thirdVisibleSlot, pos_2, (f32) gSlotLayoutTableB[5]);
        slotSetLevel(thirdVisibleSlot, offset_1 * levelScale);
        return;
    }
    layoutIndexFloat = (f32) layoutIndex;
    min_0 = (f32) gSlotLayoutTableA[0];
    offset_0 = layoutIndexFloat - 4.0f;
    pos_0 = min_0 - (offset_0 * 10.0f);
    if (pos_0 < min_0) {
        pos_0 = min_0;
    }
    levelScale = gSlotLevelScaleA;
    func_002B1948(D_005C25C0, pos_0, (f32) gSlotLayoutTableA[1]);
    slotSetLevel(D_005C25C0, (layoutIndexFloat - 2.0f) * levelScale);
    min_1 = (f32) gSlotLayoutTableA[2];
    offset_1 = layoutIndexFloat - 6.0f;
    pos_1 = min_1 - (offset_1 * 10.0f);
    if (pos_1 < min_1) {
        pos_1 = min_1;
    }
    secondVisibleSlot = &D_005C25C0[1];
    func_002B1948(secondVisibleSlot, pos_1, (f32) gSlotLayoutTableA[3]);
    slotSetLevel(secondVisibleSlot, offset_0 * levelScale);
    min_2 = (f32) gSlotLayoutTableA[4];
    pos_2 = min_2 - ((layoutIndexFloat - 8.0f) * 10.0f);
    if (pos_2 < min_2) {
        pos_2 = min_2;
    }
    thirdVisibleSlot = &D_005C25C0[2];
    func_002B1948(thirdVisibleSlot, pos_2, (f32) gSlotLayoutTableA[5]);
    slotSetLevel(thirdVisibleSlot, offset_1 * levelScale);
}

INCLUDE_ASM("asm/main/nonmatchings/tu363", func_002AF118);

extern MrPackStateBlock D_005D0670[];

void stateTailClear(void)
{
    MrPackStateBlock *packStateA = D_005D0670;

    packStateA->tail[0] = 0;
    packStateA->tail[1] = 0;
    packStateA->tail[2] = 0;
    packStateA->tail[3] = 0;
}

extern s32 D_005514B0;

s32 counterGetValue(void) {
    return D_005514B0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu363", func_002AF2A8);

extern SlotCell D_005C3270[];
extern s32 D_005C5940[];

void slotSetWordA(s32 visibleSlotIndex, s32 storedWord) {
    if (visibleSlotIndex < 3) {
        D_005C3270[visibleSlotIndex].headWord = storedWord;
        return;
    }
    D_005C5940[0] = storedWord;
}

extern SlotCell D_005C25C8[];
extern s32 D_005C4C98[];

void slotSetWordB(s32 visibleSlotIndex, s32 storedWord) {
    if (visibleSlotIndex < 3) {
        D_005C25C8[visibleSlotIndex].headWord = storedWord;
        return;
    }
    D_005C4C98[0] = storedWord;
}

extern SlotRecord D_005C26E0[];
extern SlotRecord gFallbackSlotRecord[];

void slotSetPairA(s32 visibleSlotIndex, s32 pairFirst, s32 pairSecond)
{
    s32 *pairSecondAddr;
    s32 *pairSecondAddrAlias;
    SlotRecord *fallbackSlot;
    s32 savedPairFirst;
    SlotRecord *indexedSlot;
    s32 savedPairSecond;

    pairSecondAddr = &savedPairSecond;
    savedPairFirst = pairFirst;
    savedPairSecond = pairSecond;
    if (visibleSlotIndex < 3)
    {
        indexedSlot = &D_005C26E0[visibleSlotIndex];
        indexedSlot->pairFirst = savedPairFirst;
        indexedSlot->pairSecond = savedPairSecond;
        return;
    }
    fallbackSlot = gFallbackSlotRecord;
    fallbackSlot->pairAFirst = savedPairFirst;
    pairSecondAddrAlias = pairSecondAddr;
    fallbackSlot->pairASecond = *pairSecondAddrAlias;
}

INCLUDE_ASM("asm/main/nonmatchings/tu363", func_002AF3A8);

extern SlotRecord D_005C26F0[];

void slotSetPairB(s32 visibleSlotIndex, s32 pairFirst, s32 pairSecond)
{
    SlotRecord *fallbackSlot;
    s32 savedPairFirst;
    SlotRecord *indexedSlot;
    s32 savedPairSecond;

    savedPairFirst = pairFirst;
    savedPairSecond = pairSecond;
    if (visibleSlotIndex < 3)
    {
        indexedSlot = &D_005C26F0[visibleSlotIndex];
        indexedSlot->pairFirst = savedPairFirst;
        indexedSlot->pairSecond = savedPairSecond;
        return;
    }
    fallbackSlot = gFallbackSlotRecord;
    fallbackSlot->pairBFirst = savedPairFirst;
    fallbackSlot->pairBSecond = savedPairSecond;
}

extern SlotCell D_005C2704[];
extern s32 D_005C4DD4[];

void slotSetWordC(s32 visibleSlotIndex, s32 storedWord) {
    if (visibleSlotIndex < 3) {
        D_005C2704[visibleSlotIndex].headWord = storedWord;
        return;
    }
    D_005C4DD4[0] = storedWord;
}

extern SlotCell D_005C270C[];
extern s32 D_005C4DDC[];

void slotSetWordD(s32 visibleSlotIndex, s32 storedWord) {
    if (visibleSlotIndex < 3) {
        D_005C270C[visibleSlotIndex].headWord = storedWord;
        return;
    }
    D_005C4DDC[0] = storedWord;
}

extern SlotCell D_005C26E4[];
extern s32 D_005C4DB4[];

void slotSetWordE(s32 visibleSlotIndex, s32 storedWord) {
    if (visibleSlotIndex < 3) {
        D_005C26E4[visibleSlotIndex].headWord = storedWord;
        return;
    }
    D_005C4DB4[0] = storedWord;
}

extern SlotCell D_005C3278[];
extern s32 D_005C5948[];

void slotSetWordMaskedF(s32 visibleSlotIndex, s32 storedWord) {
    s32 masked_value;

    if (visibleSlotIndex < 3) {
        masked_value = storedWord & 0xFF0FFFFF;
        D_005C3278[visibleSlotIndex].headWord = masked_value;
        return;
    }
    D_005C5948[0] = storedWord;
}

extern SlotCell D_005C3280[];
extern s32 D_005C5950[];

void slotSetWordG(s32 visibleSlotIndex, s32 storedWord) {
    if (visibleSlotIndex < 3) {
        D_005C3280[visibleSlotIndex].headWord = storedWord;
        return;
    }
    D_005C5950[0] = storedWord;
}

INCLUDE_ASM("asm/main/nonmatchings/tu363", func_002AF528);

extern SlotCell D_005C3288[];
extern s32 D_005C5958[];

void slotSetWordH(s32 visibleSlotIndex, s32 storedWord) {
    if (visibleSlotIndex < 3) {
        D_005C3288[visibleSlotIndex].headWord = storedWord;
        return;
    }
    D_005C5958[0] = storedWord;
}

INCLUDE_ASM("asm/main/nonmatchings/tu363", func_002AF5F0);

extern s8 D_005514A1;

s32 slotGetCurrentWordE(void) {
    if (D_005514A1 < 3) {
        return D_005C26E4[D_005514A1].headWord;
    }
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu363", func_002AF640);

s32 slotGetWordA(s32 visibleSlotIndex) {
    if (visibleSlotIndex < 3) {
        return D_005C3270[visibleSlotIndex].headWord;
    }
    return D_005C5940[0];
}

extern SlotRecord D_005C25D0[];
extern unsigned char D_005C4CA0[];
s32 func_002D6BA0();

s32 slotQueryI(s32 visibleSlotIndex) {
    if (visibleSlotIndex < 3) {
        return func_002D6BA0(&D_005C25D0[visibleSlotIndex]);
    }
    return func_002D6BA0(D_005C4CA0);
}

extern f32 D_005C2710[];

f32 globalFloatGet(void) {
    return D_005C2710[0];
}

s32 slotGetWordE(s32 visibleSlotIndex) {
    if (visibleSlotIndex < 3) {
        return D_005C26E4[visibleSlotIndex].headWord;
    }
    return D_005C4DB4[0];
}

INCLUDE_ASM("asm/main/nonmatchings/tu363", func_002AF730);

INCLUDE_ASM("asm/main/nonmatchings/tu363", func_002AF830);

extern s32 gPlaneOffsetTable[];
extern s32 gPlaneLevel;
void xglFontFreeBuffer();
#include "main/njPlane.h"
void njPlaneInit(NjPlane *plane);
void njPlaneSetOffset(NjPlane *plane, f32 x, f32 y);
void njPlaneSetLevel(NjPlane *plane, f32 level);
void njPlaneSetMode(NjPlane *plane, s32 mode);
void func_002CFFF0();

void slotDrawPlanes(SlotRecord *anchoredSlot) {
    s32 plane_buffer[0x30];
    s32 base_x;
    s32 base_y;
    s32 offset_x;
    s32 offset_y;
    s32 plane_index;
    s32 *planeOffsets;

    base_x = (s32) (anchoredSlot->anchor_x + 30.0f);
    base_y = (s32) (anchoredSlot->anchor_y + 5.0f);
    if (anchoredSlot->draw_count > 0) {
        xglFontFreeBuffer();
        planeOffsets = gPlaneOffsetTable;
        for (plane_index = 0; plane_index < 8; plane_index++) {
            njPlaneInit((NjPlane *) &plane_buffer[0]);
            njPlaneSetTint((NjPlane *) &plane_buffer[0], 128.0f, 128.0f, 128.0f);
            njPlaneSetLevel((NjPlane *) &plane_buffer[0], (f32) gPlaneLevel);
            njPlaneSetMode((NjPlane *) &plane_buffer[0], 0x22);
            offset_x = planeOffsets[0];
            offset_y = planeOffsets[1];
            planeOffsets += 2;
            njPlaneSetOffset((NjPlane *) &plane_buffer[0], (f32) (base_x + offset_x), (f32) (base_y + offset_y));
            func_002CFFF0(&plane_buffer[0], 0x89, 0);
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu363", func_002AFA68);

INCLUDE_ASM("asm/main/nonmatchings/tu363", func_002B0020);

INCLUDE_ASM("asm/main/nonmatchings/tu363", func_002B0720);

INCLUDE_ASM("asm/main/nonmatchings/tu363", func_002B14A8);

void slotSetLevel(SlotRecord *levelSlot, f32 level) {
    f32 clampedLevel;

    clampedLevel = level;
    if (clampedLevel > 128.0f) {
        clampedLevel = 128.0f;
    }
    if (clampedLevel < 0.0f) {
        clampedLevel = 0.0f;
    }
    levelSlot->level = (s16) clampedLevel;
}

void slotSetDisplayModeAll(s8 displayMode)
{
    SlotRecord *visibleSlots = D_005C25C0;

    visibleSlots[0].displayMode = displayMode;
    visibleSlots[1].displayMode = displayMode;
    visibleSlots[2].displayMode = displayMode;
}

INCLUDE_ASM("asm/main/nonmatchings/tu363", func_002B16B0);

extern u8 gPlaybackMode[];

void playbackModeSet(s32 storedByte) {
    gPlaybackMode[0] = storedByte;
}

INCLUDE_ASM("asm/main/nonmatchings/tu363", func_002B1948);

s32 drawEntrySetPosition(void *, s32, s32);
s32 drawEntrySetValue(void *, s32);
s32 njStringSetPosition(void *, f32, f32);
s32 njStringSetZoom(void *, s32, s32);
s32 njStringSetColor(void *, s32, s32, s32);
s32 njStringSetFlags(void *, s32);

void slotSetPosition(SlotRecord *positionedSlot, f32 x, f32 y) {
    f32 x_next;
    f32 y_next;

    positionedSlot->x = x;
    positionedSlot->y = y;
    positionedSlot->home_x = x;
    positionedSlot->home_y = y;
    x_next = x + (f32) gSlotLayoutTableB[6];
    y_next = y + (f32) gSlotLayoutTableB[7];
    positionedSlot->label_x = x_next;
    positionedSlot->label_y = y_next;
    drawEntrySetValue(positionedSlot->label, 0x4E);
    drawEntrySetPosition(positionedSlot->label, (s32) (positionedSlot->x + (f32) gSlotLayoutTableB[8]), (s32) (positionedSlot->y + (f32) gSlotLayoutTableB[9]));
    njStringSetPosition(positionedSlot->text, positionedSlot->x + (f32) gSlotLayoutTableB[12], positionedSlot->y);
    njStringSetZoom(positionedSlot->text, 0xE, 0xE);
    njStringSetColor(positionedSlot->text, 0xC4, 0xC4, 0xC4);
    njStringSetFlags(positionedSlot->text, 0xA0C);
    njPlaneSetMode((NjPlane *) positionedSlot->plane_a, 2);
    njPlaneSetMode((NjPlane *) positionedSlot->plane_b, 2);
}

INCLUDE_ASM("asm/main/nonmatchings/tu363", func_002B1B68);

INCLUDE_ASM("asm/main/nonmatchings/tu363", func_002B1F88);

INCLUDE_ASM("asm/main/nonmatchings/tu363", func_002B2780);

INCLUDE_ASM("asm/main/nonmatchings/tu363", func_002B28C8);

INCLUDE_ASM("asm/main/nonmatchings/tu363", func_002B3478);

s32 func_002B3478(s32);

void fallbackSlotReset(void) {
    SlotRecord *fallbackSlot;
    s32 *cursor;
    s32 clearedWordCount;

    fallbackSlot = gFallbackSlotRecord;
    fallbackSlot->resetHalf = 0;
    func_002B3478(0);
    fallbackSlot->resetByteA = 0;
    fallbackSlot->resetByteB = 0;
    cursor = &((s32 *) fallbackSlot->label)[0x7D];
    for (clearedWordCount = 0; clearedWordCount < 40; clearedWordCount++) {
        *cursor = 0;
        cursor -= 1;
    }
}

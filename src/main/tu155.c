#include "common.h"

#include "shared.h"
#include "main/battle_shared.h"
#include "main/tu176.h"
#include "main/tu160.h"
#include "main/tu156.h"

typedef struct StateBudget {
    /* 0x00 */ s32 amountA;
    /* 0x04 */ s32 amountB;
    /* 0x08 */ u32 remainingAmount;
    /* 0x0C */ s32 recordCategory[10];
    /* 0x34 */ s32 recordItem[10];
    /* 0x5C */ s32 recordCount;
    /* 0x60 */ s32 scaledAmountA;
    /* 0x64 */ s32 scaledAmountB;
    /* 0x68 */ s32 spentAmount;
} StateBudget;

typedef struct StateBlock {
    /* 0x00 */ StateBudget *budget;
    /* 0x04 */ s32 reserveAmountA;
    /* 0x08 */ s32 reserveAmountB;
    /* 0x0C */ s32 scaledCountA;
    /* 0x10 */ s32 scaledCountB;
    /* 0x14 */ s32 pendingLevels[10];
    /* 0x3C */ s32 messageFade[10];
    /* 0x64 */ s32 levelTimer[10];
    /* 0x8C */ u8 *message[10];
    /* 0xB4 */ s32 drainStep;
    /* 0xB8 */ s32 pollCountdown;
    /* 0xBC */ u32 totalSeconds;
    /* 0xC0 */ s32 entryId[10];
    /* 0xE8 */ union {
        s32 activeEntryCount; /* kept in unions: their alias set decides the emitted order in budgetDisplayRun */
    };
    /* 0xEC */ union {
        s32 entryCount;
    };
    /* 0xF0 */ u8 recordCount;
    /* 0xF1 */ u8 recordCategory[10];
    /* 0xFB */ u8 recordItem[10];
    /* 0x105 */ u8 recordAmount[10];
    /* 0x10F */ u8 mode;
    /* 0x110 */ u8 phase;
    /* 0x111 */ u8 unk_111;
    /* 0x112 */ u16 counter;
    /* 0x114 */ u8 fontAlpha[4];
    /* 0x118 */ u8 unk_118[8];
    /* 0x120 */ u8 flagBlock[0x340];
    /* 0x460 */ u16 gaugeValue;
    /* 0x462 */ u16 fadeValue;
    /* 0x464 */ u8 finishDelay;
    /* 0x465 */ u8 finished;
    /* 0x466 */ u8 unk_466[0xA];
} StateBlock;

/* gBudgetDisplayBlock as one struct: the functions that use it keep the pointer load apart from the int stores. */
typedef struct StateBlockHandle {
    StateBlock *block;
} StateBlockHandle;

void budgetDisplaySetMode(s32 mode);
void *memPoolAlloc(s32 size);
s32 bufferPointersRewind();
s32 completionBlockClear();
s32 shardFieldInit(void *);
s32 mcModuleReset();
s32 memset(void *dest, s32 value, s32 size);

/* Allocates and zeroes the 0x470-byte state block gBudgetDisplayBlock, sets its flag byte at 0x10F from bit 0 of
 * the argument and returns 1. Destroyed by destroyStateBlock. */
s32 createStateBlock(s32 blockFlagBits) {
    extern StateBlock *gBudgetDisplayBlock;
    extern s32 func_00224600(s32);
    void *stateBlockMemory;

    func_00224600(0);
    completionBlockClear();
    bufferPointersRewind();
    stateBlockMemory = memPoolAlloc(0x470);
    gBudgetDisplayBlock = stateBlockMemory;
    memset(stateBlockMemory, 0, 0x470);
    shardFieldInit(gBudgetDisplayBlock->flagBlock);
    mcModuleReset();
    if (blockFlagBits & 1) {
        gBudgetDisplayBlock->mode = 1U;
    } else {
        gBudgetDisplayBlock->mode = 0U;
    }
    budgetDisplaySetMode(gBudgetDisplayBlock->mode);
    return 1;
}

s32 memPoolFree(void *block);
s32 xcBlockFlagsReset(void *block);

/* Releases the state block created by createStateBlock. */
void destroyStateBlock(void) {
    extern StateBlock *gBudgetDisplayBlock;
    xcBlockFlagsReset(gBudgetDisplayBlock->flagBlock);
    memPoolFree(gBudgetDisplayBlock);
    gBudgetDisplayBlock = 0;
}

s32 fontQueueColorState(s32);
s32 xglFontDrawBoxBlend(short *rect, s32 a, s32 b, s32 c);
void fontQueueRectTwoColors(short *rect, s32 a, s32 b, s32 c);
void xglFontPrintTitle(void);
void xglFontPrintEscapeByte(s32 escapeArgument);
void xglFontPrintExtendedColor(s32 red, s32 green, s32 blue);
void xglFontSetColor(s32 red, s32 green, s32 blue);
void xglFontPrintAlphaCommand(s32 alphaMode, s32 alphaIndex, s32 alpha);
void xglFontPrintIndexed(s32 x, s32 y, const char *text);
void xglFontDrawRightAligned16(s32 baseX, s32 baseY, const char *text, s32 glyphIndex);
s32 xglFontDrawUnsignedNumber(s32 baseX, s32 baseY, s32 number, s32 scale);
s32 xglFontDrawStringScaled(s32, s32, const char *, s32);
void fontDrawRectOfCell(s32, s32, s32);
s32 xglFontPrintExtendedColorByte(s32, s32, s32, s32);
s32 func_00A85160(u8 itemId, u8 quantity);
s32 *func_A87550(s32 recordNumber);
const char *messageTextGet(s32 id);
void sprintf(char *moviePath, const char *format, s32 movieName);
extern char *gFontEscapeBuffer;

/* Draws one row of the battle result list for entry `slot`: the message with its fade-in when the entry has
 * one, otherwise the record's counts, then the fade-out and the reserve or budget amounts for the row. */
void budgetDisplayDrawRow(unsigned short baseX, s32 recordId, s32 rowIndex, s32 fade) {
    extern StateBlock *gBudgetDisplayBlock;
    extern char *gFontEscapeBuffer;
    short rect[4];
    s32 rectLeft;
    s32 rectMid;
    s32 rectRight;
    s32 rectEnd;
    s32 *record;
    s32 usesReserve;
    s32 glyph;
    s32 messageFade;
    s32 shade;
    s32 messageText;
    s32 amount;
    s32 gaugeRed;
    s32 gaugeGreen;
    s32 gaugeBlue;
    s32 gaugeHot;
    s32 fadeAlpha;

    record = func_A87550(recordId);
    usesReserve = rowIndex >= gBudgetDisplayBlock->activeEntryCount;

    rect[0] = -0x7EF8;
    rect[2] = -0x70C8;
    rect[3] = baseX + 0x300;
    rect[1] = baseX;
    fontQueueColorState(0x73221E1D);
    xglFontDrawBoxBlend(rect, 2, 0, 0);
    rectLeft = -0x7EB8;
    rectMid = baseX + 0x40;
    rectRight = -0x70F8;
    rectEnd = 0x2C0 + baseX;
    rect[1] = rectMid;
    rect[2] = rectRight;
    rect[0] = rectLeft;
    rect[3] = rectEnd;
    fontQueueRectTwoColors(rect, 0x3A2827, 0x803A2827, 0x101);

    xglFontPrintTitle();
    xglFontPrintEscapeByte(13);
    xglFontPrintExtendedColor(0, 0, 0);

    if (gBudgetDisplayBlock->message[rowIndex] != 0 && gBudgetDisplayBlock->message[rowIndex][0] != 0) {
        messageText = func_00A85160(gBudgetDisplayBlock->message[rowIndex][0], gBudgetDisplayBlock->message[rowIndex][1]);
        glyph = 13;
        switch (gBudgetDisplayBlock->message[rowIndex][0]) {
        case 2:
            glyph = 12;
            break;
        case 3:
            glyph = 15;
            break;
        case 4:
            glyph = 14;
            break;
        case 1:
            break;
        }
        messageFade = gBudgetDisplayBlock->messageFade[rowIndex];
        if (messageFade >= 113) {
            shade = 128 - messageFade;
            fontQueueColorState((((long long) (shade * 8)) << 24) | 0x808080);
            fontDrawRectOfCell((((16 - shade) << 6) + 0x8188) & 0xFFF8, (baseX + 0x100) & 0xFFFF, glyph);
            return;
        }
        fontQueueColorState(0x80808080);
        fontDrawRectOfCell(0x8188, (baseX + 0x100) & 0xFFFF, glyph);
        if (gBudgetDisplayBlock->messageFade[rowIndex] > 64) {
            xglFontPrintAlphaCommand(68, 0,
                                     0xFE & (-128 - ((((u8) gBudgetDisplayBlock->messageFade[rowIndex]) - 64) << 1)));
        }
        xglFontSetColor(90, 91, 172);
        sprintf(gFontEscapeBuffer, messageTextGet(150), messageText);
        xglFontPrintIndexed(315, ((baseX - 0x71F8) >> 4) + 15, gFontEscapeBuffer);
        xglFontPrintAlphaCommand(68, 0, 128);
        return;
    }

    if (gBudgetDisplayBlock->mode == 3) {
        fontQueueColorState(0x73284680);
        xglFontDrawStringScaled(0x81C8, (baseX + 0x110) & 0xFFFF, messageTextGet(161), 16);
    } else {
        fontQueueColorState(0x80545934);
        xglFontDrawRightAligned16(0x8348, (baseX + 0x90) & 0xFFFF, messageTextGet(64), 13);
        xglFontDrawRightAligned16(0x8348, (baseX + 0x1D0) & 0xFFFF, messageTextGet(63), 13);
        fontQueueColorState(0x80626262);
        xglFontDrawUnsignedNumber(0x8398, (baseX + 0x60) & 0xFFFF, record[0], 16);
        xglFontDrawUnsignedNumber(0x8398, (baseX + 0x1A0) & 0xFFFF, record[2], 16);
    }

    if (fade != 0) {
        if (fade >= 60) {
            fadeAlpha = ((80 - fade) * 4) + 46;
            fontQueueColorState(((long long) fadeAlpha << 8) | ((long long) fadeAlpha << 16) | 0x80000080);
        } else if (fade >= 20) {
            fadeAlpha = (fade * 2) + 6;
            fontQueueColorState(((long long) fadeAlpha << 8) | ((long long) fadeAlpha << 16) | 0x80000080);
        } else {
            fontQueueColorState(0x802E2E80);
        }
        xglFontDrawStringScaled(0x8968, (baseX + 0x120) & 0xFFFF, messageTextGet(151), 14);
        return;
    }

    if (usesReserve) {
        amount = gBudgetDisplayBlock->reserveAmountA - gBudgetDisplayBlock->scaledCountA;
    } else {
        amount = gBudgetDisplayBlock->budget->amountA - gBudgetDisplayBlock->budget->scaledAmountA;
    }
    xglFontSetColor(93, 123, 118);
    xglFontPrintExtendedColorByte(0, 0, 0, 2);
    if (gBudgetDisplayBlock->gaugeValue < 256) {
        gaugeBlue = ((((15 * gBudgetDisplayBlock->gaugeValue) >> 8) & 0xFF)) + 113;
        gaugeRed = gBudgetDisplayBlock->gaugeValue << 2;
        gaugeRed = gaugeRed + gBudgetDisplayBlock->gaugeValue;
        gaugeRed = gaugeRed << 4;
        gaugeRed = gaugeRed + gBudgetDisplayBlock->gaugeValue;
        gaugeRed = gaugeRed >> 8;
        gaugeRed = gaugeRed & 0xFF;
        gaugeRed = gaugeRed + 47;
        fontQueueColorState(gaugeRed | 0x8000 | ((long long) gaugeBlue << 16) | 0x80000000);
        xglFontSetColor(gaugeRed & 0xFF, 128, gaugeBlue & 0xFF);
    } else {
        gaugeHot = 384 - gBudgetDisplayBlock->gaugeValue;
        gaugeGreen = 70 + (((gaugeHot * 29) >> 6) & 0xFF);
        gaugeBlue = 40 + (((gaugeHot * 11) >> 4) & 0xFF);

        fontQueueColorState(0x80000080 | ((long long) gaugeGreen << 8) | ((long long) gaugeBlue << 16));
        xglFontSetColor(128, 0xFF & gaugeGreen, gaugeBlue & 0xFF);
    }
    if (gBudgetDisplayBlock->mode == 0) {
        xglFontPrintIndexed(406, (((baseX - 0x71F8) >> 4)) + 6, "+");
        xglFontPrintIndexed(406, (((baseX - 0x71F8) >> 4)) + 26, "+");
    }
    xglFontDrawUnsignedNumber(0x8A18, (baseX + 0x60) & 0xFFFF, amount, 16);

    if (usesReserve) {
        amount = gBudgetDisplayBlock->reserveAmountB - gBudgetDisplayBlock->scaledCountB;
    } else {
        amount = gBudgetDisplayBlock->budget->amountB - gBudgetDisplayBlock->budget->scaledAmountB;
    }
    xglFontDrawUnsignedNumber(0x8A18, (baseX + 0x1A0) & 0xFFFF, amount, 16);
}

typedef long long s64;

void xglFontFlush(void);
void xglFontSetDigitStep(s32 step);
void xglFontSetAlphaScale(u8 alpha);
void fontQueueSpriteReset(void);
void shardFieldUpdate(void *block);
void xglFontFreeBuffer(void);
void func_0021CB90(s32 a, s32 b, s32 c, s32 d);
void xglFontResetState(void);
void dispatchCodeRange(u16 *pos, s32 handle, s32 a, s32 b, s32 c);
void xglFontQueueBarRed(s32 a, s32 b, s32 c);
void xglFontQueueBarBlue(s32 a, s32 b, s32 c);
void *func_00222D48(s32 a, s32 b, s32 c);
void fontQueueLine(s32 a, s32 b, s32 c, s32 d);
s32 Get_Rnd_001B8CD0(s32 a, s32 b);
void xglFontDrawCharScaled(s32 a, s32 b, s32 c, s32 d);
void func_0021ABB0(s32 a, s32 b, void *text, s32 d);
void mcStateSeekEntry(s32 id);
s32 xglFontTextWidth(void *text, s32 b);
s32 xglFontDrawDigitsMinTwo(s32 x, s32 y, s32 value, s32 digits);
void xglFontDrawDigits(s32 x, s32 y, s32 value, s32 digits);

/* Draws the whole result page: the fonts, the entry rows (activeEntryCount rows highlighted), the record list,
 * the budget figures and the play-time line at the bottom. */
void budgetDisplayDrawPage(void) {
    extern StateBlock *gBudgetDisplayBlock;
    u16 rect[4];
    StateBudget *budget;
    u32 totalSeconds;
    u32 minutesTotal;
    u32 hoursTotal;
    s32 textOffset;
    u8 savedCursor;
    u8 rowsNow;
    s32 i;
    s32 entryCount;
    s32 activeRows;
    s32 inactiveRows;
    s32 digits;
    s32 greenIn;
    s32 greenOut;
    s32 blue;
    s32 fadeIn;
    s32 fadeOut;
    s32 width;
    s32 yPos;
    void *text;

    totalSeconds = gBudgetDisplayBlock->totalSeconds;
    xglFontFlush();
    xglFontSetDigitStep(10);
    xglFontSetAlphaScale(0x80);
    fontQueueSpriteReset();
    xglFontSetAlphaScale(gBudgetDisplayBlock->fontAlpha[3]);
    shardFieldUpdate(gBudgetDisplayBlock->flagBlock);
    xglFontSetAlphaScale(0x80);
    xglFontFreeBuffer();

    if (gBudgetDisplayBlock->fontAlpha[1] != 0) {
        xglFontSetAlphaScale(gBudgetDisplayBlock->fontAlpha[1]);
        func_0021CB90(124, 1, 0, 0);
        xglFontResetState();
    }

    if (gBudgetDisplayBlock->fontAlpha[0] != 0) {
        savedCursor = gBattleRosterTable.rosterPair.rosters[0].cursor;
        if (battleCurrentRosterIndexGet() != 0) {
            gBattleRosterTable.rosterPair.rosters[0].cursor = -1;
        }
        entryCount = gBudgetDisplayBlock->entryCount;
        xglFontSetAlphaScale(gBudgetDisplayBlock->fontAlpha[0]);
        rect[0] = 29640;
        rect[1] = 29928;
        for (i = 0; i < entryCount; i++) {
            if (i == gBudgetDisplayBlock->activeEntryCount) {
                rect[1] += 80;
            }
            dispatchCodeRange(rect, gBudgetDisplayBlock->entryId[i], 0, 0, 0);
            budgetDisplayDrawRow(rect[1], gBudgetDisplayBlock->entryId[i], i, gBudgetDisplayBlock->levelTimer[i]);
            if (gBudgetDisplayBlock->levelTimer[i] > 0) {
                gBudgetDisplayBlock->levelTimer[i]--;
            }
            rect[1] += 768;
        }
        activeRows = gBudgetDisplayBlock->activeEntryCount;
        xglFontQueueBarRed(28936, 29928, activeRows * 48);
        inactiveRows = gBudgetDisplayBlock->entryCount - gBudgetDisplayBlock->activeEntryCount;
        if (inactiveRows > 0) {
            xglFontQueueBarBlue(28936, ((gBudgetDisplayBlock->activeEntryCount * 768) + 30008) & 0xfff8, inactiveRows * 48);
        }
        gBattleRosterTable.rosterPair.rosters[0].cursor = savedCursor;
        xglFontResetState();
    }

    if (gBudgetDisplayBlock->fontAlpha[2] != 0) {
        xglFontSetAlphaScale(gBudgetDisplayBlock->fontAlpha[2]);
        rowsNow = gBudgetDisplayBlock->recordCount;
        rect[0] = 29272;
        rect[1] = 30776;
        rect[3] = 416 * rowsNow + 31512;
        rect[2] = -29272;
        fontQueueColorState(0x73221E1D);
        xglFontDrawBoxBlend(rect, 2, 0x73312A29, 1);
        rect[0] = 30392;
        rect[1] = -31176;
        rect[2] = -30392;
        rect[3] = -30424;
        /* kept: the call through the typed pointer decides the emitted call; the direct call does not match. */
        ((void (*)(u16 *rect, s32 style, s32 colors, s32 blend)) xglFontDrawBoxBlend)(rect, 2, 0x73312A29, 1);
        xglFontPrintTitle();
        xglFontSetColor(52, 89, 84);
        xglFontPrintExtendedColor(0, 0, 0);
        xglFontPrintEscapeByte(13);
        width = xglFontGetFormattedWidth(messageTextGet(152));
        textOffset = (width * 13) >> 5;
        text = messageTextGet(152);
        xglFontPrintIndexed(256 - textOffset, 328, text);
        xglFontPrintEscapeByte(16);
        xglFontPrintExtendedColor(0, 0, 0);

        if (gBudgetDisplayBlock->recordCount != 0) {
            width = xglFontGetFormattedWidth(messageTextGet(153));
            textOffset = width >> 1;
            text = messageTextGet(153);
            xglFontPrintIndexed(256 - textOffset, 108, text);
            fontQueueColorState(0x80423732);
            fontQueueLine(29304, 31336, 0x8D88, 31336);
            xglFontSetColor(98, 98, 98);
            rect[1] = 139;
            for (i = 0; i < gBudgetDisplayBlock->recordCount; i++) {
                text = func_00222D48(gBudgetDisplayBlock->recordCategory[i], gBudgetDisplayBlock->recordItem[i], 0);
                if (text != 0) {
                    xglFontPrintIndexed(74, rect[1], text);
                }
                sprintf(gFontEscapeBuffer, "%d", gBudgetDisplayBlock->recordAmount[i]);
                xglFontPrintIndexed(400, rect[1], gFontEscapeBuffer);
                text = messageTextGet(154);
                xglFontPrintIndexed(417, rect[1], text);
                rect[1] += 26;
            }
        } else {
            width = xglFontGetFormattedWidth(messageTextGet(155));
            textOffset = width >> 1;
            text = messageTextGet(155);
            xglFontPrintIndexed(256 - textOffset, 111, text);
        }

        if (gBudgetDisplayBlock->mode == 4) {
            fontQueueColorState(0x73284680);
            text = messageTextGet(161);
            xglFontDrawStringScaled(30648, 0x87B8, text, 16);
        } else {
            fontQueueColorState(0x80626262);
            digits = xglFontDrawUnsignedNumber(30968, 0x87B8, Get_Rnd_001B8CD0(10, 0), 16);
            xglFontDrawCharScaled((((digits << 4) + 31032) & 0xfff8), 0x87D8, 71, 16);
        }

        if (gBudgetDisplayBlock->fadeValue < 256) {
            fadeIn = gBudgetDisplayBlock->fadeValue;
            greenIn = fadeIn * 81;
            greenIn >>= 8;
            greenIn &= 0xff;
            greenIn += 47;
            blue = ((((fadeIn * 15) >> 8) & 0xff) + 113);
            fontQueueColorState(((s64) 0x80000000) | ((s64) blue << 16) | (s64) (greenIn | 0x8000));
            xglFontSetColor(greenIn & 0xff, 128, blue & 0xff);
        } else {
            fadeOut = 384 - gBudgetDisplayBlock->fadeValue;
            greenOut = (((fadeOut * 29) >> 6) & 0xff) + 70;
            blue = ((((fadeOut * 11) >> 4) & 0xff) + 40);
            fontQueueColorState(((s64) 0x80000080) | ((s64) greenOut << 8) | ((s64) blue << 16));
            xglFontSetColor(128, greenOut & 0xff, blue & 0xff);
        }

        if (gBudgetDisplayBlock->mode == 2) {
            xglFontPrintEscapeByte(13);
            xglFontPrintExtendedColorByte(0, 0, 0, 2);
            xglFontPrintIndexed(250, 348, "+");
        }

        budget = gBudgetDisplayBlock->budget;
        digits = xglFontDrawUnsignedNumber(0x8068, 0x87B8, budget->remainingAmount - budget->spentAmount, 16);
        text = messageTextGet(92);
        xglFontDrawStringScaled((((digits << 4) + 0x80A8) & 0xfff8), 0x87D8, text, 16);
    }

    xglFontSetAlphaScale(gBudgetDisplayBlock->fontAlpha[3]);
    fontQueueColorState(0x80808080);
    text = messageTextGet(156);
    func_0021ABB0(0, 11, text, 0);
    mcStateSeekEntry(418);
    text = messageTextGet(157);
    yPos = xglFontTextWidth(text, 15);
    rect[0] = 16 * (373 - yPos) + 28664;
    rect[1] = 29352;
    rect[2] = -29128;
    rect[3] = 29752;
    ((void (*)(s32 mask)) fontQueueColorState)(0x73221E1D);
    xglFontDrawBoxBlend(rect, 2, 0x73312A29, 1);
    fontQueueColorState(0x80562E2D);
    text = messageTextGet(157);
    xglFontDrawStringScaled((u16) (rect[0] - yPos + 192), (u16) (rect[1] + 96), text, 15);
    rect[0] = -32504;
    ((void (*)(s32 mask)) fontQueueColorState)(0x80626262);
    if (totalSeconds > 0x57E3F) {
        totalSeconds = 0x57E3F;
    }
    xglFontDrawDigitsMinTwo((u16) (rect[0] + 3216), (u16) (rect[1] + 64), totalSeconds % 60, 16);
    minutesTotal = totalSeconds / 60;
    xglFontDrawDigitsMinTwo((u16) (rect[0] + 2704), (u16) (rect[1] + 64), minutesTotal % 60, 16);
    hoursTotal = minutesTotal / 60;
    xglFontDrawDigits((u16) (rect[0] + 2176), (u16) (rect[1] + 64), hoursTotal, 16);
    xglFontDrawCharScaled((u16) (rect[0] + 2224), (u16) (rect[1] + 80), 58, 16);
    xglFontDrawCharScaled((u16) (rect[0] + 2752), (u16) (rect[1] + 80), 58, 16);
    xglFontSetAlphaScale(128);
}

s32 func_A87E08(s32 handle, u32 amount);
void func_A88360(s32 handle, u32 amount);
void seVoiceStartDefault(unsigned int handle);

/* Takes the amount from the budget for the first active entries and from the reserve for the later ones, each
 * clamped at zero, then adds the takes to the pending level of every entry. Sets the level timer to 80 for each
 * entry whose level changed and plays the sound when any did. */
s32 budgetTakeAmounts(s32 amount) {
    extern StateBlockHandle gBudgetDisplayBlock;
    u32 budgetTakeA;
    u32 reserveTakeA;
    s32 budgetTakeB;
    s32 reserveTakeB;
    s32 i;
    s32 entryCount;
    s32 anyLevelChanged;

    anyLevelChanged = 0;
    budgetTakeA = amount;
    budgetTakeB = amount;
    reserveTakeB = amount;
    reserveTakeA = amount;
    if (gBudgetDisplayBlock.block->budget->amountA >= budgetTakeA) {
        gBudgetDisplayBlock.block->budget->amountA -= budgetTakeA;
    } else {
        budgetTakeA = gBudgetDisplayBlock.block->budget->amountA;
        gBudgetDisplayBlock.block->budget->amountA = 0;
    }
    if (gBudgetDisplayBlock.block->budget->amountB >= budgetTakeB) {
        gBudgetDisplayBlock.block->budget->amountB -= budgetTakeB;
    } else {
        budgetTakeB = gBudgetDisplayBlock.block->budget->amountB;
        gBudgetDisplayBlock.block->budget->amountB = 0;
    }
    if (gBudgetDisplayBlock.block->reserveAmountA >= reserveTakeA) {
        gBudgetDisplayBlock.block->reserveAmountA -= reserveTakeA;
    } else {
        reserveTakeA = gBudgetDisplayBlock.block->reserveAmountA;
        gBudgetDisplayBlock.block->reserveAmountA = 0;
    }
    if (gBudgetDisplayBlock.block->reserveAmountB >= reserveTakeB) {
        gBudgetDisplayBlock.block->reserveAmountB -= reserveTakeB;
    } else {
        reserveTakeB = gBudgetDisplayBlock.block->reserveAmountB;
        gBudgetDisplayBlock.block->reserveAmountB = 0;
    }
    entryCount = gBudgetDisplayBlock.block->entryCount;
    for (i = 0; i < entryCount; i++) {
        s32 handle = gBudgetDisplayBlock.block->entryId[i];
        s32 levelBefore = gBudgetDisplayBlock.block->pendingLevels[i];

        if (i >= gBudgetDisplayBlock.block->activeEntryCount) {
            gBudgetDisplayBlock.block->pendingLevels[i] += func_A87E08(handle, reserveTakeA);
            func_A88360(handle, reserveTakeB);
        } else {
            gBudgetDisplayBlock.block->pendingLevels[i] += func_A87E08(handle, budgetTakeA);
            func_A88360(handle, budgetTakeB);
        }
        if (levelBefore != gBudgetDisplayBlock.block->pendingLevels[i]) {
            gBudgetDisplayBlock.block->levelTimer[i] = 80;
            anyLevelChanged = 1;
        }
    }
    if (anyLevelChanged) {
        seVoiceStartDefault(9);
        return 1;
    }
    return 0;
}

/* Scales the two displayed amounts on the budget and the two scaled counts on the block by 0.85. */
void scaledAmountsDecay(void) {
    extern StateBlock *gBudgetDisplayBlock;
    StateBlock *block;
    StateBudget *budget;

    block = gBudgetDisplayBlock;
    budget = block->budget;
    budget->scaledAmountA = budget->scaledAmountA * 0.85f;
    block->scaledCountA = block->scaledCountA * 0.85f;
    budget->scaledAmountB = budget->scaledAmountB * 0.85f;
    block->scaledCountB = block->scaledCountB * 0.85f;
}

/* Takes amount from the remaining amount of the budget that gBudgetDisplayBlock points to (clamped at zero) and passes the
 * amount taken to func_00224600. */
void stateBlockConsume(u32 consumeAmount) {
    extern StateBlock *gBudgetDisplayBlock;
    extern s32 func_00224600();
    u32 remainingBefore;
    u32 remainingAfter;
    u32 takenAmount;
    StateBudget *stateBudget;

    takenAmount = consumeAmount;
    stateBudget = gBudgetDisplayBlock->budget;
    remainingBefore = stateBudget->remainingAmount;
    remainingAfter = remainingBefore - takenAmount;
    if (remainingBefore >= takenAmount) {
        stateBudget->remainingAmount = remainingAfter;
    } else {
        stateBudget->remainingAmount = 0U;
        takenAmount = remainingBefore;
    }
    func_00224600(takenAmount);
}

void mcClearSetupStatus(void);
void func_00220B90(s32, s32);
void func_00220D28(void);

/* Sets the block's mode and resets its phase and counter, then sets the two message slots for the mode. */
void budgetDisplaySetMode(s32 mode) {
    extern StateBlock *gBudgetDisplayBlock;

    gBudgetDisplayBlock->mode = mode;
    gBudgetDisplayBlock->phase = 0;
    gBudgetDisplayBlock->counter = 0;
    switch (mode) {
    case 0:
        mcClearSetupStatus();
        func_00220B90(0x40, messageTextGet(0x9E));
        func_00220B90(0x20, messageTextGet(0x9F));
        func_00220D28();
        return;
    case 1:
        mcClearSetupStatus();
        func_00220B90(0x40, messageTextGet(0x9F));
        func_00220B90(0x20, 0);
        func_00220D28();
        gBudgetDisplayBlock->phase = 2;
        break;
    case 2:
        mcClearSetupStatus();
        func_00220B90(0x40, messageTextGet(0xA0));
        func_00220B90(0x20, messageTextGet(0x9F));
        func_00220D28();
        break;
    }
}

/* Returns 1 when no entry has a pending level, a fading message or a level timer left, 0 otherwise. */
s32 budgetDisplayIsIdle(void) {
    extern StateBlock *gBudgetDisplayBlock;
    s32 slot;
    s32 entryCount;

    entryCount = gBudgetDisplayBlock->entryCount;
    for (slot = 0; slot < entryCount; slot++) {
        if (gBudgetDisplayBlock->pendingLevels[slot] > 0 || gBudgetDisplayBlock->messageFade[slot] >= 2 ||
            gBudgetDisplayBlock->levelTimer[slot] > 0) {
            return 0;
        }
    }
    return 1;
}

typedef struct StateEntry {
    u8 unmodeled_00[0x14];
    s8 stepCount; /* 0x14 */
} StateEntry;

StateEntry *func_A86DF8(s32 entryId);
u8 *func_A88248(s32 entryId, s32 step);

/* Steps the pending level of one slot down. In mode 0 it stops at the first step that has a message text: it shows
 * that text (fade 0x80) and returns 1. Returns 0 when the level reaches zero without a message. */
s32 entryLevelStep(s32 slot, s32 mode) {
    extern StateBlock *gBudgetDisplayBlock;
    s32 entryId = gBudgetDisplayBlock->entryId[slot];
    StateEntry *entry = func_A86DF8(entryId);
    u8 *stepText;

    while (gBudgetDisplayBlock->pendingLevels[slot] != 0) {
        gBudgetDisplayBlock->pendingLevels[slot]--;
        stepText = func_A88248(entryId, entry->stepCount - gBudgetDisplayBlock->pendingLevels[slot]);
        if (mode == 0 && stepText != 0 && stepText[0] != 0) {
            gBudgetDisplayBlock->messageFade[slot] = 0x80;
            gBudgetDisplayBlock->message[slot] = stepText;
            gBudgetDisplayBlock->levelTimer[slot] = 0;
            gBudgetDisplayBlock->pollCountdown = 0x20;
            return 1;
        }
    }
    return 0;
}

/* Drains the pending levels of every entry without starting a message (entryLevelStep with mode 1). */
void entryLevelStepAll(void) {
    extern StateBlock *gBudgetDisplayBlock;
    s32 messageSlot;
    s32 messageSlotTotal;

    messageSlotTotal = gBudgetDisplayBlock->entryCount;
    for (messageSlot = 0; messageSlot < messageSlotTotal; messageSlot++) {
        entryLevelStep(messageSlot, 1);
    }
}

/* Starts the next pending message: the first entry with no fade and no level timer left that entryLevelStep accepts. */
void entryMessageStartNext(void) {
    extern StateBlock *gBudgetDisplayBlock;
    s32 messageSlot;
    s32 messageSlotTotal;

    messageSlotTotal = gBudgetDisplayBlock->entryCount;
    for (messageSlot = 0; messageSlot < messageSlotTotal; messageSlot++) {
        if (gBudgetDisplayBlock->pendingLevels[messageSlot] && !gBudgetDisplayBlock->messageFade[messageSlot] &&
            !gBudgetDisplayBlock->levelTimer[messageSlot] && entryLevelStep(messageSlot, 0)) {
            break;
        }
    }
}

/* Fades every shown message by fadeStep and closes the ones that reach zero. Starts the next pending message when
 * the poll countdown runs out; plays voice 0x1E when a message fades below 0x70. */
void entryMessageFade(s32 fadeStep) {
    extern StateBlock *gBudgetDisplayBlock;
    s32 messageSlot;
    s32 previousFade;
    s32 thresholdCrossed;
    s32 messageSlotTotal;

    messageSlotTotal = gBudgetDisplayBlock->entryCount;
    if (gBudgetDisplayBlock->pollCountdown <= 0 || --gBudgetDisplayBlock->pollCountdown <= 0) {
        entryMessageStartNext();
    }
    thresholdCrossed = 0;
    for (messageSlot = 0; messageSlot < messageSlotTotal; messageSlot++) {
        previousFade = gBudgetDisplayBlock->messageFade[messageSlot];
        if (previousFade != 0) {
            gBudgetDisplayBlock->messageFade[messageSlot] = previousFade - fadeStep;
            if (gBudgetDisplayBlock->messageFade[messageSlot] <= 0) {
                gBudgetDisplayBlock->messageFade[messageSlot] = 0;
                entryLevelStep(messageSlot, 0);
            }
            if (previousFade >= 0x70 && gBudgetDisplayBlock->messageFade[messageSlot] < 0x70) {
                thresholdCrossed = 1;
            }
        }
    }
    if (thresholdCrossed) {
        seVoiceStartDefault(0x1E);
    }
}

/* Clears the level timer and message fade of every entry, then calls entryLevelStep with mode 0 on each. */
void entryMessageReset(void) {
    extern StateBlock *gBudgetDisplayBlock;
    s32 messageSlot;
    s32 messageSlotTotal;

    messageSlotTotal = gBudgetDisplayBlock->entryCount;
    for (messageSlot = 0; messageSlot < messageSlotTotal; messageSlot++) {
        gBudgetDisplayBlock->levelTimer[messageSlot] = 0;
        gBudgetDisplayBlock->messageFade[messageSlot] = 0;
        entryLevelStep(messageSlot, 0);
    }
}

s32 blockStatusBitsGet(void);
s32 statusHeaderIsReadyMode20(void);
s32 statusHeaderIsReadyMode40(void);
void func_00220E00(void);

extern s8 gVsyncsPerFrame;

/* Modes of gBudgetDisplayBlock.block->mode: the budget being drained (AMOUNT_A, SCALED, REMAINING), the spent-amount
 * decay (DECAY_SPENT) and the closing hold (HOLD). Phases of block->phase: wait for the status header, drain, finish. */
enum BudgetDrainMode {
    BUDGET_DRAIN_MODE_AMOUNT_A = 0,
    BUDGET_DRAIN_MODE_HOLD = 1,
    BUDGET_DRAIN_MODE_REMAINING = 2,
    BUDGET_DRAIN_MODE_SCALED = 3,
    BUDGET_DRAIN_MODE_SPENT_DECAY = 4
};

enum BudgetDrainPhase {
    BUDGET_DRAIN_PHASE_WAIT = 0,
    BUDGET_DRAIN_PHASE_DRAIN = 1,
    BUDGET_DRAIN_PHASE_FINISH = 2
};

/* Per-frame update of the status block gBudgetDisplayBlock.block: fades the font alphas, then runs the mode machine
 * (BUDGET_DRAIN_MODE_*) and its phases (BUDGET_DRAIN_PHASE_*). */
void budgetDisplayUpdate(void) {
    extern StateBlockHandle gBudgetDisplayBlock;
    extern s8 gVsyncsPerFrame;
    s32 fontAlphaStep;

    fontAlphaStep = gVsyncsPerFrame << 4;
    func_00220E00();
    if ((s8) gBudgetDisplayBlock.block->fontAlpha[3] >= 0) {
        gBudgetDisplayBlock.block->fontAlpha[3] += fontAlphaStep;
    }
    if (gBudgetDisplayBlock.block->finishDelay != 0) {
        gBudgetDisplayBlock.block->finishDelay -= 1;
        return;
    }
    switch (gBudgetDisplayBlock.block->mode) {
    case BUDGET_DRAIN_MODE_AMOUNT_A:
        if ((s8) gBudgetDisplayBlock.block->fontAlpha[0] >= 0) {
            gBudgetDisplayBlock.block->fontAlpha[0] += fontAlphaStep;
        }
        if (gBudgetDisplayBlock.block->fontAlpha[1] != 0) {
            gBudgetDisplayBlock.block->fontAlpha[1] -= fontAlphaStep;
        }
        if (gBudgetDisplayBlock.block->fontAlpha[2] != 0) {
            gBudgetDisplayBlock.block->fontAlpha[2] -= fontAlphaStep;
        }
        if (gBudgetDisplayBlock.block->budget->scaledAmountA > 0 || gBudgetDisplayBlock.block->budget->scaledAmountB > 0) {
            gBudgetDisplayBlock.block->mode = BUDGET_DRAIN_MODE_SCALED;
            return;
        }
        switch (gBudgetDisplayBlock.block->phase) {
        case BUDGET_DRAIN_PHASE_DRAIN:
            budgetTakeAmounts(1);
            if (getStatusHeader()->held & 0x20) {
                budgetTakeAmounts(gBudgetDisplayBlock.block->drainStep + 1);
                if ((blockStatusBitsGet() & 1) == 0) {
                    seVoiceStartDefault(10);
                }
            } else if ((blockStatusBitsGet() & 3) == 0) {
                seVoiceStartDefault(10);
            }
            if (gBudgetDisplayBlock.block->budget->amountA == 0 && gBudgetDisplayBlock.block->budget->amountB == 0) {
                gBudgetDisplayBlock.block->counter = 0;
                gBudgetDisplayBlock.block->phase = BUDGET_DRAIN_PHASE_FINISH;
                mcClearSetupStatus();
                func_00220B90(64, messageTextGet(159));
                func_00220B90(32, 0);
                func_00220D28();
            }
            break;
        case BUDGET_DRAIN_PHASE_WAIT:
            if (gBudgetDisplayBlock.block->budget->amountA != 0 || gBudgetDisplayBlock.block->budget->amountB != 0) {
                s32 advancedCounter = gBudgetDisplayBlock.block->counter + 1;
                gBudgetDisplayBlock.block->counter = advancedCounter;
                if ((u16) advancedCounter >= 301 || statusHeaderIsReadyMode20()) {
                    gBudgetDisplayBlock.block->counter = 0;
                    gBudgetDisplayBlock.block->drainStep = gBudgetDisplayBlock.block->budget->amountA >> 5;
                    gBudgetDisplayBlock.block->phase = BUDGET_DRAIN_PHASE_DRAIN;
                }
            } else {
                gBudgetDisplayBlock.block->counter = 0;
                gBudgetDisplayBlock.block->phase = BUDGET_DRAIN_PHASE_FINISH;
                mcClearSetupStatus();
                func_00220B90(64, messageTextGet(159));
                func_00220B90(32, 0);
                func_00220D28();
            }
            break;
        case BUDGET_DRAIN_PHASE_FINISH:
            entryMessageFade(1);
            if (statusHeaderIsReadyMode20() || statusHeaderIsReadyMode40()) {
                if (budgetDisplayIsIdle()) {
                    seVoiceStartDefault(2);
                    budgetDisplaySetMode(2);
                } else {
                    entryMessageReset();
                }
            }
            break;
        }
        if (!statusHeaderIsReadyMode40()) {
            return;
        }
        if (gBudgetDisplayBlock.block->phase != BUDGET_DRAIN_PHASE_FINISH) {
            seVoiceStartDefault(7);
            if (budgetTakeAmounts(0xFFFFFFF)) {
                gBudgetDisplayBlock.block->finishDelay = 20;
            }
            entryLevelStepAll();
            budgetDisplaySetMode(2);
            break;
        }
        return;
    case BUDGET_DRAIN_MODE_SCALED:
        if ((s8) gBudgetDisplayBlock.block->fontAlpha[0] >= 0) {
            gBudgetDisplayBlock.block->fontAlpha[0] += fontAlphaStep;
        }
        if (gBudgetDisplayBlock.block->fontAlpha[1] != 0) {
            gBudgetDisplayBlock.block->fontAlpha[1] -= fontAlphaStep;
        }
        if (gBudgetDisplayBlock.block->fontAlpha[2] != 0) {
            gBudgetDisplayBlock.block->fontAlpha[2] -= fontAlphaStep;
        }
        switch (gBudgetDisplayBlock.block->phase) {
        case BUDGET_DRAIN_PHASE_DRAIN:
            scaledAmountsDecay();
            if (getStatusHeader()->held & 0x20) {
                scaledAmountsDecay();
                if ((blockStatusBitsGet() & 1) == 0) {
                    seVoiceStartDefault(10);
                }
            } else if ((blockStatusBitsGet() & 3) == 0) {
                seVoiceStartDefault(10);
            }
            if (gBudgetDisplayBlock.block->budget->scaledAmountA == 0 &&
                gBudgetDisplayBlock.block->budget->scaledAmountB == 0) {
                gBudgetDisplayBlock.block->counter = 0;
                gBudgetDisplayBlock.block->gaugeValue = 0;
                gBudgetDisplayBlock.block->phase = BUDGET_DRAIN_PHASE_FINISH;
                seVoiceStartDefault(30);
            }
            break;
        case BUDGET_DRAIN_PHASE_WAIT: {
            s32 advancedCounter = gBudgetDisplayBlock.block->counter + 1;
            gBudgetDisplayBlock.block->counter = advancedCounter;
            if ((u16) advancedCounter >= 301 || statusHeaderIsReadyMode20()) {
                gBudgetDisplayBlock.block->counter = 0;
                gBudgetDisplayBlock.block->phase = BUDGET_DRAIN_PHASE_DRAIN;
            }
            break;
        }
        case BUDGET_DRAIN_PHASE_FINISH: {
            if (gBudgetDisplayBlock.block->gaugeValue < 256) {
                gBudgetDisplayBlock.block->gaugeValue = gBudgetDisplayBlock.block->gaugeValue + 16;
            } else if (gBudgetDisplayBlock.block->gaugeValue < 384) {
                gBudgetDisplayBlock.block->gaugeValue = gBudgetDisplayBlock.block->gaugeValue + 4;
            }
            if (gBudgetDisplayBlock.block->gaugeValue >= 384) {
                gBudgetDisplayBlock.block->gaugeValue = 384;
                budgetDisplaySetMode(0);
            }
            if (statusHeaderIsReadyMode20()) {
                gBudgetDisplayBlock.block->gaugeValue = 384;
                gBudgetDisplayBlock.block->mode = BUDGET_DRAIN_MODE_AMOUNT_A;
                gBudgetDisplayBlock.block->counter = 0;
                gBudgetDisplayBlock.block->drainStep = gBudgetDisplayBlock.block->budget->amountA >> 5;
                gBudgetDisplayBlock.block->phase = BUDGET_DRAIN_PHASE_DRAIN;
            }
            break;
        }
        }
        if (!statusHeaderIsReadyMode40()) {
            return;
        }
        seVoiceStartDefault(7);
        gBudgetDisplayBlock.block->budget->scaledAmountA = 0;
        gBudgetDisplayBlock.block->budget->scaledAmountB = 0;
        gBudgetDisplayBlock.block->scaledCountA = 0;
        gBudgetDisplayBlock.block->scaledCountB = 0;
        if (budgetTakeAmounts(0xFFFFFFF)) {
            gBudgetDisplayBlock.block->finishDelay = 20;
        }
        entryLevelStepAll();
        budgetDisplaySetMode(2);
        break;
    case BUDGET_DRAIN_MODE_HOLD:
        if ((s8) gBudgetDisplayBlock.block->fontAlpha[1] >= 0) {
            gBudgetDisplayBlock.block->fontAlpha[1] += fontAlphaStep;
        }
        if (gBudgetDisplayBlock.block->fontAlpha[0] != 0) {
            gBudgetDisplayBlock.block->fontAlpha[0] -= fontAlphaStep;
        }
        if (gBudgetDisplayBlock.block->fontAlpha[2] != 0) {
            gBudgetDisplayBlock.block->fontAlpha[2] -= fontAlphaStep;
        }
        if (gBudgetDisplayBlock.block->phase == BUDGET_DRAIN_PHASE_FINISH) {
            s32 advancedCounter = gBudgetDisplayBlock.block->counter + 1;
            gBudgetDisplayBlock.block->counter = advancedCounter;
            if ((u16) advancedCounter >= 301 || statusHeaderIsReadyMode20()) {
                seVoiceStartDefault(2);
                budgetDisplaySetMode(0);
            }
        }
        if (!statusHeaderIsReadyMode40()) {
            return;
        }
        seVoiceStartDefault(7);
        budgetDisplaySetMode(0);
        break;
    case BUDGET_DRAIN_MODE_REMAINING:
        if ((s8) gBudgetDisplayBlock.block->fontAlpha[2] >= 0) {
            gBudgetDisplayBlock.block->fontAlpha[2] += fontAlphaStep;
        }
        if (gBudgetDisplayBlock.block->fontAlpha[0] != 0) {
            gBudgetDisplayBlock.block->fontAlpha[0] -= fontAlphaStep;
        }
        if (gBudgetDisplayBlock.block->fontAlpha[1] != 0) {
            gBudgetDisplayBlock.block->fontAlpha[1] -= fontAlphaStep;
        }
        if (gBudgetDisplayBlock.block->budget->spentAmount > 0) {
            gBudgetDisplayBlock.block->mode = BUDGET_DRAIN_MODE_SPENT_DECAY;
            return;
        }
        switch (gBudgetDisplayBlock.block->phase) {
        case BUDGET_DRAIN_PHASE_DRAIN:
            stateBlockConsume(1);
            if (getStatusHeader()->held & 0x20) {
                stateBlockConsume(gBudgetDisplayBlock.block->drainStep + 1);
                if ((blockStatusBitsGet() & 1) == 0) {
                    seVoiceStartDefault(10);
                }
            } else if ((blockStatusBitsGet() & 3) == 0) {
                seVoiceStartDefault(10);
            }
            if (gBudgetDisplayBlock.block->budget->remainingAmount == 0) {
                gBudgetDisplayBlock.block->counter = 0;
                gBudgetDisplayBlock.block->phase = BUDGET_DRAIN_PHASE_FINISH;
                mcClearSetupStatus();
                func_00220B90(64, messageTextGet(160));
                func_00220B90(32, 0);
                func_00220D28();
            }
            break;
        case BUDGET_DRAIN_PHASE_WAIT:
            if (gBudgetDisplayBlock.block->budget->remainingAmount == 0) {
                gBudgetDisplayBlock.block->counter = 0;
                gBudgetDisplayBlock.block->phase = BUDGET_DRAIN_PHASE_FINISH;
                mcClearSetupStatus();
                func_00220B90(64, messageTextGet(160));
                func_00220B90(32, 0);
                func_00220D28();
            } else {
                s32 advancedCounter = gBudgetDisplayBlock.block->counter + 1;
                gBudgetDisplayBlock.block->counter = advancedCounter;
                if ((u16) advancedCounter >= 301 || statusHeaderIsReadyMode20()) {
                    gBudgetDisplayBlock.block->counter = 0;
                    gBudgetDisplayBlock.block->drainStep = (s32) gBudgetDisplayBlock.block->budget->remainingAmount >> 5;
                    gBudgetDisplayBlock.block->phase = BUDGET_DRAIN_PHASE_DRAIN;
                }
            }
            break;
        case BUDGET_DRAIN_PHASE_FINISH:
            if (statusHeaderIsReadyMode20()) {
                seVoiceStartDefault(2);
                gBudgetDisplayBlock.block->finished = 1;
            }
            break;
        }
        if (!statusHeaderIsReadyMode40()) {
            return;
        }
        seVoiceStartDefault(7);
        stateBlockConsume(0xFFFFFFF);
        gBudgetDisplayBlock.block->finished = 1;
        break;
    case BUDGET_DRAIN_MODE_SPENT_DECAY:
        if ((s8) gBudgetDisplayBlock.block->fontAlpha[2] >= 0) {
            gBudgetDisplayBlock.block->fontAlpha[2] += fontAlphaStep;
        }
        if (gBudgetDisplayBlock.block->fontAlpha[0] != 0) {
            gBudgetDisplayBlock.block->fontAlpha[0] -= fontAlphaStep;
        }
        if (gBudgetDisplayBlock.block->fontAlpha[1] != 0) {
            gBudgetDisplayBlock.block->fontAlpha[1] -= fontAlphaStep;
        }
        switch (gBudgetDisplayBlock.block->phase) {
        case BUDGET_DRAIN_PHASE_DRAIN:
            gBudgetDisplayBlock.block->budget->spentAmount =
                (s32) ((f32) gBudgetDisplayBlock.block->budget->spentAmount * 0.85f);
            if (getStatusHeader()->held & 0x20) {
                gBudgetDisplayBlock.block->budget->spentAmount =
                    (s32) ((f32) gBudgetDisplayBlock.block->budget->spentAmount * 0.85f);
                if ((blockStatusBitsGet() & 1) == 0) {
                    seVoiceStartDefault(10);
                }
            } else if ((blockStatusBitsGet() & 3) == 0) {
                seVoiceStartDefault(10);
            }
            if (gBudgetDisplayBlock.block->budget->spentAmount == 0) {
                gBudgetDisplayBlock.block->counter = 0;
                gBudgetDisplayBlock.block->fadeValue = 0;
                gBudgetDisplayBlock.block->phase = BUDGET_DRAIN_PHASE_FINISH;
                seVoiceStartDefault(30);
            }
            break;
        case BUDGET_DRAIN_PHASE_WAIT: {
            s32 advancedCounter = gBudgetDisplayBlock.block->counter + 1;
            gBudgetDisplayBlock.block->counter = advancedCounter;
            if ((u16) advancedCounter >= 301 || statusHeaderIsReadyMode20()) {
                gBudgetDisplayBlock.block->counter = 0;
                gBudgetDisplayBlock.block->phase = BUDGET_DRAIN_PHASE_DRAIN;
            }
            break;
        }
        case BUDGET_DRAIN_PHASE_FINISH: {
            if (gBudgetDisplayBlock.block->fadeValue < 256) {
                gBudgetDisplayBlock.block->fadeValue = gBudgetDisplayBlock.block->fadeValue + 16;
            } else if (gBudgetDisplayBlock.block->fadeValue < 384) {
                gBudgetDisplayBlock.block->fadeValue = gBudgetDisplayBlock.block->fadeValue + 4;
            }
            if (gBudgetDisplayBlock.block->fadeValue >= 384) {
                gBudgetDisplayBlock.block->fadeValue = 384;
                budgetDisplaySetMode(2);
            }
            if (statusHeaderIsReadyMode20()) {
                gBudgetDisplayBlock.block->fadeValue = 384;
                gBudgetDisplayBlock.block->mode = BUDGET_DRAIN_MODE_REMAINING;
                gBudgetDisplayBlock.block->counter = 0;
                gBudgetDisplayBlock.block->drainStep = (s32) gBudgetDisplayBlock.block->budget->remainingAmount >> 5;
                gBudgetDisplayBlock.block->phase = BUDGET_DRAIN_PHASE_DRAIN;
            }
            break;
        }
        }
        if (!statusHeaderIsReadyMode40()) {
            return;
        }
        seVoiceStartDefault(7);
        gBudgetDisplayBlock.block->budget->spentAmount = 0;
        stateBlockConsume(0xFFFFFFF);
        gBudgetDisplayBlock.block->finished = 1;
        break;
    default:
        return;
    }
}

s32 runAfterCompletionIdle(s32 value);
s32 memPoolInit();
s32 memPoolReset();

/* Runs runAfterCompletionIdle once (guarded by gVsyncsPerFrame and the resume action of gSbThreadBlock != 2). */
s32 runWithStateBlockOnce(s32 runArgument) {
    extern s32 runAfterCompletionIdle();
    s32 runOutcome;
    s32 blockRunOutcome;
    s8 savedRunGuard;

    runOutcome = 0;
    savedRunGuard = gVsyncsPerFrame;
    if (gSbThreadBlock.resumeAction != 2) {
        gVsyncsPerFrame = 1;
        memPoolInit(0x980000, 0x10000);
        blockRunOutcome = runAfterCompletionIdle(runArgument);
        memPoolReset();
        gVsyncsPerFrame = savedRunGuard;
        runOutcome = blockRunOutcome;
    }
    return runOutcome;
}

s32 createStateBlock(s32 flags);
void destroyStateBlock(void);
s32 budgetDisplayRun(s32 flags);
s32 completionFlagsAny();
s32 statusBlockFrameUpdate();

/* Creates the state block, runs budgetDisplayRun on it once the queue of completionFlagsAny is empty, then releases the
 * block. Returns 0 when the block could not be created. */
s32 runAfterCompletionIdle(s32 runArgument) {
    extern s32 createStateBlock();
    s32 runOutcome;
    s32 blockOutcome;

    blockOutcome = createStateBlock();
    if (blockOutcome != 0) {
        while (completionFlagsAny() != 0) {
            statusBlockFrameUpdate();
        }
        runOutcome = budgetDisplayRun(runArgument);
        destroyStateBlock();
        blockOutcome = runOutcome;
    }
    return blockOutcome;
}

s32 rndSkipOne(s32 category, s32 item);
s32 func_001B8D08(s32 category, s32 item, s32 amount);

/* Counts each budget record (category and item, both non-zero) into the state's record list, capped at 99 per
 * record, then passes each record to func_001B8D08 with its amount (rndSkipOne plus the count). */
void budgetRecordsTally(void) {
    extern StateBlock *gBudgetDisplayBlock;
    s32 budgetIndex;
    s32 recordNumber;
    s32 category;
    s32 recordItemId;
    s32 found;
    s32 amount;

    for (budgetIndex = 0; budgetIndex < gBudgetDisplayBlock->budget->recordCount; budgetIndex++) {
        category = gBudgetDisplayBlock->budget->recordCategory[budgetIndex];
        recordItemId = gBudgetDisplayBlock->budget->recordItem[budgetIndex];
        if (category != 0 && recordItemId != 0) {
            category--;
            found = 0;
            for (recordNumber = 0; recordNumber < gBudgetDisplayBlock->recordCount; recordNumber++) {
                if (gBudgetDisplayBlock->recordCategory[recordNumber] == category &&
                    gBudgetDisplayBlock->recordItem[recordNumber] == recordItemId) {
                    gBudgetDisplayBlock->recordAmount[recordNumber]++;
                    found = 1;
                    break;
                }
            }
            if (found == 0) {
                gBudgetDisplayBlock->recordCategory[gBudgetDisplayBlock->recordCount] = category;
                gBudgetDisplayBlock->recordItem[gBudgetDisplayBlock->recordCount] = recordItemId;
                gBudgetDisplayBlock->recordAmount[gBudgetDisplayBlock->recordCount]++;
                gBudgetDisplayBlock->recordCount++;
            }
        }
    }

    for (budgetIndex = 0; budgetIndex < gBudgetDisplayBlock->recordCount; budgetIndex++) {
        if (gBudgetDisplayBlock->recordItem[budgetIndex] != 0) {
            amount = rndSkipOne(gBudgetDisplayBlock->recordCategory[budgetIndex], gBudgetDisplayBlock->recordItem[budgetIndex]) +
                     gBudgetDisplayBlock->recordAmount[budgetIndex];
            amount = (amount < 100) ? amount : 99;
            func_001B8D08(gBudgetDisplayBlock->recordCategory[budgetIndex], gBudgetDisplayBlock->recordItem[budgetIndex], amount);
        }
    }
}

s32 func_00224600(s32 amount);

/* For each entry: takes the budget amounts (active entries) or the reserve amounts (later entries) through
 * func_A87E08 and func_A88360, then shows one message step for each step taken, from the entry's last step on.
 * Returns func_00224600 of the remaining budget amount. */
s32 entryLevelsApplyAll(void) {
    extern StateBlock *gBudgetDisplayBlock;
    s32 entryIndex;
    s32 entryCount;
    s32 stepsTaken;
    s32 entryId;
    s32 nextStepNumber;

    entryCount = gBudgetDisplayBlock->entryCount;
    entryIndex = 0;
    if (entryCount > 0) {
        do {
            entryId = gBudgetDisplayBlock->entryId[entryIndex];
            nextStepNumber = func_A86DF8(entryId)->stepCount;
            if (entryIndex < gBudgetDisplayBlock->activeEntryCount) {
                stepsTaken = func_A87E08(entryId, gBudgetDisplayBlock->budget->amountA);
                func_A88360(entryId, gBudgetDisplayBlock->budget->amountB);
            } else {
                stepsTaken = func_A87E08(entryId, gBudgetDisplayBlock->reserveAmountA);
                func_A88360(entryId, gBudgetDisplayBlock->reserveAmountB);
            }
            if (stepsTaken > 0) {
                nextStepNumber++;
                do {
                    func_A88248(entryId, nextStepNumber);
                    nextStepNumber++;
                    stepsTaken--;
                } while (stepsTaken != 0);
            }
            entryIndex++;
        } while (entryIndex < entryCount);
    }
    return func_00224600(gBudgetDisplayBlock->budget->remainingAmount);
}

u32 func_AA1590(void);
s32 func_A88B40(void);
StateBudget *func_A871D8(void);
s32 statusHeaderTestFlags(void);
void sefFlagSetBit26(void);
void swapWordsSendCommand(s32 frames);

void swapWordsRestore(void);
void scratchpadSendCmd23Mode0(void *command);

static inline s32 isFinished(void) {
    extern StateBlock *gBudgetDisplayBlock;

    return gBudgetDisplayBlock->finished;
}

/* Fills the state block from the battle roster (roster 1 when flags bit 0 is set, roster 0 otherwise), sets the
 * time and the reserve amounts, then runs the frame loop until the block is finished. With flags bit 2 it runs
 * entryLevelsApplyAll instead and returns 0. */
s32 budgetDisplayRun(s32 flags) {
    extern StateBlock *gBudgetDisplayBlock;
    s32 rosterIndex;
    s32 storedCount;
    s32 k;
    s32 resolvedEntryId;
    s32 sceneCmd[5];
    StateBudget *budget;

    gBudgetDisplayBlock->budget = func_A871D8();
    if (flags & 1) {
        gBudgetDisplayBlock->totalSeconds = func_AA1590() / 30;
        gBudgetDisplayBlock->entryCount = battleRosterEntryIdCountGet(1);
        gBudgetDisplayBlock->activeEntryCount = battleRosterEntryIdCountGet(0);
        storedCount = 0;
        rosterIndex = 0;
        if (gBudgetDisplayBlock->entryCount > 0) {
            rosterIndex = 0;
            while (rosterIndex < gBudgetDisplayBlock->entryCount) {
                if (rosterIndex == 3) {
                    gBudgetDisplayBlock->activeEntryCount = storedCount;
                }
                for (k = 0; k < 2; k++) {
                    resolvedEntryId = battleRosterLinkedEntryIdGet(battleRosterEntryIdGet(rosterIndex, 1), k);
                    if (resolvedEntryId != 0) {
                        gBudgetDisplayBlock->entryId[storedCount] = resolvedEntryId;
                        storedCount++;
                    }
                }
                rosterIndex++;
            }
        }
        gBudgetDisplayBlock->entryCount = storedCount;
    } else {
        gBudgetDisplayBlock->totalSeconds = func_A88B40() / 30;
        rosterIndex = 0;
        gBudgetDisplayBlock->entryCount = battleRosterEntryIdCountGet(0);
        if (gBudgetDisplayBlock->entryCount > 0) {
            for (rosterIndex = 0; rosterIndex < gBudgetDisplayBlock->entryCount; rosterIndex++) {
                gBudgetDisplayBlock->entryId[rosterIndex] = battleRosterEntryIdGet(rosterIndex, 0);
            }
        }
        gBudgetDisplayBlock->activeEntryCount = 3;
    }

    memset(sceneCmd, 0, 20);
    sceneCmd[4] = 128;
    scratchpadSendCmd23Mode0(sceneCmd);
    budget = gBudgetDisplayBlock->budget;
    gBudgetDisplayBlock->reserveAmountA = budget->amountA * 80 / 100;
    gBudgetDisplayBlock->reserveAmountB = budget->amountB * 80 / 100;
    gBudgetDisplayBlock->scaledCountA = budget->scaledAmountA * 80 / 100;
    gBudgetDisplayBlock->scaledCountB = budget->scaledAmountB * 80 / 100;
    budgetRecordsTally();

    if (flags & 4) {
        entryLevelsApplyAll();
        return 0;
    }
    do {
        budgetDisplayUpdate();
        if (statusHeaderTestFlags()) {
            sefFlagSetBit26();
            gBudgetDisplayBlock->finished = 1;
        }
        if (isFinished()) {
            swapWordsSendCommand(30);
        }
        completionCallbackAdd(budgetDisplayDrawPage);
        ((void (*)()) statusBlockFrameUpdate)();
    } while (!isFinished());
    swapWordsRestore();
    return 0;
}

StateBlock *gBudgetDisplayBlock = 0;

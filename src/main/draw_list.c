#include "common.h"
#include "shared.h"

#include "main/sv_get_image_item_142.h"
#include "main/tu007.h"


extern DrawList svDrawList;
extern int svDrawListFlushCode;

/* A slot seen as its two doublewords. */
typedef union DrawSlot {
    Quadword quadword;
    u64 halves[2];
} DrawSlot;

/* The slot table seen as slots (DrawSlot) or as the Quadword entries of DrawList. */
typedef union DrawSlotView {
    Quadword *quads;
    DrawSlot *asSlots;
} DrawSlotView;

void xglSleep(void);

#define PAD_SELECT 0x0100
#define PAD_START 0x0800

/* A 128-bit store or load of the +0x30 analog group (a C store of a zero is `por $4,$0,$0; sq $4,0x30(pad)`;
 * QuadwordZero would drop the por and change the register allocation of xglPadRead). */
typedef unsigned int PadQuadword __attribute__((mode(TI)));


extern PadRecord PadData[2];

void xglFontDebugPrintf(int x, int y, const char *format, ...);
void xglRenderClearFrame(void);

void drawListSetFlushCode(int flushCode)
{
    svDrawListFlushCode = flushCode;
}

/* Base of the slot table. */
Quadword *drawListGetSlots(void)
{
    return svDrawList.entries;
}

/* The slot at end_index, the first one past the queued slots. */
Quadword *drawListGetNextSlot(void)
{
    return svDrawList.entries + svDrawList.end_index;
}

/* Advances end_index by one slot. */
void drawListCommitSlot(void)
{
    svDrawList.end_index = svDrawList.end_index + 1;
}

/* Advances end_index by increment slots. */
void drawListCommitSlots(int increment)
{
    svDrawList.end_index = svDrawList.end_index + increment;
}

/* Appends one slot passed by value. */
void drawListAppendSlot(Quadword slotValue)
{
    svDrawList.entries[svDrawList.end_index] = slotValue;
    svDrawList.end_index = svDrawList.end_index + 1;
}

/* Appends one slot given as its two doublewords, the low one first. */
void drawListAppendSlotHalves(u64 lowHalf, u64 highHalf)
{
    DrawSlotView slotView;

    /* the slot is written through the DrawSlot union slotView (the halves are u64 stores); entries is a Quadword * */
    slotView.quads = &svDrawList.entries[svDrawList.end_index];
    slotView.asSlots->halves[0] = lowHalf;
    slotView.asSlots->halves[1] = highHalf;
    svDrawList.end_index = svDrawList.end_index + 1;
}

/* Appends four consecutive slots copied from src. */
void drawListAppendFourSlots(const Quadword *srcSlots)
{
    int firstSlot = svDrawList.end_index;
    svDrawList.end_index = firstSlot + 4;
    svDrawList.entries[firstSlot] = srcSlots[0];
    svDrawList.entries[firstSlot + 1] = srcSlots[1];
    svDrawList.entries[firstSlot + 2] = srcSlots[2];
    svDrawList.entries[firstSlot + 3] = srcSlots[3];
}

/* Appends one slot copied from src. */
void drawListAppendSlotCopy(const Quadword *srcSlot)
{
    svDrawList.entries[svDrawList.end_index++] = *srcSlot;
}

/* Shows the reset message until SELECT and START are held together. */
void resetWaitForPad(void)
{
    while ((PadData[0].heldAndPressed & (PAD_SELECT | PAD_START)) != (PAD_SELECT | PAD_START)) {
        xglFontDebugPrintf(8, 8, "RESET");
        xglRenderClearFrame();
        xglSleep();
    }
}

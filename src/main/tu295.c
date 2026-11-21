#include "common.h"

typedef int s32;
typedef long long s64;
typedef signed char s8;
typedef unsigned char u8;
typedef float f32;
extern s32 gScratchBatchPending;

/* Record passed to scratchWindowLoadRecord: value at 0x10, two flag bytes at 0xA8 and 0xA9. */
struct ScratchSourceWindow {
    u8 unk_00[0x10];
    s64 loadedValue;
    u8 unk_18[0x90];
    u8 flagA;
    u8 flagB;
};

/* The pendingScratch state at gScratchWindows is reached from one chunk (0x30 bytes) below the symbol: chunk[-1] is the header. */
typedef struct ScratchWindow {
    u8 unk_00[0x10];
    u8 pendingScratch[0x10];                   /* +0x10 */
    u8 previousScratch[0x10];                      /* +0x20 */
} ScratchWindow;

/* Block at gScratchWindows - 0x30 that scratchWindowLoadRecord fills from a record. */
struct ScratchHeaderWindow {
    u8 hasPreviousScratch;
    u8 unk_01[0x17];
    s64 loadedValue;
    u8 unk_20[0x28];
    s8 flagA;
    s8 flagB;
    u8 unk_4A[2];
    void *loadedFrom;
};

void scratchBatchPendingClear(void) {
    gScratchBatchPending = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu295", func_00273E38);

INCLUDE_ASM("asm/main/nonmatchings/tu295", func_00273F78);

INCLUDE_ASM("asm/main/nonmatchings/tu295", func_00274058);

void xglPacketScratchFlush(void);
void xglPacketScratchGetBatch(void **quadsOut, s32 **countOut);
void func_00274108(void *quads, s32 *count);
void xglPacketScratchAddQuad(s64 high, s64 low);
void xglPacketScratchBegin(void);

void scratchBatchFlushIfPending(void)
{
  void *quadsOut;
  s32 *quadBatchCountOut;
  if (gScratchBatchPending != 0)
  {
    xglPacketScratchBegin();
    xglPacketScratchGetBatch(&quadsOut, &quadBatchCountOut);
    xglPacketScratchAddQuad(0xE, ((0x10000000LL) << 0x20) | 0x8000);
    xglPacketScratchAddQuad(0x14, 0);
    func_00274108(quadsOut, quadBatchCountOut);
    xglPacketScratchFlush();
    gScratchBatchPending = 0;
  }
}

INCLUDE_ASM("asm/main/nonmatchings/tu295", func_00274108);

INCLUDE_ASM("asm/main/nonmatchings/tu295", func_002744D0);

INCLUDE_ASM("asm/main/nonmatchings/tu295", func_002747B8);

extern ScratchWindow gScratchWindows[];
void func_00267108(void *dest, void *source);
void func_002671F0(void *first, void *second);

void scratchWindowLoadRecord(struct ScratchSourceWindow *sourceWindow)
{
  s64 loadedValue;
  struct ScratchHeaderWindow *headerWindow;
  func_00267108(gScratchWindows, &sourceWindow->loadedValue);
  headerWindow = (struct ScratchHeaderWindow *) &gScratchWindows[-1];
  memset(&gScratchWindows[-1].pendingScratch, 0, 0x10);
  loadedValue = sourceWindow->loadedValue;
  headerWindow->loadedValue = loadedValue;
  if (headerWindow->hasPreviousScratch != 0)
  {
    func_002671F0(&gScratchWindows[-1].previousScratch, &gScratchWindows[0].pendingScratch);
  }
  headerWindow->flagA = sourceWindow->flagA;
  headerWindow->flagB = sourceWindow->flagB;
  headerWindow->loadedFrom = sourceWindow;
}

INCLUDE_ASM("asm/main/nonmatchings/tu295", func_002749C0);

INCLUDE_ASM("asm/main/nonmatchings/tu295", func_00274C08);

INCLUDE_ASM("asm/main/nonmatchings/tu295", func_00275070);

void func_002747B8(s32 index, s32 mode, f32 value);

void slotFloatSet(s32 slotNumber, f32 value) {
    func_002747B8(slotNumber, 0, value);
}

void scratchEmptyHook(void) {

}

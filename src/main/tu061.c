#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu061", func_001B9850);

typedef signed char s8;
typedef int s32;
typedef unsigned int u32;
extern s32 vmStreamHandles[];

s32 vmHandleTableGet(u32 streamNumber) {
    if (!(streamNumber < 0x12U)) {
        return 0;
    }
    return vmStreamHandles[streamNumber];
}

/* gGameLoopRuntimeStorage at the offsets this file uses (the game loop runtime block). */
typedef struct GameLoopRuntime {
    unsigned char unk_000[0x18C];
    s32 streamAddress;             /* 0x18C */
    unsigned char unk_190[0x4];
    s32 streamSize;             /* 0x194 */
    s8 ready;                   /* 0x198 */
} GameLoopRuntime;

/* Payload header returned by xcLocatePayload. */
typedef struct XcDataSection {
    unsigned char unk_00[0x10];
    s32 contentOffset;             /* 0x10, added to the header address */
    s32 contentSize;               /* 0x14 */
} XcDataSection;

extern GameLoopRuntime gGameLoopRuntimeStorage[];
extern unsigned char vmPayloadBuffer[];
s32 func_0018EBE8(s32 mode);
s32 ioRpcFetchBlock(s32 source, s32 target, s32 size);
s32 poolBlockTagWordGet(s32 handle, s32 mode);
s32 func_001B6918(s32 unit, s32 channel, s32 command, s32 timeout);
s32 xcLocatePayload(XcDataSection **payload, s32 handle);
s32 func_001B9BF0();
s32 func_001CFD50(s32 target, void *source, void *destination);
s32 xglSleep(void);

s32 vmStreamPoll(s32 resetRequested)
{
  GameLoopRuntime *vmStream = gGameLoopRuntimeStorage;
  XcDataSection *fetchedPayload;
  s32 ready;
  s32 fetchRequestId;
  ready = vmStream->ready != 0;
  if ((vmStream->streamSize != 0) && (ready != 0))
  {
    ioRpcFetchBlock(vmStream->streamAddress, 0xA50000, vmStream->streamSize);
    func_001CFD50(0xA50000, 0, &vmStream->ready);
    while (func_0018EBE8(7) > 0)
    {
      xglSleep();
    }

  }
  if (resetRequested != 0)
  {
    func_001B9BF0();
  }
  fetchRequestId = func_001B6918(0, 1, 0xCF010300, -1);
  if (fetchRequestId != 0)
  {
    poolBlockTagWordGet(fetchRequestId, 0);
    if (((xcLocatePayload(&fetchedPayload, fetchRequestId) != 0) && (fetchedPayload->contentSize != 0)) && (ready != 0))
    {
      func_001CFD50(0, (s8 *) fetchedPayload + fetchedPayload->contentOffset, vmPayloadBuffer);
      while (func_0018EBE8(0xE) > 0)
      {
        xglSleep();
      }

    }
  }
  return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu061", func_001B9B88);

INCLUDE_ASM("asm/main/nonmatchings/tu061", func_001B9BF0);

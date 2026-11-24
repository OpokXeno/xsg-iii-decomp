#include "common.h"

typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;

/* Header of one slot buffer: length is the size of the data it holds (+0x0C). */
typedef struct XglCdFileBlock {
    u8 unk_00[0x0C];
    s32 length;                 /* +0x0C */
} XglCdFileBlock;

/* Bounds of the read ring: readRingSize is the size of its data, so the ring ends at 0x1800 + readRingSize (+0x14). */
typedef struct XglCdStream {
    u8 unk_00[0x14];
    s32 readRingSize;             /* +0x14 */
} XglCdStream;

/* The CD block-stream loader gXglCdBlockStream (gXglCdBlockStreamStorage) (shared with the other TUs that use it; this file reads these fields). */
typedef struct XglCdBlockStream {
    u8 unk_0000[0x1800];
    XglCdFileBlock *slotBuffers[4]; /* +0x1800: the buffer of each read slot */
    char fileName[0xC0];        /* +0x1810: the file name the slot reads use */
    XglCdStream *stream;        /* +0x18D0 */
    s32 currentActorRecord;            /* +0x18D4: xglCdPollRead does nothing while 0 */
    u8 unk_18D8[0x2628];
    u32 readSlotNumber;                   /* +0x3F00: the slot read last, 0 to 3 */
    u32 lastPolledSlot;               /* +0x3F04: the slot xglCdPollRead last saw */
    s32 readStage;                  /* +0x3F08: 0 idle, 2 start the next slot read */
    u8 unk_3F0C[0x0C];
    u32 readPosition;           /* +0x3F18: where the next read of the slot starts */
} XglCdBlockStream;

extern XglCdBlockStream gXglCdBlockStreamStorage[];
#define gXglCdBlockStream (gXglCdBlockStreamStorage[0])

void xglCdSetReadBuffers(XglCdFileBlock *ringSlotZeroBlock, XglCdFileBlock *ringSlotOneBlock, XglCdFileBlock *ringSlotTwoBlock, XglCdFileBlock *ringSlotThreeBlock)
{
  gXglCdBlockStream.slotBuffers[0] = ringSlotZeroBlock;
  gXglCdBlockStream.slotBuffers[1] = ringSlotOneBlock;
  gXglCdBlockStream.slotBuffers[2] = ringSlotTwoBlock;
  gXglCdBlockStream.slotBuffers[3] = ringSlotThreeBlock;
}

extern s32 xglCdPendingReadStage[];

void xglCdQueueSlotRead(void) {
    xglCdPendingReadStage[0] = 2;
}

void xglCdRingReadNoNotify(void);
s32 xglCdReadFilePart(const char *name, void *buffer, s32 mode, void (*notify)(void), s32 offset, s32 length);

void xglCdIssueSlotRead(void)
{
  u32 nextReadSlotNumber;
  nextReadSlotNumber = gXglCdBlockStream.readSlotNumber + 1;
  gXglCdBlockStream.readSlotNumber = nextReadSlotNumber;
  if (nextReadSlotNumber >= 4U)
  {
    gXglCdBlockStream.readSlotNumber = 0U;
  }
  xglCdReadFilePart(gXglCdBlockStream.fileName, gXglCdBlockStream.slotBuffers[gXglCdBlockStream.readSlotNumber], 1, xglCdRingReadNoNotify, gXglCdBlockStream.readPosition, 0x800);
  gXglCdBlockStream.readStage = 1;
}

s32 xglCdGetServicePending(void);

void xglCdPollRead(void)
{
  u32 nextReadSlotNumber;
  if (!(gXglCdBlockStream.currentActorRecord != 0))
  {
    return;
  }
  nextReadSlotNumber = gXglCdBlockStream.readSlotNumber + 1;
  if (nextReadSlotNumber >= 4U)
  {
    nextReadSlotNumber = 0U;
  }
  if (gXglCdBlockStream.lastPolledSlot != nextReadSlotNumber)
  {
    if (xglCdGetServicePending() == 0)
    {
      xglCdQueueSlotRead();
    }
  }
}

void xglCdIssueSlotRead(void);

void xglCdServiceSlotRead(void)
{
  u32 readRingEnd;
  u32 readPosition;
  XglCdFileBlock *finishedReadBlock;
  if (gXglCdBlockStream.readStage != 0)
  {
    if (gXglCdBlockStream.readStage == 2)
    {
      xglCdIssueSlotRead();
      return;
    }
    if (xglCdGetServicePending() == 0)
    {
      gXglCdBlockStream.readStage = 0;
      finishedReadBlock = gXglCdBlockStream.slotBuffers[gXglCdBlockStream.readSlotNumber];
      xglCdReadFilePart(gXglCdBlockStream.fileName, finishedReadBlock, 1, xglCdRingReadNoNotify, gXglCdBlockStream.readPosition, finishedReadBlock->length + 0xB0);
      readPosition = (gXglCdBlockStream.readPosition + finishedReadBlock->length) + 0xB0;
      gXglCdBlockStream.readPosition = readPosition;
      readRingEnd = gXglCdBlockStream.stream->readRingSize + 0x1800;
      if (readPosition >= readRingEnd)
      {
        gXglCdBlockStream.readPosition = 0x1800U;
      }
    }
  }
}

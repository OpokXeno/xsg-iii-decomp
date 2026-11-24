#include "common.h"

typedef unsigned char u8;

typedef int s32;

typedef signed char s8;

typedef unsigned int u32;

typedef short s16;

typedef float f32;

extern s32 gSaveBitmapA[];

extern unsigned char gSaveFlagBytes[];

extern s32 gSaveWordTable[];

extern s32 gSaveBitmapB[];

extern s8 gSaveRndTable[];

extern s32 gSaveStateIndex;

extern s32 gSaveRndState;

void *memset(void *destination, int value, unsigned int count);

/* Object whose position stateSave copies. Its 0x24 field points at a sub-object that also supplies one float. */
typedef struct PositionObject {
    u8 unk_00[0x24];
    struct StateSubobject *subobject;   /* 0x24 */
    u8 unk_28[0x40 - 0x28];
    f32 z;                              /* 0x40 */
    f32 y;                              /* 0x44 */
    f32 x;                              /* 0x48 */
} PositionObject;

typedef struct StateSubobject {
    u8 unk_00[0x54];
    f32 w;                              /* 0x54 */
} StateSubobject;

/* Save image written by stateSave and read back by stateRestore (0xA40 bytes). */
typedef struct StateSaveImage {
    s32 saveTag;                        /* 0x00: copy of gSaveTags[0] */
    f32 savedZ;                         /* 0x04 */
    f32 savedY;                         /* 0x08 */
    f32 savedX;                         /* 0x0C */
    f32 savedW;                         /* 0x10 */
    s32 saveStateIndex;                     /* 0x14 */
    u8 bitmapA[0x80];                   /* 0x18: copy of gSaveBitmapA */
    u8 saveFlagBytes[0x20];                 /* 0x98: copy of gSaveFlagBytes */
    u8 saveWordTable[0x200];                 /* 0xB8: copy of gSaveWordTable */
    u8 bitmapB[0x80];                   /* 0x2B8: copy of gSaveBitmapB */
    u8 rndTable[0x6C0];                 /* 0x338: copy of gSaveRndTable */
    s32 rndState;                       /* 0x9F8: copy of gSaveRndState */
    u8 tail[0x44];                      /* 0x9FC: copy of D_0031A560 */
} StateSaveImage;

void clearSaveTables(void) {
    gSaveStateIndex = 0;
    memset(gSaveBitmapA, 0, 0x80);
    memset(gSaveFlagBytes, 0, 0x20);
    memset(gSaveWordTable, 0, 0x200);
    memset(gSaveBitmapB, 0, 0x80);
    memset(gSaveRndTable, 0, 0x6C0);
    gSaveRndState = 0;
}

s32 saveStateIndexGet(void) {
    return gSaveStateIndex;
}

s32 saveStateIndexSet(s32 saveStateIndex) {
    s32 storedIndex;

    storedIndex = saveStateIndex;
    gSaveStateIndex = storedIndex;
    return storedIndex;
}

s32 bitmapTestBit(s32 bitIndex)
{
  unsigned int masked;
  s32 adjusted;
  adjusted = bitIndex;
  if (bitIndex < 0)
  {
    adjusted = bitIndex + 0x1F;
  }
  masked = gSaveBitmapA[adjusted >> 5] & (1 << bitIndex);
  return masked != 0;
}

s32 bitmapSetBit(s32 bitIndex, s32 newBit) {
    s32 *bitmapWord;
    s32 adjusted;
    s32 bitmapWordIndex;
    s32 updatedWord;
    s32 bitInWord;
    s32 bitValue;

    adjusted = bitIndex;
    if (bitIndex < 0) {
        adjusted = bitIndex + 0x1F;
    }
    bitValue = newBit & 1;
    bitmapWordIndex = adjusted >> 5;
    bitInWord = bitIndex - (bitmapWordIndex << 5);
    bitmapWord = gSaveBitmapA + bitmapWordIndex;
    updatedWord = (*bitmapWord & ~(1 << bitInWord)) | (bitValue << bitInWord);
    *bitmapWord = updatedWord;
    return bitValue;
}

u8 saveFlagByteGet(s32 flagNumber) {
    return gSaveFlagBytes[flagNumber & 0x1F];
}

s32 saveFlagByteSet(s32 flagNumber, s32 storedFlagByte) {
    gSaveFlagBytes[flagNumber & 0x1F] = storedFlagByte;
    return storedFlagByte & 0xFF;
}

INCLUDE_ASM("asm/main/nonmatchings/tu060", func_001B8B58);

INCLUDE_ASM("asm/main/nonmatchings/tu060", func_001B8BA0);

void clearSaveWordTable(void)
{
  s32 i;
  s32 *cursor;
  cursor = gSaveWordTable;
  cursor += 127;
  for (i = 0; i < 64; i++)
  {
    *cursor = 0;
    cursor -= 1;
  }

}

s32 bitmapSecondTestBit(s32 bitIndex)
{
  s32 adjusted;
  s32 zero;
  adjusted = bitIndex;
  if (bitIndex < 0)
  {
    adjusted = bitIndex + 0x1F;
  }
  zero = 0;
  return (gSaveBitmapB[adjusted >> 5] & (1 << bitIndex)) != zero;
}

s32 bitmapSecondSetBit(s32 bitIndex, s32 newBit) {
    s32 *bitmapWord;
    s32 adjusted;
    s32 bitmapWordIndex;
    s32 updatedWord;
    s32 bitInWord;
    s32 bitValue;

    adjusted = bitIndex;
    if (bitIndex < 0) {
        adjusted = bitIndex + 0x1F;
    }
    bitValue = newBit & 1;
    bitmapWordIndex = adjusted >> 5;
    bitInWord = bitIndex - (bitmapWordIndex << 5);
    bitmapWord = gSaveBitmapB + bitmapWordIndex;
    updatedWord = (*bitmapWord & ~(1 << bitInWord)) | (bitValue << bitInWord);
    *bitmapWord = updatedWord;
    return bitValue;
}

s32 Get_Rnd_001B8CD0(s32 rndKind, s32 rndIndex)
{
  s32 rndTableValue;
  if (rndKind == 0xA)
  {
    return gSaveRndState;
  }
  rndTableValue = gSaveRndTable[rndIndex + (rndKind * 0xC0)];
  return rndTableValue;
}

INCLUDE_ASM("asm/main/nonmatchings/tu060", func_001B8D08);

INCLUDE_ASM("asm/main/nonmatchings/tu060", func_001B8DB8);

INCLUDE_ASM("asm/main/nonmatchings/tu060", func_001B8EB0);

s32 func_001B9198(s32 address, s32 value);

s32 hwStatusReady(void) {
    return func_001B9198(0xCF801000, 0x020B0001) != 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu060", func_001B8FA0);

INCLUDE_ASM("asm/main/nonmatchings/tu060", func_001B9198);

INCLUDE_ASM("asm/main/nonmatchings/tu060", func_001B9228);

INCLUDE_ASM("asm/main/nonmatchings/tu060", func_001B93C0);

INCLUDE_ASM("asm/main/nonmatchings/tu060", func_001B9438);

s32 Get_Rnd_001B8CD0();

s32 func_00222848(s32 kind, s32 mode, s32 index);

s32 rndRollKind(s32 rollKind) {
    s32 rndTableIndex;
    s32 rolled;
    s32 rollValue;

    rolled = 0;
    rndTableIndex = rollKind + 0x40;
    if (rollKind - 1 < 0xAU) {
        if (rollKind == 0xA) {
            rollValue = Get_Rnd_001B8CD0(8, 0x31);
        } else {
            if (Get_Rnd_001B8CD0(2, rndTableIndex) != 0) {
                return 1;
            }
            rollValue = func_00222848(rollKind, 2, rndTableIndex);
        }
        rolled = rollValue != 0;
        return rolled;
    }
    return rolled;
}

INCLUDE_ASM("asm/main/nonmatchings/tu060", func_001B9590);

extern unsigned char D_0031A560[];

extern s16 gSaveTags[];

PositionObject *RgDispModelFindByKey(s32 mode);

void func_0030C130(void *source, void *destination, s32 size);

s32 stateSave(StateSaveImage *saveImage) {
    PositionObject *positionObject;
    s32 savedIndex;
    s32 savedCount;

    positionObject = RgDispModelFindByKey(0);
    memset(saveImage, 0, 0xA40);
    saveImage->saveTag = gSaveTags[0];
    if (positionObject != 0) {
        saveImage->savedZ = positionObject->z;
        saveImage->savedY = positionObject->y;
        saveImage->savedX = positionObject->x;
        saveImage->savedW = positionObject->subobject->w;
    }
    savedIndex = gSaveStateIndex;
    saveImage->saveStateIndex = savedIndex;
    func_0030C130(gSaveBitmapA, saveImage->bitmapA, 0x80);
    func_0030C130(gSaveFlagBytes, saveImage->saveFlagBytes, 0x20);
    func_0030C130(gSaveWordTable, saveImage->saveWordTable, 0x200);
    func_0030C130(gSaveBitmapB, saveImage->bitmapB, 0x80);
    func_0030C130(gSaveRndTable, saveImage->rndTable, 0x6C0);
    savedCount = gSaveRndState;
    saveImage->rndState = savedCount;
    func_0030C130(D_0031A560, saveImage->tail, 0x44);
    return 1;
}

s32 stateRestore(StateSaveImage *saveImage) {
    s32 savedIndex;
    s32 savedCount;

    savedIndex = saveImage->saveStateIndex;
    gSaveStateIndex = savedIndex;
    func_0030C130(saveImage->bitmapA, gSaveBitmapA, 0x80);
    func_0030C130(saveImage->saveFlagBytes, gSaveFlagBytes, 0x20);
    func_0030C130(saveImage->saveWordTable, gSaveWordTable, 0x200);
    func_0030C130(saveImage->bitmapB, gSaveBitmapB, 0x80);
    func_0030C130(saveImage->rndTable, gSaveRndTable, 0x6C0);
    savedCount = saveImage->rndState;
    gSaveRndState = savedCount;
    func_0030C130(saveImage->tail, D_0031A560, 0x44);
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu060", func_001B97F0);

INCLUDE_ASM("asm/main/nonmatchings/tu060", func_001B9850);

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

s32 soundBankTagsSet(s32 target, void *source, void *destination);

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
    soundBankTagsSet(0xA50000, 0, &vmStream->ready);
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
      soundBankTagsSet(0, (s8 *) fetchedPayload + fetchedPayload->contentOffset, vmPayloadBuffer);
      while (func_0018EBE8(0xE) > 0)
      {
        xglSleep();
      }

    }
  }
  return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu060", func_001B9B88);

INCLUDE_ASM("asm/main/nonmatchings/tu060", func_001B9BF0);

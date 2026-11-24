#include "common.h"

typedef signed char s8;
typedef int s32;
typedef unsigned char u8;
s32 func_00250768();

typedef struct XglCdBlockStream {
    u8 unk_0000[0x18D8];
    s32 registeredIds[0x98D];
    s32 registeredIdCount;
    u8 unk_3F10[0xC];
    s32 unk_3F1C;
    s32 unk_3F20;
    s32 unk_3F24;
    u8 unk_3F28[0x8];
    s32 unk_3F30;
    s32 unk_3F34;
    s32 unk_3F38;
    u8 unk_3F3C[0x10];
    s32 cdReadyBits;
    s32 unk_3F50;
} XglCdBlockStream;

extern XglCdBlockStream gXglCdBlockStreamStorage[];
#define gXglCdBlockStream (gXglCdBlockStreamStorage[0])

void xglCdInitReadRing(void)
{
  gXglCdBlockStream.unk_3F1C = 1;
  gXglCdBlockStream.unk_3F24 = 0;
  gXglCdBlockStream.unk_3F30 = 0;
  gXglCdBlockStream.unk_3F34 = 0;
  gXglCdBlockStream.unk_3F38 = 0;
  gXglCdBlockStream.unk_3F50 = 0;
  func_00250768();
}

extern s32 gXglCdDriveStatus[];

void xglCdSetDriveStatus(s32 driveStatus) {
    gXglCdDriveStatus[0] = driveStatus;
}

void xglCdAddReadyBits(s32 bitsToSet)
{
  s32 combined;
  combined = gXglCdBlockStream.cdReadyBits | bitsToSet;
  gXglCdBlockStream.cdReadyBits = combined;
}

extern unsigned char gXglCdFileName[];
s32 xglDebFileNameSet();
s32 func_0024E5A8();

void xglCdSetReadyFlag(void)
{
  s32 combined;
  combined = gXglCdBlockStream.cdReadyBits | 1;
  gXglCdBlockStream.cdReadyBits = combined;
  xglDebFileNameSet(gXglCdFileName);
  func_0024E5A8();
}

extern s32 gXglCdDriveReadyStatus[];

s32 xglCdGetReadyStatus(void) {
    return gXglCdDriveReadyStatus[0];
}

s32 xglCdGetDriveStatus(void) {
    return gXglCdDriveStatus[0];
}

INCLUDE_ASM("asm/main/nonmatchings/tu210", func_0024E5A8);

s32 func_0028D670();

s32 xglCdRegisterEntry(void)
{
  s32 entryId;
  s32 newRegisteredIdCount;
  s32 *idSlot;
  XglCdBlockStream *blockStream;
  entryId = func_0028D670();
  blockStream = &gXglCdBlockStream;
  newRegisteredIdCount = blockStream->registeredIdCount + 1;
  idSlot = &blockStream->registeredIds[blockStream->registeredIdCount];
  blockStream->registeredIdCount = newRegisteredIdCount;
  *idSlot = entryId;
  return entryId;
}

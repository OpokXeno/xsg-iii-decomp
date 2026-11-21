#include "common.h"
#include "shared.h"

void sefSetFogOverride(Vector4 *vectorA, Vector4 *vectorB);

#include "quadword.h"
#include "vu0_macro.h"


extern AlignedVector4 sefOverrideVectorA;
extern AlignedVector4 sefOverrideVectorB;

INCLUDE_ASM("asm/main/nonmatchings/sef_117", func_001E8AF0);

typedef int s32;
typedef long long s64;

/* A word set when either override vector was supplied; sefGetFogOverrideBlock hands out its address. */
extern s32 sefOverrideFlag[4];
extern s64 gSbStudioBlock[];
extern s32 sefEffectRecordCount;
extern s32 sefLineDataCount;
void func_001E8AF0();
void *memset();
extern s32 sefEffectRecords;
extern s32 sefLineDataRecords;
extern s32 sefPtAllocCount;
extern s32 sefPtAllocatorState;

s32 sefInitRecordPools(s32 recordPoolMemory)
{
  s64 *header;
  s32 sectionBase;
  s64 flags;
  func_001E8AF0();
  header = gSbStudioBlock;
  flags = (header[2] & (~0xC)) | 8;
  header[2] = flags;
  sefEffectRecordCount = 0x28;
  sefLineDataCount = 0xAA;
  sefPtAllocCount = 0x44C;
  sefPtAllocatorState = recordPoolMemory;
  memset((void *) recordPoolMemory, 0, 0xA7960);
  sectionBase = recordPoolMemory + 0xA7960;
  sefLineDataRecords = sectionBase;
  memset((void *) sectionBase, 0, 0x5C4E0);
  sectionBase = recordPoolMemory + 0x103E40;
  sefEffectRecords = sectionBase;
  memset((void *) sectionBase, 0, 0x3480);
  return 0x107340;
}

INCLUDE_ASM("asm/main/nonmatchings/sef_117", func_001E8CB8);

/*
 * Copies the two override vectors; a null source resets that vector to the
 * origin (0, 0, 0, 1). The flag records whether any override was given.
 */
void sefSetFogOverride(Vector4 *vectorA, Vector4 *vectorB)
{
    if (vectorA != 0) {
        QuadwordCopyT0(&sefOverrideVectorA, vectorA);
    } else {
        Vu0SetOrigin(&sefOverrideVectorA);
    }
    if (vectorB != 0) {
        QuadwordCopyT0(&sefOverrideVectorB, vectorB);
    } else {
        Vu0SetOrigin(&sefOverrideVectorB);
    }
    sefOverrideFlag[0] = vectorA != 0 || vectorB != 0;
}

void func_001E8E70();
void srsSetEffectFlag(s32 lookup, s32 value);
void eBattleWinReset(void);

void sefResetBattleWinState(void) {
    func_001E8E70();
    eBattleWinReset();
    srsSetEffectFlag(0, 1);
}

INCLUDE_ASM("asm/main/nonmatchings/sef_117", func_001E8E70);

INCLUDE_ASM("asm/main/nonmatchings/sef_117", func_001E8F78);

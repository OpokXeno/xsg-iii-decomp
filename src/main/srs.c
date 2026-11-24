#include "common.h"

typedef int s32;

extern char xglDebFileNameBuffer[];

char *strcpy(char *destination, const char *source);

void xglDebFileNameSet(const char *text) {
    strcpy(xglDebFileNameBuffer, text);
}

int xglDebFileRead(const char *name, void *buffer);

int xglDebFileSize(const char *name);

int xglDebFileReadSize(void *fileBuffer, const char *name)
{
    if (xglDebFileRead(name, fileBuffer) >= 0) {
        return xglDebFileSize(name);
    }
    return -1;
}

s32 func_0030E8E4(s32);

s32 xglDebFileBufferSize(s32 fileSize) {
    s32 paddedSize;

    paddedSize = fileSize + 0x14;
    return (func_0030E8E4(paddedSize) == 0) ? 0 : paddedSize;
}

/* Sprintf-shaped varargs callee; the link name of 0x0030E1B8 (test_050.c declares it the same way). */
int GameMpeg2Play(char *destination, const char *format, ...);

/* Kind-prefix pointer table at 0x004AD6A0 (scaffold data of this TU). */
extern char *srsEffectKindPrefix[4];

void srsFormatEffectStem(int effectNo, char *stemBuffer, int withSubNo)
{
    int effectKind = (effectNo >> 28) & 3;
    int effectIndex = (effectNo >> 12) & 0x3ff;
    int effectSub = effectNo & 0x3ff;

    if (effectNo < 0) {
        GameMpeg2Play(stemBuffer, "bad effect no:%d", effectNo);
        return;
    }
    if (effectKind == 3) {
        GameMpeg2Play(stemBuffer, "%s%04d", srsEffectKindPrefix[3], effectNo & 0x0fffffff);
        return;
    }
    if (withSubNo != 0) {
        GameMpeg2Play(stemBuffer, "%s%03d_%03d", srsEffectKindPrefix[effectKind], effectIndex, effectSub);
        return;
    }
    GameMpeg2Play(stemBuffer, "%s%03d", srsEffectKindPrefix[effectKind], effectIndex);
}

INCLUDE_ASM("asm/main/nonmatchings/srs", func_0020F790);

typedef signed char s8;

typedef unsigned char u8;

typedef unsigned int u32;

extern s32 gSrsEffectFileSuffixKindThree[];

extern s32 gSrsEffectFileSuffixKindBelowThree[];

extern s8 gSrsEffectNameBuffer[];

s32 func_0030E3D0(s8 *stemBuffer, s32 *format);

s8 *srsEffectNameGet(s32 effectNo) {
    if (!(effectNo > 0)) {
        return 0;
    }
    srsFormatEffectStem(effectNo, gSrsEffectNameBuffer, 1);
    if (((effectNo >> 0x1C) & 3) == 3) {
        func_0030E3D0(gSrsEffectNameBuffer, gSrsEffectFileSuffixKindThree);
    } else {
        func_0030E3D0(gSrsEffectNameBuffer, gSrsEffectFileSuffixKindBelowThree);
    }
    return gSrsEffectNameBuffer;
}

extern const s8 gSrsAssetFormatCeRepeatTag[];

extern const s8 gSrsAssetFormatCdChars[];

extern const s8 gSrsAssetFormatEfPosition[];

extern const s8 gSrsAssetFormatEfFallback[];

extern const s8 gSrsAssetFormatNameOnly[];

extern s8 gSrsAssetNameBuffer[];

u32 func_00181C18();

s32 xglCdGetFileSize();

s8 *srsBuildAssetName(s8 *name, s32 isLoaded)
{
    s8 nameSixthChar;
    s8 nameSeventhChar;
    u32 position;

    if (isLoaded != 0) {
        if (name[0] == 'c' && name[1] == 'e' && (nameSixthChar = name[6]) == '9' && (nameSeventhChar = name[7]) == nameSixthChar && name[8] == nameSeventhChar) {
            position = func_00181C18(0);
            GameMpeg2Play(gSrsAssetNameBuffer, gSrsAssetFormatCeRepeatTag, position, (position >> 8) & 0xFFFF, name);
        } else {
            GameMpeg2Play(gSrsAssetNameBuffer, gSrsAssetFormatCdChars, name[0], name[1], name[2], name);
        }
    } else if (name[0] == 'e' && name[1] == 'f') {
        position = func_00181C18(0);
        GameMpeg2Play(gSrsAssetNameBuffer, gSrsAssetFormatEfPosition, position, (position >> 8) & 0xFFFF, name);
        if (xglCdGetFileSize(gSrsAssetNameBuffer) < 0) {
            GameMpeg2Play(gSrsAssetNameBuffer, gSrsAssetFormatEfFallback, name);
        }
    } else {
        GameMpeg2Play(gSrsAssetNameBuffer, gSrsAssetFormatNameOnly, name);
    }
    return gSrsAssetNameBuffer;
}

s8 *srsEffectNameGet(s32 effectNo);

s8 *srsBuildAssetName(s8 *name, s32 isLoaded);

s32 func_0020FE10(s32);

void srsEffectPrepare(s32 effectNo) {
    s8 *effectName;
    s32 assetLoadResult;

    effectName = srsEffectNameGet(effectNo);
    if (effectName != 0) {
        assetLoadResult = func_0020FE10(effectNo);
        srsBuildAssetName(effectName, assetLoadResult == 0);
    }
}

s32 srsNormalizeEffectKey(s32 effectKey)
{
  u32 effectIndex;
  effectIndex = (effectKey >> 0xC) & 0x3FF;
  if ((!((effectKey >> 0x1C) & 3)) && ((effectIndex - 0x12C) < 0x29U))
  {
    effectKey = ((effectKey - 1) & (~0xF)) + 1;
  }
  return effectKey;
}

/* Maps a kind-0 key whose index is 300..340 to effectSlot (key - 1) & 0xF; any other key gives 100. */
int srsGetEffectSlot(int effectKey)
{
    u32 effectIndex = (effectKey >> 12) & 0x3ff;
    int effectSlot = 100;

    if (((effectKey >> 28) & 3) == 0 && (effectIndex - 300) < 41) {
        effectSlot = (effectKey - 1) & 0xf;
    }
    return effectSlot;
}

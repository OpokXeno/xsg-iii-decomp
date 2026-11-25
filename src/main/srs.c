#include "common.h"
#include "main/sv_get_image_item_142.h"

typedef int s32;

extern char xglDebFileNameBuffer[];

char *strcpy(char *destination, const char *source);

void xglDebFileNameSet(const char *text) {
    strcpy(xglDebFileNameBuffer, text);
}

int xglDebFileRead(const char *name, void *fileBuffer);

int xglDebFileSize(const char *name);

int xglDebFileReadSize(void *fileBuffer, const char *name)
{
    if (xglDebFileRead(name, fileBuffer) >= 0) {
        return xglDebFileSize(name);
    }
    return -1;
}

unsigned int func_0030E8E4(const char *text);

/* Returns the file name stored at +0x14 of a resource, or 0 when the name is empty. */
char *xglDebFileBufferSize(char *resource) {
    char *name;

    name = resource + 0x14;
    return (func_0030E8E4(name) == 0) ? 0 : name;
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

s32 sefMakeEffectHandle(s32 kind, s32 index, s32 sub);

s32 strcmp(signed char *text, const signed char *other);

extern const signed char gSrsEffectFileExtension[];

/* Parses an effect file name (stem pe, ce, ee or ef, then the index, and a sub number when the name has one) into an effect key. */
s32 srsParseEffectFileName(char *fileName) {
    s32 fileNameLength;
    s32 hasDotExtension;
    s32 suffixMatches;
    s32 stemStart;
    s32 effectKind;
    s32 effectIndex;
    s32 effectSub;

    if (fileName == 0) {
        return 0;
    }
    fileNameLength = func_0030E8E4(fileName);
    if (fileNameLength == 0) {
        return 0;
    }
    hasDotExtension = fileName[fileNameLength - 4] == '.';
    if (hasDotExtension) {
        suffixMatches = strcmp(fileName + fileNameLength - 3, gSrsEffectFileExtension) == 0;
    } else {
        suffixMatches = fileName[fileNameLength - 4] != '_';
    }
    if (hasDotExtension) {
        stemStart = suffixMatches ? fileNameLength - 9 : fileNameLength - 13;
    } else {
        stemStart = suffixMatches ? fileNameLength - 5 : fileNameLength - 9;
    }
    if (fileName[stemStart] == 'p' && fileName[stemStart + 1] == 'e') {
        effectKind = 0;
    } else if (fileName[stemStart] == 'c' && fileName[stemStart + 1] == 'e') {
        effectKind = 1;
    } else if (fileName[stemStart] == 'e' && fileName[stemStart + 1] == 'e') {
        effectKind = 2;
    } else {
        stemStart -= 1;
        if (fileName[stemStart] == 'e' && fileName[stemStart + 1] == 'f') {
            effectKind = 3;
        } else {
            stemStart += 1;
            effectKind = 0;
        }
    }
    if (effectKind != 3) {
        effectIndex = (fileName[stemStart + 2] - '0') * 100 + (fileName[stemStart + 3] - '0') * 10 + (fileName[stemStart + 4] - '0');
        if (effectIndex < 0) {
            return 0;
        }
        effectSub = 0;
        if (!suffixMatches) {
            effectSub =
                (fileName[stemStart + 6] - '0') * 100 + (fileName[stemStart + 7] - '0') * 10 + (fileName[stemStart + 8] - '0');
        }
    } else {
        effectIndex = (fileName[stemStart + 2] - '0') * 1000 + (fileName[stemStart + 3] - '0') * 100 +
                (fileName[stemStart + 4] - '0') * 10 + (fileName[stemStart + 5] - '0');
        effectSub = 0;
    }
    return sefMakeEffectHandle(effectKind, effectIndex, effectSub);
}

typedef signed char s8;

typedef unsigned int u32;

const s8 gSrsEffectFileSuffixKindThree[8] = ".esp";

const s8 gSrsEffectFileSuffixKindBelowThree[8] = ".esd";

static s8 gSrsEffectNameBuffer[24];

s32 func_0030E3D0(s8 *stemBuffer, const s8 *format);

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

const s8 gSrsAssetFormatCeRepeatTag[16] = "ef\\esd\\%c%c\\%s";

const s8 gSrsAssetFormatCdChars[24] = "ef\\esd\\%c%c%c00\\%s";

const s8 gSrsAssetFormatEfPosition[16] = "ef\\map\\%c%c\\%s";

const s8 gSrsAssetFormatEfFallback[16] = "ef\\map\\%s";

const s8 gSrsAssetFormatNameOnly[16] = "ef\\esp\\%s";

static s8 gSrsAssetNameBuffer[64];

u32 func_00181C18();

s32 xglCdGetFileSize();

char *srsBuildAssetName(char *name, s32 isLoaded)
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

char *srsBuildAssetName(char *name, s32 isLoaded);

s32 srsClassifyEffectKey(s32);

void srsEffectPrepare(s32 effectNo) {
    s8 *effectName;
    s32 assetLoadResult;

    effectName = srsEffectNameGet(effectNo);
    if (effectName != 0) {
        assetLoadResult = srsClassifyEffectKey(effectNo);
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

/* Load state of a battle win record. */
typedef enum BattleWinLoadState {
    BATTLE_WIN_NOT_LOADED,
    BATTLE_WIN_LOADED
} BattleWinLoadState;

/* One 40-byte battle win record; two of them fill gBattleResetTableStorage. */
typedef struct BattleWinEntry {
    s32 fileLoadAddress;
    s32 effectKey;
    char name[0x14];
    s32 fileSize;
    s32 inUse;
    BattleWinLoadState loadedState;
} BattleWinEntry;

static BattleWinEntry gBattleResetTableStorage[2];

static s32 gBattleWinNextRecord;

void eBattleWinReset(void);

void srsSetEffectFlag(s8 *name, s32 value);

void srsSetEffectIndexFlag(s8 *name, s32 value);

void *memset(void *destination, s32 value, unsigned int size);

void eBattleResetState(void) {
    memset(gBattleResetTableStorage, 0, 0x50);
    gBattleResetTableStorage[0].fileLoadAddress = 0xA1C000;
    gBattleResetTableStorage[1].fileLoadAddress = 0xA4E000;
    gBattleWinNextRecord = 0;
    eBattleWinReset();
    srsSetEffectFlag(0, 1);
    srsSetEffectIndexFlag(0, 1);
}

s32 func_001B60B8(s32);

s32 xglCdControlThread();

void xglCdWaitControl(void) {
    while (func_001B60B8(3) != 0) {
        xglCdControlThread();
    }
}

void eBattleWinInit();

typedef struct CdFileRead {
    s32 unk_00;
    char *path;
    s32 fileLoadAddress;
    void (*callback)();
    s32 commandChannel;
    s32 unk_14;
    s32 unk_18;
    s32 commandStatus;
    s32 unmodeled_20[24];
} CdFileRead;

extern s32 gCdPrimaryCommandSent;

extern s32 gCdSecondaryCommandSent;

void svRegisterResourceFile(s32, s32 *);

/* What svRegisterResourceFile registers for a loaded file: the buffer, the name and the key. */
typedef struct LoadedFile {
    s32 fileBuffer;
    s32 name;
    s32 key;
} LoadedFile;

/* CD read completion: command channels 0 and 1 store the request's status word in their command flag; other channels mark their battle win record loaded and register it. */
void srsBattleWinReadComplete(CdFileRead *fileRequest) {
    LoadedFile loadedFile;
    s32 requestChannel;

    requestChannel = fileRequest->commandChannel;
    switch (requestChannel) {
    case 0:
        gCdPrimaryCommandSent = fileRequest->commandStatus;
        return;
    case 1:
        gCdSecondaryCommandSent = fileRequest->commandStatus;
        return;
    }
    eBattleWinInit(requestChannel);
    loadedFile.fileBuffer = gBattleResetTableStorage[fileRequest->commandChannel - 2].fileLoadAddress;
    loadedFile.name = (s32) gBattleResetTableStorage[fileRequest->commandChannel - 2].name;
    requestChannel = fileRequest->commandChannel;
    gBattleResetTableStorage[requestChannel - 2].loadedState = BATTLE_WIN_LOADED;
    loadedFile.key = gBattleResetTableStorage[requestChannel - 2].effectKey;
    svRegisterResourceFile(requestChannel, (s32 *) &loadedFile);
}

/* Classifies an effect key from its kind (bits 28-29) and index (bits 12-21): 0 for none, 1 and 2 for the two file kinds, 3 for kind 3. */
s32 srsClassifyEffectKey(s32 effectNo) {
    s32 effectKind;
    s32 effectIndex;

    effectKind = (effectNo >> 28) & 3;
    effectIndex = (effectNo >> 12) & 0x3FF;
    if (effectKind == 2) {
        return 0;
    }
    if (effectKind == 3) {
        return 3;
    }
    if (effectKind == 1) {
        if (effectIndex == 100) {
            return 1;
        }
        if ((unsigned int) (effectIndex - 320) < 7) {
            return 2;
        }
        if ((unsigned int) (effectIndex - 330) < 5) {
            return 2;
        }
    }
    if (effectKind == 0) {
        if ((unsigned int) (effectIndex - 300) < 100) {
            return 1;
        }
        if ((unsigned int) (effectIndex - 1) < 10) {
            return 2;
        }
        if ((unsigned int) (effectIndex - 101) < 4) {
            return 2;
        }
        if ((unsigned int) (effectIndex - 500) < 301) {
            return 2;
        }
    }
    return 0;
}

extern SvTypeList svResourceTypeLists[];

void srsFormatEffectStem(int effectNo, char *buffer, int flag);

int strncmp(const char *left, const char *right, unsigned int length);

/* Finds the entry of a mapper whose name starts with the stem of effectId; *effectTypeKind receives the record's type kind. */
SvTypeListEntry *svFindEffectEntryByStem(int mapperIndex, int effectId, int *effectTypeKind);

#define SV_MAPPER_COUNT 44        /* mapper slots in svResourceTypeLists */

#define SV_TYPE_ENTRY_COUNT 140   /* entries of one SvTypeList */

/* Returns the first mapper from index 2 whose record holds an entry for effectNo, 0 for the skipped effect kinds and -1 when none does. */
int svFindEffectMapperIndex(int effectNo) {
    int effectKind;
    unsigned int effectIndex;
    int mapperIndex;

    effectKind = (effectNo >> 28) & 3;
    effectIndex = (effectNo >> 12) & 0x3FF;
    if (effectKind == 0 && (effectIndex - 300) < 100) {
        return 0;
    }
    if (effectKind == 1 && effectIndex == 100) {
        return 0;
    }
    for (mapperIndex = 2; mapperIndex < SV_MAPPER_COUNT; mapperIndex++) {
        if (svResourceTypeLists[mapperIndex].used != 0 &&
            svFindEffectEntryByStem(mapperIndex, effectNo, 0) != 0) {
            return mapperIndex;
        }
    }
    return -1;
}

/* Finds the entry of a mapper whose name starts with the stem of effectId; *effectTypeKind receives the record's type kind. */
SvTypeListEntry *svFindEffectEntryByStem(int mapperIndex, int effectId, int *effectTypeKind) {
    SvTypeList *mapperTypeList;
    SvTypeListEntry *matchedEntry;
    char stem[0x100];
    int mapperEntryIndex;

    if (effectId == 0) {
        return 0;
    }
    mapperTypeList = &svResourceTypeLists[mapperIndex];
    srsFormatEffectStem(effectId, stem, 1);
    for (mapperEntryIndex = 0; mapperEntryIndex < SV_TYPE_ENTRY_COUNT; mapperEntryIndex++) {
        if (mapperTypeList->entries[mapperEntryIndex].used != 0 &&
            mapperTypeList->entries[mapperEntryIndex].resourceKind == 5 &&
            mapperTypeList->entries[mapperEntryIndex].name != (char *) -1) {
            if (strncmp(stem, mapperTypeList->entries[mapperEntryIndex].name, func_0030E8E4(stem)) ==
                0) {
                if (effectTypeKind != 0) {
                    *effectTypeKind = mapperTypeList->typeKind;
                }
                return &mapperTypeList->entries[mapperEntryIndex];
            }
        }
    }
    return 0;
}

s32 xglCdGetAlignedFileSize(char *path);

void xglCdFileRequestClear(CdFileRead *fileRequest);

s32 xglCdReadRecordPrepare(CdFileRead *fileRequest);

void srsBattleWinReadComplete();

/* Starts loading the named effect into the next free battle win record; 1 when it is loaded or queued, 0 otherwise. */
s32 srsStartEffectLoad(char *effectName, s32 waitForRead) {
    CdFileRead fileRequest;
    char *assetPath;
    s32 nextRecordNumber;
    s32 effectNo = srsParseEffectFileName(effectName);
    s32 firstRecordNumber = gBattleWinNextRecord;

    if (srsClassifyEffectKey(effectNo) != 0) {
        return 0;
    }
    if (gBattleResetTableStorage[0].effectKey == effectNo || gBattleResetTableStorage[1].effectKey == effectNo) {
        return 1;
    }
    assetPath = srsBuildAssetName(effectName, 1);
    do {
        BattleWinEntry *candidateRecord = &gBattleResetTableStorage[gBattleWinNextRecord];
        nextRecordNumber = (gBattleWinNextRecord + 1) & 1;
        if (candidateRecord->inUse == 0 || svResourceTypeLists[gBattleWinNextRecord + 2].entryCount == 0) {
            s32 alignedFileSize = xglCdGetAlignedFileSize(assetPath);
            xglCdFileRequestClear(&fileRequest);
            fileRequest.path = assetPath;
            fileRequest.fileLoadAddress = candidateRecord->fileLoadAddress;
            fileRequest.callback = srsBattleWinReadComplete;
            fileRequest.commandChannel = gBattleWinNextRecord + 2;
            fileRequest.unk_00 = 0;
            if (!xglCdReadRecordPrepare(&fileRequest)) {
                return 0;
            }
            strcpy(candidateRecord->name, effectName);
            candidateRecord->effectKey = effectNo;
            candidateRecord->fileSize = alignedFileSize;
            candidateRecord->inUse = 1;
            candidateRecord->loadedState = 0;
            if (waitForRead) {
                xglCdWaitControl();
            }
            gBattleWinNextRecord = (gBattleWinNextRecord + 1) & 1;
            return 1;
        }
        gBattleWinNextRecord = nextRecordNumber;
    } while (firstRecordNumber != nextRecordNumber);
    return 0;
}

s32 srsStartEffectLoad(char *name, s32 wait);

/* Starts loading the effect named by effectNo (srsEffectNameGet), waiting for the read when waitForRead is nonzero. */
void srsSetNamedEffectValue(s32 effectNo, s32 waitForRead) {
    s8 *effectName;

    effectName = srsEffectNameGet(effectNo);
    srsStartEffectLoad((char *) effectName, waitForRead);
}

s32 func_00210280(s32 effectNo);

/* Returns the loaded state of the battle win record of the effect that srsParseEffectFileName names. */
s32 srsGetEffectLoadStateByName(s8 *effectName) {
    return func_00210280(srsParseEffectFileName(effectName));
}

/* Returns the loaded state of the battle win record of one effect, 0 when neither record holds it. */
s32 func_00210280(s32 effectNo) {
    if (effectNo > 0) {
        if (gBattleResetTableStorage[0].effectKey == effectNo) {
            return gBattleResetTableStorage[0].loadedState;
        }
        if (gBattleResetTableStorage[1].effectKey == effectNo) {
            return gBattleResetTableStorage[1].loadedState;
        }
    }
    return 0;
}

/* Clears one 40-byte battle win record of gBattleResetTableStorage. */
void srsBattleWinRecordClear(s32 recordNumber) {
    gBattleResetTableStorage[recordNumber].effectKey = 0;
    gBattleResetTableStorage[recordNumber].name[0] = 0;
    gBattleResetTableStorage[recordNumber].fileSize = 0;
    gBattleResetTableStorage[recordNumber].inUse = 0;
}

s32 sefReleaseOwnerEffects();

/* Releases or counts the battle win records of the two svResourceTypeLists slots 2 and 3 for effectKey (-1 means all); a shouldRelease of 1 releases them. */
s32 srsReleaseBattleWinEffects(s32 effectKey, s32 shouldRelease) {
    s32 recordNumber;
    if (effectKey < 0) {
        s32 summedTypeEntries = 0;
        for (recordNumber = 0; recordNumber < 2; recordNumber++) {
            if (svResourceTypeLists[recordNumber + 2].used != 0) {
                if (svResourceTypeLists[recordNumber + 2].entryCount != 0) {
                    if (shouldRelease == 1) {
                        sefReleaseOwnerEffects(effectKey);
                        eBattleWinInit(recordNumber + 2);
                        srsBattleWinRecordClear(recordNumber);
                    } else {
                        summedTypeEntries += svResourceTypeLists[recordNumber + 2].entryCount;
                    }
                } else {
                    eBattleWinInit(recordNumber + 2);
                    srsBattleWinRecordClear(recordNumber);
                }
            }
        }
        return summedTypeEntries;
    } else {
        for (recordNumber = 0; recordNumber < 2; recordNumber++) {
            if (svResourceTypeLists[recordNumber + 2].pendingCount == effectKey) {
                if (svResourceTypeLists[recordNumber + 2].entryCount != 0) {
                    if (shouldRelease == 1) {
                        sefReleaseOwnerEffects(effectKey);
                        eBattleWinInit(recordNumber + 2);
                        srsBattleWinRecordClear(recordNumber);
                        break;
                    }
                    return svResourceTypeLists[recordNumber + 2].entryCount;
                }
                eBattleWinInit(recordNumber + 2);
                srsBattleWinRecordClear(recordNumber);
                break;
            }
        }
        return 0;
    }
}

s32 srsReleaseBattleWinEffects(s32, s32);

void srsSetEffectIndexFlag(s8 *name, s32 effectFlagValue) {
    s32 effectIndex;

    effectIndex = -1;
    if (name != 0) {
        effectIndex = srsParseEffectFileName(name);
    }
    srsReleaseBattleWinEffects(effectIndex, effectFlagValue);
}

/* Starts loading a resource into a free svResourceTypeLists slot from 4 to 43; 1 when it is loaded or queued, 0 otherwise. */
s32 srsStartResourceLoad(s32 resourceHandle, s32 argument) {
    s32 loadedFileFields[4];
    char *assetName;
    s32 effectKey;
    s32 typeListNumber;

    if (resourceHandle != 0) {
        assetName = xglDebFileBufferSize((char *) resourceHandle);
        if (assetName != 0) {
            effectKey = srsParseEffectFileName(assetName);
            for (typeListNumber = 0; typeListNumber < 40; typeListNumber++) {
                if (svResourceTypeLists[typeListNumber + 4].used != 0 && svResourceTypeLists[typeListNumber + 4].pendingCount == effectKey) {
                    if (svResourceTypeLists[typeListNumber + 4].used == resourceHandle) {
                        return 1;
                    }
                    eBattleWinInit(typeListNumber + 4);
                    break;
                }
            }
            for (typeListNumber = 0; typeListNumber < 40; typeListNumber++) {
                if (svResourceTypeLists[typeListNumber + 4].used == 0) {
                    loadedFileFields[0] = resourceHandle;
                    loadedFileFields[1] = (s32) assetName;
                    loadedFileFields[2] = effectKey;
                    loadedFileFields[3] = argument;
                    svRegisterResourceFile(typeListNumber + 4, loadedFileFields);
                    return 1;
                }
            }
        }
    }
    return 0;
}

s32 srsStartResourceLoad(s32, s32);

s32 srsSetEffectHandleValueResult(s32 effectHandleIndex, s32 unused_arg1, s32 effectHandleValue) {
    return srsStartResourceLoad(effectHandleIndex, effectHandleValue);
}

void srsSetEffectHandleValue(s32 effectHandleIndex, s32 unused_arg1, s32 effectHandleValue) {
    srsStartResourceLoad(effectHandleIndex, effectHandleValue);
}

/* Releases or counts the svResourceTypeLists slots 4 to 43 whose pending key matches effectKey (-1 means all). */
s32 srsReleaseEffectRecordsByKey(s32 effectKey, s32 shouldRelease) {
    s32 typeListNumber;
    if (effectKey < 0) {
        s32 summedTypeEntries = 0;
        for (typeListNumber = 0; typeListNumber < 40; typeListNumber++) {
            if (svResourceTypeLists[typeListNumber + 4].used != 0) {
                if (svResourceTypeLists[typeListNumber + 4].entryCount != 0) {
                    if (shouldRelease == 1) {
                        sefReleaseOwnerEffects(effectKey);
                        eBattleWinInit(typeListNumber + 4);
                    } else {
                        summedTypeEntries += svResourceTypeLists[typeListNumber + 4].entryCount;
                    }
                } else {
                    eBattleWinInit(typeListNumber + 4);
                }
            }
        }
        return summedTypeEntries;
    } else {
        s32 idIsKindThree = ((effectKey >> 28) & 3) == 3;
        s32 summedTypeEntries = 0;
        s32 recordMatches = 0;
        for (typeListNumber = 0; typeListNumber < 40; typeListNumber++) {
            if (((svResourceTypeLists[typeListNumber + 4].pendingCount >> 28) & 3) == 3 && !idIsKindThree) {
                if (svFindEffectEntryByStem(typeListNumber + 4, effectKey, 0)) {
                    recordMatches = 1;
                }
            } else {
                if (svResourceTypeLists[typeListNumber + 4].pendingCount == effectKey) {
                    recordMatches = 1;
                }
            }
            if (recordMatches) {
                if (svResourceTypeLists[typeListNumber + 4].entryCount != 0) {
                    if (shouldRelease == 1) {
                        sefReleaseOwnerEffects(effectKey);
                        eBattleWinInit(typeListNumber + 4);
                        if (!idIsKindThree) {
                            return summedTypeEntries;
                        }
                    } else {
                        if (!idIsKindThree) {
                            return svResourceTypeLists[typeListNumber + 4].entryCount;
                        }
                        summedTypeEntries += svResourceTypeLists[typeListNumber + 4].entryCount;
                    }
                } else {
                    eBattleWinInit(typeListNumber + 4);
                    if (!idIsKindThree) {
                        return summedTypeEntries;
                    }
                }
            }
            recordMatches = 0;
        }
        return summedTypeEntries;
    }
}

s32 srsReleaseEffectRecordsByKey(s32, s32);

void srsSetEffectFlag(s8 *name, s32 effectFlagValue) {
    s32 effectIndex;

    effectIndex = -1;
    if (name != 0) {
        effectIndex = srsParseEffectFileName(name);
    }
    srsReleaseEffectRecordsByKey(effectIndex, effectFlagValue);
}

/* Releases or counts the svResourceTypeLists slots 4 to 43 for resourceHandle (0 means all). */
s32 srsReleaseEffectRecordsByResource(s32 resourceHandle, s32 shouldRelease) {
    s32 typeListNumber;
    if (resourceHandle == 0) {
        s32 summedTypeEntries = 0;
        for (typeListNumber = 0; typeListNumber < 40; typeListNumber++) {
            if (svResourceTypeLists[typeListNumber + 4].used != 0) {
                if (svResourceTypeLists[typeListNumber + 4].entryCount != 0) {
                    if (shouldRelease == 1) {
                        sefReleaseOwnerEffects(svResourceTypeLists[typeListNumber + 4].pendingCount);
                        eBattleWinInit(typeListNumber + 4);
                    } else {
                        summedTypeEntries += svResourceTypeLists[typeListNumber + 4].entryCount;
                    }
                } else {
                    eBattleWinInit(typeListNumber + 4);
                }
            }
        }
        return summedTypeEntries;
    } else {
        for (typeListNumber = 0; typeListNumber < 40; typeListNumber++) {
            if (svResourceTypeLists[typeListNumber + 4].used == resourceHandle) {
                if (svResourceTypeLists[typeListNumber + 4].entryCount != 0) {
                    if (shouldRelease == 1) {
                        sefReleaseOwnerEffects(svResourceTypeLists[typeListNumber + 4].pendingCount);
                        eBattleWinInit(typeListNumber + 4);
                        break;
                    }
                    return svResourceTypeLists[typeListNumber + 4].entryCount;
                }
                eBattleWinInit(typeListNumber + 4);
                break;
            }
        }
        return 0;
    }
}

char *srsEffectKindPrefix[4] = { "pe", "ce", "ee", "ef" };

char xglDebFileNameBuffer[256] = { 0 };

const signed char gSrsEffectFileExtension[4] = "esp";

s32 gCdPrimaryCommandSent = 0;

s32 gCdSecondaryCommandSent = 0;

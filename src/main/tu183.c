#include "common.h"

typedef unsigned char u8;
typedef int s32;
typedef float f32;
typedef signed char s8;
typedef short s16;
typedef unsigned short u16;

/* Voice record at SaveLoadMenu +0x720 (alpha at +0xAC). */
typedef struct SeVoice {
    u8 unk00[0xAC];
    s16 alpha;
} SeVoice;

/* Save/load menu block at gSaveLoadMenuStorage (gSaveLoadMenu, 0x10E0 bytes) and the parts this file touches. */
typedef struct SaveLoadMenu {
    u8 unk0000[0x720];
    SeVoice voice;          /* 0x720 */
    u8 unk07CE[0xA20 - 0x7CE];
    u8 voiceStartBlock[0x10CE - 0xA20];  /* 0xA20: record passed to shardFieldInit */
    u16 voiceControlBits;        /* 0x10CE: bit 0x2 set, bit 0x4 cleared on start */
    u8 unk10D0[0xA];
    s8 startPending;        /* 0x10DA */
} SaveLoadMenu;

extern SaveLoadMenu *gSaveLoadMenuStorage;

/* Header returned by getStatusHeader(); flags at +2. */
typedef struct StatusHeader {
    u8 unk00[2];
    u16 headerFlags;
} StatusHeader;

s32 xglFRandSeed(f32);
void *memPoolAlloc(s32 size);
s32 bufferPointersRewind();
s32 completionBlockClear();
s32 pagedTablePoolInit();
s32 memset(void *dst, s32 value, s32 size);
s32 xglClockReadBcd();
s32 xglPadSetRepeat(s32, s32, s32, s32);
u8 battleRosterEntryIdCountGet(s32 index);
void *func_00226F18(s32 mode);
void *seVoiceStartDefault(s32 id);

s32 seEngineAllocate(void) {
    u8 clockBcd[0x10];
    SaveLoadMenu *saveLoadMenu;
    s32 soundTableRecords;
    s32 initSucceeded;

    xglClockReadBcd(&clockBcd[0]);
    xglFRandSeed(clockBcd[1]);
    soundTableRecords = battleRosterEntryIdCountGet(0);
    initSucceeded = soundTableRecords;
    if (initSucceeded != 0) {
        completionBlockClear();
        pagedTablePoolInit();
        xglPadSetRepeat(0, 0xF000, 0x12C, 3);
        saveLoadMenu = memPoolAlloc(0x10E0);
        gSaveLoadMenuStorage = saveLoadMenu;
        memset(saveLoadMenu, 0, 0x10E0);
        bufferPointersRewind();
        initSucceeded = 1;
    }
    return initSucceeded;
}

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_00226190);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_00226958);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_002269F8);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_00226F18);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_00227450);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_002277E0);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_00227A38);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_00227B60);

s32 GameMpeg2Play(s32, s32, s32);
s32 messageTextGet(s32);
s32 sheetSetIndex(s32);

void movieStartPlayback(s32 movieId, s32 audioTrack) {
    s32 videoStream;
    s32 audioStream;

    videoStream = messageTextGet(0xF3);
    audioStream = messageTextGet(audioTrack + 0x7B);
    GameMpeg2Play(movieId, videoStream, audioStream);
    sheetSetIndex(0);
    sheetSetIndex(movieId);
}

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_00227C40);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_00227E60);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_00228058);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_00228110);

s32 clearStatusBits(s32, s32);
s32 setStatusBits(s32, s32);
s8 func_00222BF8(s32);

void statusReadSync(s8 *statusOut, s32 statusIndex) {
    s8 statusByte;

    statusByte = func_00222BF8(statusIndex);
    *statusOut = statusByte;
    if (statusByte & 0xFF) {
        clearStatusBits(0x1F, 0);
        return;
    }
    setStatusBits(0x1F, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_002282A0);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_002282C8);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_00228E18);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_00229828);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022A490);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022A810);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022A888);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022A928);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022AE58);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022B3A0);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022B768);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022C130);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022C438);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022C878);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022C938);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022CBC0);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022D1C8);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022D540);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022D970);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022DAF8);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022DF20);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022E320);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022E5D8);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022E740);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022EF20);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022F498);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022F910);

s32 func_0021E390(s32, s32, void *, s32);

void seVoiceDrawAlpha(s32 rawAlpha) {
    extern s32 xglFontSetAlphaScale(s32);
    s32 fontAlpha;
    SeVoice *voice;

    fontAlpha = rawAlpha & 0xFF;
    voice = &gSaveLoadMenuStorage->voice;
    xglFontSetAlphaScale(((fontAlpha * (0x80 - voice->alpha)) >> 7) & 0xFF);
    func_0021E390(0x30, 0x30, voice, 0);
    xglFontSetAlphaScale(fontAlpha);
}

StatusHeader *getStatusHeader(void);
s32 func_002D3D38();
s32 releaseStateBuffers();
s32 soundFlagAIsSet();

void *seVoiceRestart(void) {
    func_002D3D38();
    gSaveLoadMenuStorage->voiceControlBits |= 2;
    if (soundFlagAIsSet() != 0) {
        gSaveLoadMenuStorage->voiceControlBits &= 0xFFFB;
        releaseStateBuffers();
        if (getStatusHeader()->headerFlags & 0x10) {
            gSaveLoadMenuStorage->startPending = 1;
            return seVoiceStartDefault(7);
        }
        return func_00226F18(0);
    }
    return gSaveLoadMenuStorage;
}

s32 frameStateUpdate();

void seFontApplyAlpha(s32 rawAlpha) {
    extern s32 fontQueueSpriteBlack();
    extern s32 xglFontSetAlphaScale(s32);
    xglFontSetAlphaScale(rawAlpha & 0xFF);
    frameStateUpdate();
    fontQueueSpriteBlack();
}

s32 seIsEnabled();
s32 func_002CA0F8();
s32 seSystemShutdown();
s32 seIsInitialized();

void seVoiceRefreshState(void)
{
  s32 voiceControlBitsRefreshed;
  func_002CA0F8();
  if (seIsEnabled() != 0)
  {
    voiceControlBitsRefreshed = gSaveLoadMenuStorage->voiceControlBits | 2;
  }
  else
  {
    voiceControlBitsRefreshed = gSaveLoadMenuStorage->voiceControlBits & 0xFFFD;
  }
  gSaveLoadMenuStorage->voiceControlBits = voiceControlBitsRefreshed;
  if (!(seIsInitialized() != 0))
  {
    return;
  }
  {
    gSaveLoadMenuStorage->voiceControlBits &= 0xFFFB;
    seSystemShutdown();
    if (getStatusHeader()->headerFlags & 0x10)
    {
      seVoiceStartDefault(7);
      gSaveLoadMenuStorage->startPending = 1;
      return;
    }
    func_00226F18(0);
  }
}

void func_002CA198();

void seFontApplyFullAlpha(s32 rawAlpha) {
    extern void fontQueueSpriteBlack();
    extern void xglFontSetAlphaScale(s32);
    s32 fontAlpha;

    fontAlpha = rawAlpha & 0xFF;
    xglFontSetAlphaScale(fontAlpha);
    if (fontAlpha != 0x80) {
        return;
    }
    func_002CA198();
    fontQueueSpriteBlack();
}

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_0022FE10);

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_00230218);

s32 seEngineAllocate(void);
extern s8 gSeStartInProgress;
s32 shardFieldInit(void *block);
s32 func_00226190(s32);
s32 func_00226958();
s32 func_00230348(s32);

s32 seStartEngine(s32 requested)
{
    extern s32 seEngineAllocate();
  s32 sequenceResult;
  s32 startOptions;
  s32 flagBit;
  s32 isStarted;
  gSeStartInProgress = 0;
  flagBit = 0x10;
  startOptions = requested;
  if (startOptions & flagBit)
  {
    startOptions = startOptions | 1;
    startOptions = startOptions & (~0x10);
  }
  if ((startOptions & 1) && (battleRosterEntryIdCountGet(1) == 0))
  {
    startOptions &= ~1;
  }
  isStarted = seEngineAllocate(startOptions);
  if (isStarted != 0)
  {
    shardFieldInit(gSaveLoadMenuStorage->voiceStartBlock);
    seVoiceStartDefault(0xB);
    func_00226190(startOptions);
    sequenceResult = func_00230348(startOptions);
    func_00226958();
    isStarted = sequenceResult;
  }
  return isStarted;
}

INCLUDE_ASM("asm/main/nonmatchings/tu183", func_00230348);

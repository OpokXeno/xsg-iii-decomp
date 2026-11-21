#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu387", func_002C9820);

INCLUDE_ASM("asm/main/nonmatchings/tu387", func_002C9938);

INCLUDE_ASM("asm/main/nonmatchings/tu387", func_002C9A00);

typedef signed char s8;
typedef int s32;
typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned short u16;

typedef struct SeChannel {
    s32 capacity;
    s32 channelHandle;
} SeChannel;

typedef struct SeChannelPool {
    u8 unk_00[0x10];
    SeChannel channels[11];
} SeChannelPool;

/* A sound soundObject (0x2B0 bytes) set up by seObjectSetupFromSettings. */
typedef struct SeSoundObject {
    u8 unk_000[0x2A0];
    s32 soundSetting31;         /* 0x2A0: setting id 0x31, stored by seObjectSetSetting31 when nonzero */
    u8 unk_2A4[0xC];
} SeSoundObject;

/* The sound state embeds the soundContext gSeSoundContext at offset 0x70 (seSystemInit). */
#define offsetof(type, field) ((u32) &((type *) 0)->field)
#define gSoundState (*(SeSoundState *) ((void *) gSeSoundContext - offsetof(SeSoundState, soundContext)))
/* The channel table gSeChannelPoolStorage (seInitChannels); each function declares it as an unsized extern. */
#define gSeChannelPool (*(SeChannelPool *) gSeChannelPoolStorage)
typedef struct SeSoundState {
    s32 unk_00;
    s32 unk_04;
    s32 unk_08;
    u8 unk_0C[0x64];
    u8 soundContext[0x458];
    s32 unk_4C8;
    s32 unk_4CC;
    s32 unk_4D0;
} SeSoundState;

/* A voice status record returned by seVoiceStatusGet; voiceStartFlags bit 0x20 and 0x40 select the voice start path. */
typedef struct SeVoiceState {
    s32 unk_00;
    u16 voiceStartFlags;
} SeVoiceState;

void seObjectSetSetting31(SeSoundObject *soundObject, s32 pendingSoundSetting31) {
    if (pendingSoundSetting31 != 0) {
        soundObject->soundSetting31 = pendingSoundSetting31;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu387", func_002C9C00);

INCLUDE_ASM("asm/main/nonmatchings/tu387", func_002C9D28);

void *memset(void *, s32, s32);

extern u8 D_00402858[];

void seVoiceFlagBitsClear(void) {
    memset(&D_00402858, 0, 0x200);
}

extern s32 gSeEnabled;

s32 seIsEnabled(void) {
    return gSeEnabled != 0;
}

s32 seReturnFalse(void) {
    return 0;
}

void seNoop(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu387", func_002C9DE0);

void seInitChannels(void);
s32 seVoiceStatusReset();
s32 seChannelBankClear();
s32 seSendBufferInit();
s32 seIopTransferBufferAlloc();
s32 jpegBuffersAlloc();
s32 sineWavesReset();
s32 xglPadSetRepeat();
s32 seGetChannelHandle(s32);

void seSystemInit(void)
{
    extern unsigned char gSeSoundContext[];
    extern s32 func_002CA268();
    SeSoundState *soundState;
    xglPadSetRepeat(0, 0xF00C, 0xA, 3);
    seVoiceStatusReset();
    jpegBuffersAlloc();
    seSendBufferInit();
    seIopTransferBufferAlloc();
    soundState = &gSoundState;
    seChannelBankClear(gSeSoundContext);
    sineWavesReset();
    soundState->unk_4C8 = 0;
    soundState->unk_00 = 0;
    soundState->unk_04 = 0;
    soundState->unk_08 = 0;
    do { soundState->unk_4D0 = 0; } while (0);
    soundState->unk_4CC = 0;
    seInitChannels();
    func_002CA268();
}

s32 func_00213700(s32);

void seInitChannels(void) {
    extern unsigned char gSeChannelPoolStorage[];
    s32 channelOneHandle;
    s32 channelTwoHandle;
    s32 channelThreeHandle;
    s32 channelFiveHandle;
    s32 channelEightHandle;
    s32 channelNineHandle;

    gSeChannelPool.channels[0].capacity = 0x10;
    gSeChannelPool.channels[0].channelHandle = seGetChannelHandle(0x1D);
    gSeChannelPool.channels[1].capacity = 0x40;
    channelOneHandle = seGetChannelHandle(0x1C);
    gSeChannelPool.channels[2].capacity = 0x20;
    gSeChannelPool.channels[1].channelHandle = channelOneHandle;
    channelTwoHandle = seGetChannelHandle(0x1B);
    gSeChannelPool.channels[3].capacity = 8;
    gSeChannelPool.channels[2].channelHandle = channelTwoHandle;
    channelThreeHandle = seGetChannelHandle(0x21);
    gSeChannelPool.channels[5].capacity = 8;
    gSeChannelPool.channels[4].capacity = 4;
    gSeChannelPool.channels[3].channelHandle = channelThreeHandle;
    gSeChannelPool.channels[4].channelHandle = 0;
    channelFiveHandle = seGetChannelHandle(0x22);
    gSeChannelPool.channels[6].capacity = 4;
    gSeChannelPool.channels[7].capacity = 0x20;
    gSeChannelPool.channels[5].channelHandle = channelFiveHandle;
    gSeChannelPool.channels[6].channelHandle = 0;
    gSeChannelPool.channels[7].channelHandle = seGetChannelHandle(0x1F);
    gSeChannelPool.channels[8].capacity = 0x80;
    channelEightHandle = seGetChannelHandle(0x1E);
    gSeChannelPool.channels[9].capacity = 2;
    gSeChannelPool.channels[8].channelHandle = channelEightHandle;
    channelNineHandle = func_00213700(0x23);
    gSeChannelPool.channels[9].channelHandle = channelNineHandle;
    gSeChannelPool.channels[10].capacity = 1;
    gSeChannelPool.channels[10].channelHandle = 0;
}

s32 seSendWorkAreaInit(s32 *);
s32 func_002C7E88(s32 *);

void seStateRefresh(void) {
    extern s32 gSeSoundContext;
    seSendWorkAreaInit(&gSeSoundContext);
    func_002C7E88(&gSeSoundContext);
}

INCLUDE_ASM("asm/main/nonmatchings/tu387", func_002CA0F8);

INCLUDE_ASM("asm/main/nonmatchings/tu387", func_002CA198);

s32 setPendingValue(s32);
s32 seIopTransferBufferFree();
s32 jpegBuffersFree();

void seSystemShutdown(void) {
    jpegBuffersFree();
    seIopTransferBufferFree();
    setPendingValue(0);
}

s32 func_00220B90(s32, s32);
s32 mcClearSetupStatus();
s32 func_00220D28();

void seChannelsResetWithAux(void) {
    extern unsigned char gSeChannelPoolStorage[];

    mcClearSetupStatus();
    func_00220B90(gSeChannelPool.channels[0].capacity, gSeChannelPool.channels[0].channelHandle);
    func_00220B90(gSeChannelPool.channels[1].capacity, gSeChannelPool.channels[1].channelHandle);
    func_00220B90(gSeChannelPool.channels[2].capacity, gSeChannelPool.channels[2].channelHandle);
    func_00220B90(gSeChannelPool.channels[9].capacity, gSeChannelPool.channels[9].channelHandle);
    func_00220B90(gSeChannelPool.channels[10].capacity, gSeChannelPool.channels[10].channelHandle);
    func_00220D28();
}

INCLUDE_ASM("asm/main/nonmatchings/tu387", func_002CA268);

INCLUDE_ASM("asm/main/nonmatchings/tu387", func_002CA2C8);

s32 func_002DBF28();

void seDriverPoll(void) {
    func_002DBF28();
}

void seChannelsResetBase(void) {
    extern unsigned char gSeChannelPoolStorage[];

    mcClearSetupStatus();
    func_00220B90(gSeChannelPool.channels[0].capacity, gSeChannelPool.channels[0].channelHandle);
    func_00220B90(gSeChannelPool.channels[1].capacity, gSeChannelPool.channels[1].channelHandle);
    func_00220B90(gSeChannelPool.channels[2].capacity, gSeChannelPool.channels[2].channelHandle);
    func_00220D28();
}

INCLUDE_ASM("asm/main/nonmatchings/tu387", func_002CA3E0);

extern s32 gSeVoiceHandle;
SeVoiceState *seVoiceStatusGet();
void func_002CA5C0();
s32 seVoiceGetFlag();
void seVoiceEnvelopeUpdate();
void seVoiceStartDefault(unsigned int channelHandle);

void seUpdateVoiceStart(void)
{
    extern void func_002CA268();
    s32 voiceHandle;
    char voiceFlagMask;

    seVoiceEnvelopeUpdate();
    voiceHandle = seGetChannelHandle(0x10);
    voiceFlagMask = 1;
    gSeVoiceHandle = voiceHandle;
    if ((D_00402858[0]) & voiceFlagMask)
    {
        if ((seVoiceStatusGet()->voiceStartFlags) & 0x20)
        {
            seVoiceStartDefault(2);
            D_00402858[0] &= 0xFE;
        }
    }
    else if ((seVoiceStatusGet()->voiceStartFlags) & 0x20)
    {
        seVoiceStartDefault(2);
        if (seVoiceGetFlag() != 0)
        {
            func_002CA5C0();
        }
    }
    else if ((seVoiceStatusGet()->voiceStartFlags) & 0x40)
    {
        seVoiceStartDefault(8);
        func_002CA268();
    }
}

s32 seVoiceEnvelopeUpdateByMode();

void seDriverUpdateVoices(void) {
    seVoiceEnvelopeUpdateByMode();
}

void seChannelsResetFirstFive(void) {
    extern unsigned char gSeChannelPoolStorage[];

    mcClearSetupStatus();
    func_00220B90(gSeChannelPool.channels[0].capacity, gSeChannelPool.channels[0].channelHandle);
    func_00220B90(gSeChannelPool.channels[1].capacity, gSeChannelPool.channels[1].channelHandle);
    func_00220B90(gSeChannelPool.channels[2].capacity, gSeChannelPool.channels[2].channelHandle);
    func_00220B90(gSeChannelPool.channels[3].capacity, gSeChannelPool.channels[3].channelHandle);
    func_00220B90(gSeChannelPool.channels[4].capacity, gSeChannelPool.channels[4].channelHandle);
    func_00220D28();
}

INCLUDE_ASM("asm/main/nonmatchings/tu387", func_002CA5C0);

INCLUDE_ASM("asm/main/nonmatchings/tu387", func_002CA620);

s32 func_002E32D0();

void seDriverSyncBuffers(void) {
    func_002E32D0();
}

INCLUDE_ASM("asm/main/nonmatchings/tu387", func_002CA6C8);

INCLUDE_ASM("asm/main/nonmatchings/tu387", func_002CA728);

s32 panelDrawKeepFontColor();

void seDriverUpdateStreams(void) {
    panelDrawKeepFontColor();
}

s32 seIsInitialized(void) {
    extern s32 gSeChannelPoolStorage;
    return gSeChannelPoolStorage != 0;
}

extern s32 gSeStarted;

s32 seIsStarted(void) {
    return gSeStarted != 0;
}

extern s32 gSeSuspended;

s32 seIsSuspended(void) {
    return gSeSuspended != 0;
}

void seObjectSetSetting31(SeSoundObject *soundObject, s32 pendingSoundSetting31);
s32 func_002C9938(SeSoundObject *);
s32 func_002C9A00(SeSoundObject *, s32);
s32 func_002C9C00(SeSoundObject *);

void seObjectSetupFromSettings(void) {
    SeSoundObject soundObject;
    s32 firstSoundSetting;
    s32 secondSoundSetting;

    func_002C9938(&soundObject);
    firstSoundSetting = seGetChannelHandle(0x31);
    seObjectSetSetting31(&soundObject, firstSoundSetting);
    secondSoundSetting = seGetChannelHandle(0x32);
    func_002C9A00(&soundObject, secondSoundSetting);
    func_002C9C00(&soundObject);
}

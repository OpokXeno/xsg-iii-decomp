#include "common.h"
#include "shared.h"
#include "main/tu018.h"

/* >>> CHUNK se_bank_load */

int soundDriverReset(void);
int korgEvagAllUnitsSoundOff(void);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018E6F0);

void func_0018E6F0(int a0, int a1, int a2, unsigned int a3);

void seBgmLoadDefault(int a0, int a1, int a2)
{
    func_0018E6F0(a0, a1, a2, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018E870);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018E940);

/* One 16-byte sound slot. Driver calls take driver_id; the create call takes the voice's work
 * buffer and its size. */
typedef struct SoundSlot {
    int driver_id;
    struct {
        unsigned char *voiceWorkBuffer;    /* the voice's work buffer, carved from WorkEnd */
        int voiceWorkSize;
    } voiceWork;                     /* what the create call takes */
    VolumeRamp ramp;                  /* +0x0C */
} SoundSlot;
/* The sound control block: a 16-byte head with the song number, the levels and the master volume
 * ramp, then the two sound slots soundSlots. */
typedef union SoundControl {
    struct {
        unsigned char initialized;    /* +0x00: nonzero once the driver setup has run */
        unsigned char unmodeled_01[2];
        unsigned char song;           /* +0x03: number of the loaded song (seBgmLoad) */
        unsigned char master_volume;  /* +0x04: scales every level */
        unsigned char master_level;   /* +0x05: default master (music) level */
        unsigned char voice_level;    /* +0x06: default voice level */
        unsigned char slot_level;     /* +0x07: default sound slot level */
        unsigned char unmodeled_08[4];
        VolumeRamp master_ramp;       /* +0x0C */
        SoundSlot soundSlots[2];      /* +0x10 */
    };
    /* The head has a slot's layout too; soundSlotReopen indexes the slots as soundSlotRecords[index + 1]. */
    SoundSlot soundSlotRecords[3];
} SoundControl;
/* One sound channel's state word: state in the second byte, a counter in the first. */
typedef union SoundChannelState {
    int word;
    unsigned short half;   /* counter | state << 8 */
    struct {
        unsigned char counter;
        unsigned char state;
    } bytes;
} SoundChannelState;

typedef struct SoundRecordBlock {
    unsigned char master_volume; /* +0x00: scales the level of slot 0 */
    unsigned char volumeLevels[3];
    int current_song;            /* +0x04 */
    int driver_work;             /* +0x08 */
    VolumeRamp master_ramp;      /* +0x0C: the output volume ramp (main/tu019 reaches it as D_004C7044) */
    int reset_word;              /* +0x10 */
    int lookupTable[279];        /* +0x14 */
    unsigned char unmodeled_470[0x534 - 0x470];
    SoundChannelState table_b[63]; /* +0x534: channels 1..63, see soundApplySettings */
    SoundSlot soundSlotRecords[4]; /* +0x630 */
} SoundRecordBlock;
typedef signed char s8;
typedef int s32;
extern unsigned char gSeBankFileExt[];
extern SoundRecordBlock gSoundRecordBlock;
extern s32 D_004C7040[];
extern unsigned char gSeBankDirName[];
void func_001801C0();
int func_0018EC68(int channel);

void seBankLoadNumbered(s32 id, s32 number) {
    extern void func_0018E4C0();
    s8 name[0x100];

    func_0018EC68(id);
    if (number != 0) {
        func_0018E4C0(name, gSeBankDirName, number, gSeBankFileExt);
        func_001801C0(id, name, D_004C7040[0], 0x20000, 0, 0);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018EBE8);

void seVoiceStart(unsigned int voice, unsigned int volume, unsigned int pan);
extern const char gSeMasterBankPath[];
int xglCdReadFile(const char *name, void *buffer, int mode, void *notify);
void func_0018E940(int a0, void *bankAddress);
void func_00180DD8(unsigned int a0, int a1);

void seLoadMasterBank(void)
{
    xglCdReadFile(gSeMasterBankPath, (void *)0x01F00000, 0, 0);
    func_0018E940(0, (void *)0x01F00000);
    func_00180DD8(0xA4000000, 0x600);
}

int func_0017DAF0(int channel);

/* Stops one channel (after func_00180978 command 0xA0001100), or for a negative id every channel
 * from 4 (id -1) or 7 (any other negative id) up to 47. */
int func_0018EC68(int channel)
{
    int i;

    if (channel < 0) {
        i = (channel == -1) ? 4 : 7;
        for (; i < 48; i++) {
            func_0017DAF0(i);
        }
        return 0;
    }
    func_00180978(channel, 0xA0001100, 0);
    return func_0017DAF0(channel);
}

s32 func_001808E0();

void seVoiceStart(unsigned int voice, unsigned int volume, unsigned int pan) {
    extern s32 func_0018EDD8();
    if ((voice & 0xF0000000) == 0x10000000) {
        func_001808E0(voice & 0x0FFFFFFF, volume & 0xFF, pan & 0xFF);
        return;
    }
    func_0018EDD8(voice & 0xFF, volume, pan);
}

void seVoiceStartDefault(unsigned int voice)
{
    seVoiceStart(voice, 127, 0xffff);
}

s32 func_0018F1D0();

void soundDriverCheckStatus(s32 driverStatus) {
    if (driverStatus < 0) {
        func_0018F1D0();
    }
}

int soundDriverReset(void)
{
    return korgEvagAllUnitsSoundOff();
}

s32 func_0017E040();

void soundDriverSetVector(s32 vector) {
    func_0017E040(vector, vector + 4, vector + 8, vector + 0xC);
}

void soundDriverNoop(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018EDD8);

s32 seVoiceStartDriver(s32 voice) {
    extern s32 func_0018EDD8();
    return func_0018EDD8(voice, 0x7F, 0xFFFF);
}

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018F098);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018F118);

void soundDriverSetOutputMode(s32 outputSelect) {
    extern s32 func_00180978();
    if (outputSelect != 0) {
        func_00180978(0, 0xA0000500, 0);
        return;
    }
    func_00180978(outputSelect, 0xA0000600, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018F1D0);

/* <<< CHUNK se_bank_load */

/* >>> CHUNK sound_driver */

int seVolumeCurve(int level);


















extern SoundControl soundCtrl;
void soundSlotStartFile(int soundSlotNumber, const char *fileKey);
void ADXT_SetLpFlg(int id, int value);
void ADXT_StartFname(int id, const char *key);
int ADXT_SetOutVol(int id, int value);
void ADXM_ExecVint(int value);
SoundSlot *func_0018E490(int soundSlotNumber);
int func_00169688(int id, int value);
void func_0016A908(int id, const char *key);

/* Sets the loop flag of the driver voice of the sound slot index, then starts the file fileKey
 * on that voice. A null slot (the lookup failed) does nothing. */
void soundSlotStartFile(int soundSlotNumber, const char *fileKey)
{
    SoundSlot *soundSlot;

    soundSlot = func_0018E490(soundSlotNumber);
    if (soundSlot != 0) {
        func_00169688(soundSlot->driver_id, 1);
        func_0016A908(soundSlot->driver_id, fileKey);
    }
}

extern unsigned char gSeAdxFileExt[];
extern unsigned char gSeAdxDirName[];

/* Formats the file key from the number and starts it on the slot index. */
void soundSlotStartNumbered(s32 soundSlotNumber, s32 fileNumber) {
    extern s32 func_0018E4C0();
    s8 fileKey[0x100];

    func_0018E4C0(fileKey, gSeAdxDirName, fileNumber, gSeAdxFileExt);
    soundSlotStartFile(soundSlotNumber, fileKey);
}

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018F2D0);

s32 func_0016A968(s32, s32);

void soundSlotSetValue(s32 soundSlotNumber, s32 driverValue) {
    SoundSlot *soundSlot;

    soundSlot = func_0018E490(soundSlotNumber);
    if (soundSlot != 0) {
        func_00169688(soundSlot->driver_id, 1);
        func_0016A968(soundSlot->driver_id, driverValue);
    }
}

void func_00169938(int id, int value);

/* Pauses or resumes a sound slot's voice and records the level in its ramp. */
void soundSlotSetPause(int soundSlotNumber, int level)
{
    SoundSlot *soundSlot;

    soundSlot = func_0018E490(soundSlotNumber);
    if (soundSlot != 0) {
        func_00169938(soundSlot->driver_id, level);
        soundSlot->ramp.set_level = level;
    }
}

void soundSlotSetLevelIfStill(s32 soundSlotNumber, s32 level)
{
    SoundSlot *soundSlot;

    soundSlot = func_0018E490(soundSlotNumber);
    if (soundSlot == 0) {
        return;
    }
    if (soundSlot->ramp.set_level == 0) {
        func_00169938(soundSlot->driver_id, level);
    }
}

s32 func_00167BC8(s32);

/* Stops a sound slot: the driver stop call, then the voice is set to 0. */
void soundSlotStop(int soundSlotNumber) {
    SoundSlot *soundSlot;

    soundSlot = func_0018E490(soundSlotNumber);
    if (soundSlot == 0) {
        return;
    }
    func_00167BC8(soundSlot->driver_id);
    func_00169938(soundSlot->driver_id, 0);
}

/* The sound block at gSoundRecordBlock (splat label; the slot records start at +0x630, the table the
 * lookup in main/tu019 reads). */













int func_00167CB8(int id);
void soundSlotCreate(int soundSlotNumber);

/* Runs the two per-slot hooks for every sound slot that has a work buffer. */
void soundSlotsReset(void)
{
    int soundSlotNumber;

    for (soundSlotNumber = 0; soundSlotNumber < 4; soundSlotNumber++) {
        if (gSoundRecordBlock.soundSlotRecords[soundSlotNumber].voiceWork.voiceWorkBuffer != 0) {
            soundSlotStop(soundSlotNumber);
            soundSlotCreate(soundSlotNumber);
        }
    }
}

/* Applies the slot's current ramp level, through the volume curve, to its driver voice. */
int soundSlotApply(SoundSlot *soundSlot)
{
    int driver_id;
    int outVolume;

    driver_id = soundSlot->driver_id;
    outVolume = seVolumeCurve(soundSlot->ramp.currentLevel);
    return ADXT_SetOutVol(driver_id, outVolume);
}

int seVolumeRampSet(VolumeRamp *ramp, int level, int step);

/* The top of the sound level scale; it is also the divisor of the master volume. */
#define SOUND_LEVEL_MAX 127

/* The default level of a sound slot, a splat label of its own at gSoundSlotDefaultLevel. */
extern unsigned char gSoundSlotDefaultLevel[];

/* Sets slot index's level and ramp step; a negative extra only reports whether the ramp is moving. */
int soundSlotSetVolume(int soundSlotNumber, int level, int rampStep)
{
    SoundSlot *soundSlot;

    soundSlot = func_0018E490(soundSlotNumber);
    if (soundSlot == 0) {
        return 0;
    }
    if (rampStep < 0) {
        return soundSlot->ramp.levelDelta != 0;
    }
    if (level < 0) {
        level = gSoundSlotDefaultLevel[0];
    }
    if (soundSlotNumber == 0) {
        level = level * gSoundRecordBlock.master_volume / SOUND_LEVEL_MAX;
    }
    if (seVolumeRampSet(&soundSlot->ramp, level <= SOUND_LEVEL_MAX ? level : SOUND_LEVEL_MAX, rampStep) >= 0) {
        soundSlotApply(soundSlot);
    }
    return 1;
}

/* Returns the driver status of slot index's voice, or -1 when the slot is not open. */
int soundSlotStatus(int soundSlotNumber)
{
    SoundSlot *soundSlot;

    soundSlot = func_0018E490(soundSlotNumber);
    if (soundSlot == 0) {
        return -1;
    }
    return func_00167CB8(soundSlot->driver_id);
}

/* The play position of a voice as a sample count and the samples per second (shaped like CRI ADXT_GetTime). */
void func_00168038(int adxt, unsigned int *sampleCount, unsigned int *sampleRate);

#define SOUND_FRAMES_PER_SECOND 30.0f

/* The play position of slot soundSlotNumber's voice in frames, or -1 when the slot is not open. */
int func_0018F6E0(int soundSlotNumber)
{
    SoundSlot *soundSlot;
    unsigned int sampleCount;
    unsigned int sampleRate;

    soundSlot = func_0018E490(soundSlotNumber);
    if (soundSlot == 0) {
        return -1;
    }
    func_00168038(soundSlot->driver_id, &sampleCount, &sampleRate);
    return (float) sampleCount / (float) sampleRate * SOUND_FRAMES_PER_SECOND;
}

/* Calls the driver on slot index's voice with value 0; a slot that is not open returns 0. */
/* Clears the loop flag of the driver voice of the sound slot index (the reset half of soundSlotStartFile). */
void soundSlotClearLoopFlag(int soundSlotNumber)
{
    SoundSlot *soundSlot;

    soundSlot = func_0018E490(soundSlotNumber);
    if (soundSlot == 0) {
        return;
    }
    func_00169688(soundSlot->driver_id, 0);
}

/* A buffer size for count entries (at least 2): count * 0xD511 + 0xDD11 rounded down to
 * a 0x800 multiple, plus 0x8200. */
unsigned int func_0018F7E8(unsigned int count)
{
    if (count < 2) {
        count = 2;
    }
    return (count * 0xD511 + 0xDD11) / 0x800 * 0x800 + 0x8200;
}

typedef struct SoundWork {
    s32 voiceHandle;              /* +0x00: the voice handle, 0 when none */
    unsigned char *packetData;    /* +0x04: the packet data the voice plays */
    s32 packetSize;               /* +0x08 */
    unsigned char unk_0C[4];
} SoundWork;

extern SoundWork gSoundVoices[4];

void *soundWorkSource(s32 soundSlotNumber, void *packetData, s32 packetSize)
{
  void *currentPacketData;
  void *packetDataArg;
  s32 isValidIndex;
  s32 packetSizeArg;
  SoundWork *soundWork;
  packetDataArg = packetData;
  packetSizeArg = packetSize;
  isValidIndex = soundSlotNumber < 4;
  if (isValidIndex != 0)
  {
    soundWork = &gSoundVoices[soundSlotNumber];
    if ((packetDataArg != 0) && (packetSizeArg == 0))
    {
      currentPacketData = soundWork->packetData;
      *(void **) packetDataArg = currentPacketData;
      return currentPacketData;
    }
    soundWork->packetData = (unsigned char *) packetDataArg;
    soundWork->packetSize = packetSizeArg;
    soundSlotCreate(soundSlotNumber);
    return;
  }
  return (void *) isValidIndex;
}

void soundSlotSetDriverParam(s32 soundSlotNumber, s32 driverParam) {
    extern s32 func_00160C88(s32, s32);
    SoundSlot *soundSlot;

    soundSlot = func_0018E490(soundSlotNumber);
    if (soundSlot != 0) {
        func_00160C88(soundSlot->driver_id, driverParam);
    }
}

s32 func_00160CD0();
s32 func_00160CE8();

void soundDriverSetEnable(s32 unused_arg0, s32 driverEnabled) {
    if (driverEnabled != 0) {
        func_00160CD0();
        return;
    }
    func_00160CE8();
}

/* The same as soundSlotStartFile. */
void soundSlotStartKey(int soundSlotNumber, const char *fileKey)
{
    soundSlotStartFile(soundSlotNumber, fileKey);
}

s32 func_0016A3F8(s32);

void soundDriverSetStreamMode(s32 streamEnable) {
    extern s32 func_00180978(s32, s32, s32);
    if (streamEnable != 0) {
        func_0016A3F8(0);
        func_00180978(0, 0xA0000800, 0);
        return;
    }
    func_0016A3F8(1);
    func_00180978(0, 0xA0000900, 0);
}

/* SoundChannelState.state values, per soundApplySettings. */
#define SOUND_CHANNEL_STARTING 0 /* next pass: playing */
#define SOUND_CHANNEL_PLAYING 1  /* cleared when the counter is 0 or func_0017ECA8 reports it done */
#define SOUND_CHANNEL_STOPPING 2 /* func_0017E970 stops it, then playing with counter 0 */
#define SOUND_CHANNEL_COUNT 64
#define SOUND_CHANNEL_PLAYING_IDLE (SOUND_CHANNEL_PLAYING << 8)

int seVolumeRampStep(VolumeRamp *ramp);
void soundOutputVolumeWrite(s32 outputVolume);
void func_0017F210(void);
void func_00160200(void);
int func_0017ECA8(unsigned char channel);
void func_0017E970(unsigned char channel);

/* Per frame: steps the ramps of the open sound slots and the output volume ramp, runs the driver
 * services and advances every channel's state. */
void soundApplySettings(void)
{
    SoundSlot *soundSlot;
    SoundChannelState *channel;
    int i;
    int level;

    soundSlot = gSoundRecordBlock.soundSlotRecords;
    for (i = 0; i < 4; i++) {
        if (soundSlot->voiceWork.voiceWorkBuffer != 0) {
            if (seVolumeRampStep(&soundSlot->ramp) >= 0) {
                soundSlotApply(soundSlot);
            }
        }
        soundSlot++;
    }
    level = seVolumeRampStep(&gSoundRecordBlock.master_ramp);
    if (level >= 0) {
        soundOutputVolumeWrite(level * 0x102);
    }
    func_0017F210();
    func_00160200();
    channel = gSoundRecordBlock.table_b;
    for (i = 1; i < SOUND_CHANNEL_COUNT; i++) {
        if (channel->half != 0) {
            switch (channel->half >> 8) {
            case SOUND_CHANNEL_STARTING:
                channel->bytes.state++;
                break;
            case SOUND_CHANNEL_PLAYING:
                if (channel->half == SOUND_CHANNEL_PLAYING_IDLE) {
                    channel->word = 0;
                } else if (func_0017ECA8(i) == 0) {
                    channel->word = 0;
                }
                break;
            case SOUND_CHANNEL_STOPPING:
                func_0017E970(i);
                channel->word = SOUND_CHANNEL_PLAYING_IDLE;
                break;
            }
        }
        channel++;
    }
}

void soundSetFullVolume(void) {
    gSoundRecordBlock.master_volume = 0x7F;
    gSoundRecordBlock.volumeLevels[0] = 0x7F;
    gSoundRecordBlock.volumeLevels[1] = 0x7F;
    gSoundRecordBlock.volumeLevels[2] = 0x7F;
}

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018FAC8);

int soundVSyncCallback(void)
{
    ADXM_ExecVint(0);
    /* The end of an INTC/DMAC handler (SDK eekernel.h ExitHandler): sync, then re-enable interrupts. */
    __asm__ volatile("sync\n\tei" ::: "memory");
    return 0;
}

s32 func_00167100(s32 kind, unsigned char *packet, s32 packet_size);
s32 func_00167588();
s32 func_00168D48();

void soundSlotCreate(int soundSlotNumber) {
    extern s32 func_00160C88();
    SoundWork *soundWork;
    s32 voiceHandle;

    soundWork = &gSoundVoices[soundSlotNumber];
    if (soundWork->voiceHandle != 0) {
        func_00167588(soundWork->voiceHandle);
    }
    if (soundWork->packetData == 0) {
        soundWork->voiceHandle = 0;
        return;
    }
    voiceHandle = func_00167100(2, soundWork->packetData, soundWork->packetSize);
    soundWork->voiceHandle = voiceHandle;
    func_00168D48(voiceHandle, 0x1E);
    func_00160C88(soundWork->voiceHandle, 1);
    soundSlotSetVolume(soundSlotNumber, -1, 0);
}

/* Creates the voice of every sound slot. */
void soundSlotsCreateAll(void)
{
    int soundSlotNumber;

    for (soundSlotNumber = 0; soundSlotNumber < 4; soundSlotNumber++) {
        soundSlotCreate(soundSlotNumber);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018FD58);

/* <<< CHUNK sound_driver */

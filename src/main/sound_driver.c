#include "common.h"
#include "main/sce_libgraph.h"
#include "main/ee_stdarg.h"
#include "shared.h"
#include "main/tu018.h"

/* >>> CHUNK se_bank_load */

int soundDriverReset(void);
int korgEvagAllUnitsSoundOff(void);

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
/* Sound channels 1..63 (channel 0 is not used). */
#define SOUND_CHANNEL_COUNT 64
/* One sound channel's state word: state in the second byte, a counter in the first. */
typedef union SoundChannelState {
    int word;
    unsigned short half;   /* counter | state << 8 */
    struct {
        unsigned char counter;
        unsigned char state;
    } bytes;
} SoundChannelState;

/* Channels in the assignment table of the record block (soundApplySettings counts its channel states with
 * SOUND_CHANNEL_COUNT). */
#define SOUND_ASSIGN_COUNT 0x118
/* The songs with a swapped channel bank keep three banks of SOUND_BANK_CHANNELS channels from
 * SOUND_LIVE_BANK in the save area from SOUND_SAVED_BANK, which runs past the channel table. */
#define SOUND_BANK_CHANNELS 24
#define SOUND_LIVE_BANK 0x60
#define SOUND_SAVED_BANK 0x100
#define SOUND_CHANNEL_SLOTS (SOUND_ASSIGN_COUNT + 2 * SOUND_BANK_CHANNELS)
typedef struct SoundRecordBlock {
    unsigned char master_volume; /* +0x00: scales the level of slot 0 */
    unsigned char volumeLevels[3];
    unsigned char current_song;  /* +0x04: selects the channel range set (soundSongRangeSet) */
    unsigned char unmodeled_05[3];
    unsigned char *driver_work;  /* +0x08: the driver work area D_0089BF00 */
    VolumeRamp master_ramp;      /* +0x0C: the output volume ramp (main/tu019 reaches it as seMasterVolumeRamp) */
    unsigned int channelAssign[SOUND_CHANNEL_SLOTS]; /* +0x10: id << 24 | kind << 16 | group << 8 | index */
    SoundChannelState channelStates[SOUND_CHANNEL_COUNT]; /* +0x530: indexed by channel (1..63; 0 unused) */
    SoundSlot soundSlotRecords[4]; /* +0x630 */
} SoundRecordBlock;
typedef signed char s8;
typedef int s32;
extern unsigned char gSeBankFileExt[];
extern SoundRecordBlock gSoundRecordBlock;
extern s32 D_004C7040[];
extern unsigned char gSeBankDirName[];

/* Ids below SOUND_RANGE_IDS take their channel range from the range set of the current song. */
#define SOUND_RANGE_IDS 0x100
#define SOUND_RANGE_SET_COUNT 3
#define SOUND_RANGE_BOUNDARIES 32 /* boundaries per range set */
/* The range set (row of the range table) of each song number. */
extern unsigned char soundSongRangeSet[];

/* Assigns the channels of `id` to (id, kind, group): ids below SOUND_RANGE_IDS use the range between
 * their boundary and the next one in the current song's range set, other ids the one channel
 * id & 0xFF. A nonzero `range` (first << 16 | count) gives the range relative to that start. */
void func_0018E6F0(int id, int kind, int group, unsigned int range)
{
    unsigned short boundaries[SOUND_RANGE_SET_COUNT][SOUND_RANGE_BOUNDARIES] = {
        { 0x001, 0x013, 0x020, 0x02B, 0x100, 0x118, 0x130, 0x040, 0x042, 0x046, 0x04A, 0x04E,
          0x052, 0x056, 0x05A, 0x05E, 0x060, 0x080, 0x0A0, 0x0AF, 0x0B9, 0x0EC, 0x0FF },
        { 0x001, 0x013, 0x020, 0x02B, 0x060, 0x078, 0x090, 0x040, 0x0A8, 0x0C0, 0x0D8, 0x0F0,
          0x0FF },
        { 0x001, 0x013, 0x020, 0x02B, 0x040, 0x04C, 0x060, 0x066, 0x070, 0x040, 0x0C0, 0x0D0,
          0x0E0, 0x0F0, 0x0FF },
    };
    int first;
    int last;
    int channel;

    if (id < SOUND_RANGE_IDS) {
        first = boundaries[soundSongRangeSet[gSoundRecordBlock.current_song]][id];
        last = boundaries[soundSongRangeSet[gSoundRecordBlock.current_song]][id + 1];
    } else {
        last = id & 0xFF;
        first = last;
    }
    if (range != 0) {
        first += range >> 16;
        last = first + (range & 0xFFFF);
    }
    for (channel = first; channel < last; channel++) {
        gSoundRecordBlock.channelAssign[channel] = (id << 24) + (kind << 16) + (group << 8) + (channel - first);
    }
}

void seBgmLoadDefault(int id, int kind, int group)
{
    func_0018E6F0(id, kind, group, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018E870);

/* Bank slots that share their channels, as byte pairs (first and last slot of a group); the list ends with a
 * first slot of 0. */
extern unsigned char soundBankGroups[];
#define SOUND_BANK_GROUP_BYTES 2

/* The first word of a single bank file ("ps2_"); a bank list starts with the byte size of its offset table. */
#define SOUND_BANK_SINGLE_MAGIC 0x5F327370
/* Banks of a list start at 64-byte boundaries after the offset table. */
#define SOUND_BANK_ALIGN 64

void func_0018E870(int slot, void *bank, int numbered);

/* Registers the bank file at bankAddress in slot id & 0xFF (or only stops the slot when there is none). A single bank
 * goes to that slot; a bank list fills consecutive slots, one per nonempty entry. Without a current song, a slot of a
 * bank group first stops every slot of its group (once per group). */
void func_0018E940(int id, void *bankAddress)
{
    unsigned int *header;
    unsigned int *offset;
    unsigned int tableBytes;
    unsigned char *banks;
    unsigned char *current;
    unsigned char *next;
    int slot;
    int groupFirst;
    int groupLast;
    int i;
    int first;
    int last;

    slot = id & 0xFF;
    if (bankAddress == 0) {
        func_0018EC68(slot);
        return;
    }
    header = bankAddress;
    if (header[0] == SOUND_BANK_SINGLE_MAGIC) {
        func_0018EC68(slot);
        func_0018E870(slot, bankAddress, id <= SOUND_RANGE_IDS);
        return;
    }
    tableBytes = header[0];
    offset = header + 1;
    banks = (unsigned char *) bankAddress + ((tableBytes + SOUND_BANK_ALIGN - 1) & -SOUND_BANK_ALIGN);
    current = banks;
    groupLast = -1;
    groupFirst = -1;
    for (; tableBytes != 0; tableBytes -= sizeof(*offset)) {
        next = banks + ((*offset++ + SOUND_BANK_ALIGN - 1) & -SOUND_BANK_ALIGN);
        if (current < next) {
            if (gSoundRecordBlock.current_song == 0) {
                if (slot < groupFirst || groupLast < slot) {
                    for (i = 0; soundBankGroups[i] != 0; i += SOUND_BANK_GROUP_BYTES) {
                        first = soundBankGroups[i];
                        if (slot >= first) {
                            last = soundBankGroups[i + 1];
                            if (last >= slot) {
                                groupFirst = first;
                                groupLast = last;
                                break;
                            }
                        }
                    }
                    for (i = groupFirst; i <= groupLast; i++) {
                        func_0018EC68(i);
                    }
                }
            } else {
                func_0018EC68(slot);
            }
            func_0018E870(slot, current, 0);
        }
        current = next;
        slot++;
    }
}

void func_001801C0();
int func_0018EC68(int channel);
void func_0018E4C0(char *path, const char *dirName, const char *fileName, const char *ext);

void seBankLoadNumbered(s32 id, const char *number) {
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
void func_0018E940(int id, void *bankAddress);
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
void soundSlotStartNumbered(s32 soundSlotNumber, const char *fileNumber) {
    s8 fileKey[0x100];

    func_0018E4C0(fileKey, gSeAdxDirName, fileNumber, gSeAdxFileExt);
    soundSlotStartFile(soundSlotNumber, fileKey);
}

/* Every variadic argument takes one 8-byte slot (main/ee_stdarg.h). */
#define VA_SLOT_SIZE 8

/* The pool the playlist's file keys are written to, one after another (the driver keeps
 * the pointers). */
extern char D_004C76A8[];
/* The seamless playlist calls of the driver voice (shaped like CRI ADXT_ResetEntry,
 * ADXT_EntryFname, ADXT_SetSeamlessLp and ADXT_StartSeamless). */
void func_0015E8C0(int id);
void func_0015E240(int id, const char *key);
void func_0015E568(int id, int loop);
void func_0015E488(int id);

/* Plays the null-terminated list of file names on the sound slot as one seamless playlist:
 * each name's file key goes to the pool and into the list; a name starting with '<' ends
 * the list and loops it. A null slot (the lookup failed) does nothing. */
void func_0018F2D0(int soundSlotNumber, const char *fileName, ...)
{
    SoundSlot *soundSlot;
    char *key;
    va_list args;
    va_list slot;

    key = D_004C76A8;
    soundSlot = func_0018E490(soundSlotNumber);
    if (soundSlot != 0) {
        func_0015E8C0(soundSlot->driver_id);
        va_start(args, fileName);
        while (fileName != 0) {
            if (fileName[0] == '<') {
                func_0015E568(soundSlot->driver_id, 1);
                break;
            }
            func_0018E4C0(key, gSeAdxDirName, fileName, gSeAdxFileExt);
            func_0015E240(soundSlot->driver_id, key);
            while (*key++ != 0) {
            }
            /* the next argument's slot is kept before the step, as the original does */
            slot = args;
            args += VA_SLOT_SIZE;
            fileName = *(const char **) slot;
        }
        func_0015E488(soundSlot->driver_id);
    }
}

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
    channel = &gSoundRecordBlock.channelStates[1];
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

/* One key/value pair of a song's list; the list ends with key SOUND_PAIR_END. */
typedef struct SoundSongPair {
    int key;
    int value;
} SoundSongPair;
#define SOUND_PAIR_END -1
#define SOUND_SONG_PAIRS 48
/* The pair table func_0017D348 takes: the keys, then the values. */
typedef struct SoundSongPairTable {
    int key[SOUND_SONG_PAIRS];
    int value[SOUND_SONG_PAIRS];
} SoundSongPairTable;
/* The pair list of each song number. */
extern SoundSongPair *soundSongPairLists[];
void func_0017D348(SoundSongPairTable *table);
/* Song 3 stops the channels with SOUND_STOP_SONG3 (when it starts or ends), other changes with SOUND_STOP_OTHER. */
#define SOUND_SONG_3 3
#define SOUND_STOP_SONG3 -1
#define SOUND_STOP_OTHER -2
/* The songs with a swapped channel bank. */
#define SOUND_BANK_SONG_1 1
#define SOUND_BANK_SONG_2 2
#define SOUND_BANK_SONG_4 4

/* Switches to song `song`: stops the channels, passes the song's pair list on as a table and, for the
 * songs with a swapped bank (1, 2 and 4), saves the live bank of the old song and restores the
 * saved bank for the new one. */
void func_0018FAC8(int song)
{
    SoundSongPairTable table;
    SoundSongPair *pair;
    int i;

    if (gSoundRecordBlock.current_song == song) {
        return;
    }
    if (song == SOUND_SONG_3 || gSoundRecordBlock.current_song == SOUND_SONG_3) {
        func_0018EC68(SOUND_STOP_SONG3);
    } else {
        func_0018EC68(SOUND_STOP_OTHER);
    }
    pair = soundSongPairLists[song];
    memset(&table, 0, sizeof(table));
    for (i = 0; pair->key != SOUND_PAIR_END; i++) {
        table.key[i] = pair->key;
        table.value[i] = pair->value;
        pair++;
    }
    func_0017D348(&table);
    if (gSoundRecordBlock.current_song == SOUND_BANK_SONG_1 || gSoundRecordBlock.current_song == SOUND_BANK_SONG_2
        || gSoundRecordBlock.current_song == SOUND_BANK_SONG_4) {
        for (i = 0; i < SOUND_BANK_CHANNELS; i++) {
            gSoundRecordBlock.channelAssign[SOUND_SAVED_BANK + i] =
                gSoundRecordBlock.channelAssign[SOUND_LIVE_BANK + i];
            gSoundRecordBlock.channelAssign[SOUND_SAVED_BANK + SOUND_BANK_CHANNELS + i] =
                gSoundRecordBlock.channelAssign[SOUND_LIVE_BANK + SOUND_BANK_CHANNELS + i];
            gSoundRecordBlock.channelAssign[SOUND_SAVED_BANK + 2 * SOUND_BANK_CHANNELS + i] =
                gSoundRecordBlock.channelAssign[SOUND_LIVE_BANK + 2 * SOUND_BANK_CHANNELS + i];
        }
    }
    if (song == SOUND_BANK_SONG_1 || song == SOUND_BANK_SONG_2 || song == SOUND_BANK_SONG_4) {
        for (i = 0; i < SOUND_BANK_CHANNELS; i++) {
            gSoundRecordBlock.channelAssign[SOUND_LIVE_BANK + i] =
                gSoundRecordBlock.channelAssign[SOUND_SAVED_BANK + i];
            gSoundRecordBlock.channelAssign[SOUND_LIVE_BANK + SOUND_BANK_CHANNELS + i] =
                gSoundRecordBlock.channelAssign[SOUND_SAVED_BANK + SOUND_BANK_CHANNELS + i];
            gSoundRecordBlock.channelAssign[SOUND_LIVE_BANK + 2 * SOUND_BANK_CHANNELS + i] =
                gSoundRecordBlock.channelAssign[SOUND_SAVED_BANK + 2 * SOUND_BANK_CHANNELS + i];
        }
    }
    gSoundRecordBlock.current_song = song;
}

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

/* The parameter block of the IOP sound driver setup (func_0017CE40 passes the six words on as they
 * are); the setup gives only the first and the last word. */
typedef struct SoundIopSetup {
    int workSize;        /* +0x00 */
    int unmodeled_04[4];
    int workAddress;     /* +0x14 */
} SoundIopSetup;
#define SOUND_IOP_WORK_SIZE 0x35100
#define SOUND_IOP_WORK_ADDRESS 0x1D9000
/* No song loaded. */
#define SOUND_SONG_NONE 0xFF
/* The slot records of the record block. */
#define SOUND_SLOT_RECORDS 4
extern unsigned char D_0089BF00[];
void func_00166B30(int value);
void func_0015F8C8(int value);
void func_0015DD88(void);
void func_0017CE40(SoundIopSetup *setup);
void func_0017D020(void);
int func_0017ED00(unsigned char channel, int enable);
s32 func_00303328();

/* The sound driver setup: no song, the IOP driver with its work area, song 0 at full volume, every
 * channel assignment, channel state and slot record cleared, then the two output settings. */
void func_0018FD58(void)
{
    SoundIopSetup setup;
    int channel;
    int slot;

    gSoundRecordBlock.current_song = SOUND_SONG_NONE;
    func_00166B30(0);
    func_0015F8C8(0);
    sceGsSyncVCallback(soundVSyncCallback);
    func_0015DD88();
    func_00160CE8(0);
    memset(&setup, 0, sizeof(setup));
    setup.workSize = SOUND_IOP_WORK_SIZE;
    setup.unmodeled_04[0] = 0;
    setup.unmodeled_04[1] = 0;
    setup.unmodeled_04[2] = 0;
    setup.workAddress = SOUND_IOP_WORK_ADDRESS;
    func_0017CE40(&setup);
    func_0017D020();
    func_00180978(0, 0xA0320000, 0x7D);
    func_0018FAC8(0);
    soundSetFullVolume();
    gSoundRecordBlock.channelAssign[0] = 0;
    /* Channels below SOUND_RANGE_IDS have a voice the setup switches on. */
    for (channel = 1; channel < SOUND_RANGE_IDS; channel++) {
        gSoundRecordBlock.channelAssign[channel] = 0;
        func_0017ED00(channel, 1);
    }
    for (; channel < SOUND_ASSIGN_COUNT; channel++) {
        gSoundRecordBlock.channelAssign[channel] = 0;
    }
    for (channel = 1; channel < SOUND_CHANNEL_COUNT; channel++) {
        gSoundRecordBlock.channelStates[channel].word = 0;
    }
    for (slot = 0; slot < SOUND_SLOT_RECORDS; slot++) {
        gSoundRecordBlock.soundSlotRecords[slot].voiceWork.voiceWorkBuffer = 0;
        gSoundRecordBlock.soundSlotRecords[slot].voiceWork.voiceWorkSize = 0;
        gSoundRecordBlock.soundSlotRecords[slot].driver_id = 0;
    }
    gSoundRecordBlock.driver_work = D_0089BF00;
    func_00303328(1, 0x8010, 0x800, 0xFF0);
    func_00303328(1, 0x8010, 0x801, 0xFCC);
}

/* <<< CHUNK sound_driver */

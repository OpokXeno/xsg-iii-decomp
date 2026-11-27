#ifndef INCLUDE_MAIN_SOUND_DRIVER_H
#define INCLUDE_MAIN_SOUND_DRIVER_H

#include "main/tu018.h"

void seVoiceStartDefault(unsigned int voice);
/* Resets the voice volume of every channel `id` occupies. */
void func_0018F098(int id);
/* Sends driver command 0xA0000100 and clears the channel states. */
void func_0018F1D0(void);
/* Starts the sound id at volume and pan (-1 keeps a value); returns the voice handle. */
int func_0018EDD8(int id, int volume, int pan);
/* Per cue slot four bytes, the volume at gSoundCueVolumeTable[slot * 4] and the pan right after it. */
extern unsigned char gSoundCueVolumeTable[];
/* The slot records of the sound record block. */
#define SOUND_SLOT_RECORDS 4
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
/* Sound channels 1..63 (channel 0 is not used). */
#define SOUND_CHANNEL_COUNT 64
/* One sound channel's state word: state in the second byte, a counter in the first. */
typedef union SoundChannelState {
    int word;
    unsigned short half;   /* counter | state << 8 */
    struct {
        unsigned char counter;
        unsigned char state;
        unsigned char volume; /* the cue's start volume (func_0018EDD8) */
        unsigned char pan;    /* the cue's start pan */
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
    VolumeRamp master_ramp;      /* +0x0C: the output volume ramp */
    unsigned int channelAssign[SOUND_CHANNEL_SLOTS]; /* +0x10: id << 24 | kind << 16 | group << 8 | index */
    SoundChannelState channelStates[SOUND_CHANNEL_COUNT]; /* +0x530: indexed by channel (1..63; 0 unused) */
    SoundSlot soundSlotRecords[SOUND_SLOT_RECORDS]; /* +0x630 */
} SoundRecordBlock;
extern SoundRecordBlock gSoundRecordBlock;
/* The slot record of soundSlotNumber when it is in range and has a work buffer, else 0. */
SoundSlot *func_0018E490(int soundSlotNumber);

#endif /* INCLUDE_MAIN_SOUND_DRIVER_H */

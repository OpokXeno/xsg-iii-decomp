#ifndef INCLUDE_MAIN_SOUND_DRIVER_H
#define INCLUDE_MAIN_SOUND_DRIVER_H

void seVoiceStartDefault(unsigned int voice);
/* Resets the voice volume of every channel `id` occupies. */
void func_0018F098(int id);
/* Sends driver command 0xA0000100 and clears the channel states. */
void func_0018F1D0(void);
/* Starts the sound id at volume and pan (-1 keeps a value); returns the voice handle. */
int func_0018EDD8(int id, int volume, int pan);
/* Per cue slot four bytes, the volume at gSoundCueVolumeTable[slot * 4] and the pan right after it. */
extern unsigned char gSoundCueVolumeTable[];
/* The sound voice work records (the slot records of the sound record block seen from the voice side). */
#define SOUND_VOICE_COUNT 4
typedef struct SoundWork {
    int voiceHandle;              /* +0x00: the voice handle, 0 when none */
    unsigned char *packetData;    /* +0x04: the packet data the voice plays */
    int packetSize;               /* +0x08 */
    unsigned char unk_0C[4];
} SoundWork;

extern SoundWork gSoundVoices[SOUND_VOICE_COUNT];

#endif /* INCLUDE_MAIN_SOUND_DRIVER_H */

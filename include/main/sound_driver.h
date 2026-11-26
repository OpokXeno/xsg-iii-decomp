#ifndef INCLUDE_MAIN_SOUND_DRIVER_H
#define INCLUDE_MAIN_SOUND_DRIVER_H

void seVoiceStartDefault(unsigned int voice);
/* Resets the voice volume of every channel `id` occupies. */
void func_0018F098(int id);
/* Sends driver command 0xA0000100 and clears the channel states. */
void func_0018F1D0(void);
/* Per cue slot four bytes, the volume at gSoundCueVolumeTable[slot * 4] and the pan right after it. */
extern unsigned char gSoundCueVolumeTable[];

#endif /* INCLUDE_MAIN_SOUND_DRIVER_H */

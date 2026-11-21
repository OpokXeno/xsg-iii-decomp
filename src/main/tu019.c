#include "common.h"

typedef int s32;
extern s32 gSoundVoiceVolume[];

void soundVoiceVolumeReset(s32 voiceIndex) {
    gSoundVoiceVolume[voiceIndex] = 0x200;
}

INCLUDE_ASM("asm/main/nonmatchings/tu019", func_0018E490);

INCLUDE_ASM("asm/main/nonmatchings/tu019", func_0018E4C0);

s32 func_00303328();

void soundOutputVolumeWrite(s32 outputVolume) {
    func_00303328(1, 0x8010, 0xB80, outputVolume);
    func_00303328(1, 0x8010, 0xC80, outputVolume);
    func_00303328(1, 0x8010, 0xB81, outputVolume);
    func_00303328(1, 0x8010, 0xC81, outputVolume);
}

extern unsigned char gSoundVolumeCurve[];

s32 soundMasterVolumeSet(s32 requestedVolume) {
    s32 previousVolume;
    s32 volume;

    volume = requestedVolume;
    previousVolume = func_00303328(1, 0x8020, 0xB80);
    if (volume >= 0) {
        if (volume > 0x7FFF) {
            volume = gSoundVolumeCurve[volume] * 0x102;
        }
        soundOutputVolumeWrite(volume);
    }
    return previousVolume;
}

INCLUDE_ASM("asm/main/nonmatchings/tu019", func_0018E698);

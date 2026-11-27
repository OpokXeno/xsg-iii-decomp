#include "common.h"
#include "main/xgl_cd.h"

typedef int s32;
extern s32 gSoundVoiceVolume[];

void soundVoiceVolumeReset(s32 voiceIndex) {
    gSoundVoiceVolume[voiceIndex] = 0x200;
}

INCLUDE_ASM("asm/main/nonmatchings/tu019", func_0018E490);

int sprintf(char *destination, const char *format, ...);
/* "%08x.%08x": the location string of a sound file, its start sector and sector count in hex. */
extern const char soundCdLocationFormat[];

#define SOUND_PATH_SEPARATOR 0x5C /* backslash */
#define SOUND_SECTOR_SIZE 0x800u

/* Builds "snd", the directory, the file name (slashes become backslashes) and ext into path, looks the file up and
 * replaces the path with its location string (start sector and sector count). */
void func_0018E4C0(char *path, const char *dirName, const char *fileName, const char *ext)
{
    CdFilePosition position;
    char *cursor;
    const char *src;
    char character;

    path[0] = 's';
    path[1] = 'n';
    path[2] = 'd';
    path[3] = SOUND_PATH_SEPARATOR;
    cursor = path + 4;
    src = dirName;
    while (*src != 0) {
        *cursor++ = *src++;
    }
    *cursor++ = SOUND_PATH_SEPARATOR;
    src = fileName;
    while ((character = *src++) != 0) {
        if (character == '/') {
            character = SOUND_PATH_SEPARATOR;
        }
        *cursor++ = character;
    }
    src = ext;
    while ((*cursor++ = *src++) != 0) {
    }
    xglCdCheckFile(path, &position);
    sprintf(path, soundCdLocationFormat, position.lbn,
                  (position.stored_bytes + SOUND_SECTOR_SIZE - 1) / SOUND_SECTOR_SIZE);
}

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

/* The master volume ramp (its layout is in se_volume.c). */
typedef struct VolumeRamp VolumeRamp;
extern VolumeRamp seMasterVolumeRamp;
int seVolumeRampSet(VolumeRamp *ramp, int level, int levelPerStep);

/* Ramps the master volume towards level (a level above 0x7FFF goes through gSoundVolumeCurve) and writes it. */
s32 func_0018E698(s32 level, s32 levelPerStep)
{
    if (level >= 0) {
        if (level > 0x7FFF) {
            level = gSoundVolumeCurve[level];
        }
        level = seVolumeRampSet(&seMasterVolumeRamp, level, levelPerStep);
    }
    return soundMasterVolumeSet(level * 0x102);
}

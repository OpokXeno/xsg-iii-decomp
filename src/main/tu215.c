#include "common.h"

char *strcpy(char *destination, const char *source);
void sprintf(char *dst, const char *fmt, ...);

typedef int s32;

s32 xglCdGetFileSize(const char *path);
s32 soundSlotSetPause(s32 index, s32 value);
s32 soundSlotSetVolume(s32 index, s32 level, s32 extra);
void soundSlotStartNumbered(s32 soundSlotNumber, const char *fileNumber);
s32 soundSlotStatus(s32 index);
void xglSleep(void);

typedef struct XglCdBlockStream {
    unsigned char unk_0000[0x3F4C];
    s32 streamFlags; /* +0x3F4C: bit 2 set while an ADX sound plays from the disc */
} XglCdBlockStream;

extern XglCdBlockStream gXglCdBlockStreamStorage[];

#define gXglCdBlockStream (gXglCdBlockStreamStorage[0])

void xenoActorPlayAdxSound(const char *fileName);

/*
 * Plays the ADX sound named by fileName (the first dot and what follows it are dropped) when
 * snd/adx/<name>.adx exists on the disc. Blocks until sound slot 1 reports done.
 */
void xenoActorPlayAdxSound(const char *fileName) {
    char soundName[64];
    char adxPath[64];
    char *cursor;
    /* kept: the second pointer to adxPath is needed to match the call order */
    char *adxText = adxPath;

    strcpy(soundName, fileName);
    for (cursor = soundName; *cursor != '.'; cursor++) {
    }
    *cursor = '\0';
    sprintf(adxText, "snd/adx/%s.adx", soundName);
    if (xglCdGetFileSize(adxText) > 0) {
        gXglCdBlockStream.streamFlags |= 4;
        soundSlotSetPause(1, 1);
        soundSlotSetVolume(1, -1, 0);
        soundSlotStartNumbered(1, soundName);
        while (soundSlotStatus(1) != 3) {
            xglSleep();
        }
        soundSlotSetPause(1, 0);
    }
}

extern s32 D_003E8B10[];

void xenoActorEventTableInit(void) {
    xenoActorPlayAdxSound(D_003E8B10);
}

s32 soundSlotStop(s32);

void eventViewerSilenceSound(void) {
    soundSlotSetPause(1, 0);
    soundSlotSetVolume(1, 0, 0);
    soundSlotStop(1);
}

void xglCdRingReadNoNotify(void) {
}

#include "common.h"

typedef int s32;

typedef struct MovieRecord {
    unsigned char unk_00[8];
    s32 movieId;
} MovieRecord;

extern s32 gCurrentMovieId;

void movieClearCurrent(void) {
    gCurrentMovieId = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu285", func_0026E530);

void movieSetCurrentFromRecord(MovieRecord *movieRecord) {
    gCurrentMovieId = movieRecord->movieId;
}

typedef signed char s8;

typedef short s16;

typedef struct SwitchState {
    s16 enabled;
    s16 disabled;
    s32 activeSwitchIndex;
} SwitchState;

void setSwitchState(SwitchState *movieSwitch, s32 switchIndex) {
    if (switchIndex != 0) {
        movieSwitch->activeSwitchIndex = switchIndex;
        movieSwitch->enabled = 1;
        movieSwitch->disabled = 0;
        return;
    }
    movieSwitch->enabled = 0;
    movieSwitch->disabled = 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu285", func_0026E5C8);

INCLUDE_ASM("asm/main/nonmatchings/tu285", func_0026E648);

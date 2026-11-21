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

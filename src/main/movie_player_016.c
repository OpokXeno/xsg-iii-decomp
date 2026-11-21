#include "common.h"
#include "shared.h"

void moviePlayerInit(void);
void *memset(void *dest, int value, unsigned int size);

typedef struct MoviePlayerInitParams {
    float frameRate;
    int flag_a;
    int flag_b;
    int flag_c;
    unsigned char unmodeled_10[0x10];
} MoviePlayerInitParams;

int mwPlyInitSfdFx(MoviePlayerInitParams *sfdInitParams);
extern float movieFrameRate;

typedef int s32;
s32 func_00130158(s32 value);
s32 func_00132D90(s32 value);

s32 moviePlayerReleaseHandle(s32 *playerHandle) {
    func_00132D90(*playerHandle);
    func_00130158(*playerHandle);
    return 0;
}

typedef struct MoviePlayerParams {
    int file_type;
    int max_bit_rate;
    int max_width;
    int max_height;
    int frame_pool_count;
    int stream_count;
    int movieWorkAreaBase;
    int movieWorkAreaSize;
    int composition_mode;
    unsigned char unmodeled_24[0xC];
} MoviePlayerParams;

#define MOVIE_FILE_TYPE 1
#define MOVIE_MAX_BIT_RATE 7000000
#define MOVIE_FRAME_POOL_COUNT 3
#define MOVIE_STREAM_COUNT 1
#define MOVIE_COMPOSITION_MODE 17

int func_0012EC60(MoviePlayerParams *creationParams);
/* Two halfwords in the .sdata of this TU (no symbol name): the movie width and height. */
extern unsigned short moviePlayerWidth;
extern unsigned short moviePlayerHeight;
void func_002FB030(void);
void func_002F47D8(int value);
void func_00132140(void);
void func_00167790(void);
void func_0015DF18(void);

void moviePlayerParamsInit(MoviePlayerParams *creationParams)
{
    unsigned short width;
    unsigned short height;
    memset(creationParams, 0, sizeof(*creationParams));
    width = moviePlayerWidth;
    height = moviePlayerHeight;
    creationParams->file_type = MOVIE_FILE_TYPE;
    creationParams->composition_mode = MOVIE_COMPOSITION_MODE;
    creationParams->max_bit_rate = MOVIE_MAX_BIT_RATE;
    creationParams->max_width = width;
    creationParams->max_height = height;
    creationParams->frame_pool_count = MOVIE_FRAME_POOL_COUNT;
    creationParams->stream_count = MOVIE_STREAM_COUNT;
    creationParams->movieWorkAreaSize = func_0012EC60(creationParams);
}

void moviePlayerSetDimensions(u16 width, u16 height) {
    moviePlayerWidth = width;
    moviePlayerHeight = height;
}

INCLUDE_ASM("asm/main/nonmatchings/movie_player_016", func_0018BD50);

s32 func_0012FBB8();
s32 makeGstransChain(s32 destination, s32 image, void *frame);

/* Layout of the movie player object that moviePlayerSetupObject fills in (0x17 bytes used). */
typedef struct MoviePlayerObject {
    s32 playerHandle;                     /* +0x00, returned by func_0012FBB8 */
    s32 chainBase;                  /* +0x04 */
    unsigned char unk_08[0x4];
    s16 unk_0C;                     /* +0x0C, cleared */
    s16 unk_0E;                     /* +0x0E, cleared */
    s16 maxWidth;                   /* +0x10, from MoviePlayerParams.max_width */
    s16 maxHeight;                  /* +0x12, from MoviePlayerParams.max_height */
    unsigned char unk_14;           /* +0x14, cleared */
    unsigned char unk_15;           /* +0x15, cleared */
    unsigned char unk_16;           /* +0x16, set to 0x7F */
} MoviePlayerObject;

s32 moviePlayerSetupObject(MoviePlayerObject *player, s32 movieWorkAreaBase)
{
  MoviePlayerParams creationParams;
  s32 chainBase;
  s32 movieWorkAreaSize;

  player->unk_16 = 0x7F;
  player->unk_0E = 0;
  player->unk_0C = 0;
  player->unk_14 = 0;
  player->unk_15 = 0;
  moviePlayerParamsInit(&creationParams);
  movieWorkAreaSize = creationParams.movieWorkAreaSize;
  creationParams.movieWorkAreaBase = movieWorkAreaBase;
  chainBase = movieWorkAreaBase + ((movieWorkAreaSize + 0x3F) & (~0x3F));
  player->playerHandle = func_0012FBB8(&creationParams);
  player->maxWidth = creationParams.max_width;
  player->maxHeight = creationParams.max_height;
  player->chainBase = chainBase;
  return chainBase + (((makeGstransChain(chainBase, 0, player) * 0x10) + 0x3F) & (~0x3F));
}

void moviePlayerSetSize(void)
{
    func_002FB030();
    moviePlayerWidth = 512;
    moviePlayerHeight = 448;
}

void moviePlayerInit(void)
{
    MoviePlayerInitParams sfdInitParams;

    memset(&sfdInitParams, 0, sizeof(sfdInitParams));
    sfdInitParams.flag_c = 0;
    sfdInitParams.frameRate = movieFrameRate;
    sfdInitParams.flag_b = (sfdInitParams.flag_a = 1);
    mwPlyInitSfdFx(&sfdInitParams);
}

void moviePlayerTerminate(void)
{
    func_002F47D8(0);
    func_00132140();
    func_00167790();
    func_0015DF18();
}

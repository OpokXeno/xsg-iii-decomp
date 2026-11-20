#include "common.h"
#include "shared.h"

void moviePlayerInit(void);
void *memset(void *dest, int value, unsigned int size);

typedef struct MoviePlayerInitParams {
    float value;
    int flag_a;
    int flag_b;
    int flag_c;
    unsigned char unmodeled_10[0x10];
} MoviePlayerInitParams;

int mwPlyInitSfdFx(MoviePlayerInitParams *params);
extern float movieFrameRate;

INCLUDE_ASM("asm/main/nonmatchings/movie_player_016", func_0018BC98);

typedef struct MoviePlayerParams {
    int file_type;
    int max_bit_rate;
    int max_width;
    int max_height;
    int frame_pool_count;
    int stream_count;
    int base;
    int size;
    int composition_mode;
    unsigned char unmodeled_24[0xC];
} MoviePlayerParams;

#define MOVIE_FILE_TYPE 1
#define MOVIE_MAX_BIT_RATE 7000000
#define MOVIE_FRAME_POOL_COUNT 3
#define MOVIE_STREAM_COUNT 1
#define MOVIE_COMPOSITION_MODE 17

int func_0012EC60(MoviePlayerParams *layout);
/* Two halfwords in the .sdata of this TU (no symbol name): the movie width and height. */
extern unsigned short D_005511A0;
extern unsigned short D_005511A2;
void func_002FB030(void);
void func_002F47D8(int value);
void func_00132140(void);
void func_00167790(void);
void func_0015DF18(void);

void moviePlayerParamsInit(MoviePlayerParams *layout)
{
    unsigned short width;
    unsigned short height;
    memset(layout, 0, sizeof(*layout));
    width = D_005511A0;
    height = D_005511A2;
    layout->file_type = MOVIE_FILE_TYPE;
    layout->composition_mode = MOVIE_COMPOSITION_MODE;
    layout->max_bit_rate = MOVIE_MAX_BIT_RATE;
    layout->max_width = width;
    layout->max_height = height;
    layout->frame_pool_count = MOVIE_FRAME_POOL_COUNT;
    layout->stream_count = MOVIE_STREAM_COUNT;
    layout->size = func_0012EC60(layout);
}

INCLUDE_ASM("asm/main/nonmatchings/movie_player_016", func_0018BD40);

INCLUDE_ASM("asm/main/nonmatchings/movie_player_016", func_0018BD50);

INCLUDE_ASM("asm/main/nonmatchings/movie_player_016", func_0018BDC0);

void moviePlayerSetSize(void)
{
    func_002FB030();
    D_005511A0 = 512;
    D_005511A2 = 448;
}

void moviePlayerInit(void)
{
    MoviePlayerInitParams params;

    memset(&params, 0, sizeof(params));
    params.flag_c = 0;
    params.value = movieFrameRate;
    params.flag_b = (params.flag_a = 1);
    mwPlyInitSfdFx(&params);
}

void func_0018BEE8(void)
{
    func_002F47D8(0);
    func_00132140();
    func_00167790();
    func_0015DF18();
}

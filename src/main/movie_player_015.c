#include "common.h"
#include "shared.h"
#include "gs.h"

/* DMA chain tags (EE DMAC packet words) and the GIF tag bits of the image block. */
#define DMA_TAG_ID_RET 0x60000000U

#define DMA_TAG_ID_CNT 0x10000000U

#define DMA_TAG_ID_REF 0x30000000U

#define DMA_TAG_VIF_DIRECT(count) ((0x51000000ULL | (count)) << 32)

#define GIF_TAG_NREG_1 0x1000000000000000ULL

#define GIF_TAG_FLG_IMAGE 0x0800000000000000ULL

#define MOVIE_MACROBLOCK 16

#define MOVIE_MACROBLOCK_BYTES 0x400

#define MOVIE_MACROBLOCK_QWORDS 0x40

#define MOVIE_GIF_TAG_AD_2 (GIF_TAG_NREG_1 | GIF_TAG_EOP | 2)

#define MOVIE_GIF_TAG_IMAGE_BLOCK (GIF_TAG_FLG_IMAGE | GIF_TAG_EOP | MOVIE_MACROBLOCK_QWORDS)

/* BITBLTBUF destination: 896 pixels wide, placeholder until moviePlayerUpdate sets it. */
#define MOVIE_BITBLTBUF_DBW_896 (0xe000ULL << 36)

#define MOVIE_TRXREG_MACROBLOCK GS_SET_TRXREG(MOVIE_MACROBLOCK, MOVIE_MACROBLOCK)

/* A DMA REF tag: the tag word, the source address, then the VIF direct word. */
typedef struct DmaRefTag {
    unsigned int tag;
    unsigned int address;
    u64 upperQword;
} DmaRefTag;

/* The 16-bit frame record: an origin offset added to each macroblock position, and the extent. */
typedef struct MovieFrame {
    unsigned char unmodeled_00[12];
    short origin_x;
    short origin_y;
    short width;
    short height;
} MovieFrame;

/* Builds the DMA chain that uploads a frame's RGBA32 image to the GS, one 16x16 macroblock per
 * REF tag, and returns its length in quadwords. With a null `chainAddress` nothing is written
 * and only the length is computed. */
int makeGstransChain(unsigned int chainAddress, unsigned int image, const MovieFrame *frame)
{
    u64 *packet = (u64 *) ((chainAddress & 0x0fffffffU) | 0x20000000U);
    unsigned int macroblockSource = image;
    DmaRefTag *refTag;
    int x;
    int y;

    if (chainAddress != 0) {
        packet[0] = DMA_TAG_ID_CNT | 3;
        packet[1] = DMA_TAG_VIF_DIRECT(3);
        packet[2] = MOVIE_GIF_TAG_AD_2;
        packet[3] = GIF_AD;
        packet[4] = MOVIE_BITBLTBUF_DBW_896;
        packet[5] = GS_BITBLTBUF;
        packet[6] = MOVIE_TRXREG_MACROBLOCK;
        packet[7] = GS_TRXREG;
    }
    packet += 8;
    for (x = 0; x < frame->width; x += MOVIE_MACROBLOCK) {
        for (y = 0; y < frame->height; y += MOVIE_MACROBLOCK) {
            if (chainAddress != 0) {
                packet[0] = DMA_TAG_ID_CNT | 4;
                packet[1] = DMA_TAG_VIF_DIRECT(4);
                packet[2] = MOVIE_GIF_TAG_AD_2;
                packet[3] = GIF_AD;
                packet[4] = ((u64) (frame->origin_x + x) << 32) | ((u64) (frame->origin_y + y) << 48);
                packet[5] = GS_TRXPOS;
                packet[6] = 0;
                packet[7] = GS_TRXDIR;
                packet[8] = MOVIE_GIF_TAG_IMAGE_BLOCK;
                packet[9] = 0;
            }
            packet += 10;
            refTag = (DmaRefTag *) packet;
            if (chainAddress != 0) {
                refTag->address = macroblockSource;
                refTag->tag = DMA_TAG_ID_REF | MOVIE_MACROBLOCK_QWORDS;
                refTag->upperQword = DMA_TAG_VIF_DIRECT(MOVIE_MACROBLOCK_QWORDS);
            }
            packet += 2;
            macroblockSource += MOVIE_MACROBLOCK_BYTES;
        }
    }
    if (chainAddress != 0) {
        packet[1] = 0;
        packet[0] = DMA_TAG_ID_RET;
    }
    packet += 2;
    return (((unsigned int) packet - chainAddress) & 0x00ffffffU) >> 4;
}

/* The movie player record: the handle, the DMA packet address, the frame stamp, the size of the
 * current frame, the frame-held flag, and the extra-sound flag and volume. From +0x0C on it has
 * the layout of MovieFrame: the frame step passes the record itself to makeGstransChain. */
typedef struct MoviePlayerRecord {
    int playerHandle;
    unsigned int dma_address;
    int frame_stamp;
    unsigned char unmodeled_0c[4];
    short frame_width;
    short frame_height;
    unsigned char frame_held;
    unsigned char extra_enabled;
    unsigned char volume;
} MoviePlayerRecord;

/* The 0x30-byte file-position record; only its leading lbn and stored_bytes words are evidenced here. */
typedef struct CdFilePositionRecord {
    unsigned int lbn;
    unsigned int stored_bytes;
    unsigned char unmodeled_08[0x28];
} CdFilePositionRecord;

extern const char *gMoviePathPrefix;

extern const char gMovieKeyFormat[];

int seVolumeCurve(unsigned int volume);

void func_001344A0(int playerHandle, int value);

void func_001329D0(int playerHandle, const char *name);

int xglCdCheckFile(const char *path, void *record);

int GameMpeg2Play(char *destination, const char *format, unsigned int lbn, unsigned int sectors);

/* Starts the movie file `name` on the player record: sets the volume (unless the record has the
 * extra sound flag), builds the path from the prefix with / as \, looks it up and gives the
 * player the key "lbn.sectors" over the path from its second character. Returns -1 when the
 * file is missing, else 0. */
int moviePlayerStartFile(MoviePlayerRecord *player, const char *name)
{
    char path[256];
    CdFilePositionRecord file;
    const char *sourceCursor;
    char *pathCursor;
    char character;

    if (player->extra_enabled == 0) {
        func_001344A0(player->playerHandle, seVolumeCurve(player->volume));
    }
    pathCursor = path;
    sourceCursor = gMoviePathPrefix;
    while (*sourceCursor != 0) {
        *pathCursor = *sourceCursor;
        pathCursor++;
        sourceCursor++;
    }
    sourceCursor = name;
    do {
        character = *sourceCursor++;
        if (character == '/') {
            character = '\\';
        }
        *pathCursor++ = character;
    } while (character != 0);
    /* the third copy goes on from the byte after the name's terminator, as the original does */
    while (*sourceCursor != 0) {
        *pathCursor = *sourceCursor;
        pathCursor++;
        sourceCursor++;
    }
    if (xglCdCheckFile(path, &file) != 0) {
        return -1;
    }
    GameMpeg2Play(path + 1, gMovieKeyFormat, file.lbn, file.stored_bytes >> 11);
    func_001329D0(player->playerHandle, path + 1);
    player->frame_stamp = -1;
    return 0;
}

extern unsigned char gMovieFrameWidth[];

void func_00132E10(int playerHandle, int value);

void func_002F49F8(int packet, int dma_address, int flags);

int xglPacketGetCurrent(void);

/* The frame record func_00130B50 fills: a null image means no new frame. */
typedef struct MovieCurrentFrame {
    unsigned int image;
    unsigned char unmodeled_04[8];
    int width;
    int height;
    unsigned char unmodeled_14[0x10];
    int stamp;
    unsigned char unmodeled_28[0x68];
} MovieCurrentFrame;

/* Player statuses func_00133F08 returns that the frame step tells apart. */
#define MOVIE_STATUS_PLAY_END 3
#define MOVIE_STATUS_ERROR 4

void func_00131470(int playerHandle);
void func_00130B50(int playerHandle, MovieCurrentFrame *frame);
int func_00133F08(int playerHandle);

/* XS2 moviePlayerUpdate: releases the frame taken last time, takes the next one and, when there
 * is one, builds its upload chain (with the width from gMovieFrameWidth as BITBLTBUF destination)
 * and queues it. Returns 1 at the end of the movie, -1 on a playback error, 0 otherwise. */
int func_0018BB08(MoviePlayerRecord *player)
{
    MovieCurrentFrame frame;
    int packet;
    u64 *dma_words;

    packet = xglPacketGetCurrent();
    if (player->frame_held != 0) {
        func_00131470(player->playerHandle);
    }
    func_00130B50(player->playerHandle, &frame);
    player->frame_held = 0;
    if (frame.image != 0) {
        player->frame_stamp = frame.stamp;
        player->frame_width = frame.width;
        player->frame_height = frame.height;
        makeGstransChain(player->dma_address, frame.image, (const MovieFrame *) player);
        dma_words = (u64 *) ((player->dma_address & 0x0FFFFFFF) | 0x20000000);
        dma_words[4] = ((u64) *(u16 *)gMovieFrameWidth << 37) | (0x8000ULL << 36);
        func_002F49F8(packet, player->dma_address, 0);
        player->frame_held = 1;
    }
    switch (func_00133F08(player->playerHandle)) {
    case MOVIE_STATUS_PLAY_END:
        return 1;
    case MOVIE_STATUS_ERROR:
        return -1;
    default:
        return 0;
    }
}

/* Forwards `value` to func_00132E10 for the player's handle. When `value` is 0 and the player
 * has a frame, it stores the BITBLTBUF word (the width from gMovieFrameWidth in the DBW field) at
 * offset 0x20 of the player's DMA packet, then calls func_002F49F8 on the packet. */
int moviePlayerUpdate(MoviePlayerRecord *player, int value)
{
    int packet;
    unsigned int dma_address;
    u64 *dma_words;

    func_00132E10(player->playerHandle, value);
    if (value == 0 && player->frame_stamp > 0) {
        packet = xglPacketGetCurrent();
        dma_address = player->dma_address;
        dma_words = (u64 *) ((dma_address & 0x0FFFFFFF) | 0x20000000);
        dma_words[4] = ((u64) *(u16 *)gMovieFrameWidth << 37) | (0x8000ULL << 36);
        func_002F49F8(packet, dma_address, 0);
    }
    return 0;
}

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

/* The bytes moviePlayerSetupObject uses: the work area and the GS transfer chain, each rounded up to 64. */
s32 func_0018BD50(void)
{
    MoviePlayerObject player;
    MoviePlayerParams creationParams;
    s32 workAreaBytes;
    s32 alignMask;

    alignMask = ~0x3F;
    moviePlayerParamsInit(&creationParams);
    player.maxWidth = creationParams.max_width;
    player.maxHeight = creationParams.max_height;
    workAreaBytes = (creationParams.movieWorkAreaSize + 0x3F) & alignMask;
    return workAreaBytes + (((makeGstransChain(0, 0, (const MovieFrame *) &player) * 0x10) + 0x3F) & alignMask);
}

s32 func_0012FBB8();

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
  return chainBase + (((makeGstransChain(chainBase, 0, (const MovieFrame *) player) * 0x10) + 0x3F) & (~0x3F));
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

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

/* The movie player record: the handle, the DMA packet address, the frame stamp, and the
 * extra-sound flag and volume. */
typedef struct MoviePlayerRecord {
    int playerHandle;
    unsigned int dma_address;
    int frame_stamp;
    unsigned char unmodeled_0c[9];
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

INCLUDE_ASM("asm/main/nonmatchings/movie_player_015", func_0018BB08);

extern unsigned char gMovieFrameWidth[];
void func_00132E10(int playerHandle, int value);
void func_002F49F8(int packet, int dma_address, int flags);
int xglPacketGetCurrent(void);

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

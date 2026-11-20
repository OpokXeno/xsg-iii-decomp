#include "common.h"
#include "shared.h"
#include "gs.h"
/* DMA chain tags (EE DMAC packet words) and the GIF tag bits of the image block. */
#define DMA_TAG_ID_RET 0x60000000U
#define DMA_TAG_ID_CNT 0x10000000U
#define DMA_TAG_ID_REF 0x30000000U
#define DMA_TAG_VIF_DIRECT(count) ((u64) (0x51000000U | (count)) << 32)
#define GIF_TAG_NREG_1 0x1000000000000000ULL
#define GIF_TAG_FLG_IMAGE 0x0800000000000000ULL
#define MOVIE_MACROBLOCK 16
#define MOVIE_MACROBLOCK_BYTES 0x400
#define MOVIE_MACROBLOCK_QWORDS 0x40
#define MOVIE_GIF_TAG_AD_2 (GIF_TAG_NREG_1 | GIF_TAG_EOP | 2)
#define MOVIE_GIF_TAG_IMAGE_BLOCK (GIF_TAG_FLG_IMAGE | GIF_TAG_EOP | MOVIE_MACROBLOCK_QWORDS)
/* BITBLTBUF destination: 896 pixels wide, placeholder until moviePlayerUpdate sets it. */
#define MOVIE_BITBLTBUF_DBW_896 ((u64) 0xe000 << 36)
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
 * REF tag, and returns its length in quadwords. With a null `destination` nothing is written
 * and only the length is computed. */
int makeGstransChain(unsigned int destination, unsigned int image, const MovieFrame *frame)
{
    u64 *packet = (u64 *) ((destination & 0x0fffffffU) | 0x20000000U);
    unsigned int block = image;
    int x;
    int y;

    if (destination != 0) {
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
            if (destination != 0) {
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
            if (destination != 0) {
                ((DmaRefTag *) packet)->address = block;
                ((DmaRefTag *) packet)->tag = DMA_TAG_ID_REF | MOVIE_MACROBLOCK_QWORDS;
                ((DmaRefTag *) packet)->upperQword = DMA_TAG_VIF_DIRECT(MOVIE_MACROBLOCK_QWORDS);
            }
            packet += 2;
            block += MOVIE_MACROBLOCK_BYTES;
        }
    }
    if (destination != 0) {
        packet[1] = 0;
        packet[0] = DMA_TAG_ID_RET;
    }
    packet += 2;
    return (((unsigned int) packet - destination) & 0x00ffffffU) >> 4;
}

/* The movie player record: the handle, the frame stamp, and the extra-sound flag and volume. */
typedef struct MoviePlayerRecord {
    int handle;
    unsigned char unmodeled_04[4];
    int frame_stamp;
    unsigned char unmodeled_0c[9];
    unsigned char extra_enabled;
    unsigned char volume;
} MoviePlayerRecord;

/* The 0x30-byte file-position record; only its leading lbn and stored_bytes words are evidenced here. */
typedef struct CdFilePositionRecord {
    unsigned int lbn;
    int stored_bytes;
    unsigned char unmodeled_08[0x28];
} CdFilePositionRecord;

extern const char *D_0055119C;
extern const char D_00540380[];
int seVolumeCurve(unsigned int volume);
void func_001344A0(int handle, int value);
void func_001329D0(int handle, const char *name);
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
    const char *source;
    char *destination;
    char character;

    if (player->extra_enabled == 0) {
        func_001344A0(player->handle, seVolumeCurve(player->volume));
    }
    destination = path;
    source = D_0055119C;
    while (*source != 0) {
        *destination = *source;
        destination++;
        source++;
    }
    source = name;
    do {
        character = *source++;
        if (character == '/') {
            character = '\\';
        }
        *destination++ = character;
    } while (character != 0);
    /* the third copy goes on from the byte after the name's terminator, as the original does */
    while (*source != 0) {
        *destination = *source;
        destination++;
        source++;
    }
    if (xglCdCheckFile(path, &file) != 0) {
        return -1;
    }
    GameMpeg2Play(path + 1, D_00540380, file.lbn, (unsigned int) file.stored_bytes >> 11);
    func_001329D0(player->handle, path + 1);
    player->frame_stamp = -1;
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/movie_player_015", func_0018BB08);

INCLUDE_ASM("asm/main/nonmatchings/movie_player_015", func_0018BC00);

#include "common.h"
#include "shared.h"

void sefPacketFlush(void);
struct XglPacket;

#include "main/Packet.h"

#include "main/sv_get_image_item_142.h"
#include "gs.h"
#include "quadword.h"
#include "vu0_macro.h"
XglPacket *xglPacketGetCurrent(void);
void sceVif1PkCnt(XglPacket *packet, unsigned int count);
void sceVif1PkAddCode(XglPacket *packet, unsigned int tag);

/* VIF codes (command in bits 24..30): FLUSH, MSCNT, and MSCAL whose immediate is the micro program address / 8. */
#define VIF_CODE_FLUSH 0x11000000

typedef int s32;

/* One keyframe of an effect path: the start and end vectors and the key's time and tag. */
typedef struct SdvPacketKey {
    Vector4 start;       /* +0x00 */
    Vector4 end;         /* +0x10 */
    unsigned short time; /* +0x20 */
    unsigned short tag;  /* +0x22 */
    unsigned char unmodeled_24[0x0C];
} SdvPacketKey;

/* The key run's header sits in front of its first key: kind value and flags at these byte offsets. */
#define SDV_PACKET_KEY_KIND_VALUE (-8)
#define SDV_PACKET_KEY_FLAG_A (-6)
#define SDV_PACKET_KEY_FLAG_B (-4)
#define SDV_PACKET_KEY_FLAG_C (-2)
#define SDV_PACKET_KEY_FIELD(keys, byteOffset) (*(short *) ((unsigned char *) (keys) + (byteOffset)))

/* One image part of an effect row: alpha, frame and the two image/palette pairs. */
typedef struct SdvPacketPart {
    float alpha;         /* +0x00 */
    short frame;         /* +0x04 */
    unsigned char blend; /* +0x06 */
    unsigned char unmodeled_07;
    short image0;   /* +0x08 */
    short palette0; /* +0x0A */
    short image1;   /* +0x0C */
    short palette1; /* +0x0E */
} SdvPacketPart;

/* One image record of the 64-entry record table (0x18 bytes). */
typedef struct SdvPacketSlot {
    unsigned long long tex0;     /* +0x00 */
    unsigned long long clutTex0; /* +0x08 */
    float alpha;                 /* +0x10 */
    unsigned char kind;          /* +0x14 */
    unsigned char blend;         /* +0x15 */
    unsigned char width;         /* +0x16 */
    unsigned char height;        /* +0x17 */
} SdvPacketSlot;

/* The 64-bit flags word of a packet record: bits 0..2 come from the key header's flags A, B and C. */
typedef union SdvPacketKeyFlags {
    unsigned long long word;
    unsigned char lowByte;
} SdvPacketKeyFlags;

/* One packet of the 16-entry slot table (0x90 bytes): the matrix, the motion block and the slot ids. */
typedef struct SdvPacketRecord {
    unsigned char unmodeled_000[0x40]; /* +0x00: the matrix, Vu0CopyUnitMatrix */
    Vector4 position;                  /* +0x40 */
    Vector4 heading;                   /* +0x50 */
    float previousScale;               /* +0x60 */
    unsigned short time;               /* +0x64 */
    short keyIndex;                    /* +0x66 */
    unsigned short tag;                /* +0x68 */
    unsigned char unmodeled_06A[2];
    SdvPacketKey *keys;      /* +0x6C */
    signed char slotIds[16]; /* +0x70 */
    SdvPacketKeyFlags keyFlags; /* +0x80 */
    unsigned char unmodeled_088[0x90 - 0x88];
} SdvPacketRecord;

/* The packet table of the SDV effect player (0xF60 bytes): slot records, image records and their claim flags. */
typedef struct SdvPacketTable {
    SdvPacketRecord packets[16]; /* +0x000 */
    SdvPacketSlot imageRecords[64]; /* +0x900 */
    signed char packetFlags[16];   /* +0xF00: 1 while the slot is claimed */
    signed char imageRecordFlags[64]; /* +0xF10: 1 while the packet is claimed */
    unsigned char packetCursor;    /* +0xF50: the next slot to try */
    unsigned char imageRecordCursor;  /* +0xF51: the next packet to try */
    unsigned char unmodeled_F52[0x0E];
} SdvPacketTable;

extern SdvPacketTable gSdvPacketTable[];

void sefPacketFlush(void) {
    XglPacket *packet;

    packet = xglPacketGetCurrent();
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddCode(packet, VIF_CODE_FLUSH);
}

/* Claims the next free slot of the 16-entry slot table; returns its index, or -1 when all are claimed. */
int func_001F3700(void) {
    int cursor;
    int i;

    cursor = gSdvPacketTable->packetCursor;
    for (i = 0; i < 16; i++) {
        if (gSdvPacketTable->packetFlags[cursor] == 0) {
            gSdvPacketTable->packetFlags[cursor] = 1;
            gSdvPacketTable->packetCursor = (cursor + 1) % 16;
            return cursor;
        }
        cursor = (cursor + 1) % 16;
    }
    return -1;
}

void *sdvPacketHeadGet(int index);

void *sdvPacketClaim(s32 packetSlotIndex);
void *memset(void *destination, s32 value, unsigned int count);

/* Releases the slot at index: releases every record it lists, clears its motion block and ids, frees the slot. */
void sdvPacketRelease(int index) {
    SdvPacketRecord *packet;
    int i;

    packet = sdvPacketHeadGet(index);
    if (packet != 0) {
        for (i = 0; i < 16; i++) {
            sdvPacketClaim(packet->slotIds[i]);
        }
        memset(&packet->position, 0, 0x30);
        memset(packet->slotIds, -1, 16);
        gSdvPacketTable->packetFlags[index] = 0;
        gSdvPacketTable->packetCursor = index;
    }
}

/* Claims the next free record of the 64-entry record table; returns its index, or -1 when all are claimed. */
int func_001F3828(void) {
    int cursor;
    int i;

    cursor = gSdvPacketTable->imageRecordCursor;
    for (i = 0; i < 64; i++) {
        if (gSdvPacketTable->imageRecordFlags[cursor] == 0) {
            gSdvPacketTable->imageRecordFlags[cursor] = 1;
            gSdvPacketTable->imageRecordCursor = (cursor + 1) % 64;
            return cursor;
        }
        cursor = (cursor + 1) % 64;
    }
    return -1;
}

void *sdvPacketRecordGet(s32);

/* Claims record packetSlotIndex: clears its 0x18 bytes and marks it claimed. Returns the table address of the index
   (callers ignore it), or 0 when that packet is not in use. */
void *sdvPacketClaim(s32 packetSlotIndex) {
    unsigned char *packetSlot;

    packetSlot = sdvPacketRecordGet(packetSlotIndex);
    if (packetSlot != 0) {
        memset(packetSlot, 0, 0x18);
        packetSlot = (unsigned char *) gSdvPacketTable + packetSlotIndex;
        packetSlot[0xF10] = 0;
        ((unsigned char *) gSdvPacketTable)[0xF51] = packetSlotIndex;
    }
    return packetSlot;
}

/* Clears the whole packet table, then sets the slot ids of every slot record to -1 (no slot). */
void func_001F3900(void) {
    int i;

    memset(gSdvPacketTable, 0, sizeof(SdvPacketTable));
    for (i = 0; i < 16; i++) {
        memset(gSdvPacketTable->packets[i].slotIds, -1, 16);
    }
}

SvTypeListEntry *svGetImageListItem(int type, unsigned int flags);

/* Builds the GS TEX0 word of an image and palette pair into *dst; the image's size goes to outWidth and outHeight.
   Index bit 0x100 selects list 0 for the image. Sets *dst to 0 when either index is missing. */
void func_001F3968(u64 *dst, u8 *outWidth, u8 *outHeight, int listType, unsigned int imageIndex, unsigned int paletteIndex) {
    SvTypeListEntry *image;
    SvTypeListEntry *palette;
    int pixelFormat;

    if (imageIndex != 0 && paletteIndex != 0) {
        if (imageIndex & 0x100) {
            image = svGetImageListItem(0, imageIndex);
            palette = svGetImageListItem(0, paletteIndex);
        } else {
            image = svGetImageListItem(listType, imageIndex);
            palette = svGetImageListItem(listType, paletteIndex);
        }
    } else {
        *dst = 0;
        return;
    }
    pixelFormat = image->bitsPerPixel == 8 ? 19 : 20;
    *dst = GS_SET_TEX0(image->basePointer, image->bufferWidthPixels >> 6, pixelFormat, image->textureWidthExponent,
                       image->textureHeightExponent, 1, 0, palette->basePointer, 0, 0, 0, 1);
    if (outWidth != 0) {
        *outWidth = image->width;
    }
    if (outHeight != 0) {
        *outHeight = image->height;
    }
}

/* The owner of an effect: its time, its row table and the four record ids it has claimed. */
typedef struct SdvPacketOwner {
    float time;           /* +0x00 */
    short *rows;          /* +0x04: {frame, index} rows of four bytes */
    signed char position; /* +0x08 */
    unsigned char unmodeled_09[3];
    signed char slotIds[4]; /* +0x0C */
} SdvPacketOwner;

typedef struct SdvPacketEffect {
    unsigned char unmodeled_00[0x08];
    short entryId; /* +0x08 */
} SdvPacketEffect;

extern float sefKeyframeStepBlock[];
extern SdvPacketEffect *sefCurrentEffectRecord;
extern unsigned char MMathUnitMatrix[];
void *scGetImmAdrImmIdx2(int scriptBase, short *immediateTable, int immediateIndex);
void sefGetSourceMatrix(void *packet, short kindValue);
void func_0025A000(Vector4 *outPoint, const Vector4 *start, const Vector4 *next, float parameter);

/* Per-frame update of one effect owner. When its next row is due, claims image records and packet records for the
   row's rowParts and installs their keys; then advances every packet of the owner along its keys. */
void func_001F3A98(SdvPacketOwner *owner, int scriptBase, int elapsed) {
    SdvPacketRecord *packet;
    SdvPacketKey *keyTable;
    SdvPacketKey *key;
    SdvPacketPart *rowParts;
    SdvPacketPart *imagePart;
    SdvPacketSlot *imageSlot;
    int slotPos;
    short *row;
    float progress;
    int packetIndex;
    int imageRecordIndex;
    int imageCount;

    if (owner->rows == 0) {
        return;
    }
    if (owner->position < 4) {
        row = owner->rows + owner->position * 2;
        if (elapsed >= row[0] && row[0] < 0x1000) {
            float unitW = 1.0f;
            do {
                keyTable = scGetImmAdrImmIdx2(scriptBase, row, 1);
                if (keyTable != 0) {
                    packetIndex = func_001F3700();
                    packet = (packetIndex < 16U && gSdvPacketTable->packetFlags[packetIndex] == 1)
                                 ? &gSdvPacketTable->packets[packetIndex]
                                 : 0;
                    if (packet != 0) {
                        rowParts = scGetImmAdrImmIdx2(scriptBase, (short *) ((unsigned char *) keyTable - 10), 0);
                        if (rowParts != 0) {
                            imageCount = 0;
                            if (rowParts->frame < 0x1000) {
                                imagePart = rowParts;
                                while (imageCount < 16 && imagePart->frame < 0x1000) {
                                    int entryId;

                                    imageRecordIndex = func_001F3828();
                                    imageSlot = (imageRecordIndex < 64U && gSdvPacketTable->imageRecordFlags[imageRecordIndex] == 1)
                                                    ? &gSdvPacketTable->imageRecords[imageRecordIndex]
                                                    : 0;
                                    if (imageSlot == 0) {
                                        break;
                                    }
                                    entryId = sefCurrentEffectRecord->entryId;
                                    imageSlot->kind = imagePart->frame;
                                    imageSlot->blend = imagePart->blend;
                                    imageSlot->alpha = imagePart->alpha;
                                    func_001F3968(&imageSlot->tex0, &imageSlot->width, &imageSlot->height, entryId,
                                                  imagePart->image0, imagePart->palette0);
                                    func_001F3968(&imageSlot->clutTex0, 0, 0, entryId, imagePart->image1, imagePart->palette1);
                                    packet->slotIds[imageCount] = imageRecordIndex;
                                    imagePart++;
                                    imageCount++;
                                }
                            }
                            packet->time = 0;
                            packet->tag = 0;
                            packet->keys = keyTable;
                            if (keyTable[1].time >= 0x1000) {
                                packet->keyIndex = -1;
                                QuadwordCopyT0(&packet->position, &keyTable[0].start);
                                QuadwordCopyT0(&packet->heading, &keyTable[0].end);
                                packet->previousScale = packet->position.w;
                                packet->position.w = unitW;
                            }
                            packet->keyFlags.lowByte = 0;
                            /* Key flags: bit 0 = flagA != 0, bit 1 = flagB != 0, bit 2 = flagC == 0x2000.
                               kept apart: each bit is its own block, the block order decides the emitted code. */
                            {
                                {
                                    unsigned long long flagA = 0ULL;
                                    if (SDV_PACKET_KEY_FIELD(keyTable, SDV_PACKET_KEY_FLAG_A) != 0) {
                                        flagA = 1ULL;
                                    }
                                    packet->keyFlags.word = (packet->keyFlags.word & ~1) | flagA;
                                }
                                {
                                    unsigned long long flagB = 1ULL;
                                    if (SDV_PACKET_KEY_FIELD(keyTable, SDV_PACKET_KEY_FLAG_B) == 0) {
                                        flagB = 0ULL;
                                    }
                                    packet->keyFlags.word = (packet->keyFlags.word & ~2ULL) | (flagB << 1);
                                }
                                {
                                    unsigned long long flagC = 1ULL;
                                    if (SDV_PACKET_KEY_FIELD(keyTable, SDV_PACKET_KEY_FLAG_C) != 0x2000) {
                                        flagC = 0ULL;
                                    }
                                    packet->keyFlags.word = (packet->keyFlags.word & ~4ULL) | (flagC << 2);
                                }
                            }
                            packet->previousScale = unitW;
                            if (SDV_PACKET_KEY_FIELD(keyTable, SDV_PACKET_KEY_KIND_VALUE) != 0) {
                                sefGetSourceMatrix(packet, SDV_PACKET_KEY_FIELD(keyTable, SDV_PACKET_KEY_KIND_VALUE));
                            } else {
                                Vu0CopyUnitMatrix(packet, &MMathUnitMatrix);
                            }
                            owner->slotIds[owner->position] = packetIndex;
                        } else {
                            sdvPacketRelease(packetIndex);
                            owner->slotIds[owner->position] = -1;
                        }
                    }
                }
                owner->position++;
                row = owner->rows + owner->position * 2;
            } while (elapsed >= row[0] && row[0] < 0x1000);
        }
    }

    owner->time += (float) (int) sefKeyframeStepBlock[1];

    for (slotPos = 0; slotPos < 4; slotPos++) {
        SdvPacketRecord *ownedPacket = 0;
        int slotId = owner->slotIds[slotPos];
        if (slotId >= 0 && slotId < 16 && gSdvPacketTable->packetFlags[slotId] == 1) {
            ownedPacket = &gSdvPacketTable->packets[slotId];
        }
        if (ownedPacket != 0) {
            if (ownedPacket->keyIndex >= 0) {
                ownedPacket->time = ownedPacket->time + (int) sefKeyframeStepBlock[1];
                key = ownedPacket->keys + ownedPacket->keyIndex;
                ownedPacket->tag = key->tag;
                if ((short) ownedPacket->time > (int) key[1].time) {
                    if (key[2].time >= 0x1000) {
                        if ((ownedPacket->keyFlags.word & 4) != 0) {
                            ownedPacket->keyIndex = -1;
                            QuadwordCopyT0(&ownedPacket->position, &key[1].start);
                            QuadwordCopyT0(&ownedPacket->heading, &key[1].end);
                            ownedPacket->previousScale = ownedPacket->position.w;
                            ownedPacket->position.w = 1.0f;
                            continue;
                        } else {
                            sdvPacketRelease(owner->slotIds[slotPos]);
                            owner->slotIds[slotPos] = -1;
                            continue;
                        }
                    } else {
                        ownedPacket->keyIndex = ownedPacket->keyIndex + 1;
                        key++;
                    }
                }
                progress = (float) ((short) ownedPacket->time - key->time) / (float) (key[1].time - key->time);
                func_0025A000(&ownedPacket->position, &key->start, &key[1].start, progress);
                func_0025A000(&ownedPacket->heading, &key->end, &key[1].end, progress);
                ownedPacket->heading.w = ((key[1].end.w - key->end.w) * progress) + key->end.w;
                ownedPacket->previousScale = ((key[1].start.w - key->start.w) * progress) + key->start.w;
            }
            if ((ownedPacket->keyFlags.word & 1) == 1) {
                sefGetSourceMatrix(ownedPacket, SDV_PACKET_KEY_FIELD(ownedPacket->keys, SDV_PACKET_KEY_KIND_VALUE));
            }
        }
    }
}

#include "common.h"
#include "shared.h"

struct XglPacket;

#include "main/Packet.h"

XglPacket *xglPacketGetCurrent(void);
void sceVif1PkCnt(XglPacket *packet, unsigned int count);
void sceVif1PkAddCode(XglPacket *packet, unsigned int tag);
void sceVif1PkRef(XglPacket *packet, const void *data, unsigned int qwc, unsigned int a3, unsigned int a4,
                  unsigned int a5);

/* A 128-bit quadword (one lq/sq), as the VIF1 packet writers take it. */
typedef unsigned int XglQuadword __attribute__((mode(TI)));

/* The header sdvDrawAlter puts in front of each strip it sends by reference: sdvSetStripHeader clears its first
   two quadwords, fills the UNPACK code, primitive count and strip flag, and ORs the primitive count into the
   loop count at +0x20 (the third quadword is not cleared). */
typedef union SdvStripHeader {
    XglQuadword quad[3];
    struct {
        unsigned int unmodeled_00[3];
        unsigned int unpack;           /* +0x0C VIF UNPACK code for the whole strip */
        unsigned int primitiveCount;   /* +0x10 */
        unsigned int strip;            /* +0x14 sdvDrawAlter passes 1 with its count / 2, else 0 with count / 3 */
        unsigned int unmodeled_18[2];
        unsigned short loopCount;      /* +0x20 ORed with the primitive count */
    } layout;
} SdvStripHeader;

/* Keeps the physical address of an EE pointer: DMA references take physical addresses. */
#define SDV_PHYSICAL_ADDRESS_MASK 0x0FFFFFFF

void sefMemZero(void *dest, int size);
void *memset(void *destination, int value, unsigned int count);
void func_00258B78(void);
void func_001E7E50(int slot, int flag);

/* One 128-byte record of the table at D_0062F850; sdvFindFreeSpecialWork takes the first record whose word at +0x04 is zero. */
typedef struct SdvSpecialRecord {
    unsigned char unmodeled_00[0x04];
    unsigned int inUse;
    unsigned char unmodeled_08[0x78];
} SdvSpecialRecord;

extern SdvSpecialRecord D_0062F850[];

/* One alter record, 0xE70 bytes; sdvInitAlter zeroes all of it. Only the two bytes sdvDestroyAlter reads are named. */
typedef struct SdvAlterHeader {
    unsigned char unmodeled_00[8];
    unsigned short active;
    unsigned char unmodeled_0A[0xE1E - 0xE1A];
} SdvAlterHeader;

typedef struct SdvAlter {
    unsigned char unmodeled_000[0xAF2];
    short owner;               /* +0xAF2 compared with the owner argument of sdvDestroyOwnerAlters */
    unsigned char unmodeled_AF4[0xE10 - 0xAF4];
    SdvAlterHeader header;
    signed char specialSlot;   /* +0xE1E: the special record sdvClearSpecialRecord clears */
    signed char otherSlot;     /* +0xE1F: first argument of func_001E7E50 in sdvDestroyAlter */
    unsigned char unmodeled_E20[0xE70 - 0xE20];
} SdvAlter;

/* The 49 alter records (0xE70 bytes each) start at D_00630850. */
extern SdvAlter D_00630850[];

INCLUDE_ASM("asm/main/nonmatchings/sdv_alter", func_001EEF18);

void sdvResetSpecialWork(void)
{
    memset(D_0062F850, 0, 4096);
    func_00258B78();
}

int sdvFindFreeSpecialWork(void)
{
    SdvSpecialRecord *table = D_0062F850;
    SdvSpecialRecord *record;
    int i;

    record = &table[1];
    for (i = 1; i < 32; i++, record++) {
        if (record->inUse == 0) {
            return i;
        }
    }
    return 0;
}

/* Clears the record of one special slot; slot 0 is kept. */
void sdvClearSpecialRecord(int slot)
{
    if (slot > 0) {
        memset(&D_0062F850[slot], 0, sizeof(SdvSpecialRecord));
    }
}

INCLUDE_ASM("asm/main/nonmatchings/sdv_alter", func_001EF338);

INCLUDE_ASM("asm/main/nonmatchings/sdv_alter", func_001EF448);

INCLUDE_ASM("asm/main/nonmatchings/sdv_alter", func_001EF598);

INCLUDE_ASM("asm/main/nonmatchings/sdv_alter", func_001EFD88);

INCLUDE_ASM("asm/main/nonmatchings/sdv_alter", func_001EFF20);

INCLUDE_ASM("asm/main/nonmatchings/sdv_alter", func_001EFF50);

INCLUDE_ASM("asm/main/nonmatchings/sdv_alter", func_001F0DB0);

INCLUDE_ASM("asm/main/nonmatchings/sdv_alter", func_001F0E58);

void sdvInitAlter(SdvAlter *alter)
{
    sefMemZero(alter, sizeof(*alter));
}

INCLUDE_ASM("asm/main/nonmatchings/sdv_alter", func_001F1220);

void sdvDestroyAlter(SdvAlter *alter)
{
    func_001E7E50(alter->otherSlot, 0);
    sdvClearSpecialRecord(alter->specialSlot);
    sefMemZero(alter, sizeof(*alter));
}

void sdvDestroyOwnerAlters(int owner)
{
    SdvAlter *table;
    SdvAlterHeader *header;
    SdvAlter *alter;
    int i;
    unsigned int offset;
    unsigned int cursor;

    if (owner < 0) {
        return;
    }
    offset = 0;
    table = D_00630850;
    header = &D_00630850[0].header;
    for (i = 0; i < 49; i++) {
        cursor = offset;
        if (header->active != 0) {
            alter = (SdvAlter *)(cursor + (unsigned int)table);
            if (alter->owner == owner) {
                sdvDestroyAlter(alter);
            }
        }
        header = (SdvAlterHeader *)((char *)header + sizeof(SdvAlter));
        offset += sizeof(SdvAlter);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/sdv_alter", func_001F17D0);

void sdvPacketFlushMscnt(void)
{
    XglPacket *packet;

    packet = xglPacketGetCurrent();
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddCode(packet, 0x11000000);
    sceVif1PkAddCode(packet, 0x17000000);
}

/*
 * Clears and fills a strip header, then references its qwc quadwords (physical address) in the current packet.
 * Masking the address inside the sceVif1PkRef call loads the mask after the calls instead of before them
 * (fndiff 44 insns, 5/48 in place; this form 48/48).
 */
void sdvSetStripHeader(SdvStripHeader *header, int unused, int qwc, int primitiveCount, int strip)
{
    XglPacket *packet;
    void *physical;

    physical = (void *)((unsigned int)header & SDV_PHYSICAL_ADDRESS_MASK);
    packet = xglPacketGetCurrent();
    sdvPacketFlushMscnt();
    header->quad[0] = 0;
    header->quad[1] = 0;
    header->layout.unpack = ((qwc - 1) << 16) | 0x6C008000;
    header->layout.primitiveCount = primitiveCount;
    header->layout.strip = strip;
    header->layout.loopCount |= primitiveCount;
    sceVif1PkRef(packet, physical, qwc, 0, 0, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/sdv_alter", func_001F25B0);

void sdvInitAlters(void)
{
    SdvAlter *alter;
    int i;

    alter = D_00630850;
    for (i = 0; i < 49; i++) {
        sdvInitAlter(alter);
        alter++;
    }
}

void sdvDestroyAlters(void)
{
    SdvAlter *alter;
    int i;

    alter = D_00630850;
    for (i = 0; i < 49; i++) {
        sdvDestroyAlter(alter);
        alter++;
    }
}

void func_001F17D0(SdvAlter *alter);

void sdvExecAlters(void)
{
    int i;

    for (i = 0; i < 49; i++) {
        if (D_00630850[i].header.active != 0) {
            func_001F17D0(&D_00630850[i]);
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/sdv_alter", func_001F3458);

INCLUDE_ASM("asm/main/nonmatchings/sdv_alter", sdvDrawAlters);

INCLUDE_ASM("asm/main/nonmatchings/sdv_alter", func_001F35E0);

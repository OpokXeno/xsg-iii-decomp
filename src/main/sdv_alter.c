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
void gifStateClearModeHook(void);
void slotReleaseChecked(int slot, int flag);

/* One 128-byte record of the table at gSdvSpecialRecords; sdvFindFreeSpecialWork takes the first record whose word at +0x04 is zero. */
typedef struct SdvSpecialRecord {
    unsigned char unmodeled_00[0x04];
    unsigned int inUse;
    unsigned char unmodeled_08[0x78];
} SdvSpecialRecord;

extern SdvSpecialRecord gSdvSpecialRecords[];

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
    SdvAlterHeader alterHeader;
    signed char specialSlot;   /* +0xE1E: the special record sdvClearSpecialRecord clears */
    signed char ownedSlot;     /* +0xE1F: first argument of slotReleaseChecked in sdvDestroyAlter */
    unsigned char unmodeled_E20[0xE70 - 0xE20];
} SdvAlter;

/* The 49 alter records (0xE70 bytes each) start at gSdvAlters. */
extern SdvAlter gSdvAlters[];

INCLUDE_ASM("asm/main/nonmatchings/sdv_alter", func_001EEF18);

void sdvResetSpecialWork(void)
{
    memset(gSdvSpecialRecords, 0, 4096);
    gifStateClearModeHook();
}

int sdvFindFreeSpecialWork(void)
{
    SdvSpecialRecord *specialRecords = gSdvSpecialRecords;
    SdvSpecialRecord *specialRecord;
    int freeSpecialSlot;

    specialRecord = &specialRecords[1];
    for (freeSpecialSlot = 1; freeSpecialSlot < 32; freeSpecialSlot++, specialRecord++) {
        if (specialRecord->inUse == 0) {
            return freeSpecialSlot;
        }
    }
    return 0;
}

/* Clears the record of one special slot; slot 0 is kept. */
void sdvClearSpecialRecord(int specialSlot)
{
    if (specialSlot > 0) {
        memset(&gSdvSpecialRecords[specialSlot], 0, sizeof(SdvSpecialRecord));
    }
}

INCLUDE_ASM("asm/main/nonmatchings/sdv_alter", func_001EF338);

INCLUDE_ASM("asm/main/nonmatchings/sdv_alter", func_001EF448);

INCLUDE_ASM("asm/main/nonmatchings/sdv_alter", func_001EF598);

INCLUDE_ASM("asm/main/nonmatchings/sdv_alter", func_001EFD88);

typedef int s32;
extern s32 D_0055109C;

s32 sdvAddressInWindow(void) {
    s32 addressIsInWindow;

    addressIsInWindow = 0;
    if (D_0055109C <= 0x1014D003) {
        addressIsInWindow = D_0055109C > 0x1013FFFF;
    }
    return addressIsInWindow;
}

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
    slotReleaseChecked(alter->ownedSlot, 0);
    sdvClearSpecialRecord(alter->specialSlot);
    sefMemZero(alter, sizeof(*alter));
}

/* Visits the 49 alters in address order while the counter runs down (the original loop shape). The header pointer
   is stepped by sizeof(SdvAlter) and the alter is rebuilt from the byte offset: typed indexing (&table[i], a pointer
   walk, &table[i].alterHeader) changed the codegen. */
void sdvDestroyOwnerAlters(int owner)
{
    SdvAlter *alters;
    SdvAlterHeader *alterHeader;
    SdvAlter *alter;
    int i;
    unsigned int alterByteOffset;

    if (owner < 0) {
        return;
    }
    alterByteOffset = 0;
    alters = gSdvAlters;
    alterHeader = &gSdvAlters[0].alterHeader;
    for (i = 48; i >= 0; i--) {
        if (alterHeader->active != 0) {
            alter = (SdvAlter *)(alterByteOffset + (unsigned int)alters);
            if (alter->owner == owner) {
                sdvDestroyAlter(alter);
            }
        }
        alterHeader = (SdvAlterHeader *)((char *)alterHeader + sizeof(SdvAlter));
        alterByteOffset += sizeof(SdvAlter);
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
void sdvSetStripHeader(SdvStripHeader *stripHeader, int unused, int qwc, int primitiveCount, int strip)
{
    XglPacket *packet;
    void *physical;

    physical = (void *)((unsigned int)stripHeader & SDV_PHYSICAL_ADDRESS_MASK);
    packet = xglPacketGetCurrent();
    sdvPacketFlushMscnt();
    stripHeader->quad[0] = 0;
    stripHeader->quad[1] = 0;
    stripHeader->layout.unpack = ((qwc - 1) << 16) | 0x6C008000;
    stripHeader->layout.primitiveCount = primitiveCount;
    stripHeader->layout.strip = strip;
    stripHeader->layout.loopCount |= primitiveCount;
    sceVif1PkRef(packet, physical, qwc, 0, 0, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/sdv_alter", func_001F25B0);

void sdvInitAlters(void)
{
    SdvAlter *alter;
    int i;

    alter = gSdvAlters;
    for (i = 0; i < 49; i++) {
        sdvInitAlter(alter);
        alter++;
    }
}

void sdvDestroyAlters(void)
{
    SdvAlter *alter;
    int i;

    alter = gSdvAlters;
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
        if (gSdvAlters[i].alterHeader.active != 0) {
            func_001F17D0(&gSdvAlters[i]);
        }
    }
}

/* The work block of an owner whose alters sdvExecOwnerAlters runs; only its owner id is named. */
typedef struct SdvOwnerWork {
    unsigned char unmodeled_000[0x13E];
    short ownerId;             /* +0x13E compared with SdvAlter.owner */
} SdvOwnerWork;

void sdvExecOwnerAlters(SdvOwnerWork *ownerWork) {
    SdvAlter *alter;
    s32 i;

    alter = gSdvAlters;
    for (i = 0; i < 49; i++) {
        if ((alter->alterHeader.active != 0) && (alter->owner == ownerWork->ownerId)) {
            func_001F17D0(alter);
        }
        alter++;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/sdv_alter", sdvDrawAlters);

INCLUDE_ASM("asm/main/nonmatchings/sdv_alter", func_001F35E0);

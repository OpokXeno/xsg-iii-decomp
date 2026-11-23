#include "common.h"
#include "shared.h"

struct XglPacket;

#include "main/Packet.h"
#include "main/xgl_2.h"
#include "vu0_macro.h"

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


/* One effect record of a slot event: entries[] start at +0x0C, 32 bytes each, the first halfword is the frame the
   entry becomes due (0x2000: always). */
typedef struct SdvSlotEntry {
    unsigned short frame;
    unsigned char unmodeled_02[0x1E];
} SdvSlotEntry;

typedef struct SdvSlotData {
    unsigned short kind;     /* +0x00 low byte: effect opcode; high byte (signed): target passed to func_001EEF18 */
    unsigned short mode;     /* +0x02 8: arguments are absolute (not scaled) */
    unsigned char unmodeled_04[8];
    SdvSlotEntry entries[1]; /* +0x0C */
} SdvSlotData;

typedef struct SdvSlotEvent {
    float frame;             /* +0x00 */
    SdvSlotData *data;       /* +0x04 */
    short args[16];          /* +0x08 */
    short entryIndex;        /* +0x28 */
    unsigned short windowed; /* +0x2A nonzero: the window at +0x30 limits the effect */
    unsigned char unmodeled_2C[4];
    unsigned char window[0x40]; /* +0x30 */
    int offset[3];           /* +0x70 added to the event position */
} SdvSlotEvent;

typedef struct SefScheduler {
    unsigned char unmodeled_00[4];
    int flags;               /* +0x04 bit 22 is copied into the studio block */
} SefScheduler;

extern long long gSbStudioBlock[];
extern short D_00408090[];
int func_001EEF18(int *result, int *position, int mode, int code, float *scale, int target, void *window);
SefScheduler *sefGetNowScheduler(void);
int slotSetupMode0(int slot, int opcode, int *setup);
int slotUpdateMode1(int slot, int opcode, int *setup);
void func_00205638(Vector4 *destination, Vector4 *source, int mode);
void MMathInvertAffineMatrix(Matrix4 *out, float *src);
void xglMatrixRotX(float *matrix, float *source, float angle);
void xglMatrixRotY(float *matrix, float *source, float angle);
void xglMatrixRotZ(float *matrix, float *source, float angle);
void xglVectorMulMat(Vector4 *destination, Matrix4 matrix, const Vector4 *vector);
void xglVectorScaleXYZ(Vector4 *destination, const Vector4 *source, float scale);
Vector4 *MMathNormalizeVector2(Vector4 *destination, const Vector4 *source);
XglCamera *xglStudioSelectGetActiveCamera(int index);

static inline void normalizeInPlace(Vector4 *direction)
{
    MMathNormalizeVector2(direction, direction);
}

/* SdvSlotData.mode: the event arguments are absolute coordinates, not scaled to the located target. */
#define SDV_ARGUMENTS_ABSOLUTE 8
/* SdvSlotEntry.frame: the entry is always due. */
#define SDV_ENTRY_ALWAYS_DUE 0x2000

/* Starts (update == 0) or updates the slot effect of one event: decodes the opcode's arguments into the setup words
   and hands them to the slot system; slot receives the slot number. */
void sdvApplySlotEvent(signed char *slot, SdvSlotEvent *event, int update)
{
    int position[4];
    int located[4];
    float scale[4];
    int setup[15];
    AlignedVector4 vector;
    Matrix4 matrix;
    double percentScale;
    SdvSlotData *effectData;
    SdvSlotEntry *dueEntry;
    SdvSlotEntry *effectEntries;
    short *eventArgs;
    int opcode;
    int targetId;
    int argumentMode;
    void *window;
    int sizeOrDepth;
    unsigned int tableIndex;
    int schedulerBit22;
    int depth;
    long long *studioBlock;
    unsigned short kindWord;
    int windowActive;
    XglCamera *camera;

    if (event == 0) {
        return;
    }
    effectData = event->data;
    if (effectData == 0) {
        return;
    }
    effectEntries = effectData->entries;
    kindWord = effectData->kind;
    eventArgs = event->args;
    dueEntry = &effectEntries[event->entryIndex];
    argumentMode = effectData->mode;
    opcode = kindWord & 0xFF;
    targetId = (short) kindWord >> 8;
    windowActive = sdvAddressInWindow();
    if (event->windowed != 0 && windowActive != 0) {
        window = event->window;
    } else {
        window = 0;
    }
    memset(setup, 0, sizeof(setup));
    if (dueEntry->frame != SDV_ENTRY_ALWAYS_DUE && (float) dueEntry->frame > event->frame) {
        return;
    }
    Vu0SetOrigin(position);

    switch (opcode) {
    case 4:
        setup[0] = eventArgs[1];
        setup[1] = eventArgs[2];
        setup[2] = eventArgs[3];
        setup[3] = eventArgs[4];
        setup[4] = eventArgs[5];
        setup[5] = eventArgs[6];
        setup[6] = eventArgs[7];
        setup[7] = eventArgs[8];
        break;
    case 5:
        setup[0] = (unsigned short) eventArgs[1] << 8;
        setup[1] = eventArgs[2] + 0x100;
        setup[2] = eventArgs[3] + 0xE0;
        setup[3] = eventArgs[4];
        setup[4] = eventArgs[5];
        sizeOrDepth = 1;
        for (tableIndex = 0; tableIndex < 9; tableIndex++) {
            if (D_00408090[tableIndex] >= eventArgs[6]) {
                sizeOrDepth = D_00408090[tableIndex];
                break;
            }
        }
        setup[5] = sizeOrDepth;
        setup[6] = eventArgs[7];
        setup[7] = eventArgs[8];
        setup[8] = eventArgs[9];
        setup[9] = eventArgs[10];
        setup[10] = eventArgs[11];
        break;
    case 6:
        setup[0] = (unsigned short) eventArgs[1] << 8;
        setup[1] = eventArgs[2] + 0x100;
        setup[2] = eventArgs[3] + 0xE0;
        setup[3] = eventArgs[4];
        setup[4] = eventArgs[5];
        setup[5] = eventArgs[6];
        setup[6] = eventArgs[7];
        setup[7] = eventArgs[8];
        break;
    case 9:
        setup[0] = eventArgs[1];
        setup[1] = eventArgs[2];
        setup[2] = eventArgs[3];
        setup[3] = eventArgs[4];
        setup[4] = eventArgs[5];
        break;
    case 29:
        setup[0] = eventArgs[1];
        setup[1] = eventArgs[2];
        setup[2] = eventArgs[3];
        setup[3] = eventArgs[4];
        setup[4] = eventArgs[5];
        setup[5] = eventArgs[6];
        break;
    case 12:
        position[0] = eventArgs[2] + event->offset[0];
        position[1] = eventArgs[3] + event->offset[1];
        position[2] = eventArgs[4] + event->offset[2];
        Vu0SetOrigin(scale);
        scale[0] = 5.12f;
        scale[1] = 4.48f;
        if (func_001EEF18(located, position, argumentMode, (unsigned short) eventArgs[1], scale, targetId, window) != 0) {
            setup[0] = located[2];
            setup[1] = located[0];
            setup[2] = located[1];
            setup[3] = eventArgs[5];
            setup[4] = eventArgs[6];
            setup[5] = eventArgs[7];
            setup[6] = eventArgs[8];
        }
        break;
    case 38:
        position[0] = eventArgs[2] + event->offset[0];
        position[1] = eventArgs[3] + event->offset[1];
        position[2] = eventArgs[4] + event->offset[2];
        Vu0SetOrigin(scale);
        scale[0] = 100.0f;
        scale[1] = 100.0f;
        if (func_001EEF18(located, position, argumentMode, (unsigned short) eventArgs[1], scale, targetId, window) != 0) {
            float factor = scale[0] * 0.0001f;

            setup[0] = located[2];
            setup[1] = located[0];
            setup[2] = located[1];
            if (argumentMode == SDV_ARGUMENTS_ABSOLUTE) {
                setup[3] = eventArgs[5];
                setup[4] = eventArgs[6];
                setup[5] = eventArgs[7];
            } else {
                setup[3] = eventArgs[5] * factor;
                setup[4] = eventArgs[6];
                setup[5] = eventArgs[7] * factor;
            }
            setup[6] = eventArgs[8];
            setup[7] = eventArgs[9];
            setup[8] = eventArgs[9];
            setup[9] = eventArgs[11];
            setup[10] = eventArgs[12];
            setup[11] = eventArgs[13];
        }
        break;
    case 14:
        vector.x = eventArgs[2] + event->offset[0];
        vector.y = eventArgs[3] + event->offset[1];
        vector.z = eventArgs[4] + event->offset[2];
        vector.w = 1.0f;
        normalizeInPlace(&vector);
        xglVectorScaleXYZ(&vector, &vector, eventArgs[5]);
        camera = xglStudioSelectGetActiveCamera(0);
        func_00205638(&vector, &vector, argumentMode);
        vector.w = 1.0f;
        xglMatrixUnit(matrix);
        xglMatrixRotY(matrix[0], matrix[0], camera->travel.rotation.y);
        xglMatrixRotX(matrix[0], matrix[0], camera->travel.rotation.x);
        xglMatrixRotZ(matrix[0], matrix[0], camera->travel.rotation.z);
        MMathInvertAffineMatrix(&matrix, matrix[0]);
        xglVectorMulMat(&vector, matrix, &vector);
        setup[0] = (unsigned short) eventArgs[1] << 8;
        setup[1] = vector.x;
        setup[2] = vector.y;
        setup[5] = eventArgs[6];
        setup[6] = eventArgs[7];
        setup[7] = eventArgs[8];
        break;
    case 13:
        position[0] = eventArgs[2] + event->offset[0];
        position[1] = eventArgs[3] + event->offset[1];
        position[2] = eventArgs[4] + event->offset[2];
        Vu0SetOrigin(scale);
        scale[0] = 100.0f;
        scale[1] = 100.0f;
        if (func_001EEF18(located, position, argumentMode, (unsigned short) eventArgs[1], scale, targetId, window) != 0) {
            float factor = scale[0] * 0.0001f;

            setup[0] = located[2];
            setup[1] = located[0];
            setup[2] = located[1];
            if (argumentMode == SDV_ARGUMENTS_ABSOLUTE) {
                setup[3] = eventArgs[5];
                setup[4] = eventArgs[6];
                setup[5] = eventArgs[7];
                setup[7] = eventArgs[9];
            } else {
                setup[3] = eventArgs[5] * factor;
                setup[4] = eventArgs[6] * factor;
                setup[5] = eventArgs[7] * factor;
                setup[7] = eventArgs[9] * factor;
            }
            setup[6] = eventArgs[8];
            setup[8] = eventArgs[10];
            setup[9] = eventArgs[11];
            setup[10] = eventArgs[12];
            setup[11] = eventArgs[13];
            setup[12] = eventArgs[14];
            setup[13] = eventArgs[15];
        }
        break;
    case 17:
    case 18:
        position[0] = eventArgs[2] + event->offset[0];
        position[1] = eventArgs[3] + event->offset[1];
        position[2] = eventArgs[4] + event->offset[2];
        Vu0SetOrigin(scale);
        scale[0] = 100.0f;
        scale[1] = 100.0f;
        if (func_001EEF18(located, position, argumentMode, (unsigned short) eventArgs[1], scale, targetId, window) != 0) {
            float factor = scale[0] * 0.0001f;

            setup[0] = located[2];
            setup[1] = located[0];
            setup[2] = located[1];
            if (argumentMode == SDV_ARGUMENTS_ABSOLUTE) {
                setup[3] = eventArgs[5];
                setup[4] = eventArgs[6];
                setup[5] = eventArgs[7];
                setup[6] = eventArgs[8];
                setup[7] = eventArgs[9];
            } else {
                setup[3] = eventArgs[5] * factor;
                setup[4] = eventArgs[6] * factor;
                setup[5] = eventArgs[7] * factor;
                setup[6] = eventArgs[8] * factor;
                setup[7] = eventArgs[9] * factor;
            }
            setup[8] = eventArgs[10];
            setup[9] = eventArgs[11];
            setup[10] = eventArgs[12];
        }
        break;
    case 20:
        setup[0] = (unsigned short) eventArgs[1] << 8;
        setup[1] = eventArgs[2];
        setup[2] = eventArgs[3];
        setup[3] = eventArgs[4];
        setup[4] = eventArgs[5];
        setup[5] = eventArgs[6];
        setup[6] = eventArgs[7];
        setup[7] = eventArgs[8];
        break;
    case 25:
        setup[0] = eventArgs[1] + event->offset[0];
        setup[1] = eventArgs[2] + event->offset[1];
        if (argumentMode == SDV_ARGUMENTS_ABSOLUTE) {
            sizeOrDepth = (unsigned short) eventArgs[3] << 8;
        } else {
            sizeOrDepth = eventArgs[3] + event->offset[2];
        }
        setup[2] = sizeOrDepth;
        setup[3] = eventArgs[4];
        setup[4] = eventArgs[5];
        setup[5] = eventArgs[6];
        setup[6] = eventArgs[7];
        setup[7] = eventArgs[8];
        setup[8] = eventArgs[9];
        setup[9] = eventArgs[10];
        setup[10] = eventArgs[11] | (eventArgs[12] << 8) | (eventArgs[13] << 16) | (eventArgs[14] << 24);
        setup[11] = argumentMode | (targetId << 16);
        setup[12] = (int) sefGetNowScheduler();
        setup[14] = eventArgs[15];
        break;
    case 27:
        setup[0] = eventArgs[1] + event->offset[0];
        setup[1] = eventArgs[2] + event->offset[1];
        if (argumentMode == SDV_ARGUMENTS_ABSOLUTE) {
            depth = (unsigned short) eventArgs[3] << 8;
        } else {
            depth = eventArgs[3] + event->offset[2];
        }
        setup[2] = depth;
        setup[3] = eventArgs[4];
        setup[4] = eventArgs[5];
        setup[5] = eventArgs[6];
        setup[6] = eventArgs[7];
        setup[7] = eventArgs[8];
        setup[8] = eventArgs[9];
        setup[9] = eventArgs[10];
        setup[10] = eventArgs[11];
        setup[11] = eventArgs[12] | (eventArgs[13] << 16);
        setup[12] = eventArgs[14];
        setup[13] = argumentMode | (targetId << 16);
        setup[14] = (int) sefGetNowScheduler();
        break;
    case 21:
        setup[0] = (unsigned short) eventArgs[1] << 8;
        setup[1] = eventArgs[2];
        setup[2] = eventArgs[3];
        setup[3] = eventArgs[4];
        break;
    case 37:
        setup[0] = (unsigned short) eventArgs[1] << 8;
        setup[1] = eventArgs[2];
        setup[2] = eventArgs[3];
        break;
    case 22:
        position[0] = eventArgs[2] + event->offset[0];
        position[1] = eventArgs[3] + event->offset[1];
        position[2] = eventArgs[4] + event->offset[2];
        Vu0SetOrigin(scale);
        percentScale = 0.01;
        scale[0] = eventArgs[6] * percentScale;
        scale[1] = eventArgs[7] * percentScale;
        if (func_001EEF18(located, position, argumentMode, (unsigned short) eventArgs[1], scale, targetId, window) != 0) {
            setup[0] = located[2];
            setup[1] = eventArgs[8];
            setup[2] = eventArgs[9];
            setup[3] = eventArgs[10];
            setup[4] = eventArgs[11];
            setup[5] = eventArgs[12];
            setup[6] = eventArgs[13];
            setup[7] = eventArgs[5];
            setup[8] = located[0];
            setup[9] = located[1];
            if (argumentMode == SDV_ARGUMENTS_ABSOLUTE) {
                setup[10] = eventArgs[6];
                setup[11] = eventArgs[7];
            } else {
                setup[10] = scale[0];
                setup[11] = scale[1];
            }
        }
        break;
    case 19:
    case 23:
        setup[0] = (unsigned short) eventArgs[1] << 8;
        setup[1] = eventArgs[2];
        setup[2] = eventArgs[3];
        setup[3] = eventArgs[4];
        setup[4] = eventArgs[5];
        setup[5] = eventArgs[6];
        setup[6] = eventArgs[7];
        break;
    case 11:
    case 24:
        setup[0] = (unsigned short) eventArgs[1] << 8;
        setup[1] = eventArgs[2];
        setup[2] = eventArgs[3];
        setup[3] = eventArgs[4];
        setup[4] = eventArgs[5];
        setup[5] = eventArgs[6];
        setup[6] = eventArgs[7];
        setup[7] = eventArgs[8];
        setup[8] = eventArgs[9];
        break;
    case 26:
        position[0] = eventArgs[2] + event->offset[0];
        position[1] = eventArgs[3] + event->offset[1];
        position[2] = eventArgs[4] + event->offset[2];
        Vu0SetOrigin(scale);
        percentScale = 0.01;
        scale[0] = eventArgs[5] * percentScale;
        scale[1] = eventArgs[6] * percentScale;
        if (func_001EEF18(located, position, argumentMode, (unsigned short) eventArgs[1], scale, targetId, window) != 0) {
            setup[0] = located[2];
            setup[1] = located[0];
            setup[2] = located[1];
            if (argumentMode == SDV_ARGUMENTS_ABSOLUTE) {
                setup[3] = eventArgs[5];
                setup[4] = eventArgs[6];
            } else {
                setup[3] = scale[0];
                setup[4] = scale[1];
            }
            setup[5] = eventArgs[7];
            setup[6] = eventArgs[8];
            setup[7] = eventArgs[9];
            setup[8] = eventArgs[10];
        }
        break;
    default:
        return;
    }

    studioBlock = gSbStudioBlock;
    if (sefGetNowScheduler() != 0) {
        schedulerBit22 = (sefGetNowScheduler()->flags >> 22) & 1;
    } else {
        schedulerBit22 = 0;
    }
    studioBlock[5] = (studioBlock[5] & ~((long long) 1 << 32)) | ((long long) schedulerBit22 << 32);
    if (update != 0) {
        *slot = slotUpdateMode1(*slot, opcode, setup);
    } else {
        *slot = slotSetupMode0(*slot, opcode, setup);
    }
}

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

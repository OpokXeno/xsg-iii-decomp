#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu203", func_00249230);

INCLUDE_ASM("asm/main/nonmatchings/tu203", func_002493C0);

INCLUDE_ASM("asm/main/nonmatchings/tu203", func_00249740);

typedef signed char s8;
typedef int s32;
typedef unsigned int u32;
typedef float f32;
typedef unsigned char u8;
extern s8 gXglCdDiskStatus;

void xglCdSetDiskStatus(s8 diskStatus) {
    gXglCdDiskStatus = diskStatus;
}

INCLUDE_ASM("asm/main/nonmatchings/tu203", func_00249870);

INCLUDE_ASM("asm/main/nonmatchings/tu203", func_00249B40);

/* The request block xglCdReadFileBlocking fills: the destination and the completion flags (the bytes between are not used here). */
typedef struct XglCdReadJob {
    u8 unk_00[0x28];
    s32 loadAddress;
    u8 unk_2C[0x8];
    s32 completionFlags;
} XglCdReadJob;

#define xglCdReadJob (*(XglCdReadJob *) gXglCdReadJob)

extern unsigned char gXglCdReadJob[];
s32 xglCdReadFile(s32 fileId, s32 destination, s32 mode, s32 notify);
s32 xglCdServiceIfPending();

void xglCdReadFileBlocking(s32 fileId)
{
  while (xglCdServiceIfPending() != 0)
  {
  }
  xglCdReadFile(fileId, xglCdReadJob.loadAddress, 1, 0);
  xglCdReadJob.completionFlags |= 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu203", func_00249C00);

extern s32 gXglCdPendingStatus[];
s32 func_00249870(s32 status);

void xglCdFlushStatus(void) {
    func_00249870(gXglCdPendingStatus[0]);

}

INCLUDE_ASM("asm/main/nonmatchings/tu203", func_00249CE0);

INCLUDE_ASM("asm/main/nonmatchings/tu203", func_0024A008);

INCLUDE_ASM("asm/main/nonmatchings/tu203", func_0024A058);

INCLUDE_ASM("asm/main/nonmatchings/tu203", func_0024A0B8);

INCLUDE_ASM("asm/main/nonmatchings/tu203", func_0024A0E8);

INCLUDE_ASM("asm/main/nonmatchings/tu203", func_0024A340);

void func_0024A340(s32 id, void *payload);

/* One 0x90-byte envelope record. The view starts at the timing words (+0x40 of the record), so the record's own
 * payload and id live at the tail of the previous view. */
typedef struct EnvelopeEntry {
    s32 time;
    f32 rate;
    f32 level;
    u8 unk_0C[0x68];
    s32 id;
    u8 unk_78[0x18];
} EnvelopeEntry;

typedef struct EnvelopePayload {
    u8 envelopeBytes[0x90];
} EnvelopePayload;

/* Count-prefixed table: the same records seen from the payload start (+0x10) and from the timing words (+0x50). */
typedef struct EnvelopeTable {
    u32 count;
    u8 unk_04[0xC];
    union {
        EnvelopePayload payloads[1];
        struct {
            u8 unk_00[0x40];
            EnvelopeEntry envelopeEntries[1];
        } timing;
    } envelopeSteps;
} EnvelopeTable;

void xglEnvelopeTableUpdate(EnvelopeTable *envelopeTable, s32 time) {
    f32 level;
    u32 envelopeStepIndex;
    EnvelopeEntry *envelopeEntry;
    EnvelopePayload *envelopePayload;

    envelopeStepIndex = 0;
    if (envelopeTable->count != 0) {
        envelopePayload = envelopeTable->envelopeSteps.payloads;
        envelopeEntry = envelopeTable->envelopeSteps.timing.envelopeEntries;
        do {
            if (time >= envelopeEntry->time) {
                func_0024A340(envelopeEntry[-1].id, envelopePayload);
                level = envelopeEntry->level + envelopeEntry->rate;
                envelopeEntry->level = level;
                if (level > 128.0f) {
                    envelopeEntry->level = 128.0f;
                }
            }
            envelopeStepIndex += 1;
            envelopePayload += 1;
            envelopeEntry += 1;
        } while (envelopeStepIndex < envelopeTable->count);
    }
}

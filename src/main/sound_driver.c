#include "common.h"
#include "shared.h"
#include "main/tu018.h"

/* >>> CHUNK se_bank_load */

int soundDriverReset(void);
int korgEvagAllUnitsSoundOff(void);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018E6F0);

void func_0018E6F0(int a0, int a1, int a2, unsigned int a3);

void seBgmLoadDefault(int a0, int a1, int a2)
{
    func_0018E6F0(a0, a1, a2, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018E870);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018E940);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018EB70);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018EBE8);

void func_0018ECF0(unsigned int a0, unsigned int a1, unsigned int a2);
extern const char D_005404E0[];
int xglCdReadFile(const char *name, void *buffer, int mode, void *notify);
void func_0018E940(int a0, void *a1);
void func_00180DD8(unsigned int a0, int a1);

void seLoadMasterBank(void)
{
    xglCdReadFile(D_005404E0, (void *)0x01F00000, 0, 0);
    func_0018E940(0, (void *)0x01F00000);
    func_00180DD8(0xA4000000, 0x600);
}

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018EC68);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018ECF0);

void seVoiceStartDefault(unsigned int a0)
{
    func_0018ECF0(a0, 127, 0xffff);
}

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018ED68);

int soundDriverReset(void)
{
    return korgEvagAllUnitsSoundOff();
}

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018EDA8);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018EDD0);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018EDD8);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018F078);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018F098);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018F118);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018F190);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018F1D0);

/* <<< CHUNK se_bank_load */

/* >>> CHUNK sound_driver */

int seVolumeCurve(int level);

/* One 16-byte sound slot. Driver calls take driver_id; the create call takes the voice's work
 * buffer and its size. */
typedef struct SoundSlot {
    int driver_id;
    struct {
        unsigned char *buffer;        /* the voice's work buffer, carved from WorkEnd */
        int buffer_size;
    } create;                         /* what the create call takes */
    VolumeRamp ramp;                  /* +0x0C */
} SoundSlot;

/* The sound control block: a 16-byte head with the song number, the levels and the master volume
 * ramp, then the two sound slots (the label soundSlots is slots[0]). */
typedef union SoundControl {
    struct {
        unsigned char initialized;    /* +0x00: nonzero once the driver setup has run */
        unsigned char unmodeled_01[2];
        unsigned char song;           /* +0x03: number of the loaded song (seBgmLoad) */
        unsigned char master_volume;  /* +0x04: scales every level */
        unsigned char master_level;   /* +0x05: default master (music) level */
        unsigned char voice_level;    /* +0x06: default voice level */
        unsigned char slot_level;     /* +0x07: default sound slot level */
        unsigned char unmodeled_08[4];
        VolumeRamp master_ramp;       /* +0x0C */
        SoundSlot slots[2];           /* +0x10 */
    };
    /* The head has a slot's layout too; soundSlotReopen indexes the slots as records[index + 1]. */
    SoundSlot records[3];
} SoundControl;

extern SoundControl soundCtrl;
/* Sets the loop flag of the driver voice of the sound slot index, then starts the file fileKey * on that voice. A null slot (the lookup failed) does nothing. */ void soundSlotStartFile(int index, const char *fileKey);
void ADXT_SetLpFlg(int id, int value);
void ADXT_StartFname(int id, const char *key);
int ADXT_SetOutVol(int id, int value);
void ADXM_ExecVint(int value);
SoundSlot *func_0018E490(int index);
int func_00169688(int id, int value);
void func_0016A908(int id, const char *key);

/* Sets the loop flag of the driver voice of the sound slot index, then starts the file fileKey
 * on that voice. A null slot (the lookup failed) does nothing. */
void soundSlotStartFile(int index, const char *fileKey)
{
    SoundSlot *slot;

    slot = func_0018E490(index);
    if (slot != 0) {
        func_00169688(slot->driver_id, 1);
        func_0016A908(slot->driver_id, fileKey);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018F288);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018F2D0);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018F3F8);

void func_00169938(int id, int value);

/* Pauses or resumes a sound slot's voice and records the level in its ramp. */
void soundSlotSetPause(int index, int value)
{
    SoundSlot *slot;

    slot = func_0018E490(index);
    if (slot != 0) {
        func_00169938(slot->driver_id, value);
        slot->ramp.set_level = (unsigned char) value;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018F4A0);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018F4E8);

/* The sound block at D_004C7038 (splat label; the slot records start at +0x630, the table the
 * lookup in main/tu019 reads). */
typedef struct SoundRecordBlock {
    unsigned char master_volume; /* +0x00: scales the level of slot 0 */
    unsigned char unmodeled_01[3];
    int current_song;            /* +0x04 */
    int driver_work;             /* +0x08 */
    unsigned char unmodeled_0C[4];
    int reset_word;              /* +0x10 */
    int table[279];              /* +0x14 */
    unsigned char unmodeled_470[0x534 - 0x470];
    int table_b[63];             /* +0x534 */
    SoundSlot records[4]; /* +0x630 */
} SoundRecordBlock;

extern SoundRecordBlock D_004C7038;
int func_00167CB8(int id);
void func_0018F4E8(int index);
void func_0018FC70(int index);

/* Runs the two per-slot hooks for every sound slot that has a work buffer. */
void func_0018F538(void)
{
    int index;

    for (index = 0; index < 4; index++) {
        if (D_004C7038.records[index].create.buffer != 0) {
            func_0018F4E8(index);
            func_0018FC70(index);
        }
    }
}

/* Applies the slot's current ramp level, through the volume curve, to its driver voice. */
int soundSlotApply(SoundSlot *slot)
{
    int driver_id;
    int value;

    driver_id = slot->driver_id;
    value = seVolumeCurve(slot->ramp.current);
    return ADXT_SetOutVol(driver_id, value);
}

int seVolumeRampSet(VolumeRamp *ramp, int level, int step);

/* The top of the sound level scale; it is also the divisor of the master volume. */
#define SOUND_LEVEL_MAX 127

/* The default level of a sound slot, a splat label of its own at D_004C703B. */
extern unsigned char D_004C703B;

/* Sets slot index's level and ramp step; a negative extra only reports whether the ramp is moving. */
int soundSlotSetVolume(int index, int level, int extra)
{
    SoundSlot *slot;

    slot = func_0018E490(index);
    if (slot == 0) {
        return 0;
    }
    if (extra < 0) {
        return slot->ramp.step != 0;
    }
    if (level < 0) {
        level = D_004C703B;
    }
    if (index == 0) {
        level = level * D_004C7038.master_volume / SOUND_LEVEL_MAX;
    }
    if (seVolumeRampSet(&slot->ramp, level <= SOUND_LEVEL_MAX ? level : SOUND_LEVEL_MAX, extra) >= 0) {
        soundSlotApply(slot);
    }
    return 1;
}

/* Returns the driver status of slot index's voice, or -1 when the slot is not open. */
int soundSlotStatus(int index)
{
    SoundSlot *slot;

    slot = func_0018E490(index);
    if (slot == 0) {
        return -1;
    }
    return func_00167CB8(slot->driver_id);
}

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018F6E0);

/* Calls the driver on slot index's voice with value 0; a slot that is not open returns 0. */
void func_0018F7B0(int index)
{
    SoundSlot *slot;

    slot = func_0018E490(index);
    if (slot == 0) {
        return;
    }
    func_00169688(slot->driver_id, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018F7E8);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018F820);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018F880);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018F8C0);

/* The same as soundSlotStartFile. */
void soundSlotStartKey(int index, const char *fileKey)
{
    soundSlotStartFile(index, fileKey);
}

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018F900);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", soundApplySettings);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018FAA8);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018FAC8);

int soundVSyncCallback(void)
{
    ADXM_ExecVint(0);
    /* The end of an INTC/DMAC handler (SDK eekernel.h ExitHandler): sync, then re-enable interrupts. */
    __asm__ volatile("sync\n\tei" ::: "memory");
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018FC70);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018FD18);

INCLUDE_ASM("asm/main/nonmatchings/sound_driver", func_0018FD58);

/* <<< CHUNK sound_driver */

/*
 * OV02 original TU 90: 0x00ab1918..0x00ab1a40 (8 functions)
 */
#include "common.h"

typedef int s32;

/* Sound object record; the bound unit pointer sits at 0xA0. */
typedef struct SeEmitter {
    unsigned char unk_00[0xA0];
    void *boundUnit;
} SeEmitter;

extern s32 gSeInitialized;

void seResetInitialized(void) {
    gSeInitialized = 0;
}

void soundSlotSystemInit();

void seInitOnce(void) {
    if (gSeInitialized == 0) {
        soundSlotSystemInit();
        gSeInitialized = 1;
    }
}

void unitSoundSlotAssign(void *, s32);
void seInitOnce();

void seBindObjectSound(SeEmitter *emitter, s32 soundId) {
    seInitOnce();
    unitSoundSlotAssign(emitter->boundUnit, soundId);
}

void soundSlotStateSet(s32);

void seSetSoundStateValue(s32 soundStateValue) {
    seInitOnce();
    soundSlotStateSet(soundStateValue);
}

s32 func_00A93148(s32);

void seSetStateIndex(s32 index) {
    seInitOnce();
    func_00A93148(index);
}

s32 func_00A92FC8();
void seResetInitialized();

s32 seReleaseIfReady(void) {
    s32 isReady;

    isReady = func_00A92FC8();
    if (isReady != 0) {
        seResetInitialized();
        isReady = 1;
    }
    return isReady;
}

void seVoiceStartDefault();

void seStartDefaultVoice(void) {
    seVoiceStartDefault();
}

void seNoOpCallback(void) {

}

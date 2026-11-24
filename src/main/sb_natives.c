#include "common.h"
#include "shared.h"
#include "main/tu105.h"
#include "main/xgl_2.h"
#include "quadword.h"

/* Messages in a thread's queue (sbQueueReset: first 0, last 0xF). */
#define SB_QUEUE_LAST 0xF

struct SbThread;

void sbFatal(struct SbThread *thread);

void sbFatalReport(struct SbThread *thread, int pc);

/* The argument count of the current call: the word below the stack top. */
int VM_PLUGIN_CNT_GET(SbThread *thread)
{
    return thread->stack[thread->sp - 1];
}

/* Returns the word of argument argumentNumber (1-based) of the current call. */
int vmArgGet(SbThread *thread, int argumentNumber)
{
    int *frameTop;
    int argumentCount;

    frameTop = thread->stack + thread->sp;
    argumentCount = frameTop[-1];
    if (argumentCount < argumentNumber || argumentNumber == 0) {
        sbFatal(thread);
    }
    return frameTop[-2 - (argumentCount - argumentNumber)];
}

extern f32 gSbArgFloatScale;

/* Returns argument argumentNumber (1-based) of the current call as a float, scaled by gSbArgFloatScale. */
f32 vmArgFloatGet(SbThread *thread, int argumentNumber)
{
    return vmArgGet(thread, argumentNumber) * gSbArgFloatScale;
}

/* Returns the tag byte of argument argumentNumber (1-based) of the current call. */
int vmArgTagGet(SbThread *thread, int argumentNumber)
{
    int *frameTop;
    signed char *tagTop;
    int argumentCount;

    frameTop = thread->stack + thread->sp;
    tagTop = thread->tags + thread->sp;
    argumentCount = frameTop[-1];
    if (argumentCount < argumentNumber || argumentNumber == 0) {
        sbFatal(thread);
    }
    return tagTop[-2 - (argumentCount - argumentNumber)];
}

/*
 * Reports whether argument argumentNumber has a zero tag. An index past the frame's argument count returns 1.
 */
int vmArgTagIsZero(SbThread *thread, int argumentNumber)
{
    int *frameTop;
    signed char *tagTop;
    int argumentCount;

    frameTop = thread->stack + thread->sp;
    tagTop = thread->tags + thread->sp;
    if (argumentNumber == 0) {
        sbFatal(thread);
    }
    argumentCount = frameTop[-1];
    if (!(argumentCount >= argumentNumber)) {
        return 1;
    }
    return tagTop[-2 - (argumentCount - argumentNumber)] == 0;
}

/* Returns argument argumentNumber of the current call when its tag is nonzero, else fallbackValue. */
int vmArgGetOrDefault(SbThread *thread, int argumentNumber, int fallbackValue)
{
    int argumentValue;

    argumentValue = fallbackValue;
    if (vmArgTagIsZero(thread, argumentNumber) == 0) {
        argumentValue = vmArgGet(thread, argumentNumber);
    }
    return argumentValue;
}

/* Float form of vmArgGetOrDefault: argument argumentNumber as a scaled float when its tag is nonzero, else fallbackValue. */
f32 vmArgFloatGetOrDefault(SbThread *thread, int argumentNumber, f32 fallbackValue)
{
    f32 argumentValue;

    argumentValue = fallbackValue;
    if (vmArgTagIsZero(thread, argumentNumber) == 0) {
        argumentValue = vmArgFloatGet(thread, argumentNumber);
    }
    return argumentValue;
}

/* A keyed table: the words of its index array start keyIndexOffset bytes from the table, count entries long; the
 * entry data follows the count index words. */
struct SbKeyTable {
    int keyIndexOffset;                   /* +0x00 */
    int keyCount;                          /* +0x04 */
};

/* Table of the image's keys (a leaf in main/tu105). */
struct SbKeyTable *sbKeyTable(struct SbImageHeader *image);

/* Address of the keyed entry `key`, or 0 when key is past the table. */
char *vmKeyedEntryGet(SbThread *thread, int key)
{
    struct SbKeyTable *keyTable;
    char *keyIndexArray;

    keyTable = sbKeyTable(thread->image);
    if (!(key < keyTable->keyCount)) {
        return 0;
    }
    keyIndexArray = (char *)keyTable + keyTable->keyIndexOffset;
    return keyIndexArray + keyTable->keyCount * 4 + ((int *)keyIndexArray)[key];
}

/* Address of entry `index` of the library image linked into the thread's image (image->keys): the entry
 * count is at the header's +8, the name index array at its name_table offset, the entry data at the
 * offset stored at +0x18 (a field the SbImageHeader struct does not name yet). */
void *sbLibraryEntryGet(SbThread *thread, int libraryEntryIndex)
{
    struct SbImageHeader *library;
    char *nameIndexArray;

    library = thread->image->keys;
    if (libraryEntryIndex >= library->entry) {
        return 0;
    }
    nameIndexArray = (char *) library + library->name_table;
    return (char *) library + ((int *) library)[0x18 / 4] + ((int *) nameIndexArray)[libraryEntryIndex];
}

/* Base of the image's global table (a leaf in main/tu105). */
int *sbGlobalSection(struct SbImageHeader *image);

/* Address of global word `index` of the thread's image, as vmEntryGet computes it. */
int sbGlobalEntryGet(SbThread *thread, int globalWordNumber)
{
    int globalSectionAddress;

    globalSectionAddress = (int)sbGlobalSection(thread->image);
    return globalSectionAddress + sbGlobalSection(thread->image)[0] + globalWordNumber * 4;
}

/* Address of entry word entryWordNumber of the global section entry sectionWordNumber (each word is 4 bytes). */
int sbGlobalSectionWordAddress(SbThread *thread, int sectionWordNumber, int entryWordNumber)
{
    int globalSectionBase;
    int *indexedGlobalEntry;

    globalSectionBase = (int) sbGlobalSection(thread->image);
    globalSectionBase += sbGlobalSection(thread->image)[0];
    indexedGlobalEntry = (int *) (globalSectionBase + sectionWordNumber * 4);
    return globalSectionBase + indexedGlobalEntry[entryWordNumber] * 4;
}

/* Empty in this build: sbFatal passes the thread and its pc, as for a debug report. */
void sbFatalReport(SbThread *thread, int pc)
{
}

extern unsigned char gSbStudioBlock[];

void cameraTablesReset();

void cameraPendingClear();

void cameraSlotsReset();

struct XglCamera *xglStudioGetCamera2(int camera_index);

/* The block at gSbStudioBlock; its 64-bit word at +0x10 has bit 0 cleared by sbStudioCameraReset. */
typedef struct SbStudioBlock {
    unsigned char unmodeled_00[0x10];
    long long studioFlags; /* +0x10 */
} SbStudioBlock;

/* The status block at gSbStudioBlock as an lvalue. */
#define SB_STUDIO_BLOCK (*(SbStudioBlock *) gSbStudioBlock)

void sbStudioCameraReset(void)
{
    long long studioFlags;

    xglStudioGetCamera2(0)->cameraState = 4;
    cameraTablesReset();
    cameraSlotsReset();
    cameraPendingClear();
    studioFlags = SB_STUDIO_BLOCK.studioFlags & (~1);
    SB_STUDIO_BLOCK.studioFlags = studioFlags;
}

extern signed char D_003E0173[];

void sbStudioCameraResetIfFlagged(void) {
    if (D_003E0173[0] == 1) {
        cameraTablesReset();
        cameraSlotsReset();
        cameraPendingClear();
    }
}

void func_0025C240(void);

static inline int studioFlagBit(unsigned long long studioFlags, int flagBit)
{
    return (studioFlags >> flagBit) & 1;
}

/* Studio camera step: flag bit 5 selects the held camera (state 2, travel cleared on entry) or the stepped one. */
void sbStudioCameraStep(void)
{
    struct XglCamera *camera;

    camera = xglStudioGetCamera2(0);
    if (!studioFlagBit(SB_STUDIO_BLOCK.studioFlags, 5)) {
        if (camera->cameraState != 2) {
            QuadwordZero(&camera->travel.place_velocity);
            QuadwordZero(&camera->travel.place_delta);
            QuadwordZero(&camera->travel.angle_velocity);
            QuadwordZero(&camera->travel.angle_delta);
            camera->travel.fov_velocity = camera->travel.scale = 0.0f;
        }
        camera->cameraState = 2;
    } else {
        if (studioFlagBit(SB_STUDIO_BLOCK.studioFlags, 6)) {
            camera->cameraState = 4;
        }
        func_0025C240();
        if (studioFlagBit(SB_STUDIO_BLOCK.studioFlags, 6)) {
            camera->cameraState = 2;
        }
    }
}

typedef int s32;

/* EE scratchpad: the VM natives below build their command packets here. */
#define SCRATCHPAD_BASE 0x70000000

#define SCRATCHPAD_WORDS ((s32 *) SCRATCHPAD_BASE)

s32 vmArgGet();

s32 scratchpadSendCmd1F();

void scratchpadSendCmd0A();

void scratchpadSendCmd27Mode4();

void func_001E82E8();

void scratchpadSendMode0();

void xglRenderGlobalFadeValueSet(s32);

void slotReleaseFixedSlots();

void func_001E8550();

void sbNativeOwnerAdd(s32 *, s32 *);

extern s32 D_00407D00[];

extern s32 D_00407D48[];

extern s32 D_0053E418[];

extern s32 D_0053E420[];

/* VM native: word 1 is 2, words 0 and 2 are VM args; sent with cmd 0x1F. */
s32 vmSpadSubmitTypedPair(s32 vm) {
    s32 packetFirstWord;
    s32 packetThirdWord;

    packetFirstWord = vmArgGet(vm, 1);
    SCRATCHPAD_WORDS[1] = 2;
    SCRATCHPAD_WORDS[0] = packetFirstWord;
    packetThirdWord = vmArgGet(vm, 2);
    SCRATCHPAD_WORDS[2] = packetThirdWord;
    scratchpadSendCmd1F(SCRATCHPAD_BASE);
    return 1;
}

/* VM native: word 1 is 2, word 0 is one VM arg; sent with cmd 0x0A. */
s32 vmSpadSubmitTypedOne(s32 vm) {
    s32 packetFirstWord;

    packetFirstWord = vmArgGet(vm, 1);
    SCRATCHPAD_WORDS[0] = packetFirstWord;
    SCRATCHPAD_WORDS[1] = 2;
    scratchpadSendCmd0A(SCRATCHPAD_BASE);
    return 1;
}

/* VM native: five VM args as packet words 0-4, sent through scratchpadSendCmd27Mode4. */
s32 vmSpadSubmitFiveWords(s32 vm) {
    s32 packetFirstWord;
    s32 packetSecondWord;
    s32 packetThirdWord;
    s32 packetFourthWord;
    s32 packetFifthWord;

    packetFirstWord = vmArgGet(vm, 1);
    SCRATCHPAD_WORDS[0] = packetFirstWord;
    packetSecondWord = vmArgGet(vm, 2);
    SCRATCHPAD_WORDS[1] = packetSecondWord;
    packetThirdWord = vmArgGet(vm, 3);
    SCRATCHPAD_WORDS[2] = packetThirdWord;
    packetFourthWord = vmArgGet(vm, 4);
    SCRATCHPAD_WORDS[3] = packetFourthWord;
    packetFifthWord = vmArgGet(vm, 5);
    SCRATCHPAD_WORDS[4] = packetFifthWord;
    scratchpadSendCmd27Mode4(SCRATCHPAD_BASE);
    return 1;
}

/* Same five-word packet as vmSpadSubmitFiveWords, sent through func_001E82E8. */
s32 vmSpadSubmitFiveWordsAlt(s32 vm) {
    s32 packetFirstWord;
    s32 packetSecondWord;
    s32 packetThirdWord;
    s32 packetFourthWord;
    s32 packetFifthWord;

    packetFirstWord = vmArgGet(vm, 1);
    SCRATCHPAD_WORDS[0] = packetFirstWord;
    packetSecondWord = vmArgGet(vm, 2);
    SCRATCHPAD_WORDS[1] = packetSecondWord;
    packetThirdWord = vmArgGet(vm, 3);
    SCRATCHPAD_WORDS[2] = packetThirdWord;
    packetFourthWord = vmArgGet(vm, 4);
    SCRATCHPAD_WORDS[3] = packetFourthWord;
    packetFifthWord = vmArgGet(vm, 5);
    SCRATCHPAD_WORDS[4] = packetFifthWord;
    func_001E82E8(SCRATCHPAD_BASE);
    return 1;
}

/* VM native: calls slotReleaseFixedSlots and sets the global fade value to 0. */
s32 vmSpadFlush(void) {
    slotReleaseFixedSlots();
    xglRenderGlobalFadeValueSet(0);
    return 1;
}

/* VM native: five VM args as packet words 0-4, command 0x49 (9 words). */
s32 vmSpadCommand49(s32 vm) {
    s32 packetFirstWord;
    s32 packetSecondWord;
    s32 packetThirdWord;
    s32 packetFourthWord;
    s32 packetFifthWord;

    packetFirstWord = vmArgGet(vm, 1);
    SCRATCHPAD_WORDS[0] = packetFirstWord;
    packetSecondWord = vmArgGet(vm, 2);
    SCRATCHPAD_WORDS[1] = packetSecondWord;
    packetThirdWord = vmArgGet(vm, 3);
    SCRATCHPAD_WORDS[2] = packetThirdWord;
    packetFourthWord = vmArgGet(vm, 4);
    SCRATCHPAD_WORDS[3] = packetFourthWord;
    packetFifthWord = vmArgGet(vm, 5);
    SCRATCHPAD_WORDS[4] = packetFifthWord;
    scratchpadSendMode0(0x49, 9, SCRATCHPAD_BASE);
    return 1;
}

XglCamera *xglStudioSelectGetActiveCamera(int studio_index);

/* VM native: builds a 6-word packet (command 0x46) from the active camera window: word 0 is 0xffffff, words 1-2
   the window centre, words 3-4 its size, then three VM args. */
s32 func_001D47A0(s32 vm) {
    XglCamera *camera;
    s32 left;
    s32 top;
    s32 width;
    s32 height;
    s32 packetFirstWord;
    s32 packetSecondWord;
    s32 packetThirdWord;

    camera = xglStudioSelectGetActiveCamera(0);
    left = (s32) camera->screen.window.component[0];
    top = (s32) camera->screen.window.component[1];
    width = (s32) camera->screen.window.component[2] - left;
    height = (s32) camera->screen.window.component[3] - top;
    SCRATCHPAD_WORDS[0] = 0xffffff;
    SCRATCHPAD_WORDS[1] = left + (width >> 1);
    SCRATCHPAD_WORDS[2] = top + (height >> 1);
    SCRATCHPAD_WORDS[3] = width;
    SCRATCHPAD_WORDS[4] = height;
    packetFirstWord = vmArgGet(vm, 1);
    SCRATCHPAD_WORDS[5] = packetFirstWord;
    packetSecondWord = vmArgGet(vm, 2);
    SCRATCHPAD_WORDS[6] = packetSecondWord;
    packetThirdWord = vmArgGet(vm, 3);
    SCRATCHPAD_WORDS[7] = packetThirdWord;
    scratchpadSendMode0(0x46, 6, SCRATCHPAD_BASE);
    return 1;
}

/* VM native: five VM args as packet words 0-4, command 0x5D (0x1D words). */
s32 vmSpadCommand5D(s32 vm) {
    s32 packetFirstWord;
    s32 packetSecondWord;
    s32 packetThirdWord;
    s32 packetFourthWord;
    s32 packetFifthWord;

    packetFirstWord = vmArgGet(vm, 1);
    SCRATCHPAD_WORDS[0] = packetFirstWord;
    packetSecondWord = vmArgGet(vm, 2);
    SCRATCHPAD_WORDS[1] = packetSecondWord;
    packetThirdWord = vmArgGet(vm, 3);
    SCRATCHPAD_WORDS[2] = packetThirdWord;
    packetFourthWord = vmArgGet(vm, 4);
    SCRATCHPAD_WORDS[3] = packetFourthWord;
    packetFifthWord = vmArgGet(vm, 5);
    SCRATCHPAD_WORDS[4] = packetFifthWord;
    scratchpadSendMode0(0x5D, 0x1D, SCRATCHPAD_BASE);
    return 1;
}

/* VM native: six VM args as packet words 0-5, words 6-8 zeroed, command 0x60 (0x20 words). */
s32 vmSpadCommand60(s32 vm) {
    s32 packetFirstWord;
    s32 packetSecondWord;
    s32 packetThirdWord;
    s32 packetFourthWord;
    s32 packetFifthWord;
    s32 packetSixthWord;

    packetFirstWord = vmArgGet(vm, 1);
    SCRATCHPAD_WORDS[0] = packetFirstWord;
    packetSecondWord = vmArgGet(vm, 2);
    SCRATCHPAD_WORDS[1] = packetSecondWord;
    packetThirdWord = vmArgGet(vm, 3);
    SCRATCHPAD_WORDS[2] = packetThirdWord;
    packetFourthWord = vmArgGet(vm, 4);
    SCRATCHPAD_WORDS[3] = packetFourthWord;
    packetFifthWord = vmArgGet(vm, 5);
    SCRATCHPAD_WORDS[4] = packetFifthWord;
    packetSixthWord = vmArgGet(vm, 6);
    SCRATCHPAD_WORDS[5] = packetSixthWord;
    SCRATCHPAD_WORDS[6] = 0;
    SCRATCHPAD_WORDS[7] = 0;
    SCRATCHPAD_WORDS[8] = 0;
    scratchpadSendMode0(0x60, 0x20, SCRATCHPAD_BASE);
    return 1;
}

/* VM native: command 0x60 packet from the active camera window: five VM args, with words 4-6 offset by 8 and by the
   window's half width and half height (the offsets are in words 4, 5 and 6). */
s32 func_001D49E8(s32 vm) {
    XglCamera *camera;
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    s32 width;
    s32 height;
    s32 packetFirstWord;
    s32 packetSecondWord;
    s32 packetThirdWord;
    s32 packetFourthWord;
    s32 packetFifthWord;
    s32 packetSixthWord;
    s32 packetSeventhWord;
    s32 packetEighthWord;
    s32 packetNinthWord;

    camera = xglStudioSelectGetActiveCamera(0);
    left = (s32) camera->screen.window.component[0];
    top = (s32) camera->screen.window.component[1];
    right = (s32) camera->screen.window.component[2];
    bottom = (s32) camera->screen.window.component[3];
    packetFirstWord = vmArgGet(vm, 1);
    SCRATCHPAD_WORDS[0] = packetFirstWord;
    packetSecondWord = vmArgGet(vm, 2);
    SCRATCHPAD_WORDS[1] = packetSecondWord;
    packetThirdWord = vmArgGet(vm, 3);
    SCRATCHPAD_WORDS[2] = packetThirdWord;
    packetFourthWord = vmArgGet(vm, 4);
    SCRATCHPAD_WORDS[3] = packetFourthWord;
    packetFifthWord = vmArgGet(vm, 5);
    width = right - left;
    SCRATCHPAD_WORDS[4] = packetFifthWord + 8;
    packetSixthWord = vmArgGet(vm, 6);
    height = bottom - top;
    SCRATCHPAD_WORDS[5] = packetSixthWord + width / 2;
    packetSeventhWord = vmArgGet(vm, 7);
    SCRATCHPAD_WORDS[6] = packetSeventhWord + height / 2;
    packetEighthWord = vmArgGet(vm, 8);
    SCRATCHPAD_WORDS[7] = packetEighthWord;
    packetNinthWord = vmArgGet(vm, 9);
    SCRATCHPAD_WORDS[8] = packetNinthWord;
    scratchpadSendMode0(0x60, 0x20, SCRATCHPAD_BASE);
    return 1;
}

/* VM native: four VM args as packet words 0-3, command 0x64 (0x24 words). */
s32 vmSpadCommand64(s32 vm) {
    s32 packetFirstWord;
    s32 packetSecondWord;
    s32 packetThirdWord;
    s32 packetFourthWord;

    packetFirstWord = vmArgGet(vm, 1);
    SCRATCHPAD_WORDS[0] = packetFirstWord;
    packetSecondWord = vmArgGet(vm, 2);
    SCRATCHPAD_WORDS[1] = packetSecondWord;
    packetThirdWord = vmArgGet(vm, 3);
    SCRATCHPAD_WORDS[2] = packetThirdWord;
    packetFourthWord = vmArgGet(vm, 4);
    SCRATCHPAD_WORDS[3] = packetFourthWord;
    scratchpadSendMode0(0x64, 0x24, SCRATCHPAD_BASE);
    return 1;
}

/* VM native: calls func_001E8550 (meaning not yet identified) and returns 1. */
s32 vmSpadPoll(void) {
    func_001E8550();
    return 1;
}

/* Registers the two native owners (their data addresses) with sbNativeOwnerAdd. */
void sbNativeOwnersRegister(void) {
    sbNativeOwnerAdd(D_0053E418, D_00407D00);
    sbNativeOwnerAdd(D_0053E420, D_00407D48);
}

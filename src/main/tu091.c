#include "common.h"
#include "shared.h"
#include "main/tu105.h"
/* Messages in a thread's queue (sbQueueReset: first 0, last 0xF). */
#define SB_QUEUE_LAST 0xF

void sbSlotJumpRequest(SbThread *thread, int key);
struct SbThread;
int VM_PLUGIN_CNT_GET(struct SbThread *thread);
int vmArgGet(struct SbThread *thread, int vmArgPosition);
int Get_Rnd_001B8CD0(int min, int max);
int mathRandom(SbThread *thread);
extern u16 gMovieAutoSelectFlags[];

int scriptMovieAutoSelectFlagGet(SbThread *thread) {
    int key;
    int autoSelectFlags;

    autoSelectFlags = 0;
    if (VM_PLUGIN_CNT_GET(thread) != 1) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    key = vmArgGet(thread, 1);
    switch (key) {
    case 0:
        autoSelectFlags = gMovieAutoSelectFlags[0];
        break;
    case 1:
        autoSelectFlags = gMovieAutoSelectFlags[1];
        break;
    case 2:
        autoSelectFlags = gMovieAutoSelectFlags[2];
        break;
    }
    return autoSelectFlags;
}

extern signed char gMovieFlagBytes[];

int scriptMovieFlagByteGet(SbThread *thread) {
    int key;
    int movieFlagValue;

    movieFlagValue = 0;
    if (VM_PLUGIN_CNT_GET(thread) != 1) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    key = vmArgGet(thread, 1);
    switch (key) {
    case 0:
        movieFlagValue = gMovieFlagBytes[0];
        break;
    case 1:
        movieFlagValue = gMovieFlagBytes[1];
        break;
    case 2:
        movieFlagValue = gMovieFlagBytes[2];
        break;
    case 3:
        movieFlagValue = gMovieFlagBytes[3];
        break;
    }
    return movieFlagValue;
}

typedef signed char s8;

typedef int s32;

/* The argument block at gGameLoopRuntimeStorage: the two values sbDeferredCallStore stores (+0x10 and +0x12). */
typedef struct GameLoopRuntime {
    u8 unk_00[0x10];
    s16 firstVmArg;               /* +0x10 */
    s16 secondVmArg;              /* +0x12 */
} GameLoopRuntime;

extern GameLoopRuntime gGameLoopRuntimeStorage[];
s32 vmArgTagIsZero(SbThread *thread, s32 vmArgPosition);

s32 sbDeferredCallStore(SbThread *thread)
{
  s16 firstVmArg;
  s16 secondVmArg;
  s16 storedSecondVmArg;
    if ((VM_PLUGIN_CNT_GET(thread) <= 0) || (VM_PLUGIN_CNT_GET(thread) >= 3))
  {
    sbSlotJumpRequest(thread, 2);
    return 0;
  }
  firstVmArg = vmArgGet(thread, 1);
  if (vmArgTagIsZero(thread, 2) == 0)
  {
    secondVmArg = vmArgGet(thread, 2);
    storedSecondVmArg = secondVmArg;
  }
  else
  {
    storedSecondVmArg = -1;
  }
  gGameLoopRuntimeStorage->firstVmArg = firstVmArg;
  gGameLoopRuntimeStorage->secondVmArg = storedSecondVmArg;
  return 1;
}

s32 valueSlotChangeRequest(s16 first, s16 second);

s32 sbDeferredCallRun(void)
{
  GameLoopRuntime *deferredCall;
  s16 secondVmArg;
  deferredCall = gGameLoopRuntimeStorage;
  secondVmArg = deferredCall->secondVmArg;
  if (((deferredCall->firstVmArg) > 0) == 0U)
  {
    return 0;
  }
  valueSlotChangeRequest(deferredCall->firstVmArg, secondVmArg);
  return 1;
}

/* Script native "math.random": two arguments, min and max, passed to Get_Rnd. */
int mathRandom(SbThread *thread)
{
    int randomMin;
    int randomMax;

    if (VM_PLUGIN_CNT_GET(thread) != 2) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    randomMin = vmArgGet(thread, 1);
    randomMax = vmArgGet(thread, 2);
    return Get_Rnd_001B8CD0(randomMin, randomMax);
}

s32 func_001B8D08(s32 groupId, s32 entryId, s32 scoreBias);
s32 indexedTableGet(s32 groupId, s32 entryId);
s32 indexedCountGet(s32 groupId, s32 entryId);
s32 indexedValueGet(s32 entryIndex);
void movieSetFlagByte(s32 value, s32 movieDataValue, s32 scoreBias);
void movieDecoderReset(s32 value, s32 scoreBias);
void seVoiceStartDefault(unsigned int handle);

s32 func_001CD248(SbThread *thread) {
    s32 groupId;
    s32 entryId;
    s32 scoreBias;
    s32 groupStatus;
    s32 playsVoice;
    s32 random;
    s32 movieDataValue;
    s32 entryTally;
    s32 movieEntryValue;

    if ((VM_PLUGIN_CNT_GET(thread) < 3) || (VM_PLUGIN_CNT_GET(thread) >= 5)) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    groupId = vmArgGet(thread, 1);
    entryId = vmArgGet(thread, 2);
    scoreBias = vmArgGet(thread, 3);
    if (vmArgTagIsZero(thread, 4) == 0) {
        playsVoice = vmArgGet(thread, 4) != 0;
    } else {
        playsVoice = 1;
    }
    random = Get_Rnd_001B8CD0(groupId, entryId);
    groupStatus = func_001B8D08(groupId, entryId, random + scoreBias);
    movieDataValue = indexedTableGet(groupId, entryId);
    entryTally = indexedCountGet(groupId, entryId);
    movieEntryValue = indexedValueGet(entryTally);
    if (playsVoice != 0) {
        if (groupId == 10) {
            movieDecoderReset(movieEntryValue, scoreBias);
        } else {
            movieSetFlagByte(movieEntryValue, movieDataValue, scoreBias);
        }
        seVoiceStartDefault(12);
    }
    return groupStatus;
}

s32 func_001B5C38(s32, s32);

s32 mathArgsTest(SbThread *thread) {
    s32 firstVmArg;
    int secondVmArg;

    if (VM_PLUGIN_CNT_GET(thread) != 2) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    firstVmArg = vmArgGet(thread, 1);
    secondVmArg = vmArgGet(thread, 2);
    return func_001B5C38(firstVmArg, secondVmArg) != 0;
}

/* The dispModelRecord RgDispModelFindByKey returns (rg_dispmodel.c); this native stores its argument at +0x210, partial. */
typedef struct RgDispModelRecord {
    unsigned char unmodeled_00[0x210];
    s32 storedValue; /* +0x210 */
} RgDispModelRecord;

RgDispModelRecord *RgDispModelFindByKey(s32 handle);

s32 scriptDispModelValueStore(SbThread *thread) {
    RgDispModelRecord *dispModelRecord;
    s32 recordedValue;

    dispModelRecord = RgDispModelFindByKey(0);
    if (VM_PLUGIN_CNT_GET(thread) != 1) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    recordedValue = vmArgGet(thread, 1);
    if (RgDispModelFindByKey(recordedValue) == 0) {
        return 0;
    }
    dispModelRecord->storedValue = recordedValue;
    return 1;
}

void xenoSpawnerStartAim(f32 x, f32 y, f32 z);

static inline f32 argScaled(SbThread *thread, int vmArgPosition) {
    return (f32) vmArgGet(thread, vmArgPosition) * 0.01f;
}

s32 func_001CD490(SbThread *thread) {
    f32 x;
    f32 y;
    f32 z;

    if (VM_PLUGIN_CNT_GET(thread) != 3) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    x = argScaled(thread, 1);
    y = argScaled(thread, 2);
    z = argScaled(thread, 3);
    xenoSpawnerStartAim(x, y, z);
    return 1;
}

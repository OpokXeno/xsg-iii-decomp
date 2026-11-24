#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026BAC8);

typedef int s32;

void movieSlotsSetForOwner(s32 owner, s32 blendResetMode);

s32 vmArgGet(void *thread, s32 argIndex);

s32 sbNativeStopOwnedEntries(void *thread) {
    s32 ownerId;

    ownerId = vmArgGet(thread, 1);
    movieSlotsSetForOwner(ownerId, 0);
    return 1;
}

s32 func_0026C9A0(s32 ownerId);

s32 sbNativeQueryOwnedEntry(void *thread, s32 *outOwnedEntryFound) {
    s32 ownedEntryFound;
    s32 ownerId;

    ownerId = vmArgGet(thread, 1);
    ownedEntryFound = func_0026C9A0(ownerId);
    if (ownedEntryFound == 1) {
        *outOwnedEntryFound = ownedEntryFound;
    }
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026BC50);

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026BCD8);

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026BDA0);

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026BE00);

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026BE60);

typedef float f32;

s32 VM_PLUGIN_CNT_GET();

s32 func_0026CE08(s32, s32);

s32 vmArgGet();

s32 vmKeyedEntryGet();

s32 vmKeyedEntryTestFlag(s32 thread, s32 *testPassFlag) {
    s32 testOperand;
    s32 entryKey;
    s32 keyedEntry;

    testOperand = 0;
    if ((VM_PLUGIN_CNT_GET() <= 0) || (VM_PLUGIN_CNT_GET(thread) >= 3)) {
        return 0;
    }
    if (VM_PLUGIN_CNT_GET(thread) >= 2) {
        testOperand = vmArgGet(thread, 2);
    }
    entryKey = vmArgGet(thread, 1);
    keyedEntry = vmKeyedEntryGet(thread, entryKey);
    if (func_0026CE08(keyedEntry, testOperand) != 0) {
        *testPassFlag = 1;
    }
    return 1;
}

s32 func_0026CE48(s32, s32);

s32 vmKeyedEntryTestFlagAlt(s32 thread, s32 *testPassFlag) {
    s32 testOperand;
    s32 entryKey;
    s32 keyedEntry;

    testOperand = 0;
    if ((VM_PLUGIN_CNT_GET() <= 0) || (VM_PLUGIN_CNT_GET(thread) >= 3)) {
        return 0;
    }
    if (VM_PLUGIN_CNT_GET(thread) >= 2) {
        testOperand = vmArgGet(thread, 2);
    }
    entryKey = vmArgGet(thread, 1);
    keyedEntry = vmKeyedEntryGet(thread, entryKey);
    if (func_0026CE48(keyedEntry, testOperand) != 0) {
        *testPassFlag = 1;
    }
    return 1;
}

void func_0026D0E8();

s32 vmKeyedEntryDispatch(s32 thread) {
    s32 entryKey;
    s32 keyedEntry;

    entryKey = vmArgGet(thread, 1);
    keyedEntry = vmKeyedEntryGet(thread, entryKey);
    func_0026D0E8(keyedEntry);
    return 1;
}

s32 func_0026D238();

s32 vmNativeCheckSetDone(s32 unused_arg0, s32 *doneFlag) {
    s32 nativeCheckResult;

    nativeCheckResult = func_0026D238();
    if (nativeCheckResult == 0) {
        *doneFlag = 1;
    }
    return nativeCheckResult;
}

void movieAuxSetC(f32, f32);

s32 vmArgSetFloatPair(s32 thread) {
    f32 movieAuxFieldC;
    s32 movieAuxFieldD;

    movieAuxFieldC = vmArgGet(thread, 1);
    movieAuxFieldD = vmArgGet(thread, 2);
    movieAuxSetC(movieAuxFieldC, movieAuxFieldD);
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026C1A8);

void movieSwitchSet();

s32 vmKeyedEntrySetSwitch(s32 thread) {
    s32 entryKey;
    s32 keyedEntry;

    entryKey = vmArgGet(thread, 1);
    keyedEntry = vmKeyedEntryGet(thread, entryKey);
    movieSwitchSet(keyedEntry);
    return 1;
}

s32 func_0026D7B8();

s32 msgPutWait(s32 unused_arg0, s32 *done) {
    if (func_0026D7B8() == 0) {
        *done = 1;
    }
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026C378);

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026C588);

typedef unsigned char u8;

typedef signed char s8;

/* The flag block at gGameLoopRuntimeStorage (the leading words of the game loop runtime block; include/main/tu118.h views the same words, that header is not included here). */
typedef struct GameLoopRuntime {
    s32 loopFlags;                          /* +0x00 */
} GameLoopRuntime;

/* A layer header: a flags word whose bit 0 enables the layer. */
typedef struct MovieLayer {
    s32 layerFlags;                          /* +0x00 */
} MovieLayer;

/* gMovieLayers: six layers at +0x000, +0x0B0, +0x110, +0x1D0, +0x280 and +0x360. */
typedef struct MovieLayerBlock {
    MovieLayer layer00;                 /* +0x000 */
    u8 unk_004[0xAC];
    MovieLayer layerB0;                 /* +0x0B0 */
    u8 unk_0B4[0x5C];
    MovieLayer layer110;                /* +0x110 */
    u8 unk_114[0xBC];
    MovieLayer layer1D0;                /* +0x1D0 */
    u8 unk_1D4[0xAC];
    MovieLayer layer280;                /* +0x280 */
    u8 unk_284[0xDC];
    MovieLayer layer360;                /* +0x360 */
} MovieLayerBlock;

/* gMovieOverlay: layers at +0x000 and +0x0B0; the sub-blocks at +0x160, +0x174 and +0x188 are passed by address. */
typedef struct MovieOverlayBlock {
    MovieLayer layer00;                 /* +0x000 */
    u8 unk_004[0xAC];
    MovieLayer layerB0;                 /* +0x0B0 */
    u8 unk_0B4[0xAC];
    u8 unk_160[0x14];                   /* +0x160 */
    u8 unk_174[0x14];                   /* +0x174 */
    u8 unk_188[0x14];                   /* +0x188: extent not measured */
} MovieOverlayBlock;

/* One 0xD0-byte slot of effectSlotTable (24 slots): an active word at 0x00, flags at 0x10 (bit 0 enables,
 * bit 2 gates the call on the threshold), a threshold float at 0x48 and an owner word at 0xC0. */
typedef struct MovieSlot {
    s32 active;                         /* +0x00 */
    u8 unk_04[0x0C];
    s32 tagFlags;                          /* +0x10 */
    u8 unk_14[0x34];
    f32 threshold;                      /* +0x48 */
    u8 unk_4C[0x74];
    s32 owner;                          /* +0xC0 */
    u8 unk_C4[0x0C];
} MovieSlot;

/* effectSlotTable: the 24 slots, then two bytes at +0x1398 (playFlag, cleared by moviePlaySetup) and +0x1399 (playChannel). */
typedef struct MovieSlotTable {
    MovieSlot effectSlots[24];
    u8 unk_1380[0x18];
    s8 playFlag;                        /* +0x1398 */
    s8 playChannel;                     /* +0x1399 */
} MovieSlotTable;

/* gMovieAux: three enable bytes and four floats (0x14 bytes). */
typedef struct MovieAuxBlock {
    u8 enabledA;                        /* +0x00 */
    u8 enabledB;                        /* +0x01 */
    u8 enabledC;                        /* +0x02 */
    u8 unk_03;                          /* +0x03 */
    f32 valueA;                         /* +0x04 */
    f32 valueB;                         /* +0x08 */
    f32 valueC;                         /* +0x0C */
    f32 valueD;                         /* +0x10 */
} MovieAuxBlock;

/* The block returned by stateBlockGet: a phase word at 0x314. */
typedef struct MovieStateBlock {
    u8 unk_000[0x314];
    s32 gameState;                          /* +0x314 */
} MovieStateBlock;

/* The request of movieCounterSetValue: a key at 0x18 and a value at 0x1C. */
typedef struct MovieCounterAssignment {
    u8 unk_00[0x18];
    s32 counterId;                            /* +0x18 */
    s32 counterValue;                          /* +0x1C */
} MovieCounterAssignment;

/* Typed views of the untyped data blobs (their externs stay unsized: a sized one changes the object). */
#define gMovieSlots (*(MovieSlotTable *) effectSlotTable)

#define gMovieLayers (*(MovieLayerBlock *) movieLayerBlock)

#define gMovieOverlay (*(MovieOverlayBlock *) movieOverlayBlock)

#define gMovieAux (*(MovieAuxBlock *) movieAuxBlock)

#define gGameLoopRuntime (*(GameLoopRuntime *) gGameLoopRuntimeStorage)

extern unsigned char D_003E4120[];

extern unsigned char D_003E4180[];

extern unsigned char D_00530F80[];

extern unsigned char D_00530F88[];

s32 sbNativeOwnerAdd();

void movieRegisterNatives(void) {
    sbNativeOwnerAdd(D_00530F80, D_003E4120);
    sbNativeOwnerAdd(D_00530F88, D_003E4180);
}

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026C620);

s32 func_0026DBF0();

void movieUpdateOverlay();

s32 func_0026DCF8();

void movieUpdateSlots();

s32 func_0026DFC0();

void movieUpdateAll(void) {
    func_0026DBF0();
    movieUpdateOverlay();
    func_0026DCF8();
    func_0026DFC0();
    movieUpdateSlots();
}

extern unsigned char movieSwitchTable[];

s32 func_0026E648();

void movieSwitchUpdate(void) {
    func_0026E648(movieSwitchTable);
}

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026C6B8);

s32 func_0026C638();

void movieLayerShutdown(void) {
    func_0026C638();
}

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026C788);

extern unsigned char effectSlotTable[];

s32 blendStateResetMode();

/* The slot fields read from entry = effectSlotTable + 0x10 + k * 0xD0 (the active word is at entry - 0x10). */
typedef struct MovieSlotView {
    s32 tagFlags;                          /* +0x00 (slot +0x10) */
    u8 unk_04[0x34];
    f32 threshold;                      /* +0x38 (slot +0x48) */
    u8 unk_3C[0x74];
    s32 owner;                          /* +0xB0 (slot +0xC0) */
    u8 unk_B4[0xC];
    s32 nextActive;                     /* +0xC0 (next slot +0x00) */
    u8 unk_C4[0xC];
} MovieSlotView;

void movieSlotsSetForOwner(s32 owner, s32 blendResetMode)
{
  s32 slotTagFlags;
  s32 effectSlotIndex;
  MovieSlot *effectSlot;
  MovieSlotView *effectSlotView;
  effectSlot = gMovieSlots.effectSlots;
  effectSlotView = (MovieSlotView *) &effectSlot->tagFlags;
  for (effectSlotIndex = 0; effectSlotIndex < 24; effectSlotIndex++)
  {
    if (effectSlotView[-1].nextActive != 0)
    {
      slotTagFlags = effectSlotView->tagFlags;
      if ((slotTagFlags & 1) && (owner == effectSlotView->owner))
      {
        if (!(slotTagFlags & 4) || !(effectSlotView->threshold < 1.0f))
        {
          blendStateResetMode(effectSlotView, blendResetMode);
        }
      }
    }
    effectSlotView++;
  }
}

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026C9A0);

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026CA08);

s32 func_0026CB30();

s32 moviePlaySetup(s32 path, s32 playMode, s32 channel, s32 playParam)
{
  gMovieSlots.playFlag = 0;
  gMovieSlots.playChannel = channel;
  return func_0026CB30(path, (playMode == 0) ? 0 : 0x10, playParam);
}

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026CB30);

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026CC68);

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026CD38);

extern unsigned char movieAuxBlock[];

void movieAuxSetA(f32 newValueA)
{
  f32 storedValueA;
  storedValueA = newValueA;
  gMovieAux.enabledA = 1;
  gMovieAux.valueA = storedValueA;
}

void movieAuxSetB(f32 newValueB)
{
  f32 storedValueB;
  storedValueB = newValueB;
  gMovieAux.enabledB = 1;
  gMovieAux.valueB = storedValueB;
}

void movieAuxSetC(f32 newValueC, f32 newValueD)
{
  f32 storedValueC;
  f32 storedValueD;
  storedValueC = newValueC;
  gMovieAux.enabledC = 1;
  storedValueD = newValueD;
  gMovieAux.valueC = storedValueC;
  gMovieAux.valueD = storedValueD;
}

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026CE08);

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026CE48);

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026CE88);

extern unsigned char D_00530FE0[];

extern unsigned char D_00530FF0[];

extern unsigned char D_00531028[];

extern unsigned char moviePathBuffer[];

s32 GameMpeg2Play();

s32 func_00213700();

s32 func_0026CD38();

void moviePlayLocalized(s32 movieId, s32 variant) {
    s32 firstTextId;
    s32 secondTextId;

    firstTextId = func_00213700(0x17);
    secondTextId = func_00213700(0x18);
    GameMpeg2Play(moviePathBuffer, D_00531028, D_00530FE0, movieId, firstTextId, D_00530FF0, variant, secondTextId);
    func_0026CD38(moviePathBuffer);
}

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026D0E8);

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026D238);

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026D320);

s32 func_0026E530(s32);

s32 setSwitchState(void *, s32);

void movieSwitchSet(s32 id) {
    void *switchTable;
    s32 switchIndex;

    if (id != 0) {
        switchTable = movieSwitchTable;
        switchIndex = func_0026E530(id);
        setSwitchState(switchTable, switchIndex);
        return;
    }
    setSwitchState(movieSwitchTable, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026D7B8);

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026D898);

s32 memset();

void movieAuxClear(void) {
    memset(&gMovieAux, 0, 0x14);
}

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026DBF0);

extern unsigned char gGameLoopRuntimeStorage[];

extern unsigned char movieOverlayBlock[];

s32 func_00268B10();

s32 func_0026E5C8();

void movieUpdateOverlay(void) {
    if (!(gGameLoopRuntime.loopFlags & 0x4000)) {
        if (gMovieOverlay.layer00.layerFlags & 1) {
            func_00268B10(gMovieOverlay.unk_160);
        }
        if (gMovieOverlay.layerB0.layerFlags & 1) {
            func_00268B10(gMovieOverlay.unk_174);
        }
        func_0026E5C8(gMovieOverlay.unk_188);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026DCF8);

extern unsigned char movieLayerBlock[];

s32 func_00268070();

s32 effectSlotRelease();

s32 func_0026A2E0();

s32 movieLayerUpdate();

void movieUpdateSlots(void) {
    s32 index;
    MovieSlot *slot;
    MovieLayerBlock *layerBlock;
    MovieLayer *layer;

    slot = gMovieSlots.effectSlots;
    for (index = 0; index < 24; index++) {
        if (slot->active != 0) {
            func_00268070(slot);
            if (!(slot->tagFlags & 1)) {
                effectSlotRelease(slot);
            }
        }
        slot++;
    }
    layerBlock = &gMovieLayers;
    if (layerBlock->layer00.layerFlags & 1) {
        func_0026A2E0(layerBlock);
    }
    layer = &layerBlock->layerB0;
    if (layer->layerFlags & 1) {
        movieLayerUpdate(layer);
    }
    layer = &layerBlock->layer110;
    if (layer->layerFlags & 1) {
        func_0026A2E0(layer);
    }
    layer = &layerBlock->layer1D0;
    if (layer->layerFlags & 1) {
        func_0026A2E0(layer);
    }
    layer = &layerBlock->layer280;
    if (layer->layerFlags & 1) {
        func_0026A2E0(layer);
    }
    layer = &layerBlock->layer360;
    if (layer->layerFlags & 1) {
        func_0026A2E0(layer);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026DFC0);

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026E070);

s32 movieSefFlagTest800(void)
{
  if ((gGameLoopRuntime.loopFlags & 0x800) != 0)
  {
    return 1;
  }
  return 0;
}

MovieStateBlock *stateBlockGet();

s32 movieStatePhaseIs3(void) {
    return stateBlockGet()->gameState == 3;
}

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026E0F0);

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026E220);

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026E250);

extern unsigned char D_004B3787[];

u8 movieModeGet(void) {
    return D_004B3787[0];
}

void movieModeSet(s32 newMovieMode) {
    D_004B3787[0] = newMovieMode;
}

/* One 0xC-byte slot of the table movieCounterSlots: an active flag at 0x0 and a use counter at 0x8. */
typedef struct MovieCounterSlot {
    s32 active;
    u8 unk_04[4];
    s8 useCount;
} MovieCounterSlot;

extern MovieCounterSlot movieCounterSlots[];

void movieCounterSlotsClear(void) {
    memset(movieCounterSlots, 0, 0x6C);
}

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026E2D0);

extern s32 movieCounterValues[];

s32 func_0026E410();

void movieCounterSetValue(MovieCounterAssignment *counterAssignment)
{
  s32 counterRow;
  counterRow = func_0026E410(counterAssignment->counterId);
  movieCounterValues[counterRow * 3] = counterAssignment->counterValue;
}

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026E410);

void movieCounterTick(void) {
    s32 movieCounterIndex;
    MovieCounterSlot *movieCounterSlot;

    movieCounterSlot = movieCounterSlots;
    for (movieCounterIndex = 0; movieCounterIndex < 9; movieCounterIndex++) {
        if (movieCounterSlot->active != 0) {
            movieCounterSlot->useCount += 1;
        }
        movieCounterSlot++;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026E490);

s32 movieCounterFindMostUsed(void)
{
  s32 movieCounterIndex;
  s32 bestIndex;
  s32 slotUseCount;
  s32 bestCount;
  MovieCounterSlot *movieCounterSlot;
  bestIndex = 0;
  movieCounterIndex = 1;
  bestCount = movieCounterSlots[0].useCount;
  movieCounterSlot = &movieCounterSlots[1];
  do
  {
    slotUseCount = movieCounterSlot->useCount;
    movieCounterSlot++;
    if (bestCount < slotUseCount)
    {
      bestCount = slotUseCount;
      bestIndex = movieCounterIndex;
    }
    movieCounterIndex += 1;
  }
  while (movieCounterIndex < 9);
  return bestIndex;
}

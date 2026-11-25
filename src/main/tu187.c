#include "common.h"

typedef unsigned char u8;

typedef int s32;

typedef signed char s8;

typedef unsigned short u16;

typedef struct SeTableBlock {
    u8 unk000[0x200];
    u8 cells[9];
    u8 unk209[7];
    u8 tailA;
    u8 tailB;
    u16 tailC;
} SeTableBlock;

u8 D_00550BE0 = 0;
u8 gSeVoiceCallbackFlag = 0;

/* Clears a SeTableBlock: its nine cells, the tail bytes and D_00550BE0. The second cell view starts 8 bytes in. */
void seMenuBlockClear(SeTableBlock *cellBlock) {
    SeTableBlock *shifted;
    s32 cellIndex;

    shifted = (SeTableBlock *) ((u8 *) cellBlock + 8);
    for (cellIndex = 0; cellIndex < 9; cellIndex++) {
        cellBlock->cells[cellIndex] = 0;
        shifted->cells[cellIndex] = 0;
    }
    cellBlock->tailA = 0;
    cellBlock->tailC = 0;
    cellBlock->tailB = 0;
    D_00550BE0 = 0;
}

typedef struct VoiceKey {
    u16 flags;
} VoiceKey;

typedef struct VoiceTable {
    u16 cursor;
    u16 filledEntryCount;
    u16 unk04;
    u16 unk06;
    u8 unk08[0x18];
    s32 unk20;
    void **entryPointers;
    VoiceKey *entryKeys;
    u8 unk2C[0x8];
    u16 capacity;
    u16 topRow;
    u8 unk38[0x4];
} VoiceTable;

typedef struct SeVoiceTailBlock {
    const char *title; /* 0x00: messageTextGet(84) */
    u8 unk04[0x8];
    u8 unk0C;
    u8 unk0D;
    u8 unk0E;
    u8 unk0F;
    u16 unk10;
    u16 unk12;
    u16 unk14;
    u16 unk16;
    u8 unk18[0x8];
    s32 unk20;
    s32 unk24;
    s32 unk28;
    u8 unk2C[0xC];
} SeVoiceTailBlock;

typedef unsigned long u64;

typedef union BitRows {
    u64 words[27];
    u8 rows[9][24];
} BitRows;

/* The voice screen state at gSeVoiceState (0x9D0 bytes). */
typedef struct SeVoiceState {
    u8 unk000[0x340];
    VoiceTable unk340;       /* +0x340 */
    VoiceTable voiceKind;    /* +0x37C */
    VoiceTable voiceTable;   /* +0x3B8 */
    VoiceTable unk3F4;       /* +0x3F4 */
    SeVoiceTailBlock unk430; /* +0x430 */
    u8 unk468[0xE8];
    VoiceTable unk550; /* +0x550 */
    u8 unk58C[8];
    VoiceTable unk594; /* +0x594 */
    u8 unk5D0[0x2A];
    u16 unk5FA;
    u8 unk5FC[0x6C];
    u8 unk668[0x100];
    u8 cdPath[0x180]; /* +0x768 */
    BitRows bitRows;  /* +0x8E8 */
    u8 scene;         /* +0x9C0 */
    u8 unk9C1;
    s8 voiceFlags;  /* +0x9C2 */
    u8 bitRowIndex; /* +0x9C3 */
    u8 stateFlag;   /* +0x9C4 */
    u8 matchCount;  /* +0x9C5 */
    u8 unk9C6[2];
    u16 unk9C8;
    u8 unk9CA[6];
} SeVoiceState;

static SeVoiceState *gSeVoiceState;
s32 slotKindResolve(s32);
s32 func_00222C60(s32, s32);
s32 pagedTableGetCurrentEntry();
u8 *func_00A843F8(s32 index);

s32 seVoiceLookupSlot(void) {
    s32 resolvedSlot;
    s32 currentVoiceSlot;
    s32 voiceSlot;
    s32 keyedVoiceSlot;
    s32 voiceKindEntry;
    s32 lookupKey;

    voiceSlot = 0;
    if (gSeVoiceState->voiceTable.filledEntryCount != 0) {
        currentVoiceSlot = pagedTableGetCurrentEntry(&gSeVoiceState->voiceTable, 0);
        if ((gSeVoiceState->voiceKind.filledEntryCount != 0) && (gSeVoiceState->voiceFlags & 2)) {
            voiceKindEntry = pagedTableGetCurrentEntry(&gSeVoiceState->voiceKind);
            resolvedSlot = slotKindResolve(voiceKindEntry);
            lookupKey = pagedTableGetCurrentEntry(&gSeVoiceState->voiceTable);
            keyedVoiceSlot = func_00222C60(resolvedSlot, lookupKey);
        } else {
            keyedVoiceSlot = func_00A843F8(currentVoiceSlot);
        }
        voiceSlot = keyedVoiceSlot;
    }
    return voiceSlot;
}

s32 pagedTableMoveCursor(void *table, s32 step);

/* Moves the unk3F4 cursor one entry per step until it stands on one of the five kinds 8..12; returns whether it does. */
s32 seVoiceKindCursorSeek(s32 stepBackward) {
    s32 relativeKind;

    relativeKind = pagedTableGetCurrentEntry(&gSeVoiceState->unk3F4) - 8;
    while ((unsigned int) relativeKind < 5) {
        if (stepBackward != 0) {
            pagedTableMoveCursor(&gSeVoiceState->unk3F4, -1);
        } else {
            pagedTableMoveCursor(&gSeVoiceState->unk3F4, 1);
        }
        relativeKind = pagedTableGetCurrentEntry(&gSeVoiceState->unk3F4) - 8;
    }
    return (unsigned int) relativeKind < 5;
}

s32 pagedTableGetEntryAt(VoiceTable *table, s32 entryIndex);

/* Sets flag 0x100 on every unk3F4 entry whose kind is one of 8..12. */
void seVoiceKindMarkEntries(void) {
    s32 entryIndex;
    s32 entryCount;
    s32 relativeKind;

    entryCount = gSeVoiceState->unk3F4.filledEntryCount;
    for (entryIndex = 0; entryIndex < entryCount; entryIndex++) {
        relativeKind = pagedTableGetEntryAt(&gSeVoiceState->unk3F4, entryIndex) - 8;
        if ((unsigned int) relativeKind < 5) {
            VoiceTable *kindTable = &gSeVoiceState->unk3F4;

            if (kindTable->capacity != 0) {
                kindTable->entryKeys[entryIndex].flags |= 0x100;
            }
        }
    }
}

/* Returns 1 when bit `bit` of bit row `row` of gSeVoiceState is clear. */
s32 seVoiceBitIsClear(s32 row, s32 bit) {
    s32 byteIndex;

    byteIndex = bit >> 3;
    bit = 1 << (bit & 7);
    return !(gSeVoiceState->bitRows.rows[row][byteIndex] & bit);
}

extern BitRows gSeVoiceBitRowTemplate;

/* Sets bit `bit` of row `row` in the template bit rows gSeVoiceBitRowTemplate. */
void seVoiceTemplateBitSet(s32 row, s32 bit) {
    s32 byteIndex;

    byteIndex = bit >> 3;
    bit = 1 << (bit & 7);
    gSeVoiceBitRowTemplate.rows[row][byteIndex] |= bit;
}

static s32 func_00237CB0(s32 flag, s32 key) {
    if (flag == 0 && key == 56) {
        return 10;
    }
    return 99;
}

void *memPoolAlloc(s32 size);
s32 func_00224600(s32 arg);
void completionBlockClear(void);
void pagedTablePoolInit(void);
void bufferPointersRewind(void);
int GameMpeg2Play();
s32 xglCdReadFile(void *dst, void *aux, s32 a, s32 b);
s32 xglCdServiceIfPending();
void scratchpadSendCmd23Mode1(void *command);
void scratchpadSendCmd23Mode0(void *command);
void slotSetupMode0(s32 x, s32 y, void *slot);
u8 *stateBlockGet(void);
void pxySetColorScale(void *scale);
void pxyEnableFlag8(void);
void movieStepIfActive(void);
void func_002912B0(void);
s32 statusBlockFrameUpdate();
void shardFieldInit(void *field);
void mcModuleReset(void);
void sheetStateReset(void);
void pagedTableInit(void *table, s32 capacity);
void pagedTableAppend(void *table, s32 entry, s32 kind);
void pagedTableSetPageSizes(void *table, s32 a, s32 b);
s32 battleRosterEntryIdCountGet(s32 kind);
void pagedTableSetTextBox(void *table, s32 a, s32 b, s32 c);
const char *messageTextGet(s32 id);
void sheetSetIndex(void *text);
void mcStateSetupRequest(s32 a, s32 b);
void seVoiceSceneEnter(s32 scene);
void setSteppedValue(void *stepped, s32 a, s32 b);
void xglPadSetRepeat(s32 port, s32 mask, s32 delay, s32 interval);

typedef struct CdPath {
    char text[18];
} CdPath;

typedef struct CdCommand {
    u8 head[16];
    s32 tail;
} CdCommand;

typedef struct GameLoopRuntime {
    s32 flags;
    u8 unk004[0x170];
    u8 unk174[4];
} GameLoopRuntime;

static SeTableBlock *gSeMenuBlock;
extern const CdPath gSeVoiceCdFilePath;
extern const CdCommand gSeVoiceCdCommand;
extern GameLoopRuntime gGameLoopRuntimeStorage;

static inline void voiceTableMarkKey(VoiceTable *table, s32 index) {
    if (table->capacity != 0) {
        table->entryKeys[index].flags |= 0x200;
    }
}

static inline void voiceTableSetFlags(VoiceTable *table) {
    table->unk20 |= 0x48;
    table->unk06 |= 0x100;
}

void *memset();

/* Allocates and fills the voice screen state gSeVoiceState and its 0x214-byte cell block; `mode` != 0 first reads the CD file. */
s32 seVoiceScreenInit(s32 playMode) {
    s32 hasDefaultEntry;
    CdPath cdFilePath;
    CdCommand cdCommand;

    func_00224600(0);
    completionBlockClear();
    pagedTablePoolInit();
    bufferPointersRewind();
    gSeVoiceState = memPoolAlloc(0x9D0);
    memset(gSeVoiceState, 0, 0x9D0);
    gSeMenuBlock = memPoolAlloc(0x214);
    memset(gSeMenuBlock, 0, 0x214);
    seMenuBlockClear(gSeMenuBlock);
    if (playMode != 0) {
        cdFilePath = gSeVoiceCdFilePath;
        GameMpeg2Play(gSeVoiceState->cdPath, &cdFilePath, playMode);
        xglCdReadFile(gSeVoiceState->cdPath, gSeMenuBlock, 1, 0);
        cdCommand = gSeVoiceCdCommand;
        scratchpadSendCmd23Mode1(&cdCommand);
        while (xglCdServiceIfPending() != 0) {
            if (gGameLoopRuntimeStorage.flags & 0x40) {
                slotSetupMode0(0xA0, 0x22, gGameLoopRuntimeStorage.unk174);
            }
            pxySetColorScale(stateBlockGet() + 0x850);
            pxyEnableFlag8();
            movieStepIfActive();
            func_002912B0();
            statusBlockFrameUpdate();
        }
        scratchpadSendCmd23Mode0(&cdCommand);
    }
    gSeVoiceState->bitRows = gSeVoiceBitRowTemplate;
    shardFieldInit(gSeVoiceState);
    mcModuleReset();
    sheetStateReset();
    pagedTableInit(&gSeVoiceState->unk340, 10);
    pagedTableSetPageSizes(&gSeVoiceState->unk340, 10, 0);
    hasDefaultEntry = 0;
    if (gSeMenuBlock->cells[0] != 0) {
        pagedTableAppend(&gSeVoiceState->unk340, 100, 5);
        hasDefaultEntry = 1;
    }
    if (gSeMenuBlock->tailA != 0) {
        pagedTableAppend(&gSeVoiceState->unk340, 101, 5);
        hasDefaultEntry = 0;
    }
    if (gSeMenuBlock->tailB != 0) {
        pagedTableAppend(&gSeVoiceState->unk340, 102, 5);
        hasDefaultEntry = 0;
    }
    if (gSeVoiceState->unk340.filledEntryCount != 0) {
        voiceTableMarkKey(&gSeVoiceState->unk340, gSeVoiceState->unk340.filledEntryCount - 1);
    }
    if (hasDefaultEntry == 0) {
        pagedTableAppend(&gSeVoiceState->unk340, 103, 5);
        pagedTableAppend(&gSeVoiceState->unk340, 104, 5);
        if (battleRosterEntryIdCountGet(1) != 0) {
            pagedTableAppend(&gSeVoiceState->unk340, 105, 5);
        }
        if (gSeVoiceState->unk340.filledEntryCount != 0) {
            voiceTableMarkKey(&gSeVoiceState->unk340, gSeVoiceState->unk340.filledEntryCount - 1);
        }
        pagedTableAppend(&gSeVoiceState->unk340, 106, 5);
        if (battleRosterEntryIdCountGet(1) != 0) {
            pagedTableAppend(&gSeVoiceState->unk340, 118, 5);
        }
    }
    pagedTableAppend(&gSeVoiceState->unk340, 107, 5);
    pagedTableSetTextBox(&gSeVoiceState->unk340, 0, 219, 25);
    voiceTableSetFlags(&gSeVoiceState->unk340);
    pagedTableInit(&gSeVoiceState->voiceTable, 512);
    pagedTableInit(&gSeVoiceState->unk3F4, 10);
    pagedTableInit(&gSeVoiceState->voiceKind, 8);
    sheetSetIndex(messageTextGet(108));
    mcStateSetupRequest(7, 0);
    gSeVoiceState->unk9C8 = 48;
    seVoiceSceneEnter(0);
    gSeVoiceState->scene = 0;
    gSeVoiceState->voiceFlags = 0;
    setSteppedValue(&gSeVoiceState->unk430.unk20, 128, 128);
    setSteppedValue(&gSeVoiceState->unk430.unk24, 0, 0);
    gSeVoiceState->unk430.unk10 = 0x70F8;
    gSeVoiceState->unk430.unk12 = 0x74E8;
    gSeVoiceState->unk430.unk14 = 0x9E0;
    gSeVoiceState->unk430.unk16 = 0x4F0;
    gSeVoiceState->unk430.unk0C = 1;
    gSeVoiceState->unk430.unk0D = 10;
    gSeVoiceState->unk430.title = messageTextGet(84);
    gSeVoiceState->unk430.unk0E = 6;
    pagedTableInit(&gSeVoiceState->unk550, 6);
    pagedTableInit(&gSeVoiceState->unk594, 256);
    gSeVoiceCallbackFlag = 0;
    xglPadSetRepeat(0, 0xF000, 300, 3);
    return 1;
}

s32 xglPadSetDefaults();
s32 memPoolFree(void *);
s32 xcBlockFlagsReset(void *);

/* Waits until the CD service is idle, releases the state block gSeVoiceState and clears the pointer. */
void cdStateDestroy(void) {
    xglPadSetDefaults();
    while (xglCdServiceIfPending() != 0) {
        statusBlockFrameUpdate();
    }
    xcBlockFlagsReset(gSeVoiceState);
    memPoolFree(gSeVoiceState);
    gSeVoiceState = 0;
}

typedef short s16;

typedef unsigned int u32;

typedef struct SteppedValue {
    s16 level;
    s16 targetLevel;
} SteppedValue;

typedef struct TiSheet {
    u8 unk_00[0x2];
    u16 unk_02;
    u8 unk_04[0x16];
    u16 unk_1A;
    u16 frameBase;
    u8 unk_1E[0x36 - 0x1E];
    u16 unk_36;
} TiSheet;

typedef struct SeVoiceBox {
    const char *title; /* 0x00 */
    union {
        const char *textA;
        s32 textAId;
    };
    union {
        const char *textB;
        s32 textBId;
    };
    u8 colour; /* 0x0C */
    u8 unk_0D;
    u8 unk_0E;
    u8 unk_0F;
    u16 x; /* 0x10 */
    u16 y;
    u16 width;
    u16 height;
    u16 margin;
    u8 unk_1A[6];
} SeVoiceBox;

typedef struct SeVoiceCtrlBits {
    u32 state : 4;
    u32 previousState : 4;
    u32 confirm : 4;
    u32 playing : 1;
    u32 unk_13 : 19;
} SeVoiceCtrlBits;

typedef struct SeVoiceUnit {
    u16 phase;
    u8 unk_02[0x4];
    u16 statusFlags;
    u8 unk_08[0x34];
    SteppedValue levelA;
    u8 flag40;
    u8 unk_41[0x3];
    TiSheet sheetC;
    u8 unk_7C[0x4];
    SteppedValue levelB;
    u8 flag84;
    u8 unk_85[0x3];
    SeVoiceBox box;
    union {
        u32 ctrl;
        SeVoiceCtrlBits bits;
    };
    SteppedValue levelC;
    u8 unk_B0[0x64];
    void *showMessage; /* 0x114 */
    u8 unk118[0x180];
    u8 unk298[0x1D8];
} SeVoiceUnit;

typedef struct VoiceSheet {
    u8 unk_00[0x20];
    s32 flags;
    u8 unk_24[0x40];
    SteppedValue level;
} VoiceSheet;

typedef struct SeVoicePlaybackView {
    u8 unk000[0x37C];
    u8 voiceKindCursor[2];
    u16 voiceKindCount;
    u8 unk380[0x38];
    u8 voiceTableCursor[2];
    u16 voiceTableCount;
    u8 unk3BC[0x38];
    VoiceSheet sheet3F4;
    u8 unk45C[0xC];

    u8 unk468[0xE8];
    SeVoiceUnit unit;
    u8 unk9C0[2];
    u8 voiceFlags;
    u8 unk9C3[0x7];
    u8 statusByte;
} SeVoicePlaybackView;

/* The voice screen state viewed as its playback layout. */
#define gSeVoicePlayback ((SeVoicePlaybackView *) gSeVoiceState)

void func_002234F8(void *, s32, void *, s32);
void seVoiceStartDefault(unsigned int handle);
void setSteppedTarget(SteppedValue *, s32);
void tiSheetCopyFields(void *, void *);
extern void sceneShowMsgPairD7();
extern void sceneShowMsgPairE7();
extern void sceneShowMsgPairE5();
extern void sceneShowMsgPairE8();

/* Phase of a unit: its phase field, plus 5 when ctrl bit 12 is set. */
#define unitPhase(u) ((u)->phase + (((u)->ctrl >> 12) & 1 ? 5 : 0))

static inline void sheetClear(VoiceSheet *sheet) {
    sheet->flags &= ~1;
}

static inline void sheetSet(VoiceSheet *sheet) {
    sheet->flags |= 1;
}

/* Sets the voice unit's mode (low nibble of ctrl; the old mode moves to the next nibble) and runs that mode's setup. */
const CdPath gSeVoiceCdFilePath = { "mnu\\shop\\%02d.shp" };

const CdCommand gSeVoiceCdCommand = { { 0x0a }, 0x80 };

void seVoiceUnitSetMode(s32 unitMode) {
    SeVoiceUnit *unit;
    s32 i;
    s32 count;
    s32 phase;

    unit = &gSeVoicePlayback->unit;
    if ((unit->ctrl & 0xF) != unitMode) {
        unit->ctrl = (unit->ctrl & ~0xF0) | ((unit->ctrl & 0xF) << 4);
        unit->ctrl = (unit->ctrl & ~0xF) | (unitMode & 0xF);
    }
    gSeVoicePlayback->unit.showMessage = 0;
    switch (unitMode) {
    case 2:
    case 5:
    case 6:
    case 9:
    case 10:
    case 11:
    default:
        mcStateSetupRequest(7, 0);
        unit->bits.playing = gSeVoicePlayback->voiceFlags >> 2;
        setSteppedTarget(&gSeVoicePlayback->sheet3F4.level, 0x80);
        sheetClear(&gSeVoicePlayback->sheet3F4);
        setSteppedTarget(&unit->levelA, 0);
        setSteppedTarget(&unit->levelC, 0);
        unit->flag84 = 0;
        unit->flag40 = 0;
        break;
    case 3:
        mcStateSetupRequest(15, 0);
        setSteppedTarget(&gSeVoicePlayback->sheet3F4.level, 0);
        sheetSet(&gSeVoicePlayback->sheet3F4);
        if (unit->flag40 != 2) {
            count = (unit->ctrl >> 12) & 1 ? 6 : 5;
            pagedTableSetPageSizes(unit, count, 0);
            for (i = 0; i < count; i++) {
                pagedTableAppend(unit, i, 2);
            }
            pagedTableSetTextBox(unit, 0, 0xE0, 0x2F);
        }
        unit->flag40 = 2;
        unit->statusFlags &= 0xFF77;
        setSteppedTarget(&unit->levelA, 0x80);
        setSteppedTarget(&unit->levelB, 0);
        setSteppedTarget(&unit->levelC, 0x80);
        unit->ctrl &= ~0xF00;
        phase = unitPhase(unit);
        if ((u32) (phase - 2) < 3u) {
            mcStateSetupRequest(
                15, messageTextGet(0x21));
            return;
        }
        mcStateSetupRequest(15, 0);
        return;
    case 4:
        pagedTableSetPageSizes(&unit->sheetC, 10, 0x20);
        func_002234F8(&unit->sheetC,
                      pagedTableGetCurrentEntry(&gSeVoicePlayback->sheet3F4),
                      gSeVoicePlayback->unit.unk118, unitPhase(unit));
        tiSheetCopyFields(&unit->box, &unit->sheetC);
        unit->statusFlags |= 0x88;
        unit->flag40 = 2;
        setSteppedTarget(&unit->levelA, 0);
        setSteppedTarget(&unit->levelB, 0x80);
        phase = unitPhase(unit);
        mcStateSetupRequest(
            15, (((unit->ctrl >> 8) & 0xF) == 0 && ((u32) (phase - 2) < 3u || phase == 10))
                    ? messageTextGet(0x21)
                    : 0);
        unit->box.x = 0x80F8;
        unit->box.y = 0x7A88;
        unit->box.colour = 1;
        unit->box.unk_0D = 10;
        unit->box.unk_0E = 2;
        unit->box.unk_0F = 2;
        if ((unit->ctrl >> 12) & 1) {
            if (unit->phase == 4) {
                unit->box.width = 0x900;
                unit->box.height = 0x200;
                unit->box.margin = 0x200;
                unit->box.title = 0;
                unit->box.textA = messageTextGet(0xE3);
                unit->box.textB = messageTextGet(0x53);
                unit->flag84 = 2;
                break;
            } else if (unit->phase == 5) {
                unit->box.width = 0x900;
                unit->box.height = 0x200;
                unit->box.margin = 0x200;
                unit->box.title = 0;
                unit->box.textA = messageTextGet(0xEF);
                unit->box.textB = messageTextGet(0x53);
                unit->flag84 = 3;
                break;
            }
        }
        unit->box.width = 0xB00;
        unit->box.height = 0x200;
        unit->box.margin = 0;
        unit->box.title = 0;
        unit->box.textA = messageTextGet(0x53);
        unit->box.textB = 0;
        unit->flag84 = 1;
        break;
    case 7:
        mcStateSetupRequest(7, 0);
        gSeVoicePlayback->unit.showMessage = sceneShowMsgPairD7;
        break;
    case 8:
        phase = unitPhase(unit);
        mcStateSetupRequest(7, 0);
        gSeVoicePlayback->unit.showMessage =
            (phase == 5) ? sceneShowMsgPairE7 : sceneShowMsgPairE5;
        break;
    case 12:
        mcStateSetupRequest(7, 0);
        gSeVoicePlayback->unit.showMessage = sceneShowMsgPairE8;
        seVoiceStartDefault(3);
        break;
    }
}

typedef struct StatusHeader {
    u8 unk00[2];
    u16 headerFlags;
} StatusHeader;

s32 pagedTableUpdate(void *, s32 *);
void pagedTableAnimateSlots(void *);
s16 *func_00222768(s32, s32);
s32 func_00222AC0(s32, s32, s32);
s32 func_00223F98(s32);
s32 func_002282A0(u8);
void movieStartPlayback(void *, s32);
s32 statusHeaderIsReadyMode20(void);
s32 statusHeaderIsReadyMode40(void);
StatusHeader *getStatusHeader(void);
void stepValueTowardTarget(SteppedValue *);
void statusReadSync(u8 *, s32);
void tiSheetCopyTrailingField(void *, void *);

/* Per-frame voice unit update: steps its levels, reacts to the status header flags and advances the unit by its mode. */
void seVoiceUnitUpdate(void) {
    SeVoicePlaybackView *state = gSeVoicePlayback;
    SeVoiceUnit *unit = &state->unit;
    s32 stepDelta;
    s32 stepStarted = 0;
    s32 phase;
    s32 sheetMoved;
    s32 entryId;
    s32 valueBefore;

    if (unit->flag40 != 0) {
        pagedTableAnimateSlots(unit);
    }
    if (unit->flag84 != 0) {
        pagedTableAnimateSlots(&unit->sheetC);
    }
    stepValueTowardTarget(&unit->levelA);
    stepValueTowardTarget(&unit->levelB);
    stepValueTowardTarget(&unit->levelC);
    if ((unit->ctrl & 0xF) != 2 && (unit->ctrl & 0xF) != 7 && (unit->ctrl & 0xF) != 8) {
        if (getStatusHeader()->headerFlags & 0xC) {
            if (getStatusHeader()->headerFlags & 4) {
                stepStarted = 1;
                pagedTableMoveCursor(&gSeVoicePlayback->sheet3F4,
                                                        -1);
                seVoiceStartDefault(1);
            }
            if (getStatusHeader()->headerFlags & 8) {
                stepStarted = 1;
                pagedTableMoveCursor(&gSeVoicePlayback->sheet3F4,
                                                        1);
                seVoiceStartDefault(1);
            }
            seVoiceKindCursorSeek(getStatusHeader()->headerFlags & 4);
        }
    }

    switch (unit->ctrl & 0xF) {
    case 2:
    case 5:
    case 6:
    case 9:
    case 10:
    case 11:
    default:
        sheetMoved = pagedTableUpdate(&gSeVoicePlayback->sheet3F4, &stepDelta);
        pagedTableAnimateSlots(&gSeVoicePlayback->sheet3F4);
        sheetSetIndex(
            messageTextGet(((unit->ctrl >> 12) & 1) + 0xF4));
        if (stepDelta != 0) {
            if (stepDelta > 0) {
                seVoiceStartDefault(2);
                seVoiceUnitSetMode(3);
            } else {
                seVoiceStartDefault(8);
                seVoiceSceneEnter(0);
            }
        } else if (statusHeaderIsReadyMode20() != 0) {
            seVoiceUnitSetMode(7);
            seVoiceStartDefault(3);
        } else if (sheetMoved != 0) {
            seVoiceStartDefault(1);
        }
        break;
    case 3:
        sheetMoved = pagedTableUpdate(unit, &stepDelta);
        sheetSetIndex(messageTextGet(0xF2));
        if (sheetMoved != 0) {
            if (stepDelta == 0) {
                seVoiceStartDefault(1);
            }
            phase = unitPhase(unit);
            if (((unit->ctrl >> 8) & 0xF) != 0 && (u32) (phase - 2) >= 3u) {
                unit->ctrl &= ~0xF00;
            }
            if (((unit->ctrl >> 8) & 0xF) == 0 && (u32) (phase - 2) < 3u) {
                mcStateSetupRequest(
                    15, messageTextGet(0x21));
            } else {
                mcStateSetupRequest(15, 0);
            }
        }
        phase = unitPhase(unit);
        if (((unit->ctrl >> 8) & 0xF) != 0) {
            if ((u32) (phase - 2) >= 3u) {
                unit->ctrl = unit->ctrl & ~0xF00;
                mcStateSetupRequest(15, 0);
            }
        } else if (((getStatusHeader()->headerFlags & 0x80) != 0) && (u32) (phase - 2) < 3u) {
            unit->ctrl = (unit->ctrl & ~0xF00) | 0x100;
            mcStateSetupRequest(15, 0);
            seVoiceStartDefault(6);
        }
        if (unit->levelB.level == 0) {
            unit->statusFlags &= 0xFFBF;
        }
        if (stepDelta != 0) {
            if (stepDelta > 0) {
                unit->ctrl = unit->ctrl & ~0xF00;
                seVoiceUnitSetMode(4);
                unit->statusFlags &= 0xFFBF;
                seVoiceStartDefault(2);
            } else {
                if (((unit->ctrl >> 8) & 0xF) != 0) {
                    unit->ctrl = unit->ctrl & ~0xF00;
                    mcStateSetupRequest(
                        15, messageTextGet(0x21));
                } else {
                    seVoiceUnitSetMode(2);
                    unit->statusFlags &= 0xFFBF;
                }
                seVoiceStartDefault(8);
            }
        }
        break;
    case 4:
        phase = unitPhase(unit);
        if (phase == 10) {
            sheetSetIndex(messageTextGet(0xE6));
        } else {
            movieStartPlayback(gSeVoicePlayback->unit.unk298, phase);
        }
        if (pagedTableUpdate(&unit->sheetC, &stepDelta) != 0 && stepDelta == 0) {
            seVoiceStartDefault(1);
        }
        if (stepStarted != 0) {
            func_002234F8(&unit->sheetC,
                          pagedTableGetCurrentEntry(&gSeVoicePlayback->sheet3F4),
                          gSeVoicePlayback->unit.unk118,
                          unitPhase(unit));
        }
        sheetMoved = pagedTableUpdate(unit, &stepDelta);
        if (sheetMoved != 0 && stepDelta == 0) {
            seVoiceUnitSetMode(4);
            seVoiceStartDefault(1);
            if (((unit->ctrl >> 8) & 0xF) == 0) {
                phase = unitPhase(unit);
                mcStateSetupRequest(
                    15, ((u32) (phase - 2) < 3u || phase == 10)
                            ? messageTextGet(0x21)
                            : 0);
            }
        }
        tiSheetCopyTrailingField(&unit->box, &unit->sheetC);
        phase = unitPhase(unit);
        if (((unit->ctrl >> 8) & 0xF) != 0) {
            if ((u32) (phase - 2) >= 3u && phase != 10) {
                unit->ctrl = unit->ctrl & ~0xF00;
                mcStateSetupRequest(15, 0);
            }
        } else if (((getStatusHeader()->headerFlags & 0x80) != 0) &&
                   ((u32) (phase - 2) < 3u || phase == 10)) {
            unit->ctrl = (unit->ctrl & ~0xF00) | 0x100;
            mcStateSetupRequest(15, 0);
            seVoiceStartDefault(6);
        }
        if (statusHeaderIsReadyMode20() != 0 && unit->sheetC.unk_02 != 0) {
            valueBefore = 0;
            entryId = pagedTableGetCurrentEntry(&gSeVoicePlayback->sheet3F4);
            if (phase == 5) {
                valueBefore = func_00222768(entryId, 6)[0];
            }
            if (func_00222AC0(entryId, phase, pagedTableGetCurrentEntry(&unit->sheetC)) != 0) {
                seVoiceStartDefault(2);
                gSeVoiceCallbackFlag = 1;
                tiSheetCopyTrailingField(&unit->box, &unit->sheetC);
                if (phase == 9) {
                    if (func_00223F98(entryId) != 0) {
                        seVoiceUnitSetMode(8);
                    }
                } else if (phase == 5) {
                    if (valueBefore != func_00222768(entryId, 6)[0]) {
                        seVoiceUnitSetMode(8);
                    }
                }
                statusReadSync(&gSeVoicePlayback->statusByte, entryId);
            } else {
                seVoiceStartDefault(3);
            }
            break;
        }
        if (statusHeaderIsReadyMode40() != 0) {
            if (((unit->ctrl >> 8) & 0xF) != 0) {
                unit->ctrl &= ~0xF00;
                mcStateSetupRequest(
                    15, ((u32) (phase - 2) < 3u || phase == 10)
                            ? messageTextGet(0x21)
                            : 0);
            } else if (gSeVoicePlayback->statusByte != 0) {
                seVoiceUnitSetMode(12);
            } else {
                seVoiceUnitSetMode(3);
                unit->statusFlags |= 0x40;
            }
            seVoiceStartDefault(8);
            break;
        }
        if (func_002282A0(gSeVoicePlayback->statusByte) != 0) {
            seVoiceUnitSetMode(12);
        }
        break;
    case 7:
        if (statusHeaderIsReadyMode20() != 0 || statusHeaderIsReadyMode40() != 0) {
            seVoiceStartDefault(8);
            seVoiceUnitSetMode(2);
        }
        break;
    case 8:
        if (statusHeaderIsReadyMode20() != 0 || statusHeaderIsReadyMode40() != 0) {
            seVoiceStartDefault(8);
            seVoiceUnitSetMode(4);
        }
        break;
    case 12:
        if (statusHeaderIsReadyMode20() != 0 || statusHeaderIsReadyMode40() != 0) {
            seVoiceStartDefault(8);
            seVoiceUnitSetMode(4);
        }
        break;
    }
}

s32 func_00229828(s32, void *, void *, void *);

void seVoiceStartEntry(s32 entryId) {
    func_00229828(entryId & 0xFF, &gSeVoiceState->unk550, &gSeVoiceState->unk3F4, &gSeVoiceState->unk468);
}

extern s8 gVsyncsPerFrame;
s32 func_001B9228();
s32 memPoolInit(s32, s32);
s32 memPoolReset();
s32 pagedTablePoolShutdown();
s32 startSeVoiceAndWait();

/* Runs startSeVoiceAndWait with the guard flag gVsyncsPerFrame set, then calls func_001B9228 when gSeVoiceCallbackFlag is set. */
s32 seVoiceStartGuarded(s32 voiceArg) {
    s32 voiceResult;
    s32 savedFlag;

    savedFlag = gVsyncsPerFrame;
    gVsyncsPerFrame = 1;
    memPoolInit(0x980000, 0x100000);
    voiceResult = startSeVoiceAndWait(voiceArg);
    pagedTablePoolShutdown();
    memPoolReset();
    gVsyncsPerFrame = savedFlag;
    if (gSeVoiceCallbackFlag != 0) {
        func_001B9228();
    }
    return voiceResult;
}

void cdStateDestroy();
s32 seVoiceScreenRun();

/* Starts the default sound effect voice 0xB and waits for its result; returns -1 when seVoiceScreenInit fails. */
s32 startSeVoiceAndWait(s32 mode) {
    s32 voiceResult;

    if (seVoiceScreenInit(mode) == 0) {
        return -1;
    }
    seVoiceStartDefault(0xB);
    voiceResult = seVoiceScreenRun();
    cdStateDestroy();
    return voiceResult;
}

void fontQueueColorState(s32);
void fontQueueLine(u16 startX, u16 startY, u16 endX, u16 endY);
void xglFontDrawBoxBlend(u16 *rect, s32 inset, s32 colour, s32 blendMode);

/* Draws the blended info box at (x, y) with two horizontal rules inside it. */
void seVoiceInfoBoxDraw(s32 x, s32 y) {
    u16 rect[4];

    rect[0] = ((x + 0x6FF) << 4) + 8;
    rect[1] = ((y + 0x71F) << 4) + 8;
    rect[2] = ((x + 0x7BE) << 4) + 8;
    rect[3] = ((y + 0x75E) << 4) + 8;
    fontQueueColorState(0x73221E1D);
    xglFontDrawBoxBlend(rect, 2, 0x73312A29, 1);
    rect[1] += 352;
    fontQueueColorState(0x73423732);
    fontQueueLine(rect[0] + 32, rect[1], rect[2] - 32, rect[1]);
    rect[1] += 288;
    fontQueueLine(rect[0] + 32, rect[1], rect[2] - 32, rect[1]);
}

typedef struct VoiceBlock {
    s32 value0;
    s16 value4;
    s16 value6;
    s16 value8;
    s16 valueA;
    s16 valueC;
    s16 valueE;
    s8 value10;
    s8 value11;
    s8 value12;
    s8 value13;
    u8 unk14[0xD4];
} VoiceBlock;

typedef struct VoiceSlot {
    u8 unk00[0x38];
    s32 unk38;
    s16 unk3C;
} VoiceSlot;

typedef struct SeVoiceRecord {
    u8 *textId;
    s32 savedValue;
    s32 liveVoiceValue;
} SeVoiceRecord;

typedef struct SeVoiceRecordsView {
    u8 unk000[0x340];
    u8 pagedTable[0x3C];
    u8 voiceKindCursor[2];
    u16 voiceKindCount;
    u8 unk380[0x38];
    u8 voiceTableCursor[2];
    u16 voiceTableCount;
    u8 unk3BC[0x38];
    u8 unk3F4[0x5C];
    u8 alpha1104;
    u8 unk1105[3];
    short timer1108;
    u8 unk110A[2];
    short spriteFrame1112;
    u8 unk1114[0xE];
    VoiceBlock unk468;
    u8 unk550[0xAA];
    u16 fadeLevel;
    u8 unk532[0x4];
    SeVoiceRecord records[8];
    s32 recordCount;
    void (*onFinish)(void);
    u8 unk640[0x358];
    u8 mode;
    u8 unk9C1;
    u8 voiceFlags;
    u8 unk9C3[5];
    u16 fadePhase;
} SeVoiceRecordsView;

s32 func_002228E8(s32 slot, s32 kind, s32 value, s32 flag);
u8 *func_00223F70(s32 value);
VoiceBlock *func_A86DF8(s32 slot);
VoiceSlot *func_A982F0(s32 slot);
void fontDrawRotatedSpriteU160(s16 *rect, s32 a1, s32 a2, s32 a3);
s32 xglFontDrawStringScaled(s32 a0, s32 a1, s32 a2, s32 a3);
s32 xglFontDrawDigits(s32 a0, s32 a1, s32 a2, s32 a3);

/* Collects the voice records whose saved value differs from the current one for this voice slot and kind,
 * then draws the record table. */
void seVoiceRecordTableDraw(s32 voiceSlot, s32 kindSlot, s32 index) {
    s32 bailOut;
    s32 voiceBit;
    s32 resolved;
    unsigned int kindIndex;
    u8 *voiceKindRow = 0;
    VoiceBlock *voiceBlock;
    s32 i;
    s32 recordRow;
    s32 k;
    u16 rect[4];
    VoiceSlot *voiceSlotState;

    bailOut = 0;
    voiceBit = 0;
    ((SeVoiceRecordsView *) gSeVoiceState)->recordCount = 0;
    resolved = slotKindResolve(kindSlot);
    kindIndex = resolved - 1;
    if (kindIndex >= 7) {
        bailOut = 1;
    } else {
        voiceKindRow = (u8 *) func_00222C60(resolved, index);
        if (voiceKindRow == 0) {
            bailOut = 1;
        } else if (voiceSlot == 0) {
            bailOut = 1;
        } else {
            voiceBit = 1 << (voiceSlot - 1);
        }
    }
    if (!bailOut && index && (voiceBit & *(u16 *) (voiceKindRow + 0xC)) != 0) {
        s32 savedKindValue;
        s32 pairedKindValue = 0;
        s32 savedSlotUnk38 = 0;
        s32 savedSlotUnk3C = 0;
        s32 needsRestore = 1;

        voiceSlotState = func_A982F0(voiceSlot);
        ((SeVoiceRecordsView *) gSeVoiceState)->unk468 = *func_A86DF8(voiceSlot);
        savedKindValue = func_00222768(voiceSlot, kindSlot)[0];
        if (kindSlot == 5) {
            pairedKindValue = func_00222768(voiceSlot, 6)[0];
        }
        if (kindSlot == 6) {
            pairedKindValue = func_00222768(voiceSlot, 5)[0];
            needsRestore = func_00223F70(pairedKindValue)[1] == 0;
        }
        if (needsRestore != 0) {
            savedSlotUnk38 = voiceSlotState->unk38;
            savedSlotUnk3C = voiceSlotState->unk3C;
            func_002228E8(voiceSlot, kindSlot, index, 0);
        }
        voiceKindRow += 0x10;
        voiceBlock = func_A86DF8(voiceSlot);

        for (index = 0; index < 11; index++) {
            s32 messageId = 0;
            s32 savedValue = 0;
            s32 liveVoiceValue = 0;
            s32 matched = 0;

            switch (index) {
            case 0:
                messageId = messageTextGet(0x3D);
                savedValue = ((SeVoiceRecordsView *) gSeVoiceState)->unk468.value0;
                liveVoiceValue = voiceBlock->value0;
                break;
            case 1:
                messageId = messageTextGet(voiceSlot >= 13 ? 0x4C : 0x3E);
                liveVoiceValue = voiceBlock->value4;
                savedValue = ((SeVoiceRecordsView *) gSeVoiceState)->unk468.value4;
                break;
            case 2:
                messageId = messageTextGet(voiceSlot >= 13 ? 0x4E : 0x41);
                liveVoiceValue = voiceBlock->value6;
                savedValue = ((SeVoiceRecordsView *) gSeVoiceState)->unk468.value6;
                break;
            case 3:
                messageId = messageTextGet(voiceSlot >= 13 ? 0x4F : 0x42);
                liveVoiceValue = voiceBlock->value8;
                savedValue = ((SeVoiceRecordsView *) gSeVoiceState)->unk468.value8;
                break;
            case 4:
                messageId = messageTextGet(0x46);
                liveVoiceValue = voiceBlock->valueA;
                savedValue = ((SeVoiceRecordsView *) gSeVoiceState)->unk468.valueA;
                break;
            case 5:
                messageId = messageTextGet(0x47);
                liveVoiceValue = voiceBlock->valueC;
                savedValue = ((SeVoiceRecordsView *) gSeVoiceState)->unk468.valueC;
                break;
            case 6:
                messageId = messageTextGet(0x5E);
                liveVoiceValue = voiceBlock->valueE;
                savedValue = ((SeVoiceRecordsView *) gSeVoiceState)->unk468.valueE;
                break;
            case 7:
                messageId = messageTextGet(0x43);
                liveVoiceValue = voiceBlock->value10;
                savedValue = ((SeVoiceRecordsView *) gSeVoiceState)->unk468.value10;
                break;
            case 8:
                messageId = messageTextGet(0x44);
                liveVoiceValue = voiceBlock->value11;
                savedValue = ((SeVoiceRecordsView *) gSeVoiceState)->unk468.value11;
                break;
            case 9:
                messageId = messageTextGet(0x45);
                liveVoiceValue = voiceBlock->value12;
                savedValue = ((SeVoiceRecordsView *) gSeVoiceState)->unk468.value12;
                break;
            case 10:
                messageId = messageTextGet(0x48);
                liveVoiceValue = voiceBlock->value13;
                savedValue = ((SeVoiceRecordsView *) gSeVoiceState)->unk468.value13;
                break;
            }
            if (savedValue != liveVoiceValue) {
                matched = 1;
            } else if (kindIndex < 2) {
                for (k = 0; k < 4; k++) {
                    if (index >= 2 && ((s8 *) voiceKindRow)[2 * k] == index - 1) {
                        matched = 1;
                        break;
                    }
                }
            } else if (resolved == 4 && index == 0) {
                matched = 1;
            } else if (resolved == 5 && index == 1) {
                matched = 1;
            }
            if (matched != 0) {
                ((SeVoiceRecordsView *) gSeVoiceState)
                    ->records[((SeVoiceRecordsView *) gSeVoiceState)->recordCount]
                    .textId = (u8 *) messageId;
                ((SeVoiceRecordsView *) gSeVoiceState)
                    ->records[((SeVoiceRecordsView *) gSeVoiceState)->recordCount]
                    .savedValue = savedValue;
                ((SeVoiceRecordsView *) gSeVoiceState)
                    ->records[((SeVoiceRecordsView *) gSeVoiceState)->recordCount]
                    .liveVoiceValue = liveVoiceValue;
                ((SeVoiceRecordsView *) gSeVoiceState)->recordCount++;
                if (((SeVoiceRecordsView *) gSeVoiceState)->recordCount >= 8) {
                    break;
                }
            }
        }
        if (needsRestore != 0) {
            func_002228E8(voiceSlot, kindSlot, savedKindValue, 0);
            if (kindSlot == 5) {
                func_002228E8(voiceSlot, 6, pairedKindValue, 0);
            }
            if (kindSlot == 6) {
                func_002228E8(voiceSlot, 5, pairedKindValue, 0);
            }
            voiceSlotState->unk38 = savedSlotUnk38;
            voiceSlotState->unk3C = savedSlotUnk3C;
        }
    }

    if (((SeVoiceRecordsView *) gSeVoiceState)->recordCount <= 0) {
        ((SeVoiceRecordsView *) gSeVoiceState)->records[0].textId = 0;
        ((SeVoiceRecordsView *) gSeVoiceState)->recordCount = 1;
    }
    rect[0] = (s16) 0x80F8;
    rect[1] = 0x7898;
    rect[2] = (s16) 0x8EF8;
    rect[3] = ((((SeVoiceRecordsView *) gSeVoiceState)->recordCount * 17 + 0x79A) << 4) + 8;
    fontQueueColorState(0x73221E1D);
    xglFontDrawBoxBlend(rect, 2, 0x73312A29, 1);
    rect[1] += 0x70;
    for (recordRow = 0; recordRow < ((SeVoiceRecordsView *) gSeVoiceState)->recordCount + 1; recordRow++) {
        fontQueueColorState(0x73423732);
        fontQueueLine((rect[0] + 0x20) & 0xFFFF, rect[1],
                                                                   (rect[2] + 0xFFE0) & 0xFFFF, rect[1]);
        rect[1] += 0x110;
    }

    rect[1] = 0x7938;
    for (recordRow = 0; recordRow < ((SeVoiceRecordsView *) gSeVoiceState)->recordCount; recordRow++) {
        if (((SeVoiceRecordsView *) gSeVoiceState)->records[recordRow].textId != 0) {
            fontQueueColorState(0x80545934);
            xglFontDrawStringScaled((rect[0] + 0x70) & 0xFFFF, rect[1],
                          ((SeVoiceRecordsView *) gSeVoiceState)->records[recordRow].textId, 0xD);
            rect[2] = rect[0] + 0x7E0;
            rect[3] = rect[1];
            fontDrawRotatedSpriteU160(&rect[2], 1, 0x110, 0xD0);
            fontQueueColorState(0x80626262);
            xglFontDrawDigits((rect[0] + 0x710) & 0xFFFF, (rect[1] + 0xFFE0) & 0xFFFF,
                              ((SeVoiceRecordsView *) gSeVoiceState)->records[recordRow].savedValue, 0x10);
            if (((SeVoiceRecordsView *) gSeVoiceState)->records[recordRow].liveVoiceValue >
                ((SeVoiceRecordsView *) gSeVoiceState)->records[recordRow].savedValue) {
                fontQueueColorState(0x80713F37);
            } else if (((SeVoiceRecordsView *) gSeVoiceState)->records[recordRow].liveVoiceValue <
                       ((SeVoiceRecordsView *) gSeVoiceState)->records[recordRow].savedValue) {
                fontQueueColorState(0x8016164A);
            }
            xglFontDrawDigits((rect[0] + 0xCF0) & 0xFFFF, (rect[1] + 0xFFE0) & 0xFFFF,
                              ((SeVoiceRecordsView *) gSeVoiceState)->records[recordRow].liveVoiceValue, 0x10);
        }
        rect[1] += 0x110;
    }
}

void func_00185A20(void);
void xglFontSetDigitStep(s32 step);
void xglFontSetAlphaScale(unsigned char scale);
void fontQueueSpriteReset(void);
void shardFieldUpdate(SeVoiceRecordsView *state);
void xglFontFreeBuffer(void);
void func_0021AF70(void *table, s32 a1, s32 a2, s32 a3);
void func_0021ABB0(s32 a0, s32 a1, s32 a2, s32 a3);
void func_0021CB90(s32 a0, s32 a1, void *a2, s32 a3);
void xglFontResetState(void);
void seVoicePanelDraw(s32 alpha);
s32 sheetLatchedPositionGet(void);
void sheetDrawFrame(s32 frame);
void mcStateSeekEntry(s32 value);

/* Draws one frame of the voice screen: fade alpha from fadePhase, the sheets and glyph overlays, the
 * mode heading and the onFinish callback. */
void seVoiceScreenDraw(void) {
    func_00185A20();
    xglFontSetDigitStep(10);
    xglFontSetAlphaScale(128);
    fontQueueSpriteReset();
    xglFontSetAlphaScale(((SeVoiceRecordsView *) gSeVoiceState)->fadePhase < 16
                             ? (u8) (((SeVoiceRecordsView *) gSeVoiceState)->fadePhase << 3)
                             : 128);
    shardFieldUpdate(((SeVoiceRecordsView *) gSeVoiceState));
    xglFontSetAlphaScale(128);
    xglFontFreeBuffer();
    fontQueueColorState(0x80808080);
    xglFontSetAlphaScale(((SeVoiceRecordsView *) gSeVoiceState)->alpha1104);
    if (((SeVoiceRecordsView *) gSeVoiceState)->fadePhase >= 57) {
        func_0021AF70(((SeVoiceRecordsView *) gSeVoiceState)->pagedTable, 16, 47,
                      (((SeVoiceRecordsView *) gSeVoiceState)->fadePhase - 56) << 1);
    }
    if (((SeVoiceRecordsView *) gSeVoiceState)->fadePhase >= 21) {
        xglFontSetAlphaScale(((SeVoiceRecordsView *) gSeVoiceState)->fadePhase < 36
                                 ? (u8) ((((SeVoiceRecordsView *) gSeVoiceState)->fadePhase + 236) << 3)
                                 : 128);
        xglFontSetAlphaScale(128);
    }
    xglFontResetState();
    if (((SeVoiceRecordsView *) gSeVoiceState)->timer1108 != 0) {
        seVoicePanelDraw(((SeVoiceRecordsView *) gSeVoiceState)->timer1108);
    }
    if (((SeVoiceRecordsView *) gSeVoiceState)->mode == 7) {
        SeVoiceRecordsView *state = ((SeVoiceRecordsView *) gSeVoiceState);

        state->fadeLevel += (s8) gVsyncsPerFrame << 4;
        if ((short) state->fadeLevel >= 128) {
            state->fadeLevel = 128;
        }
        seVoiceStartEntry((u8) state->fadeLevel);
    }
    if (((SeVoiceRecordsView *) gSeVoiceState)->spriteFrame1112 != 0) {
        xglFontResetState();
        xglFontSetAlphaScale((u8) ((SeVoiceRecordsView *) gSeVoiceState)->spriteFrame1112);
        func_0021CB90(239, (((SeVoiceRecordsView *) gSeVoiceState)->voiceFlags >> 2) & 1,
                      ((SeVoiceRecordsView *) gSeVoiceState)->unk3F4, 0);
    }
    xglFontSetAlphaScale(128);
    if (((SeVoiceRecordsView *) gSeVoiceState)->fadePhase >= 49) {
        xglFontSetAlphaScale(((SeVoiceRecordsView *) gSeVoiceState)->fadePhase < 64
                                 ? (u8) ((((SeVoiceRecordsView *) gSeVoiceState)->fadePhase + 208) << 3)
                                 : 128);
        sheetDrawFrame(415);
        xglFontSetAlphaScale(128);
    }
    if (((SeVoiceRecordsView *) gSeVoiceState)->fadePhase >= 81) {
        xglFontSetAlphaScale(((SeVoiceRecordsView *) gSeVoiceState)->fadePhase < 96
                                 ? (u8) ((((SeVoiceRecordsView *) gSeVoiceState)->fadePhase + 176) << 3)
                                 : 128);
        mcStateSeekEntry(398 - sheetLatchedPositionGet());
    }
    xglFontSetAlphaScale(((SeVoiceRecordsView *) gSeVoiceState)->fadePhase < 16
                             ? (u8) (((SeVoiceRecordsView *) gSeVoiceState)->fadePhase << 3)
                             : 128);
    switch (((SeVoiceRecordsView *) gSeVoiceState)->mode) {
    case 0:
        func_0021ABB0(0, 11, messageTextGet(120), 0);
        break;
    default:
        func_0021ABB0(0, 11,
                      messageTextGet(
                          pagedTableGetCurrentEntry(((SeVoiceRecordsView *) gSeVoiceState)->pagedTable)),
                      0);
        break;
    case 7:
        func_0021ABB0(0, 11, messageTextGet(3), 0);
        break;
    }
    if (((SeVoiceRecordsView *) gSeVoiceState)->onFinish != 0) {
        ((SeVoiceRecordsView *) gSeVoiceState)->onFinish();
    }
    xglFontSetAlphaScale(128);
}

typedef struct TiFramePos {
    u16 x;
    u16 y;
} TiFramePos;

typedef struct TiFrameSheet {
    u8 unk00[0x4];
    union {
        const char *messageText;
        s32 messageValue;
    }; /* 0x04 */
    u8 unk08[0x4];
    s8 columns;     /* 0x0C */
    s8 rows;        /* 0x0D */
    u8 valueColumn; /* 0x0E */
    u8 unk0F;
    u16 originX;    /* 0x10 */
    u16 originY;    /* 0x12 */
    u16 cellWidthA; /* 0x14 */
    u16 cellWidthB; /* 0x16 */
    u16 cellWidthC; /* 0x18 */
    s16 entryCount; /* 0x1A */
    s16 frameBase;  /* 0x1C */
    u8 unk1E[0xE];
    u8 unk2C[0xC];
} TiFrameSheet;

typedef struct SeVoiceFrameView {
    u8 unk000[0x37C];
    u8 voiceKindCursor[2];
    u16 voiceKindCount;
    u8 unk380[0x38];
    u16 voiceTableCursor; /* 0x3B8 */
    u16 voiceTableCount;  /* 0x3BA */
    u8 unk3BC[0x32];
    u16 frameTickBase; /* 0x3EE */
    u8 unk3F0[0x4];
    u16 glyphTableCursor; /* 0x3F4 */
    u16 glyphTableCount;  /* 0x3F6 */
    u8 unk3F8[0x38];
    TiFrameSheet frame; /* 0x430 */
    u8 unk468[0xE8];
    u8 unk550[0x471];
    u8 rowMode;     /* 0x9C1 */
    s8 voiceFlags;  /* 0x9C2 */
    u8 bitRowIndex; /* 0x9C3 */
    u8 scaleMode;   /* 0x9C4 */
    u8 colourMode;  /* 0x9C5 */
    s16 rowStep;    /* 0x9C6 */
} SeVoiceFrameView;

s32 sheetFramePosition(TiFramePos *position, TiFrameSheet *sheet, s32 animationTick);
void sheetFrameDrawBox(TiFrameSheet *sheet);
void sheetFrameDrawText(TiFrameSheet *sheet, s32 animationTick, u32 color, s32 text);
void sheetFrameDrawValue(TiFrameSheet *sheet, s32 animationTick, s32 mask, s32 displayValue);
s32 Get_Rnd_001B8CD0(s32, s32);
void xglFontSetColorScale(u8 colorScale);
u32 colorBlendAverageAlpha(u32 colorA, u32 colorB, int alpha);
u8 statusByte0Get(void);
u8 statusByte1Get(void);
void glyphDrawByCode(s32 drawTarget, s32 glyphCode);
u8 pagedTableGetSlotByte(void *table, s32 entryIndex);
u16 pagedTableGetKeyAt(void *table, s32 entryIndex);
u8 pagedTableGetFlagByte(void *table, s32 entryIndex);
void xglFontPrintExtendedColor(s32 r, s32 g, s32 b);
void xglFontPrintEscapeByte(s32 value);
void xglFontSetColor(s32 r, s32 g, s32 b);
void xglFontPrintIndexed(s32 x, s32 y, const char *text);
s32 func_00222D48(s32, s32, s32);
s32 func_00222D18(s32, s32);
s32 func_00222848(s32, s32, s32);
s32 func_00223778(s32, s32);
void func_0021FCE0(TiFrameSheet *sheet);
void func_00220130(TiFrameSheet *sheet, s32 index, s32 mask);
void func_00221C18(TiFrameSheet *sheet, s32 index, s32 value);
void func_002215A0(void *table, s32 a, s32 b);
void func_0021FAB8(s32 a, s32 b, void *table);
void func_0021F260(s32 a, s32 b, s32 c, s32 d);
void func_0021EBD0(s32 a, s32 b, s32 c, s32 d);
s32 func_0021EA68(s32 a, s32 b, s32 c, s32 d);
void func_0021E9A8(s32 a, s32 b, s32 c, s32 d);
void fontDrawSprite16(s32 x, s32 y);
void fontDrawRotatedSpriteU232(TiFramePos *rect, s32 a, s32 b, s32 c);

/* Draws the voice list, the selected voice values and the row/step panel; when voiceFlags bit 2 is set it
 * also draws the glyph strip and the record table through seVoiceRecordTableDraw. */
void seVoicePanelDraw(s32 alpha) {
    TiFramePos pos;
    TiFramePos glyphPos;
    TiFramePos rectPos;
    s32 i;
    s32 randomBase;
    u32 colour;
    s32 voiceEntry;
    s32 entryOffset;
    s32 rowDirection;
    s32 rowValue;
    s32 clampedRow;
    s32 lookupSlot;
    s32 maxValue;
    s32 glyphFlag;
    s32 glyphEntry;
    s32 glyphEntryAgain;
    s32 voiceRow;
    s32 kindEntry;
    s32 tableEntry;
    s32 slotByte;
    s32 maxRow;

    randomBase = Get_Rnd_001B8CD0(10, 0);
    if (((SeVoiceFrameView *) gSeVoiceState)->voiceFlags & 1) {
        ((SeVoiceFrameView *) gSeVoiceState)->frame.messageText =
            ((const char *(*) (s32 id)) messageTextGet)(86);
    } else {
        ((SeVoiceFrameView *) gSeVoiceState)->frame.messageText =
            ((const char *(*) (s32 id)) messageTextGet)(85);
    }
    ((SeVoiceFrameView *) gSeVoiceState)->frame.entryCount =
        ((SeVoiceFrameView *) gSeVoiceState)->voiceTableCount;
    ((SeVoiceFrameView *) gSeVoiceState)->frame.frameBase =
        ((SeVoiceFrameView *) gSeVoiceState)->frameTickBase;
    fontQueueColorState(0x80808080);
    xglFontSetAlphaScale(alpha & 0xFF);
    func_0021FCE0(&((SeVoiceFrameView *) gSeVoiceState)->frame);
    sheetFrameDrawBox(&((SeVoiceFrameView *) gSeVoiceState)->frame);

    for (i = 0; i < ((SeVoiceFrameView *) gSeVoiceState)->voiceTableCount; i++) {
        colour = pagedTableGetSlotByte(&((SeVoiceFrameView *) gSeVoiceState)->voiceTableCursor, i);
        if (sheetFramePosition(&pos, &((SeVoiceFrameView *) gSeVoiceState)->frame, i) != 0) {
            slotByte = (colour < 0x11);
            colour = 0x80767B5D;
            if (slotByte) {
                colour = 0x80626262;
            }
            voiceEntry = (s32) ((void *(*) (void *table, s32 entryIndex)) pagedTableGetEntryAt)(
                &((SeVoiceFrameView *) gSeVoiceState)->voiceTableCursor, i);
            if (((SeVoiceFrameView *) gSeVoiceState)->rowMode == 0) {
                func_00221C18(
                    &((SeVoiceFrameView *) gSeVoiceState)->frame, i,
                    (pagedTableGetSlotByte(&((SeVoiceFrameView *) gSeVoiceState)->voiceTableCursor, i) >>
                     1) &
                        0xFF);
            } else {
                func_00220130(&((SeVoiceFrameView *) gSeVoiceState)->frame, i,
                              (i == ((SeVoiceFrameView *) gSeVoiceState)->voiceTableCursor) ? 0x5AF98980
                                                                                               : 0xF98980);
                if (i == ((SeVoiceFrameView *) gSeVoiceState)->voiceTableCursor) {
                    colour = 0x80785C5A;
                }
            }
            if (pagedTableGetKeyAt(&((SeVoiceFrameView *) gSeVoiceState)->voiceTableCursor, i) & 0x100) {
                colour = colorBlendAverageAlpha(colour, 0, 0x80);
            }
            sheetFrameDrawText(
                &((SeVoiceFrameView *) gSeVoiceState)->frame, i, colour,
                func_00222D48(((SeVoiceFrameView *) gSeVoiceState)->bitRowIndex, voiceEntry, 0));
            if (((SeVoiceFrameView *) gSeVoiceState)->voiceFlags & 1) {
                sheetFrameDrawValue(
                    &((SeVoiceFrameView *) gSeVoiceState)->frame, i, colour,
                    Get_Rnd_001B8CD0(((SeVoiceFrameView *) gSeVoiceState)->bitRowIndex, voiceEntry));
            } else {
                sheetFrameDrawValue(
                    &((SeVoiceFrameView *) gSeVoiceState)->frame, i, colour,
                    func_00222D18(
                        ((SeVoiceFrameView *) gSeVoiceState)->bitRowIndex,
                        func_00222C60(((SeVoiceFrameView *) gSeVoiceState)->bitRowIndex, voiceEntry)));
            }
        }
    }

    xglFontResetState();
    for (i = 0; i < ((SeVoiceFrameView *) gSeVoiceState)->voiceTableCount; i++) {
        if (sheetFramePosition(&pos, &((SeVoiceFrameView *) gSeVoiceState)->frame, i) != 0) {
            if ((((SeVoiceFrameView *) gSeVoiceState)->voiceFlags & 1) == 0) {
                voiceEntry = (s32) ((void *(*) (void *table, s32 entryIndex)) pagedTableGetEntryAt)(
                    &((SeVoiceFrameView *) gSeVoiceState)->voiceTableCursor, i);
                if (seVoiceBitIsClear(((SeVoiceFrameView *) gSeVoiceState)->bitRowIndex, voiceEntry) != 0) {
                    fontQueueColorState(((unsigned long long) (80 - ((statusByte1Get() >> 1) & 0xFF)) << 24) |
                                        0x1E6A69);
                    xglFontDrawStringScaled((pos.x + 0xFF00) & 0xFFFF, (pos.y + 0x40) & 0xFFFF,
                                  (s32) ((const char *(*) (s32 id)) messageTextGet)(122), 14);
                }
            }
        }
    }

    voiceEntry = 0;
    entryOffset = 0;
    if (((SeVoiceFrameView *) gSeVoiceState)->voiceTableCount != 0) {
        voiceEntry = pagedTableGetCurrentEntry(&((SeVoiceFrameView *) gSeVoiceState)->voiceTableCursor);
        entryOffset = Get_Rnd_001B8CD0(((SeVoiceFrameView *) gSeVoiceState)->bitRowIndex, voiceEntry);
        if (!(((SeVoiceFrameView *) gSeVoiceState)->voiceFlags & 1)) {
            if (((SeVoiceFrameView *) gSeVoiceState)->voiceFlags & 2) {
                entryOffset +=
                    func_00223778(((SeVoiceFrameView *) gSeVoiceState)->bitRowIndex, voiceEntry);
            }
        }
    }

    lookupSlot = seVoiceLookupSlot();
    if (((SeVoiceFrameView *) gSeVoiceState)->bitRowIndex == 3) {
        fontQueueColorState(0x80284680);
    } else {
        fontQueueColorState(0x80626262);
    }
    if (((SeVoiceFrameView *) gSeVoiceState)->voiceFlags & 4) {
        func_0021EBD0(
            16, 298, func_00222D48(((SeVoiceFrameView *) gSeVoiceState)->bitRowIndex, voiceEntry, 1), 253);
    } else {
        func_0021EA68(
            16, 298, func_00222D48(((SeVoiceFrameView *) gSeVoiceState)->bitRowIndex, voiceEntry, 1), 253);
    }
    if (((SeVoiceFrameView *) gSeVoiceState)->scaleMode == 2) {
        xglFontSetColorScale(64);
    }
    seVoiceInfoBoxDraw(305, 264);
    seVoiceInfoBoxDraw(305, 327);
    fontQueueColorState(0x80545934);

    if ((((SeVoiceFrameView *) gSeVoiceState)->voiceFlags & 1) &&
        ((SeVoiceFrameView *) gSeVoiceState)->rowMode == 0) {
        xglFontDrawStringScaled(0x8378, 0x82F8, (s32) ((const char *(*) (s32 id)) messageTextGet)(85), 13);
    } else {
        xglFontDrawStringScaled(0x8378, 0x82F8, (s32) ((const char *(*) (s32 id)) messageTextGet)(86), 13);
    }
    xglFontDrawStringScaled(0x8378, 0x86E8, (s32) ((const char *(*) (s32 id)) messageTextGet)(87), 13);
    if (((SeVoiceFrameView *) gSeVoiceState)->rowMode != 0) {
        xglFontDrawStringScaled(0x8378, 0x8538, (s32) ((const char *(*) (s32 id)) messageTextGet)(88), 13);
        xglFontDrawStringScaled(0x8378, 0x8928, (s32) ((const char *(*) (s32 id)) messageTextGet)(89), 13);
    }

    fontQueueColorState(0x80626262);
    if ((((SeVoiceFrameView *) gSeVoiceState)->voiceFlags & 1) &&
        ((SeVoiceFrameView *) gSeVoiceState)->rowMode == 0) {
        if (voiceEntry != 0) {
            xglFontDrawDigits(
                0x8DD8, 0x82C8,
                func_00222D18(((SeVoiceFrameView *) gSeVoiceState)->bitRowIndex, lookupSlot) >> 1, 16);
        }
    } else if (voiceEntry != 0) {
        xglFontDrawDigits(0x8998, 0x82C8, entryOffset, 16);
    }
    xglFontDrawDigits(0x8DD8, 0x86B8, randomBase, 16);

    if (((SeVoiceFrameView *) gSeVoiceState)->rowMode != 0) {
        rowDirection = 1;
        rowValue = func_00222D18(((SeVoiceFrameView *) gSeVoiceState)->bitRowIndex, lookupSlot);
        maxRow = func_00237CB0(((SeVoiceFrameView *) gSeVoiceState)->bitRowIndex, voiceEntry);
        if (((SeVoiceFrameView *) gSeVoiceState)->voiceFlags & 1) {
            rowDirection = -1;
            rowValue >>= 1;
        }
        clampedRow = ((SeVoiceFrameView *) gSeVoiceState)->rowStep * rowDirection + entryOffset;
        if (clampedRow < 0) {
            clampedRow = 0;
        } else if (maxRow < clampedRow) {
            clampedRow = maxRow;
        }
        xglFontPrintExtendedColor(0, 0, 0);
        xglFontPrintEscapeByte(13);
        xglFontSetColor(98, 98, 98);
        xglFontDrawDigits(0x8998, 0x8518, clampedRow, 16);
        maxValue = randomBase - rowValue * ((SeVoiceFrameView *) gSeVoiceState)->rowStep * rowDirection;
        if (maxValue > 0x5F5E0FF) {
            maxValue = 0x5F5E0FF;
        }
        xglFontDrawDigits(0x8DD8, 0x8908, maxValue, 16);
        xglFontPrintIndexed(
            371, 351, (((SeVoiceFrameView *) gSeVoiceState)->voiceFlags & 1) ? "+" : "-");
        xglFontDrawDigits(0x8DD8, 0x87E8, rowValue * ((SeVoiceFrameView *) gSeVoiceState)->rowStep, 16);
        if (((SeVoiceFrameView *) gSeVoiceState)->rowMode == 1) {
            fontQueueColorState(((unsigned long long) (((statusByte0Get() >> 2) & 0xFF) + 32) << 24) |
                                0xE3FF5F);
        } else {
            fontQueueColorState(0x73F98980);
        }
        func_0021E9A8(0x8588, 0x83D8, 1600, 304);
        if (((SeVoiceFrameView *) gSeVoiceState)->rowMode == 1) {
            fontQueueColorState(0x80767B5D);
        } else {
            fontQueueColorState(0x80785C5A);
        }
        xglFontDrawDigits(0x8998, 0x83F8, ((SeVoiceFrameView *) gSeVoiceState)->rowStep, 16);
        if (((SeVoiceFrameView *) gSeVoiceState)->rowMode == 1) {
            xglFontSetColor(93, 123, 118);
        } else {
            xglFontSetColor(90, 92, 120);
        }
        xglFontPrintIndexed(
            371, 288, (((SeVoiceFrameView *) gSeVoiceState)->voiceFlags & 1) ? "-" : "+");
    }

    if (((SeVoiceFrameView *) gSeVoiceState)->voiceFlags & 2) {
        glyphPos.y = 0x7498;
        {
            s32 left = 0x80F8 + ((7 - ((SeVoiceFrameView *) gSeVoiceState)->glyphTableCount) << 8);
            glyphPos.x = left;
        }
        for (i = 0; i < ((SeVoiceFrameView *) gSeVoiceState)->glyphTableCount; i++) {
            glyphFlag = pagedTableGetFlagByte(&((SeVoiceFrameView *) gSeVoiceState)->glyphTableCursor, i);
            fontQueueColorState((glyphFlag | (((unsigned long long) glyphFlag << 16) |
                                              ((unsigned long long) glyphFlag << 8))) |
                                ((unsigned long long) 0x80 << 24));
            glyphEntry = (s32) ((void *(*) (void *table, s32 entryIndex)) pagedTableGetEntryAt)(
                &((SeVoiceFrameView *) gSeVoiceState)->glyphTableCursor, i);
            glyphDrawByCode((s32) &glyphPos, glyphEntry);
            glyphEntryAgain = (s32) ((void *(*) (void *table, s32 entryIndex)) pagedTableGetEntryAt)(
                &((SeVoiceFrameView *) gSeVoiceState)->glyphTableCursor, i);
            if (func_00222848(glyphEntryAgain, ((SeVoiceFrameView *) gSeVoiceState)->bitRowIndex,
                              voiceEntry) != 0) {
                fontQueueColorState(0x80808080);
                fontDrawSprite16(glyphPos.x, glyphPos.y);
            }
            if (((SeVoiceFrameView *) gSeVoiceState)->colourMode >= 2) {
                slotByte =
                    pagedTableGetSlotByte(&((SeVoiceFrameView *) gSeVoiceState)->glyphTableCursor, i);
                fontQueueColorState(((unsigned long long) (u8) slotByte << 24) | 0x808080);
                rectPos.x = glyphPos.x + 0x70;
                rectPos.y = glyphPos.y - 0x100;
                fontDrawRotatedSpriteU232(&rectPos, 2, 304, 384);
            }
            glyphPos.x += 0x200;
        }
        voiceRow = pagedTableGetCurrentEntry(&((SeVoiceFrameView *) gSeVoiceState)->glyphTableCursor);
        kindEntry = pagedTableGetCurrentEntry(&((SeVoiceFrameView *) gSeVoiceState)->voiceKindCursor);
        if (((SeVoiceFrameView *) gSeVoiceState)->voiceTableCount != 0) {
            tableEntry =
                pagedTableGetCurrentEntry(&((SeVoiceFrameView *) gSeVoiceState)->voiceTableCursor);
        } else {
            tableEntry = 0;
        }
        seVoiceRecordTableDraw(voiceRow, kindEntry, tableEntry);
        func_0021FAB8(142, 47, &((SeVoiceFrameView *) gSeVoiceState)->voiceKindCursor);
    }

    xglFontResetState();
    func_002215A0(&((SeVoiceFrameView *) gSeVoiceState)->frame.unk2C, 0, 174);
    if (((SeVoiceFrameView *) gSeVoiceState)->scaleMode == 2) {
        xglFontSetColorScale(128);
        if (((SeVoiceFrameView *) gSeVoiceState)->voiceTableCount == 0) {
            func_0021F260(272, 42, ((SeVoiceFrameView *) gSeVoiceState)->bitRowIndex, 0);
        } else {
            func_0021F260(
                272, 42, ((SeVoiceFrameView *) gSeVoiceState)->bitRowIndex,
                pagedTableGetCurrentEntry(&((SeVoiceFrameView *) gSeVoiceState)->voiceTableCursor));
        }
    }
}

s32 func_00222598(u8 *, s32);
s32 pagedTableSetSlotByte(u8 *, u16, s32);

typedef struct ShopTable {
    u8 entries[0x200];
    u8 counts[8];
    u8 offsets[8];
} ShopTable;

/* Table at gSeMenuBlock, read as ShopTable (entries, counts, offsets) by the voice list builder. */
#define voiceShopTable ((ShopTable *) gSeMenuBlock)

extern u8 D_003E71C0[];

typedef struct KeyRec {
    u8 unk00[4];
    u16 flags4;
    u16 mask6;
} KeyRec;

static inline void tableSetFlags(VoiceTable *table, s32 flags) {
    table->unk20 |= flags;
}

typedef struct SeVoiceRef {
    SeVoiceState *state;
} SeVoiceRef;

static inline SeVoiceState *voiceState(void) {
    return ((SeVoiceRef *) &gSeVoiceState)->state;
}

void pagedTableSeek(VoiceTable *table, s32 cursor);

/* Builds the voice list for the current voice kind and appends the keys the flags allow. The cursor is
 * restored when restoreFlag is 0; unusedArg is kept because callers pass it. */
void seVoiceListBuild(s32 unusedArg, s32 restoreFlag) {
    s32 savedCursor = 0;
    s32 savedTop = 0;
    s32 money;
    s32 voiceListSize;
    u8 *voiceListItems;
    s32 i;
    s32 voiceMenuMode;

    money = Get_Rnd_001B8CD0(10, 0);
    switch ((u8) gSeVoiceState->voiceFlags) {
    default:
    case 0:
    case 4:
    case 5:
        gSeVoiceState->bitRowIndex = 0;
        voiceListSize = voiceShopTable->counts[gSeVoiceState->bitRowIndex];
        voiceListItems = &voiceShopTable
                    ->entries[voiceShopTable->offsets[gSeVoiceState->bitRowIndex]];
        break;
    case 1:
        gSeVoiceState->bitRowIndex = 0;
        voiceListSize = func_00222598(gSeVoiceState->unk668, gSeVoiceState->bitRowIndex);
        voiceListItems = gSeVoiceState->unk668;
        break;
    case 2:
    case 6:
        gSeVoiceState->bitRowIndex = slotKindResolve(pagedTableGetCurrentEntry(&gSeVoiceState->voiceKind));
        voiceListSize = voiceShopTable->counts[gSeVoiceState->bitRowIndex];
        voiceListItems = &voiceShopTable
                    ->entries[voiceShopTable->offsets[gSeVoiceState->bitRowIndex]];
        break;
    case 3:
    case 7:
        gSeVoiceState->bitRowIndex = slotKindResolve(pagedTableGetCurrentEntry(&gSeVoiceState->voiceKind));
        voiceListSize = func_00222598(gSeVoiceState->unk668, gSeVoiceState->bitRowIndex);
        voiceListItems = gSeVoiceState->unk668;
        break;
    }
    if (restoreFlag == 0) {
        savedCursor = gSeVoiceState->voiceTable.cursor;
        savedTop = gSeVoiceState->voiceTable.topRow;
    }
    if (gSeVoiceState->voiceFlags & 2) {
        voiceMenuMode = (gSeVoiceState->voiceFlags & 4) ? 7 : 8;
    } else {
        voiceMenuMode = 10;
    }
    pagedTableSetPageSizes(&gSeVoiceState->voiceTable, voiceMenuMode,
                                                (gSeVoiceState->voiceFlags & 2) == 0 ? 0x30 : 0x20);
    for (i = 0; i < voiceListSize; i++) {
        s32 itemKey = voiceListItems[i];
        s32 appendFlags = 0;
        s32 bitRow = gSeVoiceState->bitRowIndex;
        s32 ownedLimit = func_00237CB0(bitRow, itemKey);

        if (bitRow == 3) {
            KeyRec *keyRec = (KeyRec *) func_00222C60(3, itemKey);

            if (keyRec->mask6 != D_003E71C0[pagedTableGetCurrentEntry(&gSeVoiceState->voiceKind) - 2]) {
                continue;
            }
        }
        if (gSeVoiceState->bitRowIndex == 1) {
            if (itemKey >= 108) {
                if (gSeVoiceState->voiceFlags & 4) {
                    goto checkKind;
                }
                continue;
            } else {
                if (!(gSeVoiceState->voiceFlags & 4)) {
                    goto append;
                }
                continue;
            }
        checkKind:
            if (pagedTableGetCurrentEntry(&gSeVoiceState->voiceKind) == 5 && func_00223F70(itemKey)[0] != 1) {
                continue;
            }
            if (pagedTableGetCurrentEntry(&gSeVoiceState->voiceKind) == 6 && func_00223F70(itemKey)[0] != 0) {
                continue;
            }
        }
    append:
        if (gSeVoiceState->voiceFlags & 1) {
            if (gSeVoiceState->bitRowIndex == 0) {
                if (((KeyRec *) func_00222C60(0, itemKey))->flags4 & 0x20) {
                    continue;
                }
            } else {
                if (((KeyRec *) func_00222C60(gSeVoiceState->bitRowIndex, itemKey))->flags4 & 0x20) {
                    continue;
                }
            }
        } else {
            s32 ownedCount = Get_Rnd_001B8CD0(gSeVoiceState->bitRowIndex, itemKey);

            if (gSeVoiceState->voiceFlags & 2) {
                ownedCount += func_00223778(gSeVoiceState->bitRowIndex, itemKey);
            }
            if (money < func_00222D18(gSeVoiceState->bitRowIndex,
                                      func_00222C60(gSeVoiceState->bitRowIndex, itemKey)) ||
                ownedCount >= ownedLimit) {
                appendFlags = 0x100;
            }
            seVoiceTemplateBitSet(gSeVoiceState->bitRowIndex, itemKey);
        }
        pagedTableAppend(&gSeVoiceState->voiceTable, itemKey, appendFlags | 2);
    }
    pagedTableSetTextBox(&gSeVoiceState->voiceTable, 0, 0xFD, 0x19);
    tableSetFlags(&voiceState()->voiceTable, 0x4C);
    voiceState()->unk430.unk12 = (voiceState()->voiceFlags & 2) ? 0x77A8 : 0x74E8;
    voiceState()->unk430.unk0D =
        (voiceState()->voiceFlags & 2) ? ((voiceState()->voiceFlags & 4) ? 7 : 8) : 10;
    if (restoreFlag == 0) {
        gSeVoiceState->voiceTable.topRow = savedTop;
        pagedTableSeek(&gSeVoiceState->voiceTable, savedCursor);
        pagedTableSetSlotByte(&gSeVoiceState->voiceTable, gSeVoiceState->voiceTable.cursor, 0x80);
    }
    if (gSeVoiceState->voiceFlags & 2) {
        seVoicePlayAndSetup();
    }
}

typedef struct VoiceSlotView {
    u8 unk00[0xC];
    u16 mask;
} VoiceSlotView;

static inline void voiceTableSetFlag(VoiceTable *table, s32 index, s32 flag) {
    if (table->capacity != 0) {
        table->entryKeys[index].flags |= flag;
    }
}

static inline void voiceTableClearFlag(VoiceTable *table, s32 index, s32 flag) {
    if (table->capacity != 0) {
        table->entryKeys[index].flags &= ~flag;
    }
}

/* Plays the voice movie of the current kind, then flags the voice entries that match the slot mask
 * and posts the matching setup request. */
void seVoicePlayAndSetup(void) {
    s32 playTitleText;
    s32 voiceKindEntry;
    s32 voiceSlot;
    s32 i;
    s32 filledEntry;
    s32 hasMatch;
    s32 firstMatchIndex;
    s32 seekPending;
    s32 setupFlags;

    hasMatch = 0;
    firstMatchIndex = 0;
    seekPending = 1;
    playTitleText = messageTextGet((gSeVoiceState->voiceFlags & 1) ? 0x79 : 0x77);
    voiceKindEntry = pagedTableGetCurrentEntry(&gSeVoiceState->voiceKind);
    GameMpeg2Play(
        &gSeVoiceState->cdPath[0x80], playTitleText, messageTextGet(voiceKindEntry + 0x7B));
    sheetSetIndex(0);
    sheetSetIndex((s32) &gSeVoiceState->cdPath[0x80]);
    gSeVoiceState->matchCount = 0;
    voiceSlot = seVoiceLookupSlot();
    if (voiceSlot == 0) {
        for (i = 0; i < gSeVoiceState->unk3F4.filledEntryCount; i++) {
            voiceTableSetFlag(&gSeVoiceState->unk3F4, i, 0x100);
        }
    }

    for (i = 0; i < gSeVoiceState->unk3F4.filledEntryCount; i++) {
        filledEntry = pagedTableGetEntryAt(&gSeVoiceState->unk3F4, i);
        voiceTableSetFlag(&gSeVoiceState->unk3F4, i, 0x100);
        if (filledEntry != 0 && (filledEntry = 1 << (filledEntry - 1), voiceSlot != 0) &&
            (filledEntry & ((VoiceSlotView *) voiceSlot)->mask)) {
            voiceTableClearFlag(&gSeVoiceState->unk3F4, i, 0x100);
            if (!hasMatch) {
                firstMatchIndex = i;
                hasMatch = 1;
            }
            if (i == gSeVoiceState->unk3F4.cursor) {
                seekPending = 0;
            }
            gSeVoiceState->matchCount++;
        }
    }
    if (hasMatch != 0 && seekPending != 0) {
        pagedTableSeek(&gSeVoiceState->unk3F4, (u16) firstMatchIndex);
    }

    setupFlags = (gSeVoiceState->matchCount >= 2) ? 15 : 7;
    if (gSeVoiceState->bitRowIndex == 3 || gSeVoiceState->bitRowIndex == 7) {
        if (gSeVoiceState->stateFlag == 0) {
            gSeVoiceState->stateFlag = 1;
        }
        if (gSeVoiceState->stateFlag == 2) {
            mcStateSetupRequest(setupFlags, 0);
            return;
        }
        mcStateSetupRequest(setupFlags, messageTextGet(0x21));
        return;
    }
    gSeVoiceState->stateFlag = 0;
    mcStateSetupRequest(setupFlags, 0);
}

s32 battleRosterEntryIdGet(s32, s32);
void func_002213A8(void *, const char *, s32, s32);

static inline s32 voicePartyIndex(void) {
    if (gSeVoiceState->voiceFlags & 4) {
        return 1;
    }
    return 0;
}

/* Enters a voice scene: resets the stepped targets, refills the voice lists when the voice flags ask for it,
 * then sets the scene message and its setup. */
void seVoiceSceneEnter(s32 scene) {
    s32 i;
    s32 voiceListSize;
    s32 savedCursor;
    s32 entryId;

    gSeVoiceState->scene = scene;
    setSteppedTarget(&gSeVoiceState->unk430.unk20, 0);
    setSteppedTarget(&gSeVoiceState->unk430.unk28, 0);
    setSteppedTarget(&gSeVoiceState->unk430.unk24, 0);
    if ((gSeVoiceState->voiceFlags & 2) || scene == 7) {
        voiceListSize = battleRosterEntryIdCountGet(voicePartyIndex());
        savedCursor = gSeVoiceState->unk3F4.cursor;
        pagedTableSetPageSizes(&gSeVoiceState->unk3F4, 10, 0);
        for (i = 0; i < voiceListSize; i++) {
            entryId = battleRosterEntryIdGet(i, voicePartyIndex());
            if ((unsigned int) (entryId - 8) >= 5 || scene == 7) {
                pagedTableAppend(&gSeVoiceState->unk3F4, entryId, 2);
            }
        }
        pagedTableSeek(&gSeVoiceState->unk3F4, savedCursor);
        if (scene == 7) {
            seVoiceKindMarkEntries();
        }
        voiceListSize = voicePartyIndex() + 5;
        savedCursor = gSeVoiceState->voiceKind.cursor;
        pagedTableSetPageSizes(&gSeVoiceState->voiceKind, 10, 0x188);
        for (i = 0; i < voiceListSize; i++) {
            pagedTableAppend(&gSeVoiceState->voiceKind, (gSeVoiceState->voiceFlags & 4) ? i + 5 : i, 2);
        }
        pagedTableSeek(&gSeVoiceState->voiceKind, savedCursor);
    }
    switch (scene) {
    case 0:
        setSteppedTarget(&gSeVoiceState->unk430.unk20, 0x80);
        sheetSetIndex(messageTextGet(108));
        mcStateSetupRequest(7, 0);
        return;
    case 7:
        gSeVoiceState->unk5FA = 0;
        seVoiceUnitSetMode(2);
        return;
    case 1:
    case 2:
    case 3:
    default:
        setSteppedTarget(&gSeVoiceState->unk430.unk24, 0x80);
        seVoiceListBuild(scene, 1);
        seVoicePlayAndSetup();
        gSeVoiceState->unk9C1 = 0;
        func_002213A8(&gSeVoiceState->unk430.unk2C, messageTextGet(114), 0, 201);
        if (!(gSeVoiceState->voiceFlags & 2)) {
            sheetSetIndex(messageTextGet(109));
        }
        return;
    case 4:
    case 5:
    case 6:
        setSteppedTarget(&gSeVoiceState->unk430.unk24, 0x80);
        seVoiceListBuild(scene, 1);
        seVoicePlayAndSetup();
        gSeVoiceState->unk9C1 = 0;
        func_002213A8(&gSeVoiceState->unk430.unk2C, messageTextGet(115), 0, 201);
        if (!(gSeVoiceState->voiceFlags & 2)) {
            sheetSetIndex(messageTextGet(111));
        }
        break;
    }
}

typedef struct VoiceTableView {
    u16 cursor;
    u16 filledEntryCount;
    u16 unk04;
    u16 unk06;
    u8 unk08[0x18];
    s32 unk20;
    void **entryPointers;
    VoiceKey *entryKeys;
    u8 unk2C[0x8];
    u16 capacity;
    u8 unk36[0x6];
} VoiceTableView;

typedef struct SeVoiceScreenView {
    u8 unk000[0x340];
    VoiceTableView pagedCursor;
    u8 voiceKindCursor[2];
    u16 voiceKindCount;
    u8 unk380[0x38];
    u8 voiceTableCursor[2];
    u16 voiceTableCount;
    u8 unk3BC[0x38];
    VoiceTableView unk3F4;
    u8 unk430[0x20];
    u8 stepValue0[4];
    u8 stepValue1[4];
    u8 stepValue2[4];
    u8 clampedCounter[8];
    u8 restartRequest;
    u8 unk465[3];
    u8 unk468[0xE8];
    u8 unk550[0x298];
    u8 sheetBlock[0x1D8];
    u8 phase;
    u8 subState;
    s8 voiceFlags;
    u8 voiceKind;
    u8 stage;
    u8 timerMode;
    s16 timer;
    u16 level;
    u8 unk9CA;
    s8 endFlag;
    u8 resetFlag;
} SeVoiceScreenView;

typedef struct SeStatusHeader {
    u8 unk00[2];
    u16 headerFlags;
    u16 extFlags;
} SeStatusHeader;

s32 func_00221440(void *, s32);
void pagedTableStepCursorUnmasked(void *, s32, s32);
void func_00220E00(void);
void func_002209C0(void);
void func_001B8D08(s32, s32, s32);
s32 clampedCounterInit(void *, s32);

typedef struct SeVoiceRefView {
    SeVoiceScreenView *state;
} SeVoiceRefView;

static inline SeVoiceScreenView *voiceStateView(void) {
    return ((SeVoiceRefView *) &(*(SeVoiceScreenView **) &gSeVoiceState))->state;
}

/* One frame of the voice screen state machine: phase, substate, timer and the fade level. */
void seVoiceScreenUpdate(void) {
    s32 pendingCount = 0;
    s32 selectedKey = 0;
    s32 unitPrice = 0;
    s32 ownedBase = 0;
    s32 resolvedSlot;
    s32 subState;
    s16 oldTimer;
    s32 entryKey;
    s32 ownedLimit = 99;

    switch (voiceStateView()->phase) {
    case 0:
        pagedTableUpdate(&voiceStateView()->pagedCursor, &pendingCount);
        pagedTableAnimateSlots(&voiceStateView()->pagedCursor);
        if (pendingCount > 0) {
            entryKey = pagedTableGetCurrentEntry(&voiceStateView()->pagedCursor);
            switch (entryKey) {
            case 100:
            default:
                voiceStateView()->voiceFlags = 0;
                seVoiceStartDefault(2);
                seVoiceSceneEnter(1);
                break;
            case 101:
                voiceStateView()->voiceFlags = 2;
                seVoiceStartDefault(2);
                seVoiceSceneEnter(2);
                break;
            case 102:
                voiceStateView()->voiceFlags = 6;
                seVoiceStartDefault(2);
                seVoiceSceneEnter(3);
                break;
            case 103:
                voiceStateView()->voiceFlags = 1;
                seVoiceStartDefault(2);
                seVoiceSceneEnter(4);
                break;
            case 104:
                voiceStateView()->voiceFlags = 3;
                seVoiceStartDefault(2);
                seVoiceSceneEnter(5);
                break;
            case 105:
                voiceStateView()->voiceFlags = 7;
                seVoiceStartDefault(2);
                seVoiceSceneEnter(6);
                break;
            case 106:
            case 118:
                voiceStateView()->voiceFlags = 2;
                if (entryKey == 118) {
                    voiceStateView()->voiceFlags |= 4;
                }
                seVoiceStartDefault(2);
                seVoiceSceneEnter(7);
                break;
            case 107:
                seVoiceStartDefault(7);
                voiceStateView()->endFlag = 1;
                break;
            }
        } else if (getStatusHeader()->headerFlags & 0x40) {
            seVoiceStartDefault(7);
            voiceStateView()->endFlag = 1;
        }
        break;
    case 7:
        seVoiceUnitUpdate();
        break;
    default:
        if (voiceStateView()->voiceTableCount != 0) {
            selectedKey = pagedTableGetCurrentEntry(voiceStateView()->voiceTableCursor);
            resolvedSlot = func_00222C60(voiceStateView()->voiceKind, selectedKey);
            ownedBase = Get_Rnd_001B8CD0(voiceStateView()->voiceKind, selectedKey);
            unitPrice = func_00222D18(voiceStateView()->voiceKind, resolvedSlot);
            ownedLimit = func_00237CB0(voiceStateView()->voiceKind, selectedKey);
            if (!(voiceStateView()->voiceFlags & 1)) {
                if (voiceStateView()->voiceFlags & 2) {
                    ownedBase += func_00223778(voiceStateView()->voiceKind, selectedKey);
                }
            }
        }
        if (voiceStateView()->voiceFlags & 2) {
            pagedTableAnimateSlots(&voiceStateView()->unk3F4);
            pagedTableAnimateSlots(voiceStateView()->voiceKindCursor);
        }
        subState = voiceStateView()->subState;
        switch (subState) {
        case 0:
        default: {
            func_00221440(voiceStateView()->clampedCounter, 0);
            if (pagedTableUpdate(voiceStateView()->voiceTableCursor, &pendingCount) != 0) {
                if (voiceStateView()->voiceFlags & 2) {
                    seVoicePlayAndSetup();
                }
            }
            pagedTableAnimateSlots(voiceStateView()->voiceTableCursor);
            if (pendingCount > 0) {
                if (voiceStateView()->voiceTableCount != 0) {
                    voiceStateView()->subState = 1;
                    voiceStateView()->timer = 1;
                    voiceStateView()->stage = 0;
                    sheetSetIndex(
                        messageTextGet((voiceStateView()->voiceFlags & 1) ? 112 : 110));
                    mcStateSetupRequest(7, 0);
                    seVoiceStartDefault(2);
                } else {
                    seVoiceStartDefault(3);
                }
            } else if (pendingCount < 0) {
                if (voiceStateView()->stage == 2) {
                    voiceStateView()->stage = 1;
                    seVoicePlayAndSetup();
                } else {
                    seVoiceSceneEnter(0);
                }
            } else if (voiceStateView()->voiceFlags & 2) {
                if (pagedTableUpdate(voiceStateView()->voiceKindCursor, &pendingCount) != 0) {
                    if (pendingCount == 0) {
                        seVoiceListBuild(voiceStateView()->phase, 0);
                        seVoicePlayAndSetup();
                        seVoiceStartDefault(1);
                    }
                }
                if (voiceStateView()->timerMode >= 2) {
                    if (getStatusHeader()->headerFlags & 4) {
                        pagedTableStepCursorUnmasked(&voiceStateView()->unk3F4, -1, 256);
                    }
                    if (getStatusHeader()->headerFlags & 8) {
                        pagedTableStepCursorUnmasked(&voiceStateView()->unk3F4, 1, 256);
                    }
                    if (getStatusHeader()->headerFlags & 0xc) {
                        seVoiceStartDefault(1);
                    }
                }
                if (voiceStateView()->stage == 1) {
                    if (getStatusHeader()->headerFlags & 0x80) {
                        voiceStateView()->stage = 2;
                        seVoiceStartDefault(6);
                        seVoicePlayAndSetup();
                    }
                }
            }
        } break;
        case 1: {
            s32 roll;
            s32 remainingSpan;
            func_00221440(voiceStateView()->clampedCounter, 0);
            if (statusHeaderIsReadyMode40()) {
                seVoiceListBuild(voiceStateView()->phase, 0);
                voiceStateView()->subState = 0;
                seVoiceStartDefault(8);
                seVoicePlayAndSetup();
                if ((voiceStateView()->voiceFlags & 2) == 0) {
                    sheetSetIndex(
                        messageTextGet((voiceStateView()->voiceFlags & 1) ? 111 : 109));
                } else {
                    sheetSetIndex(&voiceStateView()->sheetBlock);
                }
            } else if (statusHeaderIsReadyMode20()) {
                voiceStateView()->subState = 2;
                clampedCounterInit(voiceStateView()->clampedCounter, 0);
                sheetSetIndex(messageTextGet(113));
                seVoiceStartDefault(2);
            } else {
                oldTimer = voiceStateView()->timer;
                if (((SeStatusHeader *) getStatusHeader())->extFlags & 0x1000) {
                    voiceStateView()->timer += 1;
                }
                if (((SeStatusHeader *) getStatusHeader())->extFlags & 0x4000) {
                    voiceStateView()->timer -= 1;
                }
                if (((SeStatusHeader *) getStatusHeader())->extFlags & 0x2000) {
                    voiceStateView()->timer += 10;
                }
                if (((SeStatusHeader *) getStatusHeader())->extFlags & 0x8000) {
                    voiceStateView()->timer -= 10;
                }
                if (voiceStateView()->timer <= 0) {
                    voiceStateView()->timer = subState;
                }
                if (voiceStateView()->voiceFlags & 1) {
                    if (ownedBase < voiceStateView()->timer) {
                        voiceStateView()->timer = ownedBase;
                    }
                } else {
                    remainingSpan = ownedLimit - ownedBase;
                    roll = Get_Rnd_001B8CD0(10, 0);
                    if (remainingSpan < voiceStateView()->timer) {
                        voiceStateView()->timer = remainingSpan;
                    }
                    if (unitPrice != 0 && roll < unitPrice * voiceStateView()->timer) {
                        voiceStateView()->timer = roll / unitPrice;
                    }
                }
                if (oldTimer != voiceStateView()->timer) {
                    seVoiceStartDefault(1);
                } else if (getStatusHeader()->headerFlags & 0xf000) {
                    seVoiceStartDefault(3);
                }
            }
        } break;
        case 2: {
            s32 roll;
            s32 remainingSpan;
            if (func_00221440(voiceStateView()->clampedCounter, 1) != 0) {
                if (voiceStateView()->restartRequest == 0) {
                    roll = Get_Rnd_001B8CD0(voiceStateView()->voiceKind, selectedKey);
                    if (voiceStateView()->voiceFlags & 1) {
                        roll -= voiceStateView()->timer;
                        if (roll < 0) {
                            roll = 0;
                        }
                        func_001B8D08(voiceStateView()->voiceKind, selectedKey, roll);
                        {
                            s32 price = unitPrice >> 1;

                            func_00224600(price * voiceStateView()->timer);
                        };
                    } else {
                        roll += voiceStateView()->timer;
                        if (roll > ownedLimit) {
                            roll = ownedLimit;
                        }
                        func_001B8D08(voiceStateView()->voiceKind, selectedKey, roll);
                        {
                            s32 price = -unitPrice;

                            func_00224600(price * voiceStateView()->timer);
                        };
                    }
                    seVoiceListBuild(voiceStateView()->phase, 0);
                    voiceStateView()->subState = 0;
                    seVoiceStartDefault(29);
                    if ((voiceStateView()->voiceFlags & 2) == 0) {
                        sheetSetIndex(messageTextGet(
                            (voiceStateView()->voiceFlags & 1) ? 111 : 109));
                    } else {
                        sheetSetIndex(&voiceStateView()->sheetBlock);
                    }
                    voiceStateView()->resetFlag = 1;
                } else {
                    voiceStateView()->subState = 1;
                    sheetSetIndex(
                        messageTextGet((voiceStateView()->voiceFlags & 1) ? 112 : 110));
                    seVoiceStartDefault(8);
                }
            }
        } break;
        }
        break;
    }

    stepValueTowardTarget(voiceStateView()->stepValue0);
    stepValueTowardTarget(voiceStateView()->stepValue1);
    stepValueTowardTarget(voiceStateView()->stepValue2);
    func_00220E00();
    if (voiceStateView()->level < 255) {
        voiceStateView()->level += gVsyncsPerFrame << 1;
        if (voiceStateView()->level == 80) {
            xglPadSetRepeat(0, 0xf000, 10, 3);
        }
    }
    if (voiceStateView()->level >= 65) {
        func_002209C0();
    }
    if (getStatusHeader()->headerFlags & 0x10) {
        seVoiceStartDefault(7);
        voiceStateView()->endFlag = 1;
    }
}

s32 statusHeaderTestFlags();
void sefFlagSetBit26();
void swapWordsSendCommand();
void completionCallbackAdd();
void swapWordsRestore();

/* Runs the voice screen until endFlag is set, sends the end command and returns resetFlag. */
s32 seVoiceScreenRun(void) {
    s32 commandWords[5];

    do {
        seVoiceScreenUpdate();
        if (statusHeaderTestFlags() != 0) {
            sefFlagSetBit26();
            ((SeVoiceScreenView *) gSeVoiceState)->endFlag = 2;
        }
        if (((SeVoiceScreenView *) gSeVoiceState)->endFlag != 0) {
            swapWordsSendCommand(((SeVoiceScreenView *) gSeVoiceState)->endFlag == 2 ? 30 : 5);
        }
        completionCallbackAdd(seVoiceScreenDraw);
        ((void (*)()) statusBlockFrameUpdate)();
    } while (((SeVoiceScreenView *) gSeVoiceState)->endFlag == 0);
    swapWordsRestore();
    if (((SeVoiceScreenView *) gSeVoiceState)->endFlag == 2) {
        memset(commandWords, 0, 20);
        commandWords[4] = 128;
        scratchpadSendCmd23Mode0(commandWords);
    }
    return (s8) ((SeVoiceScreenView *) gSeVoiceState)->resetFlag;
}

u8 D_003E71C0[16] = { 0x04, 0x02, 0x01, 0x00 };

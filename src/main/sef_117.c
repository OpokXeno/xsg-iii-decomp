#include "common.h"
#include "shared.h"

void sefSetFogOverride(Vector4 *fogParams, Vector4 *fogColor);

#include "quadword.h"
#include "vu0_macro.h"

extern AlignedVector4 sefOverrideVectorA;
extern AlignedVector4 sefOverrideVectorB;

typedef int s32;

extern s32 sefEffectRecordCount;
extern s32 sefLineDataCount;
void *memset();
extern s32 sefPtAllocCount;
extern s32 sefKeyframeStepBlock[];
extern int sefDrawEnabled;
extern s32 sefRequestKeyBlock[];
extern s32 sefEffectEntryTable[];
extern s32 sefGroupTimers[];
void eBattleResetState(void);
void svInitImageMapper(void);
void sdvResetSpecialWork(void);
void sdvInitAlters(void);
void sevInitPtAllocator(void);
void func_00200180(void);
void sefClearLineDataRecords(void);
void sefClearLineRecordBlock(void);
void func_001F3900(void);
void sefClearEffectTallies(void);
void clearRecordTables(void);
void svSceneFlagsClear(s32 flags);
void sefEffectFlagSet(s32 flag);
void sefNoiseInit(void);
void func_001E7268(void);
void sefClearEntryTable(void);
void func_001E9F48(s32 value);
void sefResetEffectSystem(void);

/*
 * Sets the default keyframe step (1000 thousandths), the record counts (40
 * effect records, 170 line records, 1100 pt allocations), then clears and
 * resets the effect tables and subsystems in order.
 */
void sefResetEffectSystem(void) {
    s32 defaultKeyframeStep = 1000;
    s32 effectRecordTotal = 0x28;
    s32 lineDataTotal = 0xAA;
    s32 ptAllocTotal = 0x44C;
    s32 *keyframeStepBlock = sefKeyframeStepBlock;

    /* kept: the empty do-while decides the emitted order of the step store. */
    do {
    } while (0);
    keyframeStepBlock[0] = defaultKeyframeStep;
    sefEffectRecordCount = effectRecordTotal;
    sefLineDataCount = lineDataTotal;
    sefPtAllocCount = ptAllocTotal;
    eBattleResetState();
    svInitImageMapper();
    sdvResetSpecialWork();
    sdvInitAlters();
    sevInitPtAllocator();
    func_00200180();
    sefClearLineDataRecords();
    sefClearLineRecordBlock();
    func_001F3900();
    memset(sefRequestKeyBlock, 0, 0x2E0);
    memset(sefEffectEntryTable, 0, 0x20720);
    memset(sefGroupTimers, 0, 0x100);
    sefClearEffectTallies();
    clearRecordTables();
    svSceneFlagsClear(1);
    sefEffectFlagSet(0);
    sefNoiseInit();
    func_001E7268();
    sefClearEntryTable();
    func_001E9F48(0);
    sefDrawEnabled = 1;
}

typedef long long s64;

/* A word set when either override vector was supplied; sefGetFogOverrideBlock hands out its address. */
extern s32 sefOverrideFlag[4];
extern s64 gSbStudioBlock[];
extern s32 sefEffectRecords;
extern s32 sefLineDataRecords;
extern s32 sefPtAllocatorState;

/*
 * Carves the effect pools out of recordPoolMemory and clears them: the first
 * 0xA7960 bytes go to the pt allocator, then the line data (0x5C4E0 bytes) and
 * the effect records (0x3480 bytes). Returns the total size, 0x107340.
 */
s32 sefInitRecordPools(s32 recordPoolMemory)
{
    s64 *studioBlock;
    s32 sectionBase;
    s64 studioFlags;

    sefResetEffectSystem();
    studioBlock = gSbStudioBlock;
    studioFlags = (studioBlock[2] & (~0xC)) | 8;
    studioBlock[2] = studioFlags;
    sefEffectRecordCount = 0x28;
    sefLineDataCount = 0xAA;
    sefPtAllocCount = 0x44C;
    sefPtAllocatorState = recordPoolMemory;
    memset(recordPoolMemory, 0, 0xA7960);
    sectionBase = recordPoolMemory + 0xA7960;
    sefLineDataRecords = sectionBase;
    memset(sectionBase, 0, 0x5C4E0);
    sectionBase = recordPoolMemory + 0x103E40;
    sefEffectRecords = sectionBase;
    memset(sectionBase, 0, 0x3480);
    return 0x107340;
}

extern s32 D_00551050;
extern s32 D_00551058;
extern char D_003935C0[];
extern char D_003635C0[];
void sefResetEffectWorld(void);
void eBattleWinInit(s32 value);
void svSceneFlagsSet(s32 flags);
s32 xglDebFileBufferSize(char *path);
void func_00209C88(s32 value, s32 *buffer);

/*
 * Scene setup by sceneMode: 0 clears scene flag 1 and takes the file at
 * D_003935C0, 1 sets it and takes the file at D_003635C0, 2 only sets it.
 * A file, when chosen, is passed to func_00209C88 with its size.
 */
void sefSetSceneMode(s32 sceneMode) {
    s32 fileLoadParams[4];
    char *filePath;
    s64 studioFlags;
    s64 sceneModeBits;

    func_001E9F48(0);
    D_00551050 = 0;
    filePath = 0;
    D_00551058 = 0;
    sefResetEffectWorld();
    sefClearEntryTable();
    sefSetFogOverride(0, 0);
    studioFlags = gSbStudioBlock[2];
    studioFlags &= ~0x40;
    sceneModeBits = (sceneMode & 3) << 2;
    studioFlags &= ~0xC;
    studioFlags |= sceneModeBits;
    gSbStudioBlock[6] = 0;
    studioFlags |= 2;
    gSbStudioBlock[2] = studioFlags;
    eBattleWinInit(0);
    if (sceneMode == 0) {
        svSceneFlagsClear(1);
        filePath = D_003935C0;
    } else if (sceneMode == 1) {
        svSceneFlagsSet(1);
        filePath = D_003635C0;
    } else if (sceneMode == 2) {
        svSceneFlagsSet(1);
    }
    if (filePath != 0) {
        fileLoadParams[0] = (s32) filePath;
        fileLoadParams[1] = xglDebFileBufferSize(filePath);
        fileLoadParams[2] = 0;
        fileLoadParams[3] = 0;
        func_00209C88(0, fileLoadParams);
    }
}

/*
 * Copies the two override vectors; a null source resets that vector to the
 * origin (0, 0, 0, 1). The flag records whether any override was given.
 */
void sefSetFogOverride(Vector4 *fogParams, Vector4 *fogColor)
{
    if (fogParams != 0) {
        QuadwordCopyT0(&sefOverrideVectorA, fogParams);
    } else {
        Vu0SetOrigin(&sefOverrideVectorA);
    }
    if (fogColor != 0) {
        QuadwordCopyT0(&sefOverrideVectorB, fogColor);
    } else {
        Vu0SetOrigin(&sefOverrideVectorB);
    }
    sefOverrideFlag[0] = fogParams != 0 || fogColor != 0;
}

void sefResetEffectWorld(void);
void srsSetEffectFlag(s32 lookup, s32 value);
void eBattleWinReset(void);

/* Resets the effect world and the battle-win state, then sets effect flag 0. */
void sefResetBattleWinState(void) {
    sefResetEffectWorld();
    eBattleWinReset();
    srsSetEffectFlag(0, 1);
}

/* The camera matrices of the effect system: current and previous, and a valid word. */
typedef struct SefCameraBlock {
    Matrix4 currentMatrix;
    Matrix4 previousMatrix;
    unsigned char unmodeled_80[0x14];
    s32 valid;
} SefCameraBlock;

extern SefCameraBlock sefCameraMatrixBlock;
extern const Matrix4 MMathUnitMatrix;
void sefReleaseOwnerEffects(s32 value);
void srsSetEffectIndexFlag(s32 index, s32 value);
void sdvDestroyAlters(void);
void tiWindResetSlots(void);
void slotReleaseAllPools(void);

/* Resets the effect world (the slotReleaseAllPools step included) and the camera matrices to the unit matrix, with valid cleared. */
void sefResetEffectWorld(void) {
    sefReleaseOwnerEffects(-1);
    slotReleaseAllPools();
    srsSetEffectIndexFlag(0, 1);
    memset(sefRequestKeyBlock, 0, 0x2E0);
    memset(sefEffectEntryTable, 0, 0x20720);
    memset(sefGroupTimers, 0, 0x100);
    sefClearEffectTallies();
    sevInitPtAllocator();
    func_00200180();
    sefClearLineDataRecords();
    sefClearLineRecordBlock();
    sdvResetSpecialWork();
    clearRecordTables();
    sdvDestroyAlters();
    tiWindResetSlots();
    Vu0CopyUnitMatrix(sefCameraMatrixBlock.previousMatrix, MMathUnitMatrix);
    Vu0CopyUnitMatrix(sefCameraMatrixBlock.currentMatrix, MMathUnitMatrix);
    sefCameraMatrixBlock.valid = 0;
}

/* The same reset as sefResetEffectWorld without its slotReleaseAllPools step; the scene-reset path calls it. */
void sefResetSceneEffects(void) {
    sefReleaseOwnerEffects(-1);
    srsSetEffectIndexFlag(0, 1);
    memset(sefRequestKeyBlock, 0, 0x2E0);
    memset(sefEffectEntryTable, 0, 0x20720);
    memset(sefGroupTimers, 0, 0x100);
    sefClearEffectTallies();
    sevInitPtAllocator();
    func_00200180();
    sefClearLineDataRecords();
    sefClearLineRecordBlock();
    sdvResetSpecialWork();
    clearRecordTables();
    sdvDestroyAlters();
    tiWindResetSlots();
    Vu0CopyUnitMatrix(sefCameraMatrixBlock.previousMatrix, MMathUnitMatrix);
    Vu0CopyUnitMatrix(sefCameraMatrixBlock.currentMatrix, MMathUnitMatrix);
    sefCameraMatrixBlock.valid = 0;
}

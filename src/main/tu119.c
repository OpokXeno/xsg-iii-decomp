#include "common.h"
#include "shared.h"
#include "gs.h"
#include "vu0_macro.h"
#include "main/xgl_2.h"

typedef int s32;
typedef long long s64;

typedef struct LinkedObject {
    s32 delegated;          /* 0x00: nonzero routes value writes to the child object */
    s32 linkBits;              /* 0x04: bit 0x400000 is set or cleared by objectSetLinkedFlag */
    u8 unk_08[2];
    s16 linkedValue;        /* 0x0A */
    u8 unk_0C[0x20];
    f32 childFloat;     /* 0x2C: written on the child when delegated */
    u8 unk_30[0x1C];
    f32 ownFloat;         /* 0x4C */
    u8 unk_50[0xC];
    s16 childKey;           /* 0x5C: key for func_001F9A68 */
    u8 unk_5E[2];
    s32 pendingFlags;       /* 0x60 */
    s32 pendingValue;       /* 0x64 */
    u8 unk_68[0xD6];
    s16 linkIndex;          /* 0x13E */
} LinkedObject;

void objectSetLinkedValue(LinkedObject *linkedObject, s32 value);
void objectSetLinkedFlag(LinkedObject *linkedObject, s32 value);

void objectApplyLinkedFlags(LinkedObject *linkedObject, s32 flagMask, s32 linkedValue) {
    if (linkedObject != 0) {
        if (flagMask & 0x400000) {
            objectSetLinkedFlag(linkedObject, linkedValue == 0);
        }
        if (flagMask & 0x200000) {
            objectSetLinkedValue(linkedObject, linkedValue);
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EA128);

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EA2B0);

s32 func_001EA128(LinkedObject *linkedObject, s32 flags, s32 value);

void objectApplyLinkedValue(LinkedObject *linkedObject, s32 linkedValue) {
    func_001EA128(linkedObject, 0x200000, linkedValue);
}

s32 func_001EA2B0(LinkedObject *linkedObject, s32 index, s32 flags, s32 value);

void objectApplyLinkedValueIndexed(LinkedObject *linkedObject, s32 slotIndex, s32 linkedValue) {
    func_001EA2B0(linkedObject, slotIndex, 0x200000, linkedValue);
}

void objectApplyLinkedFlag(LinkedObject *linkedObject, s32 linkEnabled) {
    func_001EA128(linkedObject, 0x400000, linkEnabled);
}

void objectApplyLinkedFlagIndexed(LinkedObject *linkedObject, s32 slotIndex, s32 linkEnabled) {
    func_001EA2B0(linkedObject, slotIndex, 0x400000, linkEnabled);
}

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EA448);

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EA518);

typedef struct VuQuadword {
    s32 lanes[4];
} VuQuadword;

void *vuLaneMaskQuadsWrite(void *quadStorage)
{
  VuQuadword *quads;
  void *laneMaskQuad;
  quads = quadStorage;
  quads[0].lanes[0] = 0;
  quads[0].lanes[1] = 1;
  quads[0].lanes[2] = 1;
  quads[0].lanes[3] = 0;
  __asm__ __volatile__("sqc2 vf0, 0(%0)" : : "r"(&quads[1]) : "memory");
  laneMaskQuad = &quads[2];
  __asm__ __volatile__("vmaxw.xyzw vf1, vf0, vf0w" : : : "memory");
  __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(laneMaskQuad) : "memory");
  __asm__ __volatile__("sqc2 vf0, 0(%0)" : : "r"(&quads[3]) : "memory");
  return laneMaskQuad;
}

s32 func_001F6698();

void sceneTickSubsystem(void) {
    func_001F6698();
}

s32 sefResetEffectWorld();
void srsSetEffectFlag(s32, s32);

void sceneInitSubsystems(void) {
    sefResetEffectWorld();
    srsSetEffectFlag(0, 1);
}

extern unsigned char gEffectBuffer[];
void clearEffectTable(void);
void *memset(void *destination, s32 value, unsigned int size);

void sceneClearBufferAndInit(void) {
    memset(gEffectBuffer, 0, 0xF00);
    clearEffectTable();
}

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EA6B0);

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EA798);

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EA920);

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EA960);

void objectSetPendingFlags(LinkedObject *linkedObject, s32 newPendingFlags, s32 newPendingValue) {
    if ((linkedObject != 0) && (linkedObject->delegated == 0)) {
        linkedObject->pendingFlags = newPendingFlags;
        linkedObject->pendingValue = newPendingValue;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EA9C8);

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EAA08);

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EAA70);

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EAA98);

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EAB00);

s32 func_001F7668(s16);
LinkedObject *func_001F9A68(s16);

void linkedObjectSetFloat(LinkedObject *linkedObject, f32 linkedFloat) {
    LinkedObject *childObject;

    if (linkedObject != 0) {
        if (linkedObject->delegated == 0) {
            linkedObject->ownFloat = linkedFloat;
            func_001F7668(linkedObject->linkIndex);
            return;
        }
        childObject = func_001F9A68(linkedObject->childKey);
        if (childObject != 0) {
            childObject->childFloat = linkedFloat;
        }
    }
}

s32 func_001F7740(s16 linkIndex, s32 value);

void objectSetLinkedValue(LinkedObject *linkedObject, s32 linkedValue)
{
  s32 newPendingFlags;
  if (linkedObject != 0)
  {
    if (linkedObject->delegated == 0)
    {
      linkedObject->linkedValue = linkedValue;
      func_001F7740(linkedObject->linkIndex, linkedValue);
    }
    else
    {
      newPendingFlags = (linkedObject->pendingFlags & (~0x100)) | ((linkedValue & 1) << 8);
      linkedObject->pendingFlags = newPendingFlags;
    }
  }
}

s32 func_001F77D8(s16 linkIndex, s32 value);

void objectSetLinkedFlag(LinkedObject *linkedObject, s32 linkEnabled)
{
  s32 newPendingFlags;
  s32 flagsWithLinked;
  s32 flagsWithoutLinked;
  if (linkedObject != 0)
  {
    if (linkedObject->delegated == 0)
    {
      if (linkEnabled != 0)
      {
        flagsWithLinked = linkedObject->linkBits | 0x400000;
        linkedObject->linkBits = flagsWithLinked;
      }
      else
      {
        flagsWithoutLinked = linkedObject->linkBits & 0xFFBFFFFF;
        linkedObject->linkBits = flagsWithoutLinked;
      }
      func_001F77D8(linkedObject->linkIndex, linkEnabled);
    }
    else
    {
      newPendingFlags = (linkedObject->pendingFlags & (~0x200)) | ((linkEnabled & 1) << 9);
      linkedObject->pendingFlags = newPendingFlags;
    }
  }
}

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EACA8);

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EADA0);

s32 sefReleaseOwnerEffects(s32);
s32 func_002002C8();

void checkResultReport(s32 errorCode) {
    if (errorCode < 0) {
        sefReleaseOwnerEffects(0x1006402A);
        return;
    }
    func_002002C8();
}

extern unsigned int gSceneLoadState;

s32 stateIsReady(void) {
    return gSceneLoadState == 3;
}

extern u8 *D_004C83F0[];
void slotReleaseFixedSlots();
void sceneShatterTransitionUpdate(void);

void sceneCallbackReset(void) {
    gSceneLoadState = 0;
    slotReleaseFixedSlots();
    D_004C83F0[0] = sceneShatterTransitionUpdate;
}

/* The screen-break transition. sceneCallbackReset installs it in the render state's frame hook; gSceneLoadState steps
 * it: 0 builds the shards, 1 shows the still screen for two frames, 2 lets the shards fly until none is left, 3 ends it
 * and clears the hook. The screen is cut into SHATTER_ROWS x SHATTER_COLUMNS hexagons laid on the camera's near plane;
 * each is drawn as a textured triangle fan from a half-size copy of the screen. */
#define SHATTER_ROWS 29
#define SHATTER_COLUMNS 14
#define SHATTER_PI 3.1415927f

/* A vector of the effect. The projected corners are read back through words: MMathRotTransPers leaves GS fixed-point
 * x, y and z in them, and the fourth word of a shard's center corner holds its culled flag. */
typedef union ShatterVector {
    Vector4 vector;
    float values[4];
    int words[4];
} ShatterVector;

/* One flying hexagon of the screen-break effect. */
typedef struct ShatterShard {
    ShatterVector position; /* w: the alpha, 128 when built, falling by 10 per frame once the shard has flown off */
    ShatterVector velocity; /* w: the frames left before the shard starts to move, then counts down to -2 */
    ShatterVector spin;     /* added to angle every frame */
    ShatterVector angle;    /* the rotation about x, y and z */
} ShatterShard;

/* The frame-buffer pages and the frame hook of the render state (D_004C8390) this effect reads. */
typedef struct ShatterRenderState {
    unsigned char unmodeled_00[8];
    u16 depthPage;      /* +0x08: ZBUF base */
    unsigned char unmodeled_0a[4];
    u16 outlinePage;    /* +0x0E: the page the outlines and the glow are drawn to */
    u16 copyPage;       /* +0x10: the half-size copy of the screen the shards are textured from */
    u16 glowSourcePage; /* +0x12 */
    u16 glowPage;       /* +0x14 */
    u16 screenPage;     /* +0x16: the shown screen */
    unsigned char unmodeled_18[0x48];
    void (*frameHook)(void); /* +0x60 */
} ShatterRenderState;

extern ShatterRenderState D_004C8390;
extern ShatterShard gShatterShards[SHATTER_ROWS][SHATTER_COLUMNS];
/* The shards still visible in the last frame of state 2. */
extern int gShatterActiveShardCount;
extern const Matrix4 MMathUnitMatrix;

void *memset(void *destination, int value, unsigned int size);
void xglPacketBufferInit(s32 spr_address);
void xglPacketAppendEntry(s32 word0, s32 word1, s32 word2, s32 word3);
void xglPacketAppendQuad(s64 reg, s64 value);
void xglPacketFinishTag(void);
void xglPacketSendDirect(s32 address, s32 qwordCount);
int func_0030DD10(void);
u64 func_003098D0(f32 value);
float xglSin(float angle);
float MMathCalcLength(const Vector4 *vector);
float MMathCalcLengthXY(const Vector4 *vector);
Matrix4 *MMathRotateMatrixX(Matrix4 *outMatrix, Matrix4 *matrix, float angle);
Matrix4 *MMathRotateMatrixY(Matrix4 *outMatrix, Matrix4 *matrix, float angle);
Matrix4 *MMathRotateMatrixZ(Matrix4 *outMatrix, Matrix4 *matrix, float angle);
Matrix4 *MMathMulMatrix(Matrix4 *outMatrix, Matrix4 *rhs, Matrix4 *lhs);
Vector4 *MMathApplyMatrix(Vector4 *outVector, Matrix4 *matrix, const Vector4 *vector);
void MMathRotTransPers(Vector4 *outVector, const struct XglCamera *camera, const Matrix4 *matrix,
                       const Vector4 *point);
struct XglCamera *xglStudioGetActiveCamera(void);
void drawRectOnPageColour(s32 page, s32 x, s32 y, s32 z, s32 width, s32 height);
void drawAreaSetExtended(s32 word_00, s32 word_3C, s32 word_44, s32 word_04, s32 word_40, s32 word_48, s32 frameBase,
                         s32 drawMode, s32 origin_x, s32 origin_y, s32 width, s32 height, s32 clip_x, s32 clip_y,
                         s32 draw_width, s32 draw_height);
void drawAreaSetWindow(s32 frameBase, s32 drawMode, s32 origin_x, s32 origin_y, s32 width, s32 height, s32 clip_x,
                       s32 clip_y, s32 draw_width, s32 draw_height);
void drawEnvSendRecordPacket(void);
void func_001D55E8(void);

/* The GS vertex of one projected hexagon corner. */
#define SHATTER_XYZ(point) GS_SET_XYZ((point).words[0], (point).words[1], (point).words[2])

static inline void sefAddVector(Vector4 *destination, const Vector4 *first, const Vector4 *second)
{
    __asm__ __volatile__(
        "lqc2 vf1, 0(%5)\n"
        "lqc2 vf2, 0(%6)\n"
        "vadd.xyz vf1, vf1, vf2\n"
        "sqc2 vf1, 0(%4)\n"
        : "=m" (destination->x), "=m" (destination->y), "=m" (destination->z), "=m" (destination->w)
        : "r" (destination), "r" (first), "r" (second)
        : "memory");
}

static inline void sefSubVector(Vector4 *destination, const Vector4 *minuend, const Vector4 *subtrahend)
{
    __asm__ __volatile__(
        "lqc2 vf1, 0(%5)\n"
        "lqc2 vf2, 0(%6)\n"
        "vsub.xyz vf1, vf1, vf2\n"
        "sqc2 vf1, 0(%4)\n"
        : "=m" (destination->x), "=m" (destination->y), "=m" (destination->z), "=m" (destination->w)
        : "r" (destination), "r" (minuend), "r" (subtrahend)
        : "memory");
}

static inline void sefScaleVector(Vector4 *destination, const Vector4 *vector, float scale)
{
    __asm__ __volatile__(
        "lqc2 vf1, 0(%5)\n"
        "mfc1 $8, %6\n"
        "qmtc2.ni $8, vf2\n"
        "vmulx.xyz vf1, vf1, vf2x\n"
        "sqc2 vf1, 0(%4)\n"
        : "=m" (destination->x), "=m" (destination->y), "=m" (destination->z), "=m" (destination->w)
        : "r" (destination), "r" (vector), "f" (scale)
        : "$8", "memory");
}

static inline void sefSetOrigin(Vector4 *destination)
{
    __asm__ __volatile__(
        "sqc2 vf0, 0(%4)\n"
        : "=m" (destination->x), "=m" (destination->y), "=m" (destination->z), "=m" (destination->w)
        : "r" (destination)
        : "memory");
}

static inline void sefAccumulateVector(Vector4 *destination, const Vector4 *addend)
{
    __asm__ __volatile__(
        "lqc2 vf1, 0(%4)\n"
        "lqc2 vf2, 0(%5)\n"
        "vadd.xyz vf1, vf1, vf2\n"
        "sqc2 vf1, 0(%4)\n"
        : "=m" (destination->x), "=m" (destination->y), "=m" (destination->z), "=m" (destination->w)
        : "r" (destination), "r" (addend)
        : "memory");
}


/* Screen shatter transition: case 0 seeds a random velocity and spin per hexagonal shard (SHATTER_ROWS x
 * SHATTER_COLUMNS grid), cases 1/2 draw the captured screen page as shards (1: still, 2: flying and fading)
 * and blur-glow the result; case 3 clears the frame hook. */
void sceneShatterTransitionUpdate(void)
{
    Vector4 nearOrigin;
    Vector4 nearRight;
    Vector4 nearTop;
    Vector4 nearCorner;
    Vector4 nearWidth;
    Vector4 nearHeight;
    Matrix viewMatrix;
    Vector4 grid[SHATTER_ROWS][SHATTER_COLUMNS];
    Matrix4 rotation;
    Vector4 direction;
    Matrix4 turn;
    Vector4 scaled;
    ShatterVector screen[SHATTER_COLUMNS][7];
    int row;
    int column;
    Matrix4 *cameraView;
    struct XglCamera *camera;
    float gridScaleX;
    float gridScaleY;
    float gridX;
    float gridY;
    float glowStep;
    int gridLeft;

    switch (gSceneLoadState) {
    case 0:
        gShatterActiveShardCount = 0;
        xglPacketBufferInit(0x70000000);
        xglPacketAppendEntry(0x8000, 0x10000000, GIF_AD, 0);
        xglPacketAppendQuad(GS_FBA_1, 0);
        xglPacketAppendQuad(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
        xglPacketAppendQuad(GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.depthPage, 0x31, 1));
        xglPacketAppendQuad(GS_CLAMP_1, 0);
        xglPacketAppendQuad(GS_TEX1_1, GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0));
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 2, 0x80));
        xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0, 0x3F800000));
        xglPacketAppendQuad(GS_TEX0_1, GS_SET_TEX0(D_004C8390.screenPage << 5, 8, 0, 9, 9, 0, 0, 0, 0, 0, 0, 0));
        xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.copyPage, 4, 0, 0xFF000000));
        xglPacketAppendQuad(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
        xglPacketAppendQuad(GS_UV, 0);
        xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x6FF8, 0x71F7, 0));
        xglPacketAppendQuad(GS_UV, GS_SET_UV(0x2000, 0x1C00));
        xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x7FF8, 0x81F7, 0));
        xglPacketFinishTag();
        xglPacketSendDirect(0, 0);
        for (row = 0; row < SHATTER_ROWS; row++) {
            for (column = 0; column < SHATTER_COLUMNS; column++) {
                memset(&direction, 0, sizeof(direction));
                direction.z = -10.0f;
                sefSetOrigin(&gShatterShards[row][column].position.vector);
                sefSetOrigin(&gShatterShards[row][column].velocity.vector);
                sefSetOrigin(&gShatterShards[row][column].angle.vector);
                Vu0CopyUnitMatrix(&turn, &MMathUnitMatrix);
                MMathRotateMatrixY(&turn, &turn, func_0030DD10() % 360 / 180.0f * SHATTER_PI);
                MMathApplyMatrix(&gShatterShards[row][column].velocity.vector, &turn, &direction);
                gShatterShards[row][column].spin.values[0] = (func_0030DD10() % 100 - 50) / 10.0f / 180.0f * SHATTER_PI;
                gShatterShards[row][column].spin.values[1] = (func_0030DD10() % 100 - 50) / 10.0f / 180.0f * SHATTER_PI;
                gShatterShards[row][column].spin.values[2] = (func_0030DD10() % 100 - 50) / 10.0f / 180.0f * SHATTER_PI;
                sefSetOrigin(&direction);
                gridLeft = row * 18;
                if (column & 1) {
                    direction.x = (gridLeft + 9) / 512.0f - 0.5f;
                } else {
                    direction.x = gridLeft / 512.0f - 0.5f;
                }
                direction.y = -((column * 36) / 448.0f - 0.5f);
                sefScaleVector(&direction, &direction, 10.0f);
                sefAccumulateVector(&gShatterShards[row][column].velocity.vector, &direction);
                sefScaleVector(&direction, &direction, 2.0f);
                gShatterShards[row][column].velocity.values[3] = (int) MMathCalcLengthXY(&direction) + 1;
                gShatterShards[row][column].position.values[3] = 128.0f;
            }
        }
        gSceneLoadState = 1;
    case 1:
        if (++gShatterActiveShardCount < 2) {
            break;
        }
        gSceneLoadState = 2;
        gShatterActiveShardCount = -1;
    case 2:
        if (gShatterActiveShardCount != 0) {
            gShatterActiveShardCount = 0;
            break;
        }
        gSceneLoadState = 3;
    case 3:
        D_004C8390.frameHook = 0;
        return;
    }

    viewMatrix = xglStudioGetActiveCamera()->travel.transform;
    viewMatrix.rows[3][0] = viewMatrix.rows[3][1] = viewMatrix.rows[3][2] = 0.0f;
    viewMatrix.rows[3][3] = 1.0f;
    nearOrigin = xglStudioGetActiveCamera()->volume[0].vertex[2].vector;
    nearRight = xglStudioGetActiveCamera()->volume[0].vertex[3].vector;
    nearTop = xglStudioGetActiveCamera()->volume[0].vertex[0].vector;
    nearCorner = xglStudioGetActiveCamera()->volume[0].vertex[1].vector;
    sefSubVector(&nearWidth, &nearRight, &nearOrigin);
    sefSubVector(&nearHeight, &nearTop, &nearOrigin);
    gridScaleX = MMathCalcLength(&nearWidth) / 512.0f * 1.01f;
    gridScaleY = MMathCalcLength(&nearHeight) / 448.0f * 1.01f;
    for (row = 0; row < SHATTER_ROWS; row++) {
        for (column = 0; column < SHATTER_COLUMNS; column++) {
            if (column & 1) {
                gridX = (row * 18 + 9) / 512.0f;
            } else {
                gridX = (row * 18) / 512.0f;
            }
            gridY = (column * 36) / 448.0f;
            sefScaleVector(&scaled, &nearWidth, gridX);
            sefAddVector(&grid[row][column], &nearOrigin, &scaled);
            sefScaleVector(&scaled, &nearHeight, gridY);
            sefAddVector(&grid[row][column], &grid[row][column], &scaled);
        }
    }

    xglPacketBufferInit(0x70000000);
    xglPacketAppendEntry(0x8000, 0x10000000, GIF_AD, 0);
    xglPacketAppendQuad(GS_TEXFLUSH, 0);
    xglPacketAppendQuad(GS_FBA_1, 0);
    xglPacketAppendQuad(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
    xglPacketAppendQuad(GS_CLAMP_1, 0);
    xglPacketAppendQuad(GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.depthPage, 0x31, 1));
    xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(0, 0, 0, 0x80, 0x3F800000));
    xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 2, 0x80));
    xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.outlinePage, 8, 0, 0xFF000000));
    xglPacketAppendQuad(GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 1, 0, 0, 0, 0));
    xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x6FF8, 0x71F7, 0));
    xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x8FF8, 0x8DF7, 0));
    xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 1, 0, 1, 0x80));
    xglPacketAppendQuad(GS_TEX0_1, GS_SET_TEX0(D_004C8390.copyPage << 5, 4, 0, 8, 8, 0, 0, 0, 0, 0, 0, 0));
    xglPacketAppendQuad(GS_TEX1_1, GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0));
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);

    for (row = 0; row < SHATTER_ROWS; row++) {
        xglPacketBufferInit(0x70000000);
        xglPacketAppendEntry(0x8000, 0x10000000, GIF_AD, 0);
        xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.screenPage, 8, 0, 0xFF000000));
        for (column = 0; column < SHATTER_COLUMNS; column++) {
            float halfWidth = gridScaleX * 18.0f * 0.5f;
            float halfHeight = gridScaleY * 48.0f * 0.5f;
            float quarterHeight = gridScaleY * 24.0f * 0.5f;
            ShatterVector hexagon[7] = {
                {{0.0f, 0.0f, 0.0f, 1.0f}},
                {{0.0f, halfHeight, 0.0f, 1.0f}},
                {{halfWidth, quarterHeight, 0.0f, 1.0f}},
                {{halfWidth, -quarterHeight, 0.0f, 1.0f}},
                {{0.0f, -halfHeight, 0.0f, 1.0f}},
                {{-halfWidth, -quarterHeight, 0.0f, 1.0f}},
                {{-halfWidth, quarterHeight, 0.0f, 1.0f}},
            };
            Matrix4 worldMatrix;
            Vector4 driftOffset;

            cameraView = &xglStudioGetActiveCamera()->volume[0].viewMatrix;
            switch (gSceneLoadState) {
            case 1:
                xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, 0x3F800000));
                xglPacketAppendQuad(GS_PRIM, GS_SET_PRIM(6, 1, 1, 0, 1, 0, 1, 0, 0));
                row = SHATTER_ROWS;
                column = SHATTER_COLUMNS;
                xglPacketAppendQuad(GS_UV, 0);
                xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x6FF8, 0x71F7, 0));
                xglPacketAppendQuad(GS_UV, GS_SET_UV(0x1000, 0x1000));
                xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x8FF8, 0x8DF7, 0));
                xglPacketAppendQuad(GS_PRIM, GS_SET_PRIM(2, 1, 1, 0, 1, 0, 1, 0, 0));
                xglPacketAppendQuad(GS_UV, GS_SET_UV(0x10, 0x10));
                xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x6FF8, 0x71F7, 0));
                xglPacketAppendQuad(GS_UV, GS_SET_UV(0xFF0, 0x10));
                xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x8FF8, 0x71F7, 0));
                xglPacketAppendQuad(GS_UV, GS_SET_UV(0xFF0, 0xFF0));
                xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x8FF8, 0x8DF7, 0));
                xglPacketAppendQuad(GS_UV, GS_SET_UV(0x10, 0xFF0));
                xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x6FF8, 0x8DF7, 0));
                xglPacketAppendQuad(GS_UV, GS_SET_UV(0x10, 0x10));
                xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x6FF8, 0x71F7, 0));
                break;
            case 2:
                if (gShatterShards[row][column].velocity.values[3] > 0.0f) {
                    gShatterShards[row][column].velocity.values[3] -= 1.0f;
                    if (gShatterShards[row][column].velocity.values[3] <= 0.0f) {
                        gShatterShards[row][column].velocity.values[3] = 0.0f;
                    }
                    gShatterActiveShardCount++;
                } else if (gShatterShards[row][column].velocity.values[3] <= 0.0f
                           && gShatterShards[row][column].velocity.values[3] >= -2.0f) {
                    gShatterShards[row][column].velocity.values[3] -= 1.0f;
                } else if (gShatterShards[row][column].position.values[3] > 0.0f) {
                    gShatterShards[row][column].position.values[3] -= 10.0f;
                    gShatterActiveShardCount++;
                } else {
                    gShatterShards[row][column].position.values[3] = 0.0f;
                    continue;
                }
                Vu0CopyUnitMatrix(&rotation, &MMathUnitMatrix);
                if (gSceneLoadState == 2 && gShatterShards[row][column].velocity.values[3] <= -2.0f) {
                    sefAddVector(&gShatterShards[row][column].angle.vector, &gShatterShards[row][column].angle.vector,
                                       &gShatterShards[row][column].spin.vector);
                    MMathRotateMatrixZ(&rotation, &rotation, gShatterShards[row][column].angle.values[2]);
                    MMathRotateMatrixX(&rotation, &rotation, gShatterShards[row][column].angle.values[0]);
                    MMathRotateMatrixY(&rotation, &rotation, gShatterShards[row][column].angle.values[1]);
                }
                MMathMulMatrix(&worldMatrix, &viewMatrix.rows, &rotation);
                MMathApplyMatrix(&hexagon[0].vector, &worldMatrix, &hexagon[0].vector);
                MMathApplyMatrix(&hexagon[1].vector, &worldMatrix, &hexagon[1].vector);
                MMathApplyMatrix(&hexagon[2].vector, &worldMatrix, &hexagon[2].vector);
                MMathApplyMatrix(&hexagon[3].vector, &worldMatrix, &hexagon[3].vector);
                MMathApplyMatrix(&hexagon[4].vector, &worldMatrix, &hexagon[4].vector);
                MMathApplyMatrix(&hexagon[5].vector, &worldMatrix, &hexagon[5].vector);
                MMathApplyMatrix(&hexagon[6].vector, &worldMatrix, &hexagon[6].vector);
                if (gSceneLoadState == 2 && gShatterShards[row][column].velocity.values[3] <= -2.0f) {
                    sefAddVector(&gShatterShards[row][column].position.vector, &gShatterShards[row][column].position.vector,
                                       &gShatterShards[row][column].velocity.vector);
                    sefScaleVector(&driftOffset, &gShatterShards[row][column].position.vector, gridScaleX);
                    MMathApplyMatrix(&driftOffset, &viewMatrix.rows, &driftOffset);
                    sefAccumulateVector(&hexagon[0].vector, &driftOffset);
                    sefAccumulateVector(&hexagon[1].vector, &driftOffset);
                    sefAccumulateVector(&hexagon[2].vector, &driftOffset);
                    sefAccumulateVector(&hexagon[3].vector, &driftOffset);
                    sefAccumulateVector(&hexagon[4].vector, &driftOffset);
                    sefAccumulateVector(&hexagon[5].vector, &driftOffset);
                    sefAccumulateVector(&hexagon[6].vector, &driftOffset);
                }
                sefAccumulateVector(&hexagon[0].vector, &grid[row][column]);
                sefAccumulateVector(&hexagon[1].vector, &grid[row][column]);
                sefAccumulateVector(&hexagon[2].vector, &grid[row][column]);
                sefAccumulateVector(&hexagon[3].vector, &grid[row][column]);
                sefAccumulateVector(&hexagon[4].vector, &grid[row][column]);
                sefAccumulateVector(&hexagon[5].vector, &grid[row][column]);
                sefAccumulateVector(&hexagon[6].vector, &grid[row][column]);
                {
                    int visible;
                    int corner;

                    camera = xglStudioGetActiveCamera();
                    visible = 0;
                    MMathRotTransPers(&screen[column][0].vector, camera, cameraView, &hexagon[0].vector);
                    MMathRotTransPers(&screen[column][1].vector, camera, cameraView, &hexagon[1].vector);
                    MMathRotTransPers(&screen[column][2].vector, camera, cameraView, &hexagon[2].vector);
                    MMathRotTransPers(&screen[column][3].vector, camera, cameraView, &hexagon[3].vector);
                    MMathRotTransPers(&screen[column][4].vector, camera, cameraView, &hexagon[4].vector);
                    MMathRotTransPers(&screen[column][5].vector, camera, cameraView, &hexagon[5].vector);
                    MMathRotTransPers(&screen[column][6].vector, camera, cameraView, &hexagon[6].vector);
                    /* kept: the loop (always marks the shard visible) decides the emitted order */
                    for (corner = 0; corner < 7; corner++) {
                        visible |= 1;
                    }
                    if (visible) {
                        screen[column][0].words[3] = 0;
                    } else {
                        screen[column][0].words[3] = 1;
                        continue;
                    }
                    {
                        int textureX;
                        int textureY;
                        u64 centerU;
                        u64 rightU;
                        u64 leftU;
                        u64 firstUv;
                        u64 upperV;
                        u64 lowerV;

                        textureX = (column & 1) ? row * 18 + 9 : row * 18;
                        textureX += 512;
                        textureY = column * 36 + 448;
                        xglPacketAppendQuad(GS_RGBAQ,
                                            GS_SET_RGBAQ(0x80, 0x80, 0x80,
                                                         (int) ((1.0f - xglSin((gShatterShards[row][column].position.values[3] / 128.0f * 90.0f
                                                                                + 90.0f) / 180.0f * SHATTER_PI))
                                                                * 128.0f),
                                                         0x3F800000));
                        xglPacketAppendQuad(GS_PRIM, GS_SET_PRIM(5, 1, 1, 0, 1, 0, 1, 0, 0));
                        centerU = func_003098D0((float) (textureX << 4) * 0.5f + 4.0f);
                        xglPacketAppendQuad(GS_UV, GS_SET_UV(centerU, func_003098D0((float) (textureY << 4)
                                                                                   * (256.0f / 448.0f) + 4.0f)));
                        xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][0]));
                        firstUv = GS_SET_UV(centerU, func_003098D0((float) ((textureY - 24) << 4) * (256.0f / 448.0f) + 4.0f));
                        xglPacketAppendQuad(GS_UV, firstUv);
                        xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][1]));
                        rightU = func_003098D0((float) ((textureX + 9) << 4) * 0.5f + 4.0f);
                        upperV = func_003098D0((float) ((textureY - 12) << 4) * (256.0f / 448.0f) + 4.0f);
                        xglPacketAppendQuad(GS_UV, GS_SET_UV(rightU, upperV));
                        xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][2]));
                        lowerV = func_003098D0((float) ((textureY + 12) << 4) * (256.0f / 448.0f) + 4.0f);
                        xglPacketAppendQuad(GS_UV, GS_SET_UV(rightU, lowerV));
                        xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][3]));
                        xglPacketAppendQuad(GS_UV, GS_SET_UV(centerU, func_003098D0((float) ((textureY + 24) << 4)
                                                                                   * (256.0f / 448.0f) + 4.0f)));
                        xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][4]));
                        leftU = func_003098D0((float) ((textureX - 9) << 4) * 0.5f + 4.0f);
                        xglPacketAppendQuad(GS_UV, GS_SET_UV(leftU, lowerV));
                        xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][5]));
                        xglPacketAppendQuad(GS_UV, GS_SET_UV(leftU, upperV));
                        xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][6]));
                        xglPacketAppendQuad(GS_UV, firstUv);
                        xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][1]));
                    }
                }
                break;
            }
        }
        if (gSceneLoadState == 2) {
            for (column = 0; column < SHATTER_COLUMNS; column++) {
                int alpha = (int) gShatterShards[row][column].position.values[3];
                int delay = (int) gShatterShards[row][column].velocity.values[3];
                if (screen[column][0].words[3] == 0 && delay <= 0 && alpha > 0) {
                    float brightness = alpha / 128.0f;
                    int red = (int) (brightness * 32.0f);
                    int green = (int) (brightness * 127.0f) + 0x80;
                    int blue = (int) (brightness * -95.0f) + 0xFF;
                    xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(red, green, blue, alpha, 0x3F800000));
                    xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.outlinePage, 8, 0, 0xFF000000));
                    xglPacketAppendQuad(GS_PRIM, GS_SET_PRIM(2, 1, 0, 0, 1, 0, 0, 0, 0));
                    xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][1]));
                    xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][2]));
                    xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][3]));
                    xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][4]));
                    xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][5]));
                    xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][6]));
                    xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][1]));
                }
            }
        }
        xglPacketFinishTag();
        xglPacketSendDirect(0, 0);
    }

    if (gSceneLoadState != 2) {
        return;
    }
    drawRectOnPageColour(D_004C8390.glowSourcePage, 0, 0, 0, 0x100, 0xE0);
    drawAreaSetExtended(D_004C8390.outlinePage, 8, 0, D_004C8390.glowPage, 4, 0, 0x80, 3, 0, 0, 0x200, 0x1C0, 0, 0,
                        0x100, 0xE0);
    drawEnvSendRecordPacket();
    glowStep = 4.0f;
    for (row = 0; row < 5; row++) {
        int alpha = glowStep * (5 - row);
        drawAreaSetExtended(D_004C8390.glowPage, 4, 0, D_004C8390.glowSourcePage, 4, 0, alpha, 1, 1, 1, 0x100, 0xE0,
                            row + 1, row + 1, row + 0x100, row + 0xE0);
        drawEnvSendRecordPacket();
        drawAreaSetExtended(D_004C8390.glowPage, 4, 0, D_004C8390.glowSourcePage, 4, 0, alpha, 1, 0, 1, 0xFF, 0xE0,
                            -row, row + 1, 0xFF - row, row + 0xE0);
        drawEnvSendRecordPacket();
        drawAreaSetExtended(D_004C8390.glowPage, 4, 0, D_004C8390.glowSourcePage, 4, 0, alpha, 1, 1, 0, 0x100, 0xDF,
                            row + 1, -row, row + 0x100, 0xDF - row);
        drawEnvSendRecordPacket();
        drawAreaSetExtended(D_004C8390.glowPage, 4, 0, D_004C8390.glowSourcePage, 4, 0, alpha, 1, 0, 0, 0xFF, 0xDF,
                            -row, -row, 0xFF - row, 0xDF - row);
        drawEnvSendRecordPacket();
    }
    drawAreaSetExtended(D_004C8390.glowSourcePage, 4, 0, D_004C8390.screenPage, 8, 0, 0x80, 1, 1, 1, 0xFF, 0xDF, 1, 1,
                        0x1FF, 0x1BF);
    drawEnvSendRecordPacket();
    drawAreaSetExtended(D_004C8390.glowSourcePage, 4, 0, D_004C8390.screenPage, 8, 0, 0x80, 1, 0, 0, 0xFF, 0xDF, -1, -1,
                        0x1FF, 0x1BF);
    drawEnvSendRecordPacket();
    drawAreaSetWindow(0x80, 1, 0, 0, 0x200, 0x1C0, 0, 0, 0x200, 0x1C0);
    func_001D55E8();
}

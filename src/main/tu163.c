#include "common.h"
#include "gs.h"
#include "main/xgl_2.h"
#include "main/xgl_render.h"

typedef signed char s8;

typedef int s32;

typedef struct GameLoopRuntime {
    s32 loopFlags;
} GameLoopRuntime;

extern unsigned char gGameLoopRuntimeStorage[];

#define gGameLoopRuntime (*(GameLoopRuntime *) gGameLoopRuntimeStorage)

typedef struct DecodeLane {
    s32 unk_00;
    f32 targetAngle;
    s32 unk_08;
    s32 unk_0C;
    f32 unk_10;
} DecodeLane;

typedef struct DecodeBlock {
    u8 unmodeled_000[0x30];
    s16 slotStart0;
    u8 slotFlag0;
    u8 unmodeled_033[0x70 - 0x33];
    s16 slotStart1;
    u8 slotFlag1;
    u8 unmodeled_073[0xB0 - 0x73];
    s16 slotStart2;
    u8 slotFlag2;
    u8 unmodeled_0b3[0xF0 - 0xB3];
    s16 slotStart3;
    u8 slotFlag3;
    u8 unmodeled_0f3[0x100 - 0xF3];
    DecodeLane lanes[3];
    s32 unk_13C;
    s8 text[512];
} DecodeBlock;

s32 xglSRand(void);
f32 xglFRand(void);
void xcLoadPayloadSections(void);
void xglCameraSetWindow(XglCamera *camera, s32 x, s32 y, s32 width, s32 height);

/* Resets a decode block: the camera window, random NUL-terminated text (letters, spaces and up to 16 newlines),
   four slot starts with cleared flags, and the lane seeds copied down to lanes 1 and 0. */
void shardFieldInit(DecodeBlock *block) {
    XglCamera *camera;
    s8 *textCursor;
    s32 charIndex;
    s32 newlineCount;
    s32 letterRoll;
    s32 slot0Seed;
    s32 slot1Seed;
    s32 slot2Seed;
    s32 slot3Seed;
    f32 signedRandom;
    s32 newlineRoll = 31;
    s32 newlineChar = 10;

    gGameLoopRuntime.loopFlags |= 0x400;
    xcLoadPayloadSections();
    camera = xglStudioGetCamera2(0);
    camera->cameraState = 4;
    camera->travel.position.z = 30.0f;
    camera->travel.fov = 0.69813174f;
    camera->travel.position.x = 0.0f;
    camera->travel.position.y = 0.0f;
    camera->travel.rotation.z = 0.0f;
    camera->travel.rotation.y = 0.0f;
    camera->travel.rotation.x = 0.0f;
    xglCameraSetWindow(camera, 0, 0, 512, 448);
    gEnvFlags.color.red = 0;
    gEnvFlags.color.green = 0;
    gEnvFlags.color.blue = 0;

    textCursor = block->text;
    charIndex = 0;
    newlineCount = 0;
    /* kept: the goto shares the letter branch with the newline test; a plain if/else chain changes the emitted order. */
    do {
        letterRoll = xglSRand() & 0x1f;
        if (letterRoll >= 26) {
            if (letterRoll >= 31) {
                if (letterRoll == newlineRoll) {
                    *textCursor = newlineChar;
                    newlineCount++;
                } else {
                    goto letter;
                }
            } else {
                *textCursor = ' ';
            }
        } else {
        letter:
            *textCursor = letterRoll + 'a';
        }
        charIndex++;
        textCursor++;
    } while (newlineCount < 16 && charIndex < 511);
    *textCursor = 0;

    block->slotFlag0 = 0;
    slot0Seed = xglSRand();
    block->slotFlag1 = 0;
    block->slotStart0 = (slot0Seed & 0x3f) + 1;
    slot1Seed = xglSRand();
    block->slotFlag2 = 0;
    block->slotStart1 = (slot1Seed & 0x3f) + 80;
    slot2Seed = xglSRand();
    block->slotFlag3 = 0;
    block->slotStart2 = (slot2Seed & 0x3f) + 160;
    slot3Seed = xglSRand();
    block->unk_13C = 0;
    block->lanes[2].unk_08 = 0;
    block->lanes[2].unk_00 = 0;
    block->slotStart3 = (slot3Seed & 0x3f) + 240;
    signedRandom = xglFRand() - 1.0f;
    block->lanes[2].unk_0C = 0;
    block->lanes[2].unk_10 = 30.0f;
    block->lanes[2].targetAngle = (signedRandom * 3.14159265f * 0.7f) - 1.0995574f;
    block->lanes[1] = block->lanes[2];
    block->lanes[0] = block->lanes[1];
}

s32 xcClearPayloadSections();

void xcBlockFlagsReset(void) {
    s32 remainingFlags;

    remainingFlags = gGameLoopRuntime.loopFlags & ~0x400;
    gGameLoopRuntime.loopFlags = remainingFlags;
    xcClearPayloadSections();
}

extern f32 gQuadDepthCounter;

void xcLoadCounterReset(void) {
    gQuadDepthCounter = 0.0f;
}

s32 blockStatusBitsGet(void *block);
s32 _CurRotTransPersClipScreen(ScreenVector4 *screenOut, const Vector4 *point);
s32 xglFontScaleColorAlpha(u32 color);
s32 nmlPacketGsInit(void);
void xglDirectQueueSubmitRef(void);
extern s32 xglDirectQueueCount;

/* The direct queue buffer pointer as a one-member record: its load does not conflict with the packet stores. */
typedef struct DirectQueueBuffer {
    u64 *queueBase;
} DirectQueueBuffer;

extern DirectQueueBuffer xglDirectQueueBuffer;
void _CurMatrixSet(Matrix4 matrix);
void _CurSetViewScaleTrans(Vector4 *scale, Vector4 *offset);
void xcLoadCounterReset(void);
extern u16 D_004C8390[];

/* Appends one quad to the direct queue: clips four corners, then writes the PRIM tag and RGBAQ, ST and XYZ2
   words of each corner and advances the queue count. */
void xglDirectQueueAppendTexQuad(Vector4 *corners, u32 colorA, u32 colorB, f32 scale) {
    union ScreenVertex {
        ScreenVector4 point;
        s32 words[4];
        f32 floats[4];
    } screen[4];
    union {
        s32 words[2];
        f32 floats[2];
    } stValue;
    u32 remainingBits;
    f32 remainingFloat;
    f32 stScale;
    u64 rgbaColorA;
    u64 rgbaColorB;
    u64 primTag;
    u64 *queueBuffer;
    f32 depthCentre;
    f32 depthNear;
    u64 *packetStart;
    u64 *packet;
    s32 queueCount;
    ScreenVector4 *firstPoint;

    firstPoint = &screen[0].point;
    remainingBits = 512 - (blockStatusBitsGet(corners) & 0x1ff);
    remainingFloat = remainingBits;
    depthCentre = remainingFloat * 0.00048828125f + gQuadDepthCounter;
    gQuadDepthCounter += scale;
    if (_CurRotTransPersClipScreen(firstPoint, &corners[0]) != 0) {
        return;
    }
    if (_CurRotTransPersClipScreen(&screen[1].point, &corners[1]) != 0) {
        return;
    }
    if (_CurRotTransPersClipScreen(&screen[2].point, &corners[2]) != 0) {
        return;
    }
    if (_CurRotTransPersClipScreen(&screen[3].point, &corners[3]) != 0) {
        return;
    }
    colorA = xglFontScaleColorAlpha(colorA);
    colorB = xglFontScaleColorAlpha(colorB);
    nmlPacketGsInit();
    rgbaColorB = colorB;
    rgbaColorA = colorA;
    queueCount = xglDirectQueueCount;
    queueBuffer = xglDirectQueueBuffer.queueBase;
    packet = queueBuffer + queueCount * 2;
    primTag = 93;
    packetStart = packet;
    GS_PUT(packet, GS_PRIM, primTag);
    GS_PUT(packet, GS_RGBAQ, rgbaColorA | ((u64) screen[0].words[3] << 32));
    stScale = 0.4f;
    stValue.floats[0] = 0.0f;
    stValue.floats[1] = depthCentre * screen[0].floats[3];
    GS_PUT(packet, GS_ST, (u64) stValue.words[0] | ((u64) stValue.words[1] << 32));
    GS_PUT(packet, GS_XYZ2, GS_SET_XYZ(screen[0].point.x, screen[0].point.y, screen[0].point.z));
    GS_PUT(packet, GS_RGBAQ, rgbaColorA | ((u64) screen[1].words[3] << 32));
    stValue.floats[0] = screen[1].floats[3] * stScale;
    stValue.floats[1] = depthCentre * screen[1].floats[3];
    GS_PUT(packet, GS_ST, (u64) stValue.words[0] | ((u64) stValue.words[1] << 32));
    GS_PUT(packet, GS_XYZ2, GS_SET_XYZ(screen[1].point.x, screen[1].point.y, screen[1].point.z));
    GS_PUT(packet, GS_RGBAQ, rgbaColorB | ((u64) screen[2].words[3] << 32));
    depthNear = depthCentre + scale;
    stValue.floats[0] = screen[2].floats[3] * stScale;
    stValue.floats[1] = depthNear * screen[2].floats[3];
    GS_PUT(packet, GS_ST, (u64) stValue.words[0] | ((u64) stValue.words[1] << 32));
    GS_PUT(packet, GS_XYZ2, GS_SET_XYZ(screen[2].point.x, screen[2].point.y, screen[2].point.z));
    GS_PUT(packet, GS_RGBAQ, rgbaColorB | ((u64) screen[3].words[3] << 32));
    stValue.floats[0] = 0.0f;
    stValue.floats[1] = depthNear * screen[3].floats[3];
    GS_PUT(packet, GS_ST, (u64) stValue.words[0] | ((u64) stValue.words[1] << 32));
    GS_PUT(packet, GS_XYZ2, GS_SET_XYZ(screen[3].point.x, screen[3].point.y, screen[3].point.z));
    xglDirectQueueCount = queueCount + (((u32) packet - (u32) packetStart) >> 4);
    xglDirectQueueSubmitRef();
}

/* Subdivides the patch into four corner points, draws the two halves through xglDirectQueueAppendTexQuad with a GS state
   header, and advances the direct queue. */
void shardFieldDrawPatch(Vector4 *patchCorners, s32 colorIndex, f32 scale) {
    XglCamera *camera;
    Vector4 point[4];
    Vector4 midpointBC;
    Vector4 subPatch[4];
    f32 outerScale;
    u32 colorWithAlpha;
    u64 *packet;
    s32 queueCount;
    u64 *packetStart;
    u32 alphaByte;

    alphaByte = colorIndex & 0xFF;
    camera = xglStudioGetCamera2(0);
    if (camera == 0) {
        return;
    }
    point[0].x = (patchCorners[0].x + (patchCorners[0].x + patchCorners[3].x) * 0.5f) * 0.5f;
    point[0].y = (patchCorners[0].y + (patchCorners[0].y + patchCorners[3].y) * 0.5f) * 0.5f;
    point[0].z = (patchCorners[0].z + (patchCorners[0].z + patchCorners[3].z) * 0.5f) * 0.5f;
    midpointBC.x = (patchCorners[1].x + patchCorners[2].x) * 0.5f;
    midpointBC.y = (patchCorners[1].y + patchCorners[2].y) * 0.5f;
    midpointBC.z = (patchCorners[1].z + patchCorners[2].z) * 0.5f;
    point[3].x = (patchCorners[3].x + (patchCorners[0].x + patchCorners[3].x) * 0.5f) * 0.5f;
    point[3].y = (patchCorners[3].y + (patchCorners[0].y + patchCorners[3].y) * 0.5f) * 0.5f;
    point[3].z = (patchCorners[3].z + (patchCorners[0].z + patchCorners[3].z) * 0.5f) * 0.5f;
    point[1].x = (patchCorners[1].x + midpointBC.x) * 0.5f;
    point[1].y = (patchCorners[1].y + midpointBC.y) * 0.5f;
    point[1].z = (patchCorners[1].z + midpointBC.z) * 0.5f;
    point[2].x = (patchCorners[2].x + midpointBC.x) * 0.5f;
    point[2].y = (patchCorners[2].y + midpointBC.y) * 0.5f;
    point[2].z = (patchCorners[2].z + midpointBC.z) * 0.5f;
    _CurMatrixSet(camera->volume[0].viewMatrix);
    _CurSetViewScaleTrans(&camera->screen.screenScale, &camera->screen.screenOffset);
    nmlPacketGsInit();
    queueCount = xglDirectQueueCount;
    packet = xglDirectQueueBuffer.queueBase + queueCount * 2;
    packetStart = packet;
    GS_PUT(packet, GS_TEST_1, 0x30000);
    GS_PUT(packet, GS_ZBUF_1, GS_SET_ZBUF(D_004C8390[4], 1, 1));
    GS_PUT(packet, GS_CLAMP_1, GS_SET_CLAMP(3, 3, 0x1FF, 0, 0x7F, 0));
    GS_PUT(packet, GS_TEX0_1, GS_SET_TEX0(D_004C8390[7] << 5, 8, 2, 9, 9, 1, 0, 0, 0, 0, 0, 0));
    GS_PUT(packet, GS_TEX1_1, 0x60);
    GS_PUT(packet, GS_ALPHA_1, 0x48);
    xglDirectQueueCount = queueCount + (((u32) packet - (u32) packetStart) >> 4);
    xglDirectQueueSubmitRef();
    xcLoadCounterReset();
    outerScale = scale * 0.025f;
    subPatch[0] = patchCorners[0];
    subPatch[1] = patchCorners[1];
    subPatch[2] = point[1];
    subPatch[3] = point[0];
    colorWithAlpha = (alphaByte << 24) | 0x562E2D;
    xglDirectQueueAppendTexQuad(subPatch, 0x562E2D, colorWithAlpha, outerScale);
    xglDirectQueueAppendTexQuad(point, colorWithAlpha, colorWithAlpha, scale * 0.05f);
    subPatch[0] = point[3];
    subPatch[1] = point[2];
    subPatch[2] = patchCorners[2];
    subPatch[3] = patchCorners[3];
    xglDirectQueueAppendTexQuad(subPatch, colorWithAlpha, 0x562E2D, outerScale);
}

/* The spring angles of a ShardField at 0x100 (springPos[0] and springPos[1]). */
typedef struct ShardFieldTilt {
    unsigned char unmodeled_000[0x100];
    f32 springRotX;
    f32 springRotZ;
} ShardFieldTilt;

void xglMatrixStackUnit(void);
void xglMatrixStackRotX(float angle);
void xglMatrixStackRotY(float angle);
void xglMatrixStackRotZ(float angle);
void xglMatrixStackTrans(const Vector4 *vector);
void xglMatrixStackSave(Matrix4 matrix);
void xglMatrixStackLoad(Matrix4 matrix);

/* Draws one shard: builds the tilted base transform from the field's spring angles, takes the four corners
   from the offset pivots (sign flips per corner), and hands them to shardFieldDrawPatch. */
void shardFieldDrawShard(Vector4 *position, Vector4 *rotation, Vector4 *shardOffset, u8 colorIndex, ShardFieldTilt *tilt) {
    Vector4 corners[4];
    Vector4 cornerOffset;
    Matrix4 cornerMatrix;
    Matrix4 baseMatrix;

    xglMatrixStackUnit();
    xglMatrixStackRotZ(tilt->springRotZ);
    xglMatrixStackRotX(tilt->springRotX);
    xglMatrixStackTrans(position);
    xglMatrixStackRotZ(rotation->z);
    xglMatrixStackRotY(rotation->y);
    xglMatrixStackRotX(rotation->x);
    xglMatrixStackSave(baseMatrix);

    cornerOffset = *shardOffset;
    cornerOffset.x = -cornerOffset.x;
    cornerOffset.y = -cornerOffset.y;
    xglMatrixStackTrans(&cornerOffset);
    xglMatrixStackSave(cornerMatrix);
    corners[0].x = cornerMatrix[3][0];
    corners[0].y = cornerMatrix[3][1];
    corners[0].z = cornerMatrix[3][2];
    corners[0].w = 1.0f;
    xglMatrixStackLoad(baseMatrix);

    cornerOffset = *shardOffset;
    cornerOffset.y = -cornerOffset.y;
    xglMatrixStackTrans(&cornerOffset);
    xglMatrixStackSave(cornerMatrix);
    corners[1].x = cornerMatrix[3][0];
    corners[1].y = cornerMatrix[3][1];
    corners[1].z = cornerMatrix[3][2];
    corners[1].w = 1.0f;
    xglMatrixStackLoad(baseMatrix);

    cornerOffset = *shardOffset;
    xglMatrixStackTrans(&cornerOffset);
    xglMatrixStackSave(cornerMatrix);
    corners[2].x = cornerMatrix[3][0];
    corners[2].y = cornerMatrix[3][1];
    corners[2].z = cornerMatrix[3][2];
    corners[2].w = 1.0f;
    xglMatrixStackLoad(baseMatrix);

    cornerOffset = *shardOffset;
    cornerOffset.x = -cornerOffset.x;
    xglMatrixStackTrans(&cornerOffset);
    xglMatrixStackSave(cornerMatrix);
    {
        f32 offsetY = shardOffset->y;
        corners[3].x = cornerMatrix[3][0];
        corners[3].y = cornerMatrix[3][1];
        corners[3].z = cornerMatrix[3][2];
        corners[3].w = 1.0f;
        shardFieldDrawPatch(corners, colorIndex, offsetY);
    }
}

void springFloatStep(f32 *position, f32 *previousPosition, f32 *targetPosition, f32 stiffness) {
    f32 currentPosition;
    f32 nextPosition;

    currentPosition = *position;
    nextPosition = currentPosition + ((currentPosition - *previousPosition) + ((*targetPosition - currentPosition) * stiffness));
    *position = nextPosition;
    *previousPosition = nextPosition;
}

typedef struct FieldShard {
    Vector4 position;
    Vector4 offset;
    Vector4 rotation;
    s16 timer;
    u8 state;
    unsigned char unmodeled_33[0x0D];
} FieldShard;

typedef struct ShardField {
    FieldShard shards[4];
    f32 springPos[5];
    f32 springPrev[5];
    f32 springTarget[5];
    f32 scroll;
    unsigned char label[0x10];
} ShardField;

extern const Vector4 gShardSeedX;
extern const Vector4 gShardSeedY;
extern const unsigned char gShardFieldTitleText[];
extern s32 gXcFirstSection;
extern s32 gXcSecondSection;
extern u16 movieBackdropBasePage[];
void *memset(void *dest, int value, unsigned int count);
s32 xglFontGetAlphaScale(void);
void xglFontSetAlphaScale(s32 scale);
void xglFontPrintTitle(void);
void xglFontSetColor(s32 red, s32 green, s32 blue);
void xglFontPrintIndexed(s32 x, s32 y, const void *text);
void nmlModelSetDrawMatrix(Matrix4 matrix);
void nmlModelSetStencil(s32 enable);
void nmlModelSetTransparency(f32 alpha);
void nmlModelSetTexture(s32 texture);
void pxySubmitModel(s32 model);
void xglFontPrintExtendedColorByte(s32 a, s32 b, s32 c, s32 d);
void fontQueueFrameBuffer(u16 page, s32 mode);
void fontQueueSpriteReset(void);
void fontQueueSpriteBlack(void);
void fontQueueMovieFrameBuffer(void);
void func_00185A20(void);
void func_001AA580(void);

/* One frame of the shard field: steps the four springs, scrolls, draws the backdrop models and the label,
   then runs the shard state machine (0 waits, 1 and 3 move a shard, 2 holds it until two shards hold). */
void shardFieldUpdate(ShardField *shardField) {
    Matrix4 drawMatrix;
    Vector4 drawOffset;
    f32 shardSeedX[4];
    f32 shardSeedY[4];
    f32 modelTransparency;
    s32 springRoll4;
    s32 springRoll3;
    s32 fontAlphaScale;
    s32 shardIndex;
    s32 heldShardCount;
    s32 scanIndex;
    FieldShard *shard;

    memset(&drawOffset, 0, 16);
    drawOffset.w = 1.0f;
    *(Vector4 *) shardSeedX = gShardSeedX;
    *(Vector4 *) shardSeedY = gShardSeedY;

    if (!(xglSRand() & 0xFF)) {
        shardField->springTarget[0] = ((xglFRand() - 1.0f) * 3.14159265f * 0.5f) - 0.7853982f;
    }
    if (!(xglSRand() & 0x1FF)) {
        shardField->springTarget[1] = ((xglFRand() - 1.0f) * 3.14159265f * 0.7f) - 1.0995574f;
    }
    springRoll3 = xglSRand();
    if (!(springRoll3 & 0x7F)) {
        shardField->springTarget[3] = ((xglFRand() - 1.0f) * 8.0f) - 4.0f;
    }
    springRoll4 = xglSRand();
    /* kept: the empty then-branch decides the emitted branch sense. */
    if (springRoll4 & 0x7F) {
    } else {
        shardField->springTarget[4] = 30.0f + ((xglFRand() - 1.0f) * 15.0f);
    }

    springFloatStep(&shardField->springPos[2], &shardField->springPrev[2], &shardField->springTarget[2], 0.01f);
    springFloatStep(&shardField->springPos[0], &shardField->springPrev[0], &shardField->springTarget[0], 0.0002f);
    springFloatStep(&shardField->springPos[1], &shardField->springPrev[1], &shardField->springTarget[1], 0.0001f);
    springFloatStep(&shardField->springPos[3], &shardField->springPrev[3], &shardField->springTarget[3], 0.001f);
    springFloatStep(&shardField->springPos[4], &shardField->springPrev[4], &shardField->springTarget[4], 0.0005f);

    shardField->scroll -= 0.004f;
    while (shardField->scroll < -16.0f) {
        shardField->scroll += 16.0f;
    }
    drawOffset.x = shardField->springPos[3];
    drawOffset.y = shardField->springPos[2] + shardField->scroll;
    while (drawOffset.y < -16.0f) {
        drawOffset.y += 16.0f;
    }
    while (drawOffset.y > 16.0f) {
        drawOffset.y -= 16.0f;
    }

    fontAlphaScale = xglFontGetAlphaScale();
    modelTransparency = fontAlphaScale * 0.0078125f;

    if (gXcFirstSection != 0 && gXcSecondSection != 0) {
        xglMatrixStackUnit();
        xglMatrixStackRotZ(shardField->springPos[1]);
        xglMatrixStackRotX(shardField->springPos[0]);
        xglMatrixStackTrans(&drawOffset);
        xglMatrixStackSave(drawMatrix);
        nmlModelSetDrawMatrix(drawMatrix);
        nmlModelSetStencil(1);
        nmlModelSetTransparency(modelTransparency);
        nmlModelSetTexture(gXcSecondSection);
        pxySubmitModel(gXcFirstSection);
        xglMatrixStackLoad(drawMatrix);

        drawOffset.x = 0.0f;
        drawOffset.y = 16.0f;
        xglMatrixStackTrans(&drawOffset);
        xglMatrixStackSave(drawMatrix);
        nmlModelSetDrawMatrix(drawMatrix);
        nmlModelSetStencil(1);
        nmlModelSetTransparency(modelTransparency);
        nmlModelSetTexture(gXcSecondSection);
        pxySubmitModel(gXcFirstSection);
        xglMatrixStackLoad(drawMatrix);

        xglMatrixStackTrans(&drawOffset);
        xglMatrixStackSave(drawMatrix);
        nmlModelSetDrawMatrix(drawMatrix);
        nmlModelSetStencil(1);
        nmlModelSetTransparency(modelTransparency);
        nmlModelSetTexture(gXcSecondSection);
        pxySubmitModel(gXcFirstSection);
        xglMatrixStackLoad(drawMatrix);

        drawOffset.y = -48.0f;
        xglMatrixStackTrans(&drawOffset);
        xglMatrixStackSave(drawMatrix);
        nmlModelSetDrawMatrix(drawMatrix);
        nmlModelSetStencil(1);
        nmlModelSetTransparency(modelTransparency);
        nmlModelSetTexture(gXcSecondSection);
        pxySubmitModel(gXcFirstSection);
        func_001AA580();
    }

    fontQueueFrameBuffer(movieBackdropBasePage[0], 2);
    fontQueueSpriteReset();
    xglFontSetAlphaScale(0x80);
    xglFontPrintTitle();
    xglFontPrintExtendedColorByte(0x40, 0x40, 0x40, 2);
    xglFontSetColor(0x80, 0x80, 0x80);
    xglFontPrintIndexed(0, 0, gShardFieldTitleText);
    xglFontPrintIndexed(0, 0, shardField->label);
    func_00185A20();
    fontQueueMovieFrameBuffer();
    xglFontSetAlphaScale(fontAlphaScale);

    shardIndex = 0;
    do {
        shard = shardField->shards + shardIndex;
        if (shard->timer > 0) {
            shard->timer -= 1;
        }
        switch (shard->state) {
        case 0:
            if (shard->timer == 0) {
                shard->state = 1;
                shard->timer = 31;
                shard->position.x = ((xglFRand() - 1.0f) * 8.0f - 4.0f) + shardSeedX[shardIndex] * 14.0f;
                shard->position.y = ((xglFRand() - 1.0f) * 5.0f + 2.0f) + shardSeedY[shardIndex] * 8.0f;
                shard->position.z = (xglFRand() - 1.0f) * 16.0f - 14.0f;
                shard->position.w = 1.0f;
                shard->offset.x = 5.0f;
                shard->offset.y = (xglFRand() - 1.0f) * 20.0f - 18.0f;
                shard->offset.z = 0.0f;
                shard->offset.w = 1.0f;
                shard->rotation.x = 0.0f;
                shard->rotation.y = -shard->position.x / 12.0f;
                shard->rotation.z = 0.0f;
                shard->rotation.w = 1.0f;
                if (shard->position.x < 0.0f) {
                    shard->rotation.y = shard->rotation.y + 3.14159265f;
                }
            }
            break;
        case 1:
            shardFieldDrawShard(&shard->position, &shard->rotation, &shard->offset,
                          (0x80 - (u8) shard->timer * 4), (ShardFieldTilt *) shardField);
            if (shard->timer == 0) {
                shard->state = 2;
                shard->timer = (xglSRand() & 0x1FF) + 64;
            }
            break;
        case 2:
            shardFieldDrawShard(&shardField->shards[shardIndex].position, &shardField->shards[shardIndex].rotation, &shardField->shards[shardIndex].offset,
                          0x80U, (ShardFieldTilt *) shardField);
            if (shard->timer == 0) {
                heldShardCount = 0;
                for (scanIndex = 0; scanIndex < 4; scanIndex++) {
                    if (shardField->shards[scanIndex].state == 2) {
                        heldShardCount++;
                    }
                }
                if (heldShardCount >= 2) {
                    shard->state = 3;
                    shard->timer = 31;
                }
            }
            break;
        case 3:
            shardFieldDrawShard(&shard->position, &shard->rotation, &shard->offset, ((u8) shard->timer * 4) & 0xFC,
                          (ShardFieldTilt *) shardField);
            if (shard->timer == 0) {
                shard->state = 0;
                shard->timer = (xglSRand() & 0x7F) + 128;
            }
            break;
        }
        shardIndex += 1;
    } while (shardIndex < 4);

    fontQueueSpriteBlack();
}

f32 gQuadDepthCounter = 0.0f;

const Vector4 gShardSeedX = { -1.0f, 1.0f, -1.0f, 1.0f };
const Vector4 gShardSeedY = { -1.0f, -1.0f, 1.0f, 1.0f };
const unsigned char gShardFieldTitleText[16] = {
    21, 2, 1, 8, 18, 5, 4, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

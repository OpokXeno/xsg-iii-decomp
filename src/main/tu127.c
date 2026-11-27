#include "common.h"
#include "gs.h"
#include "main/xgl_render.h"
#include "vu0_macro.h"
/* One GS register write of the direct queue: the data word, then the register address. */
#define GS_QUEUE_PUT(q, reg, value) ((q)->registerValue = (value), (q)->registerAddress = (reg), (q)++)

typedef struct XglGsPacket { u64 registerValue; u64 registerAddress; } XglGsPacket;

typedef union RenderStateHalves { u16 h[8]; int w[4]; } RenderStateHalves;

typedef union ShadowCorner { struct { float x,y,z,w; }; int raw[4]; Vector4 v; } ShadowCorner;

typedef union ShadowRecord { struct {
    float (*transform)[4];   /* 0x00: 0 selects the axis-aligned path */
    u8 unmodeled_04[12];
    float centre[3];         /* 0x10 */
    u8 unmodeled_1C[4];
    float halfExtent;        /* 0x20 */
    u8 unmodeled_24[4];
    float scale;             /* 0x28 */
    u8 unmodeled_2C[4];
    float offset[2];         /* 0x30 */
    u8 unmodeled_38[4];
    float unitRow3C;         /* 0x3C */
    float tint[3];           /* 0x40 */
    float fade;              /* 0x4C */
    u8 unmodeled_50[8];
    u64 texture1;            /* 0x58 */
    u64 texture0;            /* 0x60 */
    u8 uvU;                  /* 0x68 */
    u8 uvV;                  /* 0x69 */
    u8 blendMode;            /* 0x6A */
    u8 unmodeled_6B;
    u8 colourB;              /* 0x6C */
    u8 colourG;              /* 0x6D */
    u8 colourR;              /* 0x6E */
}; int raw[32]; } ShadowRecord;

void nmlPacketGsInit(void);
void xglDirectQueueSubmitRef(void);
void xglVectorMulMat(Vector4 *dst, Matrix4 matrix, const Vector4 *src);

#include "main/xgl_direct_queue.h"

extern XglDirectQueueEntry *xglDirectQueueBuffer;

typedef struct QueueCountView { int n; } QueueCountView;

extern int xglDirectQueueCount;
extern XglRenderStateBlock D_004C8390;

static inline u64 gsXYZ(int x, int y, int z)
{
    return (u64)(x * 16 + 0x6FF8) | ((u64)(y * 16 + 0x71F7) << 16) | ((u64)z << 32);
}

/* Queues the two GS packets of one drop shadow: a sprite for an axis-aligned record (transform == 0), else a
   rotated quad around the projected centre. Both paths first write the frame and the Z test. */
void sdvShadowQueueDraw(ShadowRecord *shadow)
{
    ShadowCorner corner[4];
    XglRenderStateBlock *state;
    XglGsPacket *queue;
    XglGsPacket *start;
    u64 rgbaq;
    float centreX;
    float centreY;
    float mirrorX0;
    float mirrorY0;
    float mirrorX1;
    float mirrorY1;
    int originX;
    int originY;
    int maxX;
    int maxY;
    int minX;
    int minY;
    int red;
    int green;
    int blue;

    state = &D_004C8390;
    nmlPacketGsInit();
    queue = (XglGsPacket *)&xglDirectQueueBuffer[((QueueCountView *)&xglDirectQueueCount)->n];
    start = queue;
    GS_QUEUE_PUT(queue, GS_ZBUF_1, (u64)((RenderStateHalves *)state)->h[7] | (0x9880ULL << 17));
    GS_QUEUE_PUT(queue, GS_TEST_1, 0x50000);

    red = (shadow->colourR >> 1) + (int)shadow->tint[0] - 128;
    green = (shadow->colourG >> 1) + (int)shadow->tint[1] - 128;
    blue = (shadow->colourB >> 1) + (int)shadow->tint[2] - 128;
    shadow->colourR = (red >= 256) ? -1 : ((red < 0) ? 0 : red);
    shadow->colourG = (green >= 256) ? -1 : ((green < 0) ? 0 : green);
    shadow->colourB = (blue >= 256) ? -1 : ((blue < 0) ? 0 : blue);

    if (shadow->transform != 0) {
        int depth;

        depth = (int)shadow->centre[2];
        originX = (int)(shadow->centre[0] + shadow->offset[0] * shadow->scale);
        originY = (int)(shadow->centre[1] + shadow->offset[1] * shadow->scale);
        corner[0].x = -shadow->halfExtent;
        corner[0].y = -shadow->halfExtent;
        corner[0].z = 0;
        corner[0].w = 1;
        corner[1].x = shadow->halfExtent;
        corner[1].y = -shadow->halfExtent;
        corner[1].z = 0;
        corner[1].w = 1;
        xglVectorMulMat(&corner[0], shadow->transform, &corner[0]);
        xglVectorMulMat(&corner[1], shadow->transform, &corner[1]);
        mirrorX0 = -corner[0].x;
        mirrorY0 = -corner[0].y;
        mirrorX1 = -corner[1].x;
        mirrorY1 = -corner[1].y;
        corner[0].x = corner[0].x + (float)originX;
        corner[0].y = corner[0].y + (float)originY;
        corner[1].x = corner[1].x + (float)originX;
        corner[1].y = corner[1].y + (float)originY;
        corner[2].x = mirrorX1 + (float)originX;
        corner[2].y = mirrorY1 + (float)originY;
        corner[3].x = mirrorX0 + (float)originX;
        corner[3].y = mirrorY0 + (float)originY;
        if (shadow->texture0 != 0) {
            GS_QUEUE_PUT(queue, GS_FRAME_1, (u64)D_004C8390.display_buffer_base | 0x00FFFFFF00080000ULL);
            GS_QUEUE_PUT(queue, GS_TEX0_1, shadow->texture0);
            GS_QUEUE_PUT(queue, GS_TEX1_1, 0x60);
            GS_QUEUE_PUT(queue, GS_ALPHA_1, (0x80ULL << 32) | 0xA8);
            GS_QUEUE_PUT(queue, GS_PRIM, 0x154);
            GS_QUEUE_PUT(queue, GS_UV, 0);
            GS_QUEUE_PUT(queue, GS_XYZF2, gsXYZ((int)corner[0].x, (int)corner[0].y, depth));
            GS_QUEUE_PUT(queue, GS_UV, (u64)shadow->uvU << 4);
            GS_QUEUE_PUT(queue, GS_XYZF2, gsXYZ((int)corner[1].x, (int)corner[1].y, depth));
            GS_QUEUE_PUT(queue, GS_UV, (u64)shadow->uvV << 20);
            GS_QUEUE_PUT(queue, GS_XYZF2, gsXYZ((int)corner[2].x, (int)corner[2].y, depth));
            GS_QUEUE_PUT(queue, GS_UV, ((u64)shadow->uvU << 4) | ((u64)shadow->uvV << 20));
            GS_QUEUE_PUT(queue, GS_XYZF2, gsXYZ((int)corner[3].x, (int)corner[3].y, depth));
        }
        switch (shadow->blendMode) {
        case 1:
            GS_QUEUE_PUT(queue, GS_ALPHA_1, (0x80ULL << 32) | (shadow->texture0 != 0 ? 0x58 : 0x48));
            break;
        case 2:
            GS_QUEUE_PUT(queue, GS_ALPHA_1, (0x80ULL << 32) | (shadow->texture0 != 0 ? 0x52 : 0x42));
            break;
        default:
            GS_QUEUE_PUT(queue, GS_ALPHA_1, (0x80ULL << 32) | (shadow->texture0 != 0 ? 0x54 : 0x44));
            break;
        }
        GS_QUEUE_PUT(queue, GS_FRAME_1, (u64)D_004C8390.display_buffer_base | 0xFF00000000080000ULL);
        GS_QUEUE_PUT(queue, GS_TEX0_1, shadow->texture1);
        GS_QUEUE_PUT(queue, GS_TEX1_1, 0x60);
        rgbaq = (u64)shadow->colourR | ((u64)shadow->colourG << 8) | ((u64)shadow->colourB << 16) | ((u64)shadow->fade << 24) | (0xFE00ULL << 46);
        GS_QUEUE_PUT(queue, GS_RGBAQ, rgbaq);
        GS_QUEUE_PUT(queue, GS_PRIM, 0x154);
        GS_QUEUE_PUT(queue, GS_UV, 0);
        GS_QUEUE_PUT(queue, GS_XYZF2, gsXYZ((int)corner[0].x, (int)corner[0].y, depth));
        GS_QUEUE_PUT(queue, GS_UV, (u64)shadow->uvU << 4);
        GS_QUEUE_PUT(queue, GS_XYZF2, gsXYZ((int)corner[1].x, (int)corner[1].y, depth));
        GS_QUEUE_PUT(queue, GS_UV, (u64)shadow->uvV << 20);
        GS_QUEUE_PUT(queue, GS_XYZF2, gsXYZ((int)corner[2].x, (int)corner[2].y, depth));
        GS_QUEUE_PUT(queue, GS_UV, ((u64)shadow->uvU << 4) | ((u64)shadow->uvV << 20));
        GS_QUEUE_PUT(queue, GS_XYZF2, gsXYZ((int)corner[3].x, (int)corner[3].y, depth));
    } else {
        int depth;

        depth = (int)shadow->centre[2];
        centreX = shadow->centre[0] + shadow->offset[0] * shadow->scale;
        centreY = shadow->centre[1] + shadow->offset[1] * shadow->scale;
        minX = (int)(centreX - shadow->halfExtent);
        maxX = (int)(centreX + shadow->halfExtent);
        minY = (int)(centreY - shadow->halfExtent);
        maxY = (int)(centreY + shadow->halfExtent);
        if (shadow->texture0 != 0) {
            GS_QUEUE_PUT(queue, GS_FRAME_1, (u64)D_004C8390.display_buffer_base | 0x00FFFFFF00080000ULL);
            GS_QUEUE_PUT(queue, GS_TEX0_1, shadow->texture0);
            GS_QUEUE_PUT(queue, GS_TEX1_1, 0x60);
            GS_QUEUE_PUT(queue, GS_ALPHA_1, (0x80ULL << 32) | 0xA8);
            GS_QUEUE_PUT(queue, GS_PRIM, 0x156);
            GS_QUEUE_PUT(queue, GS_UV, 0);
            GS_QUEUE_PUT(queue, GS_XYZF2, gsXYZ(minX, minY, depth));
            GS_QUEUE_PUT(queue, GS_UV, ((u64)shadow->uvU << 4) | ((u64)shadow->uvV << 20));
            GS_QUEUE_PUT(queue, GS_XYZF2, gsXYZ(maxX, maxY, depth));
        }
        switch (shadow->blendMode) {
        case 1:
            GS_QUEUE_PUT(queue, GS_ALPHA_1, (0x80ULL << 32) | (shadow->texture0 != 0 ? 0x58 : 0x48));
            break;
        case 2:
            GS_QUEUE_PUT(queue, GS_ALPHA_1, (0x80ULL << 32) | (shadow->texture0 != 0 ? 0x52 : 0x42));
            break;
        default:
            GS_QUEUE_PUT(queue, GS_ALPHA_1, (0x80ULL << 32) | (shadow->texture0 != 0 ? 0x54 : 0x44));
            break;
        }
        GS_QUEUE_PUT(queue, GS_FRAME_1, (u64)D_004C8390.display_buffer_base | 0xFF00000000080000ULL);
        GS_QUEUE_PUT(queue, GS_TEX0_1, shadow->texture1);
        GS_QUEUE_PUT(queue, GS_TEX1_1, 0x60);
        rgbaq = (u64)shadow->colourR | ((u64)shadow->colourG << 8) | ((u64)shadow->colourB << 16) | ((u64)shadow->fade << 24) | (0xFE00ULL << 46);
        GS_QUEUE_PUT(queue, GS_RGBAQ, rgbaq);
        GS_QUEUE_PUT(queue, GS_PRIM, 0x156);
        GS_QUEUE_PUT(queue, GS_UV, 0);
        GS_QUEUE_PUT(queue, GS_XYZF2, gsXYZ(minX, minY, depth));
        GS_QUEUE_PUT(queue, GS_UV, ((u64)shadow->uvU << 4) | ((u64)shadow->uvV << 20));
        GS_QUEUE_PUT(queue, GS_XYZF2, gsXYZ(maxX, maxY, depth));
    }

    xglDirectQueueCount += (unsigned int)((u8 *)queue - (u8 *)start) >> 4;
    xglDirectQueueSubmitRef();
}

#define GS_QUEUE_PUT(q, reg, value) ((q)->registerValue = (value), (q)->registerAddress = (reg), (q)++)

typedef struct ShadowDraw {
    float (*transform)[4];   /* 0x00: 0 selects the axis-aligned path */
    u8 unmodeled_04[12];
    float centre[3];         /* 0x10 */
    float unit1C;            /* 0x1C: 1.0 at the start of each draw */
    float halfExtent;        /* 0x20 */
    u8 unmodeled_24[4];
    float scale;             /* 0x28 */
    float unit2C;            /* 0x2C: 1.0 at the start of each draw */
    float offset[2];         /* 0x30 */
    u8 unmodeled_38[4];
    float unitRow3C;         /* 0x3C */
    float tint[3];           /* 0x40 */
    float fade;              /* 0x4C */
    u8 unmodeled_50[8];
    u64 texture1;            /* 0x58 */
    u64 texture0;            /* 0x60 */
    u8 uvU;                  /* 0x68 */
    u8 uvV;                  /* 0x69 */
    u8 blendMode;            /* 0x6A */
    u8 unmodeled_6B;
    u8 colourB;              /* 0x6C */
    u8 colourG;              /* 0x6D */
    u8 colourR;              /* 0x6E */
} ShadowDraw;

typedef union FloatWord {
    float value;
    u32 bits;
} FloatWord;

typedef struct SceneRecord {
    u8 unmodeled_00[0x50];           /* +0x00: source block of MMathApplyMatrix */
    u64 extentA;                     /* +0x50 */
    u64 extentB;                     /* +0x58 */
    float scale;                     /* +0x60 */
    u8 unmodeled_64[4];
    short indexBase;                 /* +0x68: row of the sprite table */
    u8 unmodeled_6A[6];
    signed char itemIndex[16];     /* +0x70: indices into D_0065D3C0 */
    u64 flags;                       /* +0x80 */
    u8 unmodeled_88[8];
} SceneRecord;

typedef struct ItemEntry {
    u64 lo;                          /* +0x00 */
    u64 hi;                          /* +0x08 */
    float weight;                    /* +0x10 */
    signed char kind;                /* +0x14: 1, 2 or 3 selects a draw path */
    u8 tailB;                        /* +0x15 */
    u8 tailC;                        /* +0x16 */
    u8 tailA;                        /* +0x17 */
} ItemEntry;

typedef union ColourWord {
    u32 word;
    u8 bytes[4];
} ColourWord;

typedef struct SpriteStep {
    int a;
    int b;
    ColourWord c;
} SpriteStep;

typedef struct SpriteDef {
    int size0;                       /* +0x00 */
    int size4;                       /* +0x04 */
    ColourWord colour8;              /* +0x08 */
    int size12;                      /* +0x0C */
    int size16;                      /* +0x10 */
    ColourWord colour20;             /* +0x14 */
    SpriteStep steps[16];            /* +0x18 */
} SpriteDef;

typedef struct ImageEntry {
    u8 unmodeled_00[40];
    short loadCount;                 /* +0x28 */
    u8 unmodeled_2A[5084 - 42];
} ImageEntry;

typedef struct SlotBlock {
    u8 unmodeled_00[4];
    int count;                       /* +0x04 */
    u8 unmodeled_08[4];
    signed char slotIds[4];          /* +0x0C */
} SlotBlock;

extern signed char gSdvPacketTable[];
extern long long gSbStudioBlock[];
extern ImageEntry svResourceTypeLists[];
void *xglStudioGetActiveCamera(void);
void MMathApplyMatrix(Vector4 *dst, void *src, void *matrix);
void *sefGetNowScheduler(void);
void _CurMatrixSet(void *matrix);
void _CurSetViewScaleTrans(void *scale, void *trans);
int _CurRotTransPersClipScreen(int *screen, Vector4 *vec);
void svLoadImageList(void *list);
void svRefMapperPacket(void);
void xglMatrixRotZ(Matrix4 destination, Matrix4 source, float angle);
float xglAtan2(float originY, float originX);
void xglMatrixUnit(Matrix4 destination);
void *memset(void *destination, int value, unsigned int size);

const SpriteDef D_0053EA90[5] = {
    {
        120, 0, { 0x00FFFFFF }, 160, 250, { 0x00FFFFFF },
        {
            { 31, -27, { 0x00121217 } },
            { 177, -25, { 0x0007070A } },
            { 7, 36, { 0x001E2C2E } },
            { 26, 51, { 0x000A0E1A } },
            { 20, 67, { 0x00141713 } },
            { 3, 77, { 0x005C5C66 } },
            { 2, 106, { 0x00B76669 } },
            { 16, 119, { 0x000F0C06 } },
            { 37, 132, { 0x00161408 } },
            { 23, 143, { 0x00140807 } },
            { 15, 155, { 0x0006041B } },
            { 60, 182, { 0x00080D05 } },
            { 0, 0, { 0x00000000 } },
            { 0, 0, { 0x00000000 } },
            { 0, 0, { 0x00000000 } },
            { 0, 0, { 0x00000000 } },
        },
    },
    {
        120, 0, { 0x00FFFFFF }, 183, 192, { 0x000F0F1C },
        {
            { 31, -27, { 0x00121217 } },
            { 177, -25, { 0x00161619 } },
            { 20, 23, { 0x000B1117 } },
            { 2, 54, { 0x00876A28 } },
            { 22, 65, { 0x0020282D } },
            { 3, 78, { 0x00666684 } },
            { 40, 117, { 0x00211E1C } },
            { 99, 122, { 0x00222314 } },
            { 9, 125, { 0x00292C29 } },
            { 10, 172, { 0x00352915 } },
            { 2, 175, { 0x00C8DCDC } },
            { 21, 176, { 0x00282F08 } },
            { 3, 204, { 0x003C5AD2 } },
            { 4, 206, { 0x00555D98 } },
            { 3, 208, { 0x005A6EA6 } },
            { 0, 0, { 0x00000000 } },
        },
    },
    {
        120, 0, { 0x00FFFFFF }, 160, 250, { 0x00FFFFFF },
        {
            { 140, -4, { 0x000A0A0A } },
            { 177, -8, { 0x00040406 } },
            { 2, 36, { 0x000A143C } },
            { 3, 52, { 0x000A0A32 } },
            { 8, 67, { 0x00280505 } },
            { 2, 77, { 0x00281E05 } },
            { 2, 140, { 0x00323CB4 } },
            { 20, 177, { 0x00050F14 } },
            { 23, 182, { 0x0005140F } },
            { 0, 0, { 0x00000000 } },
            { 0, 0, { 0x00000000 } },
            { 0, 0, { 0x00000000 } },
            { 0, 0, { 0x00000000 } },
            { 0, 0, { 0x00000000 } },
            { 0, 0, { 0x00000000 } },
            { 0, 0, { 0x00000000 } },
        },
    },
    {
        150, 0, { 0x00FFFFFF }, 160, 250, { 0x00FFFFFF },
        {
            { 7, 36, { 0x001E2C2E } },
            { 26, 51, { 0x000A0E1A } },
            { 20, 67, { 0x00141713 } },
            { 11, 97, { 0x000C142A } },
            { 7, 109, { 0x00143214 } },
            { 25, 118, { 0x000F0C10 } },
            { 37, 129, { 0x000F1D0D } },
            { 20, 126, { 0x00160A09 } },
            { 17, 150, { 0x000F2F2D } },
            { 56, 184, { 0x000F191E } },
            { 0, 0, { 0x00000000 } },
            { 0, 0, { 0x00000000 } },
            { 0, 0, { 0x00000000 } },
            { 0, 0, { 0x00000000 } },
            { 0, 0, { 0x00000000 } },
            { 0, 0, { 0x00000000 } },
        },
    },
    {
        120, 0, { 0x00FFFFFF }, 160, 250, { 0x00FFFFFF },
        {
            { 40, -30, { 0x00444A53 } },
            { 30, 50, { 0x00878960 } },
            { 20, 70, { 0x00AAA63F } },
            { 30, 120, { 0x00031744 } },
            { 60, 140, { 0x001534AF } },
            { 20, 150, { 0x003F4965 } },
            { 10, 170, { 0x00C7C6D6 } },
            { 30, 100, { 0x00837DA1 } },
            { 20, 190, { 0x000B0749 } },
            { 80, 200, { 0x000B0749 } },
            { 0, 0, { 0x00000000 } },
            { 0, 0, { 0x00000000 } },
            { 0, 0, { 0x00000000 } },
            { 0, 0, { 0x00000000 } },
            { 0, 0, { 0x00000000 } },
            { 0, 0, { 0x00000000 } },
        },
    },
};

/* Draws the drop shadows of the packets in a slot block: each claimed packet is placed at its projected centre,
   its items are queued as sprites, and mode 2 and up loads the image list first. */
void sdvShadowBlockDraw(SlotBlock *block, int mode)
{
    ItemEntry *recordItems[16];
    Matrix4 transform;
    ShadowDraw shadow;
    Vector4 position;
    SceneRecord *record;
    SpriteDef *sprite;
    XglGsPacket *queue;
    XglGsPacket *start;
    u8 *camera;
    u8 *scheduler;
    float *viewRect;
    int screen[3];
    int zbuf;
    int frame;
    XglRenderStateBlock *state;
    ItemEntry *item;
    float dx;
    float dy;
    int slotPos;
    int packetId;
    int itemPos;
    int drawIndex;
    int stepIndex;
    int itemId;
    int cellX;
    int cellY;
    int gsOffsetX;
    int texU;
    int texV;
    int gsOffsetY;
    float angle;

    if (block == 0 || block->count == 0) {
        return;
    }
    for (slotPos = 0; slotPos < 4; slotPos++) {
        packetId = block->slotIds[slotPos];
        record = ((unsigned int)packetId < 16 && gSdvPacketTable[0xF00 + packetId] == 1) ? &((SceneRecord *)gSdvPacketTable)[packetId] : 0;
        if (record == 0) {
            continue;
        }
        memset(recordItems, 0, 64);
        memset(&shadow, 0, 112);
        shadow.unit1C = 1.0f;
        shadow.unit2C = 1.0f;
        shadow.unitRow3C = 1.0f;

        camera = xglStudioGetActiveCamera();
        viewRect = (float *)(camera + 96);
        MMathApplyMatrix(&position, record, (u8 *)record + 64);
        scheduler = sefGetNowScheduler();
        if ((*(u32 *)(scheduler + 4) & 0x01000000) != 0) {
            scheduler = sefGetNowScheduler();
            __asm__ __volatile__(
                "lqc2 vf1, 0(%1)\n"
                "lqc2 vf2, 0(%2)\n"
                "vadd.xyz vf1, vf1, vf2\n"
                "sqc2 vf1, 0(%0)\n"
                :
                : "r" (&position), "r" (&position), "r" (scheduler + 16), "r" (&position)
                : "memory");
        }
        _CurMatrixSet(camera + 1136);
        _CurSetViewScaleTrans(camera + 128, camera + 112);
        if (_CurRotTransPersClipScreen(screen, &position) != 0) {
            return;
        }

        gsOffsetX = screen[0] - 0x6FF8;
        if (gsOffsetX < 0) {
            gsOffsetX += 15;
        }
        gsOffsetY = screen[1] - 0x71F7;
        if (gsOffsetY < 0) {
            gsOffsetY += 15;
        }
        cellX = gsOffsetX >> 4;
        cellY = gsOffsetY >> 4;
        shadow.centre[0] = (float)cellX;
        shadow.centre[1] = (float)cellY;
        shadow.centre[2] = (float)screen[2];

        if (shadow.centre[0] < viewRect[0] || viewRect[2] < shadow.centre[0] || shadow.centre[1] < viewRect[1] || viewRect[3] < shadow.centre[1]) {
            continue;
        }
        dx = (viewRect[2] - viewRect[0]) * 0.5f - shadow.centre[0];
        dy = (viewRect[3] - viewRect[1]) * 0.5f - shadow.centre[1];
        shadow.offset[0] = dx;
        shadow.offset[1] = dy;

        for (itemPos = 0; itemPos < 16; itemPos++) {
            itemId = (signed char)record->itemIndex[itemPos];
            item = ((unsigned int)itemId < 64 && gSdvPacketTable[0xF10 + itemId] == 1) ? &((ItemEntry *)(gSdvPacketTable + 0x900))[itemId] : 0;
            recordItems[itemPos] = item;
            if (item == 0) {
                break;
            }
        }

        if (mode >= 2) {
            if (svResourceTypeLists[mode].loadCount > 0) {
                svLoadImageList(&svResourceTypeLists[mode]);
                svRefMapperPacket();
            }
        }

        state = &D_004C8390;
        nmlPacketGsInit();
        start = queue = (XglGsPacket *)&xglDirectQueueBuffer[xglDirectQueueCount];
        frame = *(u16 *)((u8 *)state + 0x0E);
        GS_QUEUE_PUT(queue, GS_FBA_1, 0);
        zbuf = state->zbuf_base;
        GS_QUEUE_PUT(queue, GS_ZBUF_1, (u64)state->zbuf_base | 0x31000000ULL);
        GS_QUEUE_PUT(queue, GS_TEST_1, 0x31001);
        GS_QUEUE_PUT(queue, GS_CLAMP_1, 0);
        GS_QUEUE_PUT(queue, GS_FRAME_1, (u64)frame | 0xFF00000030080000ULL);
        GS_QUEUE_PUT(queue, GS_ALPHA_1, (0x80ULL << 32) | 0xA8);
        GS_QUEUE_PUT(queue, GS_TEX0_1, (u64)(0x27020000 | (zbuf << 5)) | (0xC800ULL << 19));
        GS_QUEUE_PUT(queue, GS_TEX1_1, 0);
        GS_QUEUE_PUT(queue, GS_RGBAQ, 0x3F80000080808080ULL);
        GS_QUEUE_PUT(queue, GS_PRIM, 0x156);
        texU = cellX << 4;
        texV = cellY << 4;
        GS_QUEUE_PUT(queue, GS_UV, (u64)texU | ((u64)texV << 16));
        GS_QUEUE_PUT(queue, GS_XYZF2, 0x71F76FF8ULL);
        GS_QUEUE_PUT(queue, GS_UV, (u64)(texU + 16) | ((u64)(texV + 16) << 16));
        GS_QUEUE_PUT(queue, GS_XYZF2, 0x8DF78FF8ULL);
        xglDirectQueueCount += (unsigned int)((u8 *)queue - (u8 *)start) >> 4;
        xglDirectQueueSubmitRef();

        *(u64 *)&shadow.tint[0] = record->extentA;
        *(u64 *)&shadow.tint[2] = record->extentB;

        if (record->flags & 2) {
            xglMatrixUnit(transform);
            angle = xglAtan2(shadow.offset[0], shadow.offset[1]);
            xglMatrixRotZ(transform, transform, -angle);
            shadow.transform = transform;
        }

        if ((int)(gSbStudioBlock[6] & 1) != 0) {
            sprite = (SpriteDef *)((int *)gSbStudioBlock)[13];
        } else {
            sprite = &D_0053EA90[record->indexBase];
        }
        if (sprite == 0) {
            return;
        }
        for (drawIndex = 0; recordItems[drawIndex] != 0; drawIndex++) {
            shadow.texture1 = recordItems[drawIndex]->lo;
            shadow.texture0 = recordItems[drawIndex]->hi;
            shadow.blendMode = recordItems[drawIndex]->tailB;
            shadow.uvU = recordItems[drawIndex]->tailC;
            shadow.uvV = recordItems[drawIndex]->tailA;
            switch (recordItems[drawIndex]->kind) {
            case 1: {
                ((FloatWord *)&shadow.halfExtent)->value = (float)sprite->size0 * recordItems[drawIndex]->weight;
                ((FloatWord *)&shadow.scale)->value = ((float)sprite->size4 / 100.0f) * record->scale;
                ((ColourWord *)&shadow.colourB)->word = sprite->colour8.word;
                sdvShadowQueueDraw(&shadow);
            } break;
            case 2: {
                for (stepIndex = 0; sprite->steps[stepIndex].a != 0; stepIndex++) {
                    ((FloatWord *)&shadow.halfExtent)->value = (float)sprite->steps[stepIndex].a * recordItems[drawIndex]->weight;
                    ((FloatWord *)&shadow.scale)->value = ((float)sprite->steps[stepIndex].b / 100.0f) * record->scale;
                    ((ColourWord *)&shadow.colourB)->word = sprite->steps[stepIndex].c.word;
                    sdvShadowQueueDraw(&shadow);
                }
            } break;
            case 3: {
                ((FloatWord *)&shadow.halfExtent)->value = (float)sprite->size12 * recordItems[drawIndex]->weight;
                ((FloatWord *)&shadow.scale)->value = ((float)sprite->size16 / 100.0f) * record->scale;
                ((ColourWord *)&shadow.colourB)->word = sprite->colour20.word;
                sdvShadowQueueDraw(&shadow);
            } break;
            }
        }
    }
}

void sdvPacketRelease(int slot);

typedef struct SlotBlockView {
    unsigned char unmodeled_00[12];
    signed char slotIds[4];
} SlotBlockView;

/* Releases the packets of a slot block (every slot id from 0 up) and clears the block to no packets. */
void sdvReleaseShadowBlock(SlotBlockView *block)
{
    int i;
    int slotId;

    for (i = 0; i < 4; i++) {
        slotId = block->slotIds[i];
        if (slotId >= 0) {
            sdvPacketRelease(slotId);
        }
    }
    memset(block, 0, 12);
    memset(block->slotIds, -1, 4);
}


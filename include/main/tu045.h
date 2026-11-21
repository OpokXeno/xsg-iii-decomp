/* header_harvest: hand-maintained (X3 express integration; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_MAIN_TU045_H
#define INCLUDE_MAIN_TU045_H

/* Types owned by main/tu045 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"
#include "main/tu141.h"
#include "main/xgl_2.h"

/* The clip planes of a packet (five quadwords): two planes, then the rotation rows pxySetClipPlaneWithAngle fills. The
 * first angle row's x and y are also read as the clip depth and range. */
typedef struct PxyClipPlanes {
    AlignedVector4 plane[2]; /* +0x00: plane normal and distance */
    AlignedVector4 angle[2]; /* +0x20: rotX, rotY, then the next rotX, rotY */
} PxyClipPlanes;

/* The "pxy" model file header. */
typedef struct PxyModelHeader {
    char magic[4]; /* "pxy" */
    float version; /* +0x04 */
    u32 flags; /* +0x08: bit 0x2000 = per-geometry shadow mask */
    u32 fileSize[1]; /* +0x0c, a one-word array, read as *h->fileSize */
    char name[0x20]; /* +0x10 */
    int layerCount; /* +0x30 */
    int hideMaskCount; /* +0x34: the map code clears one hide-mask bit for each */
    u8 unmodeled_38[4];
    int offset3C; /* +0x3c */
    int partOffset; /* +0x40 */
    u8 unmodeled_44[4];
    int partCount; /* +0x48 */
    u8 unmodeled_4C[8];
    int rangeOffset; /* +0x54 */
    int offset58; /* +0x58 */
    int nJointMatrix; /* +0x5c: number of global part matrices (pPxyHead->nJointMatrix in the original asserts) */
    int geometryCount; /* +0x60 */
    int jointMatrixOffset; /* +0x64: offset of the joint matrices */
    int entryMatrixOffset; /* +0x68: offset of the 0x40-byte entry matrices (the default part matrices) */
    int geometryOffset; /* +0x6c: offset of the 0x70-byte geometry array */
    int offset70; /* +0x70 */
    int offset74; /* +0x74 */
    int offset78; /* +0x78 */
    u8 unmodeled_7C[0x80 - 0x7C];
    AlignedVector4 sphere; /* +0x80: bounding sphere centre, radius in w */
    float minX; /* +0x90 */
    u8 unmodeled_94[0x98 - 0x94];
    float minZ; /* +0x98 */
    u8 unmodeled_9C[0xA0 - 0x9C];
    float maxX; /* +0xA0 */
    u8 unmodeled_A4[0xA8 - 0xA4];
    float maxZ; /* +0xA8 */
    u8 unmodeled_AC[0xC0 - 0xAC];
    int waterFlag; /* +0xc0: nonzero runs tiWaterObjectUpdate */
    u8 unmodeled_C4[0xC8 - 0xC4];
    int trackOffset; /* +0xc8: offset of the PxyTrack of the flags-0x10 entries */
    int entryOffsets; /* +0xcc: offset of the int table of entry offsets */
    int subPackOffset; /* +0xd0: offset of a second pack, 0 if none */
    u8 unmodeled_D4[0xE8 - 0xD4];
    int sortEntryOffset; /* +0xe8 */
} PxyModelHeader;

typedef struct PxyHideMask { u32 bits[1]; } PxyHideMask;

/* A 0x60-byte part of a pxy model (the record the scratchpad word at 0x7000001C points at while it is drawn). */
typedef struct PxyPart {
    u8 unmodeled_00[4];
    s16 entryIndex; /* +0x04: index into the model's entry table */
    u8 unmodeled_06[2];
    signed char layer; /* +0x08: layer, also the texture channel */
    u8 unmodeled_09[7];
    s16 matrixIndex; /* +0x10: index of the part matrix, also the bit of the pxyHideMask hide mask */
    u8 unmodeled_12[2];
    s16 index; /* +0x14: index into the per-part flag bytes */
    u8 unmodeled_16[2];
    u32 flags; /* +0x18 */
    signed char styleLevel; /* +0x1C */
    u8 unmodeled_1D[3];
    Vector4 boundsMin; /* +0x20 */
    Vector4 boundsMax; /* +0x30 */
    AlignedVector4 sphere; /* +0x40: centre and radius */
    u8 unmodeled_50[0x60 - 0x50];
} PxyPart;

/* A 0x20-byte range of parts: [0] opaque, [1] translucent. */
typedef struct PxyDrawPartRange {
    u8 unk00[0x14];
    int first; /* +0x14 */
    int count; /* +0x18 */
    u8 unk1C[4];
} PxyDrawPartRange;

/* The 0x180-byte light block of a shadow projector, cleared for each model: the XglLightSet layout with the first
 * direction as a Vector4 (the matrix multiply and the w store take a vector) and the first colour as three floats. */
typedef struct PxyShadowLight {
    Vector4 direction; /* +0x00: directions[0] */
    u8 unmodeled_10[0x70 - 0x10];
    float color[3]; /* +0x70: colors[0] */
    u8 unmodeled_7C[0x180 - 0x7C];
} PxyShadowLight;

/* The shadow projector pxyCurrentShadow points to (0x36 quadwords). */
typedef struct PxyShadow {
    Matrix light; /* +0x000: light space to world */
    Matrix textureInverse; /* +0x040 */
    Matrix view; /* +0x080: world to light space */
    Matrix project; /* +0x0c0: world to shadow texture */
    PxyShadowLight lighting; /* +0x100 */
    int x; /* +0x280: texture rectangle */
    int y; /* +0x284 */
    int width; /* +0x288 */
    int height; /* +0x28c */
    s16 repeatU; /* +0x290: REGION_REPEAT instead of REGION_CLAMP */
    s16 repeatV; /* +0x292 */
    s16 psm; /* +0x294 */
    s16 frameBase; /* +0x296 */
    int clutOffset; /* +0x298 */
    u8 unk29C[4];
    float scaleU; /* +0x2a0 */
    float scaleV; /* +0x2a4 */
    u8 unk2A8[8];
    u64 tex0; /* +0x2b0 */
    u64 clamp; /* +0x2b8 */
    Vector4 uv; /* +0x2c0: offset u, scale u, offset v, scale v */
    Vector4 unk2D0; /* +0x2d0 */
    Vector4 direction; /* +0x2e0 */
    Vector4 center; /* +0x2f0 */
    Vector4 min; /* +0x300 */
    Vector4 max; /* +0x310 */
    Vector4 bounds; /* +0x320: centre and radius */
    int unk330; /* +0x330 */
    u8 unk334[4];
    u64 alphaData; /* +0x338: GS ALPHA_1 data (0x42 or 0x89) */
} PxyShadow;

/* One 0x20-byte record of a model's entry table: the offset and size of its data in the model file. */
typedef struct PxyTableEntry {
    u32 offset; /* +0x00 */
    u32 count; /* +0x04 */
    u8 unmodeled_08[0x18];
} PxyTableEntry;

/* A "txy" texture block: the magic, the set table and the flag word. nmlModelSetTexture walks its bytes from +0x10 up to
 * size; the packet code reads the offsets of the texture sets from +0x10. */
typedef struct TxyBlock {
    char magic[4]; /* "txy" */
    int setCount; /* +0x04 */
    u32 size; /* +0x08 */
    u32 flags; /* +0x0c: bit 31 set while the table is pending */
    int setOffset[8]; /* +0x10: one offset per index, to a packet set */
} TxyBlock;

/* The model packet: the 0x2a0-byte parameter block nmlModelEntry copies from nmlRenderStatus. The scratchpad word at
 * 0x70000010 points at the one being drawn. */
typedef struct PxyPacket {
    u32 renderStatus; /* +0x00 */
    int camera; /* +0x04: camera that owns the model */
    int drawMode; /* +0x08: 1 and 4 cast shadows */
    u8 unmodeled_0C[4];
    PxyModelHeader *header; /* +0x10 */
    struct PxyDrawPartRange *ranges; /* +0x14 */
    u8 unmodeled_18[4];
    u8 *channelTable; /* +0x1C: 0x70-byte record per matrix index, its bone ids from +0x30 */
    PxyPart *parts; /* +0x20 */
    u8 unmodeled_24[0x2C - 0x24];
    u8 *matrixTable; /* +0x2C */
    u8 unmodeled_30[4];
    u8 *partFlags; /* +0x34 */
    TxyBlock *textureTable; /* +0x38 */
    PxyTableEntry *entries; /* +0x3C */
    u8 unmodeled_40[4];
    float alpha; /* +0x44 */
    u8 unmodeled_48[4];
    float glowScale; /* +0x4C */
    u8 unmodeled_50[0x58 - 0x50];
    float glowIntensity; /* +0x58 */
    union Matrix *boneMatrices; /* +0x5C: one per part matrix index */
    u8 unmodeled_60[0x68 - 0x60];
    int (*levelTable)[4]; /* +0x68 */
    XglLightSet *light; /* +0x6C */
    float unmodeled_70[4]; /* +0x70 */
    struct PxyShadow *shadow; /* +0x80: the shadow projector; the matrices of its camera, the view at index 3 */
    u32 drawFlags; /* +0x84 */
    u8 unmodeled_88[0x94 - 0x88];
    float shadowY; /* +0x94 */
    union Matrix *shadowMatrix; /* +0x98 */
    int frameMask; /* +0x9c */
    u8 unmodeled_A0[4];
    s16 planeCount; /* +0xA4 */
    s16 reflectMode; /* +0xA6 */
    PxyClipPlanes *planes; /* +0xA8: clip planes */
    u8 unmodeled_AC[0xB0 - 0xAC];
    float reflectBlend; /* +0xB0 */
    u8 *reflectMask; /* +0xB4 */
    float reflectScale; /* +0xB8 */
    float reflectExtra; /* +0xBC */
    float lowerClipHeight; /* +0xC0 */
    struct PxyHideMask *hideMask; /* +0xC4 */
    float upperClipHeight; /* +0xC8 */
    u8 unmodeled_CC[4];
    u8 *partIndexList; /* +0xD0: 16 part indices ended by 0xff */
    u32 unmodeled_D4; /* +0xD4 */
    float uniformScale; /* +0xD8 */
    u8 unmodeled_DC[4];
    Matrix4 localMatrix; /* +0xE0: model to world */
    Matrix worldMatrix; /* +0x120 */
    u8 unmodeled_160[0x170 - 0x160];
    int cameraMask[4]; /* +0x170: nonzero where a camera draws the model */
    int cameraFog[4]; /* +0x180: per studio camera fog factor, out of 4080 */
    float fogColorR; /* +0x190 */
    float fogColorG; /* +0x194 */
    float fogColorB; /* +0x198 */
    u8 unmodeled_19C[4];
    float unmodeled_1A0[4]; /* +0x1A0 */
    Vector4 center; /* +0x1B0: bounding sphere centre (the shadow position) */
    void (*preDraw)(int, int); /* +0x1c0 */
    u8 unmodeled_1C4[0x1D0 - 0x1C4];
    void (*postDraw)(int, int); /* +0x1d0 */
    u8 unmodeled_1D4[0x1E0 - 0x1D4];
    float unmodeled_1E0[8][4]; /* +0x1E0 */
    float unmodeled_260[4][4]; /* +0x260 */
} PxyPacket;

#endif

/* header_harvest: hand-maintained (X3 express integration; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_MAIN_XENOACTOR_H
#define INCLUDE_MAIN_XENOACTOR_H

/* Types owned by main/tu325 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"
#include "main/TiMRPackage.h"

typedef struct XenoActor XenoActor;

/* Transform block at +0x70 of the motion hierarchy (the table at actor +0x710). */
typedef struct XenoTransform {
    unsigned char unmodeled_00[4];
    float translation[3];                   /* +0x04 */
    float rotationX;                        /* +0x10 */
    float rotationY;                        /* +0x14 */
    float rotationZ;                        /* +0x18 */
    float scale[3];                         /* +0x1C */
    unsigned char unmodeled_28[0x44 - 0x28];
    float point[3];                         /* +0x44 */
} XenoTransform;

/* Per-joint record of an actor; the translation is accessed through the vector union (the original's float/word access). */
typedef struct XenoJointRecord {
    float row0[4];
    DoublewordVector4 translation;          /* +0x10 */
    float rest[8];
} XenoJointRecord;

/* Motion hierarchy (HRC) of an actor (actor->table). */
typedef struct XenoHrc {
    unsigned char unmodeled_00[0xC];
    unsigned int numTransform;              /* +0x0C */
    int leadCountA;                         /* +0x10 */
    int leadCountB;                         /* +0x14 */
    int leadCountC;                         /* +0x18 */
    int focus;                              /* +0x1C */
    int jointRecordCount;                   /* +0x20 */
    unsigned int jointCount;                /* +0x24 */
    unsigned char unmodeled_28[0x3C - 0x28];
    unsigned int count;                     /* +0x3C */
    unsigned char unmodeled_40[0x4C - 0x40];
    int dataOffset;                         /* +0x4C */
    unsigned char unmodeled_50[0x5C - 0x50];
    unsigned int flags;                     /* +0x5C */
    unsigned char unmodeled_60[0x70 - 0x60];
    XenoTransform transform;                /* +0x70 */
} XenoHrc;

typedef struct XenoMatrix {
    float m[4][4];
} XenoMatrix;

/* The 0x180-byte light set of an actor: the battle actors copy it whole (unitCreateSide in X2 src/ov01/unit_cmd.c), and
 * uwamonoLightBlendStep blends the vectors. */
typedef struct XenoLight {
    float directions[3][4];                 /* +0x00 */
    unsigned char unmodeled_30[0x70 - 0x30];
    float colors[3][4];                     /* +0x70 */
    unsigned char unmodeled_a0[0xD0 - 0xA0];
    float ambient[4];                       /* +0xD0 */
    unsigned char unmodeled_e0[0xF0 - 0xE0];
    float fourthDirection[4];               /* +0xF0 */
    u64 unmodeled_100[(0x12C - 0x100) / 8]; /* copied as doublewords with the rest of the block */
    unsigned char unmodeled_128[4];
    float intensity;                        /* +0x12C */
    u64 unmodeled_130[(0x180 - 0x130) / 8];
} XenoLight;

/* The actor of a map object, a battle unit or a cut-scene model: one record of the pool of src/main/xenoActor.c
 * (0x7F0 bytes each). Names of the original's asserts are used where the code shows them. */
struct XenoActor {
    unsigned short attr;                    /* +0x000: bit 0 = live pool record; 0 marks a free one */
    unsigned char unmodeled_002[0x4 - 0x2];
    int unmodeled_004;                      /* +0x004: cleared by the actor init */
    unsigned char unmodeled_008[0x10 - 0x8];
    float unmodeled_010;                    /* +0x010: 0.3 in the actor init */
    unsigned char unmodeled_014[0x20 - 0x14];
    DoublewordAlignedVector4 position;      /* +0x020 */
    Vector4 previousPosition;               /* +0x030: copy of the row at 0x20 (x and z before this frame's move) */
    float angle[4];                         /* +0x040: copy of rotation, its three angles wrapped into [0, 2*pi) */
    Vector4 rotation;                       /* +0x050: y is the facing angle */
    Vector4 vector60;                       /* +0x060: its w is 1.0 in the actor init */
    Vector4 vector70;                       /* +0x070 */
    Vector4 scale;                          /* +0x080 */
    int drawFlags;                          /* +0x090 */
    float alpha;                            /* +0x094 */
    float unmodeled_098;                    /* +0x098: 1.0 in the actor init */
    unsigned short unmodeled_09c;           /* +0x09C: cleared by the actor init */
    unsigned char unmodeled_09e[0xA0 - 0x9E];
    float unmodeled_0a0;                    /* +0x0A0 */
    unsigned char unmodeled_0a4[0xAC - 0xA4];
    float unmodeled_0ac;                    /* +0x0AC: 1.0 in the actor init */
    float localMatrix[4][4];                /* +0x0B0: the actor's own matrix, used when overrideMatrix is 0 */
    float focusX;                           /* +0x0F0 */
    float focusY;                           /* +0x0F4 */
    int unmodeled_0f8;                      /* +0x0F8: cleared by the actor init */
    int unmodeled_0fc;                      /* +0x0FC */
    float sweepPhase;                       /* +0x100: written by the shadow keys (tu122) */
    float sweepSpeed;                       /* +0x104 */
    float shadowValue;                      /* +0x108 */
    short shadowCount;                      /* +0x10C */
    short moveState;                        /* +0x10E */
    float clipPlaneScale;                   /* +0x110 */
    float shadowValue3;                     /* +0x114 */
    unsigned char unmodeled_118[0x4];
    int unmodeled_11c;                      /* +0x11C: cleared by the actor init */
    int unmodeled_120;                      /* +0x120 */
    float unmodeled_124;                    /* +0x124: 0.5 in the actor init */
    float unmodeled_128;                    /* +0x128: -1.0 in the actor init */
    float unmodeled_12c;                    /* +0x12C: 1.0 in the actor init */
    int unmodeled_130;                      /* +0x130 */
    float alphaScale;                       /* +0x134: distance fade factor of the draw xglFontCmdAlpha (measured from X3 asm) */
    float fadeRadius;                       /* +0x138: radius override of the camera fade, 0 = use the pack sphere */
    float unmodeled_13c;                    /* +0x13C */
    XenoLight light;                        /* +0x140: the light set the actor init defaults (func_001975C8) */
    float color[4];                         /* +0x2C0 */
    float colorBlock[3];                    /* +0x2D0 */
    unsigned char unmodeled_2dc[0x2F0 - 0x2DC];
    unsigned int visibility[8];             /* +0x2F0 */
    unsigned int subVisibility[8];          /* +0x310 */
    unsigned char unmodeled_330[0x20];      /* +0x330: 0xFF in the actor init */
    unsigned char unmodeled_350[0x100];     /* +0x350: 0xFF in the actor init */
    int unmodeled_450;                      /* +0x450: cleared by the actor init */
    int unmodeled_454;                      /* +0x454 */
    unsigned char unmodeled_458[0x460 - 0x458];
    AlignedVector4 clipPlaneA;              /* +0x460 */
    unsigned char unmodeled_470[0x480 - 0x470];
    AlignedVector4 clipPlaneB;              /* +0x480 */
    unsigned char unmodeled_490[0x4B0 - 0x490];
    DoublewordAlignedVector4 sourceBlock;   /* +0x4B0: 0xFF in the actor init */
    void *overrideMatrix;                   /* +0x4C0 */
    struct XenoActor *parent;               /* +0x4C4 */
    unsigned char attachSlotIndex[0x4];       /* +0x4C8 */
    struct XenoActor *children[8];          /* +0x4CC */
    unsigned int resetFlags;                /* +0x4EC */
    float frame;                            /* +0x4F0: loop frame of the current motion (the cached play position) */
    short currentMotionSlot;                    /* +0x4F4 */
    short currentMotionTable;                    /* +0x4F6 */
    short requestedMotionSlot;                    /* +0x4F8 */
    unsigned char unmodeled_4fa[2];
    float selectorRangeMin;                    /* +0x4FC: cached range minimum of package B's entry */
    short selectedEntry;                    /* +0x500: selected entry of package B */
    unsigned char unmodeled_502[2];
    int rsrc[12];                           /* +0x504: XAct_Rsrc_Count entries */
    unsigned char unmodeled_534[0x540 - 0x534];
    void *pLocalMat;                        /* +0x540: heap-5 buffers (names from the original asserts), one 0x40-byte matrix per hierarchy transform */
    void *pGlobalMat;                       /* +0x544 */
    void *pBlendMat;                        /* +0x548 */
    int unmodeled_54c;                      /* +0x54C */
    void *pPartsMat;                        /* +0x550 */
    unsigned char unmodeled_554[0x560 - 0x554];
    Vector4 shadowB;                        /* +0x560 */
    unsigned char shadowB2[0x10];           /* +0x570 */
    unsigned char shadowC[0x10];            /* +0x580 */
    unsigned char colorIndex[16];           /* +0x590 */
    unsigned char unmodeled_5a0[0x4];       /* +0x5A0 */
    unsigned char unmodeled_5a4[0x8];       /* +0x5A4 */
    unsigned char unmodeled_5ac[0x20];      /* +0x5AC */
    unsigned char unmodeled_5cc[0x620 - 0x5CC];
    struct TiOffsetEntry *motionKeys[4];    /* +0x620 */
    struct XenoHrc *pHrcData;               /* +0x630 */
    unsigned char unmodeled_634[0x650 - 0x634];
    struct XenoMatrix *jointMatrices;              /* +0x650 */
    unsigned char unmodeled_654[4];
    int unmodeled_658;                      /* +0x658: nonzero when the package has solver data */
    XenoJointRecord *jointRecords;          /* +0x65C */
    float (*currentMatrix)[4];              /* +0x660: points at the matrix the package is evaluated against (localMatrix) */
    unsigned char unmodeled_664[0x694 - 0x664];
    void *pDynamics;                        /* +0x694: freed by XenoActorRelease */
    unsigned char unmodeled_698[0x6A0 - 0x698];
    unsigned char collision[0x6D0 - 0x6A0]; /* +0x6A0: the block the actor init initializes (func_002A70E8) */
};

/* One motion state of the actor's TiMRPackage channel block (TiMRPackage.c's TiMRState, 0x50 bytes). */
typedef struct XenoMotionState {
    int flags;                              /* +0x00 */
    float speed;                            /* +0x04 */
    float frame;                            /* +0x08 */
    float value;                            /* +0x0C */
    int range[2];                           /* +0x10 */
    unsigned char unmodeled_18[8];
    Vector4 vectors[3];                     /* +0x20 */
} XenoMotionState;

#endif

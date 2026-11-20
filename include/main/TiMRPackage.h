/* header_harvest: hand-maintained (X3 express integration; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_MAIN_TIMRPACKAGE_H
#define INCLUDE_MAIN_TIMRPACKAGE_H

/* Types owned by main/tu337 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"
#include "main/TiDynamicsColl.h"

typedef struct OffsetTable OffsetTable;

/* Partial models of the owner's structs (src/main/TiMRPackage.c): only the fields this TU reads. */
typedef struct TiHrcData TiHrcData;

/* Dynamics node: the sphere and matrix entry counts (+0x1C, +0x20); the collision entries (16 bytes each) start at
 * +0xD0, sphere entries first. */
typedef struct TiHrcDynamics TiHrcDynamics;

/* The package: the first word, the state blocks at +0x10 and +0x60, the entry of state A at +0xC0, the hierarchy
 * data at +0xD0 and the matrix and solver arrays at +0xEC, +0xF0 and +0xFC. */
typedef struct TiMRPackage TiMRPackage;

/* Row-major 4x4 float matrix. */
typedef struct TiMRState TiMRState;

typedef struct TiNodeTable TiNodeTable;

/* One entry of an offset table, bound to a state as its animation data. */
typedef struct TiOffsetEntry TiOffsetEntry;

typedef struct TiPamData TiPamData;

/* Solver body the collision lists push: the point (+0x10) and the accumulated correction (+0x20). Other fields are not
 * modeled here. */
typedef struct TiSolverState TiSolverState;

/*
 * Counted table of entries: the entry count sits at +8 and a table of byte
 * offsets, relative to the table itself, starts at +0xC.
 */
struct OffsetTable {
    unsigned char unmodeled_00[8];
    unsigned int count;                     /* +0x08 */
    int entryOffsets[1];                    /* +0x0C: byte offset of entry i from the table */
};

/* One joint of the hierarchy (0x60 bytes). The default translation is also the range the aim solver reads. */
typedef struct TiHrcJoint {
    int parent;
    float translationDefault[3];
    float orient[3];
    float rotationDefault[3];
    float scaleDefault[3];
    float scale[3];
    float rotationScale;
    int flags;
    unsigned char unmodeled_48[0x18];
} TiHrcJoint;

/* One locator of the hierarchy (0x40 bytes): its scale is at +0x28 and its flag word at +0x34. */
typedef struct TiHrcLocator {
    int parent;
    float translationDefault[3];
    float rotationDefault[3];
    float scaleDefault[3];
    float scale[3];
    int flags;
    unsigned char unmodeled_38[0x8];
} TiHrcLocator;

/*
 * The hierarchy data at +0xD0 of the package: the node counts and table offsets, the collision record
 * tables (spheres, matrix records and capsules, see TiDynamicsColl.h) and the nodes themselves: two locators
 * at +0x70, then the joints from +0x110 (stride 0x60).
 */
struct TiHrcData {
    unsigned char unmodeled_00[0x4];
    float version;          /* +4 */
    unsigned char unmodeled_08[0x4];
    unsigned int transformCount; /* +0xC: transforms of the hierarchy; the actor allocates one 0x40-byte matrix each */
    int jointCount;
    int joint2Count;
    int joint3Count;
    int skipCount;
    int dynamicsCount;
    int staticCount;
    int joint2Offset;
    int joint3Offset;
    int skipOffset;
    int dynamicsOffset;
    int staticOffset;
    unsigned int sphereCount;   /* +0x3C */
    unsigned int matrixCount;
    unsigned int capsuleCount;
    int linkCellCount;          /* +0x48: cells of the link table (tiDynamicsSolveCellPairs) */
    int sphereOffset;           /* +0x4C: offsets of the collision records from the data */
    int matrixOffset;
    int capsuleOffset;
    int linkCellOffset;         /* +0x58: offset of the link table from the data */
    int flags;                  /* +0x5C: bit 0x8 leaves one locator */
    unsigned char unmodeled_60[0x10];
    TiHrcLocator locators[2];   /* +0x70 */
    unsigned char unmodeled_f0[0x20];
    TiHrcJoint joints[1];       /* +0x110 */
};

/* One dynamics node: 0xD0 bytes followed by its collision entries (sphereCount spheres, then matrixCount matrix
 * records, then capsuleCount capsules). The solver reads the first doubleword (parent and solverFlags) whole. */
struct TiHrcDynamics {
    union {
        unsigned long long header;
        struct {
            int parent;
            unsigned int solverFlags; /* +0x04: bit 0x2 and 0x4 zero the local z and y of the position (tiDynamicsNodeSolvePalette) */
        };
    };
    float normalScale;        /* +0x08 */
    float biasY;              /* +0x0C */
    float damping;            /* +0x10: the accumulator is scaled by it after each step */
    float pullScale;          /* +0x14: how strongly the point is pulled to its target */
    float timeStep;           /* +0x18: multiplies the wind push (tiWindDirApply) */
    unsigned int sphereCount; /* +0x1C */
    unsigned int matrixCount; /* +0x20 */
    unsigned int capsuleCount; /* +0x24 */
    int flags;                /* +0x28: bit 0x2 is the object mode bit */
    float weight;             /* +0x2C */
    Vector4 translationDefault; /* +0x30 */
    float orient[3];          /* +0x40 */
    unsigned char unmodeled_4C[0x4];
    float rotationDefault[3]; /* +0x50 */
    unsigned char unmodeled_5C[0x4];
    float scaleDefault[3];    /* +0x60 */
    unsigned char unmodeled_6C[0x4];
    float scale[3];           /* +0x70 */
    unsigned char unmodeled_7C[0x90 - 0x7C];
    Vector4 unmodeled_90;     /* +0x90 */
    unsigned char unmodeled_A0[0xD0 - 0xA0];
    TiCollEntry entries[0];   /* +0xD0 */
};

/*
 * One state block, reached at +0x10 (state A) and +0x60 (state B) of TiMRPackage.
 * Its first word is a flag word: the advance functions test bits 0x2, 0x8 and
 * 0x20 of it and the table selectors set bit 0x40. `speed` (+4) is the rate the
 * advance functions multiply by the global step; `position` (+8) is the value
 * they accumulate and compare with the range. The block size of 0x50 follows
 * from the second block's offset. `value` (+0xC) is stored by tiMRStateSetValue and
 * passed by XenoActorSetChannelValue (and its B variant) to the channel blocks
 * of XenoActor at +0x650 and +0x6A0; no reader of it is found in this TU.
 * State A also holds the aim target of tiMotionAimTarget (flag 0x10 applies its angle): the world
 * position to aim at, the smoothing bounds and the current rotations.
 */
struct TiMRState {
    int flags;      /* +0x0: flag word (tiWordSetBits/tiWordClearBits) */
    float speed;
    float position;
    float value;
    int rangeMin;   /* +0x10: the range of the advance (tiMRStateSetRange) */
    int rangeMax;   /* +0x14 */
    unsigned char unmodeled_18[0x8];
    Vector4 aimVector;   /* +0x20 */
    float aimBounds[4];  /* +0x30 */
    float aimRotateX;    /* +0x40 */
    float aimAngle;      /* +0x44 */
    float aimRotateZ;    /* +0x48 */
    unsigned char unmodeled_4C[0x4];
};

/*
 * The package (0x140 bytes: the initializer memsets 0x140). The first word, the target node of the aim at +0x04,
 * the two state blocks, the scale vector, the entry and table pointers of each state, the hierarchy pointer and
 * the matrix and solver arrays at +0xEC, +0xF0, +0xF4 and +0xFC are evidenced.
 */
struct TiMRPackage {
    int first_word;
    int targetNode;           /* +0x04: node index that the aim solver targets (0x1D after the initializer) */
    unsigned char unmodeled_08[8];
    TiMRState state_a;        /* +0x10 */
    TiMRState state_b;        /* +0x60 */
    float scale[3];           /* +0xB0: (1, 1, 1) after the initializer; multiplies the translation of the second locator */
    unsigned char unmodeled_bc[4];
    struct TiOffsetEntry *entryA;    /* +0xC0: entry of state A; the update checks its version float as pAnimData (xac) */
    struct OffsetTable *entryTableA; /* +0xC4: offset table of state A (tiMRPackageSelectAnimEntry) */
    struct TiOffsetEntry *entryB;    /* +0xC8: entry of state B; the update checks its version float as pFaceAnimData (fac) */
    struct OffsetTable *entryTableB; /* +0xCC: offset table of state B (tiMRPackageSelectFaceAnimEntry) */
    struct TiHrcData *hrcData;       /* +0xD0: pHrcData, version float at +4 */
    struct TiPamData *pamData;       /* +0xD4: parts animation data, its two shorts at +8 and +A bound state A */
    struct TiNodeTable *nodeTable;   /* +0xD8: pStatus, the node point table (tiMRPackageSetEntryTableC) */
    void *xstData;            /* +0xDC: header tagged "XST ": its ints at +0xC and +0x10 are the range of state A (func_00299730) */
    void *xstRecord;          /* +0xE0: the record of xstData that func_00299770 selected for the playback position, 0 when none */
    unsigned char unmodeled_e4[4];
    int unmodeled_e8;         /* +0xE8 */
    Matrix4 *localMat;        /* +0xEC: pLocalMat, one matrix per node (stored by tiMRPackageSetLocalMat) */
    Matrix4 *globalMat;       /* +0xF0: pGlobalMat, the placed matrices (stored by tiMRPackageSetGlobalMat) */
    Matrix4 *blendMat;        /* +0xF4: the blend matrices (stored by tiMRPackageSetBlendMat) */
    void *valueF8;            /* +0xF8: stored by tiMRPackageSetPartsMat */
    struct TiSolverState *dynMat; /* +0xFC: pDynMat, one solver body per dynamics node (stored by func_00299698) */
    unsigned char unmodeled_100[0x10];
    float copyDst110;         /* +0x110: receives copySrc114 when TiMtnBlend swaps a matrix array with blendMat */
    float copySrc114;         /* +0x114 */
    unsigned char unmodeled_118[8];
    float copyDst120;         /* +0x120: receives copySrc124 likewise */
    float copySrc124;         /* +0x124 */
    unsigned char unmodeled_128[8];
    float timeScale;          /* +0x130: stored as 1.0f by tiDynamicsNodeSolveNext (name provisional) */
    unsigned char unmodeled_134[0xC];
};

/* One entry of the indexed node table (0x1C bytes): its matrix index and transform. */
typedef struct TiNodeSlot {
    int matrixIndex;
    float offset[3];
    float rotation[3];
} TiNodeSlot;

/* Keyed node entry (0x20 bytes) of the node table when its threshold is 1.1f or more: its id, matrix index, offset and rotation. */
typedef struct TiNodeKey {
    u32 id;                     /* +0x00 */
    int matrixIndex;            /* +0x04 */
    float offset[3];            /* +0x08 */
    float rotation[3];          /* +0x14 */
} TiNodeKey;

/* The node table at +0xD8 of the package: a threshold at +4 (below 1.1f the slots are indexed, else the keys are
 * searched by id), the entry count at +8 and the entries from +0x10. */
struct TiNodeTable {
    unsigned char unmodeled_00[4];
    float threshold;
    unsigned int count;
    unsigned char unmodeled_0C[4];
    TiNodeSlot slots[0];
};

/* Animation data of the first format (version below 1.4): the joint, second joint and dynamics tables. */
typedef struct TiXacTables1 {
    int jointCount;
    int joint2Count;
    int unmodeled_08;
    int dynamicsCount;
    int jointOffset;
    int joint2Offset;
    int unmodeled_18;
    int dynamicsOffset;
} TiXacTables1;

/* Animation data of the second format: it adds the skip, third joint and visibility tables. */
typedef struct TiXacTables2 {
    int jointCount;
    int joint2Count;
    int skipCount;
    int dynamicsCount;
    unsigned int visibilityCount;
    int joint3Count;
    int skipOffset;
    int unmodeled_1C;
    int jointOffset;
    int joint2Offset;
    int unmodeled_28;
    int dynamicsOffset;
    int visibilityOffset;
    int joint3Offset;
} TiXacTables2;

/*
 * One entry of an offset table, bound to a state as its animation data: the
 * per-frame update checks its version float at +4; its float range is at +8
 * (min) and +C (max). The tables from +0x10 have one of two layouts by version.
 */
struct TiOffsetEntry {
    unsigned char unmodeled_00[4];
    float version;   /* +4 */
    float rangeMin;  /* +8 */
    float rangeMax;  /* +C */
    union {
        TiXacTables1 first;
        TiXacTables2 second;
    } tables;        /* +0x10 */
};

/* The parts animation data at +0xD4 of the package: two shorts bound state A (lower at +8, upper at +A), the joint
 * curves follow from +0xC (tiPartsAnimSamplePose reads them). */
struct TiPamData {
    unsigned char unmodeled_00[8];
    short lowerLimit;  /* +8 */
    short upperLimit;  /* +A */
    unsigned char curves[0]; /* +0xC */
};

/* One solver body (the pDynMat entry of a dynamics node, 0x40 bytes): the point, the accumulated correction and
 * the node. The vectors are DoublewordVector4 unions: the pair solvers copy and add them through the union, so
 * their stores carry alias set 0 (check.py). */
struct TiSolverState {
    unsigned char previous_position[0x10];
    DoublewordVector4 position;     /* +0x10 */
    DoublewordVector4 accumulator;  /* +0x20 */
    struct TiHrcDynamics *node;     /* +0x30 */
    float gust;               /* +0x34: wind gust factor, redrawn by the wind passes */
    unsigned char unmodeled_38[0x8];
};

#endif

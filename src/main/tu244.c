#include "common.h"

#include "shared.h"
#include "vu0_macro.h"

/* One value of the scratchpad block at 0x70000000 (the effect work area). */
#define SEF_SCRATCHPAD(type, offset) (*(type *) (0x70000000 + (offset)))
typedef signed char s8;

typedef unsigned char u8;

typedef int s32;

typedef float f32;

/* The 0xF0-byte state record at gTextCommandStateStorage (fields by the offsets the functions use). */
typedef struct TextCommandState {
    u8 commandFlags;                       /* 0x00: bit 0x2 set by textCommandStateSetKinds */
    u8 unk_01;
    u8 mirrorFlag;                  /* 0x02: nonzero negates the first value (with mirrorSuppressFlags bit 0x100 clear) */
    u8 mirrorSecondMode;                        /* 0x03: >= 3 negates the second value */
    u8 unk_04[0x7C];
    s32 mirrorSuppressFlags;                     /* 0x80: bit 0x100 */
    u8 unk_84[0x10];
    f32 keptScaledValue;                     /* 0x94: written by textCommandStateSetScaledValue unless commandFlags bit 0x2 is set */
    f32 lastScaledValue;                     /* 0x98: written by textCommandStateSetScaledValue */
    u8 unk_9C[0x44];
    f32 scale;                      /* 0xE0 */
    s8 primaryTextKind;                      /* 0xE4: set by textCommandStateSetKinds */
    s8 secondaryTextKind;                      /* 0xE5: set by textCommandStateSetKinds */
    u8 unk_E6[2];
    void *commandCallback;                 /* 0xE8: func_00265398 */
    u8 unk_EC[4];
} TextCommandState;

extern unsigned char gTextCommandStateStorage[];
extern unsigned char func_00265398[];
void *memset();

/* The state record at gTextCommandStateStorage, read through one typed view. */
#define gTextCommandState (*(TextCommandState *) gTextCommandStateStorage)

void textCommandStateReset(void) {
    memset(gTextCommandStateStorage, 0, 0xF0);
    gTextCommandState.commandCallback = func_00265398;
}

typedef struct TextCommandStateView {
    u8 commandFlags; /* 0x00: bit 0x2 set by textCommandStateSetKinds */
    u8 unk_01;
    u8 mirrorFlag; /* 0x02: nonzero negates the first value (with mirrorSuppressFlags bit 0x100 clear) */
    u8 mirrorSecondMode; /* 0x03: >= 3 negates the second value */
    u8 unk_04[0x7C];
    s32 mirrorSuppressFlags; /* 0x80: bit 0x100 */
    u8 unk_84[0x10];
    f32 keptScaledValue; /* 0x94: written by textCommandStateSetScaledValue unless commandFlags bit 0x2 is set */
    f32 lastScaledValue; /* 0x98: written by textCommandStateSetScaledValue */
    u8 unk_9C[0x44];
    f32 scale;            /* 0xE0 */
    s8 primaryTextKind;   /* 0xE4: set by textCommandStateSetKinds */
    s8 secondaryTextKind; /* 0xE5: set by textCommandStateSetKinds */
    u8 unk_E6[2];
    void *commandCallback; /* 0xE8: func_00265398 */
    s32 unk_EC;
} TextCommandStateView;

#define gTextCommandStateView (*(TextCommandStateView *) gTextCommandStateStorage)

void battleTextCommandUpdateEndpoints(void);
void battleSlotRadiusPoint(Vector4 *outPoint, s32 pointIndex);
void battleSlotBlendPoint(Vector4 *outPoint, s32 pointIndex);
void battleCameraAnglesEmit(void *outPosition, Vector4 *fromPoint, s32 ringFlags, f32 scale);
void battleSegmentHeightPoint(Vector4 *outPoint, Vector4 *sourcePoint, Vector4 *point, f32 blendFactor);
void func_00265208(Vector4 *angles, u8 a, u8 b);
void func_002655B0(f32 value, u8 a, u8 b);
void func_00265638(Vector4 *angles);
extern DoublewordAlignedVector4 gBattleScratchPoint;
extern u8 gBattleModeActors[];
/* The battle mode block at gBattleModeActors: the mode, the two actor slots and the battle phase byte at +3. */
typedef struct BattleModeState {
    s8 battleMode;      /* +0x00 */
    u8 firstActorSlot;  /* +0x01 */
    u8 secondActorSlot; /* +0x02 */
    s8 battlePhase;     /* +0x03: the D_003E0173 phase written by battleSetPhase */
    u8 unk_04[2];
} BattleModeState;

#define gBattleModeView (*(BattleModeState *) gBattleModeActors)

/* Per-frame update of the battle text command while gBattleModeActors+3 is 2. It refreshes the blended
 * points from commandFlags, then calls the stored commandCallback (gTextCommandState.commandCallback). Called by battleUpdateStep. */
void battleTextCommandUpdate(void) {
    Vector4 basePos;
    DoublewordAlignedVector4 blendedPos;

    if (gBattleModeView.battlePhase == 2 && gTextCommandStateView.commandFlags != 0) {
        battleTextCommandUpdateEndpoints();
        if (gTextCommandStateView.commandFlags & 5) {
            battleSlotRadiusPoint(&blendedPos, 0);
            /* kept: battleSlotBlendPoint is declared with two parameters; the call passes the blend buffer too */
            ((void (*)(Vector4 *dest, s32 index, DoublewordAlignedVector4 *unusedBlend)) battleSlotBlendPoint)(
                &basePos, 0, &blendedPos);
            if (gTextCommandStateView.unk_EC != 0) {
                battleCameraAnglesEmit(&blendedPos, &basePos, gTextCommandStateView.unk_EC, 0.6981312037f);
            }
            if (gTextCommandStateView.mirrorSuppressFlags & 0x40) {
                if (gTextCommandStateView.unk_01 != 0) {
                    gBattleScratchPoint = blendedPos;
                }
                battleSegmentHeightPoint(&blendedPos, &gBattleScratchPoint, &basePos, 0.6981312037f);
                gBattleScratchPoint = blendedPos;
                battleSegmentHeightPoint(&blendedPos, &gBattleScratchPoint, &basePos, 0.6981312037f);
            }
        }
        if (gTextCommandStateView.commandFlags & 1) {
            func_00265208(&basePos, 0, 0);
            ((void (*)(Vector4 *, s32, s32)) gTextCommandStateView.commandCallback)(&blendedPos, 0, 0);
            {
                func_002655B0(gTextCommandStateView.keptScaledValue, 0, 0);
            }
        }
        if (gTextCommandStateView.commandFlags & 6) {
            battleSlotRadiusPoint(&blendedPos, 1);
            ((void (*)(Vector4 *dest, s32 index, DoublewordAlignedVector4 *unusedBlend)) battleSlotBlendPoint)(
                &basePos, 1, &blendedPos);
            if (gTextCommandStateView.unk_EC != 0) {
                battleCameraAnglesEmit(&blendedPos, &basePos, gTextCommandStateView.unk_EC, 0.6981312037f);
            }
        }
        if (gTextCommandStateView.commandFlags & 2) {
            func_00265208(&basePos, gTextCommandStateView.secondaryTextKind,
                          gTextCommandStateView.primaryTextKind);
            ((void (*)(Vector4 *, u8, u8)) gTextCommandStateView.commandCallback)(
                &blendedPos, gTextCommandStateView.secondaryTextKind,
                gTextCommandStateView.primaryTextKind);
            {
                func_002655B0(gTextCommandStateView.lastScaledValue,
                              gTextCommandStateView.secondaryTextKind,
                              gTextCommandStateView.primaryTextKind);
            }
        }
        if (gTextCommandStateView.commandFlags & 4) {
            func_00265638(&basePos);
        }
        gTextCommandStateView.unk_01 = 0;
        if (!(gTextCommandStateView.mirrorSuppressFlags & 1)) {
            gTextCommandStateView.commandFlags &= 0xFC;
        }
    }
}

typedef struct TextCommandStateBlend {
    u8 commandFlags; /* 0x00: bit 0x2 set by textCommandStateSetKinds */
    u8 unk_01;
    u8 mirrorFlag; /* 0x02: nonzero negates the first value (with mirrorSuppressFlags bit 0x100 clear) */
    u8 mirrorSecondMode; /* 0x03: >= 3 negates the second value */
    u8 unk_04[0x0C];
    Vector4 startPoint; /* 0x10: first end of the blend of battleSlotBlendPoint (x, y, z) */
    Vector4 endPoint;   /* 0x20: second end of the blend */
    u8 useMatrixFlag;   /* 0x30: nonzero transforms the blend by matrix in battleSlotBlendPoint */
    u8 unk_31[0x0F];
    Matrix4 matrix;          /* 0x40 */
    s32 mirrorSuppressFlags; /* 0x80: bit 0x100 */
    f32 blendFactor[4];      /* 0x84: indexed by the caller's index in battleSlotBlendPoint */
    f32 keptScaledValue; /* 0x94: written by textCommandStateSetScaledValue unless commandFlags bit 0x2 is set */
    f32 lastScaledValue; /* 0x98: written by textCommandStateSetScaledValue */
    u8 unk_9C[4];
    Vector4 vectors[4];   /* 0xA0: indexed by the caller's index in battleSlotBlendPoint */
    f32 scale;            /* 0xE0 */
    s8 primaryTextKind;   /* 0xE4: set by textCommandStateSetKinds */
    s8 secondaryTextKind; /* 0xE5: set by textCommandStateSetKinds */
    u8 unk_E6[2];
    void *commandCallback; /* 0xE8: func_00265398 */
    u8 unk_EC[4];
} TextCommandStateBlend;

#define gTextCommandStateAlt (*(TextCommandStateBlend *) gTextCommandStateStorage)

void vector3fLerp(Vector4 *outVector, Vector4 *startVector, Vector4 *endVector, f32 t);
void xglVectorMulMat(Vector4 *destination, Matrix4 matrix, const Vector4 *vector);
void xglVectorNormal(Vector4 *destination, const Vector4 *source);
void xglVectorScale(Vector4 *destination, const Vector4 *source, f32 scale);
f32 actorSlotGetRadius(s32 slot);

/* Lerps the point of slot index+2 between the two endpoints, offsets it by the matrix or by the slot vector,
 * and keeps the result above y 0.5. Called from battleTextCommandUpdate. */
void battleSlotRadiusPoint(Vector4 *outPoint, s32 pointIndex) {
    Vector4 transformedVector;
    Vector4 scaledOffset;
    TextCommandStateBlend *blendState;
    f32 firstSlotRadius;
    f32 blendedRadius;

    vector3fLerp(outPoint, &gTextCommandStateAlt.startPoint, &gTextCommandStateAlt.endPoint,
                 gTextCommandStateAlt.blendFactor[pointIndex + 2]);
    if (gTextCommandStateAlt.useMatrixFlag != 0) {
        xglVectorMulMat(&transformedVector, gTextCommandStateAlt.matrix,
                        &gTextCommandStateAlt.vectors[pointIndex + 2]);
        Vu0AddVector(outPoint, outPoint, &transformedVector);
    } else {
        Vu0AddVector(outPoint, outPoint, &gTextCommandStateAlt.vectors[pointIndex + 2]);
    }
    blendState = &gTextCommandStateAlt;
    if (!(blendState->mirrorSuppressFlags & 0x400)) {
        firstSlotRadius = actorSlotGetRadius(blendState->mirrorSecondMode);
        blendedRadius = actorSlotGetRadius(blendState->unk_04[0]);
        blendedRadius = firstSlotRadius + (blendedRadius - firstSlotRadius) * blendState->blendFactor[pointIndex + 2];
        if (blendState->mirrorSuppressFlags & 2) {
            if (0.0f < blendState->vectors[pointIndex + 2].x) {
                outPoint->x = outPoint->x + blendedRadius;
            } else {
                outPoint->x = outPoint->x - blendedRadius;
            }
        } else {
            {
                Vector4 *slotVectors = blendState->vectors + 2;
                xglVectorNormal(&scaledOffset, slotVectors + pointIndex);
            }
            xglVectorScale(&scaledOffset, &scaledOffset, blendedRadius);
            Vu0AddVector(outPoint, outPoint, &scaledOffset);
        }
    }
    if (outPoint->y < 0.5f) {
        outPoint->y = 0.5f;
    }
}

/* Lerps the point of slot index between the two endpoints and offsets it by the matrix or by the slot vector. */
void battleSlotBlendPoint(Vector4 *outPoint, s32 pointIndex) {
    Vector4 transformedVector;
    TextCommandStateBlend *blendState;

    vector3fLerp(outPoint, &gTextCommandStateAlt.startPoint, &gTextCommandStateAlt.endPoint,
                 gTextCommandStateAlt.blendFactor[pointIndex]);
    blendState = &gTextCommandStateAlt;
    if (blendState->useMatrixFlag != 0) {
        xglVectorMulMat(&transformedVector, gTextCommandStateAlt.matrix, &gTextCommandStateAlt.vectors[pointIndex]);
        Vu0AddVector(outPoint, outPoint, &transformedVector);
    } else {
        Vu0AddVector(outPoint, outPoint, &gTextCommandStateAlt.vectors[pointIndex]);
    }
}


void cameraAnglesBetween(Vector4 *angles, s32 a1, void *a0, f32 scale);
void tiRingEmitMode2(void *ring, s32 *count, s32 mode);
s32 actorSlotGetVector(void *ring, s32 mode, s32 kind);
void func_00266B40(void *target, Vector4 *angles, void *ring, s32 count, s32 zero, f32 scale);
extern unsigned char gBattleSideSplit[];

/* Gathers the scratchpad ring entries selected by flags (their count goes to func_00266B40) and writes the
 * camera angles from fromPoint to target. Called by battleTextCommandUpdate and battleSegmentHeightPoint. */
void battleCameraAnglesEmit(void *outPosition, Vector4 *fromPoint, s32 ringFlags, f32 scale) {
    Vector4 angles;
    s32 ringCount;
    s32 i;
    s32 k;
    s32 sideFirstSlot;
    s32 sideEndSlot;

    ringCount = 0;
    cameraAnglesBetween(&angles, fromPoint, outPosition, scale);
    if (ringFlags & 4) {
        TextCommandStateBlend *blendState = &gTextCommandStateAlt;
        tiRingEmitMode2(&SEF_SCRATCHPAD(void, 0x0), &ringCount, blendState->mirrorSecondMode);
        tiRingEmitMode2(&SEF_SCRATCHPAD(void, 0x0), &ringCount, blendState->unk_04[0]);
    }
    if (ringFlags & 8) {
        TextCommandStateBlend *blendState;
        if (actorSlotGetVector(&SEF_SCRATCHPAD(u8, 0x0) + ringCount * 16, gTextCommandStateAlt.mirrorSecondMode, 0)) {
            ringCount++;
        }
        blendState = &gTextCommandStateAlt;
        if (actorSlotGetVector(&SEF_SCRATCHPAD(u8, 0x0) + ringCount * 16, blendState->mirrorSecondMode, 2)) {
            ringCount++;
        }
        if (actorSlotGetVector(&SEF_SCRATCHPAD(u8, 0x0) + ringCount * 16, blendState->unk_04[0], 0)) {
            ringCount++;
        }
        if (actorSlotGetVector(&SEF_SCRATCHPAD(u8, 0x0) + ringCount * 16, blendState->unk_04[0], 2)) {
            ringCount++;
        }
    }
    if (ringFlags & 0x10) {
        for (i = 0; i < 8; i++) {
            tiRingEmitMode2(&SEF_SCRATCHPAD(void, 0x0), &ringCount, i);
        }
    }
    if (ringFlags & 0x20) {
        i = 0;
        do {
            if (actorSlotGetVector(&SEF_SCRATCHPAD(u8, 0x0) + ringCount * 16, i, 0)) {
                ringCount++;
            }
            k = i;
            i++;
            if (actorSlotGetVector(&SEF_SCRATCHPAD(u8, 0x0) + ringCount * 16, k, 2)) {
                ringCount++;
            }
        } while (i < 8);
    }
    if (ringFlags & 0x6000) {
        if (gBattleSideSplit[0] < 3) {
            sideFirstSlot = 0;
            sideEndSlot = 3;
        } else {
            sideFirstSlot = 3;
            sideEndSlot = 8;
        }
        if (ringFlags & 0x2000) {
            for (i = sideFirstSlot; i < sideEndSlot; i++) {
                tiRingEmitMode2(&SEF_SCRATCHPAD(void, 0x0), &ringCount, i);
            }
        }
        if (ringFlags & 0x4000) {
            i = sideFirstSlot;
            if (sideFirstSlot < sideEndSlot) {
                do {
                    if (actorSlotGetVector(&SEF_SCRATCHPAD(u8, 0x0) + ringCount * 16, i, 0)) {
                        ringCount++;
                    }
                    k = i;
                    i++;
                    if (actorSlotGetVector(&SEF_SCRATCHPAD(u8, 0x0) + ringCount * 16, k, 2)) {
                        ringCount++;
                    }
                } while (i < sideEndSlot);
            }
        }
    }
    func_00266B40(outPosition, &angles, &SEF_SCRATCHPAD(void, 0x0), ringCount, 0, scale);
}

f32 xglPointLength(Vector4 *a, Vector4 *b);

/* dest is source with y = point.y + sqrt(pointDistance^2 - planarDistance^2): the 3D distance from point to
 * source moved by battleCameraAnglesEmit (blendFactor), and the planar distance from point to source. */
void battleSegmentHeightPoint(Vector4 *outPoint, Vector4 *sourcePoint, Vector4 *point, f32 blendFactor) {
    Vector4 movedSource;
    f32 deltaZ;
    f32 planarDistance;
    f32 pointDistance;
    f32 heightOffset;

    /* planarDistance first holds the x delta, then the planar distance */
    planarDistance = point->x - sourcePoint->x;
    deltaZ = point->z - sourcePoint->z;
    planarDistance = __builtin_sqrtf(planarDistance * planarDistance + deltaZ * deltaZ);
    movedSource = *sourcePoint;
    battleCameraAnglesEmit(&movedSource, point, 4, blendFactor);
    pointDistance = xglPointLength(point, &movedSource);
    heightOffset = __builtin_sqrtf(pointDistance * pointDistance - planarDistance * planarDistance);
    *outPoint = *sourcePoint;
    outPoint->y = point->y + heightOffset;
}

typedef struct TextCommandStateInit {
    u8 commandFlags; /* 0x00: bit 0x2 set by textCommandStateSetKinds */
    u8 unk_01;
    u8 mirrorFlag; /* 0x02: nonzero negates the first value (with mirrorSuppressFlags bit 0x100 clear) */
    u8 mirrorSecondMode; /* 0x03: >= 3 negates the second value */
    u8 secondActorSlot;  /* 0x04: battle block +2 (swapped with mirrorSecondMode by flags 0x8000) */
    u8 unk_05[0x7B];
    s32 mirrorSuppressFlags; /* 0x80: bit 0x100 */
    u8 unk_84[0x10];
    f32 keptScaledValue; /* 0x94: written by textCommandStateSetScaledValue unless commandFlags bit 0x2 is set */
    f32 lastScaledValue; /* 0x98: written by textCommandStateSetScaledValue */
    u8 unk_9C[0x44];
    f32 scale;            /* 0xE0 */
    s8 primaryTextKind;   /* 0xE4: set by textCommandStateSetKinds */
    s8 secondaryTextKind; /* 0xE5: set by textCommandStateSetKinds */
    u8 unk_E6[2];
    void *commandCallback;  /* 0xE8: func_00265398 */
    s32 maskedFlags; /* 0xEC: mirrorSuppressFlags & 0x603C, written by battleTextCommandInit */
} TextCommandStateInit;

extern unsigned char func_00265478[];
extern Vector4 *gCameraSourcePointers[];
extern Vector4 *D_003E3354[];
void actorSlotGetPlacement(Vector4 *dst, s32 slot);
void cameraAnglesWrapPi(Vector4 *angles);
void cameraTablesClear(void);
void func_00265528(s32 a, s32 b, f32 c);
unsigned short xglSRand(void);
float sinf(float);

#define gTextCommandStateInit (*(TextCommandStateInit *) gTextCommandStateStorage)

/* Starts a battle text command: clears the state block, stores the initial flags and mirror slots from
 * gBattleModeActors, sets the mirror and scale flags and resets the camera tables. Called from tu238. */
void battleTextCommandInit(s32 mirrorSuppressFlags) {
    Vector4 firstActorPosition;
    Vector4 secondActorPosition;
    Vector4 actorAngles;
    Vector4 cameraAngles;
    s32 suppressFlags;

    memset(gTextCommandStateStorage, 0, 0xF0);
    gTextCommandStateInit.unk_01 = 1;
    gTextCommandStateInit.commandFlags = 1;
    gTextCommandStateInit.scale = 1.0f;
    gTextCommandStateInit.mirrorSuppressFlags = mirrorSuppressFlags;
    gTextCommandStateInit.mirrorSecondMode = gBattleModeView.firstActorSlot;
    gTextCommandStateInit.secondActorSlot = gBattleModeView.secondActorSlot;
    if ((mirrorSuppressFlags & 0x8000) && gBattleModeView.firstActorSlot >= 3) {
        gTextCommandStateInit.mirrorSecondMode = gBattleModeView.secondActorSlot;
        gTextCommandStateInit.secondActorSlot = gBattleModeView.firstActorSlot;
    }

    if (gBattleModeView.battleMode == 0 &&
        (gTextCommandStateInit.mirrorSuppressFlags & 0x20000)) {
        if (gTextCommandStateInit.mirrorSecondMode < 3) {
            actorSlotGetPlacement(&firstActorPosition, gTextCommandStateInit.mirrorSecondMode);
            actorSlotGetPlacement(&secondActorPosition, gTextCommandStateInit.secondActorSlot);
        } else {
            actorSlotGetPlacement(&firstActorPosition, gTextCommandStateInit.secondActorSlot);
            actorSlotGetPlacement(&secondActorPosition, gTextCommandStateInit.mirrorSecondMode);
        }
        ((void (*)(Vector4 *out, Vector4 *from, Vector4 *to)) cameraAnglesBetween)(
            &actorAngles, &firstActorPosition, &secondActorPosition);
        ((void (*)(Vector4 *out, Vector4 *from, Vector4 *to)) cameraAnglesBetween)(
            &cameraAngles, gCameraSourcePointers[0], &secondActorPosition);
        cameraAngles.y = cameraAngles.y - actorAngles.y;
        cameraAnglesWrapPi(&cameraAngles);
        if (cameraAngles.y < 0.0f) {
            gTextCommandStateInit.mirrorFlag = 1;
        }
    } else {
        if (sinf(D_003E3354[0]->y) < 0.0f) {
            gTextCommandStateInit.mirrorFlag = 1;
        }
    }
    if ((gTextCommandStateInit.mirrorSuppressFlags & 0x10000) && (xglSRand() & 1)) {
        gTextCommandStateInit.scale = -1.0f;
    }
    suppressFlags = gTextCommandStateInit.mirrorSuppressFlags;
    if (suppressFlags & 0x800) {
        gTextCommandStateInit.commandFlags |= 4;
    }
    if (suppressFlags & 0x200) {
        gTextCommandStateInit.commandCallback = func_00265478;
    } else {
        gTextCommandStateInit.commandCallback = func_00265398;
    }
    gTextCommandStateInit.maskedFlags = gTextCommandStateInit.mirrorSuppressFlags & 0x603C;
    cameraTablesClear();
    func_00265528(0, 0, 0.6981312f);
}


void stateMirrorScalePair(f32 *xPosition, f32 *yPosition);

/* Stores a point and its blend factor in slot 1 of the alt state, mirrored by stateMirrorScalePair; slot 0
 * gets the same values unless commandFlags bit 0x2 is set. Called by tu238. */
void battleTextCommandSetFirstPoint(f32 blendFactor, f32 pointX, f32 pointY, f32 pointZ) {
    stateMirrorScalePair(&pointX, &pointZ);
    gTextCommandStateAlt.vectors[1].x = pointX;
    gTextCommandStateAlt.vectors[1].y = pointY;
    gTextCommandStateAlt.vectors[1].z = pointZ;
    gTextCommandStateAlt.blendFactor[1] = blendFactor;
    if (!(gTextCommandStateAlt.commandFlags & 2)) {
        gTextCommandStateAlt.blendFactor[0] = blendFactor;
        gTextCommandStateAlt.vectors[0] = gTextCommandStateAlt.vectors[1];
    }
}


/* Same as battleTextCommandSetFirstPoint for slot 3, with slot 2 as the fallback copy. Called by tu238. */
void battleTextCommandSetSecondPoint(f32 blendFactor, f32 pointX, f32 pointY, f32 pointZ) {
    stateMirrorScalePair(&pointX, &pointZ);
    gTextCommandStateAlt.vectors[3].x = pointX;
    gTextCommandStateAlt.vectors[3].y = pointY;
    gTextCommandStateAlt.vectors[3].z = pointZ;
    gTextCommandStateAlt.blendFactor[3] = blendFactor;
    if (!(gTextCommandStateAlt.commandFlags & 2)) {
        gTextCommandStateAlt.blendFactor[2] = blendFactor;
        gTextCommandStateAlt.vectors[2] = gTextCommandStateAlt.vectors[3];
    }
}

void stateMirrorScalePair(f32 *xPosition, f32 *yPosition)
{
  f32 negatedSecond;
  f32 negatedFirst;
  f32 scaledFirst;

  if (gTextCommandState.mirrorSecondMode >= 3U)
  {
    negatedSecond = -(*yPosition);
    *yPosition = negatedSecond;
  }
  if ((!(gTextCommandState.mirrorSuppressFlags & 0x100)) && (gTextCommandState.mirrorFlag != 0))
  {
    negatedFirst = -(*xPosition);
    *xPosition = negatedFirst;
  }
  scaledFirst = (*xPosition) * gTextCommandState.scale;
  *xPosition = scaledFirst;
}

void textCommandStateSetScaledValue(f32 unscaledValue)
{
  f32 scaledValue;
  f32 mirroredValue;

  mirroredValue = unscaledValue;
  if ((!(gTextCommandState.mirrorSuppressFlags & 0x100)) && (gTextCommandState.mirrorFlag != 0))
  {
    mirroredValue = -mirroredValue;
  }
  scaledValue = mirroredValue * gTextCommandState.scale;
  gTextCommandState.lastScaledValue = scaledValue;
  if (!(gTextCommandState.commandFlags & 2))
  {
    gTextCommandState.keptScaledValue = scaledValue;
  }
}

void textCommandStateSetKinds(s32 primaryTextKind, s32 secondaryTextKind)
{
  gTextCommandState.secondaryTextKind = secondaryTextKind;
  gTextCommandState.primaryTextKind = primaryTextKind;
  gTextCommandState.commandFlags |= 2;
}


void battleActorGroupCentre(Vector4 *center);
void xglMatrixUnit(Matrix4 destination);
void xglMatrixRotX(Matrix4 destination, Matrix4 source, f32 angle);
void xglMatrixRotY(Matrix4 destination, Matrix4 source, f32 angle);

/* Recomputes the blend end points of the alt state from the two actor slots and, when mirrorSuppressFlags
 * bit 0x1000 is set and the slots differ, the rotation matrix (useMatrixFlag). Called by battleTextCommandUpdate. */
void battleTextCommandUpdateEndpoints(void) {
    Vector4 angles;

    actorSlotGetVector(&gTextCommandStateAlt.startPoint, gTextCommandStateAlt.mirrorSecondMode, 0);
    if (!(gTextCommandStateAlt.mirrorSuppressFlags & 0x400)) {
        actorSlotGetVector(&gTextCommandStateAlt.endPoint, gTextCommandStateAlt.unk_04[0], 0);
    } else {
        battleActorGroupCentre(&gTextCommandStateAlt.endPoint);
        if (gTextCommandStateAlt.mirrorSecondMode < 3 ? gTextCommandStateAlt.unk_04[0] < 3
                                                          : gTextCommandStateAlt.unk_04[0] >= 3) {
            gTextCommandStateAlt.startPoint = gTextCommandStateAlt.endPoint;
        }
    }
    if (gTextCommandStateAlt.mirrorSuppressFlags & 0x80) {
        gTextCommandStateAlt.endPoint.y = gTextCommandStateAlt.startPoint.y;
    }
    xglMatrixUnit(gTextCommandStateAlt.matrix);
    if ((gTextCommandStateAlt.mirrorSuppressFlags & 0x1000) &&
        gTextCommandStateAlt.mirrorSecondMode != gTextCommandStateAlt.unk_04[0]) {
        gTextCommandStateAlt.useMatrixFlag = 1;
        ((void (*)(Vector4 *, s32, void *)) cameraAnglesBetween)(
            &angles, &gTextCommandStateAlt.endPoint, &gTextCommandStateAlt.startPoint);
        angles.z = 0;
        angles.w = 1.0f;
        xglMatrixRotX(gTextCommandStateAlt.matrix, gTextCommandStateAlt.matrix, angles.x);
        xglMatrixRotY(gTextCommandStateAlt.matrix, gTextCommandStateAlt.matrix, angles.y);
    } else {
        gTextCommandStateAlt.useMatrixFlag = 0;
    }
}

/* Averages the positions of the battle slots of one side (slots 0..2 or 3..7, by gBattleSideSplit) into center. */
void battleActorGroupCentre(Vector4 *center) {
    AlignedVector4 slotPosition;
    s32 actorSlot;
    s32 actorSlotEnd;
    s32 foundCount = 0;

    memset(center, 0, sizeof(Vector4));
    if (gBattleSideSplit[0] < 3) {
        actorSlot = 0;
        actorSlotEnd = 3;
    } else {
        actorSlot = 3;
        actorSlotEnd = 8;
    }
    if (actorSlotEnd - actorSlot > 0) {
        for (; actorSlot < actorSlotEnd; actorSlot++) {
            if (actorSlotGetVector(&slotPosition, actorSlot, 0) != 0) {
                Vu0AddVector(center, center, &slotPosition);
                foundCount++;
            }
        }
        if (foundCount != 0) {
            xglVectorScale(center, center, 1.0f / foundCount);
        }
    }
}

DoublewordAlignedVector4 gBattleScratchPoint = { 0 };
Vector4 *gCameraSourcePointers[] = { 0 };
Vector4 *D_003E3354[] = { 0 };
Vector4 *D_003E3358[] = { 0, 0 };
unsigned char gTextCommandStateStorage[240];

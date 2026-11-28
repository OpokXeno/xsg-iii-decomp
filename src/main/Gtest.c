#include "common.h"

#include "shared.h"
#include "quadword.h"
#include "main/tu141.h"
#include "main/tu007.h"
#include "main/xenoActor.h"
#include "main/gifState.h"
#include "main/xgl_2.h"
#include "main/assert_fail.h"
typedef signed char s8;

typedef int s32;

s32 XenoActorEnableChannel();
s32 XenoActorSetMode();

/* The actor record XenoActorEnableAndSetMode reads: the primary actor handle at +0x50, the secondary one at +0x54. */
typedef struct ActorPairRecord {
    unsigned char unmodeled_00[0x50];
    s32 primary;                        /* +0x50 */
    s32 secondary;                      /* +0x54 */
} ActorPairRecord;

/* Enables the primary actor's channel and sets both actors of the pair to one mode; returns the secondary actor's
 * XenoActorSetMode result, or 0 without one. kept: defined here, it is nothrow for its callers (func_00245C70's annulled
 * branch needs that). */
s32 XenoActorEnableAndSetMode(ActorPairRecord *actorPair, s32 actorMode) {
    s32 secondary;

    XenoActorEnableChannel(actorPair->primary);
    XenoActorSetMode(actorPair->primary, actorMode);
    secondary = actorPair->secondary;
    if (secondary != 0) {
        secondary = XenoActorSetMode(secondary, actorMode);
    }
    return secondary;
}

float tiWaterVectorAngle(float x, float z);

/* Turns the actor toward aimTarget when the target is within 15 units on both axes and within 15 horizontally.
 * Reads the heading word 5 and writes the aim angle to word 9; words 12, 13, 14 and 22 are cleared to zero. */
void XenoActorAimTowardTarget(float *actorRecord, float *aimTarget) {
    float deltaX;
    float deltaZ;
    float distance;
    float heading;
    float aim;
    double gap;

    deltaX = aimTarget[8] - actorRecord[0];
    deltaZ = aimTarget[10] - actorRecord[2];

    gap = deltaX;
    if (gap < 0.0) {
        gap = -gap;
    }
    if (gap >= 15.0) {
        return;
    }

    gap = deltaZ;
    if (gap < 0.0) {
        gap = -gap;
    }
    if (gap >= 15.0) {
        return;
    }

    distance = __builtin_sqrtf(deltaX * deltaX + deltaZ * deltaZ);
    if (!(distance < 15.0f)) {
        return;
    }

    heading = actorRecord[5];
    aim = 1.5707964f - tiWaterVectorAngle(deltaX, deltaZ);

    while (heading < 0.0f) {
        heading += 6.2831855f;
    }
    if (aim < 0.0f) {
        aim += 6.2831855f;
        while (aim < 0.0f) {
            aim += 6.2831855f;
        }
    }
    while (heading >= 6.2831855f) {
        heading -= 6.2831855f;
    }
    if (aim >= 6.2831855f) {
        aim -= 6.2831855f;
        while (aim >= 6.2831855f) {
            aim -= 6.2831855f;
        }
    }

    gap = heading - aim;
    if (gap < 0.0) {
        gap = -gap;
    }
    /* kept: the empty compare decides the schedule of the gap computation (removing it is NOTSTRICT). */
    if (gap > 3.1415927410125732) {
    }
    actorRecord[9] = aim;
    ((union {
        s32 i;
        float f;
    } *) actorRecord)[22]
        .i = 0;
    ((union {
        s32 i;
        float f;
    } *) actorRecord)[12]
        .i = 0;
    ((union {
        s32 i;
        float f;
    } *) actorRecord)[13]
        .i = 0;
    ((union {
        s32 i;
        float f;
    } *) actorRecord)[14]
        .i = 0;
}

/* Same as XenoActorAimTowardTarget for the actor whose heading is word 13: the aim angle is written to word 17. */
void func_002459C0(float *actorRecord, float *aimTarget) {
    float deltaX;
    float deltaZ;
    float distance;
    float heading;
    float aim;
    double gap;

    deltaX = aimTarget[0] - actorRecord[8];
    deltaZ = aimTarget[2] - actorRecord[10];

    gap = deltaX;
    if (gap < 0.0) {
        gap = -gap;
    }
    if (gap >= 15.0) {
        return;
    }

    gap = deltaZ;
    if (gap < 0.0) {
        gap = -gap;
    }
    if (gap >= 15.0) {
        return;
    }

    distance = __builtin_sqrtf(deltaX * deltaX + deltaZ * deltaZ);
    if (!(distance < 15.0f)) {
        return;
    }

    heading = actorRecord[13];
    aim = 1.5707964f - tiWaterVectorAngle(deltaX, deltaZ);
    while (heading < 0.0f) {
        heading += 6.2831855f;
    }
    if (aim < 0.0f) {
        aim += 6.2831855f;
        while (aim < 0.0f) {
            aim += 6.2831855f;
        }
    }
    while (heading >= 6.2831855f) {
        heading -= 6.2831855f;
    }
    if (aim >= 6.2831855f) {
        aim -= 6.2831855f;
        while (aim >= 6.2831855f) {
            aim -= 6.2831855f;
        }
    }

    gap = heading - aim;
    if (gap < 0.0) {
        gap = -gap;
    }
    /* kept: the empty compare decides the schedule of the gap computation (removing it is NOTSTRICT). */
    if (gap > 3.1415927410125732) {
    }
    actorRecord[17] = aim;
}

INCLUDE_ASM("asm/main/nonmatchings/Gtest", func_00245C70);

s32 xglCdServiceIfPending(void);

/* Reads the file NAME into the buffer at *bufferCursor, waits until the CD service is idle, advances the cursor by
 * the bytes read and returns the address the file was read to (0 when the read failed). */
s32 xglCdReadFileAtCursor(s32 *bufferCursor, const char *name) {
    s32 loadedBytes;
    s32 loadedTo;

    loadedBytes = xglCdReadFile(name, (void *) *bufferCursor, 0, 0);
    if (loadedBytes <= 0) {
        return 0;
    }
    while (xglCdServiceIfPending() != 0) {
    }
    loadedTo = *bufferCursor;
    *bufferCursor = loadedTo + loadedBytes;
    return loadedTo;
}

void cfeDirectionFromAngles(float *direction, const float *angles);
void pxyModelRenderStateLoadLight(const void *lightSet);

/* Sets the default light set (colours, ambient, six direction angles, no point lights) and loads it into the renderer. */
void gtestLightSetDefault(void) {
    XglLightSet lightSet;
    XglLightSet *lightView;
    Vector4 *directionVector;
    s32 i;

    lightSet.colors[0][0] = 0.22f;
    lightSet.colors[0][1] = 0.2f;
    lightSet.colors[1][0] = 1.0f;
    lightSet.colors[0][2] = 0.2f;
    lightSet.colors[1][1] = 0.8f;
    lightSet.colors[1][2] = 0.8f;
    lightSet.colors[2][0] = 0.11f;
    lightSet.colors[2][1] = 0.11f;
    lightSet.colors[2][2] = 0.11f;
    lightSet.ambientColor.y = 0.6f;
    lightSet.ambientColor.x = 0.65999996f;
    lightSet.ambientColor.z = 0.6f;
    lightSet.viewDirection.x = 0.0f;
    lightSet.directions[0][0] = -5.36f;
    lightSet.directions[0][1] = 0.77f;
    lightSet.directions[0][3] = 1.0f;
    lightSet.directions[1][0] = 2.1f;
    lightSet.directions[1][1] = 1.83f;
    lightSet.directions[1][3] = 1.0f;
    lightSet.directions[2][0] = 0.349f;
    lightSet.directions[2][1] = 3.7861f;
    lightSet.directions[2][3] = 1.0f;
    lightSet.ambientColorLower.z = 0.25f;
    lightSet.ambientColorLower.y = 0.25f;
    lightSet.ambientColorLower.x = 0.25f;
    lightSet.directions[3][0] = lightSet.directions[4][0] = lightSet.directions[5][0] = 1.5707964f;
    lightView = &lightSet;
    directionVector = (Vector4 *) lightSet.directions;
    i = 6;
    for (; i >= 0; i--, directionVector++) {
        cfeDirectionFromAngles((float *) directionVector, (float *) directionVector);
    }
    lightView->pointPositions[0][0] = lightView->pointPositions[0][1] = lightView->pointPositions[0][2] =
        lightView->pointPositions[0][3] = 0.0f;
    lightView->pointPositions[1][0] = lightView->pointPositions[1][1] = lightView->pointPositions[1][2] =
        lightView->pointPositions[1][3] = 0.0f;
    lightView->pointPositions[2][0] = lightView->pointPositions[2][1] = lightView->pointPositions[2][2] =
        lightView->pointPositions[2][3] = 0.0f;
    lightView->pointColors[0][0] = lightView->pointColors[0][1] = lightView->pointColors[0][2] = lightView->pointColors[0][3] = 0.0f;
    lightView->pointColors[1][0] = lightView->pointColors[1][1] = lightView->pointColors[1][2] = lightView->pointColors[1][3] = 0.0f;
    lightView->pointColors[2][0] = lightView->pointColors[2][1] = lightView->pointColors[2][2] = lightView->pointColors[2][3] = 0.0f;
    pxyModelRenderStateLoadLight(lightView);
}

/* Same as gtestLightSetDefault with the light set of the second scene (grey colours, lower ambient). */
void gtestLightSetGrey(void) {
    XglLightSet lightSet;
    XglLightSet *lightView;
    Vector4 *directionVector;
    s32 i;

    lightSet.colors[0][0] = 0.2f;
    lightSet.colors[0][1] = 0.2f;
    lightSet.colors[0][2] = 0.2f;
    lightSet.colors[1][0] = 1.0f;
    lightSet.colors[1][1] = 1.0f;
    lightSet.colors[1][2] = 1.0f;
    lightSet.colors[2][0] = 0.1f;
    lightSet.colors[2][1] = 0.1f;
    lightSet.colors[2][2] = 0.1f;
    lightSet.ambientColor.x = 0.2f;
    lightSet.ambientColor.y = 0.2f;
    lightSet.ambientColor.z = 0.2f;
    lightSet.viewDirection.x = 0.0f;
    lightSet.directions[0][0] = -5.36f;
    lightSet.directions[0][1] = 0.77f;
    lightSet.directions[0][3] = 1.0f;
    lightSet.directions[1][0] = 2.1f;
    lightSet.directions[1][1] = 1.83f;
    lightSet.directions[1][3] = 1.0f;
    lightSet.directions[2][0] = 0.349f;
    lightSet.directions[2][1] = 3.7861f;
    lightSet.directions[2][3] = 1.0f;
    lightSet.ambientColorLower.z = 0.25f;
    lightSet.ambientColorLower.y = 0.25f;
    lightSet.directions[5][0] = 1.5707964f;
    lightSet.directions[4][0] = 1.5707964f;
    lightSet.ambientColorLower.x = 0.25f;
    lightSet.directions[3][0] = 1.5707964f;
    lightView = &lightSet;
    directionVector = (Vector4 *) lightSet.directions;
    i = 6;
    for (; i >= 0; i--, directionVector++) {
        cfeDirectionFromAngles((float *) directionVector, (float *) directionVector);
    }
    lightView->pointPositions[0][0] = lightView->pointPositions[0][1] = lightView->pointPositions[0][2] =
        lightView->pointPositions[0][3] = 0.0f;
    lightView->pointPositions[1][0] = lightView->pointPositions[1][1] = lightView->pointPositions[1][2] =
        lightView->pointPositions[1][3] = 0.0f;
    lightView->pointPositions[2][0] = lightView->pointPositions[2][1] = lightView->pointPositions[2][2] =
        lightView->pointPositions[2][3] = 0.0f;
    lightView->pointColors[0][0] = lightView->pointColors[0][1] = lightView->pointColors[0][2] = lightView->pointColors[0][3] = 0.0f;
    lightView->pointColors[1][0] = lightView->pointColors[1][1] = lightView->pointColors[1][2] = lightView->pointColors[1][3] = 0.0f;
    lightView->pointColors[2][0] = lightView->pointColors[2][1] = lightView->pointColors[2][2] = lightView->pointColors[2][3] = 0.0f;
    pxyModelRenderStateLoadLight(lightView);
}

void XenoActorEnableAndSetModeGroup(s32 *actorGroup, s32 actorMode) {
    s32 *pSecondaryEntry;
    s32 secondaryActor;
    s32 i;

    pSecondaryEntry = actorGroup + 1;
    XenoActorEnableChannel(actorGroup[0]);
    XenoActorSetMode(actorGroup[0], actorMode);
    for (i = 0; i < 4; i++) {
        secondaryActor = *pSecondaryEntry;
        pSecondaryEntry += 1;
        if (secondaryActor != 0) {
            XenoActorSetMode(secondaryActor, actorMode);
        }
    }
}

typedef struct GtestRecord {
    s32 words[6]; /* 0x00-0x17 */
    unsigned char unmodeled_18[0x08];
    Vector4 destination; /* 0x20 */
    Vector4 rotation; /* 0x30 */
    Vector4 scale; /* 0x40 */
    Vector4 vector50; /* 0x50 */
    Vector4 vector60; /* 0x60 */
    float scalar70;     /* 0x70 */
    unsigned char unmodeled_74[0x0C];
    Vector4 vector80; /* 0x80 */
    Vector4 vector90; /* 0x90 */
    unsigned char unmodeledA0[0x10];
    float scalarB0;  /* 0xB0 */
    float scalarB4;  /* 0xB4 */
    s32 tail; /* 0xB8 */
} GtestRecord;

/* Clears the record for the first test actor: zero vectors, w = 1, scalars 6.0, 1.7 and 0.0015, words cleared. */
void GtestActorBlockInitFirst(GtestRecord *actorBlock) {
    f32 zeroComponent = 0.0f;
    f32 oneComponent = 1.0f;
    f32 initScalarFirst = 6.0f;
    f32 initScalarSecond = 1.7f;
    f32 initScalarThird = 0.0015f;
    s32 *wordCursor = &actorBlock->words[4];
    s32 i = 3;

    actorBlock->words[5] = 0;
    actorBlock->destination.x = zeroComponent;
    actorBlock->vector60.x = zeroComponent;
    actorBlock->destination.y = zeroComponent;
    actorBlock->vector60.y = zeroComponent;
    actorBlock->destination.z = zeroComponent;
    actorBlock->vector60.z = zeroComponent;
    actorBlock->destination.w = oneComponent;
    actorBlock->vector60.w = oneComponent;
    actorBlock->rotation.x = zeroComponent;
    actorBlock->scale.x = zeroComponent;
    actorBlock->rotation.y = zeroComponent;
    actorBlock->scale.y = zeroComponent;
    actorBlock->rotation.z = zeroComponent;
    actorBlock->scale.z = zeroComponent;
    actorBlock->rotation.w = oneComponent;
    actorBlock->scale.w = oneComponent;
    actorBlock->vector50.x = zeroComponent;
    actorBlock->vector50.y = zeroComponent;
    actorBlock->vector50.z = zeroComponent;
    actorBlock->vector50.w = oneComponent;
    actorBlock->vector80.x = zeroComponent;
    actorBlock->vector80.y = zeroComponent;
    actorBlock->vector80.z = zeroComponent;
    actorBlock->vector80.w = oneComponent;
    actorBlock->vector90.x = zeroComponent;
    actorBlock->vector90.y = zeroComponent;
    actorBlock->vector90.z = zeroComponent;
    actorBlock->vector90.w = oneComponent;
    actorBlock->scalarB0 = initScalarFirst;
    actorBlock->scalarB4 = initScalarSecond;
    actorBlock->scalar70 = initScalarThird;
    actorBlock->tail = 0;
    actorBlock->words[0] = 0;
    for (; i >= 0; i--) {
        *wordCursor = 0;
        wordCursor -= 1;
    }
}

/* Same as GtestActorBlockInitFirst with scalars 4.0, 2.0 and 0.0015 (the second test actor). */
void GtestActorBlockInitSecond(GtestRecord *actorBlock) {
    f32 zeroComponent = 0.0f;
    f32 oneComponent = 1.0f;
    f32 initScalarFirst = 4.0f;
    f32 initScalarSecond = 2.0f;
    f32 initScalarThird = 0.0015f;
    s32 *wordCursor = &actorBlock->words[4];
    s32 i = 3;

    actorBlock->words[5] = 0;
    actorBlock->destination.x = zeroComponent;
    actorBlock->vector60.x = zeroComponent;
    actorBlock->destination.y = zeroComponent;
    actorBlock->vector60.y = zeroComponent;
    actorBlock->destination.z = zeroComponent;
    actorBlock->vector60.z = zeroComponent;
    actorBlock->destination.w = oneComponent;
    actorBlock->vector60.w = oneComponent;
    actorBlock->rotation.x = zeroComponent;
    actorBlock->scale.x = zeroComponent;
    actorBlock->rotation.y = zeroComponent;
    actorBlock->scale.y = zeroComponent;
    actorBlock->rotation.z = zeroComponent;
    actorBlock->scale.z = zeroComponent;
    actorBlock->rotation.w = oneComponent;
    actorBlock->scale.w = oneComponent;
    actorBlock->vector50.x = zeroComponent;
    actorBlock->vector50.y = zeroComponent;
    actorBlock->vector50.z = zeroComponent;
    actorBlock->vector50.w = oneComponent;
    actorBlock->vector80.x = zeroComponent;
    actorBlock->vector80.y = zeroComponent;
    actorBlock->vector80.z = zeroComponent;
    actorBlock->vector80.w = oneComponent;
    actorBlock->vector90.x = zeroComponent;
    actorBlock->vector90.y = zeroComponent;
    actorBlock->vector90.z = zeroComponent;
    actorBlock->vector90.w = oneComponent;
    actorBlock->scalarB0 = initScalarFirst;
    actorBlock->scalarB4 = initScalarSecond;
    actorBlock->scalar70 = initScalarThird;
    actorBlock->tail = 0;
    actorBlock->words[0] = 0;
    for (; i >= 0; i--) {
        *wordCursor = 0;
        wordCursor -= 1;
    }
}

typedef struct GtestTransform {
    s32 kind;       /* +0x00 */
    s32 cleared[4]; /* +0x04: cleared by a loop */
    s32 mode;       /* +0x14 */
    unsigned char unmodeled_18[8];
    Vector4 destination; /* +0x20 */
    Vector4 rotation;    /* +0x30: radians, w = 1 */
    Vector4 scale;       /* +0x40 */
    Vector4 vector50;    /* +0x50 */
    Vector4 vector60;    /* +0x60 */
    float scalar70;      /* +0x70 */
    unsigned char unmodeled_74[0xC];
    Vector4 vector80; /* +0x80 */
    Vector4 vector90; /* +0x90 */
    unsigned char unmodeled_A0[0x10];
    float scalarB0; /* +0xB0 */
    float scalarB4; /* +0xB4 */
    s32 tail;       /* +0xB8 */
    unsigned char unmodeled_BC[4];
} GtestTransform;

/* Initialises the transform of the test actor: default destination, rotation and scale, the mode and a cleared word table.
 * kept: the empty do/while(0) blocks between the stores are the scheduling barriers the match needs (removing them is NOTSTRICT). */
void GtestTransformInit(GtestTransform *transform) {
    s32 transformMode;
    float zeroComponent;
    float oneComponent;
    float defaultX;
    float defaultY;
    float defaultZ;
    float defaultScalarFirst;
    float defaultScalarSecond;
    float defaultScalarThird;
    s32 i;
    s32 *clearCursor;

    transformMode = 1;
    clearCursor = &transform->cleared[3];
    i = 3;
    zeroComponent = 0.0f;
    oneComponent = 1.0f;
    defaultX = 14.0f;
    defaultY = 27.0f;
    defaultZ = -39.0f;
    defaultScalarFirst = 15.0f;
    defaultScalarSecond = 7.0f;
    do {
    } while (0);
    do {
    } while (0);
    defaultScalarThird = 0.002f;
    transform->mode = transformMode;
    do {
    } while (0);
    transform->vector60.x = defaultX;
    transform->vector60.y = defaultY;
    transform->vector60.z = defaultZ;
    do {
    } while (0);
    transform->vector90.z = zeroComponent;
    transform->vector90.w = oneComponent;
    do {
    } while (0);
    transform->scalarB0 = defaultScalarFirst;
    transform->scalarB4 = defaultScalarSecond;
    do {
    } while (0);
    transform->destination.x = defaultX;
    transform->destination.y = defaultY;
    transform->destination.z = defaultZ;
    transform->destination.w = oneComponent;
    do {
    } while (0);
    transform->vector60.w = oneComponent;
    do {
    } while (0);
    transform->rotation.x = zeroComponent;
    transform->scale.x = zeroComponent;
    transform->rotation.y = zeroComponent;
    transform->scale.y = zeroComponent;
    transform->rotation.z = zeroComponent;
    transform->scale.z = zeroComponent;
    transform->rotation.w = oneComponent;
    transform->scale.w = oneComponent;
    do {
    } while (0);
    transform->vector50.x = zeroComponent;
    transform->vector50.y = zeroComponent;
    transform->vector50.z = zeroComponent;
    transform->vector50.w = oneComponent;
    do {
    } while (0);
    transform->vector80.x = zeroComponent;
    transform->vector80.y = zeroComponent;
    do {
    } while (0);
    transform->vector80.z = zeroComponent;
    transform->vector80.w = oneComponent;
    do {
    } while (0);
    transform->vector90.x = zeroComponent;
    transform->vector90.y = zeroComponent;
    do {
    } while (0);
    transform->tail = 0;
    transform->kind = 0;
    do {
    } while (0);
    transform->scalar70 = defaultScalarThird;
    do {
    } while (0);
    for (; i >= 0; i--) {
        *clearCursor = 0;
        clearCursor--;
    }
}

typedef struct AngleState {
    unsigned char unmodeled_00[4];
    float angle; /* +0x04 */
} AngleState;

/* Blends angleState->angle toward targetAngle by fraction along the shorter way (angles in radians, wrapped at 2 pi). */
void AngleStateBlendShortestWay(AngleState *angleState, const AngleState *targetAngle, float fraction) {
    float fromAngle;
    float toAngle;
    float oneMinusFraction;
    float wrappedAngle;
    float nextAngle;

    oneMinusFraction = 1.0f - fraction;
    fromAngle = angleState->angle;
    wrappedAngle = fromAngle;
    if (6.2831855f < fromAngle) {
        do {
            nextAngle = wrappedAngle - 6.2831855f;
            wrappedAngle = nextAngle;
        } while (6.2831855f < nextAngle);
        angleState->angle = nextAngle;
        fromAngle = nextAngle;
    }
    toAngle = targetAngle->angle;
    if (__builtin_fabsf(fromAngle - toAngle) < 3.1415927f) {
        angleState->angle = fromAngle * oneMinusFraction + toAngle * fraction;
    } else if (toAngle < fromAngle) {
        angleState->angle = fromAngle * oneMinusFraction + (toAngle + 6.2831855f) * fraction;
    } else {
        angleState->angle = (fromAngle + 6.2831855f) * oneMinusFraction + toAngle * fraction;
    }
}

extern PadRecord PadData[2];
float xglSin(float angle);
float xglCos(float angle);
float func_002AA490(Vector4 *destination, Vector4 *stick, Vector4 *limit);
s32 XenoActorIsDone(s32 actor);

typedef struct GtestTransformView {
    s32 kind;       /* +0x00 */
    s32 cleared[4]; /* +0x04: cleared by a loop */
    s32 mode;       /* +0x14 */
    unsigned char unmodeled_18[8];
    Vector4 destination;     /* +0x20 */
    Vector4 rotation;        /* +0x30: radians, w = 1 */
    DoublewordVector4 scale; /* +0x40 */
    Vector4 vector50;        /* +0x50 */
    Vector4 vector60;        /* +0x60 */
    float scalar70;          /* +0x70 */
    unsigned char unmodeled_74[0xC];
    Vector4 vector80; /* +0x80 */
    Vector4 vector90; /* +0x90 */
    unsigned char unmodeled_A0[0x10];
    float scalarB0; /* +0xB0 */
    float scalarB4; /* +0xB4 */
    s32 tail;       /* +0xB8 */
    unsigned char unmodeled_BC[4];
} GtestTransformView;

/* Per-frame pad update of the test actor: the right stick turns the yaw, the left stick moves the destination,
 * and the tail word selects the motion state (0, 1 or 2). The result is copied to the actor. */
void GtestTransformUpdateFromPad(GtestTransformView *transform) {
    XenoActor *actor;
    float savedY[2];
    DoublewordVector4 stick;
    float sinYaw;
    float cosYaw;
    float zeroLength;
    float dirX;
    float dirY;
    float length;

    zeroLength = 0.0f;
    length = 0.0f;
    transform->vector90.x = transform->vector90.x + (float) PadData[0].rightX * 0.001f;
    transform->vector90.y = transform->vector90.y + (float) PadData[0].rightY * 0.001f;
    sinYaw = xglSin(-transform->vector90.x);
    cosYaw = xglCos(-transform->vector90.x);
    dirY = (float) PadData[0].leftX * cosYaw - (float) PadData[0].leftY * sinYaw;
    dirX = (float) PadData[0].leftX * sinYaw + (float) PadData[0].leftY * cosYaw;
    savedY[0] = transform->destination.y + transform->vector50.y;
    savedY[1] = transform->vector50.y;
    transform->vector50.y = 0.0f;
    stick = (DoublewordVector4){{transform->vector50.x, 0.0f, transform->vector50.z, 1.0f}};
    if ((PadData[0].held & 0x8) != 0) {
        transform->destination.x = transform->destination.x + stick.vector.x;
        transform->destination.y = transform->destination.y + stick.vector.y;
        transform->destination.z = transform->destination.z + stick.vector.z;
    } else {
        length = func_002AA490(&transform->destination, &stick.vector, (Vector4 *) (transform + 1));
        if (-1000.0f >= length) {
            length = zeroLength;
        }
    }
    transform->destination.y = savedY[0];
    transform->vector50.y = savedY[1];
    switch ((u32) transform->tail) {
    case 0:
    case 1:
        transform->vector50.x = dirY * transform->scalar70;
        transform->vector50.z = dirX * transform->scalar70;
        transform->destination.y = length;
        if (dirY != 0.0f || dirX != 0.0f) {
            transform->scale.vector.y = tiWaterVectorAngle(dirX, dirY);
            actor = (XenoActor *) transform->kind;
            if (actor->currentMotionSlot != 1) {
                XenoActorEnableAndSetModeGroup((s32 *) transform, 1);
                ((XenoActor *) transform->kind)->resetFlags &= ~2;
            }
        } else {
            if (((XenoActor *) transform->kind)->currentMotionSlot != 0) {
                XenoActorEnableAndSetModeGroup((s32 *) transform, 0);
                ((XenoActor *) transform->kind)->resetFlags &= ~2;
            }
        }
        if ((PadData[0].pressed & 0x20) && transform->mode == 1) {
            transform->vector50.x = 0.0f;
            transform->vector50.y = 0.0f;
            transform->vector50.z = 0.0f;
            transform->tail = 2;
            XenoActorEnableAndSetModeGroup((s32 *) transform, 2);
        }
        break;
    case 2:
        if (XenoActorIsDone(transform->kind)) {
            transform->tail = 0;
        }
        transform->destination.y = length;
        if (20.0f < ((XenoActor *) transform->kind)->frame) {
            if (dirY > 0.1f || dirX > 0.1f) {
                transform->tail = 1;
            }
        }
        break;
    default:
        break;
    }
    AngleStateBlendShortestWay((AngleState *) &transform->rotation, (AngleState *) &transform->scale, 0.2f);
    QuadwordCopy(&((XenoActor *) transform->kind)->position, &transform->destination);
    QuadwordCopy(&((XenoActor *) transform->kind)->rotation, &transform->rotation);
}

unsigned short xglSRand(void);
void xglCurSetRotateY(float angle);
void xglCurApplyVector(Vector4 *destination, const Vector4 *vector);

/* Picks a random yaw in radians into randomRotation->y, sets offsetVector x to 0 and z to distance and rotates it by that yaw. */
void GtestRandomYawVector(Vector4 *offsetVector, Vector4 *randomRotation, float distance) {
    randomRotation->y = (float) (xglSRand() % 360) / 180.0f * 3.1415927f;
    offsetVector->z = distance;
    offsetVector->x = 0.0f;
    xglCurSetRotateY(randomRotation->y);
    xglCurApplyVector(offsetVector, offsetVector);
}

typedef struct GtestStudioRecord {
    unsigned char unmodeled_00[0x94];
    float field94; /* +0x94 */
    unsigned char unmodeled_98[8];
    float fieldA0; /* +0xA0 */
    float fieldA4; /* +0xA4 */
    unsigned char unmodeled_A8[0x28];
    float fieldD0; /* +0xD0 */
    float fieldD4; /* +0xD4 */
    float fieldD8; /* +0xD8 */
} GtestStudioRecord;

void xglStudioMove();

/* Updates the studio record from the transform: clamps the pitch in vector90.y to [-0.26, 1.22], computes the offset in
 * vector80 from scalarB0 and scalarB4, writes the position to fieldD0-fieldD8 and the angles to fieldA0/fieldA4, then
 * calls xglStudioMove. */
void GtestStudioUpdate(GtestTransform *transform, GtestStudioRecord *record) {
    float sinX;
    float cosY;
    float sinY;
    float cosX;
    float pitch;
    GtestStudioRecord *studioOut;
    float pitchMin;

    pitch = transform->vector90.y;
    pitchMin = -0.2617994f;
    if (pitch < pitchMin) {
        transform->vector90.y = pitchMin;
        pitch = pitchMin;
    }
    if (1.2217306f < pitch) {
        transform->vector90.y = 1.2217306f;
    }
    studioOut = record;
    sinX = xglSin(transform->vector90.x);
    cosY = xglCos(transform->vector90.y);
    transform->vector80.x = sinX * transform->scalarB0 * cosY;
    sinY = xglSin(transform->vector90.y);
    transform->vector80.y = sinY * transform->scalarB0 + transform->scalarB4;
    cosX = xglCos(transform->vector90.x);
    transform->vector80.z = cosX * transform->scalarB0 * xglCos(transform->vector90.y);
    studioOut->fieldD0 = transform->destination.x + transform->vector80.x;
    studioOut->fieldD4 = transform->destination.y + transform->vector80.y;
    studioOut->fieldD8 = transform->destination.z + transform->vector80.z;
    studioOut->fieldA0 = -transform->vector90.y;
    studioOut->fieldA4 = transform->vector90.x;
    studioOut->field94 = 1.0471976f;
    xglStudioMove();
}

extern int D_00550D48;
extern unsigned char gtestEmptyName[];
s32 strcmp(s8 *name, unsigned char *table);
void func_0028D760(XenoActor *actor);
void XenoActorSetChannelValue(XenoActor *actor, float channelValue);

typedef struct GtestNameTable {
    s8 *name[4];
} GtestNameTable;

const char D_0053A590[40] = "pAct->rsrc[XAct_Rsrc_MDL] != NULL";
const char D_0053A5B8[16] = "Gtest.euc.c";
const char D_0053A5C8[40] = "pAct->rsrc[XAct_Rsrc_TEX] != NULL";
const char D_0053A5F0[40] = "pAct->rsrc[XAct_Rsrc_XHR] != NULL";
const char D_0053A618[40] = "pAct->rsrc[XAct_Rsrc_XAP] != NULL";
const char D_0053A640[24] = "_prog\\zaku\\cf\\amuro.pxy";
const char D_0053A658[24] = "_prog\\zaku\\cf\\amuro.txy";
const char D_0053A670[24] = "_prog\\zaku\\cf\\amuro.xhr";
const char D_0053A688[32] = "_prog\\zaku\\cf\\wataru.xap";
const GtestNameTable gtestAmuroNameTable = { { (s8 *) D_0053A640, (s8 *) D_0053A658, (s8 *) D_0053A670, (s8 *) D_0053A688 } };
const char D_0053A6B8[8] = "pMap";
const char D_0053A6C0[32] = "_prog\\zaku\\cf\\E3_SYU08.pxy";
const char D_0053A6E0[32] = "_prog\\zaku\\cf\\E3_SYU08.txy";
const char D_0053A700[32] = "_prog\\zaku\\cf\\E3_SYU08.id";
const char D_0053A720[32] = "pMap->rsrc[XAct_Rsrc_MDL]";
const char D_0053A740[32] = "pMap->rsrc[XAct_Rsrc_TEX]";
const char D_0053A760[16] = "undu_data";
const DoublewordAlignedVector4 D_0053A770 = { 0.0f, 0.0f, -1.0f, 1.0f };
const char D_0053A790[32] = "_prog\\zaku\\cf\\ms06\\ms06_a.pxy";
const char D_0053A7B0[32] = "_prog\\zaku\\cf\\ms06\\ms06_a.txy";
const char D_0053A7D0[32] = "_prog\\zaku\\cf\\ms06\\ms06_a.xhr";
const char D_0053A7F0[32] = "_prog\\zaku\\cf\\ms06\\ms06_a.xap";
const char D_0053A810[32] = "_prog\\zaku\\cf\\ms06\\ms06_b.pxy";
const char D_0053A830[32] = "_prog\\zaku\\cf\\ms06\\ms06_b.txy";
const char D_0053A850[32] = "_prog\\zaku\\cf\\ms06\\ms06_b.xhr";
const char D_0053A870[32] = "_prog\\zaku\\cf\\ms06\\ms06_b.xap";
const char D_0053A8D0[32] = "_prog\\zaku\\cf\\ms06\\ms06_mg.pxy";
const char D_0053A8F0[32] = "_prog\\zaku\\cf\\ms06\\ms06_mg.txy";
const char D_0053A910[32] = "_prog\\zaku\\cf\\ms06\\ms06_mg.xhr";
const char D_0053A930[32] = "_prog\\zaku\\cf\\ms06\\ms06_mg.xap";
const char D_0053A9A0[32] = "_prog\\zaku\\cf\\E2_KML02.pxy";
const char D_0053A9C0[32] = "_prog\\zaku\\cf\\E2_KML02.txy";
const char D_0053A9E0[32] = "_prog\\zaku\\cf\\E2_KML02.id";
const char D_0053AA10[32] = "_prog\\zaku\\cf\\rx78\\rx78.pxy";
const char D_0053AA30[32] = "_prog\\zaku\\cf\\rx78\\rx78.txy";
const char D_0053AA50[32] = "_prog\\zaku\\cf\\rx78\\rx78.xhr";
const char D_0053AA70[32] = "_prog\\zaku\\cf\\rx78\\rx78.xap";
const char *D_0053AA90[4] = { D_0053AA10, D_0053AA30, D_0053AA50, D_0053AA70 };
const char D_0053AAA0[40] = "_prog\\zaku\\cf\\rx78\\rx78_rifle.pxy";
const char D_0053AAC8[40] = "_prog\\zaku\\cf\\rx78\\rx78_rifle.txy";
const char D_0053AAF0[40] = "_prog\\zaku\\cf\\rx78\\rx78_rifle.xhr";
const char D_0053AB18[40] = "_prog\\zaku\\cf\\rx78\\rx78_serbel.pxy";
const char D_0053AB40[40] = "_prog\\zaku\\cf\\rx78\\rx78_serbel.txy";
const char D_0053AB68[40] = "_prog\\zaku\\cf\\rx78\\rx78_serbel.xhr";
const char D_0053AB90[40] = "_prog\\zaku\\cf\\rx78\\rx78_serbel_l.xap";
const char D_0053ABB8[40] = "_prog\\zaku\\cf\\rx78\\rx78_shield.pxy";
const char D_0053ABE0[40] = "_prog\\zaku\\cf\\rx78\\rx78_shield.txy";
const char D_0053AC08[40] = "_prog\\zaku\\cf\\rx78\\rx78_shield.xhr";
const char *D_0053AC30[16] = { D_0053AAA0, D_0053AAC8, D_0053AAF0, gtestEmptyName, D_0053AB18, D_0053AB40, D_0053AB68, D_0053AB90, D_0053AB18, D_0053AB40, D_0053AB68, D_0053AB90, D_0053ABB8, D_0053ABE0, D_0053AC08, gtestEmptyName };
const char *D_0053AC70[8] = { D_0053A790, D_0053A7B0, D_0053A7D0, D_0053A7F0, D_0053A810, D_0053A830, D_0053A850, D_0053A870 };
const char *D_0053AC90[4] = { D_0053A8D0, D_0053A8F0, D_0053A910, D_0053A930 };
const float D_0053ACA0[8] = { 5.36999989f, 26.0f, -84.0f, 1.0f, 82.0f, 23.0f, -114.0f, 1.0f };
const char D_0053ACC0[32] = "_prog\\zaku\\cf\\E2_SUI02.pxy";
const char D_0053ACE0[32] = "_prog\\zaku\\cf\\E2_SUI02.txy";
const char D_0053AD00[32] = "_prog\\zaku\\cf\\E2_SUI02.id";
const float D_0053AD20[4] = { 186.149994f, 100.521004f, 100.521004f, 1.0f };
const char D_0053AD30[16] = "\245\326\245\354\245\363\245\311\263\253\273\317\000\000\000\000";
const char D_0053AD40[16] = "\245\342\241\274\245\267\245\347\245\363\000\000\000\000\000\000";
const char D_0053AD50[16] = "\245\242\245\257\245\277\241\274\000\000\000\000\000\000\000\000";
const char D_0053AD60[32] = "\245\336\245\303\245\327\000\000\150\157\163\164\060\072\057\150\157\155\145\057\170\145\156\157\057\167\157\162\153\057\000\000";

/* Loads the four files named in paths into the buffer at bufferCursor (each when strcmp accepts its name),
 * asserts each load and returns the advanced cursor. */
s32 GtestLoadActorFiles(XenoActor *actor, s8 **paths, s32 bufferCursor) {
    s32 loadedAddress;

    if (strcmp(paths[0], gtestEmptyName) != 0) {
        loadedAddress = xglCdReadFileAtCursor(&bufferCursor, paths[0]);
        actor->rsrc[0] = loadedAddress;
        if (loadedAddress == 0) {
            assertFailed((s8 *) D_0053A590, (s8 *) D_0053A5B8, 890);
        }
    }
    if (strcmp(paths[1], gtestEmptyName) != 0) {
        loadedAddress = xglCdReadFileAtCursor(&bufferCursor, paths[1]);
        actor->rsrc[1] = loadedAddress;
        if (loadedAddress == 0) {
            assertFailed((s8 *) D_0053A5C8, (s8 *) D_0053A5B8, 894);
        }
    }
    if (strcmp(paths[2], gtestEmptyName) != 0) {
        loadedAddress = xglCdReadFileAtCursor(&bufferCursor, paths[2]);
        actor->rsrc[2] = loadedAddress;
        if (loadedAddress == 0) {
            assertFailed((s8 *) D_0053A5F0, (s8 *) D_0053A5B8, 898);
        }
    }
    if (strcmp(paths[3], gtestEmptyName) != 0) {
        loadedAddress = xglCdReadFileAtCursor(&bufferCursor, paths[3]);
        actor->rsrc[4] = loadedAddress;
        if (loadedAddress == 0) {
            assertFailed((s8 *) D_0053A618, (s8 *) D_0053A5B8, 902);
        }
    }
    func_0028D760(actor);
    XenoActorSetChannelValue(actor, 5.0f);
    return bufferCursor;
}

extern unsigned char D_004C83F0[];
extern GifStatusBlock gifStatus;

typedef struct GtestTransformAlt {
    s32 kind; /* +0x00 */
    unsigned char unmodeled_04[0x30 - 0x04];
    Vector4 rotation; /* +0x30: y at +0x34 */
    unsigned char unmodeled_40[0xC0 - 0x40];
} GtestTransformAlt;

XenoActor *func_0028D670(void);
void mapCollisionInit(s32 loaded);
void func_002A70E8(unsigned char *setup);
void func_002911C0(void);
void func_002912B0(void);
void pxyEnableFlag8(void);
void pxySetHeaderScanEnabled(int enabled);
void XenoActorSetHoldFlag(void);
void XenoActorClearHoldFlag(void);
void XenoActorPoolInitDefault(void);
void XenoActorReleaseAll(void);
void xglStudioReset(void);
void xglStudioMainCameraInit(void);
void xglCameraSetPadPort(XglCamera *camera, int port);
void setCurrentStudio(int studio);
void studioCameraAt(XglCamera **camera, int index);
void xglSleep(void);
void xglRenderClearFrame(void);
void gifStateClearModeHook(void);
void gifStateSetEffectAlpha(s32 alphaType, s32 alpha);
void gifStateSetDecrease(float step);
void gifStateSetDepth(s32 depth);
void gifStateSetFbp(s32 fbp);
void gifStateSetBlur(s32 blur);
void gifStateSetMode(s32 mode);
void nmlModelSetMulColor(Vector4 *color);

/* Gtest main: loads the map and the test actor files, then runs the pad loop until the 0x08000100 buttons are all held. */
void GtestMainMapAndPad(void) {
    GtestNameTable pathTable = gtestAmuroNameTable;
    GtestTransformAlt transformRecord;
    unsigned char mapSetup[48];
    DoublewordAlignedVector4 mulColor;
    s32 fileCursor;
    XglCamera *camera;
    XenoActor *actor;
    s32 holdState;
    GtestTransformAlt *transformRecordPtr;
    fileCursor = 0x00E00000;
    XenoActorPoolInitDefault();
    holdState = 0;
    xglStudioReset();
    setCurrentStudio(0);
    xglStudioMainCameraInit();
    camera = ((XglCamera * (*) (int index)) xglStudioGetCamera2)(0);
    camera->cameraState = 1;
    xglCameraSetPadPort(camera, 1);
    {
        XenoActor *mapObject;
        s32 collisionLoaded;
        mapObject = func_0028D670();
        if (mapObject == 0) {
            assertFailed((s8 *) D_0053A6B8, (s8 *) D_0053A5B8, 952);
        }
        mapObject->rsrc[0] = xglCdReadFileAtCursor(&fileCursor, (s8 *) D_0053A6C0);
        mapObject->rsrc[1] = xglCdReadFileAtCursor(&fileCursor, (s8 *) D_0053A6E0);
        collisionLoaded = xglCdReadFileAtCursor(&fileCursor, (s8 *) D_0053A700);
        if (mapObject->rsrc[0] == 0) {
            assertFailed((s8 *) D_0053A720, (s8 *) D_0053A5B8, 963);
        }
        if (mapObject->rsrc[1] == 0) {
            assertFailed((s8 *) D_0053A740, (s8 *) D_0053A5B8, 964);
        }
        if (collisionLoaded == 0) {
            assertFailed((s8 *) D_0053A760, (s8 *) D_0053A5B8, 965);
        }
        mapObject->scale.x = 1.0f;
        mapObject->scale.y = 1.0f;
        mapObject->scale.z = 1.0f;
        func_0028D760(mapObject);
        mapCollisionInit(collisionLoaded);
    }
    transformRecordPtr = &transformRecord;
    func_002A70E8(mapSetup);
    *(s16 *) (mapSetup + 10) = 2;
    GtestActorBlockInitFirst(transformRecordPtr);
    actor = func_0028D670();
    transformRecord.kind = (s32) actor;
    fileCursor = GtestLoadActorFiles(actor, pathTable.name, fileCursor);
    /* kept: the union and raw float stores below are the spelling the match needs (plain Vector4 stores are NOTSTRICT). */
    {
        u8 *actorBytes = (u8 *) transformRecord.kind;
        ((union { float v; } *) (actorBytes + 128))->v = 1.0f;
        *(s16 *) (actorBytes + 156) = 9;
    }
    ((union { float v; } *) ((u8 *) transformRecord.kind + 132))->v = 1.0f;
    *(float *) ((u8 *) transformRecord.kind + 136) = 1.0f;
    while ((PadData[0].heldAndPressed & 0x08000100) != 0x08000100) {
        pxyEnableFlag8();
        pxySetHeaderScanEnabled(1);
        gtestLightSetGrey();
        if (holdState == 1) {
            XenoActorSetHoldFlag();
        } else {
            XenoActorClearHoldFlag();
        }
        {
            holdState = (holdState == 2) ? 1 : holdState;
            if (PadData[0].pressed & 0x1000) {
                holdState = !(holdState == 1);
            }
            if (PadData[0].repeated & 0x4000) {
                holdState = 2;
            }
            if (holdState != 1) {
                gifStatus.decreaseGate = 0;
                GtestTransformUpdateFromPad(&transformRecord);
            } else {
                gifStatus.decreaseGate = holdState;
            }
        }
        if (PadData[0].pressed & 1) {
            mulColor = D_0053A770;
            gifStateClearModeHook();
            xglCurSetRotateY(transformRecord.rotation.y);
            xglCurApplyVector((Vector4 *) &mulColor, (Vector4 *) &mulColor);
            nmlModelSetMulColor((Vector4 *) &mulColor);
            gifStateSetEffectAlpha(0, 64);
            gifStateSetDecrease(0.05f);
            gifStateSetDepth(0x00FFFFFF);
            gifStateSetFbp(0);
            gifStateSetBlur(15);
            gifStateSetMode(2);
        }
        studioCameraAt(&camera, 0);
        GtestStudioUpdate(&transformRecord, camera);
        func_002911C0();
        func_002912B0();
        xglSleep();
    }
    *(s32 *) D_004C83F0 = 0;
    XenoActorReleaseAll();
    xglRenderClearFrame();
}

typedef struct GtestPaths {
    const char *pxy;
    const char *txy;
    const char *xhr;
    const char *xap;
} GtestPaths;

typedef struct GtestMapSetup {
    unsigned char unmodeled_00[0x0A];
    short mode; /* +0x0A: 2 */
    unsigned char unmodeled_0c[0x30 - 0x0C];
} GtestMapSetup;

typedef struct GtestActorRecord {
    AlignedVector4 position; /* +0x00 */
    unsigned char unmodeled_10[0x20 - 0x10];
    Vector4 vectorA; /* +0x20 */
    Vector4 vectorB; /* +0x30 */
    unsigned char unmodeled_40[0x50 - 0x40];
    union {
        XenoActor *actor;
        s32 primary;
    }; /* +0x50 */
    s32 children[1]; /* +0x54 */
    unsigned char unmodeled_58[0x5C - 0x58];
    GtestMapSetup setup; /* +0x5C: func_002A70E8 block, mode at +0x66 */
    unsigned char unmodeled_8c[0x90 - 0x8C];
} GtestActorRecord;

typedef struct GtestSceneInfo {
    XenoActor *actor; /* +0x00 */
    s32 children[4];  /* +0x04 */
    unsigned char unmodeled_14[0x34 - 0x14];
    float rotationY; /* +0x34 */
    unsigned char unmodeled_38[0xC0 - 0x38];
} GtestSceneInfo;

void quadVertexSlotReset(GtestActorRecord *rec);
int func_00245C70(GtestActorRecord *rec, GtestSceneInfo *info);
int XenoActorAttachChildToJoint(XenoActor *parent, XenoActor *child, int joint);

#define GTEST_ACTOR(handle) ((XenoActor *) (handle))

extern void (*D_004C8390[])(void);

/* Gtest scene: loads the Amuro actor and the map, then four ms06 actors with one child each, attached to joints;
 * the same pad loop as GtestMainMapAndPad runs afterwards. */
void GtestMultiActorMain(void) {
    GtestPaths amuroPaths = {D_0053A640, D_0053A658, D_0053A670, D_0053A688};
    GtestPaths ms06Paths[4] = {{D_0053A790, D_0053A7B0, D_0053A7D0, D_0053A7F0},
                          {D_0053A810, D_0053A830, D_0053A850, D_0053A870},
                          {D_0053A810, D_0053A830, D_0053A850, D_0053A870},
                          {D_0053A810, D_0053A830, D_0053A850, D_0053A870}};
    GtestPaths mgPaths[1] = {{D_0053A8D0, D_0053A8F0, D_0053A910, D_0053A930}};
    AlignedVector4 startPositions[4] = {{-19.37f, 3.0f, 4.0f, 1.0f},
                                   {-19.37f, 3.0f, 4.0f, 1.0f},
                                   {-19.37f, 3.0f, 4.0f, 1.0f},
                                   {-19.37f, 3.0f, 4.0f, 1.0f}};
    GtestActorRecord actorRecords[4];
    GtestSceneInfo sceneInfo;
    GtestMapSetup mapSetup;
    int loadCursor;
    XglCamera *camera;
    XenoActor *mapActor;
    int unduLoaded;
    int holdState = 0;
    unsigned int i;
    unsigned int k;
    GtestPaths *childPath;
    GtestPaths *actorPath;
    XenoActor *sceneActor;
    XenoActor **parentHandle;
    AlignedVector4 *positionCursor;
    GtestActorRecord *recordCursor;
    XenoActor *child;
    unsigned int j;
    int cursorIn;
    int cursorResult;

    loadCursor = 0xE00000;
    XenoActorPoolInitDefault();
    xglStudioReset();
    setCurrentStudio(0);
    xglStudioMainCameraInit();
    camera = xglStudioGetCamera2(0);
    camera->cameraState = 1;
    xglCameraSetPadPort(camera, 1);

    mapActor = func_0028D670();
    if (mapActor == 0) {
        assertFailed(D_0053A6B8, D_0053A5B8, 1164);
    }
    mapActor->rsrc[0] = xglCdReadFileAtCursor(&loadCursor, D_0053A9A0);
    mapActor->rsrc[1] = xglCdReadFileAtCursor(&loadCursor, D_0053A9C0);
    unduLoaded = xglCdReadFileAtCursor(&loadCursor, D_0053A9E0);
    if (mapActor->rsrc[0] == 0) {
        assertFailed(D_0053A720, D_0053A5B8, 1169);
    }
    if (mapActor->rsrc[1] == 0) {
        assertFailed(D_0053A740, D_0053A5B8, 1170);
    }
    if (unduLoaded == 0) {
        assertFailed(D_0053A760, D_0053A5B8, 1171);
    }
    mapActor->scale.x = 1.0f;
    mapActor->scale.y = 1.0f;
    mapActor->scale.z = 1.0f;
    func_0028D760(mapActor);
    mapCollisionInit(unduLoaded);
    func_002A70E8(&mapSetup);
    mapSetup.mode = 2;

    GtestActorBlockInitSecond(&sceneInfo);
    sceneInfo.actor = func_0028D670();
    loadCursor = GtestLoadActorFiles(sceneInfo.actor, &amuroPaths, loadCursor);
    sceneActor = sceneInfo.actor;
    ((union { float v; } *) &sceneActor->scale.x)->v = 1.0f;
    sceneActor->unmodeled_09c = 9;
    ((union { float v; } *) &sceneInfo.actor->scale.y)->v = 1.0f;
    sceneInfo.actor->scale.z = 1.0f;

    positionCursor = startPositions;
    recordCursor = actorRecords;
    parentHandle = &actorRecords[0].actor;
    for (i = 0; i < 4;
         parentHandle = (XenoActor **) ((char *) parentHandle + sizeof(GtestActorRecord)), positionCursor++, recordCursor++, i++) {
        actorPath = &ms06Paths[i];
        quadVertexSlotReset(&actorRecords[i]);
        actorRecords[i].actor = func_0028D670();
        actorRecords[i].actor->scale.x = 1.8f;
        actorRecords[i].actor->scale.y = 1.8f;
        actorRecords[i].actor->scale.z = 1.8f;
        loadCursor = GtestLoadActorFiles(actorRecords[i].actor, actorPath, loadCursor);
        GtestRandomYawVector(&actorRecords[i].vectorB, &actorRecords[i].vectorA, 0.2f);
        func_002A70E8(&actorRecords[i].setup);
        actorRecords[i].setup.mode = 2;
        QuadwordCopy(&recordCursor->position, positionCursor);
        childPath = mgPaths;
        j = 0;
        for (k = 0; k < 1; k++, j++) {
            child = func_0028D670();
            cursorIn = loadCursor;
            actorRecords[i].children[j] = (s32) child;
            cursorResult = GtestLoadActorFiles(child, childPath++, cursorIn);
            XenoActorAttachChildToJoint(*parentHandle, GTEST_ACTOR(actorRecords[i].children[j]),
                                        (loadCursor = cursorResult, k + 1));
        }
        /* Called as a void function: the result is unused and v0 stays free after the call. */
        ((void (*)(ActorPairRecord *, s32)) XenoActorEnableAndSetMode)((ActorPairRecord *) &actorRecords[i], 1);
    }

    while ((PadData[0].heldAndPressed & 0x08000100) != 0x08000100) {
        pxyEnableFlag8();
        pxySetHeaderScanEnabled(1);
        gtestLightSetGrey();
        if (holdState == 1) {
            XenoActorSetHoldFlag();
        } else {
            XenoActorClearHoldFlag();
        }
        if (holdState == 2) {
            holdState = 1;
        }
        if (PadData[0].pressed & 0x1000) {
            holdState = (holdState != 1);
        }
        if (PadData[0].repeated & 0x4000) {
            holdState = 2;
        }
        if (holdState != 1) {
            gifStatus.decreaseGate = 0;
            GtestTransformUpdateFromPad(&sceneInfo);
            for (i = 0; i < 4; i++) {
                func_00245C70(&actorRecords[i], 0);
            }
        } else {
            gifStatus.decreaseGate = 1;
        }
        if (PadData[0].pressed & 1) {
            DoublewordAlignedVector4 mulColor = {0.0f, 0.0f, -1.0f, 1.0f};
            gifStateClearModeHook();
            xglCurSetRotateY(sceneInfo.rotationY);
            xglCurApplyVector(&mulColor, &mulColor);
            nmlModelSetMulColor(&mulColor);
            gifStateSetEffectAlpha(0, 64);
            gifStateSetDecrease(0.05f);
            gifStateSetDepth(0xFFFFFF);
            gifStateSetFbp(0);
            gifStateSetBlur(15);
            gifStateSetMode(2);
        }
        studioCameraAt(&camera, 0);
        GtestStudioUpdate(&sceneInfo, camera);
        func_002911C0();
        func_002912B0();
        xglSleep();
    }
    D_004C8390[24] = 0;
    XenoActorReleaseAll();
    xglRenderClearFrame();
}

INCLUDE_ASM("asm/main/nonmatchings/Gtest", func_00247790);

/* .sdata: written by func_00247790; the empty name the file-name tests compare with. */
int D_00550D48 = 0;
unsigned char gtestEmptyName[] = "";

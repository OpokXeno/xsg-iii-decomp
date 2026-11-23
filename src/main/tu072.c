#include "common.h"
#include "shared.h"

typedef struct StepDivisor {
    int flags;
    short frames;
    unsigned char unmodeled_06[0x2];
} StepDivisor;

typedef struct StepObject {
    unsigned char unmodeled_00[0x40];
    Vector4 origin;
    unsigned char unmodeled_50[0x80];
    StepDivisor divisor;
    unsigned char unmodeled_d8[0x8];
    Vector4 step;
} StepObject;

Vector4 *MMathSubVectorDivS(Vector4 *outVector, const Vector4 *lhs, const Vector4 *rhs, float scale);

/* Starts a linear move: step = (target - origin) / frames, at least one frame. */
void stepMoveStart(StepObject *object, int frameCount, const Vector4 *target) {
    StepDivisor *divisor = &object->divisor;

    if (frameCount <= 0) {
        frameCount = 1;
    }
    divisor->flags |= 1;
    divisor->frames = frameCount;
    MMathSubVectorDivS(&object->step, target, &object->origin, frameCount);
}

typedef struct RotStepRecord {
    unsigned int unmodeled_flags0 : 2;
    unsigned int active : 1;
    unsigned int unmodeled_flags3 : 29;
    unsigned char unmodeled_04[0x2];
    unsigned short frames;
    unsigned char unmodeled_08[0x18];
    Vector4 stepPerFrame;
} RotStepRecord;

typedef struct RotStepObject {
    unsigned char unmodeled_00[0x60];
    Vector4 rotation;
    unsigned char unmodeled_70[0x60];
    RotStepRecord rotationStep;
} RotStepObject;

float MMathCalcRotNear(float fromAngle, float toAngle);

/* Starts a rotation move: per-frame step of the shortest angle to the target on x, y and z. */
void stepRotationStart(RotStepObject *object, int frameCount, const Vector4 *target) {
    RotStepRecord *rotationStep = &object->rotationStep;

    if (frameCount <= 0) {
        frameCount = 1;
    }
    rotationStep->active = 1;
    rotationStep->frames = frameCount;
    rotationStep->stepPerFrame.x = MMathCalcRotNear(object->rotation.x, target->x) / frameCount;
    rotationStep->stepPerFrame.y = MMathCalcRotNear(object->rotation.y, target->y) / frameCount;
    rotationStep->stepPerFrame.z = MMathCalcRotNear(object->rotation.z, target->z) / frameCount;
    rotationStep->stepPerFrame.w = 1.0f;
}

typedef struct StepDivisorB {
    int flags;
    unsigned char unmodeled_04[0x4];
    short frames;
    unsigned char unmodeled_0a[0x6];
} StepDivisorB;

typedef struct StepObjectB {
    unsigned char unmodeled_00[0x80];
    Vector4 origin;
    unsigned char unmodeled_90[0x40];
    StepDivisorB divisor;
    unsigned char unmodeled_e0[0x20];
    Vector4 step;
} StepObjectB;

/* Same linear move as stepMoveStart on the object variant whose flag is 0x10. */
void func_001C1058(StepObjectB *object, int frameCount, const Vector4 *target) {
    StepDivisorB *divisor = &object->divisor;

    if (frameCount <= 0) {
        frameCount = 1;
    }
    divisor->flags |= 0x10;
    divisor->frames = frameCount;
    MMathSubVectorDivS(&object->step, target, &object->origin, frameCount);
}

typedef struct ClampedStepRecord {
    int flags;
    unsigned char unmodeled_04[0x6];
    short frames;
    unsigned char unmodeled_0c[0x34];
    float stepPerFrame;
    float clampedRatio;
} ClampedStepRecord;

typedef struct ClampedStepObject {
    unsigned char unmodeled_00[0xD0];
    ClampedStepRecord ratioStep;
    unsigned char unmodeled_118[0x10];
    float baseRatio;
} ClampedStepObject;

/* Starts a ratio move toward the target clamped to 0..1, stepped over frameCount frames. */
void stepRatioStart(ClampedStepObject *object, int frameCount, float ratio) {
    ClampedStepRecord *ratioStep = &object->ratioStep;
    float delta;

    ratio = ratio < 0.0f ? 0.0f : (ratio > 1.0f ? 1.0f : ratio);
    delta = ratio - object->baseRatio;
    if (frameCount <= 0) {
        frameCount = 1;
    }
    ratioStep->stepPerFrame = delta / frameCount;
    ratioStep->frames = frameCount;
    ratioStep->flags |= 0x40;
    ratioStep->clampedRatio = ratio;
}

typedef struct ChildBindRequest {
    unsigned char unmodeled_00[0x10];
    unsigned int spawnKind;
} ChildBindRequest;

int actorChildBindMode(void *owner, int arg);
int actorChildBindLayer(void *owner, int arg);
int actorSetId(void *owner, int arg);
int playbackSetupWithoutFlags(void *owner, int arg);
int playbackSetupWithFlags(void *owner, int arg);
int objectSetChildId(void *owner, int arg);
int emitterRestartWithKind(void *owner, int arg);
int emitterStartMode1(void *owner, int arg);
int emitterStartMode2(void *owner, int arg);
int emitterStartMode3(void *owner, int arg);
int dispModelBindValue(void *dispModel, int bindValue);

/* Dispatches a bind request by its spawn kind to the matching binder. */
int bindRequestDispatch(ChildBindRequest *request, int arg) {
    int bindResult;

    switch (request->spawnKind) {
    case 0x1:
        bindResult = emitterStartMode1(request, arg);
        break;
    case 0x2:
        bindResult = emitterStartMode2(request, arg);
        break;
    case 0x4:
        bindResult = playbackSetupWithFlags(request, arg);
        break;
    case 0x10:
        bindResult = dispModelBindValue(request, arg);
        break;
    case 0x40:
        bindResult = emitterStartMode3(request, arg);
        break;
    case 0x80:
        bindResult = actorChildBindMode(request, arg);
        break;
    case 0x100:
        bindResult = playbackSetupWithoutFlags(request, arg);
        break;
    case 0x200:
        bindResult = objectSetChildId(request, arg);
        break;
    case 0x400:
        bindResult = actorSetId(request, arg);
        break;
    case 0x8000:
        bindResult = emitterRestartWithKind(request, arg);
        break;
    case 0x10000:
        bindResult = actorChildBindLayer(request, arg);
        break;
    }
}

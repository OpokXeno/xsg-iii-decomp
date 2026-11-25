#include "common.h"
#include "shared.h"

extern f32 D_003E7280[];
extern f32 D_003E7290[];

/* Position of the camera target for row and slot: x = slot * 4, y from the row table D_003E7280 (row & 3) plus or minus
 * D_003E7290[slot] for rows 2 and 3, and 2 lower in slot 0; z = 0, w = 1. */
void cameraSlotPosition(Vector4 *position, int row, int slot) {
    position->x = (f32) slot * 4.0f;
    position->y = D_003E7280[row & 3];
    position->z = 0.0f;
    position->w = 1.0f;
    if (row == 2) {
        position->y = position->y - D_003E7290[slot];
    } else if (row == 3) {
        position->y = position->y + D_003E7290[slot];
    }
    if (slot == 0) {
        position->y -= 2.0f;
    }
}

typedef int s32;

void setSteppedValue(s32 stepped, s32 value, s32 target);

void actorSteppedValueClear(s32 actor) {
    setSteppedValue(actor + 0x54, 0, 0);
}

typedef union CameraVector {
    Vector4 vector;
    unsigned long words[2];
    s32 integers[4];
} CameraVector;

typedef struct StudioCamera {
    s32 active;
    s32 mode;
    unsigned char unmodeled_08[0x8C];
    f32 fieldOfView;
    unsigned char unmodeled_98[8];
    CameraVector rotation;
    unsigned char unmodeled_B0[0x20];
    CameraVector position;
} StudioCamera;

typedef struct CameraActor {
    CameraVector position;
    CameraVector previousPosition;
    CameraVector rotation;
    CameraVector previousRotation;
    s32 kind;
    s32 row;
    s32 slot;
    s32 unk_4C;
    s32 unk_50;
    s32 steppedValues[2];
} CameraActor;

s32 func_00223B08(s32 kind, s32 row, s32 slot);
StudioCamera *xglStudioGetCamera2(int camera_index);

/* Attaches the camera actor to a row and slot: resets its stepped values, stores kind, row and slot (row and slot go
 * to 0 when func_00223B08 rejects them), then sets studio camera 1 to the slot target and copies the camera pose into
 * the actor. */
void cameraActorAttach(CameraActor *actor, s32 kind, s32 row, s32 slot) {
    Vector4 target;
    StudioCamera *camera;

    setSteppedValue((s32) &actor->steppedValues[0], 0, 0);
    setSteppedValue((s32) &actor->steppedValues[1], 0x1000, 0x1000);
    actor->kind = kind;
    actor->row = row;
    actor->slot = slot;
    actor->unk_4C = 0;
    actor->unk_50 = 1;
    if (func_00223B08(kind, row, slot) == 0) {
        actor->row = 0;
        actor->slot = 0;
    }
    camera = xglStudioGetCamera2(1);
    camera->mode = 4;
    cameraSlotPosition(&target, row, slot);
    camera->position.vector.x = target.x;
    camera->position.vector.y = target.y - 1.0f;
    camera->position.vector.z = 16.0f;
    camera->rotation.vector.z = 0.0f;
    camera->rotation.vector.y = 0.0f;
    camera->rotation.vector.x = 0.0f;
    camera->fieldOfView = 0.69813174f;
    actor->previousPosition = actor->position = camera->position;
    actor->previousRotation = actor->rotation = camera->rotation;
    camera->active = 1;
}

f32 D_003E7280[4] = {
    2.0f, -2.0f, 6.0f, -6.0f
};
f32 D_003E7290[8] = {
    0.0f, 1.5f, 0.5f, 0.0f, 0.0f, 0.5f, 1.5f, 0.0f
};
signed char D_003E72B0[8] = {
    0, 13, 12, 14, 15, 0, 0, 0
};
unsigned int D_003E72B8[4] = {
    0x5D317Fu, 0x7F285Du, 0x7B504Au, 0x7B504Au
};
unsigned int D_003E72C8[14] = {
    0x717F2Fu, 0x6E587Fu, 0x7F4E6Cu, 0x7B504Au, 0x7B504Au, 0x717F2Fu, 0x65437Fu, 0x7F3562u,
    0x7B504Au, 0x7B504Au, 0x767F4Cu, 0, 0, 0
};

#include "common.h"
#include "shared.h"

#include "main/xgl_2.h"

struct XglCamera;
struct XglCamera *xglStudioSelectGetActiveCamera(int studio_index);

void _CurMatrixSet(Matrix4 *matrix);


void _CurSetViewScaleTrans(Vector4 *scale, Vector4 *translation);

int _CurRotTransPersClipScreen(ScreenVector4 *screenOut, const Vector4 *point);

/* Two screen coordinates, scaled by 16 before uwamonoProjectToScreen halves them. */
typedef ScreenVector4 UwamonoScreenPos;

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A80930);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A816C8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A81940);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A81A40);

typedef int s32;
s32 func_00A807B0(s16 actorIndex);

/* One 0x2A0-byte slot of the moving-actor table at gMovingActorSlots (4 slots; offset 0x10 is also read by get_019 as a random seed) (layout from actorTableMoveEntry and actorTableFindLongestTimer). */
typedef struct MovingActorSlot {
    s32 id;                 /* 0x00, -1 when the entry is free */
    u8 unk_04[0xC];
    f32 pos_x;              /* 0x10 */
    f32 pos_y;              /* 0x14 */
    f32 pos_z;              /* 0x18 */
    u8 unk_1C[0x14];
    f32 vel_x;              /* 0x30 */
    f32 vel_y;              /* 0x34 */
    f32 vel_z;              /* 0x38 */
    u8 unk_3C[0x24];
    f32 fade;               /* 0x60 */
    u8 unk_64[0x1FC];
    s16 fadeState;              /* 0x260 */
    s16 timer;              /* 0x262 */
    u8 unk_264[0x3C];
} MovingActorSlot;

extern MovingActorSlot *gMovingActorSlots;

s32 actorTableStepFade(s16 actorIndex) {
    MovingActorSlot *actorEntry;
    f32 timer_as_float;
    f32 fade_value;
    s16 timer;
    s16 actorTableIndex;
    s32 still_running;

    actorTableIndex = actorIndex;
    actorEntry = gMovingActorSlots + actorTableIndex;
    timer = actorEntry->timer;
    timer_as_float = timer;
    still_running = timer < 0x3C;
    actorEntry->timer = actorEntry->timer + 1;
    fade_value = 1.0f - (timer_as_float / 30.0f);
    actorEntry->fade = fade_value;
    if (still_running == 0) {
        func_00A807B0(actorTableIndex);
        return 0;
    }
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A81BB0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A81CD8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A81F68);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A822E8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A82548);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A82968);

s32 func_00A8D5A8(s16 actorIndex);

void actorTableMoveEntry(s16 actorIndex, f32 delta_x, f32 delta_y, f32 delta_z) {
    MovingActorSlot *actorEntry;
    s16 actorTableIndex;

    actorTableIndex = actorIndex;
    actorEntry = gMovingActorSlots + actorTableIndex;
    actorEntry->pos_x += delta_x;
    if (func_00A8D5A8(actorTableIndex) != -1) {
        actorEntry->pos_x -= delta_x;
        actorEntry->vel_x *= 0.75f;
    }
    actorEntry->pos_y += delta_y;
    if (func_00A8D5A8(actorTableIndex) != -1) {
        actorEntry->pos_y -= delta_y;
        actorEntry->vel_y *= 0.75f;
    }
    actorEntry->pos_z += delta_z;
    if (func_00A8D5A8(actorTableIndex) != -1) {
        actorEntry->pos_z -= delta_z;
        actorEntry->vel_z *= 0.75f;
    }
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A839C8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A83A78);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A83C20);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A83E60);

s16 actorTableFindLongestTimer(void);
s32 func_00A9D718(s32 id, s32 mode, s32 value);

void actorTableExpireLongest(void) {
    s32 longestIndex;

    longestIndex = actorTableFindLongestTimer();
    if (longestIndex != -1) {
        if (gMovingActorSlots[longestIndex].timer >= 0xF) {
            gMovingActorSlots[longestIndex].timer = 0;
            gMovingActorSlots[longestIndex].fadeState = 4;
            func_00A9D718(-1, 3, 3);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A84238);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A844A0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A844E8);

s16 actorTableFindLongestTimer(void)
{
    MovingActorSlot *actorEntry;
    s16 best_timer;
    s16 best_index;
    s32 actorIndex;

    best_timer = -1;
    actorEntry = gMovingActorSlots;
    best_index = -1;
    for (actorIndex = 0; actorIndex < 4; actorIndex++) {
        if (actorEntry->id != -1 && actorEntry->fadeState == 3) {
            if (best_timer < actorEntry->timer) {
                best_timer = actorEntry->timer;
                best_index = actorIndex;
            }
        }
        actorEntry += 1;
    }
    return best_index;
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A847F8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A84870);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A84C90);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A85600);

void func_00A85890(f32 *position, s32 mode);
typedef struct EntryPosition {
    f32 x;
    f32 y;
    f32 z;
} EntryPosition;

void actorTableQueryPosition(s16 actorIndex, s32 queryMode, f32 delta_x, f32 delta_y, f32 delta_z) {
    EntryPosition queryPosition;
    MovingActorSlot *actorEntry;
    s16 actorTableIndex;

    actorTableIndex = actorIndex;
    actorEntry = gMovingActorSlots + actorTableIndex;
    queryPosition.x = actorEntry->pos_x + delta_x;
    queryPosition.y = actorEntry->pos_y + delta_y;
    queryPosition.z = actorEntry->pos_z + delta_z;
    func_00A85890(&queryPosition.x, queryMode);
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A85890);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A858F0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A86048);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A861C8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A862C0);

void actorTableNoopReset(void) {

}

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A863B8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A863C8);

void actorTableNoopTick(void) {

}

void actorTableNoopEnd(void) {

}

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A863E8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A863F8);

/*
 * Sets up the active camera's vectors, then rescales the two screen coordinates in
 * position (x16) to centred values; _CurRotTransPersClipScreen can replace both with 0x1000.
 */
void uwamonoProjectToScreen(const Vector4 *query, UwamonoScreenPos *position) {
    XglCamera *camera;

    camera = xglStudioSelectGetActiveCamera(0);
    _CurMatrixSet(&camera->volume[0].viewMatrix);
    _CurSetViewScaleTrans(&camera->screen.screenScale, &camera->screen.screenOffset);
    if (_CurRotTransPersClipScreen(position, query) != 0) {
        position->x = 0x1000;
        position->y = 0x1000;
        return;
    }
    position->x = ((position->x >> 4) - 0x700) >> 1;
    position->y = ((position->y >> 4) - 0x720) >> 1;
}

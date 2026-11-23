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

/* Coordinate of one entry in the 2-byte table at gEntryCellTable: group (0x98 per group), row (0x13 per row), column. */
typedef struct EntryCoord {
    s16 cellGroupIndex;
    s16 rowIndex;
    s16 columnIndex;
} EntryCoord;

/* A transform position: read as floats, copied as two doublewords (see ov04/tu004). */
typedef union TransformPosition {
    Vector4 vector;
    u64 words[2];
    s32 ivector[4];
} TransformPosition;

/* One 0xC0-byte transform of the pool at gTransformPool (see ov04/tu004). */
typedef struct TransformSlot {
    s16 kind;                  /* 0x00: model kind, -1 hidden */
    u8 unk_02[0xE];
    TransformPosition position; /* 0x10 */
    u8 unk_20[0xA0];
} TransformSlot;

typedef struct TransformPool {
    TransformSlot slots[277];
} TransformPool;

/* The transforms of the moving actors follow the other pool users. */
#define MOVING_ACTOR_TRANSFORM_FIRST 241

/* Pad button bits (see ov04/tu004). */
#define PAD_TRIANGLE 0x0010
#define PAD_CIRCLE 0x0020
#define PAD_CROSS 0x0040
#define PAD_SQUARE 0x0080

/* MovingActorSlot.eventFlags bits. */
#define ACTOR_EVENT_BELOW_SURFACE 0x100
#define ACTOR_EVENT_LANDING_SOUND 0x80

/* MovingActorSlot.motionState values whose meaning the code proves; the others stay literal. */
#define ACTOR_STATE_IDLE 0
#define ACTOR_STATE_WALKING 1
#define ACTOR_STATE_FELL_OFF 8
#define ACTOR_STATE_CRUSHED 9
#define ACTOR_STATE_FADING_OUT 10

extern TransformPool *gTransformPool;

/* Screen mode control block at gScreenModeControlStorage (see ov04/tu004); only the fields read here are named. */
typedef struct ScreenModeControl {
    s32 modeBits;             /* 0x0000: SCREEN_MODE_* */
    u8 unk_0004[0x224E];
    s16 speedLevel;            /* 0x2252: index of the speed tables */
    u8 unk_2254[0x36];
    u16 demoButtons;           /* 0x228A: while recording (flag 0x400), the pad button of the lift lane under the standing actor */
} ScreenModeControl;

extern ScreenModeControl gScreenModeControlStorage;
extern s16 *gEntryCellTable;

#define ENTRY_CELL(group, row, column) gEntryCellTable[(row) * 0x13 + (group) * 0x98 + (column)]
/* Byte offset of a coordinate's cell in gEntryCellTable, and the cell at a byte offset (see ov04/tu004). */
#define ENTRY_CELL_BYTE_OFFSET(coord) (((coord)->rowIndex * 0x13 + (coord)->cellGroupIndex * 0x98 + (coord)->columnIndex) * 2)
#define ENTRY_CELL_AT_OFFSET(tableByteDistance) (*(s16 *) ((u8 *) gEntryCellTable + (tableByteDistance)))
/* The home coordinates of the 10 lifts follow the 19 x 8 x 19 cell grid in the same table (tu004: lift travel and axis follow them). */
#define LIFT_HOME_COORD_OFFSET(liftIndex) (0x1690 + (liftIndex) * 6)

/* Model record of a moving actor; only the word cleared when the actor is crushed is known. */
typedef struct ActorModel {
    u8 unk_00[0x574];
    s32 unk_574;
} ActorModel;

/* One 0x2A0-byte slot of the moving-actor table at gMovingActorSlots (4 slots; offset 0x10 is also read by get_019 as a
   random seed). Shared by ov04 tu002 (walking, falling, fading) and tu004 (lifts carrying and pushing the actors). */
typedef struct MovingActorSlot {
    s32 id;                    /* 0x000: -1 when the entry is free */
    ActorModel *model;         /* 0x004 */
    s32 eventFlags;            /* 0x008: bit 0x100 fell below the board surface, 0x80 landing sound played */
    u8 unk_0C[4];
    TransformPosition position; /* 0x010 */
    u8 unk_20[0x10];
    Vector4 velocity;          /* 0x030 */
    f32 pitch;                 /* 0x040: radians */
    f32 heading;               /* 0x044: radians */
    f32 roll;                  /* 0x048: radians */
    u8 unk_4C[4];
    s32 unk_50;
    s32 unk_54;
    f32 unk_58;
    u8 unk_5C[4];
    f32 fadeAlpha;             /* 0x060: 1 -> 0 while fading out */
    u8 unk_64[0x19C];
    f32 targetPitch;           /* 0x200 */
    f32 targetHeading;         /* 0x204 */
    f32 targetRoll;            /* 0x208 */
    u8 unk_20C[0x34];
    s16 fallFrames;            /* 0x240: frames spent falling */
    s16 unk_242;
    u8 unk_244[0xC];
    TransformPosition extent;  /* 0x250: half size */
    s16 motionState;           /* 0x260: ACTOR_STATE_* */
    s16 timer;                 /* 0x262 */
    u8 unk_264[0xC];
    TransformPosition walkTarget; /* 0x270: the point the actor walks to; lifts carry it along */
    s16 direction;             /* 0x280: 0..3 walk direction or last lift push, -1 none */
    u8 unk_282[2];
    s32 frameCount;            /* 0x284 */
    s16 liftIndex;             /* 0x288: the lift the actor stands on, -1 none */
    u8 unk_28A[0x16];
} MovingActorSlot;

extern MovingActorSlot *gMovingActorSlots;

s32 actorTableStepFade(s16 actorIndex) {
    MovingActorSlot *actor;
    f32 elapsedFrames;
    f32 alpha;
    s16 timer;
    s16 actorTableIndex;
    s32 stillFading;

    actorTableIndex = actorIndex;
    actor = gMovingActorSlots + actorTableIndex;
    timer = actor->timer;
    elapsedFrames = timer;
    stillFading = timer < 0x3C;
    actor->timer = actor->timer + 1;
    alpha = 1.0f - (elapsedFrames / 30.0f);
    actor->fadeAlpha = alpha;
    if (stillFading == 0) {
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

s32 func_00A9D718(s32 id, s32 mode, s32 value);
void actorTableMoveEntry(s16 actorIndex, f32 deltaX, f32 deltaY, f32 deltaZ);
s32 func_00A816C8(s16 actorIndex);
s32 func_00A81940(s16 actorIndex);
s32 func_00A81A40(s16 actorIndex);
s32 func_00A81BB0(s16 actorIndex);
s32 func_00A81CD8(s16 actorIndex);
s32 func_00A81F68(s16 actorIndex);
s32 func_00A822E8(s16 actorIndex);
s32 func_00A82548(s16 actorIndex);
f32 floatApproach(f32 current, f32 target, f32 speed);
f32 func_00A9FB60(f32 current, f32 target, f32 speed);
void func_00A99A30(s16 group, s16 column, f32 scale);
void func_00A9DAC0(s16 actorIndex, s32 sound);
f32 func_00A84C90(s16 actorIndex, EntryCoord *floorCell);
f32 func_00A85600(s16 actorIndex);
void actorTableStartChannelSelector(s16 actorIndex, s16 actorSelector);
void actorTableStartChannelValue(s16 actorIndex, s16 actorSelector, f32 channelValue);
s32 Get_Rnd(s32 minimum, s32 maximum);
void func_00A97D90(s16 kind, TransformPosition *position, f32 x, f32 y, f32 z, f32 scale);
f32 func_00A861C8(TransformPosition *position, s16 standIndex);
s32 func_00A858F0(s16 actorIndex, s16 direction);
void boardCellCenter(EntryCoord *coord, TransformPosition *position);
void func_00A85890(f32 *position, EntryCoord *cell);
void actorTableQueryPosition(s16 actorIndex, EntryCoord *cell, f32 deltaX, f32 deltaY, f32 deltaZ);
void actorTableNoopTick(TransformPosition *startPosition, TransformPosition *endPosition, s32 tickInterval);
float Get_Angle(const Vector4 *fromPoint, const Vector4 *toPoint);
float Get_Distance(const Vector4 *fromPoint, const Vector4 *toPoint);
void func_00A84238(s16 actorIndex, EntryCoord *cell);
float sinf(float);
float cosf(float);
double fabs(double);

/* Cell step of each direction, its facing angle, and the walking speed and sound of each speed level. */
extern s16 gDirectionCellStepX[4];
extern s16 gDirectionCellStepZ[4];
extern f32 gDirectionFacingYaw[4];
extern f32 gWalkSpeedByLevel[];
extern f32 gWalkChannelByLevel[];
extern f32 gWalkChannelValueByLevel[];

/* Index of a coordinate's cell in gEntryCellTable. */
static inline s32 entryCellIndex(EntryCoord *coord)
{
    return coord->rowIndex * 0x13 + coord->cellGroupIndex * 0x98 + coord->columnIndex;
}

/* The halfword at a byte offset from the address of a coordinate field. */
static inline s16 cellFieldAtByteOffset(s16 *coordField, s32 tableByteDistance)
{
    return *(s16 *) ((u8 *) coordField + tableByteDistance);
}

/* One frame of a moving actor: the special states run their own handler; otherwise it falls, lands, slows
 * down, picks the next cell in its direction (turning when the cell is closed) and walks there. */
s32 actorTableStepMotion(s16 actorIndex)
{
    EntryCoord cell;
    EntryCoord floorCell;
    EntryCoord standCell;
    EntryCoord destinationCell;
    s32 blocked;
    MovingActorSlot *actor;
    TransformSlot *transform;
    f32 floorHeight;
    f32 ceilingHeight;
    double absVelocityX;
    double absVelocityZ;
    f32 angle;
    s32 grounded;
    s32 moving;
    s32 i;
    s16 cellValue;
    s32 cellOffset;
    s32 floorCellOffset;

    actor = gMovingActorSlots + actorIndex;
    grounded = 0;
    moving = 0;
    blocked = 0;
    cellValue = -1;
    actor->liftIndex = cellValue;
    actor->frameCount++;
    switch (actor->motionState) {
    case 3:
        return func_00A816C8(actorIndex);
    case 4:
        return func_00A81940(actorIndex);
    }
    actor->pitch = floatApproach(actor->pitch, actor->targetPitch, 8.0f);
    actor->heading = func_00A9FB60(actor->heading, actor->targetHeading, 3.0f);
    actor->roll = floatApproach(actor->roll, actor->targetRoll, 8.0f);
    (gTransformPool->slots + MOVING_ACTOR_TRANSFORM_FIRST + actorIndex)->position = actor->position;
    (gTransformPool->slots + MOVING_ACTOR_TRANSFORM_FIRST + actorIndex)->position.vector.y += 1.95f;
    switch (actor->motionState) {
    case ACTOR_STATE_CRUSHED:
        return func_00A81A40(actorIndex);
    case ACTOR_STATE_FADING_OUT:
        return actorTableStepFade(actorIndex);
    case ACTOR_STATE_FELL_OFF:
        return func_00A81BB0(actorIndex);
    case 7:
        return func_00A81CD8(actorIndex);
    case 2:
        func_00A81F68(actorIndex);
        return 1;
    case 5:
        func_00A822E8(actorIndex);
        return 1;
    case 6:
        func_00A82548(actorIndex);
        return 1;
    }
    if (actor->position.vector.y <= 1.5f && !(actor->eventFlags & ACTOR_EVENT_BELOW_SURFACE)) {
        actorTableQueryPosition(actorIndex, &cell, 0.0f, 0.0f, 0.0f);
        func_00A99A30(cell.cellGroupIndex + 1, cell.columnIndex + 1, 1.0f);
        func_00A9D718(actorIndex, 3, 5);
        actor->eventFlags |= ACTOR_EVENT_BELOW_SURFACE;
        actor->targetPitch = 3.1415927f;
    }
    if (actor->position.vector.y <= 0.0f && !(actor->eventFlags & ACTOR_EVENT_LANDING_SOUND)) {
        func_00A9DAC0(actorIndex, 7);
        actor->eventFlags |= ACTOR_EVENT_LANDING_SOUND;
    }
    if (actor->position.vector.y <= -5.0f && actor->motionState != ACTOR_STATE_FELL_OFF) {
        actor->motionState = ACTOR_STATE_FELL_OFF;
        actor->timer = 0;
        return 1;
    }
    floorHeight = func_00A84C90(actorIndex, &floorCell);
    ceilingHeight = func_00A85600(actorIndex);
    if (floorHeight != -1000.0f && ceilingHeight != -1000.0f && ceilingHeight < floorHeight + actor->extent.vector.y) {
        actor->motionState = ACTOR_STATE_CRUSHED;
        actor->extent.ivector[0] = 0;
        actor->unk_54 = 0;
        actor->timer = 0;
        actor->model->unk_574 = 0;
        func_00A9DAC0(actorIndex, 10);
        return 1;
    }
    if (floorHeight < actor->position.vector.y) {
        actor->velocity.y -= 0.02f;
        if (actor->velocity.y < -0.5f) {
            actor->velocity.y = -0.5f;
        }
        if (actor->velocity.y < 0.0f) {
            actor->fallFrames++;
        }
        if (actor->fallFrames == 8) {
            func_00A9D718(actorIndex, 0, 6);
            actorTableStartChannelSelector(actorIndex, 10);
        }
    } else {
        if (actor->velocity.y < 0.0f) {
            actor->velocity.y = 0.0f;
        }
        actor->position.vector.y = floorHeight;
        grounded = 1;
        actor->fallFrames = 0;
    }
    actor->position.vector.y += actor->velocity.y;
    if (actor->position.vector.y < floorHeight && floorHeight <= actor->position.vector.y - actor->velocity.y) {
        grounded = 1;
        actor->position.vector.y = floorHeight;
        actor->motionState = ACTOR_STATE_IDLE;
        if (actor->fallFrames >= 8) {
            func_00A9D718(-1, 0, 1);
            for (i = 0; i < 3; i++) {
                func_00A97D90(actor->id + 39, &actor->position, Get_Rnd(-250, 250) / 1000.0f, 0.25f,
                              Get_Rnd(-250, 250) / 1000.0f, 0.3f);
            }
        }
        actorTableStartChannelSelector(actorIndex, 0);
    }
    if (ceilingHeight != -1000.0f && ceilingHeight <= actor->position.vector.y + actor->extent.vector.y) {
        actor->velocity.y = 0.0f;
        actor->position.vector.y = ceilingHeight - actor->extent.vector.y;
    }
    if (!(actor->eventFlags & ACTOR_EVENT_BELOW_SURFACE)) {
        if (actor->velocity.y == 0.0f) {
            actor->targetPitch = 0.0f;
        }
        if (actor->velocity.y < 0.0f) {
            actor->targetPitch = -0.19634955f;
            actorTableStartChannelValue(actorIndex, 2, 0.25f);
        }
        if (0.0f < actor->velocity.y) {
            actor->targetPitch = -0.3926991f;
            actorTableStartChannelValue(actorIndex, 2, 0.25f);
        }
    }
    actorTableMoveEntry(actorIndex, actor->velocity.x, 0.0f, actor->velocity.z);
    if (grounded == 1) {
        actor->velocity.x *= 0.75f;
        actor->velocity.z *= 0.75f;
    } else {
        actor->velocity.x *= 0.95f;
        actor->velocity.z *= 0.95f;
    }
    absVelocityX = actor->velocity.x;
    if (absVelocityX < 0.0) {
        absVelocityX = -absVelocityX;
    }
    if (absVelocityX <= 0.01f) {
        actor->velocity.x = 0.0f;
    }
    absVelocityZ = actor->velocity.z;
    if (absVelocityZ < 0.0) {
        absVelocityZ = -absVelocityZ;
    }
    if (absVelocityZ <= 0.01f) {
        actor->velocity.z = 0.0f;
    }
    if (actor->velocity.x != 0.0f || actor->velocity.z != 0.0f) {
        moving = 1;
    }
    if (moving == 1) {
        actor->motionState = ACTOR_STATE_IDLE;
    }
    for (i = 0; i < 10; i++) {
        if (actor->position.vector.y == func_00A861C8(&actor->position, i)) {
            actor->liftIndex = i;
            break;
        }
    }
    if (gScreenModeControlStorage.modeBits & 0x400) {
        gScreenModeControlStorage.demoButtons = 0;
        if (actor->liftIndex != -1) {
            s32 liftCoordOffset;

            liftCoordOffset = LIFT_HOME_COORD_OFFSET(actor->liftIndex);
            cellValue = cellFieldAtByteOffset(&((EntryCoord *) gEntryCellTable)->columnIndex, liftCoordOffset);
            liftCoordOffset = (cellFieldAtByteOffset(&((EntryCoord *) gEntryCellTable)->rowIndex, liftCoordOffset) * 0x13 + cellFieldAtByteOffset(&((EntryCoord *) gEntryCellTable)->cellGroupIndex, liftCoordOffset) * 0x98 + cellValue) * 2;
            cellValue = ENTRY_CELL_AT_OFFSET(liftCoordOffset) - 6;
            switch (cellValue) {
            case 0:
                gScreenModeControlStorage.demoButtons = PAD_CIRCLE;
                break;
            case 1:
                gScreenModeControlStorage.demoButtons = PAD_CROSS;
                break;
            case 2:
                gScreenModeControlStorage.demoButtons = PAD_TRIANGLE;
                break;
            case 3:
                gScreenModeControlStorage.demoButtons = PAD_SQUARE;
                break;
            }
        }
    }
    if (grounded == 1) {
        /* Keep the direction while its next cell is open, else turn right, left, then back. */
        for (;;) {
            if (actor->motionState == ACTOR_STATE_IDLE && moving == 0) {
                if (func_00A858F0(actorIndex, actor->direction) == 1) {
                    break;
                }
                if (func_00A858F0(actorIndex, (actor->direction + 1) % 4) == 1) {
                    actor->direction = (actor->direction + 1) % 4;
                    break;
                }
                if (func_00A858F0(actorIndex, (actor->direction + 3) % 4) == 1) {
                    actor->direction = (actor->direction + 3) % 4;
                    break;
                }
                if (func_00A858F0(actorIndex, (actor->direction + 2) % 4) == 1) {
                    actor->direction = (actor->direction + 2) % 4;
                    break;
                }
            }
            blocked = 1;
            break;
        }
        if (!blocked) {
            if (actor->direction == -1) {
                actorTableQueryPosition(actorIndex, &cell, 0.0f, -0.1f, 0.0f);
                boardCellCenter(&cell, &actor->walkTarget);
            } else {
                actorTableQueryPosition(actorIndex, &cell, 0.0f, -0.1f, 0.0f);
                cell.cellGroupIndex += gDirectionCellStepX[actor->direction];
                cell.columnIndex += gDirectionCellStepZ[actor->direction];
                boardCellCenter(&cell, &actor->walkTarget);
            }
            actor->motionState = ACTOR_STATE_WALKING;
        } else if (grounded == 1 && actor->unk_242 > 0) {
            boardCellCenter(&floorCell, &actor->walkTarget);
            actor->motionState = ACTOR_STATE_WALKING;
            actor->unk_242 = 0;
            actor->direction = -1;
        }
        actorTableQueryPosition(actorIndex, &cell, 0.0f, -0.1f, 0.0f);
    } else {
        actor->motionState = ACTOR_STATE_IDLE;
    }
    actorTableNoopTick(&actor->position, &actor->walkTarget, 4);
    if (actor->motionState == ACTOR_STATE_WALKING) {
        angle = Get_Angle(&actor->position.vector, &actor->walkTarget.vector);
        if (grounded == 1) {
            actorTableQueryPosition(actorIndex, &standCell, 0.0f, 0.0f, 0.0f);
            func_00A85890(&actor->walkTarget.vector.x, &destinationCell);
            if ((standCell.cellGroupIndex != destinationCell.cellGroupIndex
                 || standCell.columnIndex != destinationCell.columnIndex)
                && actor->direction != -1 && func_00A858F0(actorIndex, actor->direction) == 0) {
                actor->motionState = ACTOR_STATE_IDLE;
            }
            if (actor->motionState == ACTOR_STATE_WALKING) {
                actorTableMoveEntry(actorIndex, sinf(angle) * gWalkSpeedByLevel[gScreenModeControlStorage.speedLevel], 0.0f,
                                    cosf(angle) * gWalkSpeedByLevel[gScreenModeControlStorage.speedLevel]);
                if (actor->direction != -1) {
                    actor->targetHeading = angle;
                }
                if (Get_Distance(&actor->position.vector, &actor->walkTarget.vector)
                    <= gWalkSpeedByLevel[gScreenModeControlStorage.speedLevel]) {
                    actor->motionState = ACTOR_STATE_IDLE;
                    if (actor->direction == -1) {
                        if (gDirectionFacingYaw[0] - 0.7853982f <= angle && angle <= gDirectionFacingYaw[0] + 0.7853982f) {
                            actor->direction = 0;
                        } else {
                            actor->direction =
                                (gDirectionFacingYaw[1] - 0.7853982f <= angle && angle <= gDirectionFacingYaw[1] + 0.7853982f) ? 1
                                : (gDirectionFacingYaw[3] - 0.7853982f <= angle && angle <= gDirectionFacingYaw[3] + 0.7853982f) ? 3
                                : 2;
                        }
                    }
                }
            }
        }
    }
    if (grounded == 1 && !(gScreenModeControlStorage.modeBits & 0x100)) {
        actorTableQueryPosition(actorIndex, &cell, 0.0f, -0.1f, 0.0f);
        if ((u16) cell.cellGroupIndex < 0x13 && (u16) cell.rowIndex < 8 && (u16) cell.columnIndex < 0x13) {
            cellOffset = ENTRY_CELL_BYTE_OFFSET(&cell);
            cellValue = ENTRY_CELL_AT_OFFSET(cellOffset);
            if (cellValue == 23 || cellValue == actor->id + 19) {
                do {
                    func_00A84238(actorIndex, &cell);
                    return 1;
                } while (0); /* kept: the do/while(0) decides the emitted branch layout */
            }
        }
    }
    if (grounded == 1 && floorCell.cellGroupIndex != -1 && floorCell.rowIndex != -1 && floorCell.columnIndex != -1) {
        floorCellOffset = ENTRY_CELL_BYTE_OFFSET(&floorCell);
        cellValue = ENTRY_CELL_AT_OFFSET(floorCellOffset);
        if (cellValue >= 2 && cellValue <= 5) {
            actor->timer = 0;
            actor->motionState = 2;
        }
    }
    if (grounded == 1) {
        actorTableStartChannelValue(actorIndex, gWalkChannelByLevel[gScreenModeControlStorage.speedLevel],
                                    gWalkChannelValueByLevel[gScreenModeControlStorage.speedLevel]);
    }
    return 1;
}

s32 func_00A8D5A8(s16 actorIndex);

void actorTableMoveEntry(s16 actorIndex, f32 deltaX, f32 deltaY, f32 deltaZ) {
    MovingActorSlot *actor;
    s16 actorTableIndex;

    actorTableIndex = actorIndex;
    actor = gMovingActorSlots + actorTableIndex;
    actor->position.vector.x += deltaX;
    if (func_00A8D5A8(actorTableIndex) != -1) {
        actor->position.vector.x -= deltaX;
        actor->velocity.x *= 0.75f;
    }
    actor->position.vector.y += deltaY;
    if (func_00A8D5A8(actorTableIndex) != -1) {
        actor->position.vector.y -= deltaY;
        actor->velocity.y *= 0.75f;
    }
    actor->position.vector.z += deltaZ;
    if (func_00A8D5A8(actorTableIndex) != -1) {
        actor->position.vector.z -= deltaZ;
        actor->velocity.z *= 0.75f;
    }
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A839C8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A83A78);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A83C20);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A83E60);

s16 actorTableFindLongestTimer(void);

void actorTableExpireLongest(void) {
    s32 longestIndex;

    longestIndex = actorTableFindLongestTimer();
    if (longestIndex != -1) {
        if (gMovingActorSlots[longestIndex].timer >= 0xF) {
            gMovingActorSlots[longestIndex].timer = 0;
            gMovingActorSlots[longestIndex].motionState = 4;
            func_00A9D718(-1, 3, 3);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A84238);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A844A0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A844E8);

s16 actorTableFindLongestTimer(void)
{
    MovingActorSlot *actor;
    s16 longestTimer;
    s16 longestIndex;
    s32 actorIndex;

    longestTimer = -1;
    actor = gMovingActorSlots;
    longestIndex = -1;
    for (actorIndex = 0; actorIndex < 4; actorIndex++) {
        if (actor->id != -1 && actor->motionState == 3) {
            if (longestTimer < actor->timer) {
                longestTimer = actor->timer;
                longestIndex = actorIndex;
            }
        }
        actor += 1;
    }
    return longestIndex;
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A847F8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A84870);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A84C90);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A85600);

void func_00A85890(f32 *position, EntryCoord *cell);
typedef struct EntryPosition {
    f32 x;
    f32 y;
    f32 z;
} EntryPosition;

void actorTableQueryPosition(s16 actorIndex, EntryCoord *cell, f32 deltaX, f32 deltaY, f32 deltaZ) {
    EntryPosition queryPosition;
    MovingActorSlot *actor;
    s16 actorTableIndex;

    actorTableIndex = actorIndex;
    actor = gMovingActorSlots + actorTableIndex;
    queryPosition.x = actor->position.vector.x + deltaX;
    queryPosition.y = actor->position.vector.y + deltaY;
    queryPosition.z = actor->position.vector.z + deltaZ;
    func_00A85890(&queryPosition.x, cell);
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

void actorTableNoopTick(TransformPosition *startPosition, TransformPosition *endPosition, s32 tickInterval) {

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

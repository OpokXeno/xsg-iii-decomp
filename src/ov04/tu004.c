/*
 * OV04 original TU 4: 0x00a870a8..0x00a8deb0 (40 functions)
 */
#include "common.h"
#include "main/tu007.h"

typedef signed char s8;
typedef int s32;

/* Coordinate of one entry in the 2-byte table at gEntryCellTable: group (0x98 per group), row (0x13 per row), column. */
typedef struct EntryCoord {
    s16 cellGroupIndex;
    s16 rowIndex;
    s16 columnIndex;
} EntryCoord;

/* A transform position: read as floats, copied as two doublewords. The integer view is kept because the
   original copy aliases int data (the selection slot's copy is ordered after the effect table read). */
typedef union TransformPosition {
    Vector4 vector;
    u64 words[2];
    s32 ivector[4];
} TransformPosition;

/* Screen mode control block at gScreenModeControlStorage (shared with tu005, tu006, tu016, tu020): reset values at 0xFE and 0x100, the cell value last seen by effectCoordToTableOffset at 0x114. */
typedef struct ScreenModeControl {
    s32 modeBits;             /* 0x000: SCREEN_MODE_* */
    s32 unk_04;
    s32 pulseTimer;            /* 0x008: frames of the cursor pulse */
    u8 unk_0C[0x1C];
    s16 liftTravel;            /* 0x028: cells a lift travels */
    u8 unk_2A[0xD4];
    s16 unk_FE;
    s16 unk_100;
    u8 unk_102[4];
    EntryCoord cursor;         /* 0x106 */
    EntryCoord target;         /* 0x10C: the placement target while SCREEN_MODE_PLACING is set */
    s16 targetDirection;       /* 0x112: TARGET_DIRECTION_* of the last target move, -1 none */
    s16 cachedCellEntry;       /* 0x114: also the selected entry of gScreenEffectTable */
    s16 lastMessage;           /* 0x116 */
    u8 unk_118[0xA18];
    TransformPosition cameraFocus; /* 0xB30 */
    u8 unk_B40[0x20];
    f32 cameraYaw;             /* 0xB60: radians */
    u8 unk_B64[0x1C];
    Vector4 boardOrigin;       /* 0xB80: world position of cell (0, 0, 0) */
    s16 placedKindAD;
    s16 placedKindC;
    s16 placedKindD;
    u8 unk_B96[0x16BC];
    s16 speedLevel;             /* 0x2252: speed level: index of the lift knock-back speed, slow lift sounds when > 0 */
    s16 liftState[4];          /* 0x2254: LIFT_STATE_* of each lane */
    s16 liftHeight[4];         /* 0x225C: cells each lane's lifts are raised */
    s16 liftTimer[4];          /* 0x2264: frames in the current lane state */
    s16 liftPad[4];            /* 0x226C: controller port of each lane */
    u8 unk_2274[0x16];
    u16 demoButtons;           /* 0x228A: held buttons of both pads while flag 0x10 is set */
    u8 unk_228C[0x18E4];
    f32 textX[64];             /* 0x3B70 */
    f32 textWidth[64];         /* 0x3C70 */
} ScreenModeControl;

/* One 0xC0-byte transform of the pool; the fields 0x30..0x4C are reset by resetTransformPool. */

typedef struct TransformSlot {
    s16 kind;                  /* 0x00: model kind, -1 hidden */
    u8 unk_02[4];
    EntryCoord coord;          /* 0x06 */
    u8 unk_0C[4];
    TransformPosition position; /* 0x10 */
    TransformPosition destination; /* 0x20 */
    Vector4 rotation;          /* 0x30: radians, w = 1 */
    TransformPosition scale;   /* 0x40 */
    u8 unk_50[0x70];
} TransformSlot;

/* The transform pool at the pointer stored in gTransformPool: seven fixed slots (cursor, selection, placement
   target, ...), then 270 general transforms. */
typedef struct TransformPool {
    TransformSlot slots[277];
} TransformPool;

#define TRANSFORM_POOL_FIRST_GENERAL 7

extern s16 *gEntryCellTable;
extern ScreenModeControl gScreenModeControlStorage[];
extern struct PadRecord D_4B3470[2]; /* both controller ports */

#define ENTRY_CELL(group, row, column) gEntryCellTable[(row) * 0x13 + (group) * 0x98 + (column)]
#define ENTRY_CELL_AT(group, row, column) (gEntryCellTable + ((row) * 0x13 + (group) * 0x98 + (column)))

/* Byte offset of a coordinate's cell in gEntryCellTable, and the cell at a byte offset. */
#define ENTRY_CELL_BYTE_OFFSET(coord) (((coord)->rowIndex * 0x13 + (coord)->cellGroupIndex * 0x98 + (coord)->columnIndex) * 2)
#define ENTRY_CELL_AT_OFFSET(tableByteDistance) (*(s16 *) ((u8 *) gEntryCellTable + (tableByteDistance)))

/* Model record of a moving actor; only the word cleared when the actor is crushed is known. */
typedef struct ActorModel {
    u8 unk_00[0x574];
    f32 unk_574;
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

typedef struct MovingActorTable { MovingActorSlot *slots; } MovingActorTable;
extern MovingActorTable gMovingActorSlots;

s32 func_00A8A120(EntryCoord *coord, s16 cellValue);
s32 func_00A8A1F8(EntryCoord *coord);
s32 func_00A8A2C0(EntryCoord *coord, s16 cellValue);
s32 func_00A8A3F0(EntryCoord *coord, s16 cellValue);
s32 func_00A8AC60(void);
s32 func_00A8A500(EntryCoord *coord, s16 cellValue);
void func_00A877B0(EntryCoord *coord);
s32 func_00A8DA40(s16 actorIndex, EntryCoord *coord);
void func_00A83A78(s16 actorIndex, s16 targetIndex);
void func_00A8A0A8(s16 message);
s32 func_00A874F0(EntryCoord *coord);

/* Message ids passed to func_00A8A0A8 (board editor messages), named after the condition that raises them. */
#define MESSAGE_LIMIT_KIND_A_AND_D 0x21F
#define MESSAGE_LIMIT_KIND_C 0x220
#define MESSAGE_LIMIT_KIND_D 0x221
#define MESSAGE_NO_FREE_LIFT 0x222
#define MESSAGE_CURSOR_OUT_OF_BOARD 0x223
#define MESSAGE_TARGET_OUT_OF_BOARD 0x224
#define MESSAGE_TARGET_TOO_FAR 0x225
#define MESSAGE_TARGET_OCCUPIED 0x226
#define MESSAGE_TARGET_ABOVE_KIND_C 0x227
#define MESSAGE_TARGET_ABOVE_KIND_D 0x228
#define MESSAGE_TARGET_AXIS_LOCKED 0x229
#define MESSAGE_COUNTER_A_AT_MAXIMUM 0x22A
#define MESSAGE_COUNTER_A_AT_MINIMUM 0x22B
#define MESSAGE_COUNTER_B_AT_MAXIMUM 0x22C
#define MESSAGE_COUNTER_B_AT_MINIMUM 0x22D
#define MESSAGE_LOCKED_CELL_PLACE 0x230
#define MESSAGE_LOCKED_CELL_COPY 0x231
#define MESSAGE_LOCKED_CELL_ERASE 0x232

/* MovingActorSlot.motionState values whose meaning the code proves (see ov04/tu002). */
#define ACTOR_STATE_IDLE 0
#define ACTOR_STATE_WALKING 1
#define ACTOR_STATE_CRUSHED 9

/* Refuses a placement: shows the message and restarts the pad repeat delay. */
#define REFUSE_PLACEMENT(message)                              \
    do {                                                       \
        func_00A8A0A8(message);                                \
        D_4B3470[0].unmodeled_4e = -D_4B3470[0].repeatDelay;         \
        return 0;                                              \
    } while (0)

#define CELL_IS_KIND_A(cell) ((u32) ((cell) - 1) < 5)
#define CELL_IS_KIND_B(cell) ((u32) ((cell) - 6) < 8)
#define CELL_IS_KIND_C(cell) ((u32) ((cell) - 14) < 5)
#define CELL_IS_KIND_D(cell) ((u32) ((cell) - 19) < 5)

/* Places cellValue on the board cell at coord. Returns 1 when placed, 0 when refused (a refusal shows its message). */
s32 boardPlaceCell(EntryCoord *coord, s16 cellValue) {
    s32 cellByteOffset = ENTRY_CELL_BYTE_OFFSET(coord);
    s16 currentCell = ENTRY_CELL_AT_OFFSET(cellByteOffset);
    s32 actorIndex;

    if (currentCell == cellValue) {
        return 0;
    }
    if (func_00A8A120(coord, cellValue) == 0) {
        return 0;
    }
    if (func_00A8A1F8(coord) == 0) {
        return 0;
    }
    if (func_00A8A2C0(coord, cellValue) == 0) {
        return 0;
    }
    if (func_00A8A3F0(coord, cellValue) == 0) {
        return 0;
    }
    if (!CELL_IS_KIND_B(currentCell) && CELL_IS_KIND_B(cellValue) && func_00A8AC60() == -1) {
        REFUSE_PLACEMENT(MESSAGE_NO_FREE_LIFT);
    }
    if (!CELL_IS_KIND_A(currentCell) && !CELL_IS_KIND_D(currentCell) && (CELL_IS_KIND_A(cellValue) || CELL_IS_KIND_D(cellValue))
        && gScreenModeControlStorage->placedKindAD >= 150) {
        REFUSE_PLACEMENT(MESSAGE_LIMIT_KIND_A_AND_D);
    }
    if (!CELL_IS_KIND_C(currentCell) && CELL_IS_KIND_C(cellValue) && gScreenModeControlStorage->placedKindC >= 4) {
        REFUSE_PLACEMENT(MESSAGE_LIMIT_KIND_C);
    }
    if (!CELL_IS_KIND_D(currentCell) && CELL_IS_KIND_D(cellValue) && gScreenModeControlStorage->placedKindD >= 20) {
        REFUSE_PLACEMENT(MESSAGE_LIMIT_KIND_D);
    }
    func_00A874F0(coord);
    if (CELL_IS_KIND_B(cellValue) && func_00A8A500(coord, cellValue) == -1) {
        return 1;
    }
    if (CELL_IS_KIND_A(cellValue)) {
        if (gScreenModeControlStorage->placedKindAD >= 150) {
            REFUSE_PLACEMENT(MESSAGE_LIMIT_KIND_A_AND_D);
        }
        {
            s32 cellOffset = ENTRY_CELL_BYTE_OFFSET(coord);

            ENTRY_CELL_AT_OFFSET(cellOffset) = cellValue;
        }
        gScreenModeControlStorage->placedKindAD++;
    }
    if (CELL_IS_KIND_C(cellValue)) {
        if (gScreenModeControlStorage->placedKindC >= 4) {
            REFUSE_PLACEMENT(MESSAGE_LIMIT_KIND_C);
        }
        {
            s32 cellOffset = ENTRY_CELL_BYTE_OFFSET(coord);

            ENTRY_CELL_AT_OFFSET(cellOffset) = cellValue;
        }
        gScreenModeControlStorage->placedKindC++;
    }
    if (CELL_IS_KIND_D(cellValue)) {
        if (gScreenModeControlStorage->placedKindAD >= 150) {
            REFUSE_PLACEMENT(MESSAGE_LIMIT_KIND_A_AND_D);
        }
        if (gScreenModeControlStorage->placedKindD >= 20) {
            REFUSE_PLACEMENT(MESSAGE_LIMIT_KIND_D);
        }
        {
            s32 cellOffset = ENTRY_CELL_BYTE_OFFSET(coord);

            ENTRY_CELL_AT_OFFSET(cellOffset) = cellValue;
        }
        gScreenModeControlStorage->placedKindAD++;
        gScreenModeControlStorage->placedKindD++;
    }
    func_00A877B0(coord);
    for (actorIndex = 0; actorIndex < 4; actorIndex++) {
        if (gMovingActorSlots.slots[actorIndex].id != -1 && func_00A8DA40(actorIndex, coord) == 1) {
            func_00A83A78(actorIndex, actorIndex);
        }
    }
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A874F0);
void effectRestartFromTable();
s32 func_00A9D718(s16 actorIndex, s16 mode, s32 value);

s32 effectCoordToTableOffset(EntryCoord *coord)
{
  s16 *cellValueAt;
  s32 cached_value;
  s32 rowIndex;
  s32 columnIndex;
  s32 cellByteOffset;
  s32 cellValue;
  ScreenModeControl *screenControl;
  s32 coordEntryIndex;
  rowIndex = coord->rowIndex;
  screenControl = gScreenModeControlStorage;
  columnIndex = coord->columnIndex;
  cached_value = screenControl->cachedCellEntry;
  coordEntryIndex = ((rowIndex * 0x13) + ((coord->cellGroupIndex) * 0x98)) + columnIndex;
  cellByteOffset = coordEntryIndex * 2;
  cellValueAt = gEntryCellTable + coordEntryIndex;
  cellValue = (u16) (*cellValueAt);
  if (cached_value != (*cellValueAt))
  {
    screenControl->cachedCellEntry = cellValue;
    effectRestartFromTable();
    return func_00A9D718(-1, 0, 4);
  }
  return cellByteOffset;
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A877B0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A87930);

/* The world position of the centre of a board cell. */
void boardCellCenter(EntryCoord *coord, TransformPosition *position) {
    position->vector.x = gScreenModeControlStorage->boardOrigin.x + coord->cellGroupIndex + 0.5f;
    position->vector.y = gScreenModeControlStorage->boardOrigin.y + coord->rowIndex + 0.5f;
    position->vector.z = gScreenModeControlStorage->boardOrigin.z + coord->columnIndex + 0.5f;
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A87A98);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A88190);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A881C0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A88550);

extern TransformPool *gTransformPool;

void resetTransformPool(void)
{
  ScreenModeControl *screenControl = gScreenModeControlStorage;
  float zeroScalar;
  int transformIndex;
  TransformSlot *transform;
  transform = &gTransformPool->slots[TRANSFORM_POOL_FIRST_GENERAL];
  screenControl->unk_100 = 0;
  screenControl->unk_FE = 3;
  for (transformIndex = 0; transformIndex < 270; transformIndex++)
  {
    zeroScalar = 0.0f;
    transform->scale.vector.z = zeroScalar;
    transform->scale.vector.x = (transform->scale.vector.y = zeroScalar);
    transform->scale.vector.w = 1.0f;
    transform->rotation.x = 0.0f;
    transform->rotation.z = 0.0f;
    transform->rotation.w = 1.0f;
    transform += 1;
  }

}

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A886B0);

#define PAD_L2 0x0001
#define PAD_R2 0x0002
#define PAD_L1 0x0004
#define PAD_R1 0x0008
#define PAD_TRIANGLE 0x0010
#define PAD_CIRCLE 0x0020
#define PAD_CROSS 0x0040
#define PAD_SQUARE 0x0080
#define PAD_SELECT 0x0100
#define PAD_R3 0x0400
#define PAD_START 0x0800
#define PAD_UP 0x1000
#define PAD_RIGHT 0x2000
#define PAD_DOWN 0x4000
#define PAD_LEFT 0x8000

#define SCREEN_MODE_DEMO_INPUT 0x10 /* the pads are replaced by demoButtons (script command) */
#define SCREEN_MODE_PLACING 0x01000000
#define SCREEN_MODE_LOCKED 0x02000000

/* The pad direction that moves the cursor along each axis, per camera quadrant (indices DIRECTION_KEY_*). */
#define DIRECTION_KEY_COLUMN_DOWN 0
#define DIRECTION_KEY_GROUP_UP 1
#define DIRECTION_KEY_COLUMN_UP 2
#define DIRECTION_KEY_GROUP_DOWN 3


/* Board counters kept after the cells of gEntryCellTable. */
#define BOARD_COUNTER_A 0xB7A
#define BOARD_COUNTER_B 0xB7B
#define BOARD_TITLE_TEXT 0xB90

typedef struct DirectionKeyTable {
    s16 keys[16];
} DirectionKeyTable;

/* One 0x1C-byte entry of the screen effect table. */
typedef struct ScreenEffectEntry {
    u16 modelKind;             /* 0x00 */
    s32 angle;                 /* 0x04: degrees */
    s32 unk_08;
    s32 enabled;               /* 0x0C */
    u8 unk_10[0xC];
} ScreenEffectEntry;

extern DirectionKeyTable gCursorDirectionKeys;
extern struct PadRecord D_4B3470[2]; /* both controller ports */
extern ScreenEffectEntry gBoardCellKindTable[25];
extern char gBoardStatusText[];
extern char gBoardCursorText[];
extern s16 gBoardStatusDigitOffsets[6];
extern s16 gBoardCursorDigitOffsets[3];
extern s8 gMasterCodeActive;
extern s32 D_00AADE14;

s32 boardPlaceCell(EntryCoord *coord, s16 cellValue);
s32 func_00A874F0(EntryCoord *coord);
void boardCellCenter(EntryCoord *coord, TransformPosition *position);
void func_00A8A0A8(s16 message);
void func_00A8A678(EntryCoord *cursor, EntryCoord *target);
s16 func_00A8ACA8(void);
s16 func_00A8AD00(EntryCoord *coord);
void ov04CallEntryRoutine(s32 routine);
void func_00A99A30(s16 group, s16 column, f32 scale);
void func_00A94E78(void);
void func_00A95278(void);
void func_00A97170(s32 mode);
s32 func_00AA2D28(char *text, s32 font, s32 x, s32 y, s32 width, s32 height, s32 unk6, s32 unk7, f32 scale, s32 unk9,
                  s32 color);
s32 func_00AA3428(s16 font);
f32 sinf(f32 angle);
f32 floatApproach(f32 current, f32 target, f32 speed);

void boardEditorUpdate(void)
{
    EntryCoord savedTarget;
    DirectionKeyTable directionKeys = gCursorDirectionKeys;
    TransformSlot *cursorSlot;
    TransformSlot *targetMarker;
    s16 cameraDegrees;
    s32 heading;
    s32 quadrant;
    s32 cursorMoved;
    s32 groupFirstTransform;
    s32 transformIndex;
    s32 cursorCell;
    f32 pulse;
    double groupDistance;
    double rowDistance;
    double columnDistance;

    cursorMoved = 0;
    cursorSlot = gTransformPool->slots;
    targetMarker = &cursorSlot[2];
    if (gScreenModeControlStorage->modeBits & SCREEN_MODE_PLACING) {
        if (!(D_4B3470[0].held & PAD_SQUARE)) {
            func_00A8A678(&gScreenModeControlStorage->cursor, &gScreenModeControlStorage->target);
            gScreenModeControlStorage->modeBits &= ~SCREEN_MODE_PLACING;
        }
    } else if (D_4B3470[0].held & PAD_SQUARE) {
        s16 *cellAt = ENTRY_CELL_AT(gScreenModeControlStorage->cursor.cellGroupIndex, gScreenModeControlStorage->cursor.rowIndex,
                                     gScreenModeControlStorage->cursor.columnIndex);
        cursorCell = *cellAt;
        if (cursorCell >= 6) {
            if (cursorCell < 10) {
                gScreenModeControlStorage->modeBits |= SCREEN_MODE_PLACING;
                gScreenModeControlStorage->target = gScreenModeControlStorage->cursor;
                gScreenModeControlStorage->targetDirection = -1;
                groupFirstTransform = func_00A8AD00(&gScreenModeControlStorage->cursor) * 6;
                for (transformIndex = groupFirstTransform + 0xB6; transformIndex < groupFirstTransform + 0xBB; transformIndex++) {
                    if ((gTransformPool->slots + transformIndex)->kind != -1) {
                        func_00A874F0(&(gTransformPool->slots + transformIndex)->coord);
                    }
                    (gTransformPool->slots + transformIndex)->kind = -1;
                }
                ov04CallEntryRoutine(0x1FF);
            }
        }
    }
    if (!(D_4B3470[0].heldAndPressed & (PAD_TRIANGLE | PAD_CIRCLE | PAD_CROSS))) {
        gScreenModeControlStorage->modeBits &= ~SCREEN_MODE_LOCKED;
    }

    cameraDegrees = gScreenModeControlStorage->cameraYaw / 3.1415927f * 180.0f;
    while (cameraDegrees < 0) {
        cameraDegrees += 360;
    }
    heading = cameraDegrees % 360;
    do {
        if (heading >= 45 && heading < 135) {
            quadrant = 4;
            break;
        }
        if (heading >= 135 && heading < 225) {
            quadrant = 8;
            break;
        }
        if (heading >= 225 && heading < 315) {
            quadrant = 12;
            break;
        }
        quadrant = 0;
    } while (0);

    if (!(D_4B3470[0].heldAndPressed & (PAD_SELECT | PAD_R3)) && !(gScreenModeControlStorage->modeBits & SCREEN_MODE_PLACING)) {
        if (D_4B3470[0].repeated & directionKeys.keys[quadrant + DIRECTION_KEY_GROUP_DOWN]) {
            cursorMoved = 1;
            gScreenModeControlStorage->cursor.cellGroupIndex--;
        }
        if (D_4B3470[0].repeated & directionKeys.keys[quadrant + DIRECTION_KEY_GROUP_UP]) {
            cursorMoved = 1;
            gScreenModeControlStorage->cursor.cellGroupIndex++;
        }
        if (D_4B3470[0].repeated & PAD_L1) {
            cursorMoved = 1;
            gScreenModeControlStorage->cursor.rowIndex++;
        }
        if (D_4B3470[0].repeated & PAD_L2) {
            cursorMoved = 1;
            gScreenModeControlStorage->cursor.rowIndex--;
        }
        if (D_4B3470[0].repeated & directionKeys.keys[quadrant + DIRECTION_KEY_COLUMN_DOWN]) {
            cursorMoved = 1;
            gScreenModeControlStorage->cursor.columnIndex--;
        }
        if (D_4B3470[0].repeated & directionKeys.keys[quadrant + DIRECTION_KEY_COLUMN_UP]) {
            cursorMoved = 1;
            gScreenModeControlStorage->cursor.columnIndex++;
        }
        if (cursorMoved == 1) {
            boardCellCenter(&gScreenModeControlStorage->cursor, &gScreenModeControlStorage->cameraFocus);
            func_00A9D718(-1, 3, 1);
            if (gScreenModeControlStorage->cursor.rowIndex < 2) {
                func_00A99A30(gScreenModeControlStorage->cursor.cellGroupIndex + 1,
                              gScreenModeControlStorage->cursor.columnIndex + 1, 0.5f);
            }
        }
    }

    if (gScreenModeControlStorage->modeBits & SCREEN_MODE_PLACING) {
        if (gScreenModeControlStorage->cursor.cellGroupIndex == gScreenModeControlStorage->target.cellGroupIndex
            && gScreenModeControlStorage->cursor.rowIndex == gScreenModeControlStorage->target.rowIndex
            && gScreenModeControlStorage->cursor.columnIndex == gScreenModeControlStorage->target.columnIndex) {
            gScreenModeControlStorage->targetDirection = -1;
        }
        savedTarget = gScreenModeControlStorage->target;
        if (gScreenModeControlStorage->targetDirection == -1 || gScreenModeControlStorage->targetDirection == 1
            || gScreenModeControlStorage->targetDirection == 3) {
            if ((D_4B3470[0].repeated & directionKeys.keys[quadrant + DIRECTION_KEY_COLUMN_DOWN])
                || (D_4B3470[0].repeated & directionKeys.keys[quadrant + DIRECTION_KEY_COLUMN_UP])
                || (D_4B3470[0].repeated & PAD_L1) || (D_4B3470[0].repeated & PAD_L2)) {
                if (gScreenModeControlStorage->targetDirection != -1) {
                    func_00A8A0A8(MESSAGE_TARGET_AXIS_LOCKED);
                }
            }
            if (D_4B3470[0].repeated & directionKeys.keys[quadrant + DIRECTION_KEY_GROUP_DOWN]) {
                gScreenModeControlStorage->targetDirection = 3;
                gScreenModeControlStorage->target.cellGroupIndex--;
            }
            if (D_4B3470[0].repeated & directionKeys.keys[quadrant + DIRECTION_KEY_GROUP_UP]) {
                gScreenModeControlStorage->targetDirection = 1;
                gScreenModeControlStorage->target.cellGroupIndex++;
            }
        }
        if (gScreenModeControlStorage->targetDirection == -1 || gScreenModeControlStorage->targetDirection == 4
            || gScreenModeControlStorage->targetDirection == 5) {
            if ((D_4B3470[0].repeated & directionKeys.keys[quadrant + DIRECTION_KEY_GROUP_DOWN])
                || (D_4B3470[0].repeated & directionKeys.keys[quadrant + DIRECTION_KEY_GROUP_UP])
                || (D_4B3470[0].repeated & directionKeys.keys[quadrant + DIRECTION_KEY_COLUMN_DOWN])) {
                if (gScreenModeControlStorage->targetDirection != -1) {
                    func_00A8A0A8(MESSAGE_TARGET_AXIS_LOCKED);
                }
            }
            if (D_4B3470[0].repeated & PAD_L1) {
                gScreenModeControlStorage->targetDirection = 5;
                gScreenModeControlStorage->target.rowIndex++;
            }
            if (D_4B3470[0].repeated & PAD_L2) {
                gScreenModeControlStorage->targetDirection = 4;
                gScreenModeControlStorage->target.rowIndex--;
            }
        }
        if (gScreenModeControlStorage->targetDirection == -1 || gScreenModeControlStorage->targetDirection == 0
            || gScreenModeControlStorage->targetDirection == 2) {
            if ((D_4B3470[0].repeated & directionKeys.keys[quadrant + DIRECTION_KEY_GROUP_DOWN])
                || (D_4B3470[0].repeated & directionKeys.keys[quadrant + DIRECTION_KEY_GROUP_UP])
                || (D_4B3470[0].repeated & PAD_L1) || (D_4B3470[0].repeated & PAD_L2)) {
                if (gScreenModeControlStorage->targetDirection != -1) {
                    func_00A8A0A8(MESSAGE_TARGET_AXIS_LOCKED);
                }
            }
            if (D_4B3470[0].repeated & directionKeys.keys[quadrant + DIRECTION_KEY_COLUMN_DOWN]) {
                gScreenModeControlStorage->targetDirection = 0;
                gScreenModeControlStorage->target.columnIndex--;
            }
            if (D_4B3470[0].repeated & directionKeys.keys[quadrant + DIRECTION_KEY_COLUMN_UP]) {
                gScreenModeControlStorage->targetDirection = 2;
                gScreenModeControlStorage->target.columnIndex++;
            }
        }
        if ((savedTarget.cellGroupIndex != gScreenModeControlStorage->target.cellGroupIndex
             || savedTarget.rowIndex != gScreenModeControlStorage->target.rowIndex
             || savedTarget.columnIndex != gScreenModeControlStorage->target.columnIndex)
            && (gScreenModeControlStorage->cursor.cellGroupIndex != gScreenModeControlStorage->target.cellGroupIndex
                || gScreenModeControlStorage->cursor.rowIndex != gScreenModeControlStorage->target.rowIndex
                || gScreenModeControlStorage->cursor.columnIndex != gScreenModeControlStorage->target.columnIndex)) {
            s16 *cellAt = ENTRY_CELL_AT(gScreenModeControlStorage->target.cellGroupIndex, gScreenModeControlStorage->target.rowIndex,
                                         gScreenModeControlStorage->target.columnIndex);
            if (*cellAt != 0) {
                func_00A8A0A8(MESSAGE_TARGET_OCCUPIED);
                gScreenModeControlStorage->target = savedTarget;
            }
            if (gScreenModeControlStorage->target.rowIndex - 1 >= 0
                && (u32) ((gEntryCellTable + (gScreenModeControlStorage->target.cellGroupIndex * 0x98 + gScreenModeControlStorage->target.rowIndex * 0x13 + gScreenModeControlStorage->target.columnIndex))[-1 * 0x13] - 14) < 5) {
                func_00A8A0A8(MESSAGE_TARGET_ABOVE_KIND_C);
                gScreenModeControlStorage->target = savedTarget;
            }
            if (gScreenModeControlStorage->target.rowIndex - 2 >= 0
                && (u32) ((gEntryCellTable + (gScreenModeControlStorage->target.cellGroupIndex * 0x98 + gScreenModeControlStorage->target.rowIndex * 0x13 + gScreenModeControlStorage->target.columnIndex))[-2 * 0x13] - 14) < 5) {
                func_00A8A0A8(MESSAGE_TARGET_ABOVE_KIND_C);
                gScreenModeControlStorage->target = savedTarget;
            }
            if (gScreenModeControlStorage->target.rowIndex - 1 >= 0
                && (u32) ((gEntryCellTable + (gScreenModeControlStorage->target.cellGroupIndex * 0x98 + gScreenModeControlStorage->target.rowIndex * 0x13 + gScreenModeControlStorage->target.columnIndex))[-1 * 0x13] - 19) < 5) {
                func_00A8A0A8(MESSAGE_TARGET_ABOVE_KIND_D);
                gScreenModeControlStorage->target = savedTarget;
            }
            if (gScreenModeControlStorage->target.rowIndex - 2 >= 0
                && (u32) ((gEntryCellTable + (gScreenModeControlStorage->target.cellGroupIndex * 0x98 + gScreenModeControlStorage->target.rowIndex * 0x13 + gScreenModeControlStorage->target.columnIndex))[-2 * 0x13] - 19) < 5) {
                func_00A8A0A8(MESSAGE_TARGET_ABOVE_KIND_D);
                gScreenModeControlStorage->target = savedTarget;
            }
            groupDistance = gScreenModeControlStorage->cursor.cellGroupIndex - gScreenModeControlStorage->target.cellGroupIndex;
            if (groupDistance < 0) {
                groupDistance = -groupDistance;
            }
            rowDistance = gScreenModeControlStorage->cursor.rowIndex - gScreenModeControlStorage->target.rowIndex;
            if (rowDistance < 0) {
                rowDistance = -rowDistance;
            }
            groupDistance += rowDistance;
            columnDistance = gScreenModeControlStorage->cursor.columnIndex - gScreenModeControlStorage->target.columnIndex;
            if (columnDistance < 0) {
                columnDistance = -columnDistance;
            }
            if (groupDistance + columnDistance > 5.0) {
                gScreenModeControlStorage->target = savedTarget;
                func_00A8A0A8(MESSAGE_TARGET_TOO_FAR);
            }
        }
    }

    if (gScreenModeControlStorage->cursor.cellGroupIndex <= 0) {
        func_00A8A0A8(MESSAGE_CURSOR_OUT_OF_BOARD);
        gScreenModeControlStorage->cursor.cellGroupIndex = 1;
        D_4B3470[0].unmodeled_4e = -D_4B3470[0].repeatDelay;
    }
    if (gScreenModeControlStorage->cursor.rowIndex <= 0) {
        func_00A8A0A8(MESSAGE_CURSOR_OUT_OF_BOARD);
        gScreenModeControlStorage->cursor.rowIndex = 1;
        D_4B3470[0].unmodeled_4e = -D_4B3470[0].repeatDelay;
    }
    if (gScreenModeControlStorage->cursor.columnIndex <= 0) {
        func_00A8A0A8(MESSAGE_CURSOR_OUT_OF_BOARD);
        gScreenModeControlStorage->cursor.columnIndex = 1;
        D_4B3470[0].unmodeled_4e = -D_4B3470[0].repeatDelay;
    }
    if (gScreenModeControlStorage->cursor.cellGroupIndex >= 18) {
        func_00A8A0A8(MESSAGE_CURSOR_OUT_OF_BOARD);
        gScreenModeControlStorage->cursor.cellGroupIndex = 17;
        D_4B3470[0].unmodeled_4e = -D_4B3470[0].repeatDelay;
    }
    if (gScreenModeControlStorage->cursor.rowIndex >= 8) {
        func_00A8A0A8(MESSAGE_CURSOR_OUT_OF_BOARD);
        gScreenModeControlStorage->cursor.rowIndex = 7;
        D_4B3470[0].unmodeled_4e = -D_4B3470[0].repeatDelay;
    }
    if (gScreenModeControlStorage->cursor.columnIndex >= 18) {
        func_00A8A0A8(MESSAGE_CURSOR_OUT_OF_BOARD);
        gScreenModeControlStorage->cursor.columnIndex = 17;
        D_4B3470[0].unmodeled_4e = -D_4B3470[0].repeatDelay;
    }
    if (gScreenModeControlStorage->target.cellGroupIndex <= 0) {
        func_00A8A0A8(MESSAGE_TARGET_OUT_OF_BOARD);
        gScreenModeControlStorage->target.cellGroupIndex = 1;
        D_4B3470[0].unmodeled_4e = -D_4B3470[0].repeatDelay;
    }
    if (gScreenModeControlStorage->target.rowIndex <= 0) {
        func_00A8A0A8(MESSAGE_TARGET_OUT_OF_BOARD);
        gScreenModeControlStorage->target.rowIndex = 1;
        D_4B3470[0].unmodeled_4e = -D_4B3470[0].repeatDelay;
    }
    if (gScreenModeControlStorage->target.columnIndex <= 0) {
        func_00A8A0A8(MESSAGE_TARGET_OUT_OF_BOARD);
        gScreenModeControlStorage->target.columnIndex = 1;
        D_4B3470[0].unmodeled_4e = -D_4B3470[0].repeatDelay;
    }
    if (gScreenModeControlStorage->target.cellGroupIndex >= 18) {
        func_00A8A0A8(MESSAGE_TARGET_OUT_OF_BOARD);
        gScreenModeControlStorage->target.cellGroupIndex = 17;
        D_4B3470[0].unmodeled_4e = -D_4B3470[0].repeatDelay;
    }
    if (gScreenModeControlStorage->target.rowIndex >= 8) {
        func_00A8A0A8(MESSAGE_TARGET_OUT_OF_BOARD);
        gScreenModeControlStorage->target.rowIndex = 7;
        D_4B3470[0].unmodeled_4e = -D_4B3470[0].repeatDelay;
    }
    if (gScreenModeControlStorage->target.columnIndex >= 18) {
        func_00A8A0A8(MESSAGE_TARGET_OUT_OF_BOARD);
        gScreenModeControlStorage->target.columnIndex = 17;
        D_4B3470[0].unmodeled_4e = -D_4B3470[0].repeatDelay;
    }

    if (!(gScreenModeControlStorage->modeBits & SCREEN_MODE_LOCKED)) {
        s16 *cellAt = ENTRY_CELL_AT(gScreenModeControlStorage->cursor.cellGroupIndex, gScreenModeControlStorage->cursor.rowIndex,
                                     gScreenModeControlStorage->cursor.columnIndex);
        if (*cellAt != 0x18) {
            do {
                if (D_4B3470[0].held & PAD_CIRCLE) {
                    if (boardPlaceCell(&gScreenModeControlStorage->cursor, gScreenModeControlStorage->cachedCellEntry) == 1) {
                        func_00A9D718(-1, 3, 2);
                    }
                    break;
                }
                if (D_4B3470[0].held & PAD_TRIANGLE) {
                    effectCoordToTableOffset(&gScreenModeControlStorage->cursor);
                    break;
                }
                if (D_4B3470[0].held & PAD_CROSS) {
                    if (func_00A874F0(&gScreenModeControlStorage->cursor) == 1) {
                        func_00A9D718(-1, 3, 3);
                    }
                }
            } while (0);
        } else {
            do {
                if (D_4B3470[0].held & PAD_CIRCLE) {
                    func_00A8A0A8(MESSAGE_LOCKED_CELL_PLACE);
                    break;
                }
                if (D_4B3470[0].held & PAD_TRIANGLE) {
                    func_00A8A0A8(MESSAGE_LOCKED_CELL_COPY);
                    break;
                }
                if (D_4B3470[0].held & PAD_CROSS) {
                    func_00A8A0A8(MESSAGE_LOCKED_CELL_ERASE);
                }
            } while (0);
        }
    }

    if (D_4B3470[0].held & PAD_R3) {
        if (D_4B3470[0].repeated & PAD_R1) {
            if (++gEntryCellTable[BOARD_COUNTER_A] >= 100) {
                gEntryCellTable[BOARD_COUNTER_A] = 99;
                func_00A8A0A8(MESSAGE_COUNTER_A_AT_MAXIMUM);
            } else {
                ov04CallEntryRoutine(0x1F7);
            }
        }
        if (D_4B3470[0].repeated & PAD_R2) {
            if (--gEntryCellTable[BOARD_COUNTER_A] <= 0) {
                gEntryCellTable[BOARD_COUNTER_A] = 1;
                func_00A8A0A8(MESSAGE_COUNTER_A_AT_MINIMUM);
            } else {
                ov04CallEntryRoutine(0x1F8);
            }
        }
    }
    if (D_4B3470[0].held & PAD_R3) {
        if (D_4B3470[0].repeated & PAD_L1) {
            if (++gEntryCellTable[BOARD_COUNTER_B] >= 100) {
                gEntryCellTable[BOARD_COUNTER_B] = 99;
                func_00A8A0A8(MESSAGE_COUNTER_B_AT_MAXIMUM);
            } else {
                ov04CallEntryRoutine(0x1F9);
            }
        }
        if (D_4B3470[0].repeated & PAD_L2) {
            if (--gEntryCellTable[BOARD_COUNTER_B] <= 0) {
                gEntryCellTable[BOARD_COUNTER_B] = 1;
                func_00A8A0A8(MESSAGE_COUNTER_B_AT_MINIMUM);
            } else {
                ov04CallEntryRoutine(0x1FA);
            }
        }
    }

    if (!(D_4B3470[0].heldAndPressed & (PAD_SELECT | PAD_R3))) {
        if (D_4B3470[0].repeated & PAD_R1) {
            for (;;) {
                gScreenModeControlStorage->cachedCellEntry = (gScreenModeControlStorage->cachedCellEntry + 24) % 25;
                if ((gBoardCellKindTable + gScreenModeControlStorage->cachedCellEntry)->enabled == 1) {
                    break;
                }
            }
            effectRestartFromTable();
        }
        if (D_4B3470[0].repeated & PAD_R2) {
            do {
                gScreenModeControlStorage->cachedCellEntry = (gScreenModeControlStorage->cachedCellEntry + 1) % 25;
            } while ((gBoardCellKindTable + gScreenModeControlStorage->cachedCellEntry)->enabled != 1);
            effectRestartFromTable();
        }
    }

    cursorSlot->kind = 0x24;
    boardCellCenter(&gScreenModeControlStorage->cursor, &cursorSlot->destination);
    pulse = sinf((gScreenModeControlStorage->pulseTimer % 32) * 6.2831855f * 0.03125f) * 0.1f + 1.11f;
    cursorSlot->scale.vector.w = 1.0f;
    cursorSlot->scale.vector.x = cursorSlot->scale.vector.y = cursorSlot->scale.vector.z = pulse;
    cursorSlot->position.vector.x = floatApproach(cursorSlot->position.vector.x, cursorSlot->destination.vector.x, 2.0f);
    cursorSlot->position.vector.y = floatApproach(cursorSlot->position.vector.y, cursorSlot->destination.vector.y, 2.0f);
    cursorSlot->position.vector.z = floatApproach(cursorSlot->position.vector.z, cursorSlot->destination.vector.z, 2.0f);
    gScreenModeControlStorage->pulseTimer++;
    if (gMasterCodeActive == 1 && (D_00AADE14 & 0x100)) {
        cursorSlot->kind = -1;
    }

    if (gScreenModeControlStorage->modeBits & SCREEN_MODE_PLACING) {
        targetMarker->kind = 0x25;
        boardCellCenter(&gScreenModeControlStorage->target, &targetMarker->position);
    } else {
        targetMarker->kind = -1;
    }

    cursorSlot = &gTransformPool->slots[1];
    cursorSlot->kind = gBoardCellKindTable[gScreenModeControlStorage->cachedCellEntry].modelKind;
    boardCellCenter(&gScreenModeControlStorage->cursor, &cursorSlot->position);
    cursorSlot->position = gTransformPool->slots[0].position;
    cursorSlot->rotation.y = (gBoardCellKindTable + gScreenModeControlStorage->cachedCellEntry)->angle / 180.0f * 3.1415927f;
    cursorSlot->scale.vector.z = 0.5f;
    cursorSlot->scale.vector.y = 0.5f;
    cursorSlot->scale.vector.x = 0.5f;

    if (func_00AA3428(0x12) == -1) {
        func_00AA2D28(gBoardStatusText, 0x12, 4, 4, -1, -1, 0, 0, 2.0f, 2, 0x802F4F5F);
        func_00AA2D28(gBoardCursorText, 0x12, 4, 0x24, -1, -1, 0, 0, 2.0f, 2, 0x802F4F5F);
        transformIndex = func_00AA2D28((char *) &gEntryCellTable[BOARD_TITLE_TEXT], 0x12, 4, 0x188, -1, -1, 0, 0, 2.0f, 2,
                              0x802F4F5F);
        gScreenModeControlStorage->textX[transformIndex] = 504.0f - gScreenModeControlStorage->textWidth[transformIndex];
    } else {
        gBoardCursorText[gBoardCursorDigitOffsets[0]] = (gScreenModeControlStorage->cursor.cellGroupIndex) / 10 + '0';
        gBoardCursorText[gBoardCursorDigitOffsets[0] + 1] = (gScreenModeControlStorage->cursor.cellGroupIndex) % 10 + '0';
        gBoardCursorText[gBoardCursorDigitOffsets[1]] = (gScreenModeControlStorage->cursor.rowIndex) / 10 + '0';
        gBoardCursorText[gBoardCursorDigitOffsets[1] + 1] = (gScreenModeControlStorage->cursor.rowIndex) % 10 + '0';
        gBoardCursorText[gBoardCursorDigitOffsets[2]] = (gScreenModeControlStorage->cursor.columnIndex) / 10 + '0';
        gBoardCursorText[gBoardCursorDigitOffsets[2] + 1] = (gScreenModeControlStorage->cursor.columnIndex) % 10 + '0';
        gBoardStatusText[gBoardStatusDigitOffsets[0]] = (gEntryCellTable[BOARD_COUNTER_B]) / 10 + '0';
        gBoardStatusText[gBoardStatusDigitOffsets[0] + 1] = (gEntryCellTable[BOARD_COUNTER_B]) % 10 + '0';
        gBoardStatusText[gBoardStatusDigitOffsets[1]] = (gEntryCellTable[BOARD_COUNTER_A]) / 10 + '0';
        gBoardStatusText[gBoardStatusDigitOffsets[1] + 1] = (gEntryCellTable[BOARD_COUNTER_A]) % 10 + '0';
        gBoardStatusText[gBoardStatusDigitOffsets[2]] = (150 - gScreenModeControlStorage->placedKindAD) / 100 + '0';
        gBoardStatusText[gBoardStatusDigitOffsets[2] + 1] = (150 - gScreenModeControlStorage->placedKindAD) / 10 % 10 + '0';
        gBoardStatusText[gBoardStatusDigitOffsets[2] + 2] = (150 - gScreenModeControlStorage->placedKindAD) % 10 + '0';
        gBoardStatusText[gBoardStatusDigitOffsets[3]] = (20 - gScreenModeControlStorage->placedKindD) / 10 + '0';
        gBoardStatusText[gBoardStatusDigitOffsets[3] + 1] = (20 - gScreenModeControlStorage->placedKindD) % 10 + '0';
        gBoardStatusText[gBoardStatusDigitOffsets[4]] = '4' - gScreenModeControlStorage->placedKindC;
        gBoardStatusText[gBoardStatusDigitOffsets[5]] = (s16) (func_00A8ACA8() / 10) % 10 + '0';
        gBoardStatusText[gBoardStatusDigitOffsets[5] + 1] = func_00A8ACA8() % 10 + '0';
    }

    if (D_4B3470[0].pressed & PAD_START) {
        func_00A94E78();
        func_00A97170(0);
    } else if (D_4B3470[0].pressed & PAD_SELECT) {
        func_00A95278();
        func_00A97170(0);
    }
}

extern u16 gEffectIdTable[];
void func_00AA30D0(s16, s32, s32, s32, s32, s32, s32, s32, f32, s32, s32);
void func_00AA3380();

void effectRestartFromTable(void) {
    s16 effectId;

    func_00AA3380(0x15);
    effectId = gEffectIdTable[0] + 0x200;
    func_00AA30D0(effectId, 0x15, 0x7C, 0x24, -1, -1, 0, 0, 1.0f, 0, 0x602F4F5F);
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8A0A8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8A120);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8A1F8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8A2C0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8A3F0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8A500);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8A678);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8AA60);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8AC60);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8ACA8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8AD00);

/* The lifts: LIFT_COUNT moving platforms of the board. Lift j is drawn by transform slot LIFT_FIRST_SLOT + j; its
   cell in gEntryCellTable holds 6 + the lane that drives it, LIFT_TRAVEL(j) its travel in cells (-1 when the lift is
   absent) and LIFT_DIRECTION(j) its LIFT_DIRECTION_* axis. */
#define LIFT_COUNT 10
#define LIFT_LANE_COUNT 4
#define LIFT_FIRST_SLOT 157
typedef struct BoardT { s16 cells[0xB60]; struct { s16 unk[6]; s16 travel[10]; s16 direction[10]; } lifts; } BoardT;
/* The board cell of a lift (read through BoardT: this view decides the emitted address arithmetic). */
#define LIFT_CELL(coord) ((BoardT *)gEntryCellTable)->cells[(coord).rowIndex * 0x13 + (coord).cellGroupIndex * 0x98 + (coord).columnIndex]
#define LIFT_TRAVEL(j) ((BoardT *)gEntryCellTable)->lifts.travel[j]
#define LIFT_DIRECTION(j) ((BoardT *)gEntryCellTable)->lifts.direction[j]

/* LIFT_DIRECTION_* (the axis a lift pushes an actor along): -z, +x, +z, -x, -y, +y. */
#define LIFT_DIRECTION_NONE -1

/* Lane states (ScreenModeControl.liftState). */
#define LIFT_STATE_IDLE 0
#define LIFT_STATE_RISING 1
#define LIFT_STATE_LOWERING 2
#define LIFT_STATE_LOWERED 3
#define LIFT_STATE_RAISED 4
#define LIFT_STATE_HELD 5

extern s16 gDirectionCellStepX[6];      /* x step of each LIFT_DIRECTION_* */
extern s16 gDirectionCellStepY[6];      /* y step */
extern s16 gDirectionCellStepZ[6];      /* z step */
/* Actor yaw facing away from each horizontal LIFT_DIRECTION_*: one f32[4], which the scaffold labels entry by entry. */
extern f32 gDirectionFacingYaw[];
extern f32 D_00AA44BC[];
extern f32 D_00AA44C0[];
extern f32 D_00AA44C4[];

s32 Get_Rnd(s32 minimum, s32 maximum);
f32 cosf(f32 angle);
void func_00A99728(s16 lane, s16 mode);
void func_00A9DAC0(s16 actorIndex, s16 motion);
void actorTableStartChannelSelector(short actorIndex, short actorSelector);
void func_00A97D90(s16 effect, Vector4 *position, f32 x, f32 y, f32 z, f32 scale);
s32 func_00A8D5A8(s16 actorIndex);
void func_00A84870(s16 actorIndex);

/* Runs the lifts: each lane's button raises its lifts while held, then the lifts move their transforms one step and
   carry, push or knock back the actors on and around them. */
void boardLiftsUpdate(void) {
    EntryCoord coord;
    TransformPosition home;
    TransformPosition top;
    TransformPosition previous;
    TransformPosition delta;
    u16 held[2];
    u16 pressed[2];
    u16 laneButtons[LIFT_LANE_COUNT] = { PAD_CIRCLE, PAD_CROSS, PAD_TRIANGLE, PAD_SQUARE };
    f32 knockbackSpeeds[3] = { 0.2f, 0.3f, 0.4f };
    s32 laneIndex;
    s32 liftIndex;
    s32 actorIndex;
    s32 particleIndex;
    s32 divisor;
    s16 lane;
    s16 direction;
    s16 axis;
    TransformSlot *lift;
    MovingActorSlot *actor;
    ScreenModeControl *screenControl = gScreenModeControlStorage;

    if (screenControl->modeBits & SCREEN_MODE_DEMO_INPUT) {
        pressed[0] = pressed[1] = held[0] = held[1] = screenControl->demoButtons;
    } else {
        held[0] = D_4B3470[0].held;
        held[1] = D_4B3470[1].held;
        pressed[0] = D_4B3470[0].pressed;
        pressed[1] = D_4B3470[1].pressed;
    }
    for (laneIndex = 0; laneIndex < LIFT_LANE_COUNT; laneIndex++) {
        switch (gScreenModeControlStorage->liftState[laneIndex]) {
        case LIFT_STATE_IDLE:
            if (held[gScreenModeControlStorage->liftPad[laneIndex]] & laneButtons[laneIndex]) {
                gScreenModeControlStorage->liftState[laneIndex] = LIFT_STATE_RISING;
                gScreenModeControlStorage->liftHeight[laneIndex] = 1;
                for (liftIndex = 0; liftIndex < LIFT_COUNT; liftIndex++) {
                    if (LIFT_TRAVEL(liftIndex) > 0) {
                        lift = &gTransformPool->slots[LIFT_FIRST_SLOT + liftIndex];
                        coord = lift->coord;
                        lane = LIFT_CELL(coord) - 6;
                        if (lane == laneIndex) {
                            func_00A9D718(-1, 1, gScreenModeControlStorage->speedLevel > 0 ? liftIndex + 11 : liftIndex + 1);
                            func_00A99728(gScreenModeControlStorage->liftPad[lane], 1);
                        }
                    }
                }
            }
            break;
        case LIFT_STATE_RISING:
            if (gScreenModeControlStorage->liftHeight[laneIndex]++ >= gScreenModeControlStorage->liftTravel) {
                gScreenModeControlStorage->liftState[laneIndex] = LIFT_STATE_RAISED;
                gScreenModeControlStorage->liftHeight[laneIndex] = gScreenModeControlStorage->liftTravel;
                gScreenModeControlStorage->liftTimer[laneIndex] = 0;
            }
            break;
        case LIFT_STATE_LOWERING:
            if (gScreenModeControlStorage->liftHeight[laneIndex]-- <= 0) {
                gScreenModeControlStorage->liftState[laneIndex] = LIFT_STATE_LOWERED;
                gScreenModeControlStorage->liftHeight[laneIndex] = 0;
                gScreenModeControlStorage->liftTimer[laneIndex] = 0;
            }
            break;
        case LIFT_STATE_LOWERED:
            if (gScreenModeControlStorage->liftTimer[laneIndex]++ >= 5) {
                gScreenModeControlStorage->liftState[laneIndex] = LIFT_STATE_IDLE;
            }
            break;
        case LIFT_STATE_RAISED:
            if (gScreenModeControlStorage->liftTimer[laneIndex]++ >= 5) {
                gScreenModeControlStorage->liftState[laneIndex] = LIFT_STATE_HELD;
            }
            break;
        case LIFT_STATE_HELD:
            if (!(held[gScreenModeControlStorage->liftPad[laneIndex]] & laneButtons[laneIndex])) {
                gScreenModeControlStorage->liftState[laneIndex] = LIFT_STATE_LOWERING;
                gScreenModeControlStorage->liftTimer[laneIndex] = 0;
                for (liftIndex = 0; liftIndex < LIFT_COUNT; liftIndex++) {
                    if (LIFT_TRAVEL(liftIndex) > 0) {
                        lift = &gTransformPool->slots[LIFT_FIRST_SLOT + liftIndex];
                        coord = lift->coord;
                        lane = LIFT_CELL(coord) - 6;
                        if (lane == laneIndex) {
                            func_00A9D718(-1, 0, 8);
                        }
                    }
                }
            }
            break;
        }
        if (gScreenModeControlStorage->liftHeight[laneIndex] < 0) {
            gScreenModeControlStorage->liftHeight[laneIndex] = 0;
        }
        if (gScreenModeControlStorage->liftHeight[laneIndex] > gScreenModeControlStorage->liftTravel) {
            gScreenModeControlStorage->liftHeight[laneIndex] = gScreenModeControlStorage->liftTravel;
        }
    }

    for (liftIndex = 0; liftIndex < LIFT_COUNT; liftIndex++) {
        if (LIFT_TRAVEL(liftIndex) == -1) {
            continue;
        }
        lift = &gTransformPool->slots[LIFT_FIRST_SLOT + liftIndex];
        coord = lift->coord;
        lane = LIFT_CELL(coord) - 6;
        boardCellCenter(&coord, &home);
        top = home;
        axis = LIFT_DIRECTION(liftIndex);
        top.vector.x += gDirectionCellStepX[axis] * LIFT_TRAVEL(liftIndex);
        top.vector.y += gDirectionCellStepY[axis] * LIFT_TRAVEL(liftIndex);
        top.vector.z += gDirectionCellStepZ[axis] * LIFT_TRAVEL(liftIndex);
        previous = lift->position;
        /* Eases from the top (height 0) down to home (full travel). */
        lift->position.vector.x = top.vector.x + (home.vector.x - top.vector.x)
            * ((gScreenModeControlStorage->liftTravel - gScreenModeControlStorage->liftHeight[lane]) * (gScreenModeControlStorage->liftTravel - gScreenModeControlStorage->liftHeight[lane]))
            / (gScreenModeControlStorage->liftTravel * gScreenModeControlStorage->liftTravel);
        lift->position.vector.y = top.vector.y + (home.vector.y - top.vector.y)
            * ((gScreenModeControlStorage->liftTravel - gScreenModeControlStorage->liftHeight[lane]) * (gScreenModeControlStorage->liftTravel - gScreenModeControlStorage->liftHeight[lane]))
            / (gScreenModeControlStorage->liftTravel * gScreenModeControlStorage->liftTravel);
        lift->position.vector.z = top.vector.z + (home.vector.z - top.vector.z)
            * ((gScreenModeControlStorage->liftTravel - gScreenModeControlStorage->liftHeight[lane]) * (gScreenModeControlStorage->liftTravel - gScreenModeControlStorage->liftHeight[lane]))
            / (gScreenModeControlStorage->liftTravel * gScreenModeControlStorage->liftTravel);
        delta.vector.x = lift->position.vector.x - previous.vector.x;
        delta.vector.y = lift->position.vector.y - previous.vector.y;
        delta.vector.z = lift->position.vector.z - previous.vector.z;
        if (delta.vector.x != 0.0f || delta.vector.y != 0.0f || delta.vector.z != 0.0f) {
            if (lift->position.vector.y <= 2.0f) {
                func_00A99A30(lift->position.vector.x + 1.0f, lift->position.vector.z + 1.0f, 0.25f);
            }
            /* Carry the actors standing on this lift. */
            for (actorIndex = 0; actorIndex < 4; actorIndex++) {
                if (gMovingActorSlots.slots[actorIndex].id != -1 && gMovingActorSlots.slots[actorIndex].liftIndex == liftIndex) {
                    MovingActorSlot *rider = &gMovingActorSlots.slots[actorIndex];

                    rider->position.vector.x += delta.vector.x;
                    rider->position.vector.y += delta.vector.y;
                    rider->position.vector.z += delta.vector.z;
                    if (delta.vector.y == 0.0f) {
                        rider->walkTarget = lift->position;
                    }
                    {
                        float standY = gMovingActorSlots.slots[actorIndex].walkTarget.vector.y;
                        MovingActorSlot *raised = &gMovingActorSlots.slots[actorIndex];

                        raised->walkTarget.vector.y = standY + 0.35f;
                        raised->velocity.y += delta.vector.y * 0.35f;
                        if (raised->motionState == 2) {
                            raised->motionState = ACTOR_STATE_WALKING;
                        }
                    }
                    if (delta.vector.y > 0.0f) {
                        gMovingActorSlots.slots[actorIndex].liftIndex = -1;
                        gMovingActorSlots.slots[actorIndex].velocity.x = __builtin_sinf(gMovingActorSlots.slots[actorIndex].heading) * knockbackSpeeds[gScreenModeControlStorage->speedLevel];
                        gMovingActorSlots.slots[actorIndex].velocity.z = __builtin_cosf(gMovingActorSlots.slots[actorIndex].heading) * knockbackSpeeds[gScreenModeControlStorage->speedLevel];
                    }
                }
            }
        }
        direction = LIFT_DIRECTION_NONE;
        if (delta.vector.x < 0.0f) {
            direction = 3;
        }
        if (delta.vector.x > 0.0f) {
            direction = 1;
        }
        if (delta.vector.y < 0.0f) {
            direction = 4;
        }
        if (delta.vector.y > 0.0f) {
            direction = 5;
        }
        if (delta.vector.z < 0.0f) {
            direction = 0;
        }
        if (delta.vector.z > 0.0f) {
            direction = 2;
        }
        if (direction == LIFT_DIRECTION_NONE) {
            continue;
        }
        if (delta.vector.x == 0.0f && delta.vector.y == 0.0f && delta.vector.z == 0.0f) {
            continue;
        }
        /* Push the actors this lift runs into. */
        for (actorIndex = 0; actorIndex < 4; actorIndex++) {
            actor = &gMovingActorSlots.slots[actorIndex];
            if (actor->id == -1 || actor->liftIndex == liftIndex) {
                continue;
            }
            if (actor->position.vector.x + actor->extent.vector.x <= lift->position.vector.x - 0.5f
                || lift->position.vector.x + 0.5f <= actor->position.vector.x - actor->extent.vector.x
                || actor->position.vector.z + actor->extent.vector.z <= lift->position.vector.z - 0.5f
                || lift->position.vector.z + 0.5f <= actor->position.vector.z - actor->extent.vector.z
                || actor->position.vector.y + actor->extent.vector.y <= lift->position.vector.y - 0.5f
                || lift->position.vector.y + 0.5f <= actor->position.vector.y) {
                continue;
            }
            switch (direction) {
            case 0:
                actor->position.vector.z = lift->position.vector.z - 0.5f - actor->extent.vector.z * 1.01f;
                actor->velocity.z += delta.vector.z;
                actor->targetHeading = gDirectionFacingYaw[0];
                actor->direction = 0;
                break;
            case 1:
                actor->position.vector.x = lift->position.vector.x + 0.5f + actor->extent.vector.x * 1.01f;
                actor->velocity.x += delta.vector.x;
                actor->targetHeading = D_00AA44BC[0];
                actor->direction = 1;
                break;
            case 2:
                actor->position.vector.z = lift->position.vector.z + 0.5f + actor->extent.vector.z * 1.01f;
                actor->velocity.z += delta.vector.z;
                actor->targetHeading = D_00AA44C0[0];
                actor->direction = 2;
                break;
            case 3:
                actor->position.vector.x = lift->position.vector.x - 0.5f - actor->extent.vector.x * 1.01f;
                actor->velocity.x += delta.vector.x;
                actor->targetHeading = D_00AA44C4[0];
                actor->direction = 3;
                break;
            case 4:
                actor->position.vector.y = lift->position.vector.y - 0.5f - actor->extent.vector.y * 1.01f;
                actor->velocity.y += delta.vector.y * 0.5f;
                break;
            case 5:
                actor->position.vector.y = lift->position.vector.y + 0.5f + 0.01f;
                actor->velocity.y += delta.vector.y * 0.5f;
                if (actor->motionState == 2) {
                    actor->motionState = ACTOR_STATE_WALKING;
                }
                actor->velocity.x = sinf(actor->heading) * knockbackSpeeds[gScreenModeControlStorage->speedLevel];
                actor->velocity.z = cosf(actor->heading) * knockbackSpeeds[gScreenModeControlStorage->speedLevel];
                actor->targetHeading = actor->heading;
                func_00A9D718(actorIndex, 0, 4);
                func_00A9DAC0(actorIndex, Get_Rnd(0, 1) == 0 ? 6 : 8);
                actorTableStartChannelSelector(actorIndex, 7);
                break;
            }
            if (direction >= 0 && direction < 5) {
                func_00A9D718(actorIndex, 0, 5);
                func_00A9DAC0(actorIndex, Get_Rnd(8, 9));
                actorTableStartChannelSelector(actorIndex, 4);
                for (particleIndex = 0; particleIndex < 3; particleIndex++) {
                    f32 x;
                    f32 z;

                    divisor = Get_Rnd(1, 5);
                    x = sinf(gDirectionFacingYaw[direction] + Get_Rnd(-500, 500) / 1000.0f) / divisor;
                    divisor = Get_Rnd(1, 5);
                    z = cosf(gDirectionFacingYaw[direction] + Get_Rnd(-500, 500) / 1000.0f) / divisor;

                    func_00A97D90(gMovingActorSlots.slots[actorIndex].id + 39, &actor->position.vector, x, 0.25f, z, 0.5f);
                }
            }
            if (actor->motionState == ACTOR_STATE_WALKING) {
                actor->motionState = ACTOR_STATE_IDLE;
            }
            if (func_00A8D5A8(actorIndex) == -1) {
                continue;
            }
            func_00A84870(actorIndex);
            if (func_00A8D5A8(actorIndex) == -1) {
                continue;
            }
            func_00A9D718(actorIndex, 0, 3);
            func_00A9DAC0(actorIndex, 10);
            actor->motionState = ACTOR_STATE_CRUSHED;
            actor->extent.vector.x = actor->extent.vector.z = 0.0f;
            actor->model->unk_574 = 0.0f;
            actor->timer = 0;
            if (direction == 0 || direction == 2) {
                actor->unk_58 = 0.0f;
            }
            if (direction == 1 || direction == 3) {
                actor->unk_50 = 0;
            }
            if (direction == 4 || direction == 5) {
                actor->unk_54 = 0;
            }
            switch (direction) {
            case 0:
                actor->position.vector.z = lift->position.vector.z - 0.5f;
                actor->velocity.z += delta.vector.z;
                break;
            case 1:
                actor->position.vector.x = lift->position.vector.x + 0.5f;
                actor->velocity.x += delta.vector.x;
                break;
            case 2:
                actor->position.vector.z = lift->position.vector.z + 0.5f;
                actor->velocity.z += delta.vector.z;
                break;
            case 3:
                actor->position.vector.x = lift->position.vector.x - 0.5f;
                actor->velocity.x += delta.vector.x;
                break;
            case 4:
                actor->position.vector.y = lift->position.vector.y - 0.5f;
                actor->velocity.y += delta.vector.y * 0.5f;
                break;
            case 5:
                actor->position.vector.y = lift->position.vector.y + 0.5f;
                actor->velocity.y += delta.vector.y * 0.5f;
                break;
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8BEA0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8C168);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8C1D8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8C408);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8C7A8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8CB18);

extern s32 gEffectRecordBase;

/* Byte pair at 0x24 of the record that gEffectRecordBase + 2 * effectRecordId addresses. */
typedef struct EffectRecordBytes {
    u8 unk_00[0x24];
    u8 effectByteA;
    u8 effectByteB;
} EffectRecordBytes;

void copyEffectRecordPair(s32 effectRecordId, u8 *effectBytePairOut) {
    s32 effectRecordOffset;

    effectRecordOffset = (effectRecordId << 0x10) >> 0xF;
    effectBytePairOut[0] = ((EffectRecordBytes *) (effectRecordOffset + gEffectRecordBase))->effectByteA;
    effectBytePairOut[1] = ((EffectRecordBytes *) (effectRecordOffset + gEffectRecordBase))->effectByteB;
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8CBB0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8D1B0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8D4A0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8D5A8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8D900);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8DA40);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8DB70);

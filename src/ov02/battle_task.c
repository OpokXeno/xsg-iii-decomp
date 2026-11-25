#include "common.h"
#include "shared.h"

#define BATTLE_SLOT_SKIP_UPDATE 0x2
#define BATTLE_SLOT_SKIP_DRAW 0x4

int battleSlotClaim(int *flags);

void battleSlotClear(void *slot);
void *battleTaskClaim(void);
void battleTaskClearAll(void);
void battleTaskRunDraw(void);
void battleTaskRunUpdate(void);

/* Entries in the 32-record task table battleTaskTable. */
#define BATTLE_TASK_COUNT 32

/*
 * One 0x1C-byte task record of the 32-entry table battleTaskTable. The table holds the tasks that
 * battleTaskClaim claims for the fades (monsSlotFadeInStart, unitFadeIn), the screen flash and fill and the message
 * object (msgPutObject). Bit 0x1 of flags is the claim bit (battleSlotClaim); update runs unless flags & 2
 * (update sweep battleTaskRunUpdate), draw runs unless flags & 4 (draw sweep battleTaskRunDraw).
 * A finished task is ended by battleTaskRelease.
 */
typedef struct TaskRecord {
    int taskFlags;
    void (*update)(struct TaskRecord *);
    void (*draw)(struct TaskRecord *);
    unsigned char unmodeled_0C[0x1C - 0x0C];
} TaskRecord;

static TaskRecord battleTaskTable[BATTLE_TASK_COUNT];

/* Clears all 32 task records (battleTaskClearAll), called by battleClearTaskAndSlotTables. */
void battleTaskClearAll(void)
{
    int i;

    for (i = 0; i < BATTLE_TASK_COUNT; i++) {
        battleSlotClear(&battleTaskTable[i]);
    }
}

/* Update sweep over the 32 task records (battleTaskRunUpdate): runs each record's update unless flags & 2. */
void battleTaskRunUpdate(void)
{
    void (*update)(TaskRecord *);
    TaskRecord *taskRecord;
    int i;

    taskRecord = battleTaskTable;
    for (i = 0; i < BATTLE_TASK_COUNT; i++) {
        update = taskRecord->update;
        if (update != 0 && !(taskRecord->taskFlags & BATTLE_SLOT_SKIP_UPDATE)) {
            update(taskRecord);
        }
        taskRecord++;
    }
}

/* Draw sweep over the 32 task records (battleTaskRunDraw): runs each record's draw unless flags & 4. */
void battleTaskRunDraw(void)
{
    void (*draw)(TaskRecord *);
    TaskRecord *taskRecord;
    int i;

    taskRecord = battleTaskTable;
    for (i = 0; i < BATTLE_TASK_COUNT; i++) {
        draw = taskRecord->draw;
        if (draw != 0 && !(taskRecord->taskFlags & BATTLE_SLOT_SKIP_DRAW)) {
            draw(taskRecord);
        }
        taskRecord++;
    }
}

/* Claims the first free task record: returns it, or 0 when all 32 records are claimed. */
void *battleTaskClaim(void)
{
    int i;

    for (i = 0; i < BATTLE_TASK_COUNT; i++) {
        if (battleSlotClaim(&battleTaskTable[i].taskFlags)) {
            return &battleTaskTable[i];
        }
    }
    return 0;
}

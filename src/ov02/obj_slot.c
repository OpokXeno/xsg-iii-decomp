#include "common.h"
#include "shared.h"

void *battleCallbackSlotClaim(void);

#define SAVE_STACK_LEVEL_CELLS 5  /* cells per SaveStack level */
#define SAVE_STACK_LEVELS 4       /* levels of a SaveStack (a push to this depth is refused) */

struct SaveStack;

#include "ov02/tu030.h"


struct SaveStack *battleStackNew(void);

void battleStackPop(struct SaveStack *stack);
void battleStackPush(struct SaveStack *stack, SaveStackStep pushedStep);
void battleStackSetSuccessorStep(struct SaveStack *stack, SaveStackStep successorStep);

/* battleStackNew: takes a save stack from battleCallbackSlotClaim (battle_slot.h) and resets its depth and top. */
struct SaveStack *battleStackNew(void) {
    struct SaveStack *stack;

    stack = battleCallbackSlotClaim();
    if (stack != 0) {
        stack->count = 0;
        stack->top = 0;
        stack->unmodeled_08 = 0;
    }
    return stack;
}

/* Sets the successor step of a battle step context (battleStackSetSuccessorStep, obj_slot.c): stores successorStep at +0x04.
 * Its callers pass step functions. */
void battleStackSetSuccessorStep(struct SaveStack *stack, SaveStackStep successorStep) {
    stack->step = successorStep;
}

/* battleStackPush: keeps the top in the cell below the new level and makes pushedStep the top.
 * The depth stops at SAVE_STACK_LEVELS - 1; a push that reaches the limit leaves the top alone. */
void battleStackPush(struct SaveStack *stack, SaveStackStep pushedStep)
{
    int depth;
    int *savedTop;

    depth = stack->count;
    stack->count = depth + 1;
    savedTop = &stack->cells[depth * SAVE_STACK_LEVEL_CELLS - 1];
    *savedTop = stack->top;
    if (depth + 1 >= SAVE_STACK_LEVELS) {
        stack->count = SAVE_STACK_LEVELS - 1;
        return;
    }
    stack->step = pushedStep;
}

/* battleStackPop: restores the top kept by the last push. */
void battleStackPop(struct SaveStack *stack)
{
    int depth;
    int *savedTop;

    depth = stack->count - 1;
    stack->count = depth;
    if (depth < 0) {
        stack->count = 0;
        return;
    }
    savedTop = &stack->cells[depth * SAVE_STACK_LEVEL_CELLS - 1];
    stack->top = *savedTop;
}

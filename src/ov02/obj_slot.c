#include "common.h"
#include "shared.h"

void *ov01CallbackSlotClaim(void);

#define SAVE_STACK_LEVEL_CELLS 5  /* cells per SaveStack level */
#define SAVE_STACK_LEVELS 4       /* levels of a SaveStack (a push to this depth is refused) */

struct SaveStack;

#include "ov02/tu030.h"


struct SaveStack *ov01StackNew(void);

void ov01StackPop(struct SaveStack *stack);
void ov01StackPush(struct SaveStack *stack, SaveStackStep step);
void ov01StepSetNext(struct SaveStack *stack, SaveStackStep nextStep);

/* ov01StackNew: takes a save stack from ov01CallbackSlotClaim (ov01_slot.h) and resets its depth and top. */
struct SaveStack *ov01StackNew(void) {
    struct SaveStack *stack;

    stack = ov01CallbackSlotClaim();
    if (stack != 0) {
        stack->count = 0;
        stack->top = 0;
        stack->unmodeled_08 = 0;
    }
    return stack;
}

/* Sets the successor step of a battle step context (ov01StepSetNext, obj_slot.c): stores nextStep at +0x04.
 * Its callers pass step functions. */
void ov01StepSetNext(struct SaveStack *stack, SaveStackStep nextStep) {
    stack->step = nextStep;
}

/* ov01StackPush: keeps the top in the cell below the new level and makes value the top.
 * The depth stops at SAVE_STACK_LEVELS - 1; a push that reaches the limit leaves the top alone. */
void ov01StackPush(struct SaveStack *stack, SaveStackStep value)
{
    int index;
    int *savedTop;

    index = stack->count;
    stack->count = index + 1;
    savedTop = &stack->cells[index * SAVE_STACK_LEVEL_CELLS - 1];
    *savedTop = stack->top;
    if (index + 1 >= SAVE_STACK_LEVELS) {
        stack->count = SAVE_STACK_LEVELS - 1;
        return;
    }
    stack->step = value;
}

/* ov01StackPop: restores the top kept by the last push. */
void ov01StackPop(struct SaveStack *stack)
{
    int index;
    int *savedTop;

    index = stack->count - 1;
    stack->count = index;
    if (index < 0) {
        stack->count = 0;
        return;
    }
    savedTop = &stack->cells[index * SAVE_STACK_LEVEL_CELLS - 1];
    stack->top = *savedTop;
}

/* header_harvest: hand-maintained (X3 express integration; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_OV02_TU030_H
#define INCLUDE_OV02_TU030_H

/* Types owned by ov02/tu030 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"

/* A step of the battle step chain: battleStackSetSuccessorStep and battleStackPush take one, and the chain driver calls it with its stack. */
typedef void (*SaveStackStep)(struct SaveStack *context);

/* Save stack of SAVE_STACK_LEVELS levels. `top` is the live value and `count` the depth. Level L has SAVE_STACK_LEVEL_CELLS
 * cells, cells[5L + slot]; the push from depth c keeps the old top in cells[5c - 1] (unmodeled_0c for c = 0). */
struct SaveStack {
    int unmodeled_00;
    union {
        int top;
        SaveStackStep step;   /* the successor step battleStackSetSuccessorStep stores, and battleStackPush pushes; the same word as top */
    };
    int unmodeled_08;
    int unmodeled_0c;
    int cells[19 /* 4 levels * 5 cells - 1 */];
    int count;
};

#endif

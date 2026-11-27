#ifndef INCLUDE_MAIN_TU160_H
#define INCLUDE_MAIN_TU160_H

#include "shared.h"

typedef void (*CompletionCallback)(void);

/* Status header record at gPadStatusMasks: the pad masks of the frame (+0x00 held, +0x02 pressed, +0x04 repeated,
 * +0x06 pressed in the previous frame) and the two group words at +0x08 and +0x0A. */
typedef struct PadStatusMasks {
    unsigned short held;
    unsigned short pressed;
    unsigned short repeated;
    unsigned short previousPressed;
    unsigned short unk_08;
    unsigned short unk_0A;
} PadStatusMasks;

/* Returns the pad status record of the frame. */
PadStatusMasks *getStatusHeader(void);

/* Appends a completionCallback to the completion list and keeps it null-terminated; a null completionCallback clears the list. */
void completionCallbackAdd(CompletionCallback completionCallback);

#endif /* INCLUDE_MAIN_TU160_H */

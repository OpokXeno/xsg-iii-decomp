#include "common.h"

extern unsigned char *gBufferCursor;
extern unsigned char *gBufferArenaStart;

void bufferCursorClear(void) {
    gBufferArenaStart = 0;
    gBufferCursor = 0;
}

extern unsigned char gBufferArena[];

void bufferPointersRewind(void)
{
    gBufferCursor = gBufferArena;
    gBufferArenaStart = gBufferArena;
}

extern char D_00550B78[];

/* Text of message index from the table at gBufferArenaStart: word 0 is the count, word index + 1 the offset of the text
 * from the table. A missing table or an index out of range gives the empty string at D_00550B78. */
char *messageTextGet(int index) {
    unsigned int *table = (unsigned int *) gBufferArenaStart;

    if (table == 0 || index < 0 || index >= table[0]) {
        return D_00550B78;
    }
    return (char *) table + table[index + 1];
}

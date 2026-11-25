#include "common.h"

extern unsigned char *gBufferCursor;
unsigned char *gBufferArenaStart = 0;

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


/* Text of message index from the table at gBufferArenaStart: word 0 is the count, word index + 1 the offset of the text
 * from the table. A missing table or an index out of range gives the empty string. */
char *messageTextGet(int index) {
    unsigned int *table = (unsigned int *) gBufferArenaStart;

    if (table == 0 || index < 0 || index >= table[0]) {
        return "";
    }
    return (char *) table + table[index + 1];
}

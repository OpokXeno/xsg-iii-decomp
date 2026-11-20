#include "common.h"
#include "shared.h"

void sjisToEuc(unsigned char *first, unsigned char *second);
/* Bump pointer of the test heap. */
extern int testHeapCursor;
int xglDebFileSize(const char *name);
int xglDebFileRead(const char *name, void *buffer);
void assertFailed(const char *message, const char *file, int line);
extern const char D_00539DF8[];
extern const char D_00539E00[];

/* Reads the named debug file into the test heap at the cursor and returns its address, or 0 when the file is empty. */
int testLoadFile(int *bufferCursor, const char *name) {
    int buffer;
    int size;

    buffer = 0;
    size = xglDebFileSize(name);
    if (size > 0) {
        buffer = *bufferCursor;
        *bufferCursor = (buffer + size + 0xF) & ~0xF;
        if (buffer == 0) {
            assertFailed(D_00539DF8, D_00539E00, 159);
        }
        xglDebFileRead(name, (void *) buffer);
    }
    return buffer;
}

/* Converts the two-byte character at first/second in place: the lead byte is doubled and both bytes are
 * moved into the Shift-JIS ranges. Only yg2PathReadTaggedText calls it, for each high-bit character. */
void sjisToEuc(unsigned char *first, unsigned char *second)
{
    unsigned int leadDoubled;
    unsigned char trailCode;

    leadDoubled = *first << 1;
    *first = leadDoubled;
    trailCode = *second;
    if (trailCode < 0x9F) {
        if ((leadDoubled & 0xFE) < 0x3F) {
            *first = leadDoubled + 0x9F;
        } else {
            *first = leadDoubled + 0x1F;
        }
        if (*second >= 0x7F) {
            *second = *second + 0x60;
        } else {
            *second = *second + 0x61;
        }
    } else {
        if ((leadDoubled & 0xFE) < 0x3F) {
            *first = leadDoubled + 0xA0;
        } else {
            *first = leadDoubled + 0x20;
        }
        *second = *second + 2;
    }
}

/* Sets the test heap cursor. */
void testHeapSetCursor(int newCursor) {
    testHeapCursor = newCursor;
}

/* Bump allocation: returns the next 16-byte aligned address and moves the cursor past size bytes. */
int testHeapAlloc(int size) {
    int buffer;
    int cursorAfter;

    buffer = (testHeapCursor + 0xF) & ~0xF;
    cursorAfter = buffer + size;
    testHeapCursor = cursorAfter;
    return buffer;
}

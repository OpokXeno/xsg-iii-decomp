#include "common.h"
#include "shared.h"

void sjisToEuc(unsigned char *leadByte, unsigned char *trailByte);
/* Bump pointer of the test heap. */
static int testHeapCursor;
int xglDebFileSize(const char *name);
int xglDebFileRead(const char *name, void *loadAddress);
void assertFailed(const char *message, const char *file, int line);
extern const char gTestSourceFileName[];

/* Reads the named debug file into the test heap at the cursor and returns its address, or 0 when the file is empty. */
int testLoadFile(int *bufferCursor, const char *name) {
    int loadAddress;
    int fileSize;

    loadAddress = 0;
    fileSize = xglDebFileSize(name);
    if (fileSize > 0) {
        loadAddress = *bufferCursor;
        *bufferCursor = (loadAddress + fileSize + 0xF) & ~0xF;
        if (loadAddress == 0) {
            assertFailed("pData", gTestSourceFileName, 159);
        }
        xglDebFileRead(name, (void *) loadAddress);
    }
    return loadAddress;
}

/* Converts the two-byte character at leadByte/trailByte in place: the lead byte is doubled and both bytes are
 * moved into the Shift-JIS ranges. Only yg2PathReadTaggedText calls it, for each high-bit character. */
void sjisToEuc(unsigned char *leadByte, unsigned char *trailByte)
{
    unsigned int leadDoubled;
    unsigned char trailCode;

    leadDoubled = *leadByte << 1;
    *leadByte = leadDoubled;
    trailCode = *trailByte;
    if (trailCode < 0x9F) {
        if ((leadDoubled & 0xFE) < 0x3F) {
            *leadByte = leadDoubled + 0x9F;
        } else {
            *leadByte = leadDoubled + 0x1F;
        }
        if (*trailByte >= 0x7F) {
            *trailByte = *trailByte + 0x60;
        } else {
            *trailByte = *trailByte + 0x61;
        }
    } else {
        if ((leadDoubled & 0xFE) < 0x3F) {
            *leadByte = leadDoubled + 0xA0;
        } else {
            *leadByte = leadDoubled + 0x20;
        }
        *trailByte = *trailByte + 2;
    }
}

/* Sets the test heap cursor. */
void testHeapSetCursor(int newCursor) {
    testHeapCursor = newCursor;
}

/* Bump allocation: returns the next 16-byte aligned address and moves the cursor past allocationBytes bytes. */
int testHeapAlloc(int allocationBytes) {
    int allocAddress;
    int cursorAfter;

    allocAddress = (testHeapCursor + 0xF) & ~0xF;
    cursorAfter = allocAddress + allocationBytes;
    testHeapCursor = cursorAfter;
    return allocAddress;
}

const char gTestSourceFileName[328] =
    "test.euc.c\0\0\0\0\0\0"
    "host0:/home/xeno/work/iwase/nep.pxy\0\0\0\0\0"
    "host0:/home/xeno/work/iwase/nep.txy\0\0\0\0\0"
    "host0:/home/xeno/work/iwase/nep.xhr\0\0\0\0\0"
    "host0:/home/xeno/work/iwase/kosmos01.xap\0\0\0\0\0\0\0\0"
    "host0:/home/xeno/work/iwase/shionH00.pxy\0\0\0\0\0\0\0\0"
    "host0:/home/xeno/work/iwase/shionH00.txy\0\0\0\0\0\0\0\0"
    "host0:/home/xeno/work/iwase/shionH00.xhr\0\0\0\0\0\0\0\0";

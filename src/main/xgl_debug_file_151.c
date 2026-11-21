#include "common.h"

typedef int s32;
extern char xglDebFileNameBuffer[];
char *strcpy(char *destination, const char *source);

void xglDebFileNameSet(const char *text) {
    strcpy(xglDebFileNameBuffer, text);
}

int xglDebFileRead(const char *name, void *buffer);
int xglDebFileSize(const char *name);

int xglDebFileReadSize(void *fileBuffer, const char *name)
{
    if (xglDebFileRead(name, fileBuffer) >= 0) {
        return xglDebFileSize(name);
    }
    return -1;
}

s32 func_0030E8E4(s32);

s32 xglDebFileBufferSize(s32 fileSize) {
    s32 paddedSize;

    paddedSize = fileSize + 0x14;
    return (func_0030E8E4(paddedSize) == 0) ? 0 : paddedSize;
}

#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/xgl_debug_file_151", func_0020F618);

int xglDebFileRead(const char *name, void *buffer);
int xglDebFileSize(const char *name);

int xglDebFileReadSize(void *buffer, const char *name)
{
    if (xglDebFileRead(name, buffer) >= 0) {
        return xglDebFileSize(name);
    }
    return -1;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_debug_file_151", func_0020F688);

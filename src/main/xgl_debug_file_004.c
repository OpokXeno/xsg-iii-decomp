#include "common.h"
#include "shared.h"

#include "debug_break.h"

int xglDebFileSize(const char *name);
int xglDebFileRead(const char *name, void *buffer);
int xglDebFileWrite(const char *name, const void *buffer, int size);
int printf(const char *format, ...);

/*
 * EE Timer 0.  T0_COUNT is the free-running counter and T0_MODE its control
 * word; bit 7 (0x80) of T0_MODE is CUE, which starts the count.
 */
#define T0_COUNT ((volatile u32 *)0x10000000)

#define T0_MODE ((volatile u32 *)0x10000010)
#define T0_MODE_CUE 0x80
#define T0_COUNT_MASK 0xffff

void xglTimer0Reset(int mode) {
    *T0_COUNT = 0;
    *T0_MODE = mode + T0_MODE_CUE;
}

int xglTimer0Get(void)
{
    return *T0_COUNT & T0_COUNT_MASK;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_debug_file_004", func_00181C18);

void xglDebFileEmpty(void) {

}

/* Debug stubs: each passes a fixed "Don't use ..." message to printf. */
int xglDebFileSize(const char *name)
{
    printf("Don't use xglDebFileSize()\n");
    DEBUG_BREAK();
    return -1;
}

int xglDebFileRead(const char *name, void *buffer)
{
    printf("Don't use xglDebFileRead()\n");
    DEBUG_BREAK();
    return -1;
}

int xglDebFileWrite(const char *name, const void *buffer, int size)
{
    printf("Don't use xglDebFileWrite()\n");
    DEBUG_BREAK();
    return -1;
}

#include "common.h"
#include "shared.h"

int printf(const char *format, ...);
void assertExit(void);

#include "debug_break.h"


typedef char *va_list;

#define va_start(ap, last) ((ap) = (va_list)__builtin_next_arg(last) - (8 - __builtin_args_info(2)) * 8)

int vprintf(const char *format, va_list formatArgs);
extern const char exitProgramBanner[64];
extern const char errorBanner[16];

INCLUDE_ASM("asm/main/nonmatchings/assert_fail", func_00193600);

/* Assertion failure report: the original body only tail-calls assertExit. */
void assertFailed(const char *message, const char *file, int line) {
    assertExit();
}

INCLUDE_ASM("asm/main/nonmatchings/assert_fail", func_001936A8);

INCLUDE_ASM("asm/main/nonmatchings/assert_fail", func_001936F0);

INCLUDE_ASM("asm/main/nonmatchings/assert_fail", func_00193730);

/* Exit path shared by the assertion and error reporters: prints "> Exit Program", then traps (break). */
void assertExit(void) {
    printf(exitProgramBanner);
    DEBUG_BREAK();
}

/* Fatal error with a printf-style message: prints "> Error!!", the formatted message, then exits. */
void errorFatal(const char *format, ...) {
    va_list formatArgs;

    printf(errorBanner);
    va_start(formatArgs, format);
    vprintf(format, formatArgs);
    printf(format);
    assertExit();
}

#include "common.h"
#include "main/ee_stdarg.h"
#include "shared.h"

void LOG(const void *loggedVector, const char *format, ...);
void tracePrint(const char *format, ...);


extern int printf(const char *format, ...);
extern int vsprintf(char *output, const char *format, va_list formatArgs);

void tracePrint(const char *format, ...) {
    va_list formatArgs;
    char formattedText[0x100];

    va_start(formatArgs, format);
    vsprintf(formattedText, format, formatArgs);
    printf("%s", formattedText);
    va_end(formatArgs);
}

/* Log call sites use this variadic no-op (lightSetLogDump passes a vector, a format and an index): it spills
 * its arguments and returns. */
void LOG(const void *loggedVector, const char *format, ...) {
}

/* Variadic no-op with the LOG signature. No caller in the tree names it. */
void func_001ED6D0(const void *loggedVector, const char *format, ...) {
}

/* Variadic no-op with the LOG signature. No caller in the tree names it. */
void func_001ED718(const void *loggedVector, const char *format, ...) {
}

/* Variadic no-op with the LOG signature. No caller in the tree names it. */
void func_001ED760(const void *loggedVector, const char *format, ...) {
}

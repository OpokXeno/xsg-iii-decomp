#include "common.h"
#include "main/ee_stdarg.h"
#include "shared.h"

int printf(const char *format, ...);
void assertExit(void);

#include "debug_break.h"
#include "main/sce_sif.h"



int vprintf(const char *format, va_list formatArgs);
extern const char exitProgramBanner[64];
extern const char errorBanner[16];

#define IOP_SUB_RPC_ID 0x4E3C
/* Busy-wait between two bind attempts: counts down from IOP_SUB_BIND_WAIT in steps of 4 to -1. */
#define IOP_SUB_BIND_WAIT 0xFFFF

extern sceSifClientData gIoRpcCallBlock;
/* "iopsub bind err\n" */
extern const char iopSubBindErrorText[];

/* Binds the IOP sub module's RPC server, retrying until it answers; a failed bind reports
 * iopSubBindErrorText and traps. */
int func_00193600(void)
{
    int waitEnd;
    int wait;

    while (1) {
        if (sceSifBindRpc(&gIoRpcCallBlock, IOP_SUB_RPC_ID, 0) < 0) {
            printf(iopSubBindErrorText);
            DEBUG_BREAK();
        }
        if (gIoRpcCallBlock.serve != 0) {
            break;
        }
        waitEnd = -1;
        wait = IOP_SUB_BIND_WAIT;
        do {
            wait -= 4;
            __asm__ volatile("nop");
        } while (wait != waitEnd);
    }
    return 0;
}

/* Assertion failure report: the original body only tail-calls assertExit. */
void assertFailed(const char *message, const char *file, int line) {
    assertExit();
}

/* Assertion with a printf-style message (condition, file, line, format, ...): like assertFailed, the original only
 * exits through assertExit. */
void func_001936A8(const char *condition, const char *file, int line, const char *format, ...)
{
    assertExit();
}

/* Debug log with a source position (file, line, format, ...); empty in the retail build. */
void func_001936F0(const char *file, int line, const char *format, ...)
{
}

/* Checked report with a printf-style message (condition, file, line, format, ...); empty in the retail build. */
void func_00193730(const char *condition, const char *file, int line, const char *format, ...)
{
}

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

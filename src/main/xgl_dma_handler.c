#include "common.h"
#include "shared.h"


#include "main/tu022.h"

/* The INTC causes and DMAC channels the slot maps cover. */
#define XGL_INTC_CAUSES 11

#define XGL_DMAC_CHANNELS 16

/* INTC cause / DMAC channel -> slot of its record in the table; -1 when none is registered.
 * Both slot maps are volatile: the interrupt handlers read them asynchronously while the
 * main thread (xglIntrInit and the enable/disable calls) writes and tests them. */
extern volatile int xglIntcSlotMap[];

/* Timer accumulator of the VIF1 handler (xgl_intc.c): active flag, start count, ticks total. */
extern int intrTimerActive;

extern int intrTimerStart;
extern int intrTimerAccum;

void xglIntrInit(void);

int xglIntcGetEventCount(int cause);
int xglIntcEnable(unsigned int cause);
int xglIntcDisable(unsigned int cause);
int xglDmaGetEventCount(int channel);
int xglDmaEnable(unsigned int channel);
int xglDmaDisable(unsigned int channel);

extern int AddIntcHandler(int cause, int (*handler)(int cause), int next);
extern int AddDmacHandler(int channel, int (*handler)(int channel), int next);

void EnableIntc(unsigned int cause, int value);
void DisableIntc(unsigned int cause, int value);

int EnableDmac(int channel);
int DisableDmac(int channel);

/* DMAC slot map: channel -> slot in xglDmaRecords, -1 until xglIntrInit fills it. */
extern volatile int xglDmaSlotMap[XGL_DMAC_CHANNELS];

/* INTC handler table: {handle, count, cause, handler}; the last record has cause -1. */
extern XglIntrRecord xglIntcRecords[12];

/* DMAC handler table: {handle, count, channel, handler}; the last record has channel -1. */
extern XglIntrRecord xglDmaRecords[8];

/* DMAC channel event handlers, one per channel registered in xglDmaRecords: VIF0, VIF1, GIF,
 * from IPU, to IPU, from SPR, to SPR (channels 0, 1, 2, 3, 4, 8, 9). The bodies are identical:
 * each bumps the count of its channel's record and returns 0. */
int xglDmaHandlerVif0(int channel)
{
    int *eventCount = &xglDmaRecords[xglDmaSlotMap[channel]].eventCount;

    (*eventCount)++;
    /* The end of an INTC/DMAC handler (SDK eekernel.h ExitHandler): sync, then re-enable interrupts. */
    __asm__ volatile("sync" ::: "memory");
    __asm__ volatile("ei" ::: "memory");
    return 0;
}

int xglDmaHandlerVif1(int channel)
{
    int *eventCount = &xglDmaRecords[xglDmaSlotMap[channel]].eventCount;

    (*eventCount)++;
    /* The end of an INTC/DMAC handler (SDK eekernel.h ExitHandler): sync, then re-enable interrupts. */
    __asm__ volatile("sync" ::: "memory");
    __asm__ volatile("ei" ::: "memory");
    return 0;
}

int xglDmaHandlerGif(int channel)
{
    int *eventCount = &xglDmaRecords[xglDmaSlotMap[channel]].eventCount;

    (*eventCount)++;
    /* The end of an INTC/DMAC handler (SDK eekernel.h ExitHandler): sync, then re-enable interrupts. */
    __asm__ volatile("sync" ::: "memory");
    __asm__ volatile("ei" ::: "memory");
    return 0;
}

int xglDmaHandlerFromIpu(int channel)
{
    int *eventCount = &xglDmaRecords[xglDmaSlotMap[channel]].eventCount;

    (*eventCount)++;
    /* The end of an INTC/DMAC handler (SDK eekernel.h ExitHandler): sync, then re-enable interrupts. */
    __asm__ volatile("sync" ::: "memory");
    __asm__ volatile("ei" ::: "memory");
    return 0;
}

int xglDmaHandlerToIpu(int channel)
{
    int *eventCount = &xglDmaRecords[xglDmaSlotMap[channel]].eventCount;

    (*eventCount)++;
    /* The end of an INTC/DMAC handler (SDK eekernel.h ExitHandler): sync, then re-enable interrupts. */
    __asm__ volatile("sync" ::: "memory");
    __asm__ volatile("ei" ::: "memory");
    return 0;
}

int xglDmaHandlerFromSpr(int channel)
{
    int *eventCount = &xglDmaRecords[xglDmaSlotMap[channel]].eventCount;

    (*eventCount)++;
    /* The end of an INTC/DMAC handler (SDK eekernel.h ExitHandler): sync, then re-enable interrupts. */
    __asm__ volatile("sync" ::: "memory");
    __asm__ volatile("ei" ::: "memory");
    return 0;
}

int xglDmaHandlerToSpr(int channel)
{
    int *eventCount = &xglDmaRecords[xglDmaSlotMap[channel]].eventCount;

    (*eventCount)++;
    /* The end of an INTC/DMAC handler (SDK eekernel.h ExitHandler): sync, then re-enable interrupts. */
    __asm__ volatile("sync" ::: "memory");
    __asm__ volatile("ei" ::: "memory");
    return 0;
}

/* INTC cause -> its event count, or -1 when no handler is registered for the cause. */
int xglIntcGetEventCount(int cause)
{
    volatile int *causeSlot = &xglIntcSlotMap[cause];
    int eventCount = -1;

    if (*causeSlot >= 0) {
        eventCount = xglIntcRecords[*causeSlot].eventCount;
    }
    return eventCount;
}

/* Enables INTC cause `cause` through kernel service 0x14 (EnableIntc) when the cause has a
 * handler. Returns 1 when it did. */
int xglIntcEnable(unsigned int cause)
{
    int valid_cause = cause < XGL_INTC_CAUSES;
    int enabled = 0;
    volatile int *causeSlot = &xglIntcSlotMap[cause];

    if (valid_cause && (*causeSlot >= 0)) {
        EnableIntc(cause, 0);
        enabled = 1;
    }
    return enabled;
}

/* Disables INTC cause `cause` through kernel service 0x15 (DisableIntc) when registered. */
int xglIntcDisable(unsigned int cause)
{
    int valid_cause = cause < XGL_INTC_CAUSES;
    int disabled = 0;
    volatile int *causeSlot = &xglIntcSlotMap[cause];

    if (valid_cause && (*causeSlot >= 0)) {
        DisableIntc(cause, 0);
        disabled = 1;
    }
    return disabled;
}

/* DMAC channel -> its event count, or -1 when no handler is registered for the channel. */
int xglDmaGetEventCount(int channel)
{
    volatile int *channelSlot = &xglDmaSlotMap[channel];
    int eventCount = -1;

    if (*channelSlot >= 0) {
        eventCount = xglDmaRecords[*channelSlot].eventCount;
    }
    return eventCount;
}

/* Enables DMAC channel `channel` through kernel service 0x16 (EnableDmac) when a handler
 * is registered for it. Returns 1 when it did. */
int xglDmaEnable(unsigned int channel)
{
    int valid_channel = channel < XGL_DMAC_CHANNELS;
    int enabled = 0;
    volatile int *channelSlot = &xglDmaSlotMap[channel];

    if (valid_channel && (*channelSlot >= 0)) {
        EnableDmac(channel);
        enabled = 1;
    }
    return enabled;
}

/* Disables DMAC channel `channel` through kernel service 0x17 (DisableDmac) when registered. */
int xglDmaDisable(unsigned int channel)
{
    int valid_channel = channel < XGL_DMAC_CHANNELS;
    int disabled = 0;
    volatile int *channelSlot = &xglDmaSlotMap[channel];

    if (valid_channel && (*channelSlot >= 0)) {
        DisableDmac(channel);
        disabled = 1;
    }
    return disabled;
}

/* Registers the INTC handlers (AddIntcHandler, 0x10) and the DMAC handlers (AddDmacHandler,
 * 0x12) from their tables, fills in the slot maps and clears the timer accumulator. Called by
 * InitializeSystem. */
void xglIntrInit(void)
{
    int i;

    for (i = 0; xglIntcRecords[i].id >= 0; i++) {
        xglIntcSlotMap[xglIntcRecords[i].id] = i;
        xglIntcRecords[i].handle = AddIntcHandler(xglIntcRecords[i].id, xglIntcRecords[i].handler, 0);
    }
    for (i = 0; xglDmaRecords[i].id >= 0; i++) {
        xglDmaSlotMap[xglDmaRecords[i].id] = i;
        xglDmaRecords[i].handle = AddDmacHandler(xglDmaRecords[i].id, xglDmaRecords[i].handler, 0);
    }
    intrTimerActive = 0;
    intrTimerStart = 0;
    intrTimerAccum = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_dma_handler", func_00190688);

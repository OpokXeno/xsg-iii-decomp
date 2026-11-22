#include "common.h"
#include "shared.h"
#include "main/tu022.h"

extern XglIntrRecord xglIntcRecords[];

/* INTC cause / DMAC channel -> slot of its record in the table; -1 when none is registered.
 * Both slot maps are volatile: the interrupt handlers read them asynchronously while the
 * main thread (xglIntrInit and the enable/disable calls) writes and tests them. */
extern volatile int xglIntcSlotMap[];

/* Timer accumulator of the VIF1 handler (xgl_intc.c): active flag, start count, ticks total. */
extern int intrTimerActive;

extern int intrTimerStart;

extern int intrTimerAccum;

int xglIntcHandlerGs(int cause);

int xglIntcHandlerVblankStart(int cause);

int xglIntcHandlerVblankEnd(int cause);

int xglIntcHandlerVif0(int cause);

int xglIntcHandlerVif1(int cause);

int xglIntcHandlerVu0(int cause);

int xglIntcHandlerVu1(int cause);

int xglIntcHandlerIpu(int cause);

int xglIntcHandlerTimer0(int cause);

int xglIntcHandlerTimer1(int cause);

/* EE timer 1 count, and VIF1_FBRST, whose STC bit cancels a VIF1 stall. */
#define T1_COUNT ((volatile u32 *) 0x10000800)

#define VIF1_FBRST ((volatile u32 *) 0x10003c10)

#define VIF_FBRST_STC 8

int xglIntcHandlerGs(int cause)
{
    int *eventCount = &xglIntcRecords[xglIntcSlotMap[cause]].eventCount;

    (*eventCount)++;
    /* The end of an INTC/DMAC handler (SDK eekernel.h ExitHandler): sync, then re-enable interrupts. */
    __asm__ volatile("sync" ::: "memory");
    __asm__ volatile("ei" ::: "memory");
    return 0;
}

int xglIntcHandlerVblankStart(int cause)
{
    int *eventCount = &xglIntcRecords[xglIntcSlotMap[cause]].eventCount;

    (*eventCount)++;
    /* The end of an INTC/DMAC handler (SDK eekernel.h ExitHandler): sync, then re-enable interrupts. */
    __asm__ volatile("sync" ::: "memory");
    __asm__ volatile("ei" ::: "memory");
    return 0;
}

int xglIntcHandlerVblankEnd(int cause)
{
    int *eventCount = &xglIntcRecords[xglIntcSlotMap[cause]].eventCount;

    (*eventCount)++;
    /* The end of an INTC/DMAC handler (SDK eekernel.h ExitHandler): sync, then re-enable interrupts. */
    __asm__ volatile("sync" ::: "memory");
    __asm__ volatile("ei" ::: "memory");
    return 0;
}

int xglIntcHandlerVif0(int cause)
{
    int *eventCount = &xglIntcRecords[xglIntcSlotMap[cause]].eventCount;

    (*eventCount)++;
    /* The end of an INTC/DMAC handler (SDK eekernel.h ExitHandler): sync, then re-enable interrupts. */
    __asm__ volatile("sync" ::: "memory");
    __asm__ volatile("ei" ::: "memory");
    return 0;
}

/* The VIF1 handler and timer accumulator. It reads the EE timer 1 count (0x10000800). When
 * intrTimerActive is set it adds the ticks since the start count to intrTimerAccum and clears the
 * flag; otherwise it sets the flag and records the start count. It also bumps its event count and
* cancels a VIF1 stall. */
int xglIntcHandlerVif1(int cause)
{
    int *eventCount = &xglIntcRecords[xglIntcSlotMap[cause]].eventCount;

    (*eventCount)++;
    if (intrTimerActive) {
        intrTimerAccum += *T1_COUNT - intrTimerStart;
        intrTimerActive = 0;
    } else {
        intrTimerStart = *T1_COUNT;
        intrTimerActive = 1;
    }
    *VIF1_FBRST = VIF_FBRST_STC;
    /* The end of an INTC/DMAC handler (SDK eekernel.h ExitHandler): sync, then re-enable interrupts. */
    __asm__ volatile("sync" ::: "memory");
    __asm__ volatile("ei" ::: "memory");
    return 0;
}

/* Clears the timer accumulator. */
void intrTimerReset(void)
{
    intrTimerActive = 0;
    intrTimerStart = 0;
    intrTimerAccum = 0;
}

/* Returns the accumulated timer ticks. */
int intrTimerGetAccum(void)
{
    return intrTimerAccum;
}

int xglIntcHandlerVu0(int cause)
{
    int *eventCount = &xglIntcRecords[xglIntcSlotMap[cause]].eventCount;

    (*eventCount)++;
    /* The end of an INTC/DMAC handler (SDK eekernel.h ExitHandler): sync, then re-enable interrupts. */
    __asm__ volatile("sync" ::: "memory");
    __asm__ volatile("ei" ::: "memory");
    return 0;
}

int xglIntcHandlerVu1(int cause)
{
    int *eventCount = &xglIntcRecords[xglIntcSlotMap[cause]].eventCount;

    (*eventCount)++;
    /* The end of an INTC/DMAC handler (SDK eekernel.h ExitHandler): sync, then re-enable interrupts. */
    __asm__ volatile("sync" ::: "memory");
    __asm__ volatile("ei" ::: "memory");
    return 0;
}

int xglIntcHandlerIpu(int cause)
{
    int *eventCount = &xglIntcRecords[xglIntcSlotMap[cause]].eventCount;

    (*eventCount)++;
    /* The end of an INTC/DMAC handler (SDK eekernel.h ExitHandler): sync, then re-enable interrupts. */
    __asm__ volatile("sync" ::: "memory");
    __asm__ volatile("ei" ::: "memory");
    return 0;
}

int xglIntcHandlerTimer0(int cause)
{
    int *eventCount = &xglIntcRecords[xglIntcSlotMap[cause]].eventCount;

    (*eventCount)++;
    /* The end of an INTC/DMAC handler (SDK eekernel.h ExitHandler): sync, then re-enable interrupts. */
    __asm__ volatile("sync" ::: "memory");
    __asm__ volatile("ei" ::: "memory");
    return 0;
}

int xglIntcHandlerTimer1(int cause)
{
    int *eventCount = &xglIntcRecords[xglIntcSlotMap[cause]].eventCount;

    (*eventCount)++;
    /* The end of an INTC/DMAC handler (SDK eekernel.h ExitHandler): sync, then re-enable interrupts. */
    __asm__ volatile("sync" ::: "memory");
    __asm__ volatile("ei" ::: "memory");
    return 0;
}

/* The INTC causes and DMAC channels the slot maps cover. */
#define XGL_INTC_CAUSES 11

#define XGL_DMAC_CHANNELS 16

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

/* Kernel service 0x15 returns the previous state. xglIntcDisable passes a second, cleared argument
 * (the original sets $a1 = 0 there), func_00190688 only the cause, so the prototype is variadic. */
int DisableIntc(unsigned int cause, ...);

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

int RemoveIntcHandler(int cause, int handlerId);
int RemoveDmacHandler(int channel, int handlerId);

/* Removes every handler xglIntrInit registered (RemoveIntcHandler / RemoveDmacHandler, kernel services
 * 0x11 and 0x13) and disables its INTC cause or DMAC channel. */
void func_00190688(void)
{
    int i;

    for (i = 0; xglIntcRecords[i].id >= 0; i++) {
        RemoveIntcHandler(xglIntcRecords[i].id, xglIntcRecords[i].handle);
        DisableIntc(xglIntcRecords[i].id);
    }
    for (i = 0; xglDmaRecords[i].id >= 0; i++) {
        RemoveDmacHandler(xglDmaRecords[i].id, xglDmaRecords[i].handle);
        DisableDmac(xglDmaRecords[i].id);
    }
}

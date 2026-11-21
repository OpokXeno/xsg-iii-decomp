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

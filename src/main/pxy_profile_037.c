#include "common.h"
#include "shared.h"

typedef struct PxyProfileEntry {
    u32 startTicks;
    u32 totalTicks;
    unsigned char unmodeled_08[4];
    int inUse;
    char name[128];
    char text[128];
} PxyProfileEntry;

#define PXY_ENTRY_COUNT 32

/* EE timer 1 count register. */
#define T1_COUNT ((volatile u32 *) 0x10000800)

/* The profile slots, scaffold .bss at 0x008C06B0. */
extern PxyProfileEntry pxyProfileEntries[PXY_ENTRY_COUNT];

int func_0030E68C(const char *, const char *);
char *strcpy(char *, const char *);
int GameMpeg2Play(char *destination, const char *format, ...);

INCLUDE_ASM("asm/main/nonmatchings/pxy_profile_037", func_0019C8D0);

void pxyProfileBegin(const char *name)
{
    int i;
    int found;

    found = -1;
    for (i = 0; i < PXY_ENTRY_COUNT; i++) {
        if (pxyProfileEntries[i].inUse != 0) {
            if (func_0030E68C(pxyProfileEntries[i].name, name) == 0) {
                found = i;
                break;
            }
        }
    }
    if (found < 0) {
        for (i = 0; i < PXY_ENTRY_COUNT; i++) {
            if (pxyProfileEntries[i].inUse == 0) {
                found = i;
                strcpy(pxyProfileEntries[i].name, name);
                pxyProfileEntries[i].startTicks = 0;
                pxyProfileEntries[i].totalTicks = 0;
                pxyProfileEntries[i].text[0] = 0;
                pxyProfileEntries[i].inUse = 1;
                break;
            }
        }
    }
    if (found >= 0 && found < PXY_ENTRY_COUNT) {
        pxyProfileEntries[found].startTicks = *T1_COUNT;
    }
}

float pxyProfileEnd(const char *name)
{
    int i;
    int found;
    float result;
    float msPerTick;

    found = -1;
    result = 0.0f;
    for (i = 0; i < PXY_ENTRY_COUNT; i++) {
        if (pxyProfileEntries[i].inUse != 0) {
            if (func_0030E68C(pxyProfileEntries[i].name, name) == 0) {
                found = i;
                break;
            }
        }
    }
    if (found >= 0) {
        msPerTick = 0.0052173799f;
        pxyProfileEntries[found].totalTicks = pxyProfileEntries[found].totalTicks + (*T1_COUNT - pxyProfileEntries[found].startTicks);
        result = (float) pxyProfileEntries[found].totalTicks * msPerTick;
        GameMpeg2Play(pxyProfileEntries[found].text, "%s  %3.2f", pxyProfileEntries[found].name, (double) result);
    }
    return result;
}

INCLUDE_ASM("asm/main/nonmatchings/pxy_profile_037", func_0019CBB0);

INCLUDE_ASM("asm/main/nonmatchings/pxy_profile_037", func_0019CCE8);

INCLUDE_ASM("asm/main/nonmatchings/pxy_profile_037", func_0019CDF8);

INCLUDE_ASM("asm/main/nonmatchings/pxy_profile_037", func_0019CED8);

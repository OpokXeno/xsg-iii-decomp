#include "common.h"
#include "shared.h"

typedef struct PxyProfileEntry {
    u32 startTicks;
    u32 totalTicks;
    unsigned char pad_08[4];
    int inUse;
    char name[128];
    char text[128];
} PxyProfileEntry;

#define PXY_ENTRY_COUNT 32
/* EE timer 1 count register. */
#define T1_COUNT ((volatile u32 *) 0x10000800)

/* The profile slots, scaffold .bss at 0x008C06B0. */
extern PxyProfileEntry pxyProfileEntries[PXY_ENTRY_COUNT];
int strcmp(const char *, const char *);
char *strcpy(char *, const char *);
int GameMpeg2Play(char *destination, const char *format, ...);

/* Frees every profile slot. */
void func_0019C8D0(void)
{
    int slot;

    slot = PXY_ENTRY_COUNT - 1;
    do {
        pxyProfileEntries[slot].inUse = 0;
        slot--;
    } while (slot >= 0);
}

void pxyProfileBegin(const char *name)
{
    int i;
    int profileSlot;

    profileSlot = -1;
    for (i = 0; i < PXY_ENTRY_COUNT; i++) {
        if (pxyProfileEntries[i].inUse != 0) {
            if (strcmp(pxyProfileEntries[i].name, name) == 0) {
                profileSlot = i;
                break;
            }
        }
    }
    if (profileSlot < 0) {
        for (i = 0; i < PXY_ENTRY_COUNT; i++) {
            if (pxyProfileEntries[i].inUse == 0) {
                profileSlot = i;
                strcpy(pxyProfileEntries[i].name, name);
                pxyProfileEntries[i].startTicks = 0;
                pxyProfileEntries[i].totalTicks = 0;
                pxyProfileEntries[i].text[0] = 0;
                pxyProfileEntries[i].inUse = 1;
                break;
            }
        }
    }
    if (profileSlot >= 0 && profileSlot < PXY_ENTRY_COUNT) {
        pxyProfileEntries[profileSlot].startTicks = *T1_COUNT;
    }
}

float pxyProfileEnd(const char *name)
{
    int i;
    int profileSlot;
    float elapsedMs;
    float msPerTick;

    profileSlot = -1;
    elapsedMs = 0.0f;
    for (i = 0; i < PXY_ENTRY_COUNT; i++) {
        if (pxyProfileEntries[i].inUse != 0) {
            if (strcmp(pxyProfileEntries[i].name, name) == 0) {
                profileSlot = i;
                break;
            }
        }
    }
    if (profileSlot >= 0) {
        msPerTick = 0.0052173799f;
        pxyProfileEntries[profileSlot].totalTicks = pxyProfileEntries[profileSlot].totalTicks + (*T1_COUNT - pxyProfileEntries[profileSlot].startTicks);
        elapsedMs = pxyProfileEntries[profileSlot].totalTicks * msPerTick;
        GameMpeg2Play(pxyProfileEntries[profileSlot].text, "%s  %3.2f", pxyProfileEntries[profileSlot].name, elapsedMs);
    }
    return elapsedMs;
}

/* Adds the ticks since the slot's start to its total and returns the total in
 * milliseconds, without formatting the text line; 0 when no slot has the name. */
float pxyProfileEndMs(const char *name)
{
    int i;
    int profileSlot;
    float elapsedMs;
    float msPerTick;

    profileSlot = -1;
    elapsedMs = 0.0f;
    for (i = 0; i < PXY_ENTRY_COUNT; i++) {
        if (pxyProfileEntries[i].inUse != 0) {
            if (strcmp(pxyProfileEntries[i].name, name) == 0) {
                profileSlot = i;
                break;
            }
        }
    }
    if (profileSlot >= 0 && profileSlot < PXY_ENTRY_COUNT) {
        msPerTick = 0.0052173799f;
        pxyProfileEntries[profileSlot].totalTicks = pxyProfileEntries[profileSlot].totalTicks
            + (*T1_COUNT - pxyProfileEntries[profileSlot].startTicks);
        elapsedMs = pxyProfileEntries[profileSlot].totalTicks * msPerTick;
    }
    return elapsedMs;
}

float pxyProfileGetMs(const char *name)
{
    int i;
    int profileSlot;
    float elapsedMs;
    float msPerTick;

    profileSlot = -1;
    elapsedMs = 0.0f;
    for (i = 0; i < PXY_ENTRY_COUNT; i++) {
        if (pxyProfileEntries[i].inUse != 0) {
            if (strcmp(pxyProfileEntries[i].name, name) == 0) {
                profileSlot = i;
                break;
            }
        }
    }
    if (profileSlot >= 0 && profileSlot < PXY_ENTRY_COUNT) {
        msPerTick = 0.0052173799f;
        elapsedMs = pxyProfileEntries[profileSlot].totalTicks * msPerTick;
    }
    return elapsedMs;
}

int func_0019D4E8(const char *text, int x, int y, float scale, int color, int shadowColor, int flag);

float pxyProfileGetMsUnchecked(const char *name)
{
    int i;
    int profileSlot;
    float elapsedMs;
    float msPerTick;

    profileSlot = -1;
    elapsedMs = 0.0f;
    for (i = 0; i < PXY_ENTRY_COUNT; i++) {
        if (strcmp(pxyProfileEntries[i].name, name) == 0) {
            profileSlot = i;
            break;
        }
    }
    if (profileSlot >= 0) {
        msPerTick = 0.0052173799f;
        elapsedMs = pxyProfileEntries[profileSlot].totalTicks * msPerTick;
    }
    return elapsedMs;
}

void pxyProfileDrawEntries(void)
{
    int i;
    int lineCount;

    lineCount = 0;
    for (i = 0; i < PXY_ENTRY_COUNT; i++) {
        if (pxyProfileEntries[i].inUse != 0) {
            if (pxyProfileEntries[i].name[0] != 0) {
                func_0019D4E8(pxyProfileEntries[i].text, 10, lineCount * 17 + 100, 1.3f, -1, 0x00FFFFFF, 1);
                lineCount++;
            }
        }
    }
}

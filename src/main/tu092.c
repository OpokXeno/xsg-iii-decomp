#include "common.h"

typedef int s32;
s32 xenoSpawnerAimAtTarget();

s32 vmShootTrap(void) {
    xenoSpawnerAimAtTarget();
    return 1;
}

s32 flatSlotMarkAll(void);

s32 vmClearTrap(void) {
    flatSlotMarkAll();
    return 1;
}

s32 sceneOverlayReset();

s32 vmMinigame(void) {
    sceneOverlayReset();
    return 1;
}

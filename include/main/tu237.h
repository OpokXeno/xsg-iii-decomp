#ifndef INCLUDE_MAIN_TU237_H
#define INCLUDE_MAIN_TU237_H

typedef int s32;

typedef signed char s8;

/* The xgl camera fields this file touches (the full struct is in main/xgl_2.h). */
struct XglCamera {
    unsigned int active;
    unsigned int cameraState;
};

void battleSetPhase(s32 nextBattlePhase);

void battleSetActorSlots(s8 firstActorSlot, s8 secondActorSlot);

#endif /* INCLUDE_MAIN_TU237_H */

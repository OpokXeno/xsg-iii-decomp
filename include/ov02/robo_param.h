#ifndef INCLUDE_OV02_ROBO_PARAM_H
#define INCLUDE_OV02_ROBO_PARAM_H

typedef unsigned char u8;

typedef struct RoboBattleStats RoboBattleStats;

/* The battle unit that RoboActor.battleUnit points at (+0xA0 of the actor). */
struct RoboBattleUnit {
    u8 unk_000[0x2B0];
    RoboBattleStats *battleStats;                   /* +0x2B0 */
    void *motionActor;                  /* +0x2B4: zero when the unit has no motion actor */
};

#endif /* INCLUDE_OV02_ROBO_PARAM_H */

#ifndef INCLUDE_MAIN_TU119_H
#define INCLUDE_MAIN_TU119_H

/* Replays the buffered effects of gEffectBuffer: each one with an owner is respawned from its corners and keys and runs
 * for its frames (20 frames at speed 10 when fixedTimingFlag is set). Then sefRunCompactedEffects runs. */
void sefReplayBufferedEffects(void);

#endif /* INCLUDE_MAIN_TU119_H */

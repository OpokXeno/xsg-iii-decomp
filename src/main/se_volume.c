#include "common.h"
#include "shared.h"

/* Sound levels run from 0 to SOUND_LEVEL_MAX. */
#define SOUND_LEVEL_MAX 0x7F

#include "main/tu018.h"

int seVolumeCurve(int level);
int seVolumeRampSet(VolumeRamp *ramp, int level, int levelPerStep);
int seVolumeRampStep(VolumeRamp *ramp);

extern short seVolumeCurveTable[36];

int seVolumeCurve(int level)
{
    int fraction = level & 3;
    int curveValue;

    if (level < SOUND_LEVEL_MAX && fraction != 0) {
        int lowerCurvePoint = seVolumeCurveTable[level >> 2];
        int upperCurvePoint = seVolumeCurveTable[(level + 4) >> 2];

        curveValue = lowerCurvePoint + (((upperCurvePoint - lowerCurvePoint) * fraction) >> 2);
    } else {
        curveValue = seVolumeCurveTable[level >> 2];
        if (level >= SOUND_LEVEL_MAX) {
            curveValue = 0;
        }
    }
    return curveValue;
}

/* Starts a ramp toward level, moving by step per seVolumeRampStep call (in the right
 * direction); step 0 jumps there at once and returns the new level, otherwise -1. The
 * level is read back as the stored (8-bit) target. */
int seVolumeRampSet(VolumeRamp *ramp, int level, int levelPerStep)
{
    ramp->target = level;
    if (levelPerStep == 0) {
        ramp->currentLevel = level;
        ramp->levelDelta = 0;
        return ramp->target;
    }
    if (ramp->target == ramp->currentLevel) {
        return -1;
    }
    if (ramp->target < ramp->currentLevel) {
        ramp->levelDelta = -levelPerStep;
    } else {
        ramp->levelDelta = levelPerStep;
    }
    return -1;
}

/* One ramp step: moves current by step and stops at the target (or at the ends of the level
 * range); returns the new level, or -1 when the ramp is not moving. */
int seVolumeRampStep(VolumeRamp *ramp)
{
    int levelDelta;
    int nextLevel;

    levelDelta = ramp->levelDelta;
    if (levelDelta != 0) {
        nextLevel = ramp->currentLevel;
        nextLevel += levelDelta;
        if ((levelDelta < 0 && nextLevel < ramp->target) || (levelDelta > 0 && nextLevel > ramp->target)
            || nextLevel < 0 || nextLevel > SOUND_LEVEL_MAX) {
            nextLevel = ramp->target;
            ramp->levelDelta = 0;
        }
        ramp->currentLevel = nextLevel;
        return nextLevel;
    }
    return -1;
}

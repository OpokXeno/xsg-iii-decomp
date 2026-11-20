#include "common.h"
#include "shared.h"

void LOG(const void *data, const char *format, ...);
void svSceneFlagsSet(unsigned int mask);
void svSceneFlagsClear(unsigned int mask);

/* Global flag word; sv_get_image_item_161 also reads it with masks. */
extern u32 svSceneFlags;

/* The light set (XglLightSet, xgl_2.h) as lightSetLogDump walks it: runs of 7, 8, 3, 3 and 3 vectors
 * that cross XglLightSet's field boundaries (the directions run includes viewDirection, the colours
 * run the two ambient colours, the shadow run forward and reserved). */
typedef struct LightSet {
    Vector4 direction[7];       /* +0x000 */
    Vector4 color[8];           /* +0x070 */
    Vector4 shadow[3];          /* +0x0F0 */
    Vector4 pointPosition[3];   /* +0x120 */
    Vector4 pointColor[3];      /* +0x150 */
} LightSet;

/* "sDir", "sCol", "sSha", "sPntP", "sPntC" */
extern const char lightLogDirectionLabel[];

extern const char lightLogColorLabel[];
extern const char lightLogShadowLabel[];
extern const char lightLogPointPositionLabel[];
extern const char lightLogPointColorLabel[];

void svSceneFlagsSet(u32 mask) {
    svSceneFlags |= mask;
}

void svSceneFlagsClear(u32 mask) {
    svSceneFlags &= ~mask;
}

void svSceneFlagsToggle(u32 mask) {
    svSceneFlags ^= mask;
}

/* Logs every vector of the light set with its label and index (LOG does nothing in this build). */
void lightSetLogDump(LightSet *lights)
{
    int i;

    for (i = 0; i < 7; i++) {
        LOG(&lights->direction[i], lightLogDirectionLabel, i);
    }
    for (i = 0; i < 8; i++) {
        LOG(&lights->color[i], lightLogColorLabel, i);
    }
    for (i = 0; i < 3; i++) {
        LOG(&lights->shadow[i], lightLogShadowLabel, i);
    }
    for (i = 0; i < 3; i++) {
        LOG(&lights->pointPosition[i], lightLogPointPositionLabel, i);
    }
    for (i = 0; i < 3; i++) {
        LOG(&lights->pointColor[i], lightLogPointColorLabel, i);
    }
}

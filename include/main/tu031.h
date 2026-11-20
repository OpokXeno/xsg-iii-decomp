#ifndef INCLUDE_MAIN_TU031_H
#define INCLUDE_MAIN_TU031_H

/* Types owned by main/tu031 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"

/* Light key as this TU reads it: offsets 0x00-0x48 read here, 0x44 is not read. */
typedef struct XglLightKey {
    DoublewordAlignedVector4 position; /* +0x00 */
    Vector4 rotation;                  /* +0x10: x, y, z angles */
    Vector4 extent;                    /* +0x20 */
    Vector4 color;                     /* +0x30 */
    float coneAngle;                   /* +0x40 */
    float penumbra;                    /* +0x44: read through fabs by the cone light test */
    float intensity;                   /* +0x48 */
} XglLightKey;

#endif

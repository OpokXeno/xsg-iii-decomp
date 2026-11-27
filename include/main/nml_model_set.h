#ifndef INCLUDE_MAIN_NML_MODEL_SET_H
#define INCLUDE_MAIN_NML_MODEL_SET_H

#include "shared.h"

typedef int s32;

typedef struct XglPacket XglPacket;

void pxySetDrawCallback(s32 drawCallback);
/* Init of the pxy module (xgl_render.c runs it). */
void nmlModelConstruct(void);
/* Queues a direct send; bit 0 of key keeps the data in place, key % 12 / 2 is the group, key / 12 the row. */
void nmlModelDirectSendFull(int key, void *packetData, int qwc, int sendFlag, int sendState);
/* The value of a key track (x time, y value) at time. */
float pxyInterpolateKeys(int count, Vector4 *keyframes, float time, float weight);

#endif /* INCLUDE_MAIN_NML_MODEL_SET_H */

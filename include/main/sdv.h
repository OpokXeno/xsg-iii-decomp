/* header_harvest: hand-maintained (X3 express integration; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_MAIN_SDV_H
#define INCLUDE_MAIN_SDV_H

/* The camera pad override sdv fills (func_00193850) and the camera pad updates of xgl_camera.c read
 * instead of the pads while it is active; sdvInitSpecialWork clears it. */
typedef struct CameraPadOverride {
    unsigned short held;
    unsigned short pressed;
    unsigned short repeated;
    unsigned char unmodeled_06[2];
    signed char rightX;
    signed char rightY;
    signed char leftX;
    signed char leftY;
    unsigned int active;
} CameraPadOverride;

extern CameraPadOverride sdvSpecialBuf;

#endif

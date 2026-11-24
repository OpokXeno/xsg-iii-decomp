#ifndef INCLUDE_OV02_TU097_H
#define INCLUDE_OV02_TU097_H

/* Robo unit record: the slot index at +0xE4. */
struct RoboCommandUnit {
    unsigned char unk_00[0xE4];
    int primarySlot;
};

/* The studio camera: the value at +0x04 is set to 4 when the camera hold is released. */
struct StudioCamera {
    unsigned char unk_00[0x4];
    int cameraState;
};

#endif /* INCLUDE_OV02_TU097_H */

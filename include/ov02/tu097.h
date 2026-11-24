#ifndef INCLUDE_OV02_TU097_H
#define INCLUDE_OV02_TU097_H

/* Robo unit record: the slot index at +0xE4. */
struct RoboCommandUnit {
    unsigned char unk_00[0xE4];
    int primarySlot;
};

#endif /* INCLUDE_OV02_TU097_H */

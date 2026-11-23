#ifndef INCLUDE_MAIN_TU075_H
#define INCLUDE_MAIN_TU075_H

typedef int s32;

typedef float f32;

typedef struct RgDispActor {
    unsigned char unk_00[0x24];
    union { /* +0x24: kept as a union: alias set 0 decides the emitted reloads */
        f32 offsetY;
        s32 offsetYBits;
    };
    unsigned char unk_28[0x54 - 0x28];
    f32 sceneValue; /* +0x54, copied from the scene block */
} RgDispActor;

/* One display-model slot as this TU reads it. */
struct RgDispModel {
    s32 modelBits; /* +0x00, bit 0x40 requests a table reset */
    unsigned char unk_04[0x18];
    s32 updateFnAddr; /* +0x1C, address of the per-frame update function, kept as an integer word */
    unsigned char unk_20[0x4];
    union { /* +0x24: an actor id, or the actor pointer (same word) */
        s32 actorId;
        RgDispActor *actor;
    };
    s32 argAt28; /* +0x28, passed to sbNativeHandlerMessagePost */
    unsigned char unk_2C[0x40 - 0x2C];
    f32 position[3]; /* +0x40 */
    unsigned char unk_4C[0x12C - 0x4C];
    f32 effectScale; /* +0x12C */
    f32 effectAlpha; /* +0x130 */
    f32 range; /* +0x134 */
    unsigned char unk_138[0x150 - 0x138];
    RgDispModelState block; /* +0x150 */
};

#endif /* INCLUDE_MAIN_TU075_H */

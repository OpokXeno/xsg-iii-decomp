#ifndef INCLUDE_MAIN_TU267_H
#define INCLUDE_MAIN_TU267_H

typedef int s32;

typedef signed char s8;

typedef struct { unsigned char bytes[8]; } TextTagOffset;

/* One text tag entry, 0x20 bytes, in the table at gTextTagTable (20 entries). */
struct TextTag {
    s32 key;                        /* +0x00: the caller key, 0 when the entry is free */
    s8 tagKind;                        /* +0x04 */
    unsigned char unk_05[0x3];
    s32 frames;                     /* +0x08 */
    unsigned char unk_0C[0x4];
    TextTagOffset placementOffset;           /* +0x10: copied from the caller's 8-byte placementOffset */
    unsigned char unk_18[0x8];
};

#endif /* INCLUDE_MAIN_TU267_H */

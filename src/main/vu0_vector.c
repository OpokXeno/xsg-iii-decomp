#include "common.h"
#include "shared.h"

#include "quadword.h"
/* Row-major 4x4 float matrix. */
void vu0NormalizeVf20(void);
void vu0CurMatrixMul(Matrix4 matrix);
void vu0CurMatrixMulRight(Vector4 *rows);

typedef int s32;

/* The effect owner record: the 32 effect slot numbers of the owner sit at +0xFC. */
typedef struct SefEffectOwner {
    unsigned char unk_00[0xFC];
    short slotIds[32];
} SefEffectOwner;

/* One 0x6C-byte effect slot: its id at +0x60 and its flags at +0x64. */
typedef struct SefEffectSlot {
    unsigned char unk_00[0x60];
    int effectId;
    unsigned int effectFlags;
} SefEffectSlot;

SefEffectSlot *sefGetEffectSlot(s32 index);

s32 countFlaggedSefEntries(SefEffectOwner *owner) {
    short *idCursor;
    short effectSlotIndex;
    s32 flaggedCount;
    s32 i;
    SefEffectSlot *effectSlot;

    flaggedCount = 0;
    if (owner != 0) {
        idCursor = owner->slotIds;
        for (i = 0; i < 32; i++) {
            effectSlotIndex = *idCursor;
            idCursor += 1;
            effectSlot = sefGetEffectSlot(effectSlotIndex);
            if ((effectSlot != 0) && (effectSlot->effectId != 0)) {
                if (effectSlot->effectFlags & 2) {
                    flaggedCount = flaggedCount + 1;
                }
            }
        }
    }
    return flaggedCount;
}

/* Copies the vector source into destination as one quadword, then sets its w to w. */
void vu0CopyVectorSetW(Vector4 *destinationVector, Vector4 *sourceVector, float w) {
    QuadwordCopyT0(destinationVector, sourceVector);
    destinationVector->w = w;
}

/* Normalizes vf20 in place (xyz divided by its length) and sets w to 1. */
void vu0NormalizeVf20(void) {
    __asm__ __volatile__(
        "vmul.xyz vf1, vf20, vf20\n"
        "vaddy.x vf1, vf1, vf1y\n"
        "vaddz.x vf1, vf1, vf1z\n"
        "vrsqrt Q, vf0w, vf1x\n"
        "vmove.w vf20, vf0\n"
        "vwaitq\n"
        "vmulq.xyz vf20, vf20, Q\n" : : : "memory");
}

void vu0NormalizeVector(Vector4 *vector) {
    __asm__ __volatile__("lqc2 vf20, 0(%0)" : : "r"(vector) : "memory");
    vu0NormalizeVf20();
}

void vu0CurMatrixMul(Matrix4 matrix) {
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("lqc2 vf2, 16(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("lqc2 vf3, 32(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("lqc2 vf4, 48(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("vmulax.xyzw ACC, vf26, vf1x" : : : "memory");
    __asm__ __volatile__("vmadday.xyzw ACC, vf27, vf1y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyzw ACC, vf28, vf1z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyzw vf1, vf29, vf1w" : : : "memory");
    __asm__ __volatile__("vmulax.xyzw ACC, vf26, vf2x" : : : "memory");
    __asm__ __volatile__("vmadday.xyzw ACC, vf27, vf2y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyzw ACC, vf28, vf2z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyzw vf2, vf29, vf2w" : : : "memory");
    __asm__ __volatile__("vmulax.xyzw ACC, vf26, vf3x" : : : "memory");
    __asm__ __volatile__("vmadday.xyzw ACC, vf27, vf3y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyzw ACC, vf28, vf3z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyzw vf3, vf29, vf3w" : : : "memory");
    __asm__ __volatile__("vmulax.xyzw ACC, vf26, vf4x" : : : "memory");
    __asm__ __volatile__("vmadday.xyzw ACC, vf27, vf4y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyzw ACC, vf28, vf4z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyzw vf29, vf29, vf4w" : : : "memory");
    __asm__ __volatile__("vmove.xyzw vf26, vf1" : : : "memory");
    __asm__ __volatile__("vmove.xyzw vf27, vf2" : : : "memory");
    __asm__ __volatile__("vmove.xyzw vf28, vf3" : : : "memory");
}

/* Current matrix = current x M, M given by four rows (vf26-vf29 are the current matrix). */
void vu0CurMatrixMulRight(Vector4 *rows) {
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(rows) : "memory");
    __asm__ __volatile__("lqc2 vf2, 16(%0)" : : "r"(rows) : "memory");
    __asm__ __volatile__("lqc2 vf3, 32(%0)" : : "r"(rows) : "memory");
    __asm__ __volatile__("lqc2 vf4, 48(%0)" : : "r"(rows) : "memory");
    __asm__ __volatile__("vmulax.xyzw ACC, vf1, vf26x" : : : "memory");
    __asm__ __volatile__("vmadday.xyzw ACC, vf2, vf26y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyzw ACC, vf3, vf26z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyzw vf26, vf4, vf26w" : : : "memory");
    __asm__ __volatile__("vmulax.xyzw ACC, vf1, vf27x" : : : "memory");
    __asm__ __volatile__("vmadday.xyzw ACC, vf2, vf27y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyzw ACC, vf3, vf27z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyzw vf27, vf4, vf27w" : : : "memory");
    __asm__ __volatile__("vmulax.xyzw ACC, vf1, vf28x" : : : "memory");
    __asm__ __volatile__("vmadday.xyzw ACC, vf2, vf28y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyzw ACC, vf3, vf28z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyzw vf28, vf4, vf28w" : : : "memory");
    __asm__ __volatile__("vmulax.xyzw ACC, vf1, vf29x" : : : "memory");
    __asm__ __volatile__("vmadday.xyzw ACC, vf2, vf29y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyzw ACC, vf3, vf29z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyzw vf29, vf4, vf29w" : : : "memory");
}

#include "common.h"
#include "shared.h"

/* Row-major 4x4 float matrix. */

int MMathMatrixBetweenVectors(Matrix4 *rotation, const Vector4 *fromDirection, const Vector4 *toDirection);

/* Builds in rotation the rotation that turns the unit vector from onto
 * the unit vector to: the half-angle quaternion (cross(from, to) / s, s / 2)
 * with s = sqrt(2 * (1 + dot(from, to))), expanded to a 4x4 matrix. When s is
 * below 0.01 (opposite vectors) it uses the half turn about y instead and
 * returns 0; otherwise returns 1. A hand-written VU0 macro-mode block; the
 * 0.01 threshold is the assembler's li.s literal. The original keeps an
 * untouched 16-byte frame: without the unused scratch quadword gcc drops the
 * addiu $sp pair (fd.sh: 57 insns for 59). */
int MMathMatrixBetweenVectors(Matrix4 *rotation, const Vector4 *fromDirection, const Vector4 *toDirection)
{
    int isRotatedHalfTurn;
    Vector4 frameScratch;

    __asm__ __volatile__(
        ".set noreorder\n"
        "lqc2 vf2, 0(%2)\n"
        "lqc2 vf3, 0(%3)\n"
        "vaddw.xyz vf7, vf0, vf0w\n"
        "vsub.xyzw vf1, vf0, vf0\n"
        "li.s $f9, 0.01\n"
        "vmula.xyz ACC, vf2, vf3\n"
        "vmaddaw.xy ACC, vf7, vf0w\n"
        "vmadd.xyz vf4, vf2, vf3\n"
        "viaddi vi1, vi0, 8\n"
        "vaddw.y vf1, vf0, vf0w\n"
        "addi %0, $0, 1\n"
        "vmulax.z ACC, vf7, vf4x\n"
        "vmadday.z ACC, vf7, vf4y\n"
        "vmadd.z vf4, vf7, vf4\n"
        "vopmula.xyz ACC, vf2, vf3\n"
        "vmfir.w vf6, vi1\n"
        "vsqrt Q, vf4z\n"
        "vopmsub.xyz vf5, vf3, vf2\n"
        "vitof4.w vf6, vf6\n"
        "vwaitq\n"
        "vaddq.x vf4, vf0, Q\n"
        "vrsqrt Q, vf0w, vf4z\n"
        "qmfc2.ni $8, vf4\n"
        "mtc1 $8, $f8\n"
        "abs.s $f8, $f8\n"
        "c.lt.s $f8, $f9\n"
        "vwaitq\n"
        "vaddq.y vf4, vf0, Q\n"
        "bc1tl 1f\n"
        "neg %0, $0\n"
        "vmulx.w vf1, vf6, vf4x\n"
        "vmuly.xyz vf1, vf5, vf4y\n"
        "1:\n"
        "vadd.xyzw vf2, vf1, vf1\n"
        "vaddw.xyzw vf9, vf0, vf0w\n"
        "vsub.xyzw vf8, vf0, vf0\n"
        "vmulz.xyzw vf5, vf2, vf1z\n"
        "vmuly.xyzw vf4, vf2, vf1y\n"
        "vmulx.xyzw vf3, vf2, vf1x\n"
        "vsub.xyzw vf7, vf0, vf0\n"
        "vsub.xyzw vf6, vf0, vf0\n"
        "vaddz.y vf10, vf4, vf5z\n"
        "vaddx.z vf11, vf5, vf3x\n"
        "vaddy.x vf12, vf3, vf4y\n"
        "vaddw.x vf8, vf5, vf4w\n"
        "vsubw.x vf7, vf4, vf5w\n"
        "vsuby.x vf6, vf9, vf10y\n"
        "vsubw.y vf8, vf5, vf3w\n"
        "vsubz.y vf7, vf9, vf11z\n"
        "vaddw.y vf6, vf3, vf5w\n"
        "vsubx.z vf8, vf9, vf12x\n"
        "vaddw.z vf7, vf4, vf3w\n"
        "vsubw.z vf6, vf3, vf4w\n"
        "sqc2 vf0, 0x30(%1)\n"
        "sqc2 vf8, 0x20(%1)\n"
        "sqc2 vf7, 0x10(%1)\n"
        "sqc2 vf6, 0(%1)\n"
        ".set reorder\n"
        : "=r"(isRotatedHalfTurn)
        : "r"(rotation), "r"(fromDirection), "r"(toDirection)
        : "$8", "$f8", "$f9", "memory");
    return isRotatedHalfTurn;
}

INCLUDE_ASM("asm/main/nonmatchings/m_se_obj", func_0025BA50);

INCLUDE_ASM("asm/main/nonmatchings/m_se_obj", func_0025BA70);

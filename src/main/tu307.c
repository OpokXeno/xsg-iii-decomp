#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu307", func_002869C0);

INCLUDE_ASM("asm/main/nonmatchings/tu307", func_00286A60);

typedef int s32;
typedef float f32;

void xzNoopCallback(void) {

}

s32 xzSegmentRangeTest(f32 *distanceRange, f32 *unitDirection, f32 *sweepFrom, f32 *sweepTo)
{
  __asm__ __volatile__("lqc2 vf20, 0(%0)" : : "r"(distanceRange) : "memory");
  __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(sweepFrom) : "memory");
  __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(sweepTo) : "memory");
  __asm__ __volatile__("vmul.xyzw vf21, vf20, vf20" : : : "memory");
  __asm__ __volatile__("vsub.xyz vf11, vf1, vf2" : : : "memory");
  __asm__ __volatile__("vmul.xyz vf12, vf11, vf11" : : : "memory");
  __asm__ __volatile__("vaddz.x vf12, vf12, vf12z" : : : "memory");
  __asm__ __volatile__("vaddx.x vf20, vf0, vf12x" : : : "memory");
  __asm__ __volatile__("vaddz.y vf20, vf0, vf21z" : : : "memory");
  __asm__ __volatile__("sqc2 vf20, 0(%0)" : : "r"(distanceRange) : "memory");
  if (!(distanceRange[1] >= distanceRange[0]))
  {
    return 0;
  }
  __asm__ __volatile__("vsqrt Q, vf12x" : : : "memory");
  __asm__ __volatile__("vwaitq" : : : "memory");
  __asm__ __volatile__("vaddq.z vf13, vf0, Q" : : : "memory");
  __asm__ __volatile__("vdiv Q, vf0w, vf13z" : : : "memory");
  __asm__ __volatile__("vwaitq" : : : "memory");
  __asm__ __volatile__("vmulq.xyz vf3, vf11, Q" : : : "memory");
  __asm__ __volatile__("vsub.xyz vf14, vf20, vf13" : : : "memory");
  __asm__ __volatile__("vmulz.xyz vf4, vf3, vf14z" : : : "memory");
  __asm__ __volatile__("sqc2 vf4, 0(%0)" : : "r"(distanceRange) : "memory");
  __asm__ __volatile__("sqc2 vf3, 0(%0)" : : "r"(unitDirection) : "memory");
  return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu307", func_00286B28);

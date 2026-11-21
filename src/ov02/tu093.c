/*
 * OV02 original TU 93: 0x00ab2f40..0x00ab3200 (8 functions)
 */
#include "common.h"

typedef int s32;
typedef float f32;
typedef unsigned char u8;

/* The status block at gRoboValidValueBlock (the same layout as in src/ov02/tu034.c). */
typedef struct Ov02StatusBlock {
    unsigned char unk00[0x18];
    s32 flags18;
    unsigned char unk1C[0x8];
    s32 value24;
} Ov02StatusBlock;

/* The unit fields this file writes: a ratio pair at +0xC0 and +0xC4. */
typedef struct RoboUnitRatios {
    u8 unk_00[0xC0];
    f32 ratio;
    f32 ratioShadow;
} RoboUnitRatios;

void *roboCurrentBattleUnitGet(s32 unitId);
void func_00AADDA0(void *battleUnit, s32 value);
s32 vmArgGet();

s32 roboUnitSetValue(s32 vm) {
    void *battleUnit;
    s32 unit_id;
    s32 value;

    unit_id = vmArgGet(vm, 1);
    battleUnit = roboCurrentBattleUnitGet(unit_id);
    value = vmArgGet(vm, 2);
    func_00AADDA0(battleUnit, value);
    return 1;
}

extern Ov02StatusBlock gRoboValidValueBlock;

s32 roboStatusGetValue(void)
{
  if (!(gRoboValidValueBlock.flags18 & 0x20))
  {
    return -1;
  }
  return gRoboValidValueBlock.value24;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu093", func_00AB2FC0);

void seVoiceSetCurrent(s32 newValue);

void roboSeVoiceCurrentClear(void) {
    seVoiceSetCurrent(0);
}

s32 roboBattleStateBlockGet();
void actorSetPosXY(s32 actorId, f32 yaw, f32 pitch);

void roboSetAngles(s32 vm)
{
  f32 yaw_degrees;
  f32 pitch_degrees;
  float scale;
  s32 actorId;
  yaw_degrees = vmArgGet(vm, 1);
  pitch_degrees = vmArgGet(vm, 2);
  scale = 100.0f;
  actorId = roboBattleStateBlockGet();
  actorSetPosXY(actorId, ((yaw_degrees / scale) / 180.0f) * 3.1415927f, ((pitch_degrees / 100.0f) / 180.0f) * 3.1415927f);
}

void roboOrderSetCurrentUnit();

void roboVmSelectOrderUnit(s32 vm) {
    s32 unit_id;
    unit_id = vmArgGet(vm, 1);
    roboOrderSetCurrentUnit(unit_id);
}

s32 func_00AA6F08();

s32 roboUnitIsActive(s32 vm) {
    s32 unit_id;
    void *battleUnit;
    unit_id = vmArgGet(vm, 1);
    battleUnit = roboCurrentBattleUnitGet(unit_id);
    return func_00AA6F08(battleUnit) != 0;
}

s32 battleActorHasUnitHandle();

s32 roboUnitSetRatio(s32 vm) {
    f32 level;
    s32 unit_id;
    s32 percent;
    RoboUnitRatios *unitRatios;

    unit_id = vmArgGet(vm, 1);
    percent = vmArgGet(vm, 2);
    unitRatios = roboCurrentBattleUnitGet(unit_id);
    if (battleActorHasUnitHandle(unitRatios) != 0) {
        level = percent / 100.0f;
        unitRatios->ratioShadow = level;
        unitRatios->ratio = level;
    }
    return 1;
}

#include "common.h"

typedef struct { unsigned char bytes[8]; } SingletonSource;
typedef int s32;
typedef unsigned char u8;

typedef struct SingletonIdState {
    /* 0x00 */ u8 primaryDest[0x10];
    /* 0x10 */ u8 gateDest[0x10];
    /* 0x20 */ u8 secondaryDest[0x10];
    /* 0x30 */ s32 primaryResult;
    /* 0x34 */ s32 secondaryResult;
    /* 0x38 */ s32 valid;
    /* 0x3C */ s32 singletonKind;
    /* 0x40 */ u8 primarySource[0x10];
    /* 0x50 */ u8 gateSource[0x10];
    /* 0x60 */ u8 secondarySource[0x10];
    /* 0x70 */ SingletonSource sourceSnapshot;
    /* 0x78 */ u8 unk78[8];
} SingletonIdState;

s32 func_00271EE8();
s32 func_00271F50();
s32 func_002722E0();
s32 func_002723F8();
s32 func_002725E0();
s32 func_00272650();
s32 memset();

s32 singletonIdStateBuild(SingletonIdState *singletonState, s32 singletonId, SingletonSource *singletonSource, s32 singletonKind)
{
  s32 primaryValue;
  s32 gateValue;
  s32 secondaryValue;
  s32 primaryResult;
  s32 secondaryResult;
  u8 *primarySourceBlock;
  u8 *gateDestBlock;
  u8 *secondarySourceBlock;
  u8 *secondaryDestBlock;
  s32 *primaryValuePtr;
  u8 *gateSourceBlock;
  memset(singletonState, 0, 0x80);
  singletonState->singletonKind = singletonKind;
  if (singletonSource != 0)
  {
    primaryValuePtr = &primaryValue;
    func_002725E0(singletonId);
    if (func_00271EE8(singletonSource) == 0)
    {
      gateSourceBlock = singletonState->gateSource;
      if (func_00271F50(gateSourceBlock, &gateValue, singletonSource, 1) != 0)
      {
        primarySourceBlock = singletonState->primarySource;
        secondarySourceBlock = singletonState->secondarySource;
        gateDestBlock = singletonState->gateDest;
        secondaryDestBlock = singletonState->secondaryDest;
        func_00271F50(primarySourceBlock, primaryValuePtr, singletonSource, 0);
        func_00271F50(secondarySourceBlock, &secondaryValue, singletonSource, 2);
        func_002722E0(singletonState, primarySourceBlock, primaryValue);
        func_002723F8(singletonState, primaryValue, 0);
        func_002722E0(gateDestBlock, gateSourceBlock, gateValue);
        func_002723F8(gateDestBlock, gateValue, 1);
        func_002722E0(secondaryDestBlock, secondarySourceBlock, secondaryValue);
        func_002723F8(secondaryDestBlock, secondaryValue, 0);
        primaryResult = func_00272650(primaryValue);
        singletonState->primaryResult = primaryResult;
        secondaryResult = func_00272650(secondaryValue);
        singletonState->sourceSnapshot = *singletonSource;
        singletonState->secondaryResult = secondaryResult;
        singletonState->valid = 1;
        return 1;
      }
    }
  }
  singletonState->valid = 0;
  return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu291", func_00271EE8);

INCLUDE_ASM("asm/main/nonmatchings/tu291", func_00271F50);

INCLUDE_ASM("asm/main/nonmatchings/tu291", func_002722E0);

INCLUDE_ASM("asm/main/nonmatchings/tu291", func_002723F8);

INCLUDE_ASM("asm/main/nonmatchings/tu291", func_002725E0);

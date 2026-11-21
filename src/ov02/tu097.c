/*
 * OV02 original TU 97: 0x00ab6770..0x00ab78e8 (26 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu097", func_00AB6770);

INCLUDE_ASM("asm/nonmatchings/ov02/tu097", func_00AB67E0);

typedef int s32;
typedef float f32;
typedef unsigned char u8;

/* Robo unit record: the slot index at +0xE4. */
struct RoboCommandUnit {
    unsigned char unk_00[0xE4];
    int primarySlot;
};

/* Robo unit slot: three float fields at +0x24, +0x34 and +0x44. */
struct RoboUnitDisplacement {
    unsigned char unk_00[0x24];
    f32 offsetX;
    unsigned char unk_28[0xC];
    f32 offsetY;
    unsigned char unk_38[0xC];
    f32 offsetZ;
};

void battleHookNoop(void) {

}

INCLUDE_ASM("asm/nonmatchings/ov02/tu097", func_00AB6848);

INCLUDE_ASM("asm/nonmatchings/ov02/tu097", func_00AB6938);

INCLUDE_ASM("asm/nonmatchings/ov02/tu097", func_00AB69F0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu097", func_00AB6B40);

INCLUDE_ASM("asm/nonmatchings/ov02/tu097", func_00AB6BF0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu097", func_00AB6D28);

extern int gRoboStartupStage[];
int func_001B60B8();
void func_001B6798();
#include "main/xc_decode.h"
int func_001B7190();
void runWithStateBlockOnce();
void xglSleep();

void roboRunStartupRequests(void) {
    int startupPacket[8];
    int firstStartupHandle;
    int thirdStartupHandle;

    if (gRoboStartupStage[0] == 2) {
        xcBlockClear(&startupPacket[0]);
        startupPacket[1] = 0x09010001;
        firstStartupHandle = func_001B7190(&startupPacket[0]);
        xcBlockClear(&startupPacket[0]);
        startupPacket[1] = 0x09010002;
        func_001B7190(&startupPacket[0]);
        xcBlockClear(&startupPacket[0]);
        startupPacket[1] = 0x09000004;
        thirdStartupHandle = func_001B7190(&startupPacket[0]);
        while (func_001B60B8(3) > 0) {
            xglSleep();
        }
        runWithStateBlockOnce(1);
        func_001B6798(firstStartupHandle);
        func_001B6798(firstStartupHandle);
        func_001B6798(thirdStartupHandle);
    }
}

int func_00AA07D8();

void roboUnitListReset(void) {
    func_00AA07D8();
}

void roboSlotNoop(void) {

}

INCLUDE_ASM("asm/nonmatchings/ov02/tu097", func_00AB7210);

extern char D_00ABCC50[];
int ov02AlwaysReady();
#include "ov02/robo_battle.h"
int battleActorHasUnitHandle(void *unitActor);
int func_00AA4890(void *unitActor);
int battleActorPostResetEvents(void *unitActor);
void publishEntryRecords(void);

void roboUnitsRefreshActors(void) {
    int battleUnitIndex;
    void *unitActor;

    ov02AlwaysReady();
    for (battleUnitIndex = 0; battleUnitIndex < 0xD; battleUnitIndex++) {
        unitActor = roboBattleUnitGet(battleUnitIndex, D_00ABCC50, 0x59C);
        if (battleActorHasUnitHandle(unitActor) != 0) {
            battleActorPostResetEvents(unitActor);
            func_00AA4890(unitActor);
        }
    }
    publishEntryRecords();
}

int func_00AA1928();

f32 roboCurrentUnitDisplacementClear(void) {
    struct RoboCommandUnit *currentCommandUnit;
    struct RoboUnitDisplacement *primarySlotDisplacement;
    int currentUnitId;

    currentUnitId = func_00AA1928();
    currentCommandUnit = roboBattleUnitGet(currentUnitId, D_00ABCC50, 0x5AA);
    primarySlotDisplacement = roboBattleUnitGet(currentCommandUnit->primarySlot, D_00ABCC50, 0x5AB);
    if (primarySlotDisplacement != 0) {
        primarySlotDisplacement->offsetZ = 0.0f;
        primarySlotDisplacement->offsetY = 0.0f;
        primarySlotDisplacement->offsetX = 0.0f;
    }
    return 0.0f;
}

void roboCurrentUnitDisplacementSetFixed(void)
{
    struct RoboCommandUnit *currentCommandUnit;
    struct RoboUnitDisplacement *primarySlotDisplacement;
    int currentUnitId;

    currentUnitId = func_00AA1928();
    currentCommandUnit = roboBattleUnitGet(currentUnitId, D_00ABCC50, 0x5B7);
    primarySlotDisplacement = roboBattleUnitGet(currentCommandUnit->primarySlot, D_00ABCC50, 0x5B8);
    if (primarySlotDisplacement != 0)
    {
        primarySlotDisplacement->offsetZ = 30.0f;
        primarySlotDisplacement->offsetY = 30.0f;
        primarySlotDisplacement->offsetX = 30.0f;
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu097", func_00AB73F0);

/* The studio flag block at gStudioHoldState: sceneHoldFlag and cameraHoldFlag are toggled by the functions below. */
struct StudioHoldState {
    unsigned char unk_00[0x28];
    int sceneHoldFlag;
    int cameraHoldFlag;
};

/* The studio camera: the value at +0x04 is set to 4 when the camera hold is released. */
struct StudioCamera {
    unsigned char unk_00[0x4];
    int cameraState;
};

extern struct StudioHoldState gStudioHoldState[];
struct StudioCamera *xglStudioGetCamera2(int);

void studioToggleCameraHold(void)
{
    struct StudioHoldState *holdState;
    int isCameraHeld;

    holdState = gStudioHoldState;
    isCameraHeld = (holdState->cameraHoldFlag) == 0;
    holdState->cameraHoldFlag = isCameraHeld;
    if (isCameraHeld == 0)
    {
        xglStudioGetCamera2(0)->cameraState = 4;
    }
}

int soundSlotStop(int);

void studioReleaseSoundHold(void) {
    soundSlotStop(0);
}

void studioToggleSceneHold(void)
{
    struct StudioHoldState *holdState;

    holdState = gStudioHoldState;
    holdState->sceneHoldFlag = (holdState->sceneHoldFlag) == 0;
}

void studioUpdateNoop(void) {

}

void studioStepNoop(void) {

}

INCLUDE_ASM("asm/nonmatchings/ov02/tu097", func_00AB74D0);

void studioExitNoop(void) {

}

INCLUDE_ASM("asm/nonmatchings/ov02/tu097", func_00AB7538);

INCLUDE_ASM("asm/nonmatchings/ov02/tu097", func_00AB7848);

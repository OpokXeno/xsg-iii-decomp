#include "common.h"
#include "shared.h"

#include "main/tu124.h"


/* The camera request: its search bits and override id choose the camera. */
typedef struct CameraRequest {
    unsigned char unmodeled_00[40];
    int cameraSearchBits;                    /* +0x28: bits 8 and 9 select the camera search */
    int overrideId;                 /* +0x2c */
} CameraRequest;

/* A 64-bit state word; bit 4 set makes the selection return 0. */
extern unsigned long long gCameraSelectFlags[];
int func_0030DD10(void);
int scGetImmAdrImmIdx2(int scriptBase, unsigned short *cameraEntryTable, int index);
short scGetImmNumIdx(unsigned short *cameraEntryTable, int index);

int sdvSelectCamera(SdvCameraSchedule *schedule, int scriptBase, unsigned short *cameraEntryTable, CameraRequest *cameraRequest)
{
    int cameraWeights[5];
    int cameraScripts[5];
    int overrideIds[1];
    int cameraRoll;
    int selectedCamera;
    int cumulativeWeight;
    int i;
    int searchFlagBits;
    int useOverrideCamera;
    unsigned short *cameraChannelTable;

    selectedCamera = 0;
    cameraRoll = func_0030DD10() % 100;
    if (cameraEntryTable == 0 || scriptBase == 0) {
        return 0;
    }
    for (i = 0; i < 5; i++) {
        cameraScripts[i] = scGetImmAdrImmIdx2(scriptBase, cameraEntryTable, i * 2);
        cameraWeights[i] = scGetImmNumIdx(cameraEntryTable, i * 2 + 1);
    }
    useOverrideCamera = 0;
    searchFlagBits = cameraRequest->cameraSearchBits & 0x300;
    if (searchFlagBits == 0x300 || searchFlagBits == 0x200 || searchFlagBits == 0x100) {
        useOverrideCamera = 1;
    }
    /* The id list is terminated by its first non-positive entry; it starts with -1. */
    overrideIds[0] = -1;
    for (i = 0; overrideIds[i] > 0; i++) {
        if (cameraRequest->overrideId == overrideIds[i]) {
            useOverrideCamera = 1;
            break;
        }
    }
    if (((gCameraSelectFlags[0] >> 4) & 1) != 0) {
        return 0;
    }
    if (useOverrideCamera != 0 && cameraScripts[4] != 0) {
        selectedCamera = cameraScripts[4];
    } else {
        cumulativeWeight = 0;
        for (i = 0; i < 4; i++) {
            cumulativeWeight += cameraWeights[i];
            if (cameraRoll < cumulativeWeight && cameraScripts[i] != 0) {
                selectedCamera = cameraScripts[i];
                break;
            }
        }
    }
    if (selectedCamera != 0) {
        cameraChannelTable = (unsigned short *) selectedCamera;
        schedule->channels[0].keys = (short *) scGetImmAdrImmIdx2(scriptBase, cameraChannelTable, 0);
        schedule->channels[1].keys = (short *) scGetImmAdrImmIdx2(scriptBase, cameraChannelTable, 1);
        schedule->channels[2].keys = (short *) scGetImmAdrImmIdx2(scriptBase, cameraChannelTable, 2);
        schedule->channels[3].keys = (short *) scGetImmAdrImmIdx2(scriptBase, cameraChannelTable, 3);
    }
    schedule->camera = selectedCamera;
    schedule->animation = 0;
    return selectedCamera;
}

INCLUDE_ASM("asm/main/nonmatchings/sdv_camera", func_001EE4B0);

INCLUDE_ASM("asm/main/nonmatchings/sdv_camera", func_001EE658);

INCLUDE_ASM("asm/main/nonmatchings/sdv_camera", func_001EED70);

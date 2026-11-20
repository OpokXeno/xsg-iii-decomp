#include "common.h"
#include "shared.h"

#include "main/tu124.h"


/* The request whose flags and id choose the camera. */
typedef struct CameraRequest {
    unsigned char unmodeled_00[40];
    int flags;                      /* +0x28: bits 8 and 9 select the camera search */
    int id;                         /* +0x2c */
} CameraRequest;

/* A 64-bit state word; bit 4 set makes the selection return 0. */
extern unsigned long long D_00452D50[];
int func_0030DD10(void);
int scGetImmAdrImmIdx2(int base, unsigned short *table, int index);
short scGetImmNumIdx(short *table, int index);

int sdvSelectCamera(SdvCameraSchedule *schedule, int base, unsigned short *table, CameraRequest *request)
{
    int weights[5];
    int cameras[5];
    int ids[1];
    int roll;
    int camera;
    int total;
    int i;
    int flags;
    int selected;

    camera = 0;
    roll = func_0030DD10() % 100;
    if (table == 0 || base == 0) {
        return 0;
    }
    for (i = 0; i < 5; i++) {
        cameras[i] = scGetImmAdrImmIdx2(base, table, i * 2);
        weights[i] = scGetImmNumIdx((short *) table, i * 2 + 1);
    }
    selected = 0;
    flags = request->flags & 0x300;
    if (flags == 0x300 || flags == 0x200 || flags == 0x100) {
        selected = 1;
    }
    /* The id list is terminated by its first non-positive entry; it starts with -1. */
    ids[0] = -1;
    for (i = 0; ids[i] > 0; i++) {
        if (request->id == ids[i]) {
            selected = 1;
            break;
        }
    }
    if (((D_00452D50[0] >> 4) & 1) != 0) {
        return 0;
    }
    if (selected != 0 && cameras[4] != 0) {
        camera = cameras[4];
    } else {
        total = 0;
        for (i = 0; i < 4; i++) {
            total += weights[i];
            if (roll < total && cameras[i] != 0) {
                camera = cameras[i];
                break;
            }
        }
    }
    if (camera != 0) {
        schedule->channels[0].keys = (short *) scGetImmAdrImmIdx2(base, (unsigned short *) camera, 0);
        schedule->channels[1].keys = (short *) scGetImmAdrImmIdx2(base, (unsigned short *) camera, 1);
        schedule->channels[2].keys = (short *) scGetImmAdrImmIdx2(base, (unsigned short *) camera, 2);
        schedule->channels[3].keys = (short *) scGetImmAdrImmIdx2(base, (unsigned short *) camera, 3);
    }
    schedule->camera = camera;
    schedule->animation = 0;
    return camera;
}

INCLUDE_ASM("asm/main/nonmatchings/sdv_camera", func_001EE4B0);

INCLUDE_ASM("asm/main/nonmatchings/sdv_camera", func_001EE658);

INCLUDE_ASM("asm/main/nonmatchings/sdv_camera", func_001EED70);

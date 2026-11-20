#include "common.h"
#include "shared.h"

#include "main/xgl_2.h"

/* Row-major 4x4 float matrix. */








struct XglCamera;

typedef float RgVector[4];

/*
 * The light editor of Xenosaga I (DB_lightWrite, updateLight) indexes these
 * vectors as float[4] rows (component offsets fold into the row's index, not
 * the load displacement), so each vector also has its RgVector row.
 */
typedef struct StudioLightEntry {
    union { Vector4 color; RgVector color_row; };
    union { Vector4 direction; RgVector direction_row; };
} StudioLightEntry;

/* The light block of a studio record, also the light record of an Actor
 * (+0x510). Whole-block copies (game.c GameStateSave/RestoreCameraLight) are
 * ld/sd, while the Studio records holding it are only 8-aligned (xgl_studio
 * .bss): the block is 8-aligned, which the doubleword view of its first vector
 * gives. */
typedef struct StudioLight {
    union {
        struct {
            union { Vector4 ambient_color; RgVector ambient_color_row; u64 ambient_color_words[2]; };
            StudioLightEntry lights[3];
        };
        Vector4 parameters[7];          /* the seven quadwords xglLightCalcMatrix loads: ambient, then three colour/direction pairs */
    };
    float normal_matrix[4][4];          /* +0x70: map_1.c's MapDrawLight view */
    float color_modifier_matrix[4][4];  /* +0xb0: map_1.c's MapDrawLight view */
} StudioLight;

struct XglCamera *xglStudioGetCamera2(int camera_index);

/*
 * Layout the asm readers use: a studio record is 0x3180 bytes, its cameras
 * start at +0x180 and are 0x600 apart (xglStudioGetCamera2, xglStudioGetActiveCamera,
 * xglStudioSelectGetActiveCamera and xglStudioGetCamera all compute
 * 3*i << 9). StudioLight (0xF0) plus the 0x90 bytes after it put the cameras at +0x180,
 * and XglCamera is 0x600 bytes (a 0x5F8 camera object plus padding).
 */
typedef struct Studio {
    StudioLight light;
    u8 unmodeled_f0[0x90];
    XglCamera cameras[8];
} Studio;

extern Studio *pCurrentStudio;

INCLUDE_ASM("asm/main/nonmatchings/xgl_studio", func_0019B6D8);

INCLUDE_ASM("asm/main/nonmatchings/xgl_studio", func_0019B700);

INCLUDE_ASM("asm/main/nonmatchings/xgl_studio", func_0019B708);

XglCamera *xglStudioGetActiveCamera(void)
{
    XglCamera *camera;
    int camera_index;

    for (camera_index = 0; camera_index < 2; camera_index++) {
        camera = xglStudioGetCamera2(camera_index);
        if (camera->active != 0)
            return camera;
    }
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_studio", xglStudioSelectGetActiveCamera);

INCLUDE_ASM("asm/main/nonmatchings/xgl_studio", xglStudioFlushActiveCamera);

INCLUDE_ASM("asm/main/nonmatchings/xgl_studio", func_0019B818);

INCLUDE_ASM("asm/main/nonmatchings/xgl_studio", func_0019B820);

INCLUDE_ASM("asm/main/nonmatchings/xgl_studio", func_0019B830);

XglCamera *xglStudioGetCamera2(int camera_index)
{
    return &pCurrentStudio->cameras[camera_index];
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_studio", xglStudioMainCameraInit);

INCLUDE_ASM("asm/main/nonmatchings/xgl_studio", func_0019B8B0);

INCLUDE_ASM("asm/main/nonmatchings/xgl_studio", func_0019B8D8);

INCLUDE_ASM("asm/main/nonmatchings/xgl_studio", func_0019B970);

void func_0019B8D8(void);
void xglStudioMainCameraInit(void);

void func_0019BA20(void)
{
    func_0019B8D8();
    xglStudioMainCameraInit();
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_studio", xglStudioUpdate);

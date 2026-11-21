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

/* One studio record slot of gStudioRecords (0xD80 bytes; the records are not Studio-sized). */
typedef struct StudioSlot {
    unsigned char unk_0000[0xD80];
} StudioSlot;

extern Studio *pCurrentStudio;

typedef int s32;
extern StudioSlot gStudioRecords[];

/* Makes studio record number studio_index (0xD80 bytes each in gStudioRecords) the current studio. */
void setCurrentStudio(s32 studio_index) {
    Studio *studio;

    studio = (Studio *) &gStudioRecords[studio_index];
    pCurrentStudio = studio;
}

extern s32 gSelectedStudioIndex;

void setSelectedStudioIndex(s32 studio_index) {
    gSelectedStudioIndex = studio_index;
}

void studioCameraAt(XglCamera **camera_out, s32 camera_index) {
    *camera_out = &pCurrentStudio->cameras[camera_index];
}

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

extern s32 gStudioSceneFlag;

void xglStudioClearFlag(void) {
    gStudioSceneFlag = 0;
}

void xglStudioSetFlag(void) {
    gStudioSceneFlag = 1;
}

Studio *xglStudioGetCurrent(void) {
    return pCurrentStudio;
}

XglCamera *xglStudioGetCamera2(int camera_index)
{
    return &pCurrentStudio->cameras[camera_index];
}

void setCurrentStudio();

void xglStudioMainCameraInit(void)
{
  XglCamera *camera;
  s32 initializedStudioCount;
  s32 studioIndex;
  initializedStudioCount = 0;
  do
  {
    studioIndex = initializedStudioCount;
    initializedStudioCount += 1;
    setCurrentStudio(studioIndex);
    camera = xglStudioGetCamera2(0);
    camera->active = 1;
    camera->cameraState = 1;
  }
  while (initializedStudioCount < 4);
  setCurrentStudio(0);
}

void xglStudioSetCurrentSelected(void) {
    setCurrentStudio(gSelectedStudioIndex);
    xglStudioGetCamera2(0);
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_studio", func_0019B8D8);

INCLUDE_ASM("asm/main/nonmatchings/xgl_studio", func_0019B970);

void func_0019B8D8(void);
void xglStudioMainCameraInit(void);

void xglStudioInit(void)
{
    func_0019B8D8();
    xglStudioMainCameraInit();
}

extern s32 gStudioEntrySkip;
void xglStudioSetCurrentSelected();
s32 func_0019B970();

void xglStudioUpdate(void) {
    if (gStudioEntrySkip == 0) {
        xglStudioSetCurrentSelected();
        func_0019B970();
    }
}

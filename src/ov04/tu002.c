#include "common.h"
#include "shared.h"

#include "main/xgl_2.h"

struct XglCamera;
struct XglCamera *xglStudioSelectGetActiveCamera(int studio_index);

void _CurMatrixSet(Matrix4 *matrix);


void _CurSetViewScaleTrans(Vector4 *scale, Vector4 *translation);

int _CurRotTransPersClipScreen(ScreenVector4 *screenOut, const Vector4 *point);

/* Two screen coordinates, scaled by 16 before uwamonoProjectToScreen halves them. */
typedef ScreenVector4 UwamonoScreenPos;

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A80930);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A816C8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A81940);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A81A40);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A81B10);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A81BB0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A81CD8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A81F68);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A822E8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A82548);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A82968);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A83890);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A839C8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A83A78);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A83C20);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A83E60);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A841B8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A84238);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A844A0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A844E8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A84788);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A847F8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A84870);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A84C90);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A85600);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A85828);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A85890);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A858F0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A86048);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A861C8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A862C0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A863B0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A863B8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A863C8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A863D8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A863E0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A863E8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu002", func_00A863F8);

/*
 * Sets up the active camera's vectors, then rescales the two screen coordinates in
 * position (x16) to centred values; _CurRotTransPersClipScreen can replace both with 0x1000.
 */
void uwamonoProjectToScreen(const Vector4 *query, UwamonoScreenPos *position) {
    XglCamera *camera;

    camera = xglStudioSelectGetActiveCamera(0);
    _CurMatrixSet(&camera->volume[0].viewMatrix);
    _CurSetViewScaleTrans(&camera->screen.screenScale, &camera->screen.screenOffset);
    if (_CurRotTransPersClipScreen(position, query) != 0) {
        position->x = 0x1000;
        position->y = 0x1000;
        return;
    }
    position->x = ((position->x >> 4) - 0x700) >> 1;
    position->y = ((position->y >> 4) - 0x720) >> 1;
}

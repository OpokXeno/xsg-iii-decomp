#include "common.h"
#include "main/xgl_2.h"

typedef int s32;
typedef float f32;
void _CurMatrixSet(Matrix4 *matrix);
void _CurSetViewScaleTrans(Vector4 *scale, Vector4 *translation);
XglCamera *xglStudioSelectGetActiveCamera(int studio_index);

void xglStudioSetActiveCameraMatrices(void) {
    XglCamera *camera;

    camera = xglStudioSelectGetActiveCamera(0);
    _CurMatrixSet(&camera->volume[0].viewMatrix);
    _CurSetViewScaleTrans(&camera->screen.screenScale, &camera->screen.screenOffset);
}

s32 _CurRotTransPersClipScreen(ScreenVector4 *screenOut, const Vector4 *point);

s32 xglProjectPointToScreen(f32 *outX, f32 *outY, const Vector4 *vertex)
{
  ScreenVector4 screenPosition;
  f32 screenX;
  f32 screenY;
  f32 offscreenCoord;

  if (_CurRotTransPersClipScreen(&screenPosition, vertex) != 0)
  {
    offscreenCoord = 0.0f;
    *outY = 0.0f;
    if (1)
    {
      *outX = offscreenCoord;
    }
    return 0;
  }
  screenX = screenPosition.x * 0.0625f + (-1792.0f);
  screenY = screenPosition.y * 0.0625f + (-1824.0f);
  *outX = screenX;
  *outY = screenY;
  return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu269", func_00268620);

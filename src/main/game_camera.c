#include "common.h"
#include "shared.h"

float xglAtan2(float y, float x);

void cameraAnglesBetween(Vector4 *angles, const Vector4 *from, const Vector4 *to);

/* Angles, in radians. */
#define CAMERA_PI 3.1415927f

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F3C0);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F438);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F458);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F550);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F558);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F5C0);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F688);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F6C8);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F6E8);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F718);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F728);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F750);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F798);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F7D0);

/* Writes the angles from `to` to `from`: x the pitch, y the yaw (plus pi), w the length of the difference. */
void cameraAnglesBetween(Vector4 *angles, const Vector4 *from, const Vector4 *to)
{
    float dx = from->x - to->x;
    float dy = from->y - to->y;
    float dz = from->z - to->z;

    angles->y = xglAtan2(dx, dz) + CAMERA_PI;
    dx = dx * dx;
    dz = dz * dz;
    angles->x = xglAtan2(dy, __builtin_sqrtf(dx + dz));
    angles->w = __builtin_sqrtf(dx + dy * dy + dz);
}

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F960);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F9A0);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F9D8);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F9E8);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025FA50);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025FA68);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025FB40);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025FB78);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025FC70);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025FD28);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025FD40);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025FD78);

#include "common.h"

typedef unsigned short u16;
typedef int s32;

void cycleStepNoop(void) {

}

void cycleStepIdle(void) {

}

extern u16 gMovieAutoSelectFlags[];
s32 channelTableSelectedEndGet();
s32 func_0028B268();

void cycleStepARange100(void)
{
  func_0028B268(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0xA : 1, 0, 0x64);
}

s32 func_0028B228();

void cycleStepBRange100(void)
{
  func_0028B228(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0xA : 1, 0, 0x64);
}

void cycleStepARange128(void)
{
  func_0028B268(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0xA : 1, 0, 0x80);
}

void cycleStepBRange128(void)
{
  func_0028B228(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0xA : 1, 0, 0x80);
}

void cycleStepARange255(void)
{
  func_0028B268(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0xA : 1, 0, 0xFF);
}

void cycleStepBRange255(void)
{
  func_0028B228(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0xA : 1, 0, 0xFF);
}

void cycleStepARange360(void)
{
  func_0028B268(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0xA : 1, 0, 0x168);
}

void cycleStepBRange360(void)
{
  func_0028B228(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0xA : 1, 0, 0x168);
}

void cycleStepARange448(void)
{
  func_0028B268(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0xA : 1, 0, 0x1C0);
}

void cycleStepBRange448(void)
{
  func_0028B228(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0xA : 1, 0, 0x1C0);
}

void cycleStepARange512(void)
{
  func_0028B268(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0xA : 1, 0, 0x200);
}

void cycleStepBRange512(void)
{
  func_0028B228(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0xA : 1, 0, 0x200);
}

void cycleStepASymRange512(void)
{
  func_0028B268(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0xA : 1, -0x200, 0x200);
}

void cycleStepBSymRange512(void)
{
  func_0028B228(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0xA : 1, -0x200, 0x200);
}

void cycleStepASymRange256(void)
{
  func_0028B268(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0xA : 1, -0x100, 0x100);
}

void cycleStepBSymRange256(void)
{
  func_0028B228(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0xA : 1, -0x100, 0x100);
}

void cycleStepASymRange128(void)
{
  func_0028B268(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0xA : 1, -0x80, 0x7F);
}

void cycleStepBSymRange128(void)
{
  func_0028B228(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0xA : 1, -0x80, 0x7F);
}

void cycleStepASymRange4095(void)
{
  func_0028B268(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0x10 : 1, -0xFFF, 0xFFF);
}

void cycleStepBSymRange4095(void)
{
  func_0028B228(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0x10 : 1, -0xFFF, 0xFFF);
}

void cycleStepARange65535(void)
{
  func_0028B268(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0x20 : 1, 0, 0xFFFF);
}

void cycleStepBRange65535(void)
{
  func_0028B228(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0x20 : 1, 0, 0xFFFF);
}

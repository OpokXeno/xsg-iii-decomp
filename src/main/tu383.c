#include "common.h"

typedef int s32;
s32 njStringSetColor(s32);

void subObjectTextSetColor(s32 textOwner) {
    njStringSetColor(textOwner + 0xD0);
}

s32 func_002D6AA8(s32);

void subObjectTextSetStyle(s32 textOwner) {
    func_002D6AA8(textOwner + 0xD0);
}

s32 njStringSetAlpha(s32);

void subObjectTextSetAlpha(s32 textOwner) {
    njStringSetAlpha(textOwner + 0xD0);
}

s32 njStringSetFlags(s32);

void subObjectTextSetFlags(s32 textOwner) {
    njStringSetFlags(textOwner + 0xD0);
}

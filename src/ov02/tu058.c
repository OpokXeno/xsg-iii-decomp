/*
 * OV02 original TU 58: 0x00a975d8..0x00a982e8 (14 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu058", func_00A975D8);

typedef unsigned char u8;
typedef int s32;
extern unsigned char gEmptyEntry[];
u8 * func_00A84D58();

u8 *ov02FindEntryOrDefaultA(s32 id) {
    u8 *entryOrEmpty;

    entryOrEmpty = func_00A84D58(id, 0);
    if (entryOrEmpty == 0) {
        entryOrEmpty = gEmptyEntry;
    }
    return entryOrEmpty;
}

u8 * func_00A84DD0();

u8 *ov02FindEntryOrDefaultB(s32 id) {
    u8 *entryOrEmpty;

    entryOrEmpty = func_00A84DD0(id, 0);
    if (entryOrEmpty == 0) {
        entryOrEmpty = gEmptyEntry;
    }
    return entryOrEmpty;
}

u8 * func_00A84CE0();

u8 *ov02FindEntryOrDefaultC(s32 id) {
    u8 *entryOrEmpty;

    entryOrEmpty = func_00A84CE0(id, 0);
    if (entryOrEmpty == 0) {
        entryOrEmpty = gEmptyEntry;
    }
    return entryOrEmpty;
}

u8 * func_00A84E48();

u8 *ov02FindEntryOrDefaultD(s32 id) {
    u8 *entryOrEmpty;

    entryOrEmpty = func_00A84E48(id, 0);
    if (entryOrEmpty == 0) {
        entryOrEmpty = gEmptyEntry;
    }
    return entryOrEmpty;
}

u8 * func_00A84EC0();

u8 *ov02FindEntryOrDefaultE(s32 id) {
    u8 *entryOrEmpty;

    entryOrEmpty = func_00A84EC0(id, 0);
    if (entryOrEmpty == 0) {
        entryOrEmpty = gEmptyEntry;
    }
    return entryOrEmpty;
}

u8 * func_00A84F38();

u8 *ov02FindEntryOrDefaultF(s32 id) {
    u8 *entryOrEmpty;

    entryOrEmpty = func_00A84F38(id, 0);
    if (entryOrEmpty == 0) {
        entryOrEmpty = gEmptyEntry;
    }
    return entryOrEmpty;
}

u8 * func_00A84FB0();

u8 *ov02FindEntryOrDefaultG(s32 id) {
    u8 *entryOrEmpty;

    entryOrEmpty = func_00A84FB0(id, 0);
    if (entryOrEmpty == 0) {
        entryOrEmpty = gEmptyEntry;
    }
    return entryOrEmpty;
}

extern s32 gEmptyMessageRecord;
s32 *func_00213700();

/* Message id 0 selects the shared empty message record; any other id goes to the message table. */
s32 *messageGetOrEmpty(s32 messageId) {
    s32 *messageRecord;

    if (messageId == 0) {
        return &gEmptyMessageRecord;
    }
    return messageRecord = func_00213700();
}

s32 func_002E9DD0(s32, s32, s32);
s32 func_0030E8E4(s32);
s32 func_00A981B8(s32);

/* Derives a key from seed, then checks that value matches the keyed expectation for it. */
s32 ov02ValidateKeyedValue(s32 keyedValue, s32 seed) {
    s32 recomputedValue;
    s32 key;
    s32 expected;

    key = func_00A981B8(seed);
    expected = func_0030E8E4(key);
    recomputedValue = func_002E9DD0(keyedValue, key, expected);
    return recomputedValue == func_0030E8E4(key);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu058", func_00A977D0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu058", func_00A97F60);

/* Splits a count of seconds into hours, minutes and seconds (0xE10 = 3600 s, 0x3C = 60 s). */
void splitSecondsToHms(s32 total_seconds, s32 *hours, s32 *minutes, s32 *seconds) {
    s32 remaining_seconds;
    s32 remaining_minutes_seconds;
    s32 seconds_part;
    s32 minutes_part;
    s32 hours_part;

    remaining_seconds = total_seconds;
    hours_part = remaining_seconds / 3600;
    *hours = hours_part;
    remaining_minutes_seconds = remaining_seconds - (hours_part * 0xE10);
    minutes_part = remaining_minutes_seconds / 60;
    *minutes = minutes_part;
    seconds_part = remaining_minutes_seconds - (minutes_part * 0x3C);
    *seconds = seconds_part;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu058", func_00A981B8);

#include "common.h"

typedef float f32;
typedef signed char s8;
typedef short s16;
typedef int s32;
typedef unsigned char u8;
typedef long long s64;
extern f32 gPointScratchX;
extern f32 gPointScratchY;

/* One 0x10-byte track key: the position sits at +0x04. */
typedef struct TrackKey {
    s32 unk_00;
    f32 position;
    u8 unk_08[0x8];
} TrackKey;

/* Object that a track actor points to at +0x30 (the owner). */
typedef struct TrackOwner {
    u8 unk_00[0x14];
    f32 radius;             /* +0x14 */
    u8 unk_18[0x24 - 0x18];
    f32 center;             /* +0x24 */
    u8 unk_28[0x6A0 - 0x28];
    s32 ownerValue;         /* +0x6A0 */
    u8 unk_6A4[0x6CC - 0x6A4];
    f32 boundRadius;        /* +0x6CC */
} TrackOwner;

/* Track actor. */
typedef struct TrackActor {
    u8 unk_00[0xC];
    f32 radiusCopy;         /* +0x0C */
    u8 unk_10[0x1C - 0x10];
    f32 radiusSquared;      /* +0x1C */
    s32 syncedValue;        /* +0x20 */
    s32 id;                 /* +0x24 */
    u8 unk_28[0x2C - 0x28];
    s32 actorFlags;         /* +0x2C */
    TrackOwner *owner;      /* +0x30 */
    u8 unk_34[0x4C - 0x34];
    TrackKey *track;        /* +0x4C: TrackKey array */
    u8 unk_50[0x6A - 0x50];
    s16 cursorA;            /* +0x6A */
    u8 unk_6C[0x6E - 0x6C];
    s16 cursorB;            /* +0x6E */
    u8 unk_70[0x86 - 0x70];
    s16 trackIndex;         /* +0x86 */
} TrackActor;

/* Record of 0xB0 bytes in the table at gPointSegmentTable. */
typedef struct PointSegment {
    u8 unk_00[0x40];
    s32 fromPoint;          /* +0x40 */
    s32 toPoint;            /* +0x44 */
    s32 pendingToPoint;     /* +0x48 */
    s32 linkedSegment;      /* +0x4C */
    u8 unk_50[0x68 - 0x50];
    s16 limit;              /* +0x68 */
    u8 unk_6A[0x7C - 0x6A];
    f32 scale;              /* +0x7C */
    s32 segmentBits;       /* +0x80: read as a 64-bit word by the length and scale paths */
    u8 unk_84[0x90 - 0x84];
    u8 subRecord[0x20];     /* +0x90 */
} PointSegment;

/* 0x40-byte entry in the table at gStreamChannelTable. */
typedef struct ChannelEntry {
    u8 unk_00[0x2C];
    s32 active;             /* +0x2C */
    u8 unk_30[0x10];
} ChannelEntry;

void pointScratchSet(f32 x, f32 y) {
    gPointScratchX = x;
    gPointScratchY = y;
}

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00275830);

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00275A10);

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00275B50);

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00276398);

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00276940);

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00276B00);

void pointScratchSet(f32 x, f32 y);
s32 func_00275B50();

void trackActorResetCursors(TrackActor *actor)
{
  f32 position;
  f32 center;
  f32 radius;
  TrackOwner *owner;
  pointScratchSet(0.0f, 1.0f);
  owner = actor->owner;
  center = owner->center;
  radius = owner->radius;
  position = actor->track[actor->trackIndex].position;
  if (((position <= (center + radius)) && ((center - radius) <= position)) && (func_00275B50(actor, 0x40000000) == 0))
  {
    actor->cursorB = 0;
    actor->cursorA = 0;
  }
}

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00276D58);

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00276E40);

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00277718);

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00277810);

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_002779A8);

extern s32 D_005C11C0[];
void func_002AA490();

void trackActorSyncOwnerState(TrackActor *actor) {
    f32 radius;
    f32 radiusSquared;
    f32 radiusCopy;
    s32 actorFlags;
    s32 ownerValue;
    s32 lookedUpSyncValue;
    TrackOwner *owner;

    actorFlags = actor->actorFlags;
    if (actorFlags & 0x200) {
        owner = actor->owner;
        if (owner != 0) {
            radius = owner->boundRadius;
            ownerValue = owner->ownerValue;
            radiusSquared = radius * radius;
            actor->syncedValue = ownerValue;
            actor->radiusSquared = radiusSquared;
            radiusCopy = owner->boundRadius;
            actor->radiusCopy = radiusCopy;
        }
    } else if (actorFlags & 0x100) {
        func_002AA490(actor->id, 0, D_005C11C0);
        lookedUpSyncValue = D_005C11C0[0];
        actor->syncedValue = lookedUpSyncValue;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00277D60);

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00277E48);

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00277F70);

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00278010);

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_002784C0);

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00278DE8);

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00279098);

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00279140);

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_002791F0);

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_002792D0);

extern PointSegment *gPointSegmentTable;

void pointSegmentClearFlags(s32 pointSegmentIndex) {
    gPointSegmentTable[pointSegmentIndex].segmentBits = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00279370);

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00279590);

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00279928);

f32 xglPointLength();

f32 slotRecordGetLength(s32 pointSegmentIndex) {
    f32 pointDistance;
    f32 linkLength;
    PointSegment *pointSegment;

    linkLength = 0.0f;
    pointSegment = &gPointSegmentTable[pointSegmentIndex];
    if ((*(s64 *) &pointSegment->segmentBits & 3) == 3) {
        pointDistance = xglPointLength(pointSegment->fromPoint, pointSegment->toPoint);

        linkLength = pointDistance;
    }
    return linkLength;
}

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00279AC0);

extern ChannelEntry *gStreamChannelTable;

void channelEntryClearActive(s32 channelIndex) {
    gStreamChannelTable[channelIndex].active = 0;
}

u8 *pointSegmentGetSubRecord(s32 pointSegmentIndex) {
    return gPointSegmentTable[pointSegmentIndex].subRecord;
}


INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00279DD8);

void slotRecordSetFlag800(s32 pointSegmentIndex, s32 isEnabled) {
    if (isEnabled != 0) {
        gPointSegmentTable[pointSegmentIndex].segmentBits |= 0x800;
        return;
    }
    gPointSegmentTable[pointSegmentIndex].segmentBits &= ~0x800;
}

void slotRecordCommitPending(s32 pointSegmentIndex) {
    s32 remainingBits;
    s32 committedTo;
    s32 segmentBits;
    segmentBits = gPointSegmentTable[pointSegmentIndex].segmentBits;
    if (segmentBits & 0x400) {
        committedTo = gPointSegmentTable[pointSegmentIndex].pendingToPoint;
        remainingBits = segmentBits & ~0xC00;
        gPointSegmentTable[pointSegmentIndex].segmentBits = remainingBits;
        gPointSegmentTable[pointSegmentIndex].toPoint = committedTo;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00279F20);

void slotRecordSetLimit(s32 pointSegmentIndex, s16 limit) {
    if (gPointSegmentTable[pointSegmentIndex].segmentBits & 2) {
        gPointSegmentTable[pointSegmentIndex].limit = limit;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu297", func_00279FB8);

void slotRecordSetScale(s32 pointSegmentIndex, f32 scale) {
    if ((*(s64 *) &gPointSegmentTable[pointSegmentIndex].segmentBits & 3) == 3) {
        gPointSegmentTable[pointSegmentIndex].scale = scale;
    }
}

s32 pointSegmentGetLinkedSegment(s32 pointSegmentIndex) {
    return gPointSegmentTable[pointSegmentIndex].linkedSegment;
}

#include "common.h"
#include "shared.h"
#include "main/tu173.h"
#include "main/tu176.h"
#include "main/tu178.h"
#include "main/sound_driver.h"

s32 xglFontGetFormattedWidth(const char *text);

/* Starts the Yes/No prompt: title text, box width (widened to the text width + 0x30), selection state = stepDelta > 0,
   the other answer's highlight at 0x80, fade-in 0 and phase 0x80 (as clampedCounterInit). */
void choicePromptInit(McGauge *gauge, const char *text, s32 stepDelta, s32 width)
{
  s32 isPositive;
  s32 textWidth;
  isPositive = stepDelta > 0;
  gauge->text = text;
  gauge->flag = 0x80;
  gauge->state = isPositive;
  gauge->negative = (1 - isPositive) << 7;
  gauge->width = width;
  gauge->fadeIn = 0;
  if (text != 0)
  {
    textWidth = xglFontGetFormattedWidth(text) + 0x30;
    if (gauge->width < textWidth)
    {
      gauge->width = textWidth;
    }
  }
}

void clampedCounterInit(McGauge *gauge, s32 stepDelta)
{
  s32 isPositive;
  isPositive = stepDelta > 0;
  gauge->flag = 0x80;
  gauge->state = isPositive;
  gauge->fadeIn = 0;
  gauge->negative = (1 - isPositive) << 7;
}

/* Header returned by getStatusHeader() (tu183.c); flags at +2. */
typedef struct StatusHeader {
    u8 unk00[2];
    u16 headerFlags;
} StatusHeader;

/* headerFlags bits that toggle the prompt's selection (up and down in the pad's bit order). */
#define CHOICE_PROMPT_TOGGLE_BUTTONS 0x5000

/* Signed as in the other TUs (choicePromptUpdate loads it with lb at 0x002214C4); counterEntryAdvancePhase reads it
   through a u8 cast, since without it the gate differed at 0x00221587 (lb instead of the original lbu). */
extern s8 gVsyncsPerFrame;

s32 statusHeaderIsReadyMode40(void);
s32 statusHeaderIsReadyMode20(void);
StatusHeader *getStatusHeader(void);

/* Steps the Yes/No prompt one frame. While active, confirm selects the second answer and returns 1, cancel returns
   1, and pad bits 0x5000 toggle the selection with a sound. Then the fade-in, the phase and the highlight of the
   other answer move by gVsyncsPerFrame * 0x10. */
s32 choicePromptUpdate(McGauge *gauge, s32 active)
{
  s32 decided;
  s32 step;
  s32 target;
  decided = 0;
  if (active)
  {
    if (statusHeaderIsReadyMode40())
    {
      gauge->state = 1;
      decided = 1;
    }
    else if (statusHeaderIsReadyMode20())
    {
      decided = 1;
    }
    else if (getStatusHeader()->headerFlags & CHOICE_PROMPT_TOGGLE_BUTTONS)
    {
      gauge->state = 1 - gauge->state;
      seVoiceStartDefault(1);
    }
  }
  step = gVsyncsPerFrame * 0x10;
  if (active && gauge->fadeIn >= 0)
  {
    gauge->fadeIn += step;
  }
  if ((u8)(gauge->flag - 1) < 0x7F)
  {
    gauge->flag -= step;
  }
  if (decided && gauge->flag == 0x80)
  {
    gauge->flag -= step;
  }
  target = (1 - gauge->state) << 7;
  if (target < (u8)gauge->negative)
  {
    gauge->negative -= step;
  }
  if ((u8)gauge->negative < target)
  {
    gauge->negative += step;
  }
  return decided;
}

void counterEntryAdvancePhase(McGauge *gauge)
{
  if (gauge->flag == 0x80)
  {
    gauge->flag = gauge->flag - ((u8)gVsyncsPerFrame * 0x10);
  }
}

/* Screen rectangle in GS coordinates (same layout as res_get_path.c's MenuRect; xglFontDrawBoxBlend's definition
   in tu162.c spells the parameter u16 *). */
typedef struct MenuRect {
    u16 x;
    u16 y;
    u16 x1;
    u16 y1;
} MenuRect;

int colorModulateByte(int sourceChannel, int scaleByte);
void fontQueueColorState(int mask);
void xglFontDrawBoxBlend(MenuRect *rect, s32 inset, s32 colour, s32 blendMode);
void fontQueueSprite(u16 startX, u16 startY, u16 endX, u16 endY);
void fontQueueLine(u16 startX, u16 startY, u16 endX, u16 endY);
void func_0021E9A8(u16 x, u16 y, u16 width, u16 height);
void xglFontPrintTitle(void);
void xglFontPrintEscapeByte(int escapeArgument);
void xglFontPrintExtendedColor(int red, int green, int blue);
void xglFontPrintAlphaCommand(int alphaMode, int alphaIndex, int alpha);
void xglFontSetColor(int red, int green, int blue);
void xglFontPrintIndexed(s32 x, s32 y, const char *text);
char *messageTextGet(s32 textId);
u8 statusByte0Get(void);

/* Message ids of the two answers; McGauge.state selects the first (0) or the second (1). */
#define CHOICE_PROMPT_FIRST_ANSWER_TEXT 0x74
#define CHOICE_PROMPT_SECOND_ANSWER_TEXT 0x75

/* Draws the Yes/No prompt box at row y: box, inner panel and three separator lines, the title text, the
   highlight of the selected answer (visible) or the pulsing highlights of both answers, then the two answer
   texts with the selected one in the highlight colour. left and top are the GS x/y of the box; the fade bytes
   fadeIn and negative are read unsigned here (lbu at 0x002215D0, 0x00221860 and 0x002218D0), so their colour
   math wraps in 0..0xFF. */
void choicePromptDraw(McGauge *gauge, s32 visible, s32 y)
{
  MenuRect rect;
  s32 alpha;
  s32 boxAlpha;
  s32 left;
  s32 top;

  alpha = colorModulateByte((u8) gauge->fadeIn, gauge->flag);
  if (alpha == 0)
  {
    return;
  }
  boxAlpha = colorModulateByte(0x73, alpha) & 0xFF;
  left = (0x100 - (gauge->width >> 1)) * 16 + 0x6FF8;
  top = (y << 4) + 0x71F8;
  rect.x1 = left + (u16) gauge->width * 16;
  rect.y1 = top + 0x530;
  rect.x = left;
  rect.y = top;
  fontQueueColorState(((u64) boxAlpha << 24) | 0x272120);
  xglFontDrawBoxBlend(&rect, 2, 0, 0);
  fontQueueColorState(((u64) boxAlpha << 24) | 0x554643);
  fontQueueSprite(rect.x + 0x40, rect.y + 0x40, rect.x1 - 0x40, rect.y + 0x1B0);
  rect.y += 0x1E0;
  fontQueueColorState(((u64) (u8) alpha << 24) | 0x3C312F);
  fontQueueLine(rect.x + 0x20, rect.y, rect.x1 - 0x20, rect.y);
  rect.y += 0x160;
  fontQueueLine(rect.x + 0x20, rect.y, rect.x1 - 0x20, rect.y);
  rect.y += 0x160;
  fontQueueLine(rect.x + 0x20, rect.y, rect.x1 - 0x20, rect.y);
  rect.y = top;
  xglFontPrintTitle();
  xglFontPrintEscapeByte(0x10);
  xglFontPrintExtendedColor(0, 0, 0);
  xglFontPrintAlphaCommand(0x44, 0, alpha);
  if (gauge->text)
  {
    xglFontSetColor(0x2D, 0x2D, 0x56);
    xglFontPrintIndexed(0x100 - (xglFontGetFormattedWidth(gauge->text) >> 1), ((rect.y - 0x71F8) >> 4) + 4,
                        gauge->text);
  }
  if (visible)
  {
    fontQueueColorState(((u64) (u8) colorModulateByte(0x73, alpha) << 24) | 0xF98980);
    func_0021E9A8(rect.x, rect.y + gauge->state * 0x160 + 0x1E0, gauge->width << 4, 0x160);
  }
  else
  {
    fontQueueColorState(
        ((u64) (u8) colorModulateByte(colorModulateByte((u8) ((statusByte0Get() >> 2) + 0x20), (u8) gauge->negative),
                                      alpha) << 24) | 0xE3FF5F);
    func_0021E9A8(rect.x, rect.y + 0x1E0, gauge->width << 4, 0x160);
    fontQueueColorState(
        ((u64) (u8) colorModulateByte(colorModulateByte((u8) ((statusByte0Get() >> 2) + 0x20),
                                                        (u8) (0x80 - (u8) gauge->negative)),
                                      alpha) << 24) | 0xE3FF5F);
    func_0021E9A8(rect.x, rect.y + 0x340, gauge->width << 4, 0x160);
  }
  if (gauge->state == 0)
  {
    if (visible)
    {
      xglFontSetColor(0x40, 0x44, 0x7C);
    }
    else
    {
      xglFontSetColor(0x5D, 0x7B, 0x76);
    }
  }
  else
  {
    xglFontSetColor(0x62, 0x62, 0x62);
  }
  xglFontPrintIndexed(0x100 - (xglFontGetFormattedWidth(messageTextGet(CHOICE_PROMPT_FIRST_ANSWER_TEXT)) >> 1),
                      ((rect.y - 0x71F8) >> 4) + 0x1E, messageTextGet(CHOICE_PROMPT_FIRST_ANSWER_TEXT));
  if (gauge->state == 1)
  {
    if (visible)
    {
      xglFontSetColor(0x40, 0x44, 0x7C);
    }
    else
    {
      xglFontSetColor(0x5D, 0x7B, 0x76);
    }
  }
  else
  {
    xglFontSetColor(0x62, 0x62, 0x62);
  }
  xglFontPrintIndexed(0x100 - (xglFontGetFormattedWidth(messageTextGet(CHOICE_PROMPT_SECOND_ANSWER_TEXT)) >> 1),
                      ((rect.y - 0x71F8) >> 4) + 0x34, messageTextGet(CHOICE_PROMPT_SECOND_ANSWER_TEXT));
}

void drawEntryReset(void *drawEntry);
void drawEntrySetValue(void *drawEntry, s32 spriteIndex);
void drawEntrySetFlag(void *drawEntry, s8 drawFlag);
void drawEntrySetAlpha(void *drawEntry, s32 alpha);

/* Resets the file list's draw entry and its own scroll fields: previousCursor to 0, cursor to the given
   limit, position to 0. */
void mcFileListScrollInit(McFileList *fileList, s32 position, s32 cursor)
{
    drawEntryReset(fileList);
    drawEntrySetValue(fileList, position);
    drawEntrySetFlag(fileList, 1);
    drawEntrySetAlpha(fileList, 0x80);
    fileList->previousCursor = 0;
    fileList->cursor = cursor;
    fileList->position = 0;
}

void addClampedCounter(McFileList *fileList, s16 delta)
{
  int unclampedValue;
  s16 truncatedValue;
  unclampedValue = delta + fileList->position;
  truncatedValue = unclampedValue;
  fileList->position = unclampedValue;
  if (fileList->cursor < truncatedValue)
  {
    fileList->position = fileList->cursor;
    return;
  }
  if (truncatedValue < 0)
  {
    fileList->position = 0;
  }
}

/* DrawEntry (tu370.c) has no header: the list passes its leading DrawEntry as a pointer. */
void drawEntrySetScale(void *drawEntry, s32 scale);

/* Steps previousCursor toward position by step (straight to it when closer than step) and sets the bar's scale to
   previousCursor * spriteOffset / cursor. */
void mcFileListScrollStep(McFileList *fileList, u16 step)
{
  if (((s16)fileList->position - fileList->previousCursor < 0 ? fileList->previousCursor - (s16)fileList->position
                                                             : (s16)fileList->position - fileList->previousCursor)
      < step)
  {
    fileList->previousCursor = fileList->position;
  }
  else if (fileList->previousCursor < (s16)fileList->position)
  {
    fileList->previousCursor += step;
  }
  else
  {
    fileList->previousCursor -= step;
  }
  drawEntrySetScale(fileList, (f32)fileList->previousCursor * fileList->spriteOffset / (f32)fileList->cursor);
}

s32 drawEntrySetPosition();
s32 func_002BE480(s32);

void tableSelectValue(s32 drawEntry) {
    drawEntrySetPosition();
    func_002BE480(drawEntry);
}

void func_00220130(TiFrameSheet *sheet, s32 index, s32 mask);

/* Sets the sheet cell's colour mask to 0xE3FF5F with the given alpha in the top byte. */
void sheetFrameSetCellAlpha(TiFrameSheet *sheet, s32 index, u64 alpha)
{
    func_00220130(sheet, index, (s32)(((alpha & 0xFF) << 24) | 0xE3FF5F));
}

extern s8 D_0055144A;

s8 sheetLatchedPositionGet(void) {
    return D_0055144A;
}

extern u8 gSaveAbortFlag;

void *memset(void *destination, int value, unsigned int count);
void xglClockRead(XglClock *clock);
unsigned int xglClockDayTime2UInt(XglClock *clock_time);
s32 func_001B8D08(s32 category, s32 item, s32 amount);

/* New-game reset: clears the roster table, the play-state header and the save image regions, stamps the
   start time and writes the per-character defaults. */
void newGameStateReset(void)
{
  memset(&gBattleRosterTable, 0, sizeof(gBattleRosterTable));
  memset(&gSaveGameHeader, 0, sizeof(gSaveGameHeader));
  xglClockRead(&gXglMcAgedEntries.newGameClock);
  gSaveGameHeader.startTime = xglClockDayTime2UInt(&gXglMcAgedEntries.newGameClock);
  gBattleRosterTable.characters[1].newGameDefaults[0] = 1;
  gBattleRosterTable.characters[2].newGameDefaults[0] = 0xC;
  gBattleRosterTable.characters[3].newGameDefaults[0] = 0x1B;
  gBattleRosterTable.characters[4].newGameDefaults[0] = 0x25;
  gBattleRosterTable.characters[5].newGameDefaults[0] = 0x34;
  gBattleRosterTable.characters[6].newGameDefaults[0] = 0x41;
  gBattleRosterTable.characters[7].newGameDefaults[0] = 0x51;
  gBattleRosterTable.characters[8].newGameDefaults[0] = 0x61;
  gBattleRosterTable.characters[9].newGameDefaults[0] = 0x62;
  gBattleRosterTable.characters[10].newGameDefaults[0] = 0x63;
  gBattleRosterTable.characters[13].newGameDefaults[0] = 0x6E;
  gBattleRosterTable.characters[13].newGameDefaults[1] = 0x6C;
  gBattleRosterTable.characters[14].newGameDefaults[0] = 0x7E;
  gBattleRosterTable.characters[14].newGameDefaults[1] = 0x80;
  gBattleRosterTable.characters[15].newGameDefaults[0] = 0x8E;
  gBattleRosterTable.characters[15].newGameDefaults[1] = 0x8F;
  gBattleRosterTable.characters[16].newGameDefaults[0] = 0x9E;
  gBattleRosterTable.characters[16].newGameDefaults[1] = 0;
  memset(gXglMcAgedEntries.newGameClearedRegion3A0C, 0, sizeof(gXglMcAgedEntries.newGameClearedRegion3A0C));
  memset(gXglMcAgedEntries.newGameClearedRegion157C, 0, sizeof(gXglMcAgedEntries.newGameClearedRegion157C));
  memset(gXglMcAgedEntries.newGameClearedRegion146D, 0, sizeof(gXglMcAgedEntries.newGameClearedRegion146D));
  memset(gXglMcAgedEntries.newGameClearedRegion147C, 0, sizeof(gXglMcAgedEntries.newGameClearedRegion147C));
  xglMcAgedEntriesReset();
  if (gSaveAbortFlag)
  {
    func_001B8D08(2, 0x4D, 1);
  }
  gSaveGameHeader.fileIndex = -1;
  gXglMcAgedEntries.newGameCounters[0] = 0;
  gXglMcAgedEntries.newGameCounters[1] = 0;
  gXglMcAgedEntries.newGameClearedFlag4A = 0;
  gXglMcAgedEntries.newGameClearedFlag4B = 0;
  memset(gXglMcAgedEntries.newGameClearedRegion1330, 0, sizeof(gXglMcAgedEntries.newGameClearedRegion1330));
}

BattleRoster *battleRosterGet(s32 rosterIndex) {
    return &gBattleRosterTable.rosterPair.rosters[rosterIndex];
}

u8 battleRosterEntryIdCountGet(s32 rosterIndex) {
    return gBattleRosterTable.rosterPair.rosters[rosterIndex].entryIdCount;
}

u8 battleRosterEntryIdGet(s32 entryIdIndex, s32 rosterIndex)
{
  if (!(entryIdIndex < battleRosterEntryIdCountGet(rosterIndex)))
  {
    return 0;
  }
  return gBattleRosterTable.rosterPair.rosters[rosterIndex].entryIds[entryIdIndex];
}

/* The call goes through an int-returning view of battleRosterEntryIdGet. Plain forms tried: the direct call and
   `entryId = battleRosterEntryIdGet(...); return entryId;` compile to a jal and a return instead of the original's
   tail jump (the u8 result type blocks the sibling call), so the two cursor getters come out 8 bytes longer in
   total and the link no longer fits. */
s32 battleRosterCursorEntryIdGet(s32 rosterIndex) {
    return ((s32 (*)()) battleRosterEntryIdGet)(gBattleRosterTable.rosterPair.rosters[rosterIndex].cursor, rosterIndex);
}

u8 battleCurrentRosterEntryCountGet(void)
{
  return gBattleRosterTable.rosterPair.rosters[gBattleRosterTable.currentRosterIndex].entryIdCount;
}

u8 battleCurrentRosterEntryIdGet(s32 entryIdIndex)
{
  return gBattleRosterTable.rosterPair.rosters[gBattleRosterTable.currentRosterIndex].entryIds[entryIdIndex];
}

/* Same int-returning view of battleRosterEntryIdGet as battleRosterCursorEntryIdGet. */
s32 battleCurrentRosterCursorEntryIdGet(void)
{
  return ((s32 (*)()) battleRosterEntryIdGet)(
      gBattleRosterTable.rosterPair.rosters[gBattleRosterTable.currentRosterIndex].cursor,
      gBattleRosterTable.currentRosterIndex);
}

/* Returns 1 when entryId is in its roster (roster 0 below BATTLE_ROSTER1_FIRST_ENTRY_ID, else roster 1), else 0. */
u8 battleRosterHasEntryId(s32 entryId)
{
  BattleRoster *roster;
  s32 index;
  roster = (entryId < BATTLE_ROSTER1_FIRST_ENTRY_ID) ? &gBattleRosterTable.rosterPair.rosters[0]
                                                     : &gBattleRosterTable.rosterPair.rosters[1];
  for (index = 0; index < roster->entryIdCount; index++)
  {
    if (roster->entryIds[index] == entryId)
    {
      return 1;
    }
  }
  return 0;
}

/* Selects the current roster (0 or 1). */
void battleCurrentRosterIndexSet(s32 rosterIndex)
{
  gBattleRosterTable.currentRosterIndex = rosterIndex;
  gBattleRosterTable.currentRosterIndex %= 2;
}

/* Entry id capacities of the two rosters (BattleRoster.entryIds has room for more; these are the limits the game
   enforces when adding). */
#define BATTLE_ROSTER0_ENTRY_ID_CAPACITY 7
#define BATTLE_ROSTER1_ENTRY_ID_CAPACITY 4

/* Appends entryId to its roster (roster 1 from BATTLE_ROSTER1_FIRST_ENTRY_ID on, else roster 0) when the roster does
   not hold it yet and still has room, and sets the id's bit in the table's entry mask. */
void battleRosterEntryIdAdd(s32 entryId)
{
  BattleRosterTable *table;
  BattleRoster *roster;
  s32 rosterIndex;
  s32 mask;
  s32 count;
  if (battleRosterHasEntryId(entryId) != 0)
  {
    return;
  }
  rosterIndex = entryId >= BATTLE_ROSTER1_FIRST_ENTRY_ID;
  roster = battleRosterGet(rosterIndex);
  table = &gBattleRosterTable;
  mask = 1 << entryId;
  count = roster->entryIdCount;
  if (rosterIndex != 0 ? count >= BATTLE_ROSTER1_ENTRY_ID_CAPACITY : count >= BATTLE_ROSTER0_ENTRY_ID_CAPACITY)
  {
    return;
  }
  roster->entryIds[count] = entryId;
  roster->entryIdCount++;
  table->entryMask |= mask;
}

/* Removes entry id from its roster: later entries move down one slot and the cursor follows its entry
   (a cursor on the removed entry goes back to 0). */
void battleRosterEntryIdRemove(s32 entryId)
{
  BattleRoster *roster;
  s32 index;
  roster = (entryId < BATTLE_ROSTER1_FIRST_ENTRY_ID) ? &gBattleRosterTable.rosterPair.rosters[0]
                                                     : &gBattleRosterTable.rosterPair.rosters[1];
  for (index = 0; index < roster->entryIdCount; index++)
  {
    if (roster->entryIds[index] == entryId)
    {
      if (roster->cursor == index)
      {
        roster->cursor = 0;
      }
      index++;
      if (index < roster->entryIdCount)
      {
        do
        {
          if (roster->cursor == index)
          {
            roster->cursor = index - 1;
          }
          roster->entryIds[index - 1] = roster->entryIds[index];
          index++;
        } while (index < roster->entryIdCount);
      }
      roster->entryIdCount--;
      return;
    }
  }
}

void battleRosterPairReset(void) {
    gBattleRosterTable.rosterPair.rosters[0].entryIdCount = 0;
    gBattleRosterTable.rosterPair.rosters[1].entryIdCount = 0;
    gBattleRosterTable.rosterPair.rosters[0].cursor = 0;
    gBattleRosterTable.rosterPair.rosters[1].cursor = 0;
}

/* Swaps entries indexA and indexB of a roster. When both swapped indices are below 3, a cursor on either index is
   moved through battleRosterCursorSet. For roster 0, if the cursor's entry id is at least the limit, scans indices
   0..2 and selects the first whose getter result is below it; an index past the count reads as 0 and also
   qualifies.
   value holds the entry count, then the swapped entry id, then the cursor. Plain form tried: three locals entryCount,
   swappedEntryId and cursor; the compiler then keeps the count in $3 and the swapped ids in $4/$5 instead of all
   three in $6, and the function no longer matches. */
void battleRosterEntrySwap(s32 indexA, s32 indexB, s32 rosterIndex)
{
  s32 value;
  s32 index;
  s32 entryId;
  BattleRoster *roster;
  value = battleRosterEntryIdCountGet(rosterIndex);
  if (!(indexA < value))
  {
    return;
  }
  if (!(indexB < value))
  {
    return;
  }
  roster = battleRosterGet(rosterIndex);
  value = roster->entryIds[indexA];
  roster->entryIds[indexA] = roster->entryIds[indexB];
  roster->entryIds[indexB] = value;
  if ((indexA < BATTLE_ROSTER_CURSOR_ENTRY_COUNT) && (indexB < BATTLE_ROSTER_CURSOR_ENTRY_COUNT))
  {
    value = gBattleRosterTable.rosterPair.rosters[rosterIndex].cursor;
    if (value == indexA)
    {
      battleRosterCursorSet(indexB, rosterIndex);
    }
    else if (value == indexB)
    {
      battleRosterCursorSet(indexA, rosterIndex);
    }
  }
  if (rosterIndex == 0)
  {
    if (battleRosterCursorEntryIdGet(0) >= BATTLE_ROSTER0_CURSOR_ENTRY_ID_LIMIT)
    {
      for (index = 0; index < BATTLE_ROSTER_CURSOR_ENTRY_COUNT; index++)
      {
        entryId = battleRosterEntryIdGet(index, 0);
        if (entryId < BATTLE_ROSTER0_CURSOR_ENTRY_ID_LIMIT)
        {
          battleRosterCursorSet(index, 0);
          break;
        }
      }
    }
  }
}

/* Puts the cursor of a roster on entry index when index is below BATTLE_ROSTER_CURSOR_ENTRY_COUNT; for roster 0 the
   entry id battleRosterEntryIdGet gives there must also be below BATTLE_ROSTER0_CURSOR_ENTRY_ID_LIMIT. Returns 1 when
   the cursor was set, 0 when it was left alone. */
s32 battleRosterCursorSet(s32 index, s32 rosterIndex)
{
  s32 entryId;
  if (index < BATTLE_ROSTER_CURSOR_ENTRY_COUNT)
  {
    if (rosterIndex == 0)
    {
      entryId = battleRosterEntryIdGet(index, rosterIndex);
      if (entryId >= BATTLE_ROSTER0_CURSOR_ENTRY_ID_LIMIT)
      {
        return 0;
      }
    }
    gBattleRosterTable.rosterPair.rosters[rosterIndex].cursor = index;
    return 1;
  }
  return 0;
}

/* Moves the cursor of the current roster forward to the next entry battleRosterCursorSet accepts, wrapping over the
   first one to three entries. Returns 1 when the cursor moved. */
s32 battleCurrentRosterCursorAdvance(void)
{
  s32 step;
  s32 rosterIndex;
  s32 cursor;
  s32 count;
  s32 wrapCount;
  rosterIndex = battleCurrentRosterIndexGet();
  cursor = battleRosterGet(rosterIndex)->cursor;
  count = battleRosterEntryIdCountGet(rosterIndex);
  wrapCount = count;
  if (count > BATTLE_ROSTER_CURSOR_ENTRY_COUNT)
  {
    wrapCount = BATTLE_ROSTER_CURSOR_ENTRY_COUNT;
  }
  else if (count <= 0)
  {
    wrapCount = 1;
  }
  for (step = 1; step < wrapCount; step++)
  {
    if (battleRosterCursorSet((cursor + step) % wrapCount, rosterIndex))
    {
      break;
    }
  }
  return cursor != battleRosterGet(rosterIndex)->cursor;
}

/* Returns the index of entryId in its roster (roster 1 from BATTLE_ROSTER1_FIRST_ENTRY_ID on, else roster 0), or -1
   when the roster does not hold it. */
s32 battleRosterEntryIdIndexGet(s32 entryId)
{
  BattleRoster *roster;
  s32 index;
  s32 result;
  roster = battleRosterGet(entryId >= BATTLE_ROSTER1_FIRST_ENTRY_ID);
  result = -1;
  for (index = 0; index < roster->entryIdCount; index++)
  {
    if (roster->entryIds[index] == entryId)
    {
      result = index;
      break;
    }
  }
  return result;
}

void battleUpdateEntryKinds(s32 firstEntryId, s32 secondEntryId, s32 thirdEntryId) {
    s32 entryIds[3];
    s32 entryId;
    s32 handleOrdinal;
    s32 resolvedEntryIndex;
    s32 retriedEntryIndex;

    entryIds[0] = firstEntryId;
    entryIds[1] = secondEntryId;
    entryIds[2] = thirdEntryId;
    for (handleOrdinal = 0; handleOrdinal < 3; handleOrdinal++) {
        entryId = entryIds[handleOrdinal];
        if (entryId != 0) {
            resolvedEntryIndex = battleRosterEntryIdIndexGet(entryId);
            if (resolvedEntryIndex == -1) {
                battleRosterEntryIdAdd(entryIds[handleOrdinal]);
                retriedEntryIndex = battleRosterEntryIdIndexGet(entryIds[handleOrdinal]);
                resolvedEntryIndex = retriedEntryIndex;
            }
            battleRosterEntrySwap(handleOrdinal, resolvedEntryIndex,
                                  entryIds[handleOrdinal] >= BATTLE_ROSTER1_FIRST_ENTRY_ID);
        }
    }
}

u8 battleCurrentRosterIndexGet(void) {
    return gBattleRosterTable.currentRosterIndex;
}

s32 testFlagBit(s32 bitIndex)
{
  int mask;
  mask = 1 << bitIndex;
  return (gBattleRosterTable.entryMask & mask) != 0;
}

/* Stores the two guest entry ids (VM native ptSetGuest); a lone second guest moves to the first place. */
void battleRosterHeaderPairSet(s32 firstGuestEntryId, s32 secondGuestEntryId)
{
  s32 firstGuest;
  s32 secondGuest;
  firstGuest = firstGuestEntryId;
  secondGuest = secondGuestEntryId;
  if ((firstGuest == 0) && (secondGuest != 0))
  {
    firstGuest = secondGuest;
    secondGuest = 0;
  }
  gBattleRosterTable.firstGuestEntryId = firstGuest;
  gBattleRosterTable.secondGuestEntryId = secondGuest;
}

/* For a roster 1 entry id (0xD..0x10), returns its linked roster 0 entry id number index & 1, counting only the
   linked ids present in roster 0 (present ones first); 0 when there is none or for any other entry id. */
s32 battleRosterLinkedEntryIdGet(s32 entryId, s32 index)
{
  static const u8 gBattleRosterLinkedEntryIds[4][2] = { { 7, 6 }, { 3, 4 }, { 5, 1 }, { 2, 0 } };
  s32 linked[2];
  s32 linkedEntryId;
  memset(linked, 0, sizeof(linked));
  linkedEntryId = 0;
  if (entryId >= BATTLE_ROSTER1_FIRST_ENTRY_ID && entryId <= BATTLE_ROSTER1_LAST_ENTRY_ID)
  {
    linked[0] = gBattleRosterLinkedEntryIds[entryId - BATTLE_ROSTER1_FIRST_ENTRY_ID][0];
    linked[1] = gBattleRosterLinkedEntryIds[entryId - BATTLE_ROSTER1_FIRST_ENTRY_ID][1];
    if (battleRosterHasEntryId(linked[1]) == 0)
    {
      linked[1] = 0;
    }
    if (battleRosterHasEntryId(linked[0]) == 0)
    {
      linked[0] = linked[1];
      linked[1] = 0;
    }
    linkedEntryId = linked[index & 1];
  }
  return linkedEntryId;
}

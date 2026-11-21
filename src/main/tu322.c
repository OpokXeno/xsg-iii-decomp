#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu322", func_0028A608);

INCLUDE_ASM("asm/main/nonmatchings/tu322", func_0028A840);

INCLUDE_ASM("asm/main/nonmatchings/tu322", func_0028A920);

INCLUDE_ASM("asm/main/nonmatchings/tu322", func_0028A9D8);

typedef int s32;
extern s32 gChannelTableEnabled;

/* One record reached from the table's selected slot: the base word at +4 and the element count at +8. */
typedef struct ChannelTable {
    unsigned char unmodeled_00[4];
    s32 channelBase;                           /* +0x04 */
    s32 channelCount;                          /* +0x08 */
} ChannelTable;

/* The 0x134-byte table at gChannelTableSetStorage: ten slot pointers, the selected slot at +0x28 and a ready flag at +0x2C. */
typedef struct ChannelTableSet {
    void *channelTables[10];                     /* +0x00: slot[0] also receives the index given to channelTableBegin */
    s32 selected;                       /* +0x28 */
    s32 ready;                          /* +0x2C */
    unsigned char unmodeled_30[0x134 - 0x30];
} ChannelTableSet;

void channelTableEnabledSet(s32 enabled) {
    gChannelTableEnabled = enabled;
}

void channelTableEnable(void) {
    gChannelTableEnabled = 1;
}

void channelTableDisable(void) {
    gChannelTableEnabled = 0;
}

extern unsigned char gChannelTableSetStorage[];
#define gChannelTableSet (*(ChannelTableSet *) gChannelTableSetStorage)

void *tableSelectedWordGet(void)
{
  return gChannelTableSet.channelTables[gChannelTableSet.selected];
}

s32 channelTableSelectedEndGet(void)
{
  ChannelTable *selectedTable;
  selectedTable = gChannelTableSet.channelTables[gChannelTableSet.selected];
  return selectedTable->channelBase + selectedTable->channelCount * 0xA0;
}

extern s32 gMovieStateTable[];

s32 movieFirstStateGet(void) {
    return gMovieStateTable[0];
}

INCLUDE_ASM("asm/main/nonmatchings/tu322", func_0028AB38);

s32 func_0028AB38(s32 tableId);
void *memset();

void channelTableBegin(s32 tableId) {
    memset(&gChannelTableSet, 0, sizeof(ChannelTableSet));
    gChannelTableSet.channelTables[0] = (void *) tableId;
    gChannelTableSet.ready = 1;
    gChannelTableSet.selected = 0;
    func_0028AB38(tableId);
}

INCLUDE_ASM("asm/main/nonmatchings/tu322", func_0028AC48);

INCLUDE_ASM("asm/main/nonmatchings/tu322", func_0028ACB0);

INCLUDE_ASM("asm/main/nonmatchings/tu322", func_0028B1E8);

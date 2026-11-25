#include "common.h"
#include "shared.h"

#include "main/tu105.h"

int sbNativeOwnerAdd(const char *name, SbNativeMethod *methods);

void scriptRegisterNatives(void);

extern char talkModuleName[];
extern char g2pathModuleName[];
extern char segModuleName[];
extern SbNativeMethod talkNativeTable[];
extern SbNativeMethod g2pathNativeTable[];
extern SbNativeMethod segNativeTable[];
extern SbNativeMethod talkNativeTable[17];
extern SbNativeMethod g2pathNativeTable[5];
extern SbNativeMethod segNativeTable[3];

/* .sdata of this file (0x69D7B8-0x69D7D0): the module names of scriptRegisterNatives. */
extern char talkModuleName[8];

extern char g2pathModuleName[8];
extern char segModuleName[8];

/*
 * Registers three native groups with the script VM: "talk" (table 0x3ec868),
 * "g2path" (table 0x3ec8f0) and "seg" (table 0x3ec918); the group names sit at
 * talkModuleName, g2pathModuleName and segModuleName.
 */
void scriptRegisterNatives(void)
{
    sbNativeOwnerAdd(talkModuleName, talkNativeTable);
    sbNativeOwnerAdd(g2pathModuleName, g2pathNativeTable);
    sbNativeOwnerAdd(segModuleName, segNativeTable);
}

typedef signed char s8;
typedef int s32;

#define offsetof(type, field) ((u32) &((type *) 0)->field)
#define CONTAINER_OF(member, type, field) ((type *) ((void *) (member) - offsetof(type, field)))

/*
 * The planeObject at gPlaneObject (state block +0x40): its position x and y, and a flag
 * at +0x800 (state block +0x840) that shifts the x position by -20 in stateObjectResetX.
 */
typedef struct PlaneObject {
    f32 positionX;
    f32 positionY;
    u8 unk_08[0x800 - 0x08];
    s32 isLeftShifted;
} PlaneObject;

/*
 * The state block at gSoundDriver (0x1F98 bytes): a 0x54000-byte and a 0x20000-byte
 * pool buffer (largePoolBuffer, smallPoolBuffer), 15 records of 0x10 bytes at +0x1870 (their bytes
 * +0x0C and +0x0D are the record flags) and the planeObject at gPlaneObject (+0x40).
 * Offsets marked unk_ are not yet known.
 */
/* One of the 15 records of 0x10 bytes; the bytes at +0x0C and +0x0D are flags. */
typedef struct VoiceRecord {
    u8 unk_00[0xC];
    u8 flagC;
    u8 flagD;
    u8 unk_0E[2];
} VoiceRecord;

typedef struct SoundDriver {
    s32 unk_00;
    s32 unk_04;
    s32 unk_08;
    s32 unk_0C;
    s32 unk_10;
    s32 unk_14;
    s32 unk_18;
    s32 unk_1C;
    s32 unk_20;
    s32 unk_24;
    s32 unk_28;
    s32 unk_2C;
    s32 unk_30;
    s32 unk_34;
    u8 unk_38[0x8];
    PlaneObject planeObject;
    u8 unk_844[0x1870 - 0x844];
    VoiceRecord voiceRecords[15];
    u8 unk_1960[0x1F30 - 0x1960];
    s32 unk_1F30;
    s32 pollStatus;
    s32 unk_1F38;
    s32 unk_1F3C;
    s32 unk_1F40;
    u8 unk_1F44[0x1F80 - 0x1F44];
    s32 largePoolBuffer;
    s32 smallPoolBuffer;
    s32 unk_1F88;
    s32 unk_1F8C;
    s32 loadedRecordValue;
    s8 unk_1F94;
    s8 unk_1F95;
    s8 selectedRecordIndex;
    s8 unk_1F97;
} SoundDriver;


extern s32 gSoundFlagA;
extern s32 gSoundFlagB;
extern s32 gSoundFlagC;
extern SoundDriver gSoundDriver[];
s32 memPoolAlloc(s32);
s32 messageTextGet(s32);
s32 seVoiceStatusReset();
s32 sineWavesReset();
s32 func_002D6180();
void stateSendTablePacket();
s32 soundTableValueGet();
s32 func_002D67C0();
s32 memset();
s32 xglPadSetRepeat(s32, s32, s32, s32);

void soundDriverInit(void) {
    gSoundDriver[0].unk_1F30 = 4;
    gSoundDriver[0].unk_1F3C = 1;
    gSoundDriver[0].unk_1F40 = 2;
    gSoundDriver[0].selectedRecordIndex = 0;
    gSoundDriver[0].unk_1F95 = 0;
    gSoundDriver[0].unk_1F94 = 0;
    gSoundDriver[0].unk_1F38 = 0;
    gSoundDriver[0].unk_1F97 = 0;
    gSoundDriver[0].largePoolBuffer = memPoolAlloc(0x54000);
    gSoundDriver[0].smallPoolBuffer = memPoolAlloc(0x20000);
    stateSendTablePacket();
    gSoundDriver[0].unk_00 = 0x10;
    gSoundDriver[0].unk_04 = soundTableValueGet(4);
    gSoundDriver[0].unk_08 = 0x40;
    gSoundDriver[0].unk_0C = soundTableValueGet(3);
    gSoundDriver[0].unk_10 = 0x20;
    gSoundDriver[0].unk_14 = soundTableValueGet(2);
    gSoundDriver[0].unk_18 = 8;
    gSoundDriver[0].unk_1C = soundTableValueGet(1);
    gSoundDriver[0].unk_20 = 4;
    gSoundDriver[0].unk_28 = 2;
    gSoundDriver[0].unk_24 = 0;
    gSoundDriver[0].unk_2C = messageTextGet(0x23);
    gSoundDriver[0].unk_30 = 1;
    gSoundDriver[0].unk_34 = 0;
    func_002D67C0();
    memset(gSoundDriver[0].voiceRecords, 0, 0xF0);
    func_002D6180();
    sineWavesReset();
    gSoundDriver[0].unk_1F8C = 0;
    xglPadSetRepeat(0, 0xF00C, 0xA, 3);
    seVoiceStatusReset();
    gSoundDriver[0].pollStatus = 0;
    gSoundFlagA = 0;
    gSoundFlagB = 0;
    gSoundFlagC = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D3D38);

extern u8 D_00402858[];
extern s32 gSoundUpdateMode[];
s32 xglFontFlush();
s32 func_002D4578();
s32 func_002D48C8();
void stateRefreshTables();

void frameStateUpdate(void) {
    switch (gSoundUpdateMode[0]) {
    case 1:
        func_002D4578();
        break;
    case 3:
        func_002D48C8();
        break;
    }
    if (D_00402858[0] & 8) {
        stateRefreshTables();
    }
    xglFontFlush();
}

s32 memPoolFree(s32);
s32 setPendingValue(s32);

void releaseStateBuffers(void) {
    gSoundDriver[0].unk_1F88 = 0;
    memPoolFree(gSoundDriver[0].largePoolBuffer);
    memPoolFree(gSoundDriver[0].smallPoolBuffer);
    setPendingValue(0);
}

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D3F70);

s32 soundFlagAIsSet(void) {
    return gSoundFlagA != 0;
}

s32 soundFlagBIsSet(void) {
    return gSoundFlagB != 0;
}

s32 soundFlagCIsSet(void) {
    return gSoundFlagC != 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4130);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4230);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D42D8);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4390);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4578);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4710);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4788);

extern f32 gPlaneBasePosition[];
extern PlaneObject gPlaneObject[];
void func_002D4A10();
void planeObjectSetPosition(PlaneObject *planeObject, f32 positionX, f32 positionY);
void func_002D4C50();
void func_002D58D0();
void func_002D5C80();

void stateObjectSetup(void) {
    PlaneObject *planeObject;
    SoundDriver *soundDriver;

    planeObject = gPlaneObject;
    soundDriver = CONTAINER_OF(planeObject, SoundDriver, planeObject);
    func_002D4A10(planeObject);
    soundDriver->planeObject.isLeftShifted = 1;
    planeObjectSetPosition(planeObject, gPlaneBasePosition[0], gPlaneBasePosition[1]);
    func_002D4C50(planeObject, soundDriver->selectedRecordIndex);
    func_002D58D0();
    func_002D5C80();
    soundDriver->pollStatus = 3;
}

void voiceRecordBegin();
void voiceRecordEnd();
void voiceTableRecordRead();

void planeObjectLoadTableRecord(void) {
    PlaneObject *planeObject;
    SoundDriver *soundDriver;
    s32 voiceRecordWords[0xC];
    s32 loadedRecordValue;

    voiceRecordBegin();
    planeObject = gPlaneObject;
    soundDriver = CONTAINER_OF(planeObject, SoundDriver, planeObject);
    func_002D4C50(planeObject, soundDriver->selectedRecordIndex);
    voiceRecordEnd();
    voiceTableRecordRead(voiceRecordWords, soundDriver->selectedRecordIndex);
    loadedRecordValue = voiceRecordWords[7];
    soundDriver->loadedRecordValue = loadedRecordValue;
}

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D48C8);

void stateObjectBegin(void) {

}

void stateObjectEnd(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4A10);

void planeObjectSetPosition(PlaneObject *planeObject, f32 positionX, f32 positionY) {
    planeObject->positionX = positionX;
    planeObject->positionY = positionY;
}

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4C50);

extern s32 gPlaneResetMode[];

void stateObjectResetX(PlaneObject *planeObject) {
    if (gPlaneResetMode[0] == 2) {
        if (planeObject->isLeftShifted != 0) {
            planeObjectSetPosition(planeObject, gPlaneBasePosition[0] - 20.0f, planeObject->positionY);
        } else {
            planeObjectSetPosition(planeObject, gPlaneBasePosition[0], planeObject->positionY);
        }
    }
    planeObjectSetPosition(planeObject, planeObject->positionX, planeObject->positionY);
}

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D4D90);

extern f32 gPlaneOriginX;
s32 njPlaneInit();
s32 njPlaneSetOffset(s32 *, f32, f32);
s32 njPlaneSetOrigin(s32 *, f32, f32);
s32 func_002CFDD0();
s32 func_002D10D0();

void stateObjectInitPlane(PlaneObject *planeObject) {
    s32 planeTransform[0x30];

    if (planeObject->isLeftShifted == 0) {
        njPlaneInit(&planeTransform[0]);
        njPlaneSetOffset(&planeTransform[0], planeObject->positionX, planeObject->positionY);
        njPlaneSetOrigin(&planeTransform[0], gPlaneOriginX, 103.0f);
        func_002CFDD0(&planeTransform[0], 0x40000000);
        func_002D10D0(&planeTransform[0]);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D5658);

u32 sineWaveValue32();
#include "main/njPlane.h"
s32 njPlaneSetLevel(s32 *, f32);
s32 njPlaneSetMode();
s32 func_002CFFF0();

void statePlaneDrawCursor(s32 cursorX, s32 cursorY, s32 selected)
{
  s32 transformA[0x30];
  s32 transformB[0x30];
  int alphaB;
  int alphaA;
  u32 randomA;
  u32 randomB;
  njPlaneInit(&transformB[0]);
  njPlaneSetOrigin(&transformB[0], 18.0f, 6.0f);
  njPlaneSetOffset(&transformB[0], cursorX, cursorY);
  njPlaneInit(&transformA[0]);
  njPlaneSetMode(&transformA[0], 0x20);
  njPlaneSetOrigin(&transformA[0], 38.0f, 20.0f);
  njPlaneSetOffset(&transformA[0], cursorX - 0xB, cursorY - 7);
  if (selected != 0)
  {
    njPlaneSetTint(&transformA[0], 95.0f, 255.0f, 227.0f);
    randomA = sineWaveValue32();
    njPlaneSetLevel(&transformA[0], 0x20 - (alphaA = (randomA >> 2) & 0xFF));
    func_002CFFF0(&transformB[0], 0x1D, 0);
  }
  else
  {
    njPlaneSetTint(&transformA[0], 128.0f, 137.0f, 249.0f);
    randomB = sineWaveValue32();
    alphaB = (randomB >> 2) & 0xFF;
    njPlaneSetLevel(&transformA[0], 0x20 - alphaB);
    func_002CFFF0(&transformB[0], 0x1E, 0);
  }
  func_002CFFF0(&transformA[0], 0x1F, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D58D0);

void voiceRecordBegin(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D59E0);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D5C80);

void voiceRecordEnd(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D5E28);

extern unsigned char D_00403AF0[];

void soundTableClear(void)
{
  s32 i;
  u8 *cursor;
  unsigned char *soundTableBytes;
  soundTableBytes = D_00403AF0;
  cursor = soundTableBytes + 0xE;
  for (i = 0; i < 15; i++)
  {
    *cursor = 0;
    cursor -= 1;
  }
}

void soundTableByteSet(s32 soundTableIndex, s32 soundTableValue) {
    D_00403AF0[soundTableIndex] = soundTableValue;
}

s8 soundTableByteGet(s32 soundTableIndex) {
    return D_00403AF0[soundTableIndex];
}

void voiceFlagCClear(void)
{
  s32 i;
  SoundDriver *soundDriver;
  u8 *cursor;
  soundDriver = gSoundDriver;
  cursor = &soundDriver->voiceRecords[14].flagC;
  for (i = 0; i < 15; i++)
  {
    *cursor = 0;
    cursor -= sizeof(VoiceRecord);
  }
}

extern s8 gVoiceRecordFlagC[];

void voiceFlagCSet(s32 voiceIndex, s32 voiceFlagValue) {
    gVoiceRecordFlagC[voiceIndex * 0x10] = voiceFlagValue;
}

s8 voiceFlagCGet(s32 voiceIndex) {
    return gVoiceRecordFlagC[voiceIndex * 0x10];
}

void voiceFlagDClear(void)
{
  s32 i;
  SoundDriver *soundDriver;
  u8 *cursor;
  soundDriver = gSoundDriver;
  cursor = &soundDriver->voiceRecords[14].flagD;
  for (i = 0; i < 15; i++)
  {
    *cursor = 0;
    cursor -= sizeof(VoiceRecord);
  }
}

extern s8 gVoiceRecordFlagD[];

void voiceFlagDSet(s32 voiceIndex, s32 voiceFlagValue) {
    gVoiceRecordFlagD[voiceIndex * 0x10] = voiceFlagValue;
}

s8 voiceFlagDGet(s32 voiceIndex) {
    return gVoiceRecordFlagD[voiceIndex * 0x10];
}

void soundTableClear();
void voiceFlagCClear();
void voiceFlagDClear();

void soundTablesClearAll(void) {
    soundTableClear();
    voiceFlagCClear();
    voiceFlagDClear();
}

void soundTableReset(void) {
    memset(D_00403AF0, 0, 0xF);
}

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6180);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6270);

extern unsigned char gSoundTableData[];
s32 func_001B5F28();
void xcBlockClear(void *block);
s32 func_001B70F0();

void stateSendTablePacket(void) {
    s32 packet[8];

    xcBlockClear(&packet[0]);
    packet[1] = 0x09000009;
    packet[2] = (s32) gSoundTableData;
    func_001B70F0(&packet[0]);
    func_001B5F28(0, 0, 0);
}

s32 tableGetWord();
s32 func_002C7C50();

void voiceTableRecordRead(s32 *voiceRecordWords, s32 voiceIndex) {
    u8 *voiceTable;
    s32 voiceOffset;

    voiceTable = &gSoundTableData[tableGetWord(gSoundTableData, 1)];
    voiceOffset = voiceIndex * 0xC;
    voiceRecordWords[1] = func_002C7C50(voiceTable, voiceOffset + 1);
    voiceRecordWords[2] = func_002C7C50(voiceTable, voiceOffset + 2);
    voiceRecordWords[3] = tableGetWord(voiceTable, voiceOffset + 3);
    voiceRecordWords[4] = tableGetWord(voiceTable, voiceOffset + 4);
    voiceRecordWords[5] = func_002C7C50(voiceTable, voiceOffset + 5);
    voiceRecordWords[6] = func_002C7C50(voiceTable, voiceOffset + 6);
    voiceRecordWords[7] = func_002C7C50(voiceTable, voiceOffset + 7);
    voiceRecordWords[8] = func_002C7C50(voiceTable, voiceOffset + 8);
}

s32 soundTableValueGet(s32 subtableIndex) {
    s32 subtableOffset;
    subtableOffset = tableGetWord(gSoundTableData, 3);
    return func_002C7C50(&gSoundTableData[subtableOffset], subtableIndex);
}

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6478);

void seVoiceStartDefault(unsigned int handle);

s32 stateStatusPoll(void)
{
  seVoiceStartDefault(6);
  if (gSoundDriver[0].pollStatus == 1)
  {
    return 1;
  }
  if (gSoundDriver[0].pollStatus == 3)
  {
    gSoundDriver[0].pollStatus = 0;
  }
  return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6598);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6678);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6740);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D67C0);

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D6828);

s32 func_002C9938();
s32 func_002C9A00();
s32 seObjectSetSetting31();
s32 func_002C9C00();

void stateRefreshTables(void) {
    s32 refreshBuffer[0xAC];

    func_002C9938(&refreshBuffer[0]);
    seObjectSetSetting31(&refreshBuffer[0], soundTableValueGet(0xF));
    func_002C9A00(&refreshBuffer[0], soundTableValueGet(0x10));
    func_002C9C00(&refreshBuffer[0]);
}

INCLUDE_ASM("asm/main/nonmatchings/tu392", func_002D68E0);

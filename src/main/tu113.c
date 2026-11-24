#include "common.h"

#include "main/tu007.h"
void xglNoOpHook(void) {
}

void xglNoOpHookAlt(void) {
}

extern PadRecord PadData[];

typedef struct CommandBytes {
    u8 bytes[8];
} CommandBytes;

extern CommandBytes D_0053E860;
extern CommandBytes D_0053E868;
void func_001E8108(int flags);
void scratchpadSendCmd0A(u32 *sendParams);

#define SCRATCHPAD_WORDS ((u32 *) 0x70000000)
/* EE scratchpad: the packet words are copied here. The empty do/while is a scheduling barrier between the copies and
   the sends: without it the emitted order changes (kept: it decides the emitted order). */
#define SCRATCHPAD_WORDS ((u32 *) 0x70000000)
#define SCRATCHPAD_ORDER_BARRIER() do { } while (0)

/* Copies the 14 words of sendParams to the scratchpad, then sends the stored command 0x0A pairs by pad button:
   held 0x10 calls func_001E8108; pressed 4 sends D_0053E860, pressed 1 sends D_0053E868. */
void func_001E6DF8(u32 *sendParams) {
    CommandBytes firstCommand;
    CommandBytes secondCommand;
    u32 packetWord1;
    u32 packetWord2;
    u32 packetWord3;
    u32 packetWord4;
    u32 packetWord5;
    u32 packetWord6;
    u32 packetWord7;
    u32 packetWord8;
    u32 packetWord9;
    u32 packetWord10;
    u32 packetWord11;
    u32 packetWord12;
    u32 packetWord13;
    u32 packetWord14;
    u16 held;
    u16 pressed;
    packetWord1 = sendParams[1];
    packetWord2 = sendParams[2];
    packetWord3 = sendParams[3];
    packetWord4 = sendParams[4];
    packetWord5 = sendParams[5];
    packetWord6 = sendParams[6];
    packetWord7 = sendParams[7];
    packetWord8 = sendParams[8];
    packetWord9 = sendParams[9];
    packetWord10 = sendParams[10];
    packetWord11 = sendParams[11];
    packetWord12 = sendParams[12];
    packetWord13 = sendParams[13];
    packetWord14 = sendParams[14];
    SCRATCHPAD_ORDER_BARRIER();
    SCRATCHPAD_WORDS[0] = packetWord1;
    SCRATCHPAD_WORDS[1] = packetWord2;
    SCRATCHPAD_WORDS[2] = packetWord3;
    SCRATCHPAD_WORDS[3] = packetWord4;
    SCRATCHPAD_WORDS[4] = packetWord5;
    SCRATCHPAD_WORDS[5] = packetWord6;
    SCRATCHPAD_WORDS[6] = packetWord7;
    SCRATCHPAD_WORDS[7] = packetWord8;
    SCRATCHPAD_WORDS[8] = packetWord9;
    SCRATCHPAD_WORDS[9] = packetWord10;
    SCRATCHPAD_WORDS[10] = packetWord11;
    SCRATCHPAD_WORDS[11] = packetWord12;
    SCRATCHPAD_WORDS[12] = packetWord13;
    SCRATCHPAD_WORDS[13] = packetWord14;

    held = PadData[0].held;
    func_001E8108(held & 0x10);
    pressed = PadData[0].pressed;
    if (pressed & 4) {
        firstCommand = D_0053E860;
        scratchpadSendCmd0A((u32 *) &firstCommand);
    }
    pressed = PadData[0].pressed;
    if (pressed & 1) {
        secondCommand = D_0053E868;
        scratchpadSendCmd0A((u32 *) &secondCommand);
    }
}

void scratchpadClearSendCmd29(void);
int slotStateInactive(void);

/* Copies the 9 words of sendParams to the scratchpad; pressed 0x80 clears the cmd29 state and held 0x10 makes the
   slot inactive. */
void func_001E6F48(u32 *sendParams) {
    u32 packetWord1;
    u32 packetWord2;
    u32 packetWord3;
    u32 packetWord4;
    u32 packetWord5;
    u32 packetWord6;
    u32 packetWord7;
    u32 packetWord8;
    u32 packetWord9;
    packetWord1 = sendParams[1];
    packetWord2 = sendParams[2];
    packetWord3 = sendParams[3];
    packetWord4 = sendParams[4];
    packetWord5 = sendParams[5];
    packetWord6 = sendParams[6];
    packetWord7 = sendParams[7];
    packetWord8 = sendParams[8];
    packetWord9 = sendParams[9];
    SCRATCHPAD_ORDER_BARRIER();
    SCRATCHPAD_WORDS[0] = packetWord1;
    SCRATCHPAD_WORDS[1] = packetWord2;
    SCRATCHPAD_WORDS[2] = packetWord3;
    SCRATCHPAD_WORDS[3] = packetWord4;
    SCRATCHPAD_WORDS[4] = packetWord5;
    SCRATCHPAD_WORDS[5] = packetWord6;
    SCRATCHPAD_WORDS[6] = packetWord7;
    SCRATCHPAD_WORDS[7] = packetWord8;
    SCRATCHPAD_WORDS[8] = packetWord9;

    if (PadData[0].pressed & 0x80) {
        scratchpadClearSendCmd29();
    }

    if (PadData[0].held & 0x10) {
        slotStateInactive();
    }
}

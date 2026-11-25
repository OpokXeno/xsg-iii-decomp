#include "common.h"
#include "shared.h"

void xglSleep(void);
void xglRenderFrame(void);
void xglStudioUpdate(void);
void xglCdControlThread(void);
void func_002E7750(unsigned int mode);
void func_002F7718(unsigned int mode);
int func_002ED8C0(const char *image);
int func_002ED870(void);
void func_002EC440(void);
void func_002ECB20(void);
void func_002F8490(int media);
void func_002E92B0(void);
void xglCdInitHookNoop(void);
void xglCdSifLoadModule(const char *name, const char *args);
void func_003031D0(void);
void func_00303328(int a, int b, int c);
void xglCdInitial(void);
void func_0018FD58(void);
void xglPadInitial(void);
void xglMcInitial(void);
void xglDmaInitial(void);
void xglIntrInit(void);
void func_00193600(void);
void xglGeometryInit(void);
void xglPacketInitDoubleBuffer(unsigned int base, unsigned int size, float rate);
void func_0019ADD8(void);
void func_00185D58(void);
void moviePlayerSetSize(void);
void initScratchpadBuffers(void);

const char iopRebootImagePath[32] = "cdrom0:\\IOP\\IOPRP300.IMG;1";
const char sio2manModuleName[8] = "sio2man";
const char mcmanModuleName[8] = "mcman";
const char mcservModuleName[8] = "mcserv";
const char padmanModuleName[8] = "padman";
const char libsdModuleName[8] = "libsd";
const char sdrdrvModuleName[8] = "sdrdrv";
const char adxiModuleName[16] = "cri_adxi";
const char adxiModuleArgs[24] = "sdinit=0\0spucore=2";
const char sndfiModuleName[8] = "sndfi";
const char iopsubModuleName[16] = "iopsub";

void InitializeSystem(void) {
    func_002E7750(0);
    func_002F7718(0);
    while (func_002ED8C0(iopRebootImagePath) == 0) {
    }
    while (func_002ED870() == 0) {
    }
    func_002E7750(0);
    func_002EC440();
    func_002ECB20();
    func_002F7718(0);
    func_002F8490(2);
    func_002E92B0();
    xglCdInitHookNoop();
    xglCdSifLoadModule(sio2manModuleName, 0);
    xglCdSifLoadModule(mcmanModuleName, 0);
    xglCdSifLoadModule(mcservModuleName, 0);
    xglCdSifLoadModule(padmanModuleName, 0);
    xglCdSifLoadModule(libsdModuleName, 0);
    xglCdSifLoadModule(sdrdrvModuleName, 0);
    func_003031D0();
    func_00303328(1, 0x8000, 0);
    xglCdSifLoadModule(adxiModuleName, adxiModuleArgs);
    xglCdSifLoadModule(sndfiModuleName, 0);
    xglCdSifLoadModule(iopsubModuleName, 0);
    xglCdInitial();
    func_0018FD58();
    xglPadInitial();
    xglMcInitial();
    xglDmaInitial();
    xglIntrInit();
    func_00193600();
    xglGeometryInit();
    xglPacketInitDoubleBuffer(0xB00000, 0x180000, 50.0f);
    func_0019ADD8();
    func_00185D58();
    moviePlayerSetSize();
    initScratchpadBuffers();
}

extern int mainParamArgc;
extern char **mainParamArgv;
void func_00309B48(void);
void bootPreInitHook(void);
void xglCdOpenDevice(void);
void xglRenderMovieBegin(void);
void xglStudioInit(void);
void seLoadMasterBank(void);
void func_00A80668(void);
void xglCdLoadOverlay(unsigned int overlay);
unsigned int func_00A81718(void);

/* An overlay entry word (low 24 bits: the code address) seen as an address or as the function it names. */
typedef union OverlayEntry {
    unsigned int address;
    unsigned int (*call)(void);
} OverlayEntry;

int gameBootAndScreenLoop(int argc, char **argv) {
    unsigned int selection;
    OverlayEntry chosenScreen;

    func_00309B48();
    mainParamArgc = argc;
    mainParamArgv = argv;
    bootPreInitHook();
    InitializeSystem();
    xglCdOpenDevice();
    xglRenderMovieBegin();
    xglRenderFrame();
    xglCdControlThread();
    xglStudioInit();
    xglStudioUpdate();
    seLoadMasterBank();
    xglCdLoadOverlay(1);
    func_00A80668();

    for (;;) {
        xglCdLoadOverlay(1);
        selection = func_00A81718();
        if (selection == 0) {
            continue;
        }
        chosenScreen.address = selection & 0xFFFFFF;
        if (chosenScreen.call == func_00A81718) {
            continue;
        }
        if ((selection >> 24) == 1) {
            continue;
        }
        xglCdLoadOverlay(selection >> 24);
        chosenScreen.call();
    }
}

void xglSleep(void) {
    xglRenderFrame();
    xglCdControlThread();
    xglStudioUpdate();
}

int mainParamArgc = 0;
char **mainParamArgv = 0;

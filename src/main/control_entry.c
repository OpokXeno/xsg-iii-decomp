#include "common.h"
#include "shared.h"

void xglSleep(void);
void xglRenderFrame(void);
void xglStudioUpdate(void);
void xglCdControlThread(void);
extern const char D_00527800[];
extern const char D_00527820[];
extern const char D_00527828[];
extern const char D_00527830[];
extern const char D_00527838[];
extern const char D_00527840[];
extern const char D_00527848[];
extern const char D_00527850[];
extern const char D_00527860[];
extern const char D_00527878[];
extern const char D_00527880[];
void func_002E7750(unsigned int mode);
void func_002F7718(unsigned int mode);
int func_002ED8C0(const char *image);
int func_002ED870(void);
void func_002EC440(void);
void func_002ECB20(void);
void func_002F8490(int media);
void func_002E92B0(void);
void func_00188D38(void);
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
void func_00197128(void);
void func_001998E0(unsigned int base, unsigned int size, float rate);
void func_0019ADD8(void);
void func_00185D58(void);
void moviePlayerSetSize(void);
void func_00213BD8(void);

void InitializeSystem(void) {
    func_002E7750(0);
    func_002F7718(0);
    while (func_002ED8C0(D_00527800) == 0) {
    }
    while (func_002ED870() == 0) {
    }
    func_002E7750(0);
    func_002EC440();
    func_002ECB20();
    func_002F7718(0);
    func_002F8490(2);
    func_002E92B0();
    func_00188D38();
    xglCdSifLoadModule(D_00527820, 0);
    xglCdSifLoadModule(D_00527828, 0);
    xglCdSifLoadModule(D_00527830, 0);
    xglCdSifLoadModule(D_00527838, 0);
    xglCdSifLoadModule(D_00527840, 0);
    xglCdSifLoadModule(D_00527848, 0);
    func_003031D0();
    func_00303328(1, 0x8000, 0);
    xglCdSifLoadModule(D_00527850, D_00527860);
    xglCdSifLoadModule(D_00527878, 0);
    xglCdSifLoadModule(D_00527880, 0);
    xglCdInitial();
    func_0018FD58();
    xglPadInitial();
    xglMcInitial();
    xglDmaInitial();
    xglIntrInit();
    func_00193600();
    func_00197128();
    func_001998E0(0xB00000, 0x180000, 50.0f);
    func_0019ADD8();
    func_00185D58();
    moviePlayerSetSize();
    func_00213BD8();
}

extern int D_00550280;
extern char **D_00550284;
void func_00309B48(void);
void func_00A80000(void);
void xglCdOpenDevice(void);
void func_0019B670(void);
void func_0019BA20(void);
void seLoadMasterBank(void);
void func_00A80668(void);
void xglCdLoadOverlay(unsigned int overlay);
unsigned int func_00A81718(void);

int func_00181AF0(int argc, char **argv) {
    unsigned int selection;
    unsigned int (*entry)(void);

    func_00309B48();
    D_00550280 = argc;
    D_00550284 = argv;
    func_00A80000();
    InitializeSystem();
    xglCdOpenDevice();
    func_0019B670();
    xglRenderFrame();
    xglCdControlThread();
    func_0019BA20();
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
        entry = (unsigned int (*)(void))(selection & 0xFFFFFF);
        if (entry == func_00A81718) {
            continue;
        }
        if ((selection >> 24) == 1) {
            continue;
        }
        xglCdLoadOverlay(selection >> 24);
        entry();
    }
}

void xglSleep(void) {
    xglRenderFrame();
    xglCdControlThread();
    xglStudioUpdate();
}

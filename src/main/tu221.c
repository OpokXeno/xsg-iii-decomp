#include "common.h"
#include "shared.h"
#include "main/gifState.h"

#include "main/tu007.h"

#define PAD_UP 0x1000
#define PAD_DOWN 0x4000
extern GifStatusBlock gifStatus;
/* Scaffold data without a config name (splat names): the per-mode table of pointer-sized entries, and the
 * word the selected entry is copied into. */
extern void *gifStateEffectDraws[];
void gifStateDrawRadialBlur();
void gifStateDrawRing();
void gifStateDrawCameraQuad();
void gifStateDrawJitterGrid();
void gifStateDrawFadeQuad();
void gifStateDrawNone();
extern void *gifStateModeHook[];

void gifStateSetMode(int effectMode)
{
    if (effectMode < 6) {
        gifStateModeHook[0] = gifStateEffectDraws[effectMode];
        gifStatus.state = effectMode;
        gifStatus.ratio = 1.0f;
    }
}

void gifStateClearModeHook(void) {
    gifStateModeHook[0] = 0;
}

void gifStateNoOp(void) {

}

void eventViewerFileMenu(void);
void movieListScreen(void);
void movieMenuRun(void);
void glowRunScene(void);
void tiWaterSceneLoop(void);
void blendTestMain(void);
void cullingTestMain(void);
void blendNoopStop(void);
void XstAnimTestMain(void);
void scisTestRun(void);
void narrEvCatPictureTest(void);
void GtestMainMapAndPad(void);
void GtestMultiActorMain(void);
void func_00247790(void);
void motionViewerNoOp(void);
void movieTestMainLoop(void);
void xglFontPrint(int x, int y, unsigned int color, const char *text);
void xglFontPrintDirect(const char *text);
void xglFontDebugPrefixSet(int *debugPrefix);
int func_00185CB0(int, int);
extern PadRecord PadData[];
void xglSleep(void);
extern int gifStateTravelAlpha;
extern int *D_00550D24;

/*
 * Debug test-menu loop: lists the sixteen test scenes, moves the cursor with the d-pad and runs the
 * highlighted one with the 0x20 button; SELECT held with START pressed leaves the loop.
 */
void debugTestMenuMain(void) {
    void (*menuActions[16])(void) = {eventViewerFileMenu,    movieListScreen, movieMenuRun,    glowRunScene,
                               tiWaterSceneLoop, blendTestMain, cullingTestMain,  blendNoopStop,
                               XstAnimTestMain,    scisTestRun, narrEvCatPictureTest,    GtestMainMapAndPad,
                               GtestMultiActorMain,    func_00247790, motionViewerNoOp, movieTestMainLoop};
    const char *menuLabels[16] = {
        "\xa1\xa6\xa3\xc5\xa3\xd6\xa3\xc5\xa3\xce\xa3\xd4\xa1\xa1\xa3\xd6\xa3\xc9\xa3\xc5\xa3\xd7\xa3\xc5\xa3\xd2" /* ・ＥＶＥＮＴ　ＶＩＥＷＥＲ */
        ,
        "\xa1\xa6\xa3\xcd\xa3\xcf\xa3\xd6\xa3\xc9\xa3\xc5\xa1\xa1\xa3\xd6\xa3\xc9\xa3\xc5\xa3\xd7\xa3\xc5\xa3\xd2" /* ・ＭＯＶＩＥ　ＶＩＥＷＥＲ */
        ,
        "\xa1\xa6\xa3\xcd\xa3\xcf\xa3\xd4\xa3\xc9\xa3\xcf\xa3\xce\xa1\xa1\xa3\xd6\xa3\xc9\xa3\xc5\xa3\xd7\xa3\xc5\xa3\xd2" /* ・ＭＯＴＩＯＮ　ＶＩＥＷＥＲ */
        ,
        "\xa1\xa6\xa3\xc7\xa3\xcc\xa3\xcf\xa3\xd7" /* ・ＧＬＯＷ */,
        "\xa1\xa6\xa3\xd7\xa3\xc1\xa3\xd4\xa3\xc5\xa3\xd2" /* ・ＷＡＴＥＲ */,
        "\xa1\xa6\xa3\xc2\xa3\xcc\xa3\xc5\xa3\xce\xa3\xc4" /* ・ＢＬＥＮＤ */,
        "\xa1\xa6\xa3\xc3\xa3\xd5\xa3\xcc\xa3\xcc\xa3\xc9\xa3\xce\xa3\xc7" /* ・ＣＵＬＬＩＮＧ */,
        "\xa1\xa6\xa3\xc2\xa3\xd2\xa3\xc1\xa3\xd6\xa3\xc5\xa1\xa1\xa3\xd3\xa3\xd4\xa3\xcf\xa3\xd2\xa3\xd9" /* ・ＢＲＡＶＥ　ＳＴＯＲＹ */
        ,
        "\xa1\xa6\xa3\xd8\xa3\xd3\xa3\xd4\xa1\xa1\xa3\xc1\xa3\xce\xa3\xc9\xa3\xcd" /* ・ＸＳＴ　ＡＮＩＭ */,
        "\xa1\xa6\xa3\xd3\xa3\xc3\xa3\xc9\xa3\xd3" /* ・ＳＣＩＳ */,
        "\xa1\xa6\xa3\xce\xa3\xc1\xa3\xd2\xa3\xd2" /* ・ＮＡＲＲ */,
        "\xa1\xa6\xa3\xc1\xa3\xcd\xa3\xd5\xa3\xd2\xa3\xcf" /* ・ＡＭＵＲＯ */,
        "\xa1\xa6\xa3\xc1\xa3\xcd\xa3\xd5\xa3\xd2\xa3\xcf\xa3\xb2" /* ・ＡＭＵＲＯ２ */,
        "\xa1\xa6\xa3\xc7\xa3\xd5\xa3\xce\xa3\xc4\xa3\xc1\xa3\xcd" /* ・ＧＵＮＤＡＭ */,
        "\xa1\xa6\xa3\xcd\xa3\xcf\xa3\xc4\xa3\xc5\xa3\xcc\xa1\xa1\xa3\xd6\xa3\xc9\xa3\xc5\xa3\xd7\xa3\xc5\xa3\xd2" /* ・ＭＯＤＥＬ　ＶＩＥＷＥＲ */
        ,
        "\xa1\xa6\xa3\xc3\xa3\xc6\xa1\xa1\xa3\xcc\xa3\xc9\xa3\xc7\xa3\xc8\xa3\xd4" /* ・ＣＦ　ＬＩＧＨＴ */
    };
    int selectedIndex;
    int row;
    unsigned short repeatedButtons;

    selectedIndex = 0;
    gifStateNoOp();
    func_00185CB0(1, 0);
    for (;;) {
        xglFontDebugPrefixSet(D_00550D24);
        if ((PadData[0].heldAndPressed & 0x08000100) == 0x08000100) {
            break;
        }
        repeatedButtons = PadData[0].repeated;
        /* kept: the net-zero -- ++ steps below decide the emitted order (removing them is NOTSTRICT) */
        if (repeatedButtons & PAD_UP) {
            selectedIndex--;
            selectedIndex++;
            selectedIndex--;
        }
        if (repeatedButtons & PAD_DOWN) {
            selectedIndex++;
            selectedIndex--;
            selectedIndex++;
        }
        if (selectedIndex < 0) {
            selectedIndex = 15;
        }
        if (selectedIndex >= 16) {
            selectedIndex = 0;
        }
        if ((PadData[0].pressed & 0x20) && menuActions[selectedIndex] != 0) {
            menuActions[selectedIndex]();
            gifStateNoOp();
        }
        for (row = 0; row < 16; row++) {
            if (selectedIndex == row) {
                xglFontPrint(20, row * 20 + 20, 0xFFFFFF,
                             "$bg3;$prop1;$han1;$xx0,36;$xx1,8;$xx2,0;$col60c000;");
                xglFontPrintDirect(menuLabels[row]);
            } else {
                xglFontPrint(20, row * 20 + 20, 0xFFFFFF,
                             "$bg3;$prop1;$han1;$xx0,36;$xx1,8;$xx2,0;$col707070;");
                xglFontPrintDirect(menuLabels[row]);
            }
        }
        xglSleep();
    }
}

void *gifStateEffectDraws[16] = {
    gifStateDrawRadialBlur,
    gifStateDrawRing,
    gifStateDrawCameraQuad,
    gifStateDrawJitterGrid,
    gifStateDrawFadeQuad,
    gifStateDrawNone,
};

int *D_00550D24 = &gifStateTravelAlpha + 1;
